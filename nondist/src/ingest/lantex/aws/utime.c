/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1992, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1993/3/2 13:59:22
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: granger $
 *   $Locker:  $
 *   $Date: 1997-02-07 18:03:51 $
 *   $Id: utime.c,v 1.1 1997-02-07 18:03:51 granger Exp $
 *   $Revision: 1.1 $
 *   $State: Exp $
 *
 *   $Log: not supported by cvs2svn $
 * Revision 1.2  1993/08/11  15:16:56  dixon
 * All compression routines now in compress.c
 *
 */
 
#ifndef LINT
static char RCS_id[] = "$Id: utime.c,v 1.1 1997-02-07 18:03:51 granger Exp $";
static char SCCS_id[] = "%W% %D% %T%";
#endif /* not LINT */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*************************************************************************
 * utime.c: Routines to do time/date convertions
 *
 * Obtained from Frank Hage
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 *************************************************************************/

#include <sys/types.h>
#include <time.h>
#include <stdio.h>

extern time_t time(time_t *);

#define JAN_1_1970 2440587

typedef struct {
 long year, month, day, hour, min, sec;
 long unix_time;
} date_time_t;

typedef struct {
 long year, month, day, hour, min, sec;
} dattim_t;

void ucalendar_date(long int jdate, long int *day,
		    long int *month, long int *year);
long ujulian_date(long int day, long int month, long int year);
long uconvert_to_utime(date_time_t *date_time);
void uconvert_from_utime(date_time_t *date_time);

/*************************************************************************
 * uunix_time()
 *
 * return the unix time from dattim_t struct
 */

long uunix_time(dattim_t *date_time)
{

  long day, days;
  long u_time;

  day = ujulian_date(date_time->day,
		    date_time->month,
		    date_time->year);

  days = day - JAN_1_1970;

  u_time = (days * 86400) + (date_time->hour * 3600) +
    (date_time->min * 60) + date_time->sec;

  return u_time;

}

/*************************************************************************
 * udattim()
 *
 * return pointer to dattim struct corresponding to unix time. The pointer
 * refers to a static held by this routine
 *
 */

dattim_t *udattim(long int unix_time)
{

  static dattim_t dattim;

  long	day;
  
  day = (unix_time / 86400);
  
  ucalendar_date((JAN_1_1970 + day),
		 &dattim.day,
		 &dattim.month,
		 &dattim.year);
  
  day = (unix_time % 86400);
  dattim.hour = day / 3600;
  dattim.min = (day / 60) - (dattim.hour * 60);
  dattim.sec = day % 60;

  return (&dattim);

}
 
/*************************************************************************
 *
 */

long uconvert_to_utime(date_time_t *date_time)
{

  long day, days;
  long u_time;

  day = ujulian_date(date_time->day,
		    date_time->month,
		    date_time->year);

  days = day - JAN_1_1970;

  u_time = (days * 86400) + (date_time->hour * 3600) +
    (date_time->min * 60) + date_time->sec;

  date_time->unix_time = u_time;
  
  return u_time;

}

/*************************************************************************
 *
 */

void uconvert_from_utime(date_time_t *date_time)
{

  long	unix_time;
  long	day;
  
  unix_time = date_time->unix_time;

  day = (unix_time / 86400);
  
  ucalendar_date((JAN_1_1970 + day),
		 &date_time->day,
		 &date_time->month,
		 &date_time->year);
  
  day = (unix_time % 86400);
  date_time->hour = day / 3600;
  date_time->min = (day / 60) - (date_time->hour * 60);
  date_time->sec = day % 60;

}
 
/*************************************************************************
 *	JULIAN_DATE: Calc the Julian calendar Day Number
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

long ujulian_date(long int day, long int month, long int year)
{

  long a, b;
  double yr_corr;

  /*
   * correct for negative year
   */

  yr_corr = (year > 0? 0.0: 0.75);

  if(month <=2) {
    year--;
    month += 12;
  }

  b=0;

  /*
   * Cope with Gregorian Calendar reform
   */

  if(year * 10000.0 + month * 100.0 + day >= 15821015.0) {
    a = year / 100;
    b = 2 - a + a / 4;
  }
	
  return ((365.25 * year - yr_corr) +
	  (long) (30.6001 * (month +1)) +
	  day + 1720994L + b);

}

/*************************************************************************
 *	CALENDAR_DATE: Calc the calendar Day from the Julian date
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

void ucalendar_date(long int jdate, long int *day,
		    long int *month, long int *year)
{

  long a, b, c, d, e, z, alpha;

  z = jdate +1;

  /*
   * Gregorian reform correction
   */

  if (z < 2299161) { 
    a = z; 
  } else {
    alpha = (long) ((z - 1867216.25) / 36524.25);
    a = z + 1 + alpha - alpha / 4;
  }

  b = a + 1524;
  c = (long) ((b - 122.1) / 365.25);
  d = (long) (365.25 * c);
  e = (long) ((b - d) / 30.6001);
  *day = (long) b - d - (long) (30.6001 * e);
  *month = (long) (e < 13.5)? e - 1 : e - 13;
  *year = (long) (*month > 2.5)? (c - 4716) : c - 4715;

}

/**************************************************************************
 * void ulocaltime()
 *
 * puts the local time into a date_time_t struct
 *
 **************************************************************************/

void ulocaltime(dattim_t *dattim)
{

  struct tm *local_time;
  time_t now;

  time(&now);
  local_time = localtime(&now);

  dattim->year = local_time->tm_year + 1900;
  dattim->month = local_time->tm_mon + 1;
  dattim->day = local_time->tm_mday;
  dattim->hour = local_time->tm_hour;
  dattim->min = local_time->tm_min;
  dattim->sec = local_time->tm_sec;

}

/**************************************************************************
 * void ugmtime()
 *
 * puts the gm time into a dattim_t struct
 *
 **************************************************************************/

void ugmtime(dattim_t *dattim)
{

  struct tm *gm_time;
  time_t now;

  time(&now);
  gm_time = gmtime(&now);

  dattim->year = gm_time->tm_year + 1900;
  dattim->month = gm_time->tm_mon + 1;
  dattim->day = gm_time->tm_mday;
  dattim->hour = gm_time->tm_hour;
  dattim->min = gm_time->tm_min;
  dattim->sec = gm_time->tm_sec;

}

/**************************************************************************
 * char *utimestr()
 *
 * returns a string composed from the time struct. This routine has a
 * number of static storage areas, and loops through these areas. Every
 * 10 times you make the call you will overwrite a previous result.
 *
 **************************************************************************/

char *utimestr(dattim_t *dattim)
{

  static char time_buf[10][32];
  static int count = 0;

  char *timestr;

  count++;
  
  if (count > 9)
    count = 0;

  timestr = time_buf[count];

  sprintf(timestr, "%.4ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld",
	  dattim->year,
	  dattim->month,
	  dattim->day,
	  dattim->hour,
	  dattim->min,
	  dattim->sec);

  return (timestr);

}
