/******************************************************************
	file: import.c
	routines for processing imports in prds
******************************************************************/
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <gdd.h>

#define EXT extern
#include "prds.h"


static int dsp_msg1[LIST_SIZE],dsp_msg2[LIST_SIZE],ocnt1=-1,ocnt2=-1; 
		/* store displaied lists */

char map_names[32][32];

/**********************************************************************
the main control of the overlay 				     */

int over_control(msg)
int *msg;
{
int i,sw,spid;

if(msg[0]==19){   /* produst selection changed */
	/* msg[1]: 0 - deselect; 1 - select; 3 - change sub list */
	/* msg[2]: the string;  msg[7]: index in the list */

	if(msg[1]==3){	/* change lists but not graphics */
		sel_pid=pconf[msg[7]].pid;
		sel_lind=msg[7];
		sel_pind=-1;
		send_over_list1();
		return (0);
	}

	spid=pconf[msg[7]].pid;
	lock_shm(1);
	sw=0;
	for(i=0;i<omsg->np;i++){
	  if(prod[i].deleted!=0) continue;
	  if((int)prod[i].pid != spid) continue;
	  if(msg[1]==0) {  /* deselect */
		pconf[msg[7]].selected=0;
		prod[i].selected= prod[i].selected & 7;
	  }
	  else {  /* selected */
		pconf[msg[7]].selected=1;
		prod[i].selected= prod[i].selected | 8;
	  }
	  sw=1; /* need update the graphics */
	}
	omsg->new=omsg->new|sw;
	lock_shm(0); /* release the shm */

}

return (0);

}

/**********************************************************************
wait until shm is accessible and set busy                             */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int lock_shm(sw)
int sw;
{
static struct sembuf op_lock[2] = {
	0,0,0,   /* wait for sem#0 to become 0 */
	0,1,SEM_UNDO    /* increment sem */
};
static struct sembuf op_unlock[1] = {
	0,-1,(IPC_NOWAIT | SEM_UNDO)  /* decrement sem */
};
static int semid=-1;
extern int errno;

if(semid<0){
	if((semid=semget(SHM_KEY,1,IPC_CREAT | 0666))<0){
		printf("Error in semget.\n");
		return (1);
	}
}

if(sw==1){ /* lock */
again:
	if(semop(semid,&op_lock[0],2)<0){
		if(errno==4) goto again;
		printf("Error in lock shm (errno: %d).\n",errno);
		return (1);
	}
	return (0);
}

if(sw==0){ /* unlock */
	if(semop(semid,&op_unlock[0],1)<0){
		printf("shm. not locked\n");
		return (1);
	}
	return (0);
}
}

/**********************************************************************
check time out and modify the data base.
Called by the timer every 10 seconds				     */

int check_time_out()
{
int i,ac;

ac=0;
for(i=0;i<omsg->np;i++){
	if(prod[i].deleted>0 || prod[i].tindex<0) continue;
	if(prod[i].timer1>0) {
	    prod[i].timer1=prod[i].timer1-10;
	    if(prod[i].timer1<=0) {
		if(ac==0) lock_shm(1); /* lock shm */	
		if(prod[i].tindex==0) 
			deselect_an_index(i);
		else
			deselect_a_time(i);
		ac=1;
	    }
	}

	if(prod[i].timer2>0) {
	    prod[i].timer2=prod[i].timer2-10;
	    if(prod[i].timer2<=0){
		if(ac==0) lock_shm(1); /* lock shm */		  
		if((prod[i].selected&1)==0) delete_a_time(i);
		else delete_an_index(i);
		ac=1;
	    }
	}
}

if(ac==1) {
	omsg->new=1;
	lock_shm(0); /* unlock shm */
	send_over_list1();
	update_list0();
}

alarm_time[1]=10;
return (0);

}

/******************************************************************
select a index                                                   */

int select_an_index(i)
int i;
{
int k;
int ind,pid;

if(prod[i].deleted>0) return (0);

if((prod[i].selected&1) == 0) { /* time sequence */
	prod[i].selected=prod[i].selected | 6;
	    ind=prod[i].index;
	    pid=prod[i].pid;
	    for(k=i+1;k<omsg->np;k++){
	        if(prod[k].deleted>0) continue;
		if(prod[k].index==ind && prod[k].pid==pid)
		    prod[k].selected=prod[k].selected | 4;
	    }
}
else  prod[i].selected=prod[i].selected | 4;
return(0);

}

/******************************************************************
select a time                                                    */

int select_a_time(i)
int i;
{
int k;
int ind,pid;

if(prod[i].deleted>0) return (0);

prod[i].selected=prod[i].selected | 6;
if(prod[i].tindex != 0) { 
	    ind=prod[i].index;
	    pid=prod[i].pid;
	    for(k=i-1;k>=0;k--){
	        if(prod[k].deleted>0) continue;
		if(prod[k].index==ind && prod[k].pid==pid && 
					prod[k].tindex==0){
		    prod[k].selected=prod[k].selected | 4;
		    break;
		}
	    }
}
return(0);

}

/******************************************************************
deselect a index                                                   */

int deselect_an_index(i)
int i;
{
int k;
int ind,pid;

if(prod[i].deleted>0) return (0);

if((prod[i].selected&1) == 0) { /* time sequence */
	    ind=prod[i].index;
	    pid=prod[i].pid;
	    for(k=omsg->np-1;k>=0;k--){
	        if(prod[k].deleted>0) continue;
		if(prod[k].index==ind && prod[k].pid==pid){
		    prod[k].selected=prod[k].selected & 11;
		    if(prod[k].tindex==0) break;
		}
	    }
}
else  prod[i].selected=prod[i].selected & 11;
return(0);

}

/******************************************************************
deselect a time                                                  */

int deselect_a_time(i)
int i;
{

prod[i].selected=prod[i].selected & 13;
return(0);

}

/******************************************************************
delete a specified index                                         */

int delete_an_index(i)
int i;
{
int k,ind,pid;

if(prod[i].deleted!=0 || prod[i].tindex<0) return (0);

if(prod[i].tindex==0) {

	 if((prod[i].selected&1) == 0) { /* time sequence */
	    ind=prod[i].index;
	    pid=prod[i].pid;
	    for(k=i+1;k<omsg->np;k++){
	        if(prod[k].deleted>0) continue;
		if(prod[k].index==ind && prod[k].pid==pid){
		  prod[k].deleted=1;
		  prod[k].selected=0;
	        }
	    }
	}
	n_prod[prod[i].cf_ind]--;
	if(sel_pind==prod[i].index && prod[i].pid==sel_pid) sel_pind=-1;

	prod[i].deleted=1;
	prod[i].selected=0;
}
else printf("Error calling delete_an_index\n");

return (0);
}

/******************************************************************
delete an item in time sequence                                 */

int delete_a_time(i)
int i;
{
int k,ind,pid;
int new_tindex,tm;

if(prod[i].deleted!=0 || prod[i].tindex<0) return (0);

new_tindex=prod[i].tindex;
tm=new_tindex;

if((prod[i].selected&1) == 0) { /* time sequence */
	prod[i].deleted=1;
	prod[i].selected=0;
	ind=prod[i].index;
	pid=prod[i].pid;
	for(k=i+1;k<omsg->np;k++){
	        if(prod[k].deleted>0) continue;
		if(prod[k].index==ind && prod[k].pid==pid)
		  prod[k].tindex=new_tindex++;
	}
	if(new_tindex==tm && tm==0){ /* the whole sequence is gone */
	  n_prod[prod[i].cf_ind]--;
	  if(sel_pind==prod[i].index && prod[i].pid==sel_pid) sel_pind=-1;
	}
}
else printf("Error in calling delete_a_time\n");

return (0);
}

/*********************************************************************
select and cut the data on user's actions                           */

int modify_data(msg)
int *msg;
{
int index,i,item,k;
char *cpt;

i=msg[7]>>16;
item=msg[7]&0xffff;

cpt=(char *)&msg[2];
index=-1;
if(msg[0]==32) {/* action in ind_list */
	if(prod[i].tindex>=0){ /* we don't read maps */
	  if(sscanf(cpt,"%*s %d",&index)!=1){
		printf("Error found in list read back.\n");
		return(1);
	  }

	  if(msg[1]==3) { /* change tim_list only */
		if(index>=0){
			sel_pind=index;
			send_over_list2();
		}
		return (0);
	  }
	}
}


lock_shm(1);
/* the i is to be modified */
if(msg[0]==32){
  if(msg[1]==0) {   /* deselected */
	deselect_an_index(i);
	dsp_msg1[item]=dsp_msg1[item] & 0xffff;
  }
  if(msg[1]==1) {  /* selected */
	select_an_index(i);
	prod[i].timer1=pconf[sel_lind].etm_dis;
	dsp_msg1[item]=dsp_msg1[item] | 0x10000;
  }
}
if(msg[0]==33){
  if(msg[1]==0) {   /* deselected */
	deselect_a_time(i);
	dsp_msg2[item]=dsp_msg2[item] & 0xffff;
  }
  if(msg[1]==1) {  /* selected */
	select_a_time(i);
	prod[i].timer1=pconf[sel_lind].etm_dis;
	dsp_msg2[item]=dsp_msg2[item] | 0x10000;
  }
}
if(msg[1]==2 && prod[i].tindex>=0) { /* deleted, maps not deletable */
	if(msg[0]==32) delete_an_index(i);
	else delete_a_time(i);
}
omsg->new=1;
lock_shm(0); /* release the shm */

/* update the lists and the graphics */
send_over_list1();
update_list0();

return (0);

}

/**********************************************************************
send list1 to the interface					     */

int send_over_list1()
{
int i,j;
char *cpt;
int first_sel;
static int msg[8];
int h,m,s;
char str[32];
static int dsp_sel_pid=-1; /* for list label */
int cnt,tmp;
int time;
int selected;
int pass;

if(sel_lind>=n_list0 || sel_lind<0) return (1);

pass=0;

again:
pass++;
/* send a message to accumulate the messages */
msg[0]=34;
msg[1]=0;
set_list(msg);

first_sel=-1;

cpt=(char *)&msg[3];
msg[0]=30;

/* the label */
if(sel_pid!=dsp_sel_pid){
	msg[1]=3;
	sprintf(cpt,"  %s",pconf[sel_lind].fname);
	set_list(msg);
}

cnt=0;
for(i=0;i<omsg->np;i++){
	if(prod[i].deleted>0) continue;
	if(prod[i].pid!=sel_pid) continue;
	if(prod[i].tindex>0) continue;

	selected=(((int)prod[i].selected)>>2)&1;
	tmp=(selected<<16)+i;
	if(cnt>=LIST_SIZE) { /* too big a list */
		if(pass>=2) break; /* we may fail in remove,
			due to displaied and deleted guys */
		list_overflow(1);
		ocnt1=0; /* update all, since data in dsp_msg1 incorrect */
		goto again;
	}
	if(cnt>=ocnt1 || dsp_msg1[cnt]!=tmp){
	  dsp_msg1[cnt]=tmp;

	  /* the lists */
	  msg[1]=1;
	  if(selected==0) msg[1]=2;
	
	  if(pconf[sel_lind].tss==-1){ /* a map */
	  	sprintf((char *)&msg[3]," %s",(char *)prod[i].time);
	  }

	  else { /* a regular product */
	 	time=prod[i].time;
	  	s=time%60;
	  	time=time/60;
	  	m=time%60;
	  	time=time/60;
	  	h=time%24;
	  	get_time_string(h,m,s,str);
/*	  	str[5]='\0'; */
	  	sprintf((char *)&msg[3],"%s %d %s",
			pconf[sel_lind].sname,prod[i].index,str);
	  }

	  msg[2]=cnt;
	  msg[7]=i;
	  set_list(msg);
	}

	cnt++;

	if(first_sel<0 && selected==1) first_sel=prod[i].index;
}

/*if(sel_pind<0) sel_pind=first_sel;  give a chance for fixed sel_pind */

/* remove extra items */
if(n_list1>cnt){
	msg[0]=30;
	msg[1]=0;
	msg[2]=cnt;
	msg[3]=n_list1-1;
	set_list(msg);
}
ocnt1=cnt;
n_list1=cnt;

/* flush the messages */
msg[0]=34;
msg[1]=1;
set_list(msg);

/* update here, otherwise is may not be updated because resend
	msg: 34,0 */
dsp_sel_pid=sel_pid;

/* the list2 */
send_over_list2();

return (0);

}

/**********************************************************************
send list2 to the interface					     */

int send_over_list2()
{
int i;
char *cpt;
int msg[8];
int h,m,s;
char str[32];
static int dsp_sel_pind=-1; /* for list label */
static int dsp_sel_pid=-1; /* for list label */
int cnt,tmp;
int time;
int selected;

int pass;

cpt=(char *)&msg[3];
msg[0]=31;
pass=0;

if(sel_pind<0 || pconf[sel_lind].tss<=0) {  /* remove the list */
	msg[1]=0;
	msg[2]=0;
	msg[3]=n_list2;
	set_list(msg);

	n_list2=0;
	dsp_sel_pind=ocnt2=-1;

	/* remove the lable */
	msg[1]=3;
	cpt[0]='\0';
	set_list(msg);

	return (1);
}

again:
pass++;
/* send a message to accumulate the messages */
msg[0]=34;
msg[1]=0;
set_list(msg);

msg[0]=31;

/* the label */
if(sel_pind!=dsp_sel_pind || sel_pid!=dsp_sel_pid){
	msg[1]=3;
	sprintf(cpt,"  %s%d",pconf[sel_lind].sname,sel_pind);
	set_list(msg);
	dsp_sel_pind=sel_pind;
}

cnt=0;
for(i=0;i<omsg->np;i++){
	if(prod[i].deleted>0) continue;
	if(prod[i].pid!=sel_pid) continue;
	if(prod[i].index!=sel_pind) continue;

	selected=(((int)prod[i].selected)>>1)&1;
	tmp=(selected<<16)+i;
	if(cnt>=LIST_SIZE) { /* too big a list */
		if(pass>=2) break; /* see send_over_list1 */
		list_overflow(2);
		ocnt2=0; /* update all, since data in dsp_msg1 incorrect */
		goto again;
	}
	if(cnt>=ocnt2 || dsp_msg2[cnt]!=tmp){
	  dsp_msg2[cnt]=tmp;
send:
	  msg[1]=1;
	  if(selected==0) msg[1]=2;

	  if(pconf[sel_lind].tss==2){ /* list the id */
		sprintf(cpt,"  %d",(int)prod[i].id);
	  }
	  else{  /* list the time */
	  	time=prod[i].time;
	  	s=time%60;
	  	time=time/60;
	  	m=time%60;
	  	time=time/60;
	  	h=time%24;
	  	get_time_string(h,m,s,str);
	  	sprintf(cpt,"%s",str);
	  }

	  msg[2]=cnt;
	  msg[7]=i;
	  set_list(msg);
	}

	cnt++;
}

/* remove extra items */
if(n_list2>cnt){
	msg[0]=31;
	msg[1]=0;
	msg[2]=cnt;
	msg[3]=n_list2-1;
	set_list(msg);
}
ocnt2=cnt;
n_list2=cnt;

/* flush the messages */
msg[0]=34;
msg[1]=1;
set_list(msg);

return (0);

}

/********************************************************************
initialize. Called by initialization				  */

int initialize_over()
{
int i;

alarm_time[1]=10;   /* import time out */
alarm_time[2]=30;   /* call rearange data every 5 minutes */
alarm_time[3]=4;   /* poll the import */

sel_pid=pconf[sel_lind].pid;

for(i=0;i<LIST_SIZE;i++) {
	n_prod[i]=0;
}


return (0);
}

/******************************************************************
function creating the time string                                */

int get_time_string(h,m,s,str)
int h,m,s;
char *str;
{
char *cpt;

cpt=str;

/* the hour */
if(h<10) sprintf(cpt,"0%d:",h);
else sprintf(cpt,"%d:",h);
cpt=cpt+3;

/* the mimute */
if(m<10) sprintf(cpt,"0%d:",m);
else sprintf(cpt,"%d:",m);
cpt=cpt+3;

/* the second */
if(s<10) sprintf(cpt,"0%d",s);
else sprintf(cpt,"%d",s);

return (0);
}


/************************************************************************
reads in configuration                                                */

int read_conf()
{
FILE *fl;
char buf[128];
int cnt;
int id,t1,t2,t3,t4,t5,t6;
int ind;

if((fl=fopen("prds.conf","r"))==NULL){
	printf("Can not find prds.conf file.\n");
	return (1);
}

cnt=0;
while(get_line(buf,fl)>=0){
	if(buf[0]=='#') continue; /* a comment line */
	if(strncmp(buf,"Rows in lists",13)==0){
		sscanf(buf,"%*s %*s %*s %d",&rows_in_lists);
		continue;
	}
	if(strncmp(buf,"Font",4)==0){
		sscanf(buf,"%*s %s",font_name);
		continue;
	}
	if(strncmp(buf,"Loop available",4)==0){
		loop_available=1;
		continue;
	}
	if(strncmp(buf,"Background color",16)==0){
		sscanf(buf,"%*s %*s %s",background_color);
		continue;
	}
	if(strncmp(buf,"Foreground color",16)==0){
		sscanf(buf,"%*s %*s %s",foreground_color);
		continue;
	}
	if(strncmp(buf,"Window at",9)==0){
		sscanf(buf,"%*s %*s %d %d",&w_loc_x,&w_loc_y);
		continue;
	}
	if(strncmp(buf,"Initially closed",16)==0){
		initially_closed=1;
		continue;
	}
	if(strncmp(buf,"Volume time",11)==0){
		sscanf(buf,"%*s %*s %d",&v_time);
		continue;
	}
	if(sscanf(buf,"%d %s %s %d %d %d %d %d %d",&id,pconf[cnt].sname,
		pconf[cnt].fname,&t1,&t2,&t3,&t4,&t5,&t6)!=9){
		printf(
		 "Error found in file prds.conf. (line %d)\n",cnt);
		return (1);
	}
	pconf[cnt].pid=id;
	pconf[cnt].etm_dis=t1;
	pconf[cnt].etm_lif=t2;
	pconf[cnt].psta=0;  /* down */
	pconf[cnt].selected=t3;
	pconf[cnt].tss=t4;
	pconf[cnt].dflag=t5;
	pconf[cnt].time_sel=t6;
	cnt++;
	if(cnt>=LIST_SIZE) break;
}
n_list0=cnt;
fclose(fl);

return (0);
}

/**************************************************************************
function reads maps                                                  */

int get_maps()
{
int i,index;
int k,nmaps,np;
int *dt;
FILE *fl;
int x,y,npoint;

index=-1;
for(i=0;i<n_list0;i++){
	if(index>=0 && pconf[i].tss==-1) {
		printf("More than one map items found.\n");
		return (1);
	}
	if(pconf[i].tss==-1) index=i;
}
if(index<0) return (0); /* no maps need */

pconf[index].psta=1;

if(strcmp(pconf[index].sname,"L")==0){
	printf("Read map from the local file.\n");
	if((fl=fopen("prds_maps","r"))==NULL){
		printf("Can not open file prds_maps.\n");
		return (1);
	}
	lock_shm(1);
	if(fscanf(fl,"%d",&nmaps)!=1) goto err;
	if(nmaps>32) {
		printf("At most 32 maps can be processed\n");
		nmaps=32;
	}
	np=omsg->np;
	dt=data+d_pt;
	for(i=0;i<nmaps;i++){
	  	if(fscanf(fl,"%s %d",map_names[i],&npoint)!=2) goto err;
		map_names[i][14]='\0';
		prod[np].time=(int)map_names[i]; /* name location */
		prod[np].tindex=-1; /* we use -1 to indicating no time out
					and no remove */
		prod[np].lpt=d_pt;
		prod[np].pid=pconf[index].pid;
		strcpy(prod[np].color_name,"White");
		prod[np].line_type=1;
		prod[np].tlen=0;
		prod[np].index=0;
		prod[np].llen=npoint;
		prod[np].deleted=0;
		prod[np].selected=9;
		if((pconf[index].selected&(1<<i))!=0) prod[np].selected=15;
		prod[np].display=0;
		prod[np].active=0;
		for(k=0;k<npoint;k++){
		  if(fscanf(fl,"%d %d",&x,&y)!=2) goto err;
		  *dt++=(y<<16)+(x&0xffff);
		}
		np++;
	        d_pt=d_pt+npoint;
	}
	omsg->np=omsg->np+nmaps;
	n_prod[index]=nmaps;
}
else{   /* get map from the product server */
	printf("Getting maps from the data server.\n");
	printf("Not implemented yet.\n");
	return (1);
}

omsg->new=1;
lock_shm(0);
fclose(fl);

return (0);

err:
lock_shm(0);
printf("Error reading map file prds_maps.\n");
return (1);

}
  
/********************************************************************
function gets a non-empty line                                    */

int get_line(buf,fl)
char *buf;
FILE *fl;
{
char *pt;

while(1){
	if(fgets(buf,127,fl)==NULL) return (-1);
	pt=buf;
	while(pt[0]==' ' || pt[0]=='\t' || pt[0]=='\n') pt++;
	if(pt[0]!='\0') return (0);
}

} /* end of fcn */

/**********************************************************************
delete older pros due to list overflow                               */

list_overflow(sw)
int sw; /* sw=1: called from ind_list; =2: from tim_list */
{
int ind[20],i,cnt,j;
int nd;

/*printf("Here: list_overflow\n");*/

return (0);
}

/**********************************************************************
delete older pros due to index overflow                               */

int index_overflow(pid)
int pid; 
{
int i,cnt,j;
int nd;

nd=LIST_SIZE/4;
if(nd>20) nd=20;
cnt=0;

/*printf("Here: ind overflow %d\n",nd);*/

	/* delete */
	lock_shm(1);
	for(i=0;i<omsg->np;i++){
		if(prod[i].deleted>0 || prod[i].pid!=pid) continue;
		if(prod[i].tindex!=0) continue;
		delete_an_index(i);
		if(cnt++>nd) break;
	}

omsg->new=1;
lock_shm(0);

return (0);
}

/**********************************************************************
delete older pros due to time sequence overflow                      */

int time_overflow(pid,ind)
int pid,ind; 
{
int i,cnt,j;
int nd;

/*printf("Here: time_overflow\n");*/

nd=LIST_SIZE/4;
if(nd>20) nd=20;

	cnt=0;
	lock_shm(1);
	for(i=0;i<omsg->np;i++){
		if(prod[i].deleted>0 || prod[i].pid!=pid) continue;
		if(ind==prod[i].index) {
			delete_a_time(i);
			cnt++;
			if(cnt>=nd) break;
		}
	}

omsg->new=1;
lock_shm(0);

return (0);
}

/**********************************************************************
process shm overflow                                                 */

int buffer_overflow()
{
int i,n,cnt;
int rep;

rep=0;
again:

rearange_data();

/* check */
if(omsg->np <= N_PROD*9/10 && d_pt <= data_size*9/10) return (0);
else {
	rep++;
	if(rep>=2){
		printf("Buffer full (prds). Display does not response.\n");
		return (1);
	}
}

/* remove older prods */
n=N_PROD/9;
cnt=0;

for(i=0;i<omsg->np;i++){
        /* for maps, never deleted. Here we assume the number of maps <<n */
	if(prod[i].tindex<0) continue; 
	prod[i].selected=prod[i].selected&0x7;
	if(prod[i].tindex==0) delete_an_index(i);
	else delete_a_time(i);
	cnt++;
	if(cnt>=n) break;;

}
omsg->new=1;

goto again;

}

/**********************************************************************
my timer                                                         */

int my_usleep(us)
int us;
{
struct timeval timeout;

timeout.tv_sec=0;
timeout.tv_usec=us;

select (3,NULL,NULL,NULL,&timeout);

return (0);

}

/**********************************************************************
rearange the data base. called by buffer_overflow and timer          */

int rearange_data()
{
int i,in,*p,*pn;
int j;
int sw;
int i1,i2;

/*printf("Here: Rearange\n");*/
sw=0;
in=0;
pn=data;
lock_shm(1);

for(i=0;i<omsg->np;i++){
	if(prod[i].deleted>0 && prod[i].display==0) {
		/* Note: we dont remove displaied even deleted */
		sw=1;
		continue;
	}
	if(sw==0) { /* no need to copy */
		in++;
		pn=pn+prod[i].llen;
		pn=pn+prod[i].tlen;
		continue;
	}

	prod[in]=prod[i];
	prod[in].lpt=(int)(pn-data);
	prod[in].tpt=prod[in].lpt+prod[i].llen;
	in++;
	
	p=data+prod[i].lpt;
	for(j=0;j<prod[i].llen;j++) *pn++=*p++;
	
	p=data+prod[i].tpt;
	for(j=0;j<prod[i].tlen;j++) *pn++=*p++;
}
omsg->np=in;
d_pt=(int)(pn-data);
lock_shm(0);  

ocnt1=ocnt2=-1; /* the saved list is not good anymore */
send_over_list1(); /* update client data in the lists */

return (0);
}

/******************************************************************
function getting the current time                                */

int get_current_time()
{

struct timeval tp;
struct timezone tzp;

if(gettimeofday(&tp,&tzp)!=0) return (-1);
return (tp.tv_sec);
}

/**********************************************************************
read the input and put in the data base. Called by the timer	   */

int import_input_s()
{
static int cnt=0;
int i,selected,lind;
/* we assume the index is increasing
	This assumption is only for setting tindex */
int k;
int index,pid;
int x1,y1,x2,y2,*ipt;

static int lseed=232323;

if(create==0) return (0);

lseed=(lseed*123/19)%9876543;

/* simulate the lind */
lind=((lseed>>4)%2)+1;

index=cnt;

if(index>50 && index%2==0) index=50;
cnt++;

again:

if(d_pt+2+0>=data_size || omsg->np>=N_PROD) {
	if(buffer_overflow()!=0) return (0);
	goto again;
}

lock_shm(1);

i=omsg->np;
pid=pconf[lind].pid;
prod[i].pid=pid;
prod[i].index=index;
prod[i].cf_ind=lind;
prod[i].time=get_current_time();
prod[i].deleted=0;

prod[i].id=lind;


selected=6;
if(pconf[lind].tss<=0) selected=5;
if(pconf[lind].selected>0) selected+=8;
prod[i].selected=selected;

prod[i].timer1=pconf[lind].etm_dis;
prod[i].timer2=pconf[lind].etm_lif;

prod[i].period=pconf[lind].etm_dis;

set_tindex(i,lind);

/* the data */
prod[i].lpt=d_pt;
prod[i].llen=2;
prod[i].tlen=0;

random_lines(&x1,&y1,&x2,&y2);
ipt=data+d_pt;
*ipt++=(y1<<16)+(x1&0xffff);
*ipt++=(y2<<16)+(x2&0xffff);
d_pt=d_pt+2;

prod[i].tlen=0;

strcpy(prod[i].color_name,"White");
prod[i].line_type=1;

/* clear */
prod[i].active=prod[i].display=0;
	
omsg->np=i+1;

omsg->new=1;

lock_shm(0);

if(n_prod[prod[i].cf_ind]>LIST_SIZE) index_overflow(prod[i].pid);
if(prod[i].tindex>=LIST_SIZE) time_overflow(prod[i].pid,prod[i].index);

send_over_list1();
update_list0();

return (0);

}

/*****************************************************************
function set tindex and optionally delect the previous item     */

int set_tindex(i,lind)
int i,lind;
{
int k;
int pid,index;

pid=prod[i].pid;
index=prod[i].index;

/* set the tindex */
if(pconf[lind].tss<=0) prod[i].tindex=0;
else{ /* search for the sequence */
	for(k=omsg->np-1;k>=0;k--) {
		if(prod[k].deleted>0) continue;
		if(prod[k].pid==pid && prod[k].index==index) break;
	}
	if(k<0) prod[i].tindex=0; /* not found */
	else prod[i].tindex=prod[k].tindex+1;
}
	    
if(prod[i].tindex==0) n_prod[prod[i].cf_ind]++;

/* deselect the last */
if(pconf[lind].dflag==1){ 
	for(k=i-1;k>=0;k--){
		if(prod[k].deleted>0) continue;
		if(prod[k].pid==pid) break;
	}
	if(k<0 || prod[k].index==index) return (0);
	deselect_an_index(k);

	/* if new item in a sequence that has been deselected */
	if(prod[i].tindex>0)
			prod[i].selected=prod[i].selected & 11;
}

return (0);
}

/****************************************************************
random lines                                                   */

int random_lines(x1,y1,x2,y2)
int *x1,*x2,*y1,*y2;
{
static int rand=123435;

rand=(rand*123/19)%9876543;
*x1=rand%20000;
*x1=*x1-10000;
rand=(rand*123/19)%9876543;
*x2=rand%20000;
*x2=*x2-10000;
rand=(rand*123/19)%9876543;
*y1=rand%20000;
*y1=*y1-10000;
rand=(rand*123/19)%9876543;
*y2=rand%20000;
*y2=*y2-10000;
/*
printf("%d %d %d %d\n",*x1,*y1,*x2,*y2);*/

return (0);
}

/**********************************************************************
read the input and put in the data base. Called by the timer	   */

#include "pdb.h"
#include "sock.h"

int import_input()
{
int cnt;
int i,selected,lind;
/* we assume the index is increasing
	This assumption is only for setting tindex */
int k,j;
int index,pid;
int x1,y1,x2,y2,*ipt;
static int pcnt=0;

PDBproduct *ppt;
PDBpt *ptpt;
char *message;
int msg_len,type;

if(product_server_on==0) return (0);

cnt=0;
for(k=0;k<20;k++) { /* we accept at more 20 products in one second */

  if((i=PDBresponse(0,&type,&message,&msg_len,0))<0) {
	  printf("Error found reading PDB.\n");
	  break;
  }

  if(i<=0) break;

  if(type==PDB_STATUS) { /* update product status */
	int idt,stt,idind;
	sscanf(message,"%d %d",&idt,&stt);
	idind=-1;
	for(i=0;i<n_list0;i++){
		if(pconf[i].tss==-1) continue; /* map */
		if(pconf[i].pid==idt) {
			idind=i;
			break;
		}
	}
	if(idind<0) continue;
	pconf[i].psta=stt;
	cnt++;
	continue;
  }

  if(type!=PDB_PRODUCT) continue;

	ppt=(PDBproduct *)message;

/* find the line index */
lind=-1;
for(i=0;i<n_list0;i++){
	if(pconf[i].pid==ppt->type) {
		lind=i;
		break;
	}
}
if(lind<0 || ppt->type==12) {
	if(pcnt<50) printf("Type not found: %d\n",ppt->type);
	pcnt++;
	/*return (1);*/
	continue;
}

again:

if(d_pt+ppt->npts+0>=data_size || omsg->np>=N_PROD) {
	if(buffer_overflow()!=0) return (0);
	goto again;
}

lock_shm(1);

i=omsg->np;

pid=ppt->type;
prod[i].pid=pid;
prod[i].index=ppt->id;
prod[i].cf_ind=lind;
prod[i].time=ppt->generate_time;
if(pconf[lind].time_sel==1) prod[i].time=ppt->start_time;
if(pconf[lind].time_sel==2) prod[i].time=ppt->expire_time;
prod[i].deleted=0;

prod[i].id=ppt->subid; /* this needs true number */

selected=6;
if(pconf[lind].tss<=0) selected=5;
if(pconf[lind].selected>0) selected+=8;
prod[i].selected=selected;

prod[i].timer1=pconf[lind].etm_dis;
prod[i].timer2=pconf[lind].etm_lif;

prod[i].period=pconf[lind].etm_dis;

set_tindex(i,lind);

/* the data */
prod[i].lpt=d_pt;
prod[i].llen=ppt->npts;
prod[i].tlen=0;

ipt=data+d_pt;

ptpt=ppt->pts;
for(j=0;j<prod[i].llen;j++){
	*ipt++=(ptpt->y<<16)+(ptpt->x&0xffff);
	ptpt++;
}

d_pt=d_pt+prod[i].llen;

strcpy(prod[i].color_name,ppt->color);
prod[i].line_type=ppt->line_type;

/* clear */
prod[i].active=prod[i].display=0;
	
omsg->np=i+1;

omsg->new=1;
lock_shm(0);

if(n_prod[prod[i].cf_ind]>LIST_SIZE) index_overflow(prod[i].pid);
if(prod[i].tindex>=LIST_SIZE) time_overflow(prod[i].pid,prod[i].index);

cnt++;

}  /* end of for k */

if(cnt>0){
  send_over_list1();
  update_list0();
}

/*printf("%d\n",omsg->np);*/

return (0);

}

/*****************************************************************
initialize product server connection                            */

int init_product_server()
{

if(SOCKinit("prds",getpid(),3,-1)<0) {
	printf("Failed in SOCKinit.\n");
	return(1);
}
if(SOCKsetServiceFile(SOCK_ENV,"ALG_SERVICES")<0){
	printf("Failed in SOCKsetServiceFile.\n");
	return(1);
}

if(PDBinitProductServer("prds")==0){
	  printf("Failed in connecting to PDB.\n");
	  return(1);
}

if(SOCKbeginOperations(TRUE)<0){ /* this call may fail if another
				user used the machine */
	printf("Failed in SOCKbeginOperations.\n");
	return(1);
}

PDBrequest(PDB_CONNECT,"prds",5);
PDBrequest(PDB_REQUEST_PRODUCT,"ALL",4);
PDBrequest(PDB_START,"",1);

return (0);

}
