#ifndef PDB_WAS_INCLUDED
#define PDB_WAS_INCLUDED

#define PDB_MAX_COLOR_NAME 20

typedef struct PDBpt_ {
   short 	x;	/* for now, km*100, origin = radar */
   short	y; 	/* x = 0x7FFF for discontinuous segment */ 
   } PDBpt;

typedef struct PDBproduct_ {
   unsigned short 	type;		/* product type */
   unsigned short       text_size;	/* size of text array in bytes */
   unsigned short 	npts;		/* number of points in pts array */
   unsigned short	line_type;
   unsigned short       holds[2];	/* future use */

   char	color[PDB_MAX_COLOR_NAME]; /* X color name */
   long	id;		/* group identifier */
   long	start_time;  	/* time that the product starts being valid */
   long	expire_time; 	/* time that the product stops being valid */
   long	generate_time; 	/* time that the product was made */
   long time4;		/* future use */
   long subid;		/* event within the group */
   long hold[2];	/* future use */
   
   PDBpt 	*pts;  		/* array of polyline points */
   char		*text;		/* each text element has the following form:
				   location: short y, short x
				   text, zero terminated
				   zero padded to next 32-bit boundry
			         */
   } PDBproduct;


/* REQUEST/RESPONSE TYPES */
#define PDB_CONNECT		1
#define PDB_DISCONNECT		2

#define PDB_PRODUCT		10
#define PDB_LIST_PRODUCTS	11
#define PDB_PRODUCT_DESCRIPTION 12
#define PDB_REQUEST_PRODUCT	13
#define PDB_STOP_PRODUCT	14
#define PDB_START		15
#define PDB_STOP		16
#define PDB_STATUS		17

#define PDB_LIST_MAPS		20
#define PDB_REQUEST_MAP 	21


/* PRODUCT FORMAT */
#define PDB_FORMAT_RAW		1
#define PDB_FORMAT_POLYLINE 	2

/* LINE TYPE */
#define PDB_LINETYPE_THIN_SOLID  1
#define PDB_LINETYPE_THICK_SOLID 2
#define PDB_LINETYPE_DASHED      3


/* COLORS.
   Currently (7/23/91) we use only 7 colors; but in the future there may be more.
   The TAAC display, however, will probabably always only use these seven:
   	Red, Green, LightBlue, Cyan, Magenta, Yellow, White
 */

#endif
