/*
 * Ingest AWS ASCII data into the system.
 *
 * Usage:
 *	aws_ingest <aws_file> <platform>
 *
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

/*
 * Hong Kong field project. LANTEX.
 *
 * Mike Dixon, March 1994
 *
 * The data for this program comes from two sets of sensors - the AWS stations
 * of the Royal Observatory (RO) and the Campbell stations on Lantau.
 *
 * The RO AWS data is captured from a serial line, using program ro_aws_ingest.
 * This program writes line-oriented ASCII files which are then read in by
 * this program.
 *
 * The Campbell data is captured by the Campbell PC208 software, running
 * on an IBM compatible PC. The PC disk is crossmounted, so that the
 * files appear on the unix disk in the ingest data directory.
 *
 * The fields in the two file types are different, and they are merged into
 * a common field format for Zeb. Quality control is implemented.
 *
 * It is assumed that the data in the files is sampled at a rate of
 * once per minute, i.e. there is a data line for each minute.
 *
 * The ingest works as follows:
 *
 * Retrieval and ingest occurs in batches, one batch per retrieval period.
 * This is done so that retrieval may be performed less frequently than
 * sampling, i.e. at less than once per minute.
 *
 * Retrieval is carried out from the previous retreival time to the current
 * time less a 'retrieval delay'. The delay in imposed because some of the stations
 * will take a while to report. The Campbell stations can only be polled every
 * 5 to 10 minutes, whereas the data is taken every minute. Therefore the
 * delay is needed to make sure the data has arrived by the retrieval time.
 *
 * The time at which the previous retrieval was performed is stored in a history file.
 * This time is read at the start of each retrieval period.
 *
 */

# include <copyright.h>
# include "aws_ingest.h"
 
/* 
 * global variable  - a hack for realtime archiving
 */

long archive_time;
int archive_realtime_rap;


/*
 * prototypes
 */

static int read_history(char *history_file_path,
			long *history_time);

static int read_params(char *params_file_path,
		       char **aws_data_dir_p,
		       char **history_file_path_p,
		       long *max_retrieval_nmins,
		       long *max_valid_data_age,
		       long *nstations_p,
		       station_t *stations);

static int write_history(char *history_file_path,
			 long history_time);

/*
 * main
 */

main (int argc, char **argv)

{

  char *param_file_path;
  char *platform_name;
  char *aws_data_dir;
  char *history_file_path;

  char *field_names[NFIELDS] = {FIELD_NAMES};

  int forever = TRUE;

  long ifield, istation, imin;
  long nfields = NFIELDS;
  long nstations;
  long prev_nperiod = 0;
  long nperiod;
  long history_time;
  long prev_min, now_min;
  long max_retrieval_nmins;
  long max_valid_data_age;
  long retrieval_nmins;
  long start_min, end_min;
  long file_mark[MAX_STATIONS];

  time_t now;
  struct tm *ltime;
  
  FieldId fid[NFIELDS];
  PlatformId plat;
  Location loc[MAX_STATIONS];
  PlatformId pid[MAX_STATIONS];
  station_t stations[MAX_STATIONS];
  station_t *station;
  
  /*
   * Basic arg check.
   */

  IngestParseOptions(&argc, argv, NULL);

  if (argc < 3) {
    printf ("Usage: %s <param_file> <platform> [archive] \n", argv[0]);
    exit (1);
  }
  
  param_file_path = argv[1];
  platform_name = argv[2];

  if (argc > 3) archive_realtime_rap=TRUE;
  else archive_realtime_rap=FALSE;
  
      

  /*
   * Hook into the world.
   */

#ifdef USE_ZEB
  IngestInitialize ("AWSIngest");
#endif

  /*
   * More initialization.
   */

#ifdef USE_ZEB
  if ((plat = ds_LookupPlatform (platform_name)) == BadPlatform) {
    IngestLog (EF_PROBLEM, "Unknown platform: ", platform_name);
    exit (-1);
  }
#endif
  
  /*
   * read in parameters
   */

  if (read_params(param_file_path,
		  &aws_data_dir,
		  &history_file_path,
		  &max_retrieval_nmins,
		  &max_valid_data_age,
		  &nstations,
		  stations)) {

#ifdef USE_ZEB
    IngestLog (EF_PROBLEM, "Error reading params file '%s'", errno, 
	       param_file_path);
#else
    fprintf(stderr, "Error reading params file '%s'", errno, 
	    param_file_path);
#endif
    exit (-1);
  }

#ifdef DBUG
  fprintf(stderr, "History_file_path: %s\n", history_file_path);
  fprintf(stderr, "Max_Retrieval_Nmins: %ld\n", max_retrieval_nmins);
  fprintf(stderr, "Max_valid_data_age: %ld\n", max_valid_data_age);
#endif

  /*
   * Look up (or declare) the fields
   */

#ifdef USE_ZEB
  for (ifield = 0; ifield < nfields; ifield++)
    fid[ifield] = F_Lookup (field_names[ifield]);
#endif
    
  /*
   * Make sure that the stations are known to the system.
   */
  
  station = stations;

  for (istation = 0; istation < nstations; istation++) {

#ifdef USE_ZEB
    if ((station->pid = ds_LookupPlatform (station->label)) == BadPlatform) {
      msg_ELog (EF_PROBLEM, "Unknown subplat '%s'", station->label);
    }
#endif
    
    pid[istation] = station->pid;
    loc[istation] = station->loc;
    
    station++;
    
  } /* istation */

  /*
   * initialize the file seek positions to 0
   */

  memset((void *) file_mark, (int) 0,
	 (size_t) (MAX_STATIONS * sizeof(long)));
  
  /*
   * Loop forever, getting the data at retrieval period intervals
   */

  while (forever) {

   /* 
    * KLUDGE ALERT - These system commands are to solve the problem
    * that the permissions on the Campbell station datafiles are
    * being reset somehow near midnight so that the data cannot be
    * obtained.  This resets so that data can be written to the files.
    * Add new stations as they come online.
    *
    * The above method alone was not successful as the program would
    * die before it was successful at that time of day.  The code looks
    * like it should take care of new files, etc, except it the file were 
    * changed in midstream.  Hack here to prevent processing during the
    * period of 23:50 to midnight.
    */

    time(&now);
    ltime = localtime(&now);
    if ( ltime->tm_hour == 23 && ltime->tm_min >= 50 ) {
/*	printf(" %d %d Sleeping now\n\r",ltime->tm_hour,ltime->tm_min); */
	sleep(900); 
    }
    
/*    system("chmod a+w /usr/users/lantex/zeb/data_ingest/aws/stalt3.dat");
    system("chmod a+w /usr/users/lantex/zeb/data_ingest/aws/staust.dat"); */

    /*
     * find start of next retrieval period
     */

    time(&now);
    /* Store time as a global for real-time archive */
    archive_time = now;
    
    nperiod = now / RETRIEVAL_PERIOD;
#ifdef CDBUG
  fprintf(stderr, "nperiod: %ld\n", nperiod);    
#endif

    if (nperiod > prev_nperiod) {

      /*
       * read the last time for which data was stored
       */

      if (read_history(history_file_path, &history_time))
	exit(-1);

#ifdef DBUG
      fprintf(stderr, "History time : %ld\n", history_time);
#endif

      /*
       * compute the time in mins
       */

      prev_min = history_time / 60;
      now_min = now / 60;

      /*
       * compute the time period which still needs to be retrieved
       */
      
      retrieval_nmins = now_min - prev_min;

      if (retrieval_nmins > max_retrieval_nmins) {
	retrieval_nmins = max_retrieval_nmins;
	start_min = now_min - retrieval_nmins + 1;
      } else {
	start_min = prev_min + 1;
      }

      end_min = now_min - RETRIEVAL_DELAY / 60;

      /*
       * read in data for each minute period between the start and end
       */

      for (imin = start_min; imin <= end_min; imin++) {

	read_data(imin,
		 file_mark,
		 max_valid_data_age,
		 plat,
		 aws_data_dir,
		 nfields,
		 fid,
		 nstations,
		 stations,
		 loc,
		 pid);

	{

	  date_time_t store_time;

	  store_time.unix_time = imin * 60;
	  uconvert_from_utime(&store_time);

#ifdef DBUG	  
	  fprintf(stderr, "Stored at time %s\n",
		  utimestr((dattim_t *) &store_time));
#endif

	}

	/*
	 * write the history time
	 */

	if (write_history(history_file_path, (long) (imin * 60))) 
	    exit (-1);

	
      }

      prev_nperiod = nperiod;
      
      /*
       *  Move realtime archive files to a different directory to be read
       */
      
      save_archive();
      
    } else {

      /*
       * sleep a little, waiting for next period
       */
      
      sleep(10);

    } /* if (nperiod ... */

  } /* while */
  
  exit (0);
  
}

/*****************************************************
 * read_history()
 *
 * returns 0 on success, 1 on failure
 */

static int read_history(char *history_file_path,
			long *history_time)

{

  FILE *history_file;

  /*
   * open history file
   */

  history_file = fopen(history_file_path, "r");

  if (history_file == NULL) {
#ifdef USE_ZEB
    msg_ELog (EF_PROBLEM, "Cannot open history file %s for reading\n",
	      history_file_path);
#else
    fprintf(stderr, "Cannot open history file for reading\n");
    perror(history_file_path);
#endif
    *history_time = 0;
    return (0);
  }

  if (fscanf(history_file, "%ld", history_time) != 1) {
#ifdef USE_ZEB
    msg_ELog (EF_PROBLEM, "Cannot read history file %s\n",
	      history_file_path);
#else
    fprintf(stderr, "Cannot read history file\n");
    perror(history_file_path);
#endif
    fclose(history_file);
    *history_time = 0;
    return (0);
  }

  fclose(history_file);

  return (0);

}

/*******************************************
 * read_params()
 *
 * returns 0 on success, -1 on failure
 */

static int read_params(char *params_file_path,
		       char **aws_data_dir_p,
		       char **history_file_path_p,
		       long *max_retrieval_nmins_p,
		       long *max_valid_data_age_p,
		       long *nstations_p,
		       station_t *stations)

{

  char *comment1;
  char comment2[MAX_LINE];
  char *aws_data_dir;
  char *history_file_path;
  char line[MAX_LINE];
  char type_str[16];
  long istation;
  station_t *station;
  FILE *param_file;
  
  if ((param_file = fopen (params_file_path, "r")) == NULL) {
#ifdef USE_ZEB
    IngestLog (EF_PROBLEM, "Error %d opening '%s'", errno, 
	       params_file_path);
#else
    fprintf(stderr, "Error %d opening '%s'", errno, 
	    params_file_path);
#endif
    return (-1);
  }
  
  /*
   * aws data directory
   */

  fgets(line, MAX_LINE, param_file);
  comment1 = strtok(line, " ,\n\t\r");
  aws_data_dir = strtok((char *) NULL, " ,\n\t\r");
 
  if (aws_data_dir == NULL) {
#ifdef USE_ZEB
    IngestLog (EF_PROBLEM, "Error parsing data dir, file '%s'", 
	       params_file_path);
#else
    fprintf(stderr, "Error parsing data dir, file '%s'", 
	    params_file_path);
#endif
    return (-1);
  }
  
  *aws_data_dir_p = (char *) malloc (strlen(aws_data_dir) + 1);
  strcpy(*aws_data_dir_p, aws_data_dir);

  /*
   * history file
   */

  fgets(line, MAX_LINE, param_file);
  comment1 = strtok(line, " ,\n\t\r");
  history_file_path = strtok((char *) NULL, " ,\n\t\r");
 
  if (history_file_path == NULL) {
#ifdef USE_ZEB
    IngestLog (EF_PROBLEM, "Error parsing data dir, file '%s'", 
	       params_file_path);
#else
    fprintf(stderr, "Error parsing data dir, file '%s'", 
	    params_file_path);
#endif
    return (-1);
  }
  
  *history_file_path_p = (char *) malloc (strlen(history_file_path) + 1);
  strcpy(*history_file_path_p, history_file_path);

  /*
   * How many mins into the past will we retrieve?
   */

  fgets(line, MAX_LINE, param_file);
  if (sscanf (line, "%s %ld", comment2, max_retrieval_nmins_p) != 2) {
#ifdef USE_ZEB
    IngestLog (EF_PROBLEM, "Error parsing max_retrieval_nmins, file '%s'", 
	       params_file_path);
#else
    fprintf(stderr, "Error parsing max_retrieval_nmins, file '%s'", 
	    params_file_path);
#endif
    return (-1);
  }

  /*
   * How long will data stay valid before it is regarded as missing
   */

  fgets(line, MAX_LINE, param_file);
  if (sscanf (line, "%s %ld", comment2, max_valid_data_age_p) != 2) {
#ifdef USE_ZEB
    IngestLog (EF_PROBLEM, "Error parsing max_valid_data_age, file '%s'", 
	       params_file_path);
#else
    fprintf(stderr, "Error parsing max_valid_data_age, file '%s'", 
	    params_file_path);
#endif
    return (-1);
  }

  /*
   * How many stations?
   */

  fgets(line, MAX_LINE, param_file);
  if (sscanf (line, "%s %ld", comment2, nstations_p) != 2) {
#ifdef USE_ZEB
    IngestLog (EF_PROBLEM, "Error parsing nstations, file '%s'", 
	       params_file_path);
#else
    fprintf(stderr, "Error parsing nstations, file '%s'", 
	    params_file_path);
#endif
    return (-1);
  }
  
  if (*nstations_p > MAX_STATIONS) {
#ifdef USE_ZEB
    IngestLog (EF_PROBLEM, "Too many stations : %ld", 
	       *nstations_p);
#else
    fprintf(stderr, "Too many stations : %ld", 
	    *nstations_p);
#endif
    return (-1);
  }

  /*
   * Read the station params
   */
  
  station = stations;
  
  for (istation = 0; istation < *nstations_p; istation++) {

    /*
     * read the info for this station
     */

    fgets(line, MAX_LINE, param_file);

    if (sscanf (line, "%s %s %f %f %f",
		station->label,
		type_str,
		&station->loc.l_lon,
		&station->loc.l_lat,
		&station->loc.l_alt) != 5) {
#ifdef USE_ZEB
      IngestLog (EF_PROBLEM, "Error parsing station params, file '%s'", 
		 params_file_path);
#else
      fprintf(stderr, "Error parsing station params, file '%s'", 
	      params_file_path);
#endif
      return (-1);
    }
    
    if (!strcmp(type_str, "CAMPBELL"))
      station->type = campbell;
    else
      station->type = ro;
    
    station++;
    
  } /* istation */

  return (0);

}

/*****************************************************
 * write_history()
 *
 * returns 0 on success, 1 on failure
 */

static int write_history(char *history_file_path,
			 long history_time)

{

  FILE *history_file;

  /*
   * open history file
   */

  history_file = fopen(history_file_path, "w");


  if (history_file == NULL) {
#ifdef USE_ZEB
    msg_ELog (EF_PROBLEM, "Cannot open history file %s for writing\n",
	      history_file_path);
#else
    fprintf(stderr, "Cannot open history file for writing\n");
    perror(history_file_path);
#endif
    return (-1);
  }

  if (fprintf(history_file, "%ld\n", history_time) == EOF) {
#ifdef USE_ZEB
    msg_ELog (EF_PROBLEM, "Cannot write history file %s\n",
	      history_file_path);
#else
    fprintf(stderr, "Cannot write history file\n");
    perror(history_file_path);
#endif
    fclose(history_file);
    return (-1);
  }
  
  fclose(history_file);

  return (0);
  
}












