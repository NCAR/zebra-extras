/*
 * read_campbell_data.c
 *
 * Module for ingesting campbell data from file. File is created by
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

static int parse_campbell_data(char *data_file_name,
			       char *line,
			       long nfields,
			       float *fields);

static double Max_change[N_CAMPBELL_FIELDS] =
{
  5.0,     /* pres */
  5.0,     /* pres max */
  5.0,     /* pres min */
  10.0,    /* tdry */
  20.0,    /* rh */
  200.0,   /* wspd */
  360.0,   /* wdir */
  360.0,   /* wdir_sdev */
  100.0,   /* wspd_max */
  100.0,   /* wspd_min */
  100.0,   /* wspd_sdev */
  100.0,   /* rain */
  2000.0,  /* solrad */
  5.0,	   /* battery voltage */
};

/*****************************************************
 * read_campbell_data()
 *
 * returns 0 on success, 1 on failure
 */

int read_campbell_data(char *data_file_name,
		       long *file_mark_p,
		       long max_valid_data_age,
		       long search_sec,
		       date_time_t *data_time,
		       field_t *field_data)

{

  char *token;
  char line[MAX_LINE];

  int valid_data_found;
  int nyears, n_leapdays;
  int archive_rap = TRUE;
  
  long i, ifield;
  long data_age;
  long latest_valid_time;
  long julian_day,hr_min,hour,minute;
  float fields_in[N_CAMPBELL_FIELDS];
  float fields_merged[N_CAMPBELL_FIELDS];
  float fields_prev[N_CAMPBELL_FIELDS];
  float fields_prev_valid[N_CAMPBELL_FIELDS];

  struct stat file_stats;
  FILE *data_file;

#ifdef CDBUG
  fprintf(stderr, "\r\nFile : %s  ", data_file_name);
  fprintf(stderr, "file_mark: %ld\n\r", *file_mark_p);
  fprintf(stderr, " max valid data age: %ld\n\r", max_valid_data_age);
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

  init_fields((long) N_CAMPBELL_FIELDS, fields_merged,
	      fields_prev, fields_prev_valid);

  /*
   * look through file for data younger that the max_valid_data_age
   */
#ifdef CDBUG
    fprintf(stderr,"Latest valid time: %ld Max valid data age %ld\n\r",
	    latest_valid_time,max_valid_data_age);
#endif

  
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

    if (sscanf (line, "111,%ld,%ld,%ld",
		&data_time->year, &julian_day, &hr_min) != 3) {
      /* 
       * Some Campbell data files have had blank lines, i.e. missing data
       * for that minute.  This hack should jump right over them.
       */
#ifdef USE_ZEB
      IngestLog (EF_PROBLEM, "Error parsing date/time, file '%s'", 
		 data_file_name);
#else
      fprintf(stderr, "Error parsing date/time, file '%s'", 
	      data_file_name);
      perror (data_file_name);
#endif

      goto end_of_while;
    }

#ifdef CDBUG
    fprintf(stderr,"Line: %s\n\r",line);
#endif
    
    /* Perform time conversion, including Time Zone */

    hour = (int) floor((double) (hr_min/100.0));
    minute = hr_min - (hour*100);

    nyears = data_time->year - 1970;
    n_leapdays = (nyears + 1)/4;
    if (data_time->year >= 2000)
      n_leapdays--;

#ifdef CDBUG
    fprintf(stderr,"nyears: %d  Leap days: %d\n\r",nyears,n_leapdays);
#endif
    
    data_time->unix_time = 
      (((((((nyears * 365) + n_leapdays + (julian_day-1)) * 24) 
	  + hour) * 60) + minute) * 60);
    
    uconvert_from_utime(data_time);

    data_age = latest_valid_time - data_time->unix_time;

#ifdef CDBUG    
    fprintf(stderr,"Unix Time: %ld Data Age: %ld\n\r",data_time->unix_time,
	    data_age);
#endif

    if (data_age > 0) {
      
      /*
       * data is in the past relative to our search time
       */
      
      if (data_age < max_valid_data_age) {
	
	/* 
	 * Archive data realtime for RAP 
	 */

	if (archive_rap) {
	  archive_realtime(data_file_name,line);
	  archive_rap=FALSE;
	}
	
	/*
	 * data is young enough - merge with our fields so far.
	 * Latest data overrides previous data
	 */
	
	valid_data_found = TRUE;

#ifdef CDBUG    
	fprintf(stderr,"File Time: %ld %ld %ld\n\r", 
		data_time->year, julian_day, hr_min);
	fprintf(stderr,"Unix Time: %ld Data Age: %ld\n\r",data_time->unix_time,
		data_age);
	fprintf(stderr,"Converted Time: %ld %ld %ld %ld %ld\n\r",
		data_time->year,data_time->month,data_time->day,
		data_time->hour,data_time->min,data_time->sec);
	fprintf(stderr,"Line: %s\n\r",line);
	fprintf(stderr,"Valid Data: Parsing Campbell data\n\r");
#endif	
	
	parse_campbell_data(data_file_name,
			    line,
			    (long) N_CAMPBELL_FIELDS,
			    fields_in);
	
#ifdef CDBUG
	fprintf(stderr,"Start Merging Campbell data\n\r");
#endif
	merge_fields((long) N_CAMPBELL_FIELDS,
		     fields_in,
		     fields_merged,
		     fields_prev,
		     fields_prev_valid,
		     Max_change);
	
#ifdef CDBUG
	fprintf(stderr,"Finished Merging Campbell data\n\r");
#endif
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
  end_of_while:
 
  } /* while */
  
  field_data->pres = fields_merged[PRES_CB];
  field_data->pres_max = fields_merged[PRES_MAX_CB];
  field_data->pres_min = fields_merged[PRES_MIN_CB];
  field_data->cpres0 = DC_BADVAL;
  field_data->tdry = fields_merged[TDRY_CB];
  field_data->twet = DC_BADVAL;
  field_data->rh = fields_merged[RH_CB];
  field_data->dp = DC_BADVAL;
  field_data->mr = DC_BADVAL;
  field_data->pt = DC_BADVAL;
  field_data->ept = DC_BADVAL;
  field_data->wspd = fields_merged[WSPD_CB];
  field_data->wspd_max = fields_merged[WSPD_MAX_CB];
  field_data->wspd_min = fields_merged[WSPD_MIN_CB];
  field_data->wspd_sdev = fields_merged[WSPD_SDEV_CB];
  field_data->wdir = fields_merged[WDIR_CB];
  field_data->wdir_sdev = fields_merged[WDIR_SDEV_CB];
  field_data->u_wind = DC_BADVAL;
  field_data->v_wind = DC_BADVAL;
  field_data->raina01 = fields_merged[RAIN_CB];
  field_data->solrad = fields_merged[SOLRAD_CB];
  field_data->vis = DC_BADVAL;

#ifdef CDBUG
  fprintf(stderr,"File: %s\n\r",data_file_name);
    fprintf(stderr,"File Time: %ld %ld %ld\n\r", 
	    data_time->year, julian_day, hr_min);
    fprintf(stderr,"Unix Time: %ld Data Age: %ld\n\r",data_time->unix_time,
	    data_age);
    fprintf(stderr,"Converted Time: %ld %ld %ld %ld %ld\n\r",
	   data_time->year,data_time->month,data_time->day,
	   data_time->hour,data_time->min,data_time->sec);
    fprintf(stderr,"Line: %s\n\r",line);
    fprintf(stderr, "Field_data for Campbell ingest \n\r");
  fprintf(stderr, "P: %f %f %f Tdry %f RH %f wspd %f %f %f %f wdir %f %f srad %f\n", 
  fields_merged[PRES_CB],fields_merged[PRES_MAX_CB],fields_merged[PRES_MIN_CB],
	  fields_merged[TDRY_CB],fields_merged[RH_CB],fields_merged[WSPD_CB],
	  fields_merged[WSPD_MAX_CB],fields_merged[WSPD_MIN_CB],
	  fields_merged[WSPD_SDEV_CB],fields_merged[WDIR_CB],
	  fields_merged[WDIR_SDEV_CB],fields_merged[SOLRAD_CB]);
  
#endif

  return (0);

}

/*******************************************
 * parse_campbell_data()
 *
 * returns 0 on success, -1 on failure
 */

static int parse_campbell_data(char *data_file_name,
			       char *line,
			       long nfields,
			       float *fields_in)

{
 
  long i, ifield;
  char *token;

  /*
   * skip past date and Station ID
   */

  token = strtok(line, " ,\t\n\r");
  for (i = 0; i < 3; i++)
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
      perror (data_file_name);
#endif
      return (-1);
    }

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
    }
  } /* ifield */
  
  return (0);

}








