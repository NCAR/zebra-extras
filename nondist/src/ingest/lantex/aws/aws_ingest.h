/*
 * aws_ingest.h
 *
 *
 * Header file for aws_ingest.c and related modules
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
 * define USE_ZEB for normal ops, comment out for debugging without
 * ZEB running
 */

#define USE_ZEB  

/*
 * define the ARCHIVE_DIR to get realtime archiving
 * of data desired for RAP in the HongKong field project
 * Files will be put in ARCHIVE_DIR/read when they are complete
 */

#define ARCHIVE_DIR "/zeb/data_ingest/archive/aws/real_time"


/*
 * define DBUG for debug output, comment out for normal ops
 */

/*
#define DBUG
#define CDBUG
*/

# include <errno.h>
# include <math.h>
# include <string.h>
# include <time.h>
# include <sys/types.h>
# include <sys/stat.h>

# include "ingest.h"
# include "aws_fields.h"

# ifndef lint
MAKE_RCSID ("profs_ingest.c,v 1.2 1992/12/22 21:11:40 granger Exp")
# endif

#define DEG_TO_RAD(x) ((x)*0.017453293)

typedef struct {
  long year, month, day, hour, min, sec;
} dattim_t;

typedef struct {
  long year, month, day, hour, min, sec;
  long unix_time;
} date_time_t;

typedef struct {
  char label[16];
  enum {campbell, ro} type;
  Location loc;
  PlatformId pid;
} station_t;

typedef struct {
  float pres;
  float pres_max;
  float pres_min;
  float cpres0;
  float tdry;
  float twet;
  float rh;
  float dp;
  float mr;
  float pt;
  float ept;
  float wspd;
  float wspd_max;
  float wspd_min;
  float wspd_sdev;
  float wdir;
  float wdir_sdev;
  float u_wind;
  float v_wind;
  float raina01;
  float solrad;
  float vis;
} field_t;

#define TRUE 1
#define FALSE 0

#define FIELD_NAMES \
     "pres", "pres_min", "pres_max", "cpres0", \
     "tdry", "twet", "rh",\
     "dp", "mr", "pt", "ept",\
     "wspd", "wspd_max", "wspd_min", "wspd_sdev",\
     "wdir", "wdir_sdev", "u_wind", "v_wind",\
     "raina01", "solrad", "vis"

#define MAX_LINE 1024

#define RETRIEVAL_PERIOD 60 /* the time period for data retrievals - they
			     * are done oncer per period */

#define RETRIEVAL_DELAY 300 /* delay is imposed to give data a chance to 
			     * arrive before being ingested */

#define NFIELDS 22 /* number of fields in final data */

#define SECS_AHEAD_GMT 28800 /* GMT correction */

#define MAX_STATIONS 100

#define DC_BADVAL -9999.0

/*
 * prototypes
 */

extern double pr_vwnd(double v, double dir);
extern double pr_drct(double u, double v);
extern double pr_vapr(double t);
extern double pr_relh(double t, double td);
extern double pr_thta(double t, double p);
extern double pr_mixr(double td, double p);
extern double pr_thte(double p, double t, double td);
extern double pr_tlcl(double t, double td);
extern double pr_tmst(double tthe, double p, double tguess);
extern double pr_rhdp(double t, double relh);
extern double pr_twet(double p, double t, double td);
extern double pr_tvrt(double p, double t, double td);

extern void impose_limits (long nfields,
			   field_t *field_data);

extern void impose_spatial_consistency (long nfields,
					long nstations,
					float *field_data);

extern void init_fields(long nfields,
			float *fields_merged,
			float *fields_prev,
			float *fields_prev_valid);

extern void merge_fields(long nfields,
			 float *fields_in,
			 float *fields_merged,
			 float *fields_prev,
			 float *fields_prev_valid,
			 double *max_change);

extern int read_campbell_data(char *data_file_name,
			      long *file_mark_p,
			      long max_valid_data_age,
			      long search_sec,
			      date_time_t *data_time,
			      field_t *field_data);

extern void read_data (long search_min,
		       long *file_mark,
		       long max_valid_data_age,
		       PlatformId plat,
		       char *aws_data_dir,
		       long nfields,
		       FieldId *fid,
		       long nstations,
		       station_t *stations,
		       Location *loc,
		       PlatformId *pid);

extern int read_ro_data(char *data_file_name,
			long *file_mark_p,
			long max_valid_data_age,
			long search_sec,
			date_time_t *data_time,
			field_t *field_data);

extern long uunix_time(dattim_t *date_time);
 
extern dattim_t *udattim(long int unix_time);
 
extern long uconvert_to_utime(date_time_t *date_time);
 
extern void uconvert_from_utime(date_time_t *date_time);
 
extern long ujulian_date(long int day, long int month, long int year);
 
extern void ucalendar_date(long int jdate, long int *day,
                           long int *month, long int *year);
 
extern void ulocaltime(dattim_t *dattim);
 
extern void ugmtime(dattim_t *dattim);
 
extern char *utimestr(dattim_t *dattim);
 









