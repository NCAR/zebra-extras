/*
 * Consume RO RADAR images into the data store.
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
  double altitude;
} grid_var_t;

typedef struct {
  int year, month, day;
  int hour, min, sec;
  long unix_time;
} date_time_t;

#define MAX_LINE 256

#define ERROR_RETURN -1
#define FORMAT_PROBLEM 8
#define ZEB_PROBLEM 9

#define HK_CAPPI256 '\b'
#define TIMEZONE 8           /* Hours relative to GMT */

/* Comment USE_ZEB out for debugging */

#define USE_ZEB


/*
 * Useful stuff
 */

# define BETWEEN(x,lower,upper)	(((x)-(lower))*((x)-(upper)) <= 0)
# define DEG_TO_RAD(x)	((x)*0.017453292)
# define RAD_TO_DEG(x)	((x)*57.29577951)
# define DEG_TO_KM(x)	((x)*111.3238367) /* on a great circle */
# define OFFSET 0
# define MISSING_VAL 255

/*
 * prototypes
 */

static u_char *create_mask(grid_var_t *grid);

static void Die (int retval);

static int do_ingest(char *platform,
		     char *data_path,
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

static int read_radar(char *data_path,
		      u_char *data_array,
		      grid_var_t *grid,
		      date_time_t *radar_time,
		      u_char *mask);

/*
 * main
 */

int main (int argc, char **argv)

{

  u_char *mask;

  char line[MAX_LINE];
  char *data_path;
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

  if (argc != 4) {
    printf ("Usage: %s <param_file> <data_file> <platform>\n", argv[0]);
    return (ERROR_RETURN);
  }

  data_path = argv[2];
  params_path = argv[1];
  platform = argv[3];

#ifndef USE_ZEB
  fprintf (stderr, "Debug version --- Zeb not being used.\n");
#endif

#ifdef USE_ZEB
  /*
   * Initialize UI symbol stuff and connect to the message handler
   */

  usy_init ();

  if (!msg_connect (MDispatcher, "RORadarIngest")) {
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

  mask = create_mask(&grid);

  /*
   * data ingest
   */

  retval = do_ingest (platform,
		      data_path,
		      nfields,
		      fields,
		      &grid,
		      data_time,
		      mask);

  Die (retval);

}

/************************************************
 * create_mask()
 *
 * create the mask array for blanking out the
 * corners
 */

static u_char *create_mask(grid_var_t *grid)

{

  u_char *mp;

  static u_char *mask = NULL;
  static long nx_used = 0;
  static long ny_used = 0;

  long ix, iy;

  double limit_radius, radius;
  double dx, dy;

  /*
   * initialize
   */

  limit_radius = (double) grid->nx * 0.47;

  /*
   * set up grid
   */

  if (nx_used != grid->nx || ny_used != grid->ny) {

    if (mask == NULL) {

      mask = (u_char *) malloc
	((u_int) (grid->nx * grid->ny * sizeof(u_char)));

    } else {

      mask = (u_char *) realloc
	((void *) mask,
	 (u_int) (grid->nx * grid->ny * sizeof(u_char)));

    }

    nx_used = grid->nx;
    ny_used = grid->ny;

  } /* if (nx_used != grid->nx || ny_used != grid->ny) */
  
  /*
   * load mask
   */

  mp = mask;

  for (iy = 0; iy < grid->ny; iy++) {

    for (ix = 0; ix < grid->nx; ix++) {

      dx = (double) (ix - grid->nx / 2);
      dy = (double) (iy - grid->ny / 2);

      radius = sqrt(dy * dy + dx * dx);

      if (radius < limit_radius)
	*mp = 0;
      else
	*mp = 1;

      mp++;

    } /* ix */

  } /* iy */

  return (mask);

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
		     long nfields,
		     field_var_t *fields,
		     grid_var_t *grid,
		     time_t data_time,
		     u_char *mask)


     /*
      * Ingest the RO_RADAR data
      */

{

  u_char *data_array = NULL;

  int retval = 0;
  
  long npoints;
  long ifield;
  
  ZebTime zt;
  RGrid rg;
  Location loc;
  PlatformId pid;
  date_time_t radar_time;

  field_var_t *field;
  DataChunk *dc = NULL;
  FieldId *fid = NULL;
  ScaleInfo *scale = NULL;

  /*
   * allocate arrays
   */

  npoints = grid->ny * grid->nx;

  data_array = (u_char *) malloc ((u_int) (npoints * sizeof(u_char)));
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

    /*
     * load up fid
     */

#ifdef USE_ZEB    
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
  
    if (read_radar(data_path,
		   data_array,
		   grid,
		   &radar_time,
		   mask)) {
/*
#ifdef USE_ZEB
      msg_ELog (EF_PROBLEM, "Error reading radar file '%s'\n", data_path);
#else
      fprintf(stderr,""Error reading radar file '%s'\n", data_path);
#endif
*/
      retval = FORMAT_PROBLEM;
      goto error_return;
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
    
    rg.rg_nX = grid->nx;
    rg.rg_nY = grid->ny;
    rg.rg_nZ = 1;
    
    /*
     * time
     */
   
    /* convert HK local time to GMT */
    radar_time.hour -= TIMEZONE;

    uconvert_to_utime(&radar_time);
    zt.zt_Sec = radar_time.unix_time;
    zt.zt_MicroSec = 0;

    /* convert days and hours to GMT properly */
    uconvert_from_utime(&radar_time);

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
    msg_ELog (EF_INFO, "RORadar added data for time %s", msg);
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
    fprintf(stderr,"Cannot open params file '%s', error %d\n",
             params_path, errno);
#endif
    Die (ERROR_RETURN);
  }
  
  /*
   * grid params
   */

  fgets(line, MAX_LINE, params_file);

  if (sscanf(line, "%ld%ld%lg%lg%lg%lg%lg%lg%lg",
	     &grid->ny, &grid->nx,
	     &grid->miny, &grid->minx,
	     &grid->deltay, &grid->deltax,
	     &grid->origin_lat, &grid->origin_lon,
	     &grid->altitude) != 9) {
#ifdef USE_ZEB
    msg_ELog(EF_PROBLEM, "Error reading grid params, file '%s', error %d\n",
	     params_path, errno);
#else
    fprintf(stderr,"Error reading grid params, file '%s', error %d\n",
             params_path, errno);
#endif
    fclose(params_file);
    Die (ERROR_RETURN);
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
    fprintf(stderr,"Error reading nfields, file '%s', error %d\n",
             params_path, errno);
#endif
    fclose (params_file);
    Die (ERROR_RETURN);
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
      fprintf(stderr, 
	      "Error reading field params, file '%s', field %ld, error %d\n",
               params_path, ifield, errno);
#endif
      fclose (params_file);
      Die (ERROR_RETURN);
    }
    
  } /* ifield */

  *fields_p = fields;

  fclose(params_file);

}

/************************************************
 * read_radar()
 *
 * Returns 0 on success, -1 on failure
 */

static int read_radar(char *data_path,
		      u_char *data_array,
		      grid_var_t *grid,
		      date_time_t *radar_time,
		      u_char *mask)

{
  
  FILE *data_file;
  u_char *data;

  /*
   * lookup vals
   *
   * Coastline : 0
   * Ground : 6
   * Circle : 10
   * Background within circle : 15
   * Background outside circle : 7
   * Not used : 3, 8
   * Rain rates .3 - 2 : 2
   *            2 - 5 : 11
   *            5 - 10 : 1
   *            10 - 20 : 9
   *            20 - 30 : 5
   *            30 - 50 : 13
   *            50 - 100 : 12
   *            100 - 200 : 4
   *            > 200 : 14
   */

  u_char lookup[16] = {10 + OFFSET,
		       37 + OFFSET,
                       20 + OFFSET,
                        MISSING_VAL,
		       58 + OFFSET,
		       45 + OFFSET,
			MISSING_VAL,
			MISSING_VAL,
			MISSING_VAL,
		       42 + OFFSET,
			MISSING_VAL,
		       30 + OFFSET,
		       53 + OFFSET,
		       48 + OFFSET,
		       63 + OFFSET,
		        MISSING_VAL};

  char id[5],header[41];
  char nColor;
  int nLine,nTotalLength,nLength;
  int i;

  /*
   * Open the data file
   */
  
  data_file = fopen (data_path, "r");
  
  if (data_file == NULL) {
#ifdef USE_ZEB
    msg_ELog (EF_PROBLEM, "Error %d opening file '%s'\n", errno, data_path);
#else
    fprintf(stderr, "Error %d opening file '%s'\n", errno, data_path);
#endif
    return (ERROR_RETURN);
  }

  /* Check first 5 bytes for correctness	*/
  for (i=0; i<5; i++) {
      id[i] = getc(data_file);
      if (feof(data_file)) {
	  fclose(data_file);
#ifdef USE_ZEB
	  msg_ELog(EF_PROBLEM,"Error in file '%s'. Premature end of ID.",
		   data_path);
#else
	  fprintf(stderr,"Error in file '%s'. Premature end of ID.\n\r",
                   data_path);
#endif
	  return (FORMAT_PROBLEM);
      }
  }
  
  if ((id[0] != 0) || (id[1] != 1) || (id[3] != 0) || (id[4] != 2) ||
      (id[2] != HK_CAPPI256)) {
      fclose(data_file);
#ifdef USE_ZEB
      msg_ELog(EF_PROBLEM,"ID error in file '%s'.",data_path);
#else
      fprintf(stderr,"ID error in file '%s'.\n\r",data_path);
#endif
      return (FORMAT_PROBLEM);
  }
  
  /* Decode day and time from 40-byte header */
  if (fread(header,1,40,data_file) != 40) {
      fclose(data_file);
#ifdef USE_ZEB
      msg_ELog(EF_PROBLEM,"Error in file '%s'. Premature end of header.",
	       data_path);
#else
      fprintf(stderr,"Error in file '%s'. Premature end of header.\n\r",
               data_path);
#endif
      return (FORMAT_PROBLEM);
  }

  sscanf(&(header[15]),"%2d:%2d",&(radar_time->hour),&(radar_time->min));
  sscanf(&(header[24]),"%2d-%2d-%4d",
	 &(radar_time->day),&(radar_time->month),&(radar_time->year));
  radar_time->sec = 0;
  
/* Decode the run-length endoded image data	*/
  data = data_array;
  nLine = 0;
  while (nLine < 255) {
      /* Check first 2 characters of next image line */
      for (i=0;i<2;i++) id[i]=getc(data_file);
      if ((id[0] != 0) || (id[1] != 3)) {
	  fclose(data_file);
#ifdef USE_ZEB
	  msg_ELog(EF_PROBLEM,"Error in file '%s'. Data id error.",
		   data_path);
#else
	  fprintf(stderr,"Error in file '%s'. Data id error\n\r",data_path);
#endif
	  return (FORMAT_PROBLEM);
      }
      
      nLine = getc(data_file);
      nTotalLength = 0;
      while (nTotalLength < 256) {
	  nColor = getc(data_file);
	  nColor--;
	  nLength = getc(data_file);
	  memset(data,lookup[nColor],nLength);
	  data += nLength;
	  nTotalLength += nLength;
      }
  }

  /* mask out corners */

  data = data_array;

  for (i = 0; i < grid->ny * grid->nx; i++) {

      if (*mask)
	*data = MISSING_VAL;

      data++;
      mask++;

  } /* i */
  
  fclose (data_file);
  return (0);

}

