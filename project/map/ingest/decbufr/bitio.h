/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          BITIO.H

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
Includefile for BITIO.C. More details can be found there.

AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

/*===========================================================================*/
/* function prototypes                                                       */
/*===========================================================================*/

int bitio_i_open (P2(void *buf, size_t size));
int bitio_i_input (P3(int handle, unsigned long *val, int nbits));
size_t bitio_o_get_size (P1(int handle));
void bitio_i_close (P1(int handle));
int bitio_o_open (P0);
long bitio_o_append (P3(int handle, unsigned long val, int nbits));
void bitio_o_outp (P4(int handle, unsigned long val, int nbits, long bitpos));
void *bitio_o_close (P2(int handle, size_t *nbytes));

/* end of file */

