/*
 * Consume RO SYNOPTIC images into the data store.
 */
/*		Copyright (C) 1992 by UCAR
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

static char *rcsid = "$Id";

# include <copyright.h>
# include <unistd.h>
# include <errno.h>
# include <math.h>
# include <stdio.h>
# include <dirent.h>
# include <defs.h>
# include <message.h>
# include <timer.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <DataStore.h>


typedef struct {
  double min_val;
  double max_val;
  char name[64];
} field_var_t;

typedef struct {
  long nx, ny;
  double minx, miny;
  double deltax, deltay;
  double origin_lat, origin_lon;
} grid_var_t;

#define MAX_LINE 256

/*
 * Useful stuff
 */
# define BETWEEN(x,lower,upper)	(((x)-(lower))*((x)-(upper)) <= 0)
# define DEG_TO_RAD(x)	((x)*0.017453292)
# define RAD_TO_DEG(x)	((x)*57.29577951)
# define DEG_TO_KM(x)	((x)*111.3238367) /* on a great circle */
# define BADVAL -9999.0

/*
 * prototypes
 */

static void Die (void);

static int do_ingest(char *platform,
		     char *control_path,
		     char *data_path,
		     long nfields,
		     field_var_t *fields,
		     grid_var_t *grid,
		     time_t data_time);

static int MDispatcher (struct message *msg);

static int new_file(char *control_path,
		    char *data_path,
		    time_t *data_time);

static void read_params(char *params_path,
			char *data_path,
			char *control_path,
			long *nfields_p,
			grid_var_t *grid,
			field_var_t **fields_p);

static int write_control(char *control_path,
			 time_t data_time);

/*
 * main
 */

main (int argc, char **argv)

{

  char line[MAX_LINE];
  char data_path[MAX_LINE];
  char control_path[MAX_LINE];
  char *platform;
  char *params_path;

  int forever = TRUE;

  long nfields;
  time_t data_time;
  
  grid_var_t grid;
  field_var_t *fields;
  
  /*
   * Checking
   */

  if (argc != 3) {
    printf ("Usage: %s <platform> <param_file>\n", argv[0]);
    exit (-1);
  }

  platform = argv[1];
  params_path = argv[2];
  
  /*
   * Initialize UI symbol stuff and connect to the message handler
   */

  usy_init ();
  msg_connect (MDispatcher, "ROSynopticIngest");

  /*
   * DS initialization
   */

  ds_Initialize ();

  /*
   * read in params
   */
  
  read_params(params_path,
	      data_path,
	      control_path,
	      &nfields,
	      &grid,
	      &fields);
  
  /*
   * data ingest
   */

  while (forever) {

    if (new_file(control_path, data_path, &data_time) == TRUE) {

      if (do_ingest (platform,
		     control_path,
		     data_path,
		     nfields,
		     fields,
		     &grid,
		     data_time) == 0) {
	  
	write_control(control_path, data_time);

      }

    }

    sleep (60);

  }

  Die ();

}

/************************************************
 * Die()
 */

static void Die ()

     /*
      * Finish gracefully.
      */

{

  ui_finish ();
  exit (0);

}

/************************************************
 * do_ingest()
 */

static int do_ingest(char *platform,
		     char *control_path,
		     char *data_path,
		     long nfields,
		     field_var_t *fields,
		     grid_var_t *grid,
		     time_t data_time)


     /*
      * Ingest the RO_SYNOPTIC data
      */

{

  int retval = 0;
  
  long ipoint, npoints;
  long ifield;
  
  float *data_array = NULL;

  field_var_t *field;
  ZebTime zt;
  RGrid rg;
  DataChunk *dc = NULL;
  Location loc;
  PlatformId pid;
  FieldId *fid = NULL;

  FILE	*data_file;

  /*
   * allocate arrays
   */

  npoints = grid->ny * grid->nx;

  data_array = (float *) malloc ((u_int) (npoints * sizeof(float)));
  fid = (FieldId *) malloc ((u_int) (nfields * sizeof(FieldId)));

  /*
   * Platform ID
   */

  if ((pid = ds_LookupPlatform (platform)) == BadPlatform) {
    msg_ELog (EF_PROBLEM, "Bad platform '%s'", platform);
    retval = -1;
    goto error_return;
  }

  /*
   * load up fid
   */

  F_Init ();

  for (ifield = 0; ifield < nfields; ifield++)
    fid[ifield] = F_Lookup (fields[ifield].name);

  /*
   * Create the data chunk
   */
  
  dc = dc_CreateDC (DCC_RGrid);
  dc->dc_Platform = pid;
  dc_RGSetup (dc, (int) nfields, fid);
  
  /*
   * set the bad value
   */

  dc_SetBadval (dc, BADVAL);
  
  /*
   * Open the data file
   */

  data_file = fopen (data_path, "r");

  if (data_file == 0) {
    msg_ELog (EF_PROBLEM, "Error %d opening file '%s'\n", errno,
	      data_path);
    retval = -1;
    goto error_return;
  }

  /*
   * loop through the fields
   */

  field = fields;

  for (ifield = 0; ifield < nfields; ifield++) {

    /*
     * Read the input data
     */
    
    if (fread((void *) data_array,
	      (size_t) sizeof(float),
	      (int) npoints,
	      data_file) != npoints) {
      msg_ELog (EF_PROBLEM, "Error %d reading from file '%s'\n", errno,
		data_path);
      retval = -1;
      goto error_return;
    }

    /*
     * Location
     */
    
    loc.l_lat = grid->miny + grid->deltay / 2.0;
    loc.l_lon = grid->minx + grid->deltax / 2.0;
    loc.l_alt = 0.000;

    /*
     * rgrid info
     */
    
    rg.rg_Xspacing =
      DEG_TO_KM (grid->deltax) * cos (DEG_TO_RAD (grid->origin_lat));
    rg.rg_Yspacing = DEG_TO_KM (grid->deltay);
    rg.rg_Zspacing = 1.0;
  
    rg.rg_nX = grid->nx;
    rg.rg_nY = grid->ny;
    rg.rg_nZ = 1;
    
    /*
     * time
     */

    zt.zt_Sec = data_time;
    zt.zt_MicroSec = 0;

    /*
     * Put the grid into the data chunk
     */

    dc_RGAddGrid (dc, 0, fid[ifield], &loc, &rg, &zt, data_array, 0);
    
    /*
     * free up
     */

    field++;

  } /* ifield */

  /*
   * Store it
   */
  
  ds_Store (dc, 1, NULL, 0);

  msg_ELog (EF_INFO, "ROSynoptic added data for time %ld", data_time);
  
 error_return:

  /*
   * We're done with the file
   */

  fclose (data_file);

  /*
   * Free all that memory we hogged up
   */

  if (dc != NULL)
    dc_DestroyDC (dc);

  if (fid != NULL)
    free (fid);
    
  if (data_array != NULL)
    free (data_array);

  return (retval);

}

/******************************************************
 * MDispatcher ()
 */

static int MDispatcher (struct message *msg)

     /*
      * Deal with a message.
      */

{

  struct mh_template *tmpl = (struct mh_template *) msg->m_data;

  switch (msg->m_proto)	{

  case MT_MESSAGE:
    if (tmpl->mh_type == MH_DIE)
      exit (-1);
    break;
  }
  return (0);
}   	

/*******************************************************
 * new_file()
 *
 * Check for new file
 *
 * returns 1 for new file, 0 not new file, -1 on error
 */

static int new_file(char *control_path,
		    char *data_path,
		    time_t *data_time) 

{

  char line[MAX_LINE];
  time_t prev_time, data_file_time;
  FILE *control_file;
  struct stat stats;
  
  /*
   * get time of previous store operation
   */

  control_file = fopen(control_path, "r");

  if (control_file == NULL) {
    msg_ELog(EF_PROBLEM, "Error %d opening control file '%s'\n",
	     errno, control_path);
    return (-1);
  }

  fgets(line, MAX_LINE, control_file);

  if (sscanf(line, "%ld", &prev_time) != 1) {
    msg_ELog(EF_PROBLEM, "Error %d reading prev_time, file '%s'",
	     errno, control_path);
    return (-1);
  }

  /*
   * get data file time
   */
  
  if (stat(data_path, &stats)) {
    msg_ELog(EF_PROBLEM, "Error %d getting stats, file '%s'\n",
	     errno, data_path);
    return (-1);
  }
  
  data_file_time = stats.st_mtime;
  *data_time = data_file_time;

  fclose(control_file);
  
  if (prev_time == data_file_time)
    return (FALSE);
  else
    return (TRUE);

}

/*********************************************
 * read_params()
 */

static void read_params(char *params_path,
			char *data_path,
			char *control_path,
			long *nfields_p,
			grid_var_t *grid,
			field_var_t **fields_p)

{
	       
  char line[MAX_LINE];
  long ifield;
  long nfields;
  field_var_t *fields;
  
  FILE *params_file;

  /*
   * Open the params file and read in params
   */

  params_file = fopen (params_path, "r");

  if (params_file == NULL) {
    msg_ELog(EF_PROBLEM, "Cannot open params file '%s', error %d\n",
	     params_path, errno);
    Die ();
  }
  
  /*
   * data file and control file paths
   */
  
  fgets(data_path, MAX_LINE, params_file);
  data_path[strlen(data_path) - 1] = '\0';
  
  fgets(control_path, MAX_LINE, params_file);
  control_path[strlen(control_path) - 1] = '\0';
  
  /*
   * grid params
   */

  fgets(line, MAX_LINE, params_file);

  if (sscanf(line, "%ld%ld%lg%lg%lg%lg%lg%lg",
	     &grid->ny, &grid->nx,
	     &grid->miny, &grid->minx,
	     &grid->deltay, &grid->deltax,
	     &grid->origin_lat, &grid->origin_lon) != 8) {
    msg_ELog(EF_PROBLEM, "Error reading grid params, file '%s', error %d\n",
	     params_path, errno);
    fclose(params_file);
    Die ();
  }

  /*
   * nfields
   */

  fgets(line, MAX_LINE, params_file);

  if (sscanf(line, "%ld", &nfields) != 1) {
    msg_ELog(EF_PROBLEM, "Error reading nfields, file '%s', error %d\n",
	     params_path, errno);
    fclose (params_file);
    Die ();
  }
  
  *nfields_p = nfields;

  /*
   * field params
   */

  fields =
    (field_var_t *) malloc ((u_int) (nfields * sizeof(field_var_t)));

  for (ifield = 0; ifield < nfields; ifield++) {

    fgets(line, MAX_LINE, params_file);
    
    if (sscanf(line, "%s%lg%lg",
	       &fields[ifield].name,
	       &fields[ifield].min_val,
	       &fields[ifield].max_val) != 3) {
      msg_ELog(EF_PROBLEM,
	       "Error reading field params, file '%s', field %ld, error %d\n",
	       params_path, ifield, errno);
      fclose (params_file);
      Die ();
    }
    
  } /* ifield */

  *fields_p = fields;

  fclose(params_file);

}

/*************************************************
 * write_control()
 *
 * returns 0 on success, -1 on error
 */

static int write_control(char *control_path,
			  time_t data_time)

{

  FILE *control_file;
  
  control_file = fopen(control_path, "w");

  if (control_file == NULL) {
    msg_ELog(EF_PROBLEM, "Cannot open control file '%s', error %d\n",
	     control_path, errno);
    return (-1);
  }

  /*
   * set time to new value
   */

  fprintf(control_file, "%ld\n", data_time);
  
  fclose(control_file);

  return (0);

}

