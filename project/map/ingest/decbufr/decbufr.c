/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          DECBUFR.C

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
This function reads a BUFR-file, decodes ist and stores decoded data in a
text-file.


AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "desc.h"
#include "bufr.h"
#include "bitio.h"
#include "rlenc.h"

/*===========================================================================*/
/* internal functions                                                        */
/*===========================================================================*/

static int get_filenames (P4(int argc, char *argv[], char *buffile, char *destfile));
static int get_lens (P2(char *buf, size_t secl[6]));
static int decode_it (P4(char *dd, size_t ddlen, char *da, size_t dalen));
static int ascii_out (P2(varfl val, int ind));

/*===========================================================================*/
/* internal functions                                                        */
/*===========================================================================*/

static char ddbuf[10];           /* Buffer needed for output of data-descriptors */
static FILE *fo;                 /* File-Pointer for outputfile */
static char imgfile[80];         /* Name of the file where the uncompressed image is to be stored */

#define DESCFILE   "descr.txt"   /* File holding the supported data-descriptors */
#define RIOUTFILE  "img.dec"     /* Name of file for uncompressed radar image */

/*===========================================================================*/
int main (argc, argv)

int argc;
char *argv[];

{
  char destfile[80], buffile[80], *buf;
  FILE *fp;
  long len, co;
  int i;
  size_t secl[6];
  char *sec[6];

/******* Get input- and output-filenames from the command-line */

  if (!get_filenames (argc, argv, buffile, destfile)) return 1;

/******* read supported data descriptors */

  if (!read_desc (DESCFILE)) {
    printf ("Unable to read supported data descriptors !\n");
    return 0;
  }

/****** open output-file */

  fo = fopen (destfile, "w");
  if (fo == NULL) {
    printf ("Unable to open outputfile '%s'\n", destfile);
    return 0;
  }

/****** read source-file. Therefore allocate memory to hold the complete
        BUFR-message */

  fp = fopen (buffile, "rb");
  if (fp == NULL) {
    printf ("unable to open file '%s'\n", buffile);
    return 0;
  }
  fseek (fp, 0L, SEEK_END);
  len = ftell (fp);
  fseek (fp, 0L, SEEK_SET);
  buf = malloc ((size_t) len);
  if (buf == NULL) {
    printf ("unable to allocate %ld bytes to hold BUFR-message !\n", len);
    fclose (fp);
    return 0;
  }
  if (fread (buf, 1, (size_t) len, fp) != (size_t) len) goto err;

/****** Get length of all 6 sections */

  if (!get_lens (buf, secl)) {
    printf ("unable to read length of data- and/or descriptor-section !\n");
    goto err;
  }

/******* calculate a pointer to the beginning of each section */

  co = 0L;
  for (i = 0; i < 6; i ++) {
    sec[i] = buf + co;
    co += secl[i];
  }

/******* decode data descriptor- and data-section now */

  if (!decode_it (sec[3], secl[3], sec[4], secl[4])) {
    printf ("unable to decode BUFR-message !\n");
    goto err;
  }
  fclose (fp);
  fclose (fo);
  free (buf);
  return 0;

/******* An error occoured: */

err:
  fclose (fp);
  free (buf);
  return 1;
}

/*===========================================================================*/
static int decode_it (dad, dadlen, da, dalen)

char *dad;
size_t dadlen;
char *da;
size_t dalen;

/* This function decodes a BUFR-message consiting of a data-descriptor-section
   and a data-section and saves the result by calling ASCII_OUT

   parameters:
   DAD:      Is where the data-descriptor-section is stored.
   DADLEN:   Number of baytes of the data-descriptor-section.
   DA:       Is where the data-section is stored.
   DALEN:    Number of bytes of the data-section.

   The function returns 1 on success, 0 on a fault.
*/

{
  dd *dds, *d;
  int ndds, i;
  unsigned int vali, nvals, n;
  varfl *vals;
  char buf[80];

  i = bufr_read_msg (da, dad, dalen, dadlen, &dds, &ndds, &vals, &nvals);
  if (!i) return 0;

/******* Save data read to an ASCII-File */

  vali = 0;
  i = 0;
  while (i < ndds) {
    d = dds + i;
    sprintf (ddbuf, "%2d %2d %3d", d->f, d->x, d->y);

/****** There is a radar image in the BUFR-message... */

    if (d->f == 3 && d->x == 21 && d->y == 193) {
      if (!rldec (imgfile, vals + vali, &n)) return 0;
      vali += n;
      strcpy (buf, ddbuf);
      strcat (buf, " ");
      strcat (buf, imgfile);
      fprintf (fo, "%s\n", buf);
    }      

/******* "ordinary" data */

    else {
      sprintf (ddbuf, "%2d %2d %3d", d->f, d->x, d->y);
      if (!bufr_parse (d, 0, 0, vals, &vali, ascii_out)) return 0;
    }

/******* If there is a replication factor -> increase i by the number of 
         descriptors to be replicated */

    if      (d->f == 1 && d->y == 0) i += d->x + 2;
    else if (d->f == 1 && d->y != 0) i += d->x + 1;
    else                             i ++;
  }

  return 1;
}

/*===========================================================================*/
static int ascii_out (val, ind)

varfl val;
int ind;

/* This function outputs one value to an ASCII-file

   parameters:
   VAL:    Data-value to be output.
   IND:    Index to the global array DES[] holding the description of
           known data-descriptors.

   The function returns 1 on success, 0 on a fault.
*/

{
  int i;

  i = fprintf (fo, "%s %12.5f  %2d %2d %3d %s\n", ddbuf, val, 
               des[ind]->elseq.el.d.f, des[ind]->elseq.el.d.x, 
               des[ind]->elseq.el.d.y, des[ind]->elseq.el.elname);
  strcpy (ddbuf, "         ");
  if (i == 6) {
    printf ("Error writing output-File !\n");
    return 0;
  }
  else return 1;
}

/*===========================================================================*/
static int get_lens (buf, secl)

char *buf;
size_t secl[6];

/* This function reads from a bufr-message the length of data- and
   data-descriptor-section. Therefore the buffer is opened as a bitstream
   and data is read.

   parameters:
   BUF:   Memory-area containing the BUFR-message.
   DDLEN: Size of data-descriptor-section.
   DALEN: Size of data-section.

   The return-value is 1 on success, 0 on a fault.
*/

{
  int h, co;
  unsigned long l;

/******* The length of section 0 is constant */

  secl[0] = 8;
  co = 8;

/******* length of section 1 */

  h = bitio_i_open (buf + co, 20);
  if (h == -1) return 0;
  bitio_i_input (h, &l, 24);
  secl[1] = (size_t) l;
  co += secl[1];
  bitio_i_close (h);

/******* there is no section 2 */

  secl[2] = 0;

/******* length of section 3 */

  h = bitio_i_open (buf + co, 20);
  if (h == -1) return 0;
  bitio_i_input (h, &l, 24);
  secl[3] = (size_t) l;
  co += secl[3];
  bitio_i_close (h);

/******* length of section 4 */

  h = bitio_i_open (buf + co, 20);
  if (h == -1) return 0;
  bitio_i_input (h, &l, 24);
  secl[4] = (size_t) l;
  co += secl[4];
  bitio_i_close (h);

/******* length of section 5 is constant */

  secl[5] = 4;

  return 1;
}

/*===========================================================================*/
static int get_filenames (argc, argv, buffile, destfile)

int argc;
char *argv[];
char *buffile;
char *destfile;

/* This function reads the source-file and the destination-file from the
   input-line.

   Parameters:
   ARGC, ARGV:  parameters from main
   BUFFILE:     name of BUFR-output-file.
   DESTFILE:    name of destination-file for decoded data.

   The function returns 1 if the filenames could be read successfully from
   the command-line, 0 if not.
*/

{
  if (argc < 3) {
    printf ("usage: decbufr input_file output_file [image_file]\n");
    return 0;
  }

  strcpy (buffile, argv[1]);
  strcpy (destfile, argv[2]);

  if (argc == 4) strcpy (imgfile, argv[3]);
  else strcpy (imgfile, RIOUTFILE);
  return 1;
}

/* end of file */
