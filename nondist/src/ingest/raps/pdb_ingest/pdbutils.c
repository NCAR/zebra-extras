/*
 * pdbutils.c -- A collection of useful utilities for
 *		 ingesting boundary products, especially an intelligent
 *		 common Location buffer.
 *
 * $Id: pdbutils.c,v 1.1 1993-06-28 05:35:17 granger Exp $
 */

#if !defined(saber) && !defined(lint)
static char rcsid[]="$Id: pdbutils.c,v 1.1 1993-06-28 05:35:17 granger Exp $";
#endif

/*------- For PDBS: ----*/

#include "common.h"
#undef ABS

/*------- For Zeb: -----*/

#include "ingest.h"
#include "pdb_ingest.h"		/* For the ProductRecord type */
#include "pdbutils.h"

#define PROF(a) 

/*
 * Use a static Locations buffer for converting points to lat/lon
 * and storing in data-chunks.  It re-allocs as necessary to supply
 * adequate space for each request, and then remains that size for
 * later requests.  It checks for consistently reduced requests and
 * reduces its allocation accordingly.  It is hoped this reduces
 * malloc delays and memory thrashing.
 */
Location *
GetLocnsBuffer(npts)
	unsigned long npts;	/* Number locns required */
{
	static Location *locns = NULL;
	static unsigned int numlocns = 0;
	static unsigned long count = 0;
	static unsigned long max = 0;
#	define NCOUNTS 20
	/*
	 * max == Maximum # points requested in last 'count' calls
	 *	  which were less than numlocns
	 * After NCOUNTS counts, if max < 0.5*numlocns, we'll reduce
	 * our memory allocation
	 */

	if (!locns)  /* set up some first guess at a good size */
	{
		numlocns = (npts > 100) ? npts : 100;
		PROF((
   "Initializing locations buffer: %li requested, %li points allocated\n",
		   npts,numlocns))
		locns = (Location *)malloc(numlocns * sizeof(Location));
	}
	else if (numlocns < npts)
	{
	PROF(("Expanding buffer from %li to requested %li\n",numlocns,npts))
		locns = (Location *)
			realloc((char *)locns, npts * sizeof(Location));
		numlocns = npts;
		count = 0;	/* Restart our underused count */
		max = 0;
	}
	/* else buffer is already big enough;
	 * we'll keep a count of these occasions and perhaps try to
	 * reduce our memory need after some # of times
	 */
	else
	{
		++count;
		if (npts > max)
			max = npts;
		if (count > NCOUNTS) 
		{
			PROF(("Last %i requests have been less than %li.  ",
			   NCOUNTS, max))
			PROF(("There are currently %li locations allocated\n",
			   numlocns))
			if ((numlocns >> 1) > max)
			{
				PROF(("Re-alloc'ing %li locations to %li\n",
				   numlocns, max))
				locns = (Location *)
				   realloc((char *)locns,
					   max * sizeof(Location));
				numlocns = max;
			}
			count = 0;
			max = 0;
		}
	}

	return(locns);
}

/*
 * To get a sub-platform for a storm, we automatically start over with
 * the platform sub-id's if its been more than 4 minutes since the
 * last storm product.  This is based on the assumption that all of the
 * storm products come in at once about every 7 minutes.  Since we don't
 * get valid id numbers with which we can correlate sub-platform id's and
 * overwrite older data for the same storm, we just always keep all
 * storm products in the lowest possible range of sub-id's.  This way
 * any previous storm data will be over-written.
 */
static void
GetStormSubid(pr, idnum, subid)
	ProductRecord *pr;
	long idnum;
	int *subid;
{
	static int last_subid = 0;
	static time_t last_storm = 0;	/* Time of last storm data surge */
	time_t now;

	/*
	 * If this sub-id request is recent, then just use the next
	 * possible sub-id.  Else this is a new surge and we should
	 * reset our sub-id to 0 and start counting over.
	 */
	Time(&now);
	if (now - last_storm < 240/*secs*/)
	{
		/* We still must stay in range of sub-id's */
		last_subid = (last_subid % MaxSubplatforms) + 1;
	}
	else
	{
		last_subid = 0;
		last_storm = now;
	}
	*subid = last_subid;
}


/*
 * Uses an LRU replacement strategy for giving the most recently
 * used id_numbers identical subplatform ids.  Each
 * ProductRecord holds four important fields:
 * index_oldest == Points to least recently used idnum
 * index_newest == Points the idnum most recently used
 * id_numbers[subid-1] == The id_number last used for this subid
 * next_oldest[] == Index of the next least recently used idnum
 */
static void
GetLRUSubid(pr, idnum, subid)
	ProductRecord *pr;
	long idnum;
	int *subid;
{
	short i,j;

	/*
	 * idnum is assumed to be an id number from the data
	 * product.  This id number is searched for in the LRU arrays
	 * from oldest to newest.  If the search reaches the newest
	 * id number (at index_newest), then the idnum was not found and
	 * it is inserted at index_oldest.  
	 * Otherwise the index of the found id number becomes the subid,
	 * and the found subid becomes the newest idnum
	 */
	i = pr->index_oldest;
	j = i;
	while (pr->id_numbers[i] != idnum)
	{
		if ((i == pr->index_newest))
		{
			/* Id number not found!
			 * Have to replace the oldest id_number 
			 * with this idnum.
			 * Essentially this means inserting 
			 * at the beginning of next_oldest list
			 */
			i = pr->index_oldest;
			j = pr->index_newest;
			break;
		}
		/* 
		 * Go to the next_oldest id_number, but store the
		 * current index for use when inserting
		 */
		j = i;
		i = pr->next_oldest[i];
	}
	/*
	 * i is the index to store the idnum in.  It becomes the newest idnum.
	 * j is the index of the element in next_oldest which pointed to i.
	 * j must be made to point to what i pointed to.
	 */
	*subid = i+1;
	pr->id_numbers[i] = idnum;
	if (i != pr->index_newest) /* else this is already the newest idnum */
	{
		if (i == pr->index_oldest)
		{
			pr->index_oldest = pr->next_oldest[i];
		}
		pr->next_oldest[j] = pr->next_oldest[i];
		pr->next_oldest[pr->index_newest] = i;
		pr->index_newest = i;
		pr->next_oldest[i] = pr->index_oldest;
	}
}



/* 
 * Here we use a subid to create a new platform name
 * and get a platid for it... If its a bad platform,
 * return NULL for the platform name.  
 *
 * To accomodate special handling of storm products, these are
 * just directed to a different function, GetStormSubPlatform().
 * All other products use the LRU method above.
 */
char *
GetSubPlatform(pr, idnum, subid, platid)
	ProductRecord *pr;
	long idnum;
	int *subid;
	PlatformId *platid;
{
	static char platform[15];

	/*
	 * First check for a valid idnum.  idnum == -1 implies null product
	 */
	if (!CheckIdNum(pr, idnum))
		return NULL;
	/*
	 * Now check for storm products, pdb_id's 30, 31, and 32.
	 * --- but only for polyline format...
	 */
	switch (pr->pdb_id)
	{
	   case 30,31,32:
		if (pr->format == PDB_FORMAT_POLYLINE)
		{
			GetStormSubid(pr, idnum, subid);
			break;
		}
	   default:
		GetLRUSubid(pr, idnum, subid);
	}

	sprintf(platform,"%s.%i",pr->platform,*subid);
	*platid = ds_LookupPlatform(platform);
	if (*platid == BadPlatform)
	{
	   IngestLog(EF_PROBLEM,"bad platform %s for product %d",
	      platform, pr->rap_id);
	   return(NULL);
	}
	IngestLog(EF_INFO,
	 "%s(%d,%d) idnum %li maps to subid %i, sub-platform %s, plat_id %d",
	 pr->code, pr->rap_id, pr->pdb_id, 
	 idnum, *subid, platform, *platid);
	return(platform);
}



/*
 * Print time_t times using ZebTime's TimePrintFormat 
 * Mimics asctime and ctime but without the annoying \n
 * Note that since it uses a static char buffer, this 
 * function can only be invoked once in a parameter list
 */
char *
AscSysTime(systime, fmt)
	long systime;
	TimePrintFormat fmt;
{
	static char timestring[50];
	ZebTime ztime;

	TC_SysToZt(systime, &ztime);
	TC_EncodeTime(&ztime, fmt, timestring);
	return(timestring);
}



/*
 * Set an integer attribute in a DataChunk sample
 */
void
SetIntAttr(dc, sample, att, val)
	DataChunk *dc;
	int sample;
	char *att;
	int val;
{
	char value[10];

	sprintf(value,"%li",val);
	dc_SetSampleAttr(dc, sample, att, value);
}


/*
 * Set a float attribute in a single DataChunk sample
 */
void
SetFloatAttr(dc, sample, att, val)
	DataChunk *dc;
	int sample;
	char *att;
	float val;
{
	char value[15];

	sprintf(value,"%f",val);
	dc_SetSampleAttr(dc, sample, att, value);
}


/*
 * Set a point attr in a DataChunk using Nowc_pt coordinates x,y
 */
void
SetNowcCenterAttr(dc, x, y)
	DataChunk *dc;
	short x, y;
/*
 * Assumes cvt_Origin has already been set.
 * Sets the "center_lat" and "center_lon" attributes of a DataChunk
 */
{
	char value[30];
	float lat, lon;

	/* Convert the x,y center to lat/long */
	cvt_ToLatLon(FKM(x), FKM(y), &lat, &lon);
	sprintf(value,"%f",lat);
	dc_SetSampleAttr(dc, 0, AttCenterLat, value);
	sprintf(value,"%f",lon);
	dc_SetSampleAttr(dc, 0, AttCenterLon, value);
}



/*
 * Translate an array of Nowc_pt points into lat/long and
 * return an array of locns to which the lat/long has been
 * appended starting at position start.
 */
void
AppendNowcLocations(locns, start, shortpts, npts)
	Location **locns;  /* Array to append to, may be moved */
	int start;	   /* Where to start adding at */
	Nowc_pt *shortpts; /* {short x; short y;}  */
	int  npts;  	   /* # pts to add from shortpts */
/*
 * Assumes cvt_Origin has already been set 
 */
{
	int i;

	/*
	 * Make sure our locns buffer has enough room
	 * to store more points
	 */
	(*locns) = GetLocnsBuffer(start + npts);

	for (i=0; i< npts; i++)
	{
		cvt_ToLatLon(FKM(shortpts[i].x), 
			     FKM(shortpts[i].y),
			     &(*locns)[start+i].l_lat,
			     &(*locns)[start+i].l_lon);
		(*locns)[start + i].l_alt = 0.0;
	}
}



void
TimerTriggerCall(zt, param)
	ZebTime *zt;
	void *param;
{
	ProductRecord *pr = (ProductRecord *)param;
	static Location trigger = { 0.0, 0.0, 0.0 };
	PlatformId trigger_id;
	DataChunk *tdc;

	tdc = dc_CreateDC(DCC_Boundary);
	trigger_id = ds_LookupPlatform(pr->platform);
	tdc->dc_Platform = trigger_id;
	dc_BndAdd(tdc, zt, trigger_id, &trigger, 1);
	if (!ds_Store(tdc, FALSE, 0, 0))
	{
		IngestLog(EF_PROBLEM,
		   "Trigger for %s failed at %s",
		   pr->platform,
		   AscSysTime(TC_ZtToSys(zt),TC_Full));
	}
	else
	{
		IngestLog(EF_DEBUG,
		   "Trigger stored for platform %s at %s",
		   pr->platform,
		   AscSysTime(TC_ZtToSys(zt),TC_Full));
	}
	/* Now delete the data just written to save space */
	ds_DeleteData(trigger_id, 0);
	dc_DestroyDC(tdc);
}


void
TriggerDataStore(pr, when, platid)
	ProductRecord *pr;
	ZebTime *when;
	PlatformId platid;
/*
 * Trigger new data by writing one point to the parent
 * platform. XXX as Jon would say...
 * But we have to make sure we don't do it too often,
 * so platforms like act, acs, and hal should not use
 * this trigger method but instead a timeout like 1m
 * We assume that all data products are global triggers,
 * so if a trigger is pending from any other platform, its
 * trigger will serve to update other platforms needing
 * an update as well.
 *
 * At the moment, we are just using the current system time
 * as the trigger platform time. This could change...
 *
 * Prevents triggers from occurring more often than every 5 secs
 * by using timer relative events to actually trigger data
 */
{
	static time_t last_trigger = 0;
	time_t now;

	now = Time(0);
	if ((now - last_trigger < 4))
		return;
	last_trigger = now;
	if (!NoMessageHandler)
	{
		tl_RelativeReq(
		   TimerTriggerCall, (void *)pr, 5*INCFRAC, 0);
		IngestLog(EF_DEBUG,
		   "Trigger event requested for %s at %s",
		   pr->platform, AscSysTime(now, TC_Full));
	}
}


/** Time Utilities ******************************************************/


char *Stime6( tim)
   Time6	tim;
   {
      static char result[30];
      sprintf( result, "%2d/%2d/%2d %2d:%2d:%2d", tim.year, 
	tim.month, tim.day, tim.hour, tim.minute, tim.second);
      return (result);
   }

char *St6_time( tim)
   Time6	tim;
   {
      static char result[30];
      sprintf( result, "%2d:%2d:%2d", tim.hour, tim.minute, tim.second);
      return (result);
   }


long Ctime6( tim)
   Time6 tim;
   {
      struct tm tm;

      tm.tm_year = tim.year;
      tm.tm_mon = tim.month-1;
      tm.tm_mday = tim.day;
      tm.tm_hour = tim.hour;
      tm.tm_min = tim.minute;
      tm.tm_sec = tim.second;

      tm.tm_isdst = 0;
      tm.tm_gmtoff = 0;
      tm.tm_zone = NULL;
      /* tm.tm_zone = "GMT"; */

      return (timegm( &tm));
    }



void
StrToUpper(str)
	char *str;
{
	while (*str)
		*(str++) = toupper(*str);
}


void
StrToLower(str)
	char *str;
{
	while (*str)
		*(str++) = tolower(*str);
}


time_t 
Time( tloc )
	time_t *tloc;
/*
 * Replicates a call to time() which can't be used because of
 * the 'time' typedef in Zeb's defs.h
 */
{
	struct timeval tp;

	gettimeofday(&tp, NULL);
	if (tloc)
		*tloc = tp.tv_sec;
	return(tp.tv_sec);
}


/*
 * Most PDB products contain id's of -1 to indicate NULL products
 */
short
CheckIdNum(pr, idnum)
	ProductRecord *pr;
	long idnum;
{
	/* 
	 * If the idnum is -1, then this a NULL packet and we should ignore it
	 */
	if (idnum < 0)
	{
		IngestLog(EF_DEBUG,"  %s(%d): Null packet ignored",
			  pr->code, pr->rap_id);
		return(0);
	}
	else
	{
		return(1);
	}
}


