/*
 * Consume RO SATELLITE images into the data store.
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

/*
 * Return values for the shell:
 *
 * 0 - success
 * 9 - Zeb problem
 * 8 - File problem
 * 7 - Overlay file problem
 * any other value - any other failure
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
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <DataStore.h>
# include "ro_sat_ingest.h"

typedef struct {
  double min_val;
  double max_val;
  char name[64];
} field_var_t;


typedef struct {
  int year, month, day;
  int hour, min, sec;
  long unix_time;
} date_time_t;

#define MAX_LINE 256

#define ZEB_PROBLEM 9
#define FORMAT_PROBLEM 8
#define OVERLAY_PROBLEM 7

#define USE_ZEB

/* #define DEBUG_FILE "/zeb/src/ingest/ro_sat/image.debug" */


/*
 * prototypes
 */

static u_char *create_mask(grid_var_t *grid);

static void Die (int retval);

static int do_ingest(char *platform,
		     char *data_path,
		     char *overlay_path,
		     long nfields,
		     field_var_t *fields,
		     grid_var_t *grid,
		     time_t data_time,
		     u_char *mask);

static int MDispatcher (struct message *msg);

static void read_params(char *params_path,
			long *nfields_p,
			grid_var_t *grid,
			field_var_t **fields_p);

static int get_date_from_path(char *data_path,
			      date_time_t *sat_time);

static int read_sat_image(char *data_path,
			  u_char *data_array,
			  grid_var_t *grid,
			  date_time_t *sat_time);

/*
 * main
 */

int main (int argc, char **argv)

{

  u_char *mask;

  char line[MAX_LINE];
  char *data_path;
  char *overlay_path=NULL;
  char *platform;
  char *params_path;

  int forever = TRUE;
  int retval;

  long nfields;
  time_t data_time;
  
  grid_var_t grid;
  field_var_t *fields;
  
  /*
   * Checking
   */

  if (argc != 4 && argc != 5) {
    printf("Usage: %s <param_file> <data_file> <platform> [ <overlay_file> ]\n"
	   , argv[0]);
    return (-1);
  }

  data_path = argv[2];
  params_path = argv[1];
  platform = argv[3];
  if (argc==5) overlay_path = argv[4];
  
  
#ifdef USE_ZEB
  /*
   * Initialize UI symbol stuff and connect to the message handler
   */

  usy_init ();

  if (!msg_connect (MDispatcher, "ROSatImageIngest")) {
    fprintf(stderr, "%s: Zeb not running\n", argv[0]);
    return (ZEB_PROBLEM);
  }

  /*
   * DS initialization
   */

  if (!ds_Initialize ()) {
    fprintf(stderr, "%s: Zeb not running\n", argv[0]);
    return (ZEB_PROBLEM);
  }
#endif

  /*
   * read in params
   */
  
  read_params(params_path,
	      &nfields,
	      &grid,
	      &fields);

  /*
   * data ingest
   */
  
  retval = do_ingest (platform,
		      data_path,
		      overlay_path,
		      nfields,
		      fields,
		      &grid,
		      data_time,
		      mask);

  Die (retval);

}

  
/************************************************
 * Die()
 */

static void Die (int retval)

     /*
      * Finish gracefully.
      */

{
#ifdef USE_ZEB
  ui_finish ();
#endif
  exit (retval);

}

/************************************************
 * do_ingest()
 */

static int do_ingest(char *platform,
		     char *data_path,
		     char *overlay_path,
		     long nfields,
		     field_var_t *fields,
		     grid_var_t *grid,
		     time_t data_time,
		     u_char *mask)


     /*
      * Ingest the RO_Satellite Image data
      */

{

  u_char *data_array = NULL;
  u_char *overlay_array = NULL;
  u_char *overlaid_data_array = NULL;

  int retval = 0;
  
  long npoints;
  long ifield;
  
  ZebTime zt;
  RGrid rg;
  Location loc;
  PlatformId pid;
  date_time_t sat_time;

  field_var_t *field;
  DataChunk *dc = NULL;
  FieldId *fid = NULL;
  ScaleInfo *scale = NULL;

  /*
   * allocate arrays
   */

  npoints = grid->sub_ny * grid->sub_nx;

  data_array = (u_char *) malloc ((u_int) (npoints * sizeof(u_char)));
  overlay_array = (u_char *) malloc ((u_int) (npoints * sizeof(u_char)));
/*
  overlaid_data_array = (u_char *) malloc ((u_int) (npoints * sizeof(u_char)));
*/
  fid = (FieldId *) malloc ((u_int) (nfields * sizeof(FieldId)));
  scale = (ScaleInfo *) malloc ((u_int) (nfields * sizeof(ScaleInfo)));
  
#ifdef USE_ZEB
  /*
   * Platform ID
   */

  if ((pid = ds_LookupPlatform (platform)) == BadPlatform) {
    msg_ELog (EF_PROBLEM, "Bad platform '%s'", platform);
    retval = ZEB_PROBLEM;
    goto error_return;
  }

  F_Init ();
#endif

  for (ifield = 0; ifield < nfields; ifield++) {

#ifdef USE_ZEB
    /*
     * load up fid
     */
    
    fid[ifield] = F_Lookup (fields[ifield].name);
#endif

    /*
     * Scaling
     */
  
    scale[ifield].s_Scale = 1.0;
    scale[ifield].s_Offset = (double) OFFSET;

  }

#ifdef USE_ZEB
  /*
   * Create the data chunk
   */
  
  dc = dc_CreateDC (DCC_Image);
  dc->dc_Platform = pid;
  dc_ImgSetup (dc, (int) nfields, fid, scale);
#endif
  
  /*
   * loop through the fields
   */

  for (ifield = 0; ifield < nfields; ifield++) {

    /*
     * Read the input data
     */
  
    if (read_sat_image(data_path,
		       data_array,
		       grid,
		       &sat_time)) {

#ifdef USE_ZEB
      msg_ELog (EF_PROBLEM, "Error reading sat image file '%s'\n",
		data_path);
#else
      fprintf(stderr,"Error reading sat image file '%s'\n",
                data_path);
#endif
      retval = FORMAT_PROBLEM;
      goto error_return;
    }

    if (overlay_path!=NULL) {
	if (extract_overlay(overlay_path,overlay_array,grid)) {

#ifdef USE_ZEB
	    msg_ELog (EF_PROBLEM, "Error reading overlay file '%s'\n",
		      overlay_path);
#else
	    fprintf(stderr,"Error reading overlay file '%s'\n",
		    overlay_path);
#endif
	    retval = OVERLAY_PROBLEM;

	} else {
	    apply_overlay(npoints,data_array,overlay_array,data_array);
	}
    }

    
    /*
     * Location
     */
    
    cvt_Origin (grid->origin_lat, grid->origin_lon);
    cvt_ToLatLon (grid->minx, grid->miny, &loc.l_lat, &loc.l_lon);
    loc.l_alt = grid->altitude; /* km */

    /*
     * rgrid info
     */
    
    rg.rg_Xspacing = grid->deltax;
    rg.rg_Yspacing = grid->deltay;
    rg.rg_Zspacing = 1.0;
    
    rg.rg_nX = grid->sub_nx;
    rg.rg_nY = grid->sub_ny;
    rg.rg_nZ = 1;
    
    /*
     * time
     */
   
    uconvert_to_utime(&sat_time);
    zt.zt_Sec = sat_time.unix_time;
    zt.zt_MicroSec = 0;

#ifdef USE_ZEB
    /*
     * Put the grid into the data chunk
     */

    dc_ImgAddImage (dc, 0, fid[ifield], &loc, &rg, &zt, data_array, 0);
#endif    
    /*
     * free up
     */

  } /* ifield */

#ifdef USE_ZEB

  /*
   * Store it
   */
  
  ds_Store (dc, 1, NULL, 0);

  {
    char msg[256];

    TC_EncodeTime (&zt, TC_Full, msg);
    msg_ELog (EF_INFO, "ROSatImage added data for time %s", msg);
  }

#endif

 error_return:

  /*
   * Free all that memory we hogged up
   */
#ifdef USE_ZEB
  if (dc != NULL)
    dc_DestroyDC (dc);
#endif

  if (fid != NULL)
    free (fid);
    
  if (scale != NULL)
    free (scale);
    
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
      Die (ZEB_PROBLEM);
    break;
  }
  return (0);
}   	

/*********************************************
 * read_params()
 */

static void read_params(char *params_path,
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
#ifdef USE_ZEB
    msg_ELog(EF_PROBLEM, "Cannot open params file '%s', error %d\n",
	     params_path, errno);
#else
    fprintf(stderr, "Cannot open params file '%s'\n", params_path);
#endif
    Die (ZEB_PROBLEM);
  }
  
  /*
   * grid params
   */

  fgets(line, MAX_LINE, params_file);

  if (sscanf(line, "%ld%ld%ld%ld%ld%ld%lg%lg%lg%lg%lg%lg%lg",
	     &grid->ny, &grid->nx,
	     &grid->sub_ny, &grid->sub_nx,
	     &grid->sub_y0, &grid->sub_x0,
	     &grid->miny, &grid->minx,
	     &grid->deltay, &grid->deltax,
	     &grid->origin_lat, &grid->origin_lon,
	     &grid->altitude) != 13) {
#ifdef USE_ZEB
    msg_ELog(EF_PROBLEM, "Error reading grid params, file '%s', error %d\n",
	     params_path, errno);
#else
    fprintf(stderr,"Error reading grid params, file '%s'\n", params_path);
#endif
    fclose(params_file);
    Die (ZEB_PROBLEM);
  }

  /*
   * nfields
   */

  fgets(line, MAX_LINE, params_file);

  if (sscanf(line, "%ld", &nfields) != 1) {
#ifdef USE_ZEB
    msg_ELog(EF_PROBLEM, "Error reading nfields, file '%s', error %d\n",
	     params_path, errno);
#else
    fprintf(stderr,"Error reading nfields, file '%s'\n", params_path);
#endif
    fclose (params_file);
    Die (ZEB_PROBLEM);
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
#ifdef USE_ZEB
      msg_ELog(EF_PROBLEM,
	       "Error reading field params, file '%s', field %ld, error %d\n",
	       params_path, ifield, errno);
#else
      fprintf(stderr, "Error reading field params, file '%s', field %ld\n\r",
	               params_path, ifield);
#endif
      fclose (params_file);
      Die (ZEB_PROBLEM);
    }
    
  } /* ifield */

  *fields_p = fields;

  fclose(params_file);

}


/************************************************
 * get_date_from_path()
 *
 * Returns 0 on success, -1 on failure
 */

static int 
get_date_from_path(char *data_path,
		   date_time_t *sat_time)

{
  
    char *fnp;
    int year;
    
    
    /* find beginning of file name */
    fnp = strrchr(data_path,'/');
    
    fnp = (fnp!=NULL) ? ++fnp : data_path;

    /* format of pathname from RO is YYMMDDhhmm. */
    if (sscanf(fnp,"%2d%2d%2d%2d%2d", 
	   &year,&sat_time->month,&sat_time->day,
	   &sat_time->hour,&sat_time->min) != 5) {
#ifdef USE_ZEB
	msg_ELog(EF_PROBLEM,
	       "Error getting date from path, file '%s'\n",
	       data_path);
#else
	fprintf(stderr,"Error getting date/time from path %s\n\r",data_path);
#endif
	return(-1);
    }
    
    sat_time->year = 1900 + year;
    sat_time->sec = 0;

    return(0);
}

	   

/************************************************
 * read_sat_image()
 *
 * Returns 0 on success, -1 on failure
 */

static int 
read_sat_image(char *data_path,
	       u_char *data_array,
	       grid_var_t *grid,
	       date_time_t *sat_time
	       )

{
  
  FILE *data_file,*dbg;
  u_char *data,pixel;

  int maxLine,maxElement,minElement;
  int nLine,nElement,nprint=0;
  int i,j;

  /* Open the debug image file, if requested */

#ifdef DEBUG_FILE
  dbg = fopen(DEBUG_FILE,"w");
#else
  dbg = NULL;
#endif

  /*
   * Open the data file
   */
  
  data_file = fopen (data_path, "r");
  
  if (data_file == NULL) {
#ifdef USE_ZEB
    msg_ELog (EF_PROBLEM, "Error %d opening file '%s'\n", errno,
	      data_path);
#else
    fprintf(stderr,"Error opening file '%s'\n", data_path);
#endif
    return (-1);
  }

  /* Set up dimensions of grid to be read */
  maxLine = grid->sub_y0 + grid->sub_ny;
  minElement = grid->sub_x0;
  maxElement = grid->sub_x0 + grid->sub_nx;
  
  
  /* Get the time data from the data pathname */

  get_date_from_path(data_path,sat_time);
  
  /* Extract the image data	*/
  data = data_array;
  nLine = 0;
  while (nLine < maxLine) {
      /* Just read through the first lines outside the subgrid */
      if (nLine < grid->sub_y0) {
	  for (j=0;j<grid->nx;j++) getc(data_file);
      } else {
	  if (dbg != NULL) {
	      fprintf(dbg,"Line %d\n",nLine);
	      nprint=0;
	  }
	  
	  nElement = 0;
	  i = 0;
	  data = data_array + ((nLine - grid->sub_y0) * grid->sub_nx);
	  while (nElement < grid->nx) {
	      pixel = getc(data_file);

	      /* Error checking */
	      if (feof(data_file)) {
#ifdef USE_ZEB
		  msg_ELog (EF_PROBLEM, "Premature end of file '%s'\n",
			    data_path);

#else
		  fprintf(stderr,"ROSatImage Premature EOF for %s\n\r",
			  data_path);
#endif
		  fclose(data_file);
		  if (dbg!=NULL) fclose(dbg);
		  return(1);
	      }
	      
	      if (nElement >= minElement && nElement < maxElement) {  
		  data[i] = pixel;
		  i++;
		  
		  if (dbg != NULL) {
		      fprintf(dbg,"%2h ",pixel);
		      if (++nprint >= 40) {
			  fprintf(dbg,"\n");
			  nprint=0;
		      }
		  }
	      }
	      nElement++;
	  }
      }
      nLine++;
  }
  
  fclose (data_file);
  if (dbg != NULL) fclose(dbg);
  return (0);

}

