/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          BITIO.C

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
The functions in this file can be used for input and output to/from a bit-
stream as needed for BUFR-messages. Data is stored on/read from a bitstream
as follows: For example if you wan to store a 12 bit-value VAL on a bit-stream,
consisting of a character-array C, the bits are assigned (bit 0 is the least
segnificant bis).

VAL bit 00 -> C[0] bit 00
VAL bit 01 -> C[0] bit 01
VAL bit 02 -> C[0] bit 02
VAL bit 03 -> C[0] bit 03
VAL bit 04 -> C[0] bit 04
VAL bit 05 -> C[0] bit 05
VAL bit 06 -> C[0] bit 06
VAL bit 07 -> C[1] bit 07
VAL bit 08 -> C[1] bit 00
VAL bit 09 -> C[1] bit 01
VAL bit 10 -> C[1] bit 02
VAL bit 11 -> C[1] bit 03

if you append another 2-bit value VAL1 to the stream:

VAL bit 00 -> C[1] bit 04
VAL bit 01 -> C[1] bit 05

Functions for output of data to a bit-stream are named bitio_o_*, those for 
inputing from a bitstream bitio_i_*.

Output to a bit-stream must be as follows:

h = bitio_o_open ();            open a bitstrem, handle H is returned to identify
                                for subsequent calls.
bitio_o_append (h, val, nbits); Append VAL to the bitstream.
bitio_o_close (h, nbytes);      close bitstream.


Input from a bit-stream must be as follows:

h = bitio_i_open ();            open a bitstream for input
bitio_i_read ();                read a value from the bitstream
bitio_i_close ();               close the bitstream

More details can be found at the description of the functions. Note that the
buffer holding the bitstream is organized as an array of characters. So the
functions are independent from the computer-architecture (byte-swapping).

Include the file BITIO.H to get the function-prototypes for the function in
this file.

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
#include "bitio.h"

/*===========================================================================*/
/* functions for bit-io follow:                                   */
/*===========================================================================*/

/******* internal data and definitions needed to hold the bitstreams: */

#define MAXIOSTR    10                 /* Max. number of streams that can be 
                                          open simultaneously */
#define INCSIZE     1000               /* for holding a bitstream the open-
                                          function allocates INCSIZE bytes of
                                          memory. When outputing to the 
                                          bitstream and the size of the
                                          memoryblocks exceeds subsequent 
                                          blocks with INCSIZE bytes are
                                          allocated to hold the bitstream. This
                                          is done by a realloc of the buffer. */

typedef struct bitio_stream {          /* structure that defines a bitstrem */
  int used;                            /* identifier if the bitstream is used */
  char *buf;                           /* buffer holding the bitstream */
  long nbits;                          /* currend size of bitstream (counted 
                                          in bits !) */
  size_t size;                         /* current size of allocated memory for
                                          holding the bitstream. */
} bitio_stream;

bitio_stream bios[MAXIOSTR];          /* Data describing MAXIOSTR bitstreams */
int first = 1;                         /* to indicate the first call to one of these functions */

/*===========================================================================*/
int bitio_i_open (buf, size)

void *buf;
size_t size;

/* This function opens a bitstream for input.

   parameters:
   BUF:    Buffer to be used for input
   SIZE:   Size of buffer.

   the function returns a handle by which the bitstream can be identified for
   all subsequent actions or -1 if the maximum number of opened bitstreams
   exceeds.
*/

{
  int i, handle;

/******* On the first call mark all bitstreams as unused */

  if (first) {
    for (i = 0; i < MAXIOSTR; i ++) bios[i].used = 0;
    first = 0;
  }

/******* search for an unused stream. */

  for (handle = 0; handle < MAXIOSTR; handle ++) {
    if (!bios[handle].used) goto found;
  }
  return -1;

/******* unused bitstream found -> initialize bitstream-data */

found:
  bios[handle].used = 1;
  bios[handle].buf = (char *) buf;
  bios[handle].size = size;
  bios[handle].nbits = 0;                 /* Holds the current bitposition */
  return handle;
}

/*===========================================================================*/
int bitio_i_input (handle, val, nbits)

int handle;
unsigned long *val; 
int nbits;

/* This functions read a value from a bitstream.

   parameters:
   HANDLE:   Identifies the bitstream.
   VAL:      Is where the input-value is stored.
   NBITS:    Number of bits the value consists of.

   The function returns 1 on success or 0 on a fault (number of bytes in the
   bitstream exceeded.
*/

{
  int i, bit;
  size_t byte;
  unsigned long l, bitval;
  char *pc;

  
  l = 0;
  for (i = nbits - 1; i >= 0; i --) {

/****** calculate bit- and byte-number for input and check if bytenumber is
        in a valid range */

    byte = (int) (bios[handle].nbits / 8);
    bit  = (int) (bios[handle].nbits % 8);
    bit = 7 - bit;
    if (byte >= bios[handle].size) return 1;

/******* get bit-value from input-stream */

    pc = bios[handle].buf + byte;
    bitval = (unsigned long) ((*pc >> bit) & 1);

/******* Set a 1-bit in the data value, 0-bits need not to be set, as L has
         been initialized to 0 */

    if (bitval) {
      l |= (bitval << i);
    }
    bios[handle].nbits ++;
  }
  *val = l;
  return 1;
}

/*===========================================================================*/
void bitio_i_close (handle)

int handle;

/* Closes an bitstream that was opened for input */

{
  bios[handle].used = 0;
}

/*===========================================================================*/
int bitio_o_open (P0)

/* This function opens a bitstream for output. The return-vaule is a handle by
   which the bit-stream can be identified for all subesquent actions or -1
   if there is no unused bitstream available.
*/

{
  int i, handle;

/******* On the first call mark all bitstreams as unused */

  if (first) {
    for (i = 0; i < MAXIOSTR; i ++) bios[i].used = 0;
    first = 0;
  }

/******* search for an unused stream. */

  for (handle = 0; handle < MAXIOSTR; handle ++) {
    if (!bios[handle].used) goto found;
  }
  return -1;

/******* unused bitstream found -> initalize it and allocate memory for it */

found:
  bios[handle].buf = malloc (INCSIZE);
  if (bios[handle].buf == NULL) return -1;
  bios[handle].used = 1;
  bios[handle].nbits = 0;
  bios[handle].size = INCSIZE;

  return handle;
}

/*===========================================================================*/
long bitio_o_append (handle, val, nbits)

int handle;
unsigned long val;
int nbits;

/* This function appends a value to a bitstream.

   parameters:

   HANDLE:  Indicates the bitstream for appending.
   VAL:     Value to be output.
   NBITS:   Number of bits of VAL to be output to the stream. Note that NBITS
            muste be less that sizeof (LONG)

   The return-value is the bit-position of the value in the bit-stream, or -1 
   on a fault.
*/

{
/******* Check if bitstream is allready initialized and number of bits does not
         exceed sizeof (unsigned long). */

  assert (bios[handle].used);
  assert (sizeof (unsigned long) * 8 >= nbits);

/******* check if there is enough memory to store the new value. Reallocate
         the memory-block if not */

  if ((bios[handle].nbits + nbits) / 8 + 1 > (long) bios[handle].size) {
    bios[handle].buf = realloc (bios[handle].buf, bios[handle].size + INCSIZE);
    if (bios[handle].buf == NULL) return 0;
    bios[handle].size += INCSIZE;
  }

/******* output data to bitstream */

  bitio_o_outp (handle, val, nbits, bios[handle].nbits);
  bios[handle].nbits += nbits;

  return bios[handle].nbits;
}

/*===========================================================================*/
void bitio_o_outp (handle, val, nbits, bitpos)

int handle;
unsigned long val;
int nbits;
long bitpos;

/* This function outputs a value to a specified position of a bitstream

   parameters:

   HANDLE:  Indicates the bitstream for output.
   VAL:     Value to be output.
   NBITS:   Number of bits of VAL to be output to the stream. Note that NBITS
            must be less then sizeof (LONG)
   BITPOS:  bitposition of the value in the bitstream.
*/

{
  int i, bit, bitval;
  size_t byte;
  char *pc, c;

/******* Check if bitstream is allready initialized and number of bits does not
         exceed sizeof (unsigned long). */

  assert (bios[handle].used);
  assert (sizeof (unsigned long) * 8 >= nbits);

  for (i = nbits - 1; i >= 0; i --) {

/******* Get bit-value */

    bitval = (int) (val >> i) & 1;

/******* calculate bit- and byte-number for output */

    /*byte = (int) (bitpos / 8);
    bit  = (int) (bitpos % 8);*/
    byte = (int) (bitpos / 8);
    bit  = (int) (bitpos % 8);
    bit  = 7 - bit;

/******* set bit-value to output stream */

    pc = bios[handle].buf + byte;
    if (bitval) {
      c = (char) (1 << bit);
      *pc |= c;
    }
    else {
      c = (char) (1 << bit);
      c ^= 0xff;
      *pc &= c;
    }
    bitpos ++;
  }
}

/*===========================================================================*/
size_t bitio_o_get_size (handle)

int handle;

/* This function returns the size of an output-bitstream (number of bytes) */

{
  if (!bios[handle].used) return 0;

  return (size_t) ((bios[handle].nbits - 1) / 8 + 1);
}


/*===========================================================================*/
void *bitio_o_close (handle, nbytes)

int handle;
size_t *nbytes;

/* This function closes a output-bitstream identified by HANDLE and returns
   a pointer to the memory-area holding the bit-stream.

   parameters:
   HANDLE:  Bit-stream-handle
   NBYTES:  number of bytes in the bitstream.

   The funcion returns a pointer to the memory-area holding the bit-stream or
   NULL if an invalid handle was specified. The memory area must be freed by
   the calling function.
*/

{

  if (!bios[handle].used) return NULL;

/******* Fill up the last byte with 0-bits */

  while (bios[handle].nbits % 8 != 0) bitio_o_append (handle, 0, 1);

  *nbytes = (size_t) ((bios[handle].nbits - 1) / 8 + 1);
  bios[handle].used = 0;
  return (void *) bios[handle].buf;
}

/* end of file */
