/*
 * impose_limits.c
 *
 * Checks the data for validity by imposing limits - if the
 * data lies outside these limits, it is set to the bad val
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
 * impose_limits()
 */

void impose_limits (long nfields,
		    field_t *field_data)

{

  static double min_val[NFIELDS] =
    {
      900.0,   /* pres */
      900.0,   /* pres_max */
      900.0,   /* pres_min */
      900.0,   /* cpres0 */
      -60.0,   /* tdry */
      0.0,     /* twet */
      0.0,     /* rh */
      -60.0,   /* dp */
      0.0,     /* mr */
      0.0,     /* pt */
      0.0,     /* ept */
      0.0,     /* wspd */
      0.0,     /* wspd_max */
      0.0,     /* wspd_min */
      0.0,     /* wspd_sdev */
      0.0,     /* wdir */
      0.0,     /* wdir_sdev */
      -100.0,  /* u_wind */
      -100.0,  /* v_wind */
      0.0,     /* raina01 */
      0.0,     /* solrad */
      0.0      /* vis */
    };

  static double max_val[NFIELDS] =
    {
      1100.0,   /* pres */
      1100.0,   /* pres_max */
      1100.0,   /* pres_min */
      1100.0,   /* cpres0 */
      60.0,     /* tdry */
      60.0,     /* twet */
      120.0,    /* rh */
      60.0,     /* dp */
      100.0,    /* mr */
      500.0,    /* pt */
      500.0,    /* ept */
      100.0,    /* wspd */
      100.0,    /* wspd_max */
      100.0,    /* wspd_min */
      100.0,    /* wspd_sdev */
      360.0,    /* wdir */
      360.0,    /* wdir_sdev */
      100.0,    /* u_wind */
      100.0,    /* v_wind */
      20.0,     /* raina01 */
      2500.0,   /* solrad */
      50000.0   /* vis */
    };

  double *minv, *maxv;
  float *fd;
  long ifield;

  minv = min_val;
  maxv = max_val;
  fd = (float *) field_data;

  for (ifield = 0; ifield < nfields; ifield++) {

    if (*fd < *minv)
      *fd = DC_BADVAL;

    if (*fd > *maxv)
      *fd = DC_BADVAL;

    fd++;
    minv++;
    maxv++;

  } /* ifield */
				      
}
