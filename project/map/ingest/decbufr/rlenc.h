/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          RLENC.H

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
function prototype for rlenc.c

AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

int rlenc (P5(char *infile, int nrows, int ncols, varfl **vals, size_t *nvals));
int rldec (P3(char *outfile, varfl *vals, size_t *nvals));

/* end of file */
