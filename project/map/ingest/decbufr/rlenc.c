/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          RLENC.C

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
This function encodes a "one byte per pixel" radar image to BUFR runlength-
code and stores the resulting values by a call to VAL_TO_ARRAY.

AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "desc.h"
#include "bufr.h"
#include "rlenc.h"

#define LBUFLEN 1000         /* Size of an internal buffer holding one 
                                uncompressed line */
#define ENCBUFL 500          /* Size of an internal buffer holding one
                                compressed line */

static int compress_line (P5(int line, char *src, int ncols, varfl **dvals, 
                             size_t *nvals));

/*============================================================================*/
int rlenc (infile, nrows, ncols, vals, nvals)

char *infile;
int nrows;
int ncols;
varfl **vals;
size_t *nvals;

/* This function encodes a "one byte per pixel" radar image to BUFR runlength-
   code and stores the resulting values by a call to VAL_TO_ARRAY.

   parameters:
   INFILE:   File holding the "one byte per pixel" radar image.
   NCOLS:    Number of columns of the image.
   NROWS:    Number of rows of the image.
   VALS:     Float-array holding the coded image.
   NVALS:    Number of values in VALS.

   The return-value ist 1 on success, 0 on a fault.
*/

{
  FILE *fp;
  char buf[LBUFLEN];
  int i;

/******* check if the internal buffer is large enough to hold one uncompressed
         line */

  assert (ncols <= LBUFLEN);

/******* open file holding the radar image */

  fp = fopen (infile, "rb");
  if (fp == NULL) return 0;

/******* output number of rows */

  val_to_array (vals, (varfl) nrows, nvals);  

/******* compress line by line */

  for (i = 0; i < nrows; i ++) {
    if (fread (buf, 1, ncols, fp) != (size_t) ncols) goto err;
    if (!compress_line (i, buf, ncols, vals, nvals)) goto err;
  }
  fclose (fp);
  return 1;

err:
  fclose (fp);
  return 0;
}

/*===========================================================================*/
static int compress_line (line, src, ncols, dvals, nvals)

int line;
char *src;
int ncols;
varfl **dvals;
size_t *nvals;

/* This function encodes one line of a radar image to BUFR runlength-code and
   stores the resulting values by a call to VAL_TO_ARRAY.

   parameters:
   LINE:     Line number.
   SRC:      Is where the uncompressed line is stored.
   NCOLS:    Number of pixels per line.
   VALS:     Float-array holding the coded image.
   NVALS:    Number of values in VALS.
   
   The function returns 1 on success, 0 on a fault.
*/

{
  int count, i, n, npar, lens[LBUFLEN], cw, ncgi, nngi = 0;
  char val, lval = 0, vals[LBUFLEN];
  varfl encbuf[ENCBUFL];

/******* compress Line into a runlength format */

  count = n = 0;
  for (i = 0; i < ncols; i ++) {
    val = *(src + i);
    if (i != 0 && val != lval) {
      lens[n] = count;
      vals[n] = lval;
      n ++;
      count = 0;
      lval = val;
    }
    lval = val;
    count ++;
  }
  lens[n] = count;
  vals[n] = lval;
  n ++;
  

/******* line is runlength-compressed now to N parts, each of them identified 
         by a length (LENS) and a value (VALS). */
    

/******* Count number of parcels. One parcel is identified by a COUNT of 1
         followed by a COUNT > 1 */

  npar = 0;
  for (i = 0; i < n - 1; i ++) if (lens[i] == 1 && lens[i+1] > 1) npar ++;
  npar ++;

/******* output line-number */

  for (i = 0; i < ENCBUFL; i ++) encbuf[i] = (varfl) 0.0;
  cw = 0;
  encbuf[cw++] = line;  

/******* compress it to parcels */

  encbuf[cw++] = npar;                   /* number of parcels */

  ncgi = cw ++;                          /* is where the number of compressable groups is stored */
  encbuf[ncgi] = (varfl) 0.0;            /* number of compressable groups */

  i = 0;
  for (i = 0; i < n; i ++) {
    if (lens[i] > 1) {                   /* compressable group found */
      if (i > 0 && lens[i-1] == 1) {     /* A new parcel starts here */
        ncgi = cw ++;                    /* is where the number of compressable groups is stored */
        encbuf[ncgi] = (varfl) 0.0;      /* number of compressable groups */
      }
      encbuf[ncgi] += 1.0;
      encbuf[cw++] = lens[i];
      encbuf[cw++] = vals[i];
    }
    else {                               /* non compressable group found */
      if (i == 0 || lens[i-1] != 1) {    /* this is the first uncompressable group in the current parcel */
        nngi = cw ++;                    /* is where the number of non compressable groups is stored */
        encbuf[nngi] = (varfl) 0.0;      /* Number of non compressable groups */
      }
      encbuf[nngi] += 1.0;
      encbuf[cw++] = vals[i];
    }
  }
  if (lens[n-1] != 1) encbuf[cw++] = 0;  /* number of noncompressable groups in the last parcel = 0 */
  assert (cw <= ENCBUFL);

/******* compresson to parcels finished */


  for (i = 0; i < cw; i ++) if (!val_to_array (dvals, encbuf[i], nvals)) return 0;

/******* Output data for debugging purposes */

  /*cw = 0;
  printf ("\n\nline no. %d:\n", (int) encbuf[cw++]);
  npar = (int) encbuf[cw];
  printf ("number of parcels: %d\n", (int) encbuf[cw++]);
  for (i = 0; i < npar; i ++) {
    ncgi = (int) encbuf[cw];
    printf ("number of compressable groups: %d\n", (int) encbuf[cw++]);
    for (j = 0; j < ncgi; j ++) {
      printf ("count: %d\n", (int) encbuf[cw++]);
      printf ("val: %d\n", (int) encbuf[cw++]);
    }
    nngi = (int) encbuf[cw];
    printf ("number of uncompressable pixels: %d\n", (int) encbuf[cw++]);
    for (j = 0; j < nngi; j ++) {
      printf ("val: %d\n", (int) encbuf[cw++]);
    }
  }*/

  return 1;
}

/*============================================================================*/
int rldec (outfile, vals, nvals)

char *outfile;
varfl *vals;
size_t *nvals;

/* This function decodes a BUFR-runlength-encoded radar image stored at
   VALS. The decoded image is stored in a one "byte-per-pixel-format" at
   the file OUTFILE.

   parameters:
   OUTFILE:  Destination-file for the "one byte per pixel" radar image.
   VALS:     Float-array holding the coded image.
   NVALS:    Number of values needed for the radar image.

   The return-value ist 1 on success, 0 on a fault.
*/

{
  FILE *fp;
  int i, j, k, l, ngr, nrows, npar, val, count, nup;
  varfl *ovals;

/******* Open destination-file for output */

  fp = fopen (outfile, "wb");
  if (fp == NULL) return 0;

/******* decode line by line */

  ovals = vals;
  nrows = (int) *vals ++;               /* number of rows */
  for (i = 0; i < nrows; i ++) {        /* loop for lines */
    vals ++;                            /* skip linenumber */
    npar = (int) *vals ++;              /* number of parcels */
    for (j = 0; j < npar; j ++) {       /* loop for parcels */
      ngr = (int) *vals ++;             /* number of compressable groups */
      for (k = 0; k < ngr; k ++) {      /* loop for compressable groups */
        count = (int) *vals ++;
        val =   (int) *vals ++;
        for (l = 0; l < count; l ++) {  /* loop for length of group */
          fputc (val, fp);
        }
      }
      nup = (int) *vals ++;             /* number of uncompressable pixels */
      for (k = 0; k < nup; k ++) {      /* loop for uncompressable pixels */
        val = (int) *vals ++;
        fputc (val, fp);
      }
    }
  }

/******* close file */

  fclose (fp);

/******* calculate number of values in VALS occupied by the radar image */

  *nvals = vals - ovals;
  return 1;
}

/* end of file */

