/*
 * read_data.c
 *
 * Main data module for aws_ingest.c
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
 * read_data()
 *
 * Read and store the data
 */

void read_data (long search_min,
		long *file_mark,
		long max_valid_data_age,
		PlatformId plat,
		char *aws_data_dir,
		long nfields,
		FieldId *fid,
		long nstations,
		station_t *stations,
		Location *loc,
		PlatformId *pid)

{
  
  char data_file_name[256];
  char message[MAX_LINE];
  char line[MAX_LINE];
  char *token;

  long year, month, day, hour, min, sec;
  long i, ifield, istation, id;
  long npoints;
  long search_sec;
  long data_sec;
  
  float *data, *dp, *flp;

  date_time_t data_time;
  station_t *station;
  field_t field_data;
  ZebTime t;
  DataChunk *dc;
 
  data_sec = search_min * 60;
  search_sec = data_sec + SECS_AHEAD_GMT;
  
  /*
   * Create and initialize the data chunk
   */

#ifdef USE_ZEB
  dc = dc_CreateDC (DCC_IRGrid);
  dc->dc_Platform = plat;
  dc_IRSetup (dc, nstations, pid, loc, nfields, fid);
  dc_SetBadval (dc, DC_BADVAL);
#endif

  /*
   * Allocate the data array, and initialize to bad data val
   */

  npoints = nfields * nstations;
  
  data = (float *) malloc (npoints * sizeof (float));

  dp = data;
  for (i = 0; i < npoints; i++) {
    *dp = DC_BADVAL;
    dp++;
  }

  /*
   * Loop through the stations.
   */

  station = stations;

  for (istation = 0; istation < nstations; istation++) {
    
    sprintf (data_file_name, "%s/sta%s.dat",
	     aws_data_dir, station->label);

    if (station->type == ro) {

      if (read_ro_data(data_file_name,
		       file_mark + istation,
		       max_valid_data_age,
		       search_sec,
		       &data_time,
		       &field_data))
	goto end_of_sta_loop;

    } else {

      if (read_campbell_data(data_file_name,
			     file_mark + istation,
			     max_valid_data_age,
			     search_sec,
			     &data_time,
			     &field_data))
	goto end_of_sta_loop;

    }

    /*
     * Calculate u_wind and v_wind if possible
     */

    if (field_data.wspd != DC_BADVAL && field_data.wdir != DC_BADVAL) {
      field_data.u_wind =
	field_data.wspd * cos (DEG_TO_RAD (270.0 - field_data.wdir));
      field_data.v_wind =
	field_data.wspd * sin (DEG_TO_RAD (270.0 - field_data.wdir));
    } else {
      field_data.u_wind = DC_BADVAL;
      field_data.v_wind = DC_BADVAL;
    }

    /*
     * compute derived fields
     */
    
    if (station->type == campbell) {

      field_data.dp = pr_rhdp((double) field_data.tdry, (double) field_data.rh);

      field_data.twet = pr_twet((double) field_data.pres,
				(double) field_data.tdry,
				(double) field_data.dp);

    } /* if (station->type == campbell) */

    field_data.mr = pr_mixr((double) field_data.dp,
			    (double) field_data.pres);

    field_data.pt = pr_thta((double) field_data.tdry,
			    (double) field_data.pres);

    field_data.ept = pr_thte((double) field_data.pres,
			     (double) field_data.tdry,
			     (double) field_data.dp);

    /*
     * quality control - individual station check
     */

    impose_limits(nfields, &field_data);

#ifdef DBUG
    fprintf(stderr,
	    "%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g%s%g\n",
	    " pres: ", field_data.pres,
	    " pres_max: ", field_data.pres_max,
	    " pres_min: ", field_data.pres_min,
	    " cpres0: ", field_data.cpres0,
	    " tdry: ", field_data.tdry,
	    " twet: ", field_data.twet,
	    " rh: ", field_data.rh,
	    " dp: ", field_data.dp,
	    " mr: ", field_data.mr,
	    " pt: ", field_data.pt,
	    " ept: ", field_data.ept,
	    " wspd: ", field_data.wspd,
	    " wspd_max: ", field_data.wspd_max,
	    " wspd_min: ", field_data.wspd_min,
	    " wspd_sdev: ", field_data.wspd_sdev,
	    " wdir: ", field_data.wdir,
	    " wdir_sdev: ", field_data.wdir_sdev,
	    " u_wind: ", field_data.u_wind,
	    " v_wind: ", field_data.v_wind,
	    " raina01: ", field_data.raina01,
	    " solrad: ", field_data.solrad,
	    " vis: ", field_data.vis);
#endif
	    
    /*
     * Get the unix time
     */

    t.zt_Sec = data_sec;
    t.zt_MicroSec = 0;

    /*
     * Get and store the fields.
     */

    dp = data + istation;
    flp = (float *) &field_data;
    
    for (ifield = 0; ifield < nfields; ifield++) {

      *dp = *flp;
      dp += nstations;
      flp++;

    }

 end_of_sta_loop:
#ifdef CDBUG
    fprintf("Incrementing station %d \n\r",istation);
#endif    
    station++;

  } /* istation */

  /*
   * quality control - interstation check for values which depart from the
   * mean significantly
   */

  impose_spatial_consistency(nfields, nstations, data);

  /*
   * Put the data into the data chunk and store the data chunk
   */

  for (ifield = 0; ifield < nfields; ifield++) {
#ifdef USE_ZEB
    dc_IRAddGrid (dc, &t, 0, fid[ifield], data + nstations * ifield);
#endif
  }

#ifdef USE_ZEB
  sprintf(message, "Stored %ld fields, %ld stations", nfields, nstations);
  IngestLog (EF_INFO, message);
  ds_Store (dc, FALSE, 0, 0);
#endif

  /*
   * Clean up
   */
#ifdef USE_ZEB
  dc_DestroyDC (dc);
#endif

  free (data);

}

