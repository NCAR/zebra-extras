/*
 * $Id: pdbstime.c,v 1.1 1992-07-03 18:26:13 granger Exp $
 *
 * Common time conversion routines used by PDBS and the handlers.c to
 * interpret raw product times, which are often stored in Time6 format
 */

#if !defined(SABER) && !defined(lint)
static char rcsid[] = "$Id: pdbstime.c,v 1.1 1992-07-03 18:26:13 granger Exp $";
#endif

#include "common.h"


/** Time Utilities ******************************************************/

char *Stime6( time)
   Time6	time;
   {
      static char result[30];
      sprintf( result, "%2d/%2d/%2d %2d:%2d:%2d", time.year, 
	time.month, time.day, time.hour, time.minute, time.second);
      return (result);
   }

char *St6_time( time)
   Time6	time;
   {
      static char result[30];
      sprintf( result, "%2d:%2d:%2d", time.hour, time.minute, time.second);
      return (result);
   }


long Ctime6( time)
   Time6 time;
   {
      struct tm tm;

      tm.tm_year = time.year;
      tm.tm_mon = time.month-1;
      tm.tm_mday = time.day;
      tm.tm_hour = time.hour;
      tm.tm_min = time.minute;
      tm.tm_sec = time.second;

      tm.tm_isdst = 0;
      tm.tm_gmtoff = 0;
      tm.tm_zone = NULL;
      /* tm.tm_zone = "GMT"; */

      return (timegm( &tm));
    }

