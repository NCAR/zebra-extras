/*
 * Fake synoptic grid data.
 *
 * Usage:
 *	fake_synoptic <synoptic_file>
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
# include <malloc.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "ingest.h"

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
 * prototypes
 */

static void read_params(char *params_path,
			char *data_path,
			char *control_path,
			long *nfields_p,
			grid_var_t *grid,
			field_var_t **fields_p);

static void write_control(char *control_path,
			  char *data_path);

static void write_data(char *data_path,
		       long nfields,
		       field_var_t *fields,
		       grid_var_t *grid);

/*
 * main
 */

main (int argc, char **argv)

{
  
  char line[MAX_LINE];
  char data_path[MAX_LINE];
  char control_path[MAX_LINE];
  char *params_path;

  long nfields;
  
  grid_var_t grid;
  field_var_t *fields;
  
  /*
   * Basic arg check.
   */

  if (argc != 2) {
    printf ("Usage: %s <param_file>\n", argv[0]);
    exit (-1);
  }

  /*
   * read in params
   */

  params_path = argv[1];
  
  read_params(params_path,
	      data_path,
	      control_path,
	      &nfields,
	      &grid,
	      &fields);
  
  /*
   * create and write the data file
   */

  write_data(data_path,
	     nfields,
	     fields,
	     &grid);

  /*
   * write control file
   */

  write_control(control_path,
		data_path);

}

/**********************************************************************
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
    fprintf(stderr, "Cannot open params file\n");
    perror(params_path);
    exit (-1);
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
    fprintf(stderr, "Error reading grid params\n");
    perror(params_path);
    exit (-1);
  }

  /*
   * nfields
   */

  fgets(line, MAX_LINE, params_file);

  if (sscanf(line, "%ld", &nfields) != 1) {
    fprintf(stderr, "Error reading nfields\n");
    perror(params_path);
    exit (-1);
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
      fprintf(stderr, "Error reading field params, field %ld\n", ifield);
      perror(params_path);
      exit (-1);
    }
    
  } /* ifield */

  *fields_p = fields;

  fclose(params_file);

}

/*************************************************
 * write_control()
 */

static void write_control(char *control_path,
			  char *data_path)

{

  long now;
  FILE *control_file;
  struct stat stats;

  control_file = fopen(control_path, "w");

  if (control_file == NULL) {
    fprintf(stderr, "Cannot open control file\n");
    perror(control_path);
    exit(-1);
  }

  /*
   * set time to 1 sec before file time to force a read on the file
   */
  
  if (stat(data_path, &stats)) {
    fprintf(stderr, "Error getting stats\n");
    perror(data_path);
    exit (-1);
  }

  now = stats.st_mtime - 1;

  fprintf(control_file, "%ld\n", now);

  fclose(control_file);

}

/*************************************************
 * write_data()
 */

static void write_data(char *data_path,
		       long nfields,
		       field_var_t *fields,
		       grid_var_t *grid)

{

  long ifield;
  long ix, iy;
  long npoints;
  float *data, *dp;
  double dist, max_dist;
  double start, range, fraction, val;
  FILE *data_file;
  field_var_t *field;

  data_file = fopen(data_path, "w");
  
  if (data_file == NULL) {
    fprintf(stderr, "Cannot open data file\n");
    perror(data_path);
    exit(-1);
  }

  /*
   * allocate data array
   */

  npoints = grid->ny * grid->nx;
  data = (float *) malloc ((u_int) (npoints* sizeof(float)));
  
  /*
   * create and store the data
   *
   * data vals are set according to the distance from
   * the origin
   */

  max_dist = sqrt((double) (grid->ny * grid->ny + grid->nx * grid->nx));
  field = fields;
  
  for (ifield = 0; ifield < nfields; ifield++) {
    
    dp = data;
    start = field->min_val;
    range = field->max_val - field->min_val;

    for (iy = 0; iy < grid->ny; iy++) {

      for (ix = 0; ix < grid->nx; ix++) {
	
	dist = sqrt((double) (iy * iy + ix * ix));
	fraction = dist / max_dist;
	val = start + fraction * range;
	*dp = (float) val;
	dp++;
	
      } /* ix */

    } /* iy */

    if (fwrite((void *) data,
	       (size_t) sizeof(float),
	       (int) npoints,
	       data_file) != npoints) {
      fprintf(stderr, "Cannot write data, field %ld\n", ifield);
      perror(data_path);
      exit(-1);
    }
    
    field++;

  } /* ifield */

  /*
   * free data array
   */

  free ((void *) data);

  fclose(data_file);

}

