/*
 * impose_spatial_consistency.c
 *
 * Checks the data for validity by imposing a spatial consistency
 * check. If a value lies more than a given number of standard deviations
 * away from the mean, it is set to bad.
 *
 * Mike Dixon, RAP. NCAR, March 1994
 */

/*		Copyright (C) 1987-92 by UCAR
 *	University Corporation for Atmospheric Research
 *		   All rights reserved
 *
 * No part of this work covered by the copyrights herein may be reproduced
 * or used in any form or by any means -- graphic, electronic, or mechanical,
 * including photocopying, recording, taping, or information storage and
 * retrieval systems -- without permission of the copyright owner.
 * 
 * This software and any accompanying written materials are provided "as is"
 * without warranty of any kind.  UCAR expressly disclaims all warranties of
 * any kind, either express or implied, including but not limited to the
 * implied warranties of merchantibility and fitness for a particular purpose.
 * UCAR does not indemnify any infringement of copyright, patent, or trademark
 * through use or modification of this software.  UCAR does not provide 
 * maintenance or updates for its software.
 */

#include "aws_ingest.h"

/*************************************************
 * impose_spatial_consistency()
 */

void impose_spatial_consistency (long nfields,
				 long nstations,
				 float *field_data)

{

  /*
   * max_sdev lists the max allowable number of std dev away from
   * the mean. If negative, the check is not made
   */
  
  static double max_sdev[NFIELDS] =
    {
      5.0,   /* pres */
      5.0,   /* pres_max */
      5.0,   /* pres_min */
      3.0,   /* cpres0 */
      3.0,   /* tdry */
      3.0,   /* twet */
      3.0,   /* rh */
      3.0,   /* dp */
      3.0,   /* mr */
      3.0,   /* pt */
      3.0,   /* ept */
      -1.0   /* wspd */
      -1.0   /* wspd_max */
      -1.0   /* wspd_min */
      -1.0   /* wspd_sdev */
      -1.0   /* wdir */
      -1.0   /* wdir_sdev */
      -1.0,  /* u_wind */
      -1.0,  /* v_wind */
      -1.0,  /* raina01 */
      -1.0,  /* solrad */
      -1.0   /* vis */
    };

  long ifield, istation;

  float *fd;
  float val;

  double min_val, max_val;
  double mean;
  double sdev;
  double sumx, sumx2;
  double nvalid;
  double *maxsd;

  maxsd = max_sdev;
  
  /*
   * loop through fields
   */

  for (ifield = 0; ifield < nfields; ifield++) {

    if (*maxsd > 0) {

      /*
       * maxsd is pos, so apply the check
       */

      /*
       * compute mean and sdev
       */

      nvalid = 0.0;
      sumx = 0.0;
      sumx2 = 0.0;
      fd = field_data + ifield * nstations;

      for (istation = 0; istation < nstations; istation++) {

	val = *fd;
	
	if (val > DC_BADVAL) {
	  sumx += val;
	  sumx2 += val * val;
	  nvalid++;
	} /* if (val > DC_BADVAL)*/
	
	fd++;
	
      } /* istation */

      if (nvalid > 4) {

	/*
	 * there are sufficient stations for the analysis
	 * so proceed
	 */
	
	mean = sumx / nvalid;
	sdev = (sqrt(fabs(nvalid * sumx2 - sumx * sumx)) /
		(nvalid - 1.0));

	min_val = mean - sdev * *maxsd;
	max_val = mean + sdev * *maxsd;

	fd = field_data + ifield * nstations;

	for (istation = 0; istation < nstations; istation++) {

	  val = *fd;

	  if (val > DC_BADVAL) {

	    /*
	     * set to bad val if outside limits
	     */

	    if (val < min_val ||
		val > max_val) {
#ifdef CDBUG
	fprintf(stderr,"Spacial consistancy bad val: %f\n\r",val);
#endif
	      *fd = DC_BADVAL;
	    }

	  } /* if (val > DC_BADVAL)*/
	  
	  fd++;
	  
	} /* istation */
	
      } /* if (nvalid > 4) */
      
    } /* if (*maxsd > 0) */

    maxsd++;

  } /* ifield */
				      
}
