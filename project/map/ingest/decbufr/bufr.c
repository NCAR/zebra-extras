/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          BUFR.C

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
The Functions in this file can be used to encode/decode general BUFR-messages.
The file bufr.h must be included to get the function-prototyping for the
functions in this file. More details can be found in the descriptions of
the functions.

AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "desc.h"
#include "bufr.h"
#include "bitio.h"

/*===========================================================================*/
/* internal data                                                             */
/*===========================================================================*/

int desch;                   /* bitstream-handle for data-descriptor-section */
int datah;                   /* bitstream-handle for data-section */

#define MAXREPCOUNT 300      /* Max. replication count */
typedef struct repdat {      /* Internal structure needed to support replication descriptors */
  int ndesc;                 /* Number of descriptors to be replicated */
  dd desc[MAXREPCOUNT];      /* descriptors to be replicated */
  int nrep;                  /* Number of replications */
  int curdesc;               /* current descriptor in replication loop */
  int currep;                /* Current replication */
} repdat;

/*===========================================================================*/
/* internal functions                                                        */
/*===========================================================================*/

static int bufr_out_descsec (P2(dd *descp, int ndescs));
static int bufr_out_datasec (P5(dd *descs, int start, int end, varfl *vals, 
                                unsigned *vali));
static int data_to_datastream (P2(varfl val, int ind));
static int bufr_data_from_datasect (P5(dd *descp, int start, int end, 
                                       varfl **vals, size_t *nvals));

/*===========================================================================*/
int bufr_create_msg (descs, ndescs, vals, datasec, ddsec, datasecl, ddescl)

dd *descs;
int ndescs;
varfl *vals;
void **datasec;
void **ddsec;
size_t *datasecl;
size_t *ddescl;

/* This function creates from a number of varfl-values a data section and
   a data descripor section for a BUFR-message. Memory for both sections is
   allocated in this function and must ne freed by the calling functions.

   parameters:

   DESCS:   Data-descriptors corresponding to VALS. For each descriptor there
            must be a data-vaule stored in VALS. DESCS may also include
            replication factors. In that case there must be a larger number of
            VALS then of DESCS.

   NDESCS:  Number of data descriptos contained in DESCS.

   VALS:    Data-values to be coded in the data section. For each entry in
            DESCS there must be an entry in VALS. If there are relication
            factors in DESCS, of course there must be as much VALS as definded
            by the replication factor.

   DATASEC: Is where  the data-section is stored. The memory-area for the
            data-section is allocated by this function and must be freed by
            the calling function.

   DDESC:   Is where the data-descriptor-section in stored. The memory needed is
            allocated by this function and must be freed by the calling 
            function.

   DATASECL:Number of bytes in DATASEC.

   DDESCL:  Number of bytes in DDESC.

   The return-value is 1 if data was successfully stored, 0 if not.
*/

{
  unsigned vali;
  size_t n;

/******* Open two bitstreams, one for data-descriptors, one for data */

  desch = bitio_o_open ();
  datah = bitio_o_open ();
  if (desch == -1 || datah == -1) goto err;

/******* output data to the data descriptor bitstream */

  bitio_o_append (desch, 0L, 24);  /* length of descriptor-section, set to 0.
                                      The currect length is set at the end of
                                      this section. */
  bitio_o_append (desch, 0L, 8);   /* reseved octet, set to 0 */
  bitio_o_append (desch, 1L, 16);  /* number of data subsets */
  bitio_o_append (desch, 128L, 8); /* observed non-compressed data */

  bufr_out_descsec (descs, ndescs);

/******* output data to the data-section */

  bitio_o_append (datah, 0L, 24);  /* Length of section (correct value stored 
                                      later) */
  bitio_o_append (datah, 0L, 8);   /* reserved octet, set to 0 */

  vali = 0;       /* current output-value is *(vals + vali) */
  bufr_out_datasec (descs, 0, ndescs - 1, vals, &vali);

/******* Save number of octets in the descriptor-section */

  n = bitio_o_get_size (desch);
  bitio_o_outp (desch, (long) n, 24, 0L);

/******* Save number of octets in the data-section */

  n = bitio_o_get_size (datah);
  bitio_o_outp (datah, (long) n, 24, 0L);

/******* close bitstreams */

  *datasec = (void *) bitio_o_close (datah, datasecl);
  *ddsec = (void *) bitio_o_close (desch, ddescl);
  printf ("data-descriptor-section: %d bytes, data section: %d bytes.\n", *ddescl, *datasecl);
  return 1;

/******* An error occoured during output to bitstreams */
err:
  if (desch != -1) bitio_o_close (desch, &n);
  if (datah != -1) bitio_o_close (datah, &n);
  return 0;
}

/*===========================================================================*/
static int bufr_out_descsec (descp, ndescs)

dd *descp;
int ndescs;

/* This function outputs the data-descriptors to the data-descriptor-section.

   parameters:
   DESCP:   Pointer to the data-descriptors.
   NDESCS:  Number of data-descriptors.

   The function returns 1 on success, 0 if there was an error outputing to the
   bitstream.
*/

{
  unsigned long l;
  int i;

/******* Append data descriptor to data descriptor section */

  for (i = 0; i < ndescs; i ++) {
    l = (unsigned long) descp->f;
    if (!bitio_o_append (desch, l, 2)) return 0;
    l = (unsigned long) descp->x;
    if (!bitio_o_append (desch, l, 6)) return 0;
    l = (unsigned long) descp->y;
    if (!bitio_o_append (desch, l, 8)) return 0;
    descp ++;
  }

  return 1;
}

/*===========================================================================*/
static int bufr_out_datasec (descs, start, end, vals, vali)

dd *descs;
int start;
int end;
varfl *vals;
unsigned *vali;

/* This function appends one data-value to the data-section and scales it
   according to the scale in reference-value of the data-descriptor DESCP.

   parameters:
   DESCS:   Pointer to the data-descriptors.
   START:   First data-descriptor for output.
   END:     Last data-descriptor for output.
   VALS:    Pointer to an array of values.
   VALI:    Index for the array VALS that identifies the values to be used. VALI
            is increased after data-output.

   The function returns 1 on success, 0 if there was an error outputing to the
   bitstreams.
*/

{
  int ret;

  ret = bufr_parse (descs, start, end, vals, vali, data_to_datastream);
  return ret;
}

/*===========================================================================*/
static int data_to_datastream (val, ind)

varfl val;
int ind;

/* This function outputs one data-value to the data-bitstream.

   parameters:
   VAL:    Data-value to be output.
   IND:    Index to the global array DES[] holding the description of
           known data-descriptors.

   The function returns 1 on success, 0 on a fault.
*/

{
  unsigned long l;
  int ret, wi;

  ret = 1;

  l = (unsigned long) (val * pow (10.0, (varfl) des[ind]->elseq.el.scale) 
                       - des[ind]->elseq.el.refval + 0.5);  /* + 0.5 to round to integer values */
  wi = des[ind]->elseq.el.dw + dw - 128;
  if (bitio_o_append (datah, l, wi) == -1) ret = 0;

/******* check if data width was large enough to hold data to be coded */

  if (l >> wi != 0) {
    printf ("WARNING: Tried to code %ld to %d bits !\n", l, wi);
    printf ("         Decoding will fail !\n");
  }

  return ret;
}

/*===========================================================================*/
int bufr_parse (descs, start, end, vals, vali, userfkt)

dd *descs;
int start;
int end;
varfl *vals;
unsigned *vali;
int (*userfkt) (P2(varfl val, int ind));
			     
/* This functions parses a descriptor/data-array and calls the user-function
   USERFKT for each data-value


   parameters:
   DESCS:   Pointer to the data-descriptors.
   START:   First data-descriptor for output.
   END:     Last data-descriptor for output.
   VALS:    Pointer to an array of values.
   VALI:    Index for the array VALS that identifies the values to be used. VALI
            is increased after data-output.
   USERFKT: User-function to be called for each data-value.

   The function returns 1 on success, 0 if there was an error outputing to the
   bitstreams.
*/
{
  int i, ok, ind, nrep, nd;
  varfl d;
  dd descr;

  /*printf ("BUFR_DATA_TO_DATASECT called with %2d%2d%4d\n", descp->f, descp->x, descp->y);*/

  
  /*printf ("bufr_out_descsec call:\n");
  for (i = start; i <= end; i ++) {
    printf ("%d %2d 3%d\n", (descs + i)->f, (descs + i)->x, (descs + i)->y);
  }*/

  ind = start;
  while (ind <= end) {
    memcpy (&descr, descs + ind, sizeof (dd));

/******* If data-descriptor is a sequence descriptor -> call this function 
         again for each entry in the sequence descriptor */

    ok = 0;
    for (i = 0; i < ndes; i ++) {
      if (des[i]->id == SEQDESC) {
        if (memcmp (&descr, &des[i]->elseq.seq.d, sizeof (dd)) == 0) {

/******* call this function again for each entry in the sequence-descriptor */

          ok = 1;
          if (!bufr_parse (des[i]->elseq.seq.del, 0, des[i]->elseq.seq.nel - 1,
                           vals, vali, userfkt)) return 0;
        }
      }
    }

/******* Append data to data section. At the beginning the value has to be 
         scaled according to scale- and reference-value of the data-descriptor */

/******* serach for the data-descriptor in the list of known descriptors */

    if (!ok) {
      for (i = 0; i < ndes; i ++) {
        if (des[i]->id == ELDESC) {
          if (memcmp (&descr, &des[i]->elseq.el.d, sizeof (dd)) == 0) {

/******* Store data in data-section-bitstream */

            d = *(vals + (*vali) ++);
            if (!(*userfkt) (d, i)) return 0;
            ok = 1;
            /*printf ("%d %2d %3d: %f\n", desc.f, desc.x, desc.y, d);*/
  
            break;
          }
        }
      }
    }

/******* Descriptor is a replication factor */

    if (!ok) {
      if (descr.f == 1) {
        nd   = descr.x;
        nrep = descr.y;

/******* if there is a delayed replication factor */

        if (nrep == 0) {
          nrep = (int) *(vals + *vali);
          if (!bufr_parse (descs, ind + 1, ind + 1, vals, vali, userfkt)) 
            return 0;
          ind ++;
        }

/******* do the replication now */

        for (i = 0; i < nrep; i ++) {
          if (!bufr_parse (descs, ind + 1, ind + nd, vals, vali, userfkt)) 
            return 0;
        }
        ind += nd;
        ok = 1;
      }
    }

/******* Descriptor found to modify the current data width */

    if (!ok) {
      if (descr.f == 2 && descr.x == 1) {
        if (descr.y == 0) dw = 128;
        else dw = descr.y;      /* valid until cancelled by 2 01 000 */
        ok = 1;
      }
    }

/******* DESCP is an invalid descriptor */

    if (!ok) {
      printf ("Unknown data descriptor found: F=%d, X=%d, Y=%d !\n", descr.f,
              descr.x, descr.y);
      return 0;
    }
    ind ++;
  }
  return 1;
}

/*===========================================================================*/
int bufr_read_msg (datasec, ddsec, datasecl, ddescl, descr, ndescs, vals, nvals)

void *datasec;
void *ddsec;
size_t datasecl;
size_t ddescl;
dd **descr;
int *ndescs;
varfl **vals;
size_t *nvals;

/* This function reads from a BUFR-data-section and data-descriptor section
   the values coded there. Memory for storing descriptor- and data-section is
   allocated by this function and has to be freed by the calling function.

   parameters:

   DATASEC:  Is where the data-section is stored.

   DDSEC:    Is where the data-descriptor-section is stored.

   DATASECL: Number of bytes of the data-section.

   DDESCL:   Number of bytes of the data-descriptor-section.

   DESCR:    Is where the data-descriptor are stored after reading them from
             the data-descriptor section. This memory area is allocated by this
             function and has to be freed by the calling function.

   NDESCS:   Number of data-descriptors found in the data-descriptor section.

   VALS:     Is where the data corresponding to the data-descriptors is stored.

   NVALS:    Number of values read from BUFR-message.

   The function-value is 1 if both sections were decoded successfuly, atherwise
   it is 0.
*/

{
  int ndesc, i, err, ret = 1;
  dd *d;
  unsigned long l;

/******* open two bitstreams, one for data-descriptos, one for data */

  desch = bitio_i_open (ddsec, ddescl);
  datah = bitio_i_open (datasec, datasecl);
  if (desch == -1 || datah == -1) goto error;

/******* skip trailing octets (56 bits) of the data-descriptor-section */

  for (i = 0; i < 56; i ++) bitio_i_input (desch, &l, 1);
  ddescl -= 56 / 8;

/******* skip trailing octets (32 bits) of the data-section */

  for (i = 0; i < 32; i ++) bitio_i_input (datah, &l, 1);
  datasecl -= 32 / 8;

/******* allocate memory and read data descriptors from bitstream */

  *ndescs = ndesc = (ddescl * 8) / 16;   /* number of data descriptors stored in the descriptor section */
  d = *descr = (dd *) malloc (ndesc * sizeof (dd));
  if (d == NULL) {
    printf ("Unable to allocate memory for data descriptors !\n");
    goto error;
  }
  for (i = 0; i < ndesc; i ++) {
    err = 0;
    err = err || !bitio_i_input (desch, &l, 2);
    d->f = (unsigned char) l;
    if (!err) err = err || !bitio_i_input (desch, &l, 6);
    d->x = (unsigned char) l;
    if (!err) err = err || !bitio_i_input (desch, &l, 8);
    d->y = (unsigned char) l;
    if (err) {
      printf ("Number of bits for descriptor-section exceeded !\n");
      ret = 0;
      goto error;
    }
    d ++;
  }

/******* Input data from data-section according to the data-descriptors */

  *vals = NULL;
  *nvals = 0;
  d = *descr;
  if (!bufr_data_from_datasect (d, 0, ndesc - 1, vals, nvals)) {
    printf ("Error reading data from data-section !\n");
    ret = 0;
  }
  ret = 1;

/******* An Error occoured during reading from bitstream */

error:
  if (desch != -1) bitio_i_close (desch);
  if (datah != -1) bitio_i_close (datah);
  return ret;
}

/*===========================================================================*/
static int bufr_data_from_datasect (descp, start, end, vals, nvals)

dd *descp;
int start;
int end;
varfl **vals;
size_t *nvals;

/* This function reads one data-value from the data-section and scales it 
   according to the data-descriptor.

   parameters:
   DESCP:   Pointer to the data-descriptor describing the data-value.
   START:   Index to the first descriptor for which data is to be read.
   END:     Index to the last descriptor for which data is to be read.
   VALS:    Pointer to the memory are holding the data-values. If VALS == NULL
            the memory has to be allocated by this function. This function has
            to "remember" the number of floats already stored in VALS and
            has to realloc it if necessary.
   NVALS:   Number of values in VALS.

   The function-value is 1 on success, 0 on a fault.
*/

#define MEMBLOCK   100   /* The memory-area holding data-values is allocated and
                            reallocated in blocks of MEMBLOCK floats. */

{
  int ok, i, ind, nd, nrep;
  unsigned long l;
  varfl v;
  dd descr;


  ind = start;
  while (ind <= end) {
    memcpy (&descr, descp + ind, sizeof (dd));

/******* If data-descriptor is a sequence descriptor -> call this function
         again for each entry in the sequence descriptor */

    ok = 0;
    for (i = 0; i < ndes; i ++) {
      if (des[i]->id == SEQDESC) {
        if (memcmp (&descr, &des[i]->elseq.seq.d, sizeof (dd)) == 0) {

/******* call this function again for each entry in the sequence-descriptor */

          ok = 1;
          if (!bufr_data_from_datasect (des[i]->elseq.seq.del, 0, 
                                        des[i]->elseq.seq.nel - 1, vals, 
                                        nvals)) return 0;
        }
      }
    }

/******* read data from data-section. First search for data-descripto in the list
         of known descriptors */

    if (!ok) {
      for (i = 0; i < ndes; i ++) {
        if (des[i]->id == ELDESC) {
          if (memcmp (&descr, &des[i]->elseq.el.d, sizeof (dd)) == 0) {

/******* read data from data-section-bitstream and scale it according to the
         data-descriptor */

            if (!bitio_i_input (datah, &l, des[i]->elseq.el.dw + dw - 128)) {
              printf ("Error reading data from bitstrem !\n");
              return 0;
            }
            v = ((varfl) l + des[i]->elseq.el.refval) / 
                pow (10.0, (varfl) des[i]->elseq.el.scale);
            if (!val_to_array (vals, v, nvals)) {
              printf ("Error storing data-value in the array !\n");
              return 0;
            }
            ok = 1;
            break;
          }
        }
      }
    }

/******* Descriptor is a replication factor */

    if (!ok) {
      if (descr.f == 1) {
        nd   = descr.x;
        nrep = descr.y;

/******* If there is a delayed replication factor */

        if (nrep == 0) {
          if (!bufr_data_from_datasect (descp, ind + 1, ind + 1, vals, nvals)) 
            return 0;
          nrep = (int) *(*vals + *nvals - 1);
          ind ++;
        }

/******* Do the replication now */

        for (i = 0; i < nrep; i ++) {
          if (!bufr_data_from_datasect (descp, ind + 1, ind + nd, vals, nvals)) 
            return 0;
        }
        ind += nd;
        ok = 1;
      }
    }

/******* Descriptor found to modify the current data width */

    if (!ok) {
      if (descr.f == 2 && descr.x == 1) {
        if (descr.y == 0) dw = 128;
        else dw = descr.y;      /* valid until cancelled by 2 01 000 */
        ok = 1;
      }
    }

/******* DESC is an invalid descriptor */

    if (!ok) {
      printf ("Unknown data descriptor found: F=%d, X=%d, Y=%d !\n", descr.f,
              descr.x, descr.y);
      return 0;
    }
    ind ++;
  }
  return 1;
}

/*===========================================================================*/
int val_to_array (vals, v, nvals)

varfl **vals;
varfl v;
size_t *nvals;

/* This function stores the value V to an array of floats VALS. The memory-
   block for VALS is allocated in this function and has to be freed by the
   calling function.
*/

{
  static unsigned int nv;         /* Number of values already read from bitstream */
  static unsigned int memsize;    /* Current size of memory-block holding data-values */
  varfl *d;

/******* Allocate memory if not yet done */

  if (*vals == NULL) {
    *vals = (varfl *) malloc (MEMBLOCK * sizeof (varfl));
    if (*vals == NULL) return 0;
    nv = 0;
    memsize = MEMBLOCK;
  }

/******* Check if memory block is large anough to hold new data */

  if (memsize == nv) {
    *vals = (varfl *) realloc (*vals, (memsize + MEMBLOCK) * sizeof (varfl));
    if (*vals == NULL) return 0;
    memsize += MEMBLOCK;
    if (memsize - 1 > (~(unsigned int) 0) / sizeof (varfl)) {
      printf ("VAL_TO_ARRAY failed in file %s, line %d\n", __FILE__, __LINE__);
      printf ("Try to define varfl as float in file desc.h \n");
      return 0;
    }
  }

/******* Add value to array */

  d = *vals;
  *(d + nv) = v;
  nv ++;
  *nvals = nv;
  return 1;
}

/* end of file */
