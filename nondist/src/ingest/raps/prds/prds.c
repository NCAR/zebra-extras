/*********************************************************************
	file: prds.c
	A implementation of the product selector
*********************************************************************/
#define EXT 
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <gdd.h>
#include <sys/ipc.h>
#include <xview/cursor.h>
#include <xview/cms.h>

#include "prds_ui.h"

#include "prds.h"

#include <X11/Xatom.h>

prds_basewindow_objects	*prds_basewindow;
prds_popup1_objects	 *prds_popup1;

static Xv_cursor cursor0,cursor1;

/*
 * Instance XV_KEY_DATA key.  An instance is a set of related
 * user interface objects.  A pointer to an object's instance
 * is stored under this key in every object.  This must be a
 * global variable.
 */
Attr_attribute	INSTANCE;

static int rm_shm=0;
static int test_mode=0;

void
main(argc, argv)
	int		argc;
	char		**argv;
{
char *av[32];
int i,ac;

while (argc>1) {
    argc--; argv++;
    if (*argv[0]=='-'){
	char *s;
	s = argv[0]+1; 
	switch(*s){
	    case 'r':
		rm_shm=1;
		break;
	    case 't':
		test_mode=1;
		break;
	    default:
		break;
	 }
    }
}

	/* initialize global vars */
	if(initialize_vars()!=0) exit(1);

	/* reads configuration */
	if(read_conf()!=0) exit(1);

	/* initialize cons in import pack */
	initialize_over();

	/* connect to the product server */
	if(init_product_server()!=0)
		printf("Failed in connecting to the product server.\n");
	else product_server_on=1; 

	/* get maps */
	if(get_maps()!=0) exit(1);

	/* the command line vars */
	for(i=0;i<argc;i++) av[i]=argv[i];
	ac=argc;
	/* the font */
	if(font_name[0]!='\0'){ 
	    av[ac++]="-Wt";
	    av[ac++]=font_name;
	}
	
	/*
	 * Initialize XView.
	 */
	xv_init(XV_INIT_ARGC_PTR_ARGV, &ac, av, 0);
	INSTANCE = xv_unique_key();
	
	/*
	 * Initialize user interface components.
	 */
	prds_basewindow = prds_basewindow_objects_initialize(NULL, NULL);
	prds_popup1 = prds_popup1_objects_initialize(NULL, prds_basewindow->basewindow);

	/* set_additional properties */
	if(set_additional_properties()!=0) exit(1);
	
	/*
	 * Turn control over to XView.
	 */
	xv_main_loop(prds_basewindow->basewindow);
	
	exit_process();
}

/***********************************************************************
process before exit                                                   */

Notify_func exit_process()
{

/* destroy the shared mem flag */
printf("exit: %d %d\n",omsg->key,omsg->pid);
omsg->key=0;
/* detach the shm */
shmdt(shmpt);
if(rm_shm==1){
  if(shmctl(shmid,IPC_RMID)<0) printf("Failed in shmctl(RM).\n");
}
lock_shm(0); /* release the semaphore */
exit(0);

}

/************************************************************************
set additional properties                                               */

int set_additional_properties()
{
extern Notify_func exit_process();
extern Notify_func msg_from_display();
char *cpt;
Xv_singlecolor fg;
fg.red=fg.green=fg.blue=255;

xv_set(prds_basewindow->basewindow, XV_X, 500, XV_Y, 400, 
		FRAME_SHOW_HEADER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, FALSE,
		NULL);

xv_set(prds_popup1->popup1, 
		XV_X, w_loc_x, XV_Y, w_loc_y,
		FRAME_SHOW_HEADER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, FALSE, 
		NULL);

if(initially_closed==0)
	xv_set(prds_popup1->popup1, XV_SHOW, TRUE, NULL);

cursor0=(Xv_cursor)xv_get(prds_basewindow->basewindow,
		WIN_CURSOR);
cursor1=xv_create(NULL,CURSOR,CURSOR_SRC_CHAR,
		OLC_PANNING_PTR, 
		CURSOR_FOREGROUND_COLOR, &fg,
		NULL);

/* font */
font=(Xv_font)xv_get(prds_popup1->popup1,XV_FONT);
space_width=(int)xv_get(font,FONT_CHAR_WIDTH, '.');

/* size of the window */
{
int inc,size,y;
int w0,w1,w2;
int cw,ch;
int w,h,wset1;

cw=(int)xv_get(font,FONT_DEFAULT_CHAR_WIDTH);
ch=(int)xv_get(font,FONT_DEFAULT_CHAR_HEIGHT);
w0=20*cw+40;
w1=18*cw+30;
w2=10*cw+30;
wset1=16*cw;
if(w2<wset1) w2=wset1;

w=w0+w1+w2+10;
ch=ch+4;
if(ch<18) ch=18;

h=(rows_in_lists+2)*ch+16;

s_line0=30+15*cw;
s_line1=w0+20+6*cw;
w_list0=16*cw;

xv_set(prds_basewindow->basewindow,XV_WIDTH,cw*50,NULL);

xv_set(prds_popup1->popup1,XV_HEIGHT,h,XV_WIDTH,w,NULL);
xv_set(prds_popup1->cut,XV_Y,h-ch-ch/2,XV_X,w0+w1+10,NULL);
xv_set(prds_popup1->loop,XV_Y,h-ch-ch/2,XV_X,w0+w1+10+8*cw,NULL);
xv_set(prds_popup1->ind_list,XV_X,w0+2,NULL);
xv_set(prds_popup1->tim_list,XV_X,w0+w1+2,NULL);
xv_set(prds_popup1->prd_list,
	PANEL_LIST_DISPLAY_ROWS,rows_in_lists,
	PANEL_LIST_WIDTH, w0-20,
	NULL);
xv_set(prds_popup1->ind_list,
	PANEL_LIST_DISPLAY_ROWS,rows_in_lists,
	PANEL_LIST_WIDTH, w1-20,
	NULL);
xv_set(prds_popup1->tim_list,
	PANEL_LIST_DISPLAY_ROWS,rows_in_lists-2,
	PANEL_LIST_WIDTH, w2-20,
	NULL);

}

if(loop_available==0) xv_set(prds_popup1->loop,PANEL_INACTIVE,TRUE,NULL);

/* initialize the timer */
timer_func();

/* the signals */
notify_set_destroy_func(prds_basewindow->basewindow,exit_process);
	/* SIGTERM */
notify_set_signal_func(prds_basewindow->basewindow,exit_process,
	SIGHUP,NOTIFY_ASYNC);
notify_set_signal_func(prds_basewindow->basewindow,exit_process,
	SIGINT,NOTIFY_ASYNC);
notify_set_signal_func(prds_basewindow->basewindow,msg_from_display,
	SIGUSR1,NOTIFY_SYNC);

/* set lists */
update_list0();
send_over_list1();

/* set color */
gcm_initialize_colors(prds_popup1->controls1, 
	background_color,foreground_color);
xv_set(prds_popup1->prd_list,
	PANEL_ITEM_COLOR, gcm_color_index(foreground_color),NULL);
xv_set(prds_popup1->ind_list,
	PANEL_ITEM_COLOR, gcm_color_index(foreground_color),NULL);
xv_set(prds_popup1->tim_list,
	PANEL_ITEM_COLOR, gcm_color_index(foreground_color),NULL);
xv_set(prds_popup1->cut,
	PANEL_CHOICE_COLOR, 0, gcm_color_index(foreground_color),NULL);
xv_set(prds_popup1->loop,
	PANEL_CHOICE_COLOR, 0, gcm_color_index(foreground_color),NULL);

return (0);

}

/******************************************************************


/**********************************************************************
process a message from the display                                   */

Notify_func msg_from_display()
{

if(strcmp(imsg->msg,"Close import win")==0)
	xv_set(prds_popup1->popup1,XV_SHOW,FALSE,NULL);

if(strcmp(imsg->msg,"Show import win")==0)
	xv_set(prds_popup1->popup1,XV_SHOW,TRUE,NULL);

/*printf("Message from display\n");*/

}

/******************************************************************
update the list0                                                  */

int update_list0()
{
static short up[] ={	
	0x1DE8,
	0x77FC,
	0x5836,
	0xE00F,
	0xC006,
	0xC005,
	0x8423,
	0x8002,
	0x8003,
	0x8813,
	0x8813,
	0xC425,
	0xC3C5,
	0xA009,
	0x9833,
	0x87C1
};
static short down[] ={
	0x0AA8,
	0x5FD0,
	0x3836,
	0xE008,
	0x4007,
	0x4004,
	0x8422,
	0x8002,
	0x8002,
	0x8002,
	0x87C2,
	0x4C64,
	0x5014,
	0x2008,
	0x1830,
	0x07C0
};

static Server_image upg,downg;
static int init=0;
int i;
char str[64];
static short ogl[LIST_SIZE],on_prod[LIST_SIZE];

Font_string_dims dims;


if(init==0){
  init=1;
  upg=(Server_image)xv_create(NULL,SERVER_IMAGE,
	XV_WIDTH, 16, XV_HEIGHT, 16,
	SERVER_IMAGE_BITS, (char *)up,  NULL);
  downg=(Server_image)xv_create(NULL,SERVER_IMAGE,
	XV_WIDTH, 16, XV_HEIGHT, 16,
	SERVER_IMAGE_BITS, (char *)down,  NULL);
  for(i=0;i<n_list0;i++){
	xv_set(prds_popup1->prd_list,
		PANEL_LIST_INSERT,  i,
		PANEL_LIST_ROW_HEIGHT,  i, 16,
		PANEL_LIST_CLIENT_DATA,  i,  i,           
		NULL);
  }
  for(i=0;i<n_list0;i++){
    if(pconf[i].selected>0)
	xv_set(prds_popup1->prd_list,
		PANEL_LIST_SELECT,  i,  TRUE, NULL);
    else
	xv_set(prds_popup1->prd_list,
		PANEL_LIST_SELECT,  i,  FALSE, NULL);
    ogl[i]=on_prod[i]=-1;
  }

}


for(i=0;i<n_list0;i++){
  char *pt,str1[64];
  int len,st;

  if(ogl[i]==pconf[i].psta && n_prod[i]==on_prod[i]) continue;
  ogl[i]=pconf[i].psta;
  on_prod[i]=n_prod[i];

  sprintf(str1,"%d",(int)n_prod[i]);	
  xv_get(font,FONT_STRING_DIMS,str1,&dims);
  st=w_list0-dims.width;
  xv_get(font,FONT_STRING_DIMS,pconf[i].fname,&dims);
  len=dims.width;
  strcpy(str,pconf[i].fname);
  pt=str+strlen(str);
  while(1){ /* add spaces */
	if(len>st) break;
	*pt++='.';
	len+=space_width;
  }
  *pt='\0';
  sprintf(str1,"%s%d",str,(int)n_prod[i]);
	
  if(pconf[i].psta==1)
	xv_set(prds_popup1->prd_list,
		PANEL_LIST_GLYPH, i, upg, 
		PANEL_LIST_STRING, i, str1, NULL);
  else
	xv_set(prds_popup1->prd_list,
		PANEL_LIST_GLYPH, i, downg, 
		PANEL_LIST_STRING, i, str1, NULL);
}

return (0);

}
  

/**********************************************************************
the timer                                                             */

Notify_func timer_func()
{
struct itimerval mytimer;
extern Notify_func timer_func();
int i;

for(i=0;i<6;i++){
	if(alarm_time[i]<0) continue;
	alarm_time[i]=alarm_time[i]-1;
	if(alarm_time[i]<=0) alarm_time[i]=-1;
	else continue;

	/* process i */

    switch(i){
	case 0:  /* for switch off the main window */
	        xv_set(prds_basewindow->basewindow, XV_SHOW, FALSE, NULL);
		break;
	case 1:
		check_time_out(); /* for import time out */
		break;
	case 2:
		rearange_data();
		alarm_time[2]=120;
		break;
	case 3:
		if(test_mode==0) import_input();
		else import_input_s();
		alarm_time[3]=1;
		break;
	case 4:
		break;

	default:
		break;
    }
}

mytimer.it_value.tv_sec=1;
mytimer.it_interval.tv_sec=1;
mytimer.it_value.tv_usec=0;
mytimer.it_interval.tv_usec=0;

notify_set_itimer_func(prds_basewindow->basewindow,
			timer_func, ITIMER_REAL, &mytimer, NULL);


return (0);
}

/******************************************************************
initialize global vars                                         */

int initialize_vars()
{
int i;
char *cpt;

rows_in_lists=10;
strcpy(background_color, "gray");
strcpy(foreground_color, "black");
font_name[0]='\0';
initially_closed=0;
w_loc_x=0;
w_loc_y=0;

overlay_cut_mode=0;

for(i=0;i<6;i++) alarm_time[i]=-1;
alarm_time[0]=4; /* for main window control */
alarm_time[4]=4;

sel_pind=0;
sel_lind=0;

n_list0=n_list1=n_list2=0;
product_server_on=0;
create=1;
loop_available=0;

d_pt=0;

v_time=300;

/* the shared memory */
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
data_size=SHM_SIZE/4-((int)(cpt-shmpt))/4;
printf("Data size: %d\n",data_size);

lock_shm(1); 

omsg->key=VALID;
omsg->pid=getpid();
omsg->np=0;
omsg->new=1;
omsg->loop=0;
lock_shm(0);

return (0);
}

/*
 * Event callback function for `popup1'.
 */
Notify_value
popup1_e(win, event, arg, type)
	Xv_window	win;
	Event		*event;
	Notify_arg	arg;
	Notify_event_type type;
{
	prds_popup1_objects	*ip = (prds_popup1_objects *) xv_get(win, XV_KEY_DATA, INSTANCE);

	if(overlay_cut_mode==1){
	  overlay_cut_mode=0;
	  xv_set(prds_popup1->cut,PANEL_VALUE,0,NULL);
	  xv_set(prds_popup1->controls1,WIN_CURSOR, cursor0, NULL);
		if(create==1) create=0;
		else create=1;
	}

	return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*
 * Notify callback function for `prd_list'.
 */
int
prd_list_h(item, string, client_data, op, event)
	Panel_item	item;
	char		*string;
	Xv_opaque	client_data;
	Panel_list_op	op;
	Event		*event;
{
	prds_popup1_objects	*ip = (prds_popup1_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
int setout,ind,sel;

	setout=0;
	if(event_id(event)==32563){
		if(event->ie_locx>s_line0) setout=1;
	}

	sel=-1;

	switch(op) {
	case PANEL_LIST_OP_DESELECT:
		sel=0;
		break;

	case PANEL_LIST_OP_SELECT:
		sel=1;
		break;

	case PANEL_LIST_OP_VALIDATE:
		break;

	case PANEL_LIST_OP_DELETE:
		break;
	}
	if(sel==-1) return XV_OK;


	if(setout==1) { /* don't change */
	  /* get the item index */
	  ind=(int )client_data&0xffff;

	  if(ind>=0){ /* reset the selection */
		if(sel==0)
 		   xv_set(item,
	           PANEL_LIST_SELECT,  ind,  TRUE,
	           NULL);
		if(sel==1)
 		   xv_set(item,
	           PANEL_LIST_SELECT,  ind,  FALSE,
	           NULL);
	  }
	  msg[1]=3; 
	}
	else msg[1]=sel; 
		/* msg[1]:0,1,3: deselect, select, not changed */

	if(overlay_cut_mode==1){
	  overlay_cut_mode=0;
	  xv_set(prds_popup1->controls1,WIN_CURSOR, cursor0, NULL);
	  xv_set(prds_popup1->cut,PANEL_VALUE,0,NULL);
	  return XV_OK;
	}

	msg[0]=19;
	string[22]='\0';
	strcpy((char *)&msg[2],string);
	msg[7]=(int )client_data; /* index in the list */
	over_control(msg); 

	return XV_OK;

}

/*
 * Notify callback function for `ind_list'.
 */
int
ind_list_h(item, string, client_data, op, event)
	Panel_item	item;
	char		*string;
	Xv_opaque	client_data;
	Panel_list_op	op;
	Event		*event;
{
	prds_popup1_objects	*ip = (prds_popup1_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
int setout,ind,sel;

	setout=0;
	if(event_id(event)==32563){
		if(event->ie_locx>s_line1) setout=1;
	}

	sel=-1;
	
	switch(op) {
	case PANEL_LIST_OP_DESELECT:
		sel=0;
		if(overlay_cut_mode==1) sel=2;
		break;

	case PANEL_LIST_OP_SELECT:
		sel=1;
		if(overlay_cut_mode==1) sel=2;
		break;

	case PANEL_LIST_OP_VALIDATE:
		break;

	case PANEL_LIST_OP_DELETE:
		break;
	}
	if(sel==-1) return XV_OK;


	if(setout==1 && sel<2) { /* don't change */
	  /* get the item index */
	  ind=(int )client_data&0xffff;

	  if(ind>=0){ /* reset the selection */
		if(sel==0)
 		   xv_set(item,
	           PANEL_LIST_SELECT,  ind,  TRUE,
	           NULL);
		if(sel==1)
 		   xv_set(item,
	           PANEL_LIST_SELECT,  ind,  FALSE,
	           NULL);
	  }
	  msg[1]=3; 
	}
	else{
	  msg[1]=sel;
	}

		/* msg[1]:0,1,2,3:deselected,selected,deleted,not changed */

	msg[0]=32;
	string[22]='\0';
	strcpy((char *)&msg[2],string);
	msg[7]=(int )client_data; /* msg[7]<<16+index in the list */
	modify_data(msg); 

	return XV_OK;
}

/*
 * Notify callback function for `tim_list'.
 */
int
tim_list_h(item, string, client_data, op, event)
	Panel_item	item;
	char		*string;
	Xv_opaque	client_data;
	Panel_list_op	op;
	Event		*event;
{
	prds_popup1_objects	*ip = (prds_popup1_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);

	msg[1]=-1;
	
	switch(op) {
	case PANEL_LIST_OP_DESELECT:
		msg[1]=0;
		if(overlay_cut_mode==1) msg[1]=2;
		break;

	case PANEL_LIST_OP_SELECT:
		msg[1]=1;
		if(overlay_cut_mode==1) msg[1]=2;
		break;

	case PANEL_LIST_OP_VALIDATE:
		break;

	case PANEL_LIST_OP_DELETE:
		break;
	}
		/* msg[1]:0,1,2:deselected,selected,not changed */

	if(msg[1]>=0){
		msg[0]=33;
		string[22]='\0';
		strcpy((char *)&msg[2],string);
		msg[7]=(int )client_data;/* msg[7]<<16+index in the list */
		modify_data(msg);
	}
	return XV_OK;
}

/*
 * Notify callback function for `cut'.
 */
void
cut_h(item, value, event)
	Panel_item	item;
	unsigned int	value;
	Event		*event;
{
	prds_popup1_objects	*ip = (prds_popup1_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);

	if(pconf[sel_lind].tss==-1) { /* can not be cut */
		if(value==0)
	 	   xv_set(prds_popup1->cut,PANEL_VALUE, 1, NULL);
		else 
	 	   xv_set(prds_popup1->cut,PANEL_VALUE, 0, NULL);
		return;
	}
	
	if(value==0){
		overlay_cut_mode=0;
	  xv_set(prds_popup1->controls1,WIN_CURSOR, cursor0, NULL);
	}
	else{
		overlay_cut_mode=1;
	  xv_set(prds_popup1->controls1,WIN_CURSOR, cursor1, NULL);
	}
	
/*	fprintf(stderr, "prds: cut_h: value: 0x%x\n", value);*/
}

/*
 * Notify callback function for `loop'.
 */
void
loop_h(item, value, event)
	Panel_item	item;
	unsigned int	value;
	Event		*event;
{
	prds_popup1_objects	*ip = (prds_popup1_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);

	lock_shm(1);
	if(value==0) omsg->loop=0;
	else omsg->loop=v_time;
	lock_shm(0);
	
/*	fprintf(stderr, "prds: loop_h: value: 0x%x\n", value); */
}



/******************************************************************
function setting lists                                           */

int set_list(msg)
int *msg;
{
static int cnt[3]={0,0,0};
Xv_opaque id;
int i,j,k;
int item,sw; 
	/* item=0,1: ind_list, tim_list in popup1
	sw=msg[1]=0: clear from item msg[2] to item msg[3],
	 =1: rewrite item msg[2] selected, =2 not selected
	 =3: set label;
	msg[0]=34: accumulate. msg[1]=0: start; =1: update the list;
		If call with msg[1]=0 again before calling with =1,
		the old accumulated is discarded.
	msg[2]: the list index to work on */
char *pt;
static int acu_mode=0,amsg[LIST_SIZE+4][8],nac=0;
int lo,loop,from_save;
static int need_close=0;
int closed;
static int from1=0,from2=0;

loop=1;
from_save=0;

if(msg[0]==34){
	if(msg[1]==0){ /* start accumulate mode */
	    need_close=0;
	    from1=from2=0;
	    acu_mode=1;
	    nac=0; /* see the comments after msg[0]=34 */
	    return (0);
	}
	if(msg[1]==1){ /* terminate accumulate mode */
	    acu_mode=0;
	    loop=nac;
	    nac=0;
	    from_save=1;
	}
}

if(acu_mode==1){ /* accumulate the messages */
	    if(nac>=LIST_SIZE+4){
		printf("Error found in set_list.\n");
		return (1);
	    }
	    for(i=0;i<8;i++) amsg[nac][i]=msg[i];
	    if(msg[1]==0&&msg[3]-msg[2]>4) need_close=1;
	    if(msg[0]==30) from1=1;
	    if(msg[0]==31) from2=1;
	    nac++;
	    return (0);
}

if(msg[0]==30) from1=1;
if(msg[0]==31) from2=1;

closed=0;
if(loop>4 || need_close==1 || (msg[1]==0 && msg[3]-msg[2]>4)){
	if(from2==1) 
		xv_set(prds_popup1->tim_list,XV_SHOW,FALSE,NULL);
	if(from1==1) 
	        xv_set(prds_popup1->ind_list,XV_SHOW,FALSE,NULL);
	closed=1;
}

need_close=0;

for(lo=0;lo<loop;lo++){

if(from_save==1){
	for(i=0;i<8;i++) msg[i]=amsg[lo][i];
}
item=msg[0]-30;
sw=msg[1];
pt=(char *)&msg[3];

if(item<0 || item>1) continue;

id=prds_popup1->ind_list;
if(item==1) id=prds_popup1->tim_list;

if(sw==0){
	j=cnt[item]-1;
	if(j>msg[3]) j=msg[3];
	for(i=j;i>=msg[2];i--){
  		xv_set(id,
	        PANEL_LIST_DELETE,  i,
		NULL);
	}
	j=j-msg[2]+1;
	if(j<0) j=0;
	cnt[item]=cnt[item]-j;
}

if(sw==1){
  if(msg[2]>=cnt[item]){
    xv_set(id,
	PANEL_LIST_INSERT,  cnt[item],
	PANEL_LIST_STRING,  cnt[item],  pt,
	PANEL_LIST_SELECT,  cnt[item],  TRUE,
	PANEL_LIST_CLIENT_DATA,  cnt[item],  (msg[7]<<16)+cnt[item],           
	NULL);
    cnt[item]++;
  }
  else
  xv_set(id,
	PANEL_LIST_STRING,  msg[2],  pt,
	PANEL_LIST_SELECT,  msg[2],  TRUE,
	PANEL_LIST_CLIENT_DATA,  msg[2],  (msg[7]<<16)+msg[2],           
	NULL);
}

if(sw==2){
  if(msg[2]>=cnt[item]){
    xv_set(id,
	PANEL_LIST_INSERT,  cnt[item],
	PANEL_LIST_STRING,  cnt[item],  pt,
	PANEL_LIST_SELECT,  cnt[item],  FALSE,
	PANEL_LIST_CLIENT_DATA,  cnt[item],  (msg[7]<<16)+cnt[item],           
	NULL);
    cnt[item]++;
  }
  else
  xv_set(id,
	PANEL_LIST_STRING,  msg[2],  pt,
	PANEL_LIST_SELECT,  msg[2],  FALSE,
	PANEL_LIST_CLIENT_DATA,  msg[2],  (msg[7]<<16)+msg[2],           
	NULL);
}

if(sw==3){
  xv_set(id,
	PANEL_LABEL_STRING, pt,
	NULL);
}

} /* end of for lo */

if(closed==1){
	if(from2==1) 
		xv_set(prds_popup1->tim_list,XV_SHOW,TRUE,NULL);
	if(from1==1) 
	        xv_set(prds_popup1->ind_list,XV_SHOW,TRUE,NULL);
	closed=0;
}
from1=from2=0;

return(0);
}
