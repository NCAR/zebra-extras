/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          DESC.H

AUTHOR:        Konrad Koeck
               Institute of Communication and Wave Propagation, 
               Technical University Graz, Austria

DATE CREATED:  23-FEB-1994

STATUS:        DEVELOPMENT FINISHED

This software may only be used, copied and distributed within members
of the Central European Weather Radar Network (CREAD) or within the 
Liaison Group on Operational European Weather Radar Networking (GORN). 
Any other use only with the permission of the author.


FUNCTIONAL DESCRIPTION:
-----------------------
Includefile that defines the data-structures needed to hold the supported
data-descriptors. Function-prototype for READDESC.

AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

typedef double varfl;    /* Defines the internal float-variable type. This can
                            be float or double. Float needs less memory than
                            double. Double-floats need not to be converted by
                            your machine before operation (software runs 
                            faster).
                            Note that the format-string in all scanf-calls
                            must be changed for VARFL-values !
                         */

/*===========================================================================*/
/* definitions of data-structures                                            */
/*===========================================================================*/

typedef struct dd {          /* Structure that describes one data descriptor */
  int f;
  int x;
  int y;
} dd;

#define MAXUNIT 20           /* Max. Length of the Unit */
#define MAXELN  80           /* Max. Length of an element name */
#define MAXDEL  15           /* One sequence descriptor consists of max. MAXDEL descriptors */
#define SEQDESC 0            /* Identifier for a sequence descriptor */
#define ELDESC  1            /* Identifier for an element descriptor */
#define MAXCTEL 20           /* Max. number of elements in a code-table */
#define MTCMENL 80           /* Max. Length of a code table meaning */
#define MAXTAB  30           /* Max. number of tables */

typedef struct del {         /* Structure that defines an element descriptor */
  dd d;                      /* Descriptor ID */
  char unit[MAXUNIT+1];      /* Unit */
  int scale;                 /* Scale */
  varfl refval;             /* Reference Value */
  int dw;                    /* Data width (number of bits) */
  char elname[MAXELN+1];     /* element name */
} del;

typedef struct dseq {        /* Structure that defines a sequence of descriptors */
  dd d;                      /* sequence-descriptor ID */
  int nel;                   /* Number of elements */
  dd del[MAXDEL];            /* list of element descriptors */
} dseq;

typedef struct desc {        /* Structure that defines on descriptor. This can 
                                be an element descriptor or a sequence descriptor */
  int id;                    /* Can be SEQDESC or ELDESC */
  union elseq {
    del el;                  /* Element descriptor */
    dseq seq;                /* Sequence descriptor */
  } elseq;
} desc;

typedef struct ctent {       /* defines one entry in a code-table */
  int code;                  /* code */
  char mean[MTCMENL+1];      /* What code means */
} ctent;

typedef struct ctab {        /* Structure that defines a code-table */
  dd d;                      /* Descriptor the table is valid for */
  int ne;                    /* Number of entries */
  ctent ce[MAXCTEL];         /* Code-Table entries */
} ctab;

/*===========================================================================*/
/* variables needed to hold data descriptors                                 */
/* If READDESC_MAIN is not defined all variables are declared as external.   */
/* So you sould define READDESC_MAIN only in one function. Otherwise you will*/
/* have this symbols multiple defined.                                       */
/*===========================================================================*/

#define MAXDESC   100         /* Max. number of descriptors in the descriptor-file */

#ifdef READDESC_MAIN
  int ndes;                  /* Number of descriptors found in the descriptor file */
  desc *des[MAXDESC];        /* data descriptors */
  int ntab;                  /* number of tables */
  ctab *tab[MAXTAB];         /* Max. number of tables */
  int dw = 128;              /* add dw - 128 to the data-width (dw can be optionally set by 2 01 YYY) */
#else
  extern int ndes;
  extern desc *des[MAXDESC];
  extern int ntab;
  extern ctab *tab[MAXTAB];
  extern int dw;
#endif

/*===========================================================================*/
/* The following definition will be used to have either                      */
/* function-prototyping in ANSI-C e.g.: void abc (int a, int b);   or        */
/* Kernighan-Ritchie-prototyping link   void abc ();                         */
/*===========================================================================*/

#if defined (NON_ANSI)
#define P0
#define P1(a)                
#define P2(a,b)              
#define P3(a,b,c)            
#define P4(a,b,c,d)          
#define P5(a,b,c,d,e)        
#define P6(a,b,c,d,e,f)      
#define P7(a,b,c,d,e,f,g)    
#define P8(a,b,c,d,e,f,g,h)  
#else
#define P0                   void
#define P1(a)                a
#define P2(a,b)              a,b
#define P3(a,b,c)            a,b,c
#define P4(a,b,c,d)          a,b,c,d
#define P5(a,b,c,d,e)        a,b,c,d,e
#define P6(a,b,c,d,e,f)      a,b,c,d,e,f
#define P7(a,b,c,d,e,f,g)    a,b,c,d,e,f,g
#define P8(a,b,c,d,e,f,g,h)  a,b,c,d,e,f,g,h
#endif

/* for compilers having SEEK_CUR and SEEK_SET not defined: */

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

/*===========================================================================*/
/* function prototype                                                        */
/*===========================================================================*/

int read_desc (P1(char *filename));
void trim (P1(char *buf));

/* end of file */
