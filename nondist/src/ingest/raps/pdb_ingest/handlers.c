/*
 * handlers.c -- A collection of handlers for raw and polyline format
 *		 products.  Requires the routines in pdbutils.c.
 *
 * $Id: handlers.c,v 1.2 1993-03-24 22:58:28 granger Exp $
 */

#if !defined(saber) && !defined(lint)
static char rcsid[]="$Id: handlers.c,v 1.2 1993-03-24 22:58:28 granger Exp $";
#endif

/*------- For PDBS: ----*/

#include "common.h"
#undef ABS

/*------- For Zeb: -----*/

#include "ingest.h"
#include "pdb_ingest.h"		/* For the ProductRecord type */
#include "pdbutils.h"


static void
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



/* 
 * IDs: 9203, 9204, 9225, 9226, 9229, 9230
 *
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

	IngestLog(EF_DEBUG,
   	   "  %s(%d): %d extraps, id_num %li, subid %d, generated %s",
	   pr->code, pr->rap_id, nextraps, id_num, subid, 
	   AscSysTime(gen_time, TC_Full));

	dc = dc_CreateDC(DCC_Boundary);
	dc->dc_Platform = platid;

	SetIntAttr(dc, 0, AttId, (int)id_num);
	dc_SetSampleAttr(dc, 0, AttGenerateTime, 
	   AscSysTime(Ctime6(exhead->machine_time), TC_Full));
	
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
		SetIntAttr(dc, 0, AttDbz, (int)(core_head->dbz));
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

	    SetNowcCenterAttr(dc, x_ctr, y_ctr);
	    sprintf(att,"extrap-time-%i",n);
	    dc_SetSampleAttr(dc, 0, att, AscSysTime(extrap_time, TC_Full));

	    IngestLog(EF_DEBUG,
	       "  %s(%d): extrap #%d has %i pts, extrap_time = %s",
	       pr->code, pr->rap_id, n, npts, 
	       AscSysTime(extrap_time, TC_Full));

	    /*
	     * Add points to our locns buffer
	     */
	    AppendNowcLocations(&locns, totpts, nowc, npts);
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
      	long		start_time, gen_time;
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
		id_num = chead->id_num;
		x_ctr = chead->x_ctr;
		y_ctr = chead->y_ctr;
	}
	else
		return (0);

	SetIntAttr(dc, 0, AttId, (int)id_num);
	dc_SetSampleAttr(dc, 0, AttStartTime,
	   AscSysTime(start_time, TC_Full));
	dc_SetSampleAttr(dc, 0, AttGenerateTime,
	   AscSysTime(gen_time, TC_Full));
	SetNowcCenterAttr(dc, x_ctr, y_ctr);

	if (!(platform = GetSubPlatform(pr, id_num, &subid, &platid)))
	{
		dc_DestroyDC(dc);
		return(0);
	}

	/* Since AscSysTime can't be called twice in same function,
	 * we must store one of the times before passing to Log */
	(void)strcpy(timestring, AscSysTime(start_time, TC_Full));
	IngestLog(EF_DEBUG,
   	 "  %s(%d): id_num %li, subid %d, npts %d, start: %s",
	 pr->code, pr->rap_id, id_num, subid, npts,
	 timestring, AscSysTime(gen_time, TC_Full));

	dc->dc_Platform = platid;

	AppendNowcLocations(&locns, 0, nowc, npts);

	TC_SysToZt(gen_time, &when);
	StoreBoundary(pr, dc, &when, platid, locns, npts);
	TriggerDataStore(pr, &when, platid);
	dc_DestroyDC(dc);
	return (0);
}



/*
 * Ingest HG Nowcasts, 9205.
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
	nhead = (Nowcast *) messin;
	nregions = *(int *)(&nhead->num_region);
	gen_time = Ctime6( nhead->entered_time);
	exp_time = Ctime6( nhead->valid_time);
	start_time = Ctime6( nhead->issued_time );

	if (nregions == 0)
	{
		IngestLog(EF_PROBLEM, 
		   "%s(%d): Nowcast product has no regions",
		   pr->platform, pr->rap_id);
		return(0);
	}

	IngestLog(EF_DEBUG,
   	   "  %s(%d): %d regions, type %li, gen_time = %s", 
	   pr->code, pr->rap_id, nregions,  nhead->nowcast_type,
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
	
		IngestLog(EF_DEBUG,
		   "  %s(%d): region #%i, %i pts, type %li, region_id %li",
		   pr->code, pr->rap_id, i, nregion->npts,
		   nregion->region_type, nregion->region_id);

		dc_SetSampleAttr(dc, 0, AttStartTime, 
			AscSysTime(start_time, TC_Full));
		dc_SetSampleAttr(dc, 0, AttGenerateTime, 
			AscSysTime(gen_time, TC_Full));
		dc_SetSampleAttr(dc, 0, AttExpireTime, 
			AscSysTime(exp_time, TC_Full));
		SetIntAttr(dc, 0, AttNowcastType, (int)nhead->nowcast_type);
		SetIntAttr(dc, 0, AttProbability, (int)nhead->probability);
		SetIntAttr(dc, 0, AttRegionType, (int)nregion->region_type);
		SetIntAttr(dc, 0, AttRegionId, (int)nregion->region_id);

		AppendNowcLocations(&locns, 0, nowc, (int)(nregion->npts));
		StoreBoundary(pr, dc, &when, platid, 
			      locns, (int)(nregion->npts));
		TriggerDataStore(pr, &when, platid);
		dc_DestroyDC(dc);
	}

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
	    IngestLog(EF_DEBUG,"  %s(%d): Null product ignored",
		pr->code, pr->rap_id);
            return (0);
	}

	if (!(platform = GetSubPlatform(pr, fhead->id_number, 
					&subid, &platid)))
		return(0);

	dc = dc_CreateDC(DCC_Boundary);
	dc->dc_Platform = platid;

	cvt_Origin(pr->origin_lat,pr->origin_lon);

	probability = (int)fhead->probability;
	period = (int)fhead->period;		/* Forecast time in mins */
	SetIntAttr(dc, 0, AttId, (int)fhead->id_number);
	SetIntAttr(dc, 0, AttProbability, probability);
	SetIntAttr(dc, 0, AttPeriod, period);
	SetNowcCenterAttr(dc, fhead->x_ctr, fhead->y_ctr);

      	start_time = Ctime6( fhead->creation_time);
      	gen_time = Ctime6( fhead->creation_time);
	expire_time = start_time + 3600;

	IngestLog(EF_DEBUG,
	   "  %s(%d): %d pts, id_num %li, subid %i, gen_time = %s",
		pr->code, pr->rap_id, npts, fhead->id_number, subid,
		AscSysTime(gen_time, TC_Full));

	AppendNowcLocations(&locns, 0, nowc, npts);
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
	   IngestLog(EF_DEBUG,"  %s(%d): empty polyline product ignored.",
		  pr->code, pr->pdb_id);
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
	dc_SetSampleAttr(dc, 0, AttLineType, attr);
	sprintf(attr,"%li",pd->id);
	dc_SetSampleAttr(dc, 0, AttId, attr);
	sprintf(attr,"%li",pd->subid);
	dc_SetSampleAttr(dc, 0, AttSubid, attr);
	dc_SetSampleAttr(dc, 0, AttStartTime,
			 AscSysTime(pd->start_time, TC_Full));
	if (pd->expire_time >= 0)
		dc_SetSampleAttr(dc, 0, AttExpireTime,
			 AscSysTime(pd->expire_time, TC_Full));
	dc_SetSampleAttr(dc, 0, AttGenerateTime,
			 AscSysTime(pd->generate_time, TC_Full));
	/* 
	 * This is the value the ov_Boundary() routine will 
	 * look for to distinguish multiple polylines
	 */
	dc_SetSampleAttr(dc, 0, AttPenupPoint, "0.0");

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
	 * Then destroy the DataChunk
	 */
	dc_DestroyDC(dc);
	return(0);
}




