/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          ENCBUFR.C

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
This function read source-data from a textfile and codes is into a BUFR-file.


AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "desc.h"
#include "bufr.h"
#include "rlenc.h"
#include "bitio.h"

/*===========================================================================*/
/* internal functions                                                        */
/*===========================================================================*/

int rincols, rinrows;       /* number of rows/columns of a radar image */

/*===========================================================================*/
/* internal functions                                                        */
/*===========================================================================*/

static int get_filenames (P4(int argc, char *argv[], char *srcfile, char *buffile));
static int code_it (P5 (char *srcfile, char **das, size_t *ndas, char **dds, 
                        size_t *ndds));
static int save_it (P3(char *sec[], size_t secl[], char *buffile));
static int setup_section0125 (P2(char *sec[], size_t secl[]));

#define DESCFILE   "descr.txt"   /* File holding the supported data-descriptors */

/*===========================================================================*/
int main (argc, argv)

int argc;
char *argv[];

{
  char *sec[6];      /* 6 sections in BUFR-message */
  size_t secl[6];    /* length of 6 sections */
  char srcfile[80], buffile[80];
  int ret, i;

  for (i = 0; i < 6; i ++) sec[i] = NULL;
  rincols = rinrows = -1;   /* to indicate that the number of rows/columns of a radar image is unknown */

/******* Get input- and output-filenames from the command-line */

  if (!get_filenames (argc, argv, srcfile, buffile)) return 1;

/******* read supported data descriptors */

  if (!read_desc (DESCFILE)) {
    printf ("Unable to read supported data descriptors !\n");
    return 1;
  }

/******* code data in the source-file to a data-descriptor- and data-section */

  if (!code_it (srcfile, &sec[4], &secl[4], &sec[3], &secl[3])) return 1;

/******* setup section 0, 1, 2, 5 */

  if (!setup_section0125 (sec, secl)) {
    printf ("Unable to create section 0, 1, 2 and/or 5\n");
    ret = 1;
    goto exit;
  }

/******* Save coded data */

  if (!save_it (sec, secl, buffile)) ret = 1;
  else ret = 0;

/******* Free data */

exit:
  for (i = 0; i < 6; i ++) if (sec[i] != NULL) free (sec[i]);
  return ret;
}

/*===========================================================================*/
static int setup_section0125 (sec, secl)

char *sec[];
size_t secl[];

/* This function creates sections 0, 1, 2 and 5. The function returns 1 on
   success, 0 on a fault.
*/

{
  int i, hand;
  long len;
  time_t t;
  struct tm *ts;

  time (&t);
  ts = localtime (&t);

/******* create section 1 */

  hand = bitio_o_open ();
  if (hand == -1) return 0;
  bitio_o_append (hand, 24L, 24);                  /* length of section */
  bitio_o_append (hand, 0L, 8);                    /* master table used */
  bitio_o_append (hand, 0xffffL, 16);              /* originating center - missing */
  bitio_o_append (hand, 0L, 8);                    /* original BUFR message */
  bitio_o_append (hand, 0L, 8);                    /* no optional section */
  bitio_o_append (hand, 6L, 8);                    /* message type (radar = 6) */
  bitio_o_append (hand, 0L, 8);                    /* message subtype (radar-image = 0) */
  bitio_o_append (hand, 2L, 8);                    /* version number of master table used */
  bitio_o_append (hand, 1L, 8);                    /* version number of local table used */
  bitio_o_append (hand, (long) ts->tm_year, 8);    /* year */
  bitio_o_append (hand, (long) ts->tm_mon + 1, 8); /* month */
  bitio_o_append (hand, (long) ts->tm_mday, 8);    /* day */
  bitio_o_append (hand, (long) ts->tm_hour, 8);    /* hour */
  bitio_o_append (hand, (long) ts->tm_min, 8);     /* minute */
  bitio_o_append (hand, (long) ' ', 8);            /* originating radar station and country (8 octets) */
  bitio_o_append (hand, (long) ' ', 8);
  bitio_o_append (hand, (long) ' ', 8);
  bitio_o_append (hand, (long) ' ', 8);
  bitio_o_append (hand, (long) ' ', 8);
  bitio_o_append (hand, (long) ' ', 8);
  bitio_o_append (hand, 0L, 8);                    /* filler (0) */
  sec[1] = (char *) bitio_o_close (hand, &secl[1]);

/******* there is no section 2 */

  sec[2] = NULL;
  secl[2] = 0;

/******* create section 5 */

  hand = bitio_o_open ();
  for (i = 0; i < 4; i ++) bitio_o_append (hand, (long) '7', 8);
  sec[5] = (char *) bitio_o_close (hand, &secl[5]);

/******* calculate total length of BUFR-message */

  secl[0] = 8;     /* section 0 not yet setup */
  len = 0L;
  for (i = 0; i < 6; i ++) len += (long) secl[i];
  

/******* create section 0 */

  hand = bitio_o_open ();
  if (hand == -1) return 0;
  bitio_o_append (hand, (unsigned long) 'B', 8);
  bitio_o_append (hand, (unsigned long) 'U', 8);
  bitio_o_append (hand, (unsigned long) 'F', 8);
  bitio_o_append (hand, (unsigned long) 'R', 8);
  bitio_o_append (hand, len, 24);          /* length of BUFR-message */
  bitio_o_append (hand, 2L, 8);            /* BUFR edition number */
  sec[0] = (char *) bitio_o_close (hand, &secl[0]);
  return 1;
}

/*===========================================================================*/
static int save_it (sec, secl, buffile)

char *sec[];
size_t secl[];
char *buffile;

/* This functions save the BUFR-message 

   parameters:
   SEC:     Poiter-Array to the 6 sections.
   SECL:    Length of the sections.
   BUFFILE: Output-File

   The function returns 1 on success, 0 on a fault.
*/

{
  FILE *fp;
  int i;

/******* open file */

  fp = fopen (buffile, "wb");
  if (fp == NULL) return 0;

/******* output all sections */

  for (i = 0; i < 6; i ++) {
    if (fwrite (sec[i], 1, secl[i], fp) != secl[i]) goto err;
  }

/******* close file and return */

  fclose (fp);
  return 1;

/******* An error occoured during write */

err:
  fclose (fp);
  printf ("An error occoured during writing '%s'. File is invalid !\n", buffile);
  return 0;
}

/*===========================================================================*/
#define MAXDESCS   50   /* Maximum number of descriptors in the source-file */

static int code_it (srcfile, das, ndas, dds, ndds)

char *srcfile;
char **das;
size_t *ndas;
char **dds;
size_t *ndds;

/* This function creates from a source-file a data-descriptor- and a data-
   section.

   Parameters:
   SRCFILE:   Name of the file containing data to be coded.
   DAS:       Memory-area where the data-descriptor-section is stored.
   NDAS:      Number of bytes needed for the data-descriptor section.
   DDS:       Memory-area where data-section is stored.
   NDDS:      Number of bytes needed for the data-descriptor section.

   The function returns 1 on success, 0 on a fault.
*/

{
  char buf[200], rfile[80];
  int i, l, ndesc, n;
  unsigned int nv;
  dd descr[50], d;
  FILE *fp;
  varfl *vals, v;

/*===========================================================================*/
/* prepare input data as a list of data-descriptos and a list of             */
/* corresponding data-values                                                 */
/*===========================================================================*/

  fp = fopen (srcfile, "r");
  if (fp == NULL) return 0;

  vals = NULL;
  ndesc = 0;
  nv = 0;

/******* main loop */

  while (fgets (buf, 200, fp) != NULL) {
    l = strlen (buf);
    if (buf[l-1] == '\n') buf[l-1] = 0;        /* delete terminating \n */
    trim (buf);                                /* delete terminating blanks */
    /*printf ("%s\n", buf);*/
    if (buf[0] != '#' && strlen (buf) != 0) {  /* skip commets */

/******* read data from source-line */

      n = sscanf (buf, "%d %d %d %lf", &d.f, &d.x, &d.y, &v);

/******* line contains the name of an uncompressed radar-image-file */

      if (n == 3 && d.f == 3 && d.x == 21 && d.y == 192) {
        sscanf (buf, "%d %d %d %s", &d.f, &d.x, &d.y, rfile);
        if (rinrows == -1 || rincols == -1) {
          printf ("Unknown number of rows and/or columns\n");
          return 0;
        }
        if (!rlenc (rfile, rinrows, rincols, &vals, &nv)) {
          printf ("Error during runlength-compression.\n");
          return 0;
        }
        if (ndesc == MAXDESCS) {
          printf ("To many descripors in source-file '%s'\n", srcfile);
          return 0;
        }
        memcpy (&descr[ndesc], &d, sizeof (d));
        ndesc ++;
      }

/******* line contains of a descriptor and data */

      else if (n == 4) {
        if (ndesc == MAXDESCS) {
          printf ("To many descripors in source-file '%s'\n", srcfile);
          return 0;
        }
        memcpy (&descr[ndesc], &d, sizeof (d));
        ndesc ++;
        val_to_array (&vals, v, &nv);

/******* check is there is the number of rows/columns for a radar image */

        if (d.f == 0 && d.x == 30 && d.y == 21) rincols = (int) v;
        if (d.f == 0 && d.x == 30 && d.y == 22) rinrows = (int) v;
      }

/******* line consists only of a data-value */

      else {
        n = sscanf (buf, "%lf", &v);
        val_to_array (&vals, v, &nv);
      }
    }
  }

/******* input of source-file finished, close file */

  fclose (fp);

/*===========================================================================*/
/* code the two lists to a descriptor- and data-section                      */
/*===========================================================================*/

  /*for (i = 0; i < nv; i ++) printf ("%f\n", *(vals + i));*/
  i = bufr_create_msg (descr, ndesc, vals, (void **) das, (void **) dds, ndas, 
                       ndds);
  free (vals);
  return i;
}


/*===========================================================================*/
static int get_filenames (argc, argv, srcfile, buffile)

int argc;
char *argv[];
char *srcfile;
char *buffile;

/* This function reads the source-file and the destination-file from the
   input-line.

   Parameters:
   ARGC, ARGV:  parameters from main
   SRCFILE:     name of source-file to be coded.
   BUFFILE:     name of BUFR-output-file.

   The function returns 1 if the filknames could be read successfully from
   the command-line, 0 if not.
*/

{
  if (argc != 3) {
    printf ("usage: encbufr input_file output_file\n");
    return 0;
  }

  strcpy (srcfile, argv[1]);
  strcpy (buffile, argv[2]);
  return 1;
}

/* end of file */
