/******************************************************************
	file: over.c
	routines for using prds in rdi
******************************************************************/

#ifndef TEST
#define EXT extern 
#endif

/* for test only */
#ifdef EXT
#include "rdisplay.h"
static int xradar=30000, yradar=30000;
#else
static int xradar=30000, yradar=30000;
static int over_loop_pause=0;
#endif

#include "prds_user.h"

#define GB_SIZE 1536
#define TB_SIZE 256

int shmid;
char *shmpt;

Product *prod;
Out_msg *omsg;
In_msg *imsg;
int *data; /* address for data field (points and texts) */


int volume_interval;
static int first_time=0x7fffffff,last_time=0; 

static int import_on=0; /* status of the connection to prds */
static int loop_mode=0;

extern int errno;
char *PDBnextText();

static char color_ind[128]={
	2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,
	2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,
	2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,
	2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,

	2, 2, 2, 5,   2, 2, 2, 2,   2, 2, 2, 2,   3, 6, 2, 2,
	2, 2, 1, 2,   2, 2, 2, 7,   2, 4, 2, 2,   2, 2, 2, 2,
	2, 2, 2, 5,   2, 2, 2, 2,   2, 2, 2, 2,   3, 6, 2, 2,
	2, 2, 1, 2,   2, 2, 2, 7,   2, 4, 2, 2,   2, 2, 2, 2  };


/**********************************************************************
test called by the main loop                                         */

int poll_prds()
{
static int last=-1;

if(omsg->key==VALID) import_on=1;
else import_on=0;

if(last!=import_on) {
	/* we must clear the graph left, after the prds gone */
	if(import_on==0) {
		clear_graphics();
		printf("Prds is off\n");
	}
	else {
		start_graphics();
		printf("Prds is on\n");
	}
}
last=import_on;

if(import_on==0) return (1);

if(omsg->loop!=0 && loop_mode==0){ /* turn on loop */
	over_all(0);
	loop_mode=1;
	volume_interval=omsg->loop;
	set_prod_loop(); /* set first_time, last_time and volume interval */
}
if(omsg->loop==0 && loop_mode==1){ /* turn off loop */
	loop_mode=0;
	over_all(1);
}

if(loop_mode==1){
	over_movie();
	if(omsg->new!=0) map_all(2);
	return(0);
}

/* update */
if(omsg->new==0) return (0); 
over_all(2);
map_all(2);

return (0);

}

/******************************************************************
prepare for product movie                                        */

int set_prod_loop()
{
int i,j,n,it,ind,max;
int cnt[32];

/* search for first_time and last_time */
lock_shm(1);

for(i=0;i<omsg->np;i++){
	if(prod[i].tindex<0 || prod[i].deleted!=0) continue; 
						/* skip the map */
	first_time=last_time=prod[i].time;
	break;
}
for(j=i;j<omsg->np;j++){
	if(prod[j].tindex<0 || prod[j].deleted!=0) continue; 
	if(prod[j].time<first_time) first_time=prod[j].time;
	if(prod[j].time>last_time) last_time=prod[j].time;
}
lock_shm(0);

/* find volume_interval */
#ifdef EXT
/*n=v_cnt;
if(n>nvolume) n=nvolume;
for(i=0;i<32;i++) cnt[i]=0;
for(i=1;i<n;i++){
	it=volume_time[i]-volume_time[i-1];
	if(it<0) it=0;
	it=it/60+1;
	if(it>31) it=31;
	cnt[it]++;
}
ind=0;
max=cnt[0];
for(i=1;i<32;i++){
	if(cnt[i]>max){
		max=cnt[i];
		ind=i;
	}
}
if(ind<2) ind=2; 
volume_interval=ind*60;
volume_interval=600;*/
#else
/*volume_interval=10;*/
#endif

return (0);

}

/******************************************************************
clear whole overlay planes                                       */

#ifdef EXT
int clear_graphics()
{

draw_graph(NULL);

return (0);

}
#endif

/******************************************************************
initialize the prod[i].display                                   */

int start_graphics()
{
int i;

lock_shm(1);
for(i=0;i<omsg->np;i++){
	prod[i].display=prod[i].active=0;
}
omsg->new=1;

lock_shm(0);
over_all(1);
map_all(1);
return (0);

}

/******************************************************************
turn on/off the prds window. Called by overlay button            */

int switch_the_window()
{
static int w_on=0; /* initially closed */

if(omsg->key!=VALID) {
#ifdef EXT
	send_fatal_msg("prds not running");
#else
	printf("Error: prds is not running.\n");
#endif
	return (1);
}
if(w_on==1) {
	w_on=0;
	strcpy(imsg->msg,"Close import win");
	kill(omsg->pid,30);
}
else {
	w_on=1;
	strcpy(imsg->msg,"Show import win");
	kill(omsg->pid,30);
}
return (0);
}


/*********************************************************************
initialize, called by main                                          */

int init_import()
{
char *cpt;
FILE *fl;
float x0,y0;

/* try to get modified radar location */
if(((fl=fopen("prds_origin","r"))!=NULL) &&
    (fscanf(fl,"%f %f",&x0,&y0)==2)){
	xradar = (int)(.5+(float)xradar-(x0*.1));
	yradar = (int)(.5+(float)yradar+(y0*.1));
	printf("Use new prds origin (%f, %f).\n",x0,y0);
}
fclose(fl);

/* initialize the shared memory */
/* open shared memory segment */
if((shmid=shmget((key_t)SHM_KEY,SHM_SIZE,0666|IPC_CREAT))<0){
	printf("Error shmget - prds.\n");
	return(1);
}
shmpt=(char *)shmat(shmid,(char *)0,0);
if((int)shmpt==-1){
	printf("Error shmat - prds.\n");
	return(1);
}
imsg=(In_msg *)shmpt;
cpt=shmpt+sizeof(In_msg);
omsg=(Out_msg *)(cpt);
cpt=cpt+sizeof(Out_msg);
prod=(Product *)cpt;
cpt=cpt+N_PROD*sizeof(Product);
data=(int *)cpt;

/*lock_shm(0);  initialize semaphore */

return (0);

}


/**********************************************************************
refresh the graphics overlay					     */

int refresh_overlay(sw)
int sw;
    /* sw=0: erase; sw=1: draw;
       based on prod[].active;
       reset active after erasing or drawing		     */

    /* important: the color is put in the high two bytes of the
	number of points or # of characters - we must modify grawing
	program in the lower level, text drawing too		 */

{
int color,mask;
int buf[GB_SIZE];
int i,point,t;
int kk,cnt;
int pp;
int tp;
char *cpt,*cpt1;
int *ipt;
int count;
int x,y,x1,y1;
int clen;

/* the color and mask */
mask=0xe0e0e0;
buf[3]=mask;
buf[2]=0xffffffff; /* this means individual color and line
		type is assigned to each line	  */
/*color=(color<<16)+(color<<8)+color; this is processed in low level */

/* first we draw the text */
cnt=0;
kk=4;
pp=64;
for(i=0;i<=omsg->np;i++) {

	if(i<omsg->np && prod[i].active==0) continue;

	if(pp+prod[i].tlen>TB_SIZE || i==omsg->np || cnt>=30){
				 /* draw text */
		buf[0]=pp;
		buf[1]=cnt;
		if(cnt>0) draw_text(buf); 
		if(i==omsg->np) break;
		cnt=0;
		kk=4;
		pp=64;
	}

	/* get the text */
    if(prod[i].tlen>0){
	char *str,*txt,*etxt;
	int xx,yy;

	tp=prod[i].tpt;
	txt=(char *)&data[tp];
	etxt=txt+4*prod[i].tlen;
	while(1){
	  if((str=PDBnextText(&txt,&yy,&xx))==NULL) break;
	  if(txt>=etxt) break;
	  xx=xx+xradar;
	  yy=yradar-yy;
	  world_to_window(xx,yy,&x1,&y1); /* commented for test */
	  x1=x1-6;
	  y1=y1-10; /* use center location */
	  x=(y1<<16)+x1;

	  buf[pp]=(x>>1)&0x7fff7fff;
	  cpt1=(char *)&buf[pp+1];
	  strcpy(cpt1,str);

	  color=0;
	  if(sw==1) color=color_ind[*prod[i].color_name]<<5;
	  buf[kk]=pp;
	  buf[kk+1]=strlen(str)+(color<<16);

	  cnt++;
	  pp=pp+2+(buf[kk+1]&0xffff)/4;
	  kk=kk+2;
	}
    } 
}

/* we draw the lines */
cnt=0;
kk=4;
pp=264;
clen=0;
for(i=0;i<=omsg->np;i++) {

	if(i<omsg->np){
	    if(prod[i].active==0) continue;
	}

	if(pp+prod[i].llen>GB_SIZE || 
	   i==omsg->np || cnt>=130){ /* draw graph */
		buf[0]=pp;
		buf[1]=cnt;
		if(cnt>0) draw_graph(buf);
		if(i==omsg->np) break;
		cnt=0;
		kk=4;
		pp=264;
		clen=0;
	}

	/* get the data */
    if(prod[i].llen>0){
	tp=prod[i].lpt;
	ipt=&data[tp];
	buf[kk]=pp;
	clen=0;
	for(t=0;t<prod[i].llen;t++){
		x1=ipt[t];
		x=((x1<<16)>>16);
		if(x==0x7fff) x=0xffff;
		else x=x+xradar;
		y=yradar-(x1>>16);
	        buf[pp++]=(y<<16)+x;
		clen++;
		if(pp>=GB_SIZE){
		    int tmp;

		    tmp=buf[pp-1];
		    buf[0]=pp;
		    buf[1]=cnt+1;

		    color=0;
		    if(sw==1) color=color_ind[*prod[i].color_name]<<5;
		    buf[kk+1]=clen+(color<<16)+
				(prod[i].line_type<<24);
		    draw_graph(buf);
		    cnt=0;
		    pp=264;
		    kk=4;
		    buf[kk]=pp;
		    clen=0;

		    buf[pp++]=tmp;
		    clen++;
		}
	}

	color=0;
	if(sw==1) color=color_ind[*prod[i].color_name]<<5;
	buf[kk+1]=clen+(color<<16)+
				(prod[i].line_type<<24);
	cnt++;
	kk=kk+2;
    }
    prod[i].active=0;
    prod[i].display=sw;
}

return (0);

}


/**********************************************************************
refresh the map overlay					     */

int refresh_maps(sw)
int sw;
    /* sw=0: erase; sw=1: draw;
       based on prod[].active;
       reset active after erasing or drawing		     */

{
int color,mask;
int buf[GB_SIZE];
int i,point,t;
int kk,cnt;
int pp;
int tp;
char *cpt,*cpt1;
int *ipt;
int count;
int x,y,x1,y1;

/* process the maps */

/* the color and mask */
mask=0x80000000;
buf[3]=mask;
buf[2]=0; 
if(sw==1) buf[2]=mask;

cnt=0;
kk=4;
pp=264;
for(i=0;i<=omsg->np;i++) {

	if(i<omsg->np && prod[i].active==0) continue;

	if(pp+prod[i].llen>GB_SIZE || 
	   i==omsg->np || cnt>=130){ /* draw graph */
		buf[0]=pp;
		buf[1]=cnt;
		if(cnt>0) draw_graph(buf);
		if(i==omsg->np) return (0);
		cnt=0;
		kk=4;
		pp=264;
	}

	/* we may process big data as we do in refresh_overlay instead
	   of the following check */
	if(pp+prod[i].llen>GB_SIZE){
	    printf("Map size is too big.\n");
	    continue;
	} 

	/* get the data */
    if(prod[i].llen>0){
	tp=prod[i].lpt;
	ipt=&data[tp];
	buf[kk]=pp;
	for(t=0;t<prod[i].llen;t++){
		x1=ipt[t];
		x=((x1<<16)>>16);
		if(x==0x7fff) x=0xffff;
		else x=x+xradar;
		y=yradar-(x1>>16);
	        buf[pp++]=(y<<16)+x;
	}
	buf[kk+1]=prod[i].llen;
	cnt++;
	kk=kk+2;
    }
    prod[i].active=0;
    prod[i].display=sw;
}

return (0);

}


/*********************************************************************
decode the text data                                               */

char *PDBnextText( text_ptr, ycoord, xcoord)
   char **text_ptr;
   int	*ycoord, *xcoord;
{
      char *cpt,*cpte;
      int x,y,len;
      static char buf[64];

      cpt=*text_ptr;
      if((len=strlen(*text_ptr))>64) return (NULL);
      cpte=cpt+len;
      if(sscanf(cpt,"%d %d %s",&y,&x,buf)!=3) return (NULL);
      len=strlen(buf);
      while(cpt<cpte){
	if(strncmp(buf,cpt,len)!=0) cpt++;
	else break;
      }
      
      strcpy(buf,cpt);
      *text_ptr=cpt+strlen(buf)+1;
      *xcoord=x;
      *ycoord=y;

      return(buf);
}

/**********************************************************************
display all products     					     */

over_all(sw)
int sw; /* =0: erase all drawn; =1: draw all selected =2: update selected
	    */
{
int i,cnt;
int ret;

if(import_on==0 || (sw!=0 && loop_mode==1)) return (0);

ret=0;
lock_shm(1);

if(sw==0){
	cnt=0;
	for(i=0;i<omsg->np;i++){
	  if(prod[i].tindex<0) continue; /* maps, not processed */
	  if(prod[i].display!=0){
		prod[i].active=1;
		cnt++;
	  }
	}
	if(cnt>0) refresh_overlay(0);
	goto rett;
}

if(sw==2){
	cnt=0;
	for(i=0;i<omsg->np;i++){
	  if(prod[i].tindex<0) continue; /* maps, not processed */
	  if(prod[i].selected>12 && prod[i].deleted==0) continue;
	  if(prod[i].display!=0){
		prod[i].active=1;
		cnt++;
	  }
	}
	if(cnt>0) refresh_overlay(0);
	if(cnt!=0) goto next;

	for(i=0;i<omsg->np;i++){ /* draw after no erase */
	  if(prod[i].deleted>0) continue;
	  if(prod[i].tindex<0) continue; /* maps, not processed */
	  if(prod[i].display==0 && prod[i].selected>12){
		prod[i].active=1;
		cnt++;
	  }
	}
	if(cnt>0) refresh_overlay(1);
	goto rett;
}

next:
cnt=0;
for(i=0;i<omsg->np;i++){	/* draw all selected  */
	if(prod[i].deleted>0) continue;
	if(prod[i].tindex<0) continue; /* maps, not processed */
	if(prod[i].selected<=12) continue;
	prod[i].active=1;
	cnt++;
}
if(cnt>0) refresh_overlay(1);


rett:
omsg->new=0;
lock_shm(0);

return (0);

}

/**********************************************************************
display all maps 						     */

map_all(sw)
int sw; /* =0: erase all drawn; =1: draw all selected =2: update selected
	    */
{
int i,cnt;
int ret;

if(import_on==0) return (0);

ret=0;
lock_shm(1);

if(sw==0){
	cnt=0;
	for(i=0;i<omsg->np;i++){
	  if(prod[i].tindex>=0) continue; /* process maps only */
	  if(prod[i].display!=0){
		prod[i].active=1;
		cnt++;
	  }
	}
	if(cnt>0) refresh_maps(0);
	goto rett;
}

if(sw==2){
	cnt=0;
	for(i=0;i<omsg->np;i++){
	  if(prod[i].tindex>=0) continue; /* process maps only */
	  if(prod[i].selected>12 && prod[i].deleted==0) continue;
	  if(prod[i].display!=0){
		prod[i].active=1;
		cnt++;
	  }
	}
	if(cnt>0) refresh_maps(0);
	if(cnt!=0) goto next;

	for(i=0;i<omsg->np;i++){ /* draw after no erase */
	  if(prod[i].tindex>=0) continue; /* process maps only */
	  if(prod[i].deleted>0) continue;
	  if(prod[i].display==0 && prod[i].selected>12){
		prod[i].active=1;
		cnt++;
	  }
	}
	if(cnt>0) refresh_maps(1);
	goto rett;
}

next:
cnt=0;
for(i=0;i<omsg->np;i++){	/* draw all selected  */
	if(prod[i].deleted>0) continue;
	if(prod[i].tindex>=0) continue; /* process maps only */
	if(prod[i].selected<=12) continue;
	prod[i].active=1;
	cnt++;
}
if(cnt>0) refresh_maps(1);


rett:
omsg->new=0;
lock_shm(0);

return (0);

}


/**********************************************************************
control the overlay movie. 					  */

int over_movie()
{
int time1,time2;
static int old[1024],old_cnt=0;
static int old_time=0;
int i;
static int loop_skip=0;
static int loop_time=0;
int cnt;
int global_time;

/* we need global variables: loop_mode (0: all; 1: loop )
			     global_time       (Time of volume)	*/

if(loop_mode==0 || import_on==0 || over_loop_pause==1) return (0);

#ifdef EXT
if(ctrl.movie!=0 && movie_type==0) { /* to be syncronized with movie */
	global_time=current_time;
#else
if(first_time==0){ /* this is impossible case */
#endif

	if(global_time==old_time) return (0);
	old_time=global_time;

	lock_shm(1);
	/* clear the old */
	for(i=0;i<old_cnt;i++) prod[old[i]].active=1;
	refresh_overlay(0);

	/* draw the new */
	time1=global_time-volume_interval;
	time2=global_time+volume_interval;
	old_cnt=0;
	for(i=0;i<omsg->np;i++){
		if(prod[i].deleted!=0) continue;
		if(prod[i].time<time1 || prod[i].time>time2) continue;
		if(prod[i].selected<=12) continue;
		prod[i].active=1;
		old[old_cnt++]=i;
	}
/*printf("Here: %d %d %d %d %d %d\n",global_time,time1,time2,old_cnt,prod[1].time,prod[omsg->np-2].time);*/
	refresh_overlay(1);
	lock_shm(0);
	return (0);
}
else {	 /* auto movie */

	if(loop_time<first_time) loop_time=first_time;

	loop_skip--;
	if(loop_skip>0) return (0);

	/* clear the old */
	lock_shm(1);
	for(i=0;i<old_cnt;i++) prod[old[i]].active=1;
	refresh_overlay(0);

	/* redraw */
	cnt=0;
again:
	loop_time=loop_time+volume_interval;
	if(loop_time>last_time) {
		loop_time=first_time;
		cnt++;
		if(cnt>1) goto ret;  /* empty data */
	}
	time1=loop_time;
	time2=time1+volume_interval;
	old_cnt=0;
	for(i=0;i<omsg->np;i++){
		if(prod[i].deleted!=0) continue;
		if(prod[i].selected<=12) continue;
		if(prod[i].time>last_time) last_time=prod[i].time;
		if(prod[i].time<time1 || prod[i].time>time2) continue;
		prod[i].active=1;
		old[old_cnt++]=i;
	}
	/*if(old_cnt<=0) goto again;  skip empty time intervel */
	if(old_cnt>0) refresh_overlay(1);

#ifdef EXT
	loop_skip=200; /* define the loop speed */
#else
	loop_skip=2; /* define the loop speed */
#endif
}

ret:
lock_shm(0);
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

















#define DIC_XT 0x7fff 

/* the following is for testing */
#ifdef EXT 
#else

/******************************************************************
draw                                                          */

int draw_mlines(data)
int *data;
{
int i,*pt,k,t;
int color;

k=4;

for(i=0;i<data[1];i++){
	if(data[3]==0x80000000){
		color=0;
		if(data[2]!=0) color=1;
	}
	else{
		color=0;
		if((data[k+1]&0xff0000)!=0) color=1;
	}
	t=data[k];
	pt=&data[t];
	draw_lines(data[k+1]&0xffff,color,pt);
	k=k+2;
}	

return(0);
}


/****************************************************************/



int draw_lines(n,col,data)
int n,col,*data;
{
int x1,x2,y1,y2,tt,xx2,yy2,mask;
int dx,dy,i,t,cnt;

mask=0xffff;

cnt=0;

for(i=0;i<n;i++){

	xx2=data[i]&mask;
	yy2=(data[i]>>16)&mask;

	if(xx2==0xffff){
		cnt=0;
		continue;
	}
	if(cnt==0){
		x1=xx2;
		y1=yy2;
		x1=x1/100;
		y1=y1/100;
		cnt++;
	 	continue;
	}

	x2=xx2;
	y2=yy2;
	x2=x2/100;
	y2=y2/100;

/*printf("Draw line: %d %d %d %d\n",x1,y1,x2,y2);
	XSetForeground(dpy,gc,col); */

	if(col==0) {
		XDrawLine(dpy,gxid,gc,x1,y1,x2,y2); 
	}
	else {
		XDrawLine(dpy,gxid,gc1,x1,y1,x2,y2); 
	}

	x1=x2;
	y1=y2;
}
	
}  /* end of the function */


/**********************************************************************
function send graph_data to taac and let it to draw                  */

int draw_graph(graph_data)
int *graph_data;
/* the data is in graph_data */
{
int i,k;

if(graph_data[0]>GB_SIZE){
	printf("Too big graphics data\n");
	return (1);
}
/* further check */

k=4;
for(i=0;i<graph_data[1];i++){
	if(graph_data[k]+(graph_data[k+1]&0xffff)>GB_SIZE){
		printf("Error of graphics data:\n");
		for(i=0;i<k+2;i++) printf("%d\n",graph_data[i]);
		return (1);
	}
	k=k+2;
}

draw_mlines(graph_data);

return(0);

}

/******************************************************************
graw text                                                        */

int draw_text(data)
int *data;
{

return (0);
}

/******************************************************************
								*/
int world_to_window(xx,yy,x1,y1)
int xx,yy,*x1,*y1;
{

return (0);
}

#endif
