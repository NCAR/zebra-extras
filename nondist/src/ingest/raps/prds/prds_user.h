
/********************************************************************
	file: prds_user.h
	header file for prds user
********************************************************************/

/*#include <xview/font.h>*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define SHM_KEY 6713
#define SHM_SIZE 300*1024
#define VALID 1234567891

#define N_PROD     1024

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
