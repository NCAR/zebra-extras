/*
 * read_ro_data.c
 *
 * Module for ingesting ro data from file. File is created by
 * ro_aws_ingest.
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

static int parse_ro_data(char *data_file_name,
			 char *line,
			 long nfields,
			 float *fields_in);

static double Max_change[N_RO_FIELDS] =
{
  360.0,   /* wdir */
  200.0,   /* wspd */
  100.0,   /* wspd_max */
  10.0,    /* tdry */
  10.0,    /* twet */
  10.0,    /* dp */
  20.0,    /* rh */
  5.0,     /* pres */
  5.0,     /* cpres0 */
  20.0,    /* raina01 */
  5000.0,  /* vis */
  360.0,   /* wdir_sdev */
  100.0,   /* wspd_sdev */
  2000.0   /* solrad */
};

#define WDIR_RO 0
#define WSPD_RO 1
#define WSPD_MAX_RO 2
#define TDRY_RO 3
#define TWET_RO 4
#define DP_RO 5
#define RH_RO 6
#define PRES_RO 7
#define CPRES0_RO 8
#define RAINA01_RO 9
#define VIS_RO 10
#define WDIR_SDEV_RO 11
#define WSPD_SDEV_RO 12
#define SOLRAD_RO 13

/*****************************************************
 * read_ro_data()
 *
 * returns 0 on success, 1 on failure
 */

int read_ro_data(char *data_file_name,
		 long *file_mark_p,
		 long max_valid_data_age,
		 long search_sec,
		 date_time_t *data_time,
		 field_t *field_data)

{

  char *token;
  char line[MAX_LINE];

  int valid_data_found;

  long i, ifield;
  long data_age;
  long latest_valid_time;
  int archive_rap=TRUE;
  

  float fields_in[N_RO_FIELDS];
  float fields_merged[N_RO_FIELDS];
  float fields_prev[N_RO_FIELDS];
  float fields_prev_valid[N_RO_FIELDS];

  struct stat file_stats;
  FILE *data_file;

#ifdef DBUG
  fprintf(stderr, "File : %s\n", data_file_name);
  fprintf(stderr, "file_mark: %ld\n", *file_mark_p);
#endif
#ifdef CDBUG
/*  fprintf(stderr, "\r\nFile : %s  ", data_file_name);
  fprintf(stderr, "file_mark: %ld\n\r", *file_mark_p);
  fprintf(stderr, " max valid data age: %ld\n\r",max_valid_data_age);
*/  
#endif

  /*
   * set latest_valid_time to half minute past search sec
   */

  latest_valid_time = search_sec + 31;

  /*
   * get file stats
   */

  if (stat(data_file_name, &file_stats)) {
    return (-1);
  }

  /*
   * if the file size is less than the previously returned file mark,
   * set mark to start of file
   */

  if (file_stats.st_size < *file_mark_p)
    *file_mark_p = 0;

  /*
   * open file
   */

  if ((data_file = fopen (data_file_name, "r")) == NULL) {
    return (-1);
  }
  
  /*
   * seek to file mark set on previous read of this file
   */
  
  fseek(data_file, *file_mark_p, 0);

  /*
   * initialize
   */

  valid_data_found = FALSE;

  init_fields((long) N_RO_FIELDS, fields_merged,
	      fields_prev, fields_prev_valid);
  
  /*
   * look through file for data younger that the max_valid_data_age
   */

  while (!feof(data_file)) {

    /*
     * if we have not yet reached the valid data, read the file
     * posn. Once young data is reached, this is not changed so
     * that the valid data will be read in again on the following ingest cycle
     */

    if (!valid_data_found)
      *file_mark_p = ftell(data_file);
    
    if (fgets(line, MAX_LINE, data_file) == NULL) {
      if (valid_data_found) {
	fclose(data_file);
	break;
      } else {
	fclose(data_file);
	return (-1);
      }
    }

    if (sscanf (line, "%ld %ld %ld %ld %ld %ld",
		&data_time->year, &data_time->month, &data_time->day,
		&data_time->hour, &data_time->min, &data_time->sec) != 6) {
      continue;
    }
    
    uconvert_to_utime(data_time);
    data_age = latest_valid_time - data_time->unix_time;

    if (data_age > 0) {


      /*
       * data is in the past relative to our search time
       */

      if (data_age < max_valid_data_age) {

      /* 
       * Archive data realtime for RAP 
       *   since data gets reread each time, only archive oldest
       *   valid data
       */
        if (archive_rap ) { 
	  archive_realtime(data_file_name,line);
	  archive_rap = FALSE;
        }
	
	/*
	 * data is young enough - merge with our fields so far.
	 * Latest data overrides previous data
	 */
	
	valid_data_found = TRUE;

	parse_ro_data(data_file_name,
		      line,
		      (long) N_RO_FIELDS,
		      fields_in);

	merge_fields((long) N_RO_FIELDS,
		     fields_in,
		     fields_merged,
		     fields_prev,
		     fields_prev_valid,
		     Max_change);
	
      } /* if (data_age < max_valid_data_age) */

      if (feof(data_file)) {
	  fclose(data_file);
	  break;
      }

    } else {

      /*
       * we have gone beyond our search point
       */

      if (valid_data_found) {

 	fclose(data_file);
	break;

      } else {

	/*
	 * move beyond this point in case there is bad data fouling
	 * things up
	 */

	*file_mark_p = ftell(data_file);

	fclose(data_file);
	return (-1);

      }
      
    } /* if (data_age > 0) */

  } /* while */
  
  field_data->pres = fields_merged[PRES_RO];
  field_data->pres_max = DC_BADVAL;
  field_data->pres_min = DC_BADVAL;
  field_data->cpres0 = fields_merged[CPRES0_RO];
  field_data->tdry = fields_merged[TDRY_RO];
  field_data->twet = fields_merged[TWET_RO];
  field_data->rh = fields_merged[RH_RO];
  field_data->dp = fields_merged[DP_RO];
  field_data->mr = DC_BADVAL;
  field_data->pt = DC_BADVAL;
  field_data->ept = DC_BADVAL;
  field_data->wspd = fields_merged[WSPD_RO];
  field_data->wspd_max = fields_merged[WSPD_MAX_RO];
  field_data->wspd_min = DC_BADVAL;
  field_data->wspd_sdev = fields_merged[WSPD_SDEV_RO];
  field_data->wdir = fields_merged[WDIR_RO];
  field_data->wdir_sdev = fields_merged[WDIR_SDEV_RO];
  field_data->u_wind = DC_BADVAL;
  field_data->v_wind = DC_BADVAL;
  field_data->raina01 = fields_merged[RAINA01_RO];
  field_data->solrad = fields_merged[SOLRAD_RO];
  field_data->vis = fields_merged[VIS_RO];

#ifdef CDBUG
/*    fprintf(stderr,"Unix Time: %ld Data Age: %ld\n\r",data_time->unix_time,
	    data_age);
    fprintf(stderr,"Converted Time: %ld %ld %ld %ld %ld\n\r",
	   data_time->year,data_time->month,data_time->day,
	   data_time->hour,data_time->min,data_time->sec);
    fprintf(stderr,"Line: %s\n\r",line);
    fprintf(stderr, "Field_data for RO ingest \n\r");
  fprintf(stderr, "P: %f %f T %f %f RH %f dp %f wspd %f %f %f wdir %f %f rn %f srad %f\n\r", 
  fields_merged[PRES_RO],fields_merged[CPRES0_RO],fields_merged[TDRY_RO],
	  fields_merged[TWET_RO],fields_merged[RH_RO],fields_merged[DP_RO],
	  fields_merged[WSPD_RO],fields_merged[WSPD_MAX_RO],
	  fields_merged[WSPD_SDEV_RO],fields_merged[WDIR_RO],
	  fields_merged[WDIR_SDEV_RO],fields_merged[RAINA01_RO],
          fields_merged[SOLRAD_RO]);
*/  
#endif

  return (0);

}

/*******************************************
 * parse_ro_data()
 *
 * returns 0 on success, -1 on failure
 */

static int parse_ro_data(char *data_file_name,
			 char *line,
			 long nfields,
			 float *fields_in)

{
 
  long i;
  long ifield;
  char *token;

  /*
   * skip past date
   */

  token = strtok(line, " ,\t\n\r");
  for (i = 0; i < 5; i++)
    token = strtok((char *) NULL, " ,\t\n\r");
  
  /*
   * Get the fields
   */	
  
  for (ifield = 0; ifield < nfields; ifield++) {
    
    token = strtok((char *) NULL, " ,\t\n\r");
    
    if (token == NULL) {

#ifdef USE_ZEB
      IngestLog (EF_PROBLEM, "Error parsing data, file '%s'", 
		 data_file_name);
#else
      fprintf(stderr, "Error parsing data, file '%s'", 
	      data_file_name);
      perror(data_file_name);
#endif      
      return (-1);
      
    }  /* if (token == NULL) */

    if (sscanf (token, "%f", &fields_in[ifield]) != 1) {

#ifdef USE_ZEB
      IngestLog (EF_PROBLEM, "AWS file %s : short data line.",
		 data_file_name);
#else
      fprintf(stderr, "AWS file %s : short data line.",
	      data_file_name);
      perror (data_file_name);
#endif

      return (-1);

    } /* if (sscanf ... */

  } /* ifield */
  
  return (0);

}

