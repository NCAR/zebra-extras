/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          BUFR.H

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
Includefile for BUFR.C. More details can be found there.

AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

/*===========================================================================*/
/* protypes of functions in BUFR.C                                           */
/*===========================================================================*/

int bufr_create_msg (P7(dd *descs, int ndescs, varfl *vals, void **datasec, 
                        void **ddsec, size_t *datasecl, size_t *ddescl));
int bufr_read_msg (P8(void *datasec, void *ddsec, size_t datasecl, size_t ddescl,
                      dd **desc, int *ndescs, varfl **vals, size_t *nvals));
int val_to_array (P3(varfl **vals, varfl v, size_t *nvals));
int bufr_parse (P6(dd *descs, int start, int end, varfl *vals, unsigned *vali,
                   int (*userfkt) (P2(varfl val, int ind))));

/* end of file */
