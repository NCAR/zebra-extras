/*
 * handlers.c -- A collections of handlers for raw and polyline format
 *		 products.  Contains a useful collection of utilities for
 *		 ingesting boundary products, especially an intelligent
 *		 common Location buffer.
 *
 * $Id: handlers.c,v 1.1 1992-07-03 18:04:24 granger Exp $
 */

#if !defined(saber) && !defined(lint)
static char rcsid[]="$Id: handlers.c,v 1.1 1992-07-03 18:04:24 granger Exp $";
#endif

/*------- For PDBS: ----*/

#include "common.h"
#undef ABS

/*------- For Zeb: -----*/

#include "ingest.h"
#include "pdb_ingest.h"		/* For the ProductRecord type */

extern Location *GetLocnsBuffer FP((unsigned long npts));

/*----------------------*/

/*
 * To convert i16 coords in decameters to float km
 */
#define FKM(c) ((float)(((float)(c))/100.0))


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
#	define NCOUNTS 15
	/*
	 * max == Maximum # points requested in last 'count' calls
	 *	  which were less than numlocns
	 * After NCOUNTS counts, if max < 0.75*numlocns, we'll reduce
	 * our memory allocation
	 */

	if (!locns)  /* set up some first guess at a good size */
	{
		numlocns = (npts > 500) ? npts : 500;
		locns = (Location *)malloc(numlocns * sizeof(Location));
	}
	else if (numlocns < npts)
	{
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
			if ((numlocns >> 1) + (numlocns >> 2) > max)
			{
				locns = (Location *)
				   realloc((char *)locns,
					   max * sizeof(Location));
			}
			count = 0;
			max = 0;
		}
	}

	return(locns);
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
		IngestLog(EF_DEBUG,"Null packet ignored for %s, rap id %d",
			  pr->platform, pr->rap_id);
		return(0);
	}
	else
	{
		return(1);
	}
}


/* 
 * Here we use a subid to create a new platform name
 * and get a platid for it... If its a bad platform,
 * return NULL for the platform name.  It uses an LRU
 * replacement strategy for giving the most recently
 * used id_numbers identical subplatform ids.  Each
 * ProductRecord holds four important fields:
 * index_oldest == Points to least recently used idnum
 * index_newest == Points the idnum most recently used
 * id_numbers[subid-1] == The id_number last used for this subid
 * next_oldest[] == Index of the next least recently used idnum
 */
char *
GetSubPlatform(pr, idnum, subid, platid)
	ProductRecord *pr;
	long idnum;
	int *subid;
	PlatformId *platid;
{
	static char platform[15];
	short i,j,jump;

/*
 * First check for a valid idnum.  idnum == -1 implies null product
 */
	if (!CheckIdNum(pr, idnum))
		return NULL;
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
			 * Have to replace the oldest id_number with this idnum
			 * Essentially this means inserting at the beginning of
			 * next_oldest list
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

	sprintf(platform,"%s.%i",pr->platform,*subid);
	*platid = ds_LookupPlatform(platform);
	if (*platid == BadPlatform)
	{
	   IngestLog(EF_PROBLEM,"bad platform %s for product %d",
	      platform, pr->rap_id);
	   return(NULL);
	}
	IngestLog(EF_DEBUG,"subplatform %s: idnum %li, subid %i, plat_id %d",
	   platform, idnum, *subid, *platid);
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
SetIntAttr(dc, att, val)
	DataChunk *dc;
	char *att;
	int val;
{
	char value[10];

	sprintf(value,"%li",val);
	dc_SetSampleAttr(dc, 0,att,value);
}


/*
 * Set a point attr in a DataChunk using Nowc_pt coordinates x,y
 */
void
SetNowcPointAttr(dc, att, x, y)
	DataChunk *dc;
	char *att;
	short x, y;
/*
 * Assumes cvt_Origin has already been set
 */
{
	char value[30];
	float lat, lon;

	/* Convert the x,y center to lat/long */
	cvt_ToLatLon(FKM(x), FKM(y), &lat, &lon);
	sprintf(value,"%5.4f %5.4f",lat, lon);
	dc_SetSampleAttr(dc, 0,att,value);
}



/*
 * Translate an array of Nowc_pt points into lat/long and
 * return an array of locns to which the lat/long has been
 * appended starting at position start.
 */
void
AppendLocations(locns, start, shortpts, npts)
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
 *
 * At the moment, we are just using the current system time
 * as the trigger. This could change...
 *
 * Prevents triggers from occurring more often than every 10 secs
 */
{
	static Location trigger = { 0.0, 0.0, 0.0 };
	static time_t last_trigger = 0;  /* Time last trigger sent */
	PlatformId trigger_id;
	DataChunk *tdc;
	ZebTime now;

	TC_SysToZt(Time(0), &now);
	if (TC_ZtToSys(&now) - last_trigger < 10)
		return;
	last_trigger = TC_ZtToSys(&now);
	tdc = dc_CreateDC(DCC_Boundary);
	trigger_id = ds_LookupPlatform(pr->platform);
	tdc->dc_Platform = trigger_id;
	dc_BndAdd(tdc, &now, trigger_id, &trigger, 1);
	if (!ds_Store(tdc, FALSE, 0, 0))
	{
		IngestLog(EF_PROBLEM,
		   "DataStore for rap_trigger failed at %s",
		   AscSysTime(TC_ZtToSys(&now),TC_Full));
	}
	/* Now delete the data just written to save space */
	ds_DeleteData(trigger_id, 0);
	dc_DestroyDC(tdc);
}


void
StoreBoundary(pr, dc, when, platid, locns, numlocns)
	ProductRecord *pr;
	DataChunk *dc;
	ZebTime *when;
	PlatformId platid;
	Location *locns;
	int numlocns;
/*
 * Takes care of adding the locns to the DC, and doing 
 * simple error checking and debug logging
 *
 * For the moment, we're going to cheat and use the current
 * system time as the DataChunk time to be sure the
 * time will be current and the data will be retrieved.
 */
{
	ZebTime now;

	TC_SysToZt(Time(0), &now);
	if (numlocns>0)
	{
		dc_BndAdd(dc, &now, platid, locns, numlocns);
		if (DumpDataChunks) dc_DumpDC(dc);
		if (!ds_Store(dc, FALSE, 0, 0))
		{
			IngestLog(EF_PROBLEM,
			"platform %s id %d, RAP %d, time %s: DataStore failed",
			pr->platform, platid, pr->rap_id, 
			AscSysTime(Time(0), TC_Full));
		}
	}
	else
	{
		IngestLog(EF_PROBLEM,
			"platform %s id %d, RAP %d: empty product",
			pr->platform, platid, pr->rap_id);
	}
}



/* IDs: 9203, 9204, 9225, 9226, 9229, 9230
 * For extrapolations, data is ingested from the platform
 * according to pr->platform postfixed with ".<id_num>"
 * from the Extrapolation_header.  <id_num> will be called
 * the subid.  For now, the extrapolation
 * boundaries are connected so that they can be ingested
 * as one boundary data-chunk.  In the future a single
 * extrap time may be chosen.  Other info, such as core 
 * extrap time, dbz, and center coordinates, will be stored
 * as global attributes in the data-chunk.
 *
 * The time assigned to the DataChunk is the generated time
 * from exhead->machine_time. 
 */

/*ARGSUSED*/
int 
IngestExtrapolations( pr, mtype, messin, lenin)
    ProductRecord *pr;
    int		mtype;
    char  	*messin;
    int   	lenin;
{
	Extrapolation_header *exhead;
	Ex_boundry	*bound_head;
	Ex_core		*core_head;
	Nowc_pt		*nowc;
	long		id_num;

	DataChunk *dc;
	ZebTime when;
	PlatformId platid;
	Location *locns;
	int n; 
	int npts, totpts; 
	short x_ctr, y_ctr;
	int nextraps;
	int subid;
	char	*ptr;
	static char	att[50];
	char	*platform;
	time_t	gen_time, extrap_time;

       	/* raw_structure */
       	exhead = (Extrapolation_header *) messin;
      	id_num = exhead->id_num;
       	ptr = messin + sizeof( Extrapolation_header);
      	nextraps = exhead->num_extraps;
      	gen_time = Ctime6( exhead->machine_time);

	if (!(platform = GetSubPlatform(pr, id_num, &subid, &platid)))
		return(0);

	IngestLog(EF_INFO,
   	   "%s(%d): %d extraps, id_num %li, subid %d, generated %s",
	   pr->platform, pr->rap_id, nextraps, id_num, subid, 
	   AscSysTime(gen_time, TC_Full));

	dc = dc_CreateDC(DCC_Boundary);
	dc->dc_Platform = platid;
	
	cvt_Origin(pr->origin_lat,pr->origin_lon);

	totpts = 0;	/* The total # pts in all extraps */
      	for (n=0; n <nextraps; n++)
	{
            if (pr->rap_id == 9204)
	    {
	    	core_head = (Ex_core *) ptr;
	    	ptr += sizeof(Ex_core);
	    	nowc = (Nowc_pt *) ptr;
            	npts = core_head->npts;
	    	ptr += npts * sizeof( Nowc_pt);
	    	extrap_time = Ctime6(core_head->extrap_time);
		x_ctr = core_head->x_ctr;
		y_ctr = core_head->y_ctr;
		SetIntAttr(dc, "dbz", (int)(core_head->dbz));
	    }
            else
	    {
	    	bound_head = (Ex_boundry *) ptr;
	    	ptr += sizeof(Ex_boundry);
	    	nowc = (Nowc_pt *) ptr;
            	npts = bound_head->npts;
	    	ptr += npts * sizeof( Nowc_pt);
	    	extrap_time = Ctime6(bound_head->extrap_time);
		x_ctr = bound_head->x_ctr;
		y_ctr = bound_head->y_ctr;
	    }

	    SetNowcPointAttr(dc, "center", x_ctr, y_ctr);
	    sprintf(att,"extrap_time_%i",n);
	    dc_SetSampleAttr(dc, 0, att, AscSysTime(extrap_time, TC_Full));

	    IngestLog(EF_INFO,
	       "%s(%d): extrap #%d has %i pts, time: %s",
	       platform, pr->rap_id, n, npts, AscSysTime(extrap_time, TC_Full));

	    /*
	     * Add points to our locns buffer
	     */
	    AppendLocations(&locns, totpts, nowc, npts);
	    totpts += npts;

	}
	/*
	 * We have now stored all of the points of each of
	 * the extrapolations into one continuous array of locns
	 */
	TC_SysToZt(gen_time, &when);
	StoreBoundary(pr, dc, &when, platid, locns, totpts);
	TriggerDataStore(pr, &when, platid);
	dc_DestroyDC(dc);

      	return (0);
}



/*
 * Ingest RAP product types 9201(boundary), 9202(core) 
 */
/* ARGSUSED */
int 
IngestNowcasts( pr, mtype, messin, lenin)
    ProductRecord *pr;
    int		mtype;
    char  	*messin;
    int   	lenin;
{
	Bdry_header	*bhead;
	Core_header	*chead;
	Nowc_pt		*nowc;
      	long		start_time, gen_time, exp_time;
	long		id_num;

	DataChunk 	*dc;
	ZebTime 	when;
	static char 	timestring[40];
	PlatformId 	platid;
	Location 	*locns;
	int 		npts;
	short 		x_ctr, y_ctr;
	int 		subid;
	char		*platform;

	dc = dc_CreateDC(DCC_Boundary);
	
	cvt_Origin(pr->origin_lat,pr->origin_lon);

	/*
	 * Decode the header to get the subid, time values, 
	 * number of points, and center coords.
	 * After this section, nowc will point to the points array,
	 * npts will be the # of points, x_ctr and y_ctr will
	 * be set, and subid will be set to id_num
	 */
	if (pr->rap_id == 9201)
	{
		bhead = (Bdry_header *) messin;
	        npts = bhead->npts;
		nowc = (Nowc_pt *) (messin + sizeof(Bdry_header));
		start_time = Ctime6( bhead->scan_time);
		gen_time = Ctime6( bhead->machine_time);
	        exp_time = -1;
		id_num = bhead->id_num;
		x_ctr = bhead->x_ctr;
		y_ctr = bhead->y_ctr;
	}
	else if (pr->rap_id == 9202)
	{
		chead = (Core_header *) messin;
	        npts = chead->npts;
	      	nowc = (Nowc_pt *) (messin + sizeof(Core_header));
		start_time = Ctime6( chead->scan_time);
		gen_time = Ctime6( chead->machine_time);
	        exp_time = -1;
		id_num = chead->id_num;
		x_ctr = chead->x_ctr;
		y_ctr = chead->y_ctr;
	}
	else
		return (0);

	SetNowcPointAttr(dc, "center", x_ctr, y_ctr);

	if (!(platform = GetSubPlatform(pr, id_num, &subid, &platid)))
	{
		dc_DestroyDC(dc);
		return(0);
	}

	/* Since AscSysTime can't be called twice in same function,
	 * we must store one of the times before passing to Log */
	(void)strcpy(timestring, AscSysTime(start_time, TC_Full));
	IngestLog(EF_INFO,
   	   "%s(%d): id_num %li, subid %d, start: %s, generated %s",
	   platform, pr->rap_id, id_num, subid, 
	   timestring, AscSysTime(gen_time, TC_Full));

	dc->dc_Platform = platid;

	AppendLocations(&locns, 0, nowc, npts);

      	pr->last = gen_time;
	TC_SysToZt(gen_time, &when);
	StoreBoundary(pr, dc, &when, platid, locns, npts);
	TriggerDataStore(pr, &when, platid);
	dc_DestroyDC(dc);
	return (0);
}



/*
 * Ingest HG Nowcasts, 9205.  Assigns a locally generated id
 * from 1 to 30 for each region contained in the message
 */
/* ARGSUSED */
int 
IngestNowcastRegions( pr, mtype, messin, lenin)
    ProductRecord *pr;
    int		mtype;
    char  	*messin;
    int   	lenin;
{
	Nowcast	        *nhead;
	Nowcast_region  *nregion;
	Nowc_pt		*nowc;
	int		nregions = 0;
      	char		*ptr;
      	long		start_time, gen_time, exp_time;

	DataChunk 	*dc;
	ZebTime 	when;
	PlatformId 	platid;
	Location 	*locns;
      	int 		i;
	int 		subid;
	char		*platform;

	cvt_Origin(pr->origin_lat,pr->origin_lon);

	/*
	 * Decode the header to get the subid, time values, 
	 * number of points, and center coords.
	 * After this section, nowc will point to the points array,
	 * npts will be the # of points, x_ctr and y_ctr will
	 * be set
	 */
	if (pr->rap_id == 9205)
	{
		nhead = (Nowcast *) messin;
		nregions = *(int *)(&nhead->num_region);
		gen_time = Ctime6( nhead->entered_time);
		exp_time = Ctime6( nhead->valid_time);
	}
	else
		return (0);

	if (nregions == 0)
	{
		IngestLog(EF_PROBLEM, 
		   "%s(%d): Nowcast product has no regions",
		   pr->platform, pr->rap_id);
		return(0);
	}

	IngestLog(EF_INFO,
   	   "%s(%d): %d regions, type %li, gentime %s", 
	   pr->platform, pr->rap_id, nregions,  nhead->nowcast_type,
	   AscSysTime(gen_time, TC_Full));

	TC_SysToZt(gen_time, &when);
	/* 
	 * Set our pointer to beginning of nowcast regions,
	 * following nowcast header
	 */
	ptr = messin + sizeof( Nowcast);
	for (i=0; i< nregions; i++)
	{
		nregion = (Nowcast_region *) ptr;
		ptr += sizeof(Nowcast_region);
		nowc = (Nowc_pt *) ptr;
		ptr += nregion->npts * sizeof( Nowc_pt);

		dc = dc_CreateDC(DCC_Boundary);
		if (!(platform = GetSubPlatform(pr, nregion->region_id,
						&subid, &platid)))
		{
			dc_DestroyDC(dc);
			continue;
		}
		dc->dc_Platform = platid;
	
		IngestLog(EF_INFO,
		   "%s(%d): region #%i, %i points, type %li, region_id %li",
		   platform, pr->rap_id, i, nregion->npts,
		   nregion->region_type, nregion->region_id);

		AppendLocations(&locns, 0, nowc, (int)(nregion->npts));
		StoreBoundary(pr, dc, &when, platid, locns, (int)(nregion->npts));
		TriggerDataStore(pr, &when, platid);
		dc_DestroyDC(dc);
	}

      	pr->last = gen_time;
	return (0);
}



/*
 * For RAP ids 9227, 9228, 9231, and 9232
 */
/* ARGSUSED */
int 
IngestForecasts( pr, mtype, messin, lenin )
	ProductRecord *pr;
	int mtype;
	char *messin;
	int   	lenin;
{
      	Nowc_forecast_header *fhead;
      	Nowc_pt		*nowc;

	Location	*locns;
	DataChunk	*dc;
	ZebTime		when;
	int		subid, platid;
      	int		npts;
      	char		*ptr;
	char 		*platform;
      	time_t		start_time, gen_time, expire_time;
	int		period;
	int		probability;

       	/* raw_structure */
       	fhead = (Nowc_forecast_header *) messin;
       	ptr = messin + sizeof( Nowc_forecast_header);
	nowc = (Nowc_pt *) ptr;
	npts = fhead->n_points;

      	if ((fhead->id_number < 0) || (npts <= 0))
	{
	    IngestLog(EF_INFO,"%s(%d): Null product ignored",
		pr->platform, pr->rap_id);
            return (0);
	}

	if (!(platform = GetSubPlatform(pr, fhead->id_number, 
					&subid, &platid)))
		return(0);

	dc = dc_CreateDC(DCC_Boundary);
	dc->dc_Platform = platid;

	cvt_Origin(pr->origin_lat,pr->origin_lon);

	/* The assignments convert the structure field to int */
	probability = fhead->probability;
	period = fhead->period;		/* Forecast time in mins */
	SetIntAttr(dc, "probability", probability);
	SetIntAttr(dc, "period", period);
	SetNowcPointAttr(dc, "center", fhead->x_ctr, fhead->y_ctr);

      	start_time = Ctime6( fhead->creation_time);
      	gen_time = Ctime6( fhead->creation_time);
	expire_time = start_time + 3600;

	IngestLog(EF_INFO,"%s(%d): %d points, id_number %li, generated %s",
		platform, pr->rap_id, npts, fhead->id_number,
		AscSysTime(gen_time, TC_Full));

	AppendLocations(&locns, 0, nowc, npts);
      	pr->last = gen_time;
	TC_SysToZt(gen_time, &when);
	StoreBoundary(pr, dc, &when, platid, locns, npts);
	TriggerDataStore(pr, &when, platid);
	dc_DestroyDC(dc);
	return (0);
}



/* ARGSUSED */
int
IngestPolylineProduct(pr, mtype, msg, mlen)
	ProductRecord *pr;
	int mtype;
	char *msg;
	int mlen;
/*
 * Split a PDBproduct up into its header, pts, and text,
 * and store it in a DCC_Boundary-class DataChunk and send
 * it off to the DataStore
 *
 * Revised: Rather than use the supplied id, give every product
 * of a given type a new id so that products (like storm tracks)
 * which use the same id's for several products won't overwrite
 * each other.
 * The last used sub-platform-id (<plat>.<subid>) is now stored
 * in the product record.
 */
{
	DataChunk *dc;
	PlatformId platid;
	PDBproduct *pd;
	Location *locns;
	char *platform;
	char attr[30];
	int subid;
	PDBpt *pdbpts, *ptr;
	int npts;
	ZebTime when;

	pd = (PDBproduct *)msg;
	if (pd->npts == 0)		/* Check for an empty product */
	{
		IngestLog(EF_DEBUG,"%s, empty polyline product ignored.",
			  pr->platform);
		return(0);
	}

	platform = GetSubPlatform(pr, pr->next_id, &subid, &platid);
	++(pr->next_id);
	if (!platform)
		return(0);

	dc = dc_CreateDC(DCC_Boundary);
	dc->dc_Platform = platid;
	/* 
	 * Add the fields of the PDBproduct as attributes
	 */
	sprintf(attr,"%hu",pd->line_type);
	dc_SetSampleAttr(dc, 0, "line-type", attr);
	sprintf(attr,"%li",pd->id);
	dc_SetSampleAttr(dc, 0, "id", attr);
	sprintf(attr,"%li",pd->subid);
	dc_SetSampleAttr(dc, 0, "subid", attr);
	dc_SetSampleAttr(dc, 0, "start-time", 
			 AscSysTime(pd->start_time, TC_Full));
	dc_SetSampleAttr(dc, 0, "expire-time", 
			 AscSysTime(pd->expire_time, TC_Full));
	dc_SetSampleAttr(dc, 0, "generate-time", 
			 AscSysTime(pd->generate_time, TC_Full));
	/* 
	 * This is the value the ov_Boundary() routine will 
	 * look for to distinguish multiple polylines
	 */
	dc_SetSampleAttr(dc, 0, "penup-point", "0.0");

	cvt_Origin(pr->origin_lat, pr->origin_lon);
	pdbpts = (PDBpt *)(msg + sizeof(PDBproduct));
	locns = GetLocnsBuffer(pd->npts);
	for (npts = 0, ptr = pdbpts; npts < pd->npts; ++npts, ++ptr)
	{
# ifdef PLINE_QC
		fprintf(stderr,"%i point: ",npts);
# endif /* PLINE_QC */
		if (ptr->x != PDBPT_PENUP)
		{
			cvt_ToLatLon(FKM(ptr->x),
				     FKM(ptr->y),
				     &(locns[npts].l_lat),
				     &(locns[npts].l_lon));
			locns[npts].l_alt = 0.0;
# ifdef PLINE_QC
			/*
			 * Do some QC:
			 */
			if (FKM(ptr->x) > 140 ||
			    FKM(ptr->x) < -50 ||
			    FKM(ptr->y) > 100 ||
			    FKM(ptr->y) < -100)
			{
			   fprintf(stderr,
				"Bad point from product: %4.2f %4.2f",
				FKM(ptr->x),FKM(ptr->y));
			   fprintf(stderr," ==> %f lat %f lon\n",
				locns[npts].l_lat,locns[npts].l_lon);
			}
			else
			{
			   fprintf(stderr,
				"Point from product: %4.2f %4.2f",
				FKM(ptr->x),FKM(ptr->y));
			   fprintf(stderr," ==> %f lat %f lon\n",
				locns[npts].l_lat,locns[npts].l_lon);
			}
# endif /* PLINE_QC */
		}
		else
		{
		/*
		 * Otherwise insert a penup
		 */
# ifdef PLINE_QC
			fprintf(stderr,"Penup point.\n");
# endif /* PLINE_QC */
			locns[npts].l_alt = 0.0;
			locns[npts].l_lat = 0.0;
			locns[npts].l_lon = 0.0;
		}
	}

	/*
	 * Add the boundary to dc, store it, and trigger the trigger
	 * platform.
	 */
	TC_SysToZt(pd->start_time, &when);
	StoreBoundary(pr, dc, &when, platid, locns, npts);
	/*
	 * Only trigger infrequent platforms, those other than acs, act,
	 * llwas, hal...
	 */
	switch(pr->pdb_id) {
	   case 11: /* ACT */
	   case 17: /* ACS */
	   case 13: /* LLWAS */
	   case 29: /* HAL */
		break;
	   default:
		TriggerDataStore(pr, &when, platid);
		break;
	}
	/*
	 * Then update our last-received field and destroy the DChunk
	 */
	pr->last = Time(0);
	dc_DestroyDC(dc);
	return(0);
}

