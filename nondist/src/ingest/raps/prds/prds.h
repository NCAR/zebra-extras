
/********************************************************************
	file: prds.h
	header file for prds
********************************************************************/

#include <xview/font.h>

EXT int alarm_time[6]; /* timers */

EXT Notify_func timer_func();
EXT Notify_func exit_process();
EXT Notify_func msg_from_display();

#define SHM_KEY 6713
#define SHM_SIZE 300*1024
#define VALID 1234567891

#define N_PROD     1024
#define LIST_SIZE  128

EXT int data_size; /* size of the buffer for data in long */

EXT int shmid;
EXT char *shmpt;

struct outmsg {
	int key;  /* a key words, 1234567891*/
	int pid;   /* pid of prds */
	unsigned int np;   /* number of products in the data base */
	int new; /* >1: new data base, 0: processed by display */
	int loop; /* the loop is on/off (v_intvl/0) */
	char res[12];
};

typedef struct outmsg Out_msg;

struct inmsg {
	char msg[28];  /* a msg string */
	unsigned int flag; /* flag for communacation */
};

typedef struct inmsg In_msg;

EXT Out_msg *omsg;
EXT In_msg *imsg;

/* product struct */
struct product {	
		unsigned char pid;  /* id of the products */
		unsigned char line_type;  /* line type, 0, 1, ... */
		unsigned short tlen;
			/* text data length in long words */
		char color_name[20];  /* color name */
		short index;
			 /* index in a class */
		short tindex;
			 /* time index in a time sequence */
			 /* if <0, this is a map */
		unsigned short llen;
			/* # of points for the line drawing */
		short id; /* a product id from sender */
		unsigned int time;  /* also used for pointor to
					map name string */
		unsigned int lpt; 
			/* offset (in long int) in data field */
		unsigned int tpt;
		unsigned char deleted;
		unsigned char selected;
		unsigned char active;   /* used by display */
		unsigned char display;   /* used by display */
		unsigned short period; /* valid period. in seconds */
		unsigned short next_tms; /* next item in the time series */
		short timer1; /* timer for display */
		short cf_ind; /* index in configuration file */
		int timer2; /* timer for life */
};
typedef struct product Product;

EXT Product *prod; /* adress for product structs */

struct prod_conf {  /* the product informatin */
	unsigned char pid;
	char tss;  /* indicating that it is a time sequence */
	unsigned char psta;  /* up or down */
	unsigned char res; 
	unsigned short selected; 
	unsigned short dflag; 
	unsigned short etm_dis;	
	unsigned short etm_lif;
	int time_sel;
	char sname[4];
	char fname[16];
};
typedef struct prod_conf Prod_conf;

EXT Prod_conf pconf[LIST_SIZE];

EXT int sel_pid,sel_pind,sel_lind;

EXT int *data; /* address for data field (points and texts) */
EXT int d_pt; /* dataend offset in data field */

EXT int overlay_cut_mode;

/* the number of items in lists */
EXT int n_list0,n_list1,n_list2;

 /* for window configuration */
EXT int rows_in_lists;  
EXT char background_color[32];
EXT char foreground_color[32];
EXT char font_name[64];
EXT int initially_closed;
EXT int w_loc_x,w_loc_y;

EXT int n_prod[LIST_SIZE]; /* number of prods for each type */

EXT int msg[8];


EXT Xv_font font;
EXT int space_width,s_line0,s_line1,w_list0;

EXT int product_server_on;
EXT int create;
EXT loop_available;

EXT int v_time;
