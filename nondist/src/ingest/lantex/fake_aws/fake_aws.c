/*
 * Fake AWS ASCII data.
 *
 * Usage:
 *	fake_aws <aws_file>
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

# include <copyright.h>
# include <errno.h>
# include <math.h>
# include <stdlib.h>
# include <time.h>
# include "ingest.h"

typedef struct {
  double min;
  double max;
  double movement;
  double bias;
  double sdev;
  double current;
} met_var_t;

/*
 * prototypes
 */

void move_conditions(void);

void move_current(met_var_t *var);

void set_initial_conditions(void);

void set_initial(met_var_t *var,
		 double current,
		 double max,
		 double min,
		 double movement,
		 double bias,
		 double sdev);

void store_data (void);

/*
 * Global stuff
 */

#define MAX_LINE 1024
#define RAND_MAX 2147483647
#define SLEEP_TIME 120

FILE *Infile, *Outfile;
int Nsta, Nfld;
char Aws_data_dir[MAX_LINE];

met_var_t Press;
met_var_t Temp;
met_var_t Dewpt_dep;
met_var_t Wspeed;
met_var_t Wspeed_sdev;
met_var_t Wdir;
met_var_t Wdir_sdev;
met_var_t Rain_rate;
met_var_t Sol_rad;

main (int argc, char **argv)

{
  
  char line[MAX_LINE];
  int forever = 1;
  int now;

  /*
   * Basic arg check.
   */

  IngestParseOptions(&argc, argv, NULL);

  if (argc != 2) {
    printf ("Usage: %s <param_file>\n", argv[0]);
    exit (-1);
  }

  /*
   * Hook into the world.
   */

/*   IngestInitialize ("AWSFake"); */

  /*
   * Open the info file and read first line
   */

  if ((Infile = fopen (argv[1], "r")) == NULL) {
    IngestLog (EF_PROBLEM, "Error %d opening '%s'", errno, 
	       argv[1]);
    exit (-1);
  }

  /*
   * aws data directory
   */

  fgets(Aws_data_dir, MAX_LINE, Infile);
  Aws_data_dir[strlen(Aws_data_dir) - 1] = '\0';
  
  /*
   * How many stations and fields?
   */

  fgets(line, MAX_LINE, Infile);
  sscanf (line, "%d %d", &Nsta, &Nfld);

  fclose (Infile);

  /*
   * set up initial conditions
   */

  set_initial_conditions();
  now = time(&now);
  srand((unsigned int) now);

  /*
   * Loop forever, storing some data every 2 minutes. 
   */

  while (forever) {

    move_conditions();
    
    store_data ();

    sleep (SLEEP_TIME);
    
  }

  exit (0);

}

/*************************************************
 * get_val()
 */

double get_val(met_var_t *var)

{

  double delta;
  double random_num;
  double val;

  random_num = (double) rand() / (double) RAND_MAX - 0.5;
  delta = random_num * var->sdev;
  val = var->current + delta;

  if (val < var->min)
    val = var->min;

  if (val > var->max)
    val = var->max;

  return (val);
  
}

/*************************************************
 * move_conditions()
 */

void move_conditions()

{

  move_current(&Press);
  move_current(&Temp);
  move_current(&Dewpt_dep);
  move_current(&Wspeed);
  move_current(&Wspeed_sdev);
  move_current(&Wdir);
  move_current(&Wdir_sdev);
  move_current(&Rain_rate);
  move_current(&Sol_rad);

}

/*************************************************
 * move_current()
 */

void move_current(met_var_t *var)

{

  double delta;
  double random_num;

  random_num = (double) rand() / (double) RAND_MAX - 0.5;
  delta = var->movement * random_num + var->bias;
  var->current += delta;
  
  if (var->current < var->min)
    var->current = var->min;

  if (var->current > var->max)
    var->current = var->max;

}

/*************************************************
 * set_initial_conditions()
 */

void set_initial_conditions()

{

  set_initial(&Press, 1013.0, 1040.0, 960.0, 2.0, 0.0, 1.0);
  set_initial(&Temp, 30.0, 40.0, 10.0, 3.0, 0.0, 2.0);
  set_initial(&Dewpt_dep, 5.0, 20.0, 0.0, 1.5, 0.0, 2.0);
  set_initial(&Wspeed, 10.0, 80.0, 0.0, 5.0, 0.0, 3.0);
  set_initial(&Wspeed_sdev, 5.0, 30.0, 0.0, 3.0, 0.0, 5.0);
  set_initial(&Wdir, 120.0, 359.0, 0.0, 20.0, 0.0, 20.0);
  set_initial(&Wdir_sdev, 20.0, 100.0, 0.0, 5.0, 0.0, 10.0);
  set_initial(&Rain_rate, 0.0, 100.0, 0.0, 20.0, -5.0, 30.0);
  set_initial(&Sol_rad, 1.0, 1.3, 0.0, 0.3, 0.0, 0.7);

}

/*************************************************
 * set_initial()
 */

void set_initial(met_var_t *var,
		 double current,
		 double max,
		 double min,
		 double movement,
		 double bias,
		 double sdev)

{

  var->current = current;
  var->max = max;
  var->min = min;
  var->movement = movement;
  var->bias = bias;
  var->sdev = sdev;

}

/***************************************************
 * store_data()
 */

void store_data ()

/*
 * Store some fake data.
 */

{

  int	year, month, day, hour, min, sec, fld, sta, id;
  ZebTime	zt;
  time_t	t;
  char	filename[MAX_LINE];

  double pressure;
  double temperature;
  double dewpt_depression;
  double dewpt;
  double wind_speed;
  double wind_speed_max;
  double wind_speed_min;
  double wind_speed_sdev;
  double wind_dirn;
  double wind_dirn_sdev;
  double rainfall_rate;
  double solar_radiation;
  double e, es;
  double relative_humidity;

  /*
   * Loop through the stations.
   */

  for (sta = 0; sta < Nsta; sta++) {

    /*
     * Open the data file.
     */

    sprintf (filename, "%s/station%d.dat", Aws_data_dir, sta + 1);

    if ((Outfile = fopen (filename, "a")) == NULL) {
      IngestLog (EF_PROBLEM, "Error %d opening '%s'", errno, 
		 filename);
      exit (-1);
    }

/*     Outfile = stdout; */

    /*
     * get the current conditions
     */

    pressure = get_val(&Press);
    temperature = get_val(&Temp);
    dewpt_depression = get_val(&Dewpt_dep);
    wind_speed = get_val(&Wspeed);
    wind_speed_sdev = get_val(&Wspeed_sdev);
    wind_dirn = get_val(&Wdir);
    wind_dirn_sdev = get_val(&Wdir_sdev);
    rainfall_rate = get_val(&Rain_rate);
    solar_radiation = get_val(&Sol_rad);

    /*
     * compute
     */

    dewpt = temperature - dewpt_depression;

    es = 611.0 * exp((17.27 * temperature) / (237.3 + temperature));
    e = 611.0 * exp((17.27 * dewpt) / (237.3 + dewpt));
    relative_humidity = 100.0 * e / es;

    wind_speed_max = wind_speed + 2.0 * wind_speed_sdev;
    wind_speed_min = wind_speed - 1.0 * wind_speed_sdev;

    if (wind_speed_min < 0.0)
      wind_speed_min = 0.0;
    
    /*
     * Write some fake data.
     */

    time (&t);
    TC_SysToZt (t, &zt);
    TC_ZtSplit (&zt, &year, &month, &day, &hour, &min, &sec, NULL);

    fprintf (Outfile, "%d/%d/%d %d:%d:%d ", year, month, day, hour,
	     min, sec);
    fprintf (Outfile, " %d ", sta + 1);

    fprintf (Outfile, " %f ", pressure);
    fprintf (Outfile, " %f ", temperature);
    fprintf (Outfile, " %f ", relative_humidity);
    fprintf (Outfile, " %f ", dewpt);
    fprintf (Outfile, " %f ", wind_speed);
    fprintf (Outfile, " %f ", wind_speed_max);
    fprintf (Outfile, " %f ", wind_speed_min);
    fprintf (Outfile, " %f ", wind_speed_sdev);
    fprintf (Outfile, " %f ", wind_dirn);
    fprintf (Outfile, " %f ", wind_dirn_sdev);
    fprintf (Outfile, " %f ", rainfall_rate);
    fprintf (Outfile, " %f ", solar_radiation);
    fprintf (Outfile, "\n");

    fclose (Outfile);

  } /* sta */

}

