/*
 * $Id: tstracks.c,v 1.1 1993-06-28 05:38:29 granger Exp $
 *
 * Predecessor to a separate, raw, stand-alone storm-track Zeb ingestor.
 * Presently attempting to connect to storm track server and dump storm
 * data in ASCII format to see what we have to work with
 */

#if !defined(SABER) && !defined(lint)
static char rcsid[]="$Id: tstracks.c,v 1.1 1993-06-28 05:38:29 granger Exp $";
#endif

#include "common.h"
#undef ABS

#include "ingest.h"
#include "pdb_ingest.h"		/* For the ProductRecord type */
#include "pdbutils.h"		/* For PR and boundary DC utils */

extern bool cvt_Origin FP((double lat, double lon));
extern void cvt_ToLatLon FP((double x, double y, float *lat, float *lon));

static void DumpStormTracks FP((tdata_basic_index_t *index));
static void Usage FP((char *prog));
static void ParseOptions FP((int *argc, char *argv[]));
static PRptr InitProduct FP(( PRptr pr, int pdbid, long rapid, char *code,
				char *name ));
static void InitStormProducts ();
static void IngestStormTracks FP((tdata_basic_index_t *index));
static void IngestPresentTrack FP((ZebTime *when, float factor,
				   tdata_basic_track_entry_t *t_entry));
static void AppendTrackEntry FP((
	Location **locns,
	int *start,
	float factor,
	tdata_basic_track_entry_t *t_entry));

/***** Locations *****/
static float Mhr_loc_long = -104.7568;	/* long - MHR */
static float Mhr_loc_lat = 39.8783;	/* lat - MHR */

/*
 * This must publicly available since some of the routines in pdbutils
 * require this value
 */
int MaxSubplatforms = 15;

/*
 * Define a 3-element array to hold the three storm products about
 * which we will be concerned: STR, STP, and STF.  The indices to
 * each of these elements will be fixed by cpp macro defines.
 */
ProductRecord Products[3];

#define P_STR 0
#define P_STF 1
#define P_STP 2


#define ERR_CONTROL_SETTING "ON STD OFF SLOG OFF LOG ON USR"

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
int
main(argc, argv)
   int         argc;
   char       *argv[];
{
   tdata_basic_index_t *index;
   extern void LogERRtoEL();
   int err;
   long port;
   char *server;

   /*
    * Parse any ingest options on command line
    */
   IngestParseOptions(&argc, argv, Usage);
   ParseOptions(&argc, argv);

   /*
    * The remaining arguments should be the host and the port number
    */
   if (argc != 3)
   {
	printf("Must specify a server and port number!\n");
	Usage(argv[0]);
	exit(1);
   }
   server = argv[1];
   port = atol(argv[2]);

   IngestInitialize("StormTracks");

   /*
    * Initialize the error module.  Disables command line options
    * for ERR by not passing argc or argv.  We want to override
    * the options here, and set up our own handler...
    */
   ERRinit(argv[0], 0, NULL);
   ERRcontrol(ERR_CONTROL_SETTING);
   err = ERRuser(LogERRtoEL);
   ERRuserActive(err, 1); 	/* Make sure our handler is active */

   /*
    * We're not using TS_connect() so that we can bypass the server mapper
    * stuff; we'll connect directly to a storm track host
    */
   if (Connect_to_tserver(server,port) == 0)
   {
      IngestLog(EF_INFO, "Connected to track server on %s,%d",
	      server, port);
   }
   else
   {
      IngestLog(EF_PROBLEM,"Could not connect to track server %s on %d",
		server, port);
      return(1);
   }

   /*
    * Initialize our table of products
    */
   InitStormProducts();

   /* poll for track server data */
   while (1)
   {
      IngestLog(EF_DEBUG,"Polling track server for new data...");
      if ((TS_check_new_data()))
      {
         index = TS_getdata();
   
         if (index == NULL)
            IngestLog(EF_PROBLEM,"TS_getdata() failed");
         else
         {
            IngestLog(EF_DEBUG,"TS_getdata() n_complex_tracks = %li",
               index->header.n_complex_tracks);
            IngestStormTracks(index);
         }
      }
      else
      {
   	sleep(30);
      }
   }
}
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/


static void
ParseOptions(argc, argv)
	int *argc;
	char *argv[];
{
	/*
	 * No options yet...
	 */
}


static void
Usage(prog)
   char *prog;
{
   printf("%s [options] server port\n",prog);
   IngestUsage();
   printf("\nExample:\n%s -log all -chunks tornado.rap 53000\n",prog);
}


static void
DumpBasicHeader(h)
	tdata_basic_header_t *h;
{
	double factor;

	factor = (double) h->factor;
	printf("header.factor = %li\n",h->factor);
	printf("header.dbz_threshold = %f dbz\n",
		(float)h->dbz_threshold/factor);
	printf("header.volume_threshold = %f km^3\n",
		(float)h->volume_threshold/factor);
	printf("header.max_tracking_speed = %f km/h\n",
		(float)h->max_tracking_speed/factor);
	printf("header.max_delta_time = %f minutes\n",
		(float)h->max_delta_time/factor);
	printf("header.n_complex_tracks = %li\n",
		h->n_complex_tracks);
	printf("header.time = %s",
		ctime(&h->time));
	printf("header.data_start_time = %s",
		ctime(&h->data_start_time));
	printf("header.data_end_time = %s",
		ctime(&h->data_end_time));
}


static void
DumpComplexParams(c_params)
	tdata_basic_complex_params_t *c_params;
{
	printf("	complex_track_num = %li\n",
			c_params->complex_track_num);
	printf("	n_simple_tracks = %li\n", 
			c_params->n_simple_tracks);
	printf("	start_time = %s",ctime(&c_params->start_time));
	printf("	end_time = %s",ctime(&c_params->end_time));
	printf("	duration_in_scans = %li\n",
			c_params->duration_in_scans);
	printf("	duration_in_secs = %li secs\n",
			c_params->duration_in_secs);
	printf("	volume_at_start_of_sampling = %li km^3\n",
			c_params->volume_at_start_of_sampling);
	printf("	volume_at_end_of_sampling = %li km^3\n",
			c_params->volume_at_end_of_sampling);
}



static void
DumpSimpleParams(s_params)
	tdata_basic_simple_params_t *s_params;
{
	short i;

	printf("		simple_track_num = %li\n",
				s_params->simple_track_num);
	printf("		complex_track_num = %li\n",
				s_params->complex_track_num);
	printf("		start_time = %s",
				ctime(&s_params->start_time));
	printf("		end_time = %s",ctime(&s_params->end_time));
	printf("		duration_in_scans = %li\n",
				s_params->duration_in_scans);
	printf("		duration_in_secs = %li secs\n",
				s_params->duration_in_secs);
	printf("		Parents: ");
	for (i=0; i<s_params->nparents; ++i)
		printf("%6li",s_params->parent[i]);
	printf("\n");
	printf("		Children: ");
	for (i=0; i<s_params->nchildren; ++i)
		printf("%6li",s_params->child[i]);
	printf("\n");
}


static void
DumpTrackEntry(t_entry, factor)
	tdata_basic_track_entry_t *t_entry;
	double factor;
{
	static char *indent = "			"; /* 3 tabs */

	printf("%ssimple_track_num = %li\n",
		indent, t_entry->simple_track_num);
	printf("%scomplex_track_num = %li\n",
		indent, t_entry->complex_track_num);
	printf("%stime = %s",indent,ctime(&t_entry->time));
	printf("%shistory_in_scans = %li\n",
		indent,t_entry->history_in_scans);
	printf("%sdatum_longitude = %f deg long\n",
		indent, (float)t_entry->datum_longitude/factor);
	printf("%sdatum_latitude = %f deg lat\n",
		indent, (float)t_entry->datum_latitude/factor);

#if defined(__STDC__) || defined(__GNUC__)
#define Pfield(field,units) printf("%s%s = %f %s\n",indent,#field, \
				   (float)t_entry->field/factor, units)
#else
#define Pfield(field,units) printf("%s%s = %f %s\n",indent,"field ?", \
				   (float)t_entry->field/factor, units)
#endif
	
	Pfield(refl_centroid_x,"km");
	Pfield(refl_centroid_y,"km");
	Pfield(refl_centroid_z,"km");

	Pfield(top,"km");
	Pfield(base,"km");
	Pfield(volume,"km^3");
	Pfield(area_mean,"km^2");
	Pfield(rain_flux,"m^3/sec");
	Pfield(mass,"ktons");

	Pfield(projected_area,"km^2");
	Pfield(projected_area_centroid_x,"km");
	Pfield(projected_area_centroid_y,"km");
	Pfield(projected_area_orientation,"degT");
	Pfield(projected_area_minor_radius,"km");
	Pfield(projected_area_major_radius,"km");

	Pfield(projected_area_centroid_dx_dt,"km/hr");
	Pfield(projected_area_centroid_dy_dt,"km/hr");
	Pfield(dvolume_dt,"km3/hr");
	Pfield(dprojected_area_dt,"km2/hr");
}



static void  
DumpStormTracks(index)
   tdata_basic_index_t *index;

{
      long icomplex, isimple, ientry;
      double factor;

      tdata_basic_complex_params_t *c_params;
      tdata_basic_simple_params_t **s_params_ptr;
      tdata_basic_track_entry_t **t_entry_ptr;
      tdata_basic_track_entry_t ***t_entry_ptr_ptr;

      DumpBasicHeader(&index->header);
      factor = (double)index->header.factor;

      if (index->header.n_complex_tracks <= 0)
      {
	 printf ("Index received with 0 or less complex tracks.\n");
	 return;
      }

      c_params = index->complex_params;
      s_params_ptr = index->simple_params;
      t_entry_ptr_ptr = index->track_entry;

      for (icomplex = 0; icomplex < index->header.n_complex_tracks; 
			 icomplex++)
      {
	 tdata_basic_simple_params_t *s_params = *s_params_ptr;

	 printf("Complex track #%d:\n",icomplex);
	 DumpComplexParams(c_params);

	 t_entry_ptr = *t_entry_ptr_ptr;

	 for (isimple = 0; isimple < c_params->n_simple_tracks; isimple++)
	 {
	    tdata_basic_track_entry_t *t_entry = *t_entry_ptr;

	    printf("	Simple track #%d:\n",isimple);
	    DumpSimpleParams(s_params);

	    for (ientry = 0; ientry < s_params->duration_in_scans; ientry++)
	    {
		printf("\n		Track entry #%d:\n",ientry);
		DumpTrackEntry(t_entry, factor);
		t_entry++;
	    }

	    t_entry_ptr++;
	    s_params++;
	 }			/* isimple */

	 t_entry_ptr_ptr++;
	 s_params_ptr++;
	 c_params++;
      }				/* icomplex */
      fflush(stdout);
}



static void  
IngestStormTracks(index)
   tdata_basic_index_t *index;
{
      long icomplex, isimple, ientry;
      double factor;
      long idnum;

      DataChunk *dc;
      char *platform;
      PlatformId platid;
      int subid;
      ProductRecord *pr;
      ZebTime when;

      Location *locns;
      int nlocns;

      tdata_basic_complex_params_t *c_params;
      tdata_basic_simple_params_t **s_params_ptr;
      tdata_basic_track_entry_t **t_entry_ptr;
      tdata_basic_track_entry_t ***t_entry_ptr_ptr;

      factor = (double)index->header.factor;

      if (index->header.n_complex_tracks <= 0)
      {
	 IngestLog(EF_INFO, 
	    "received 0 or less complex tracks. Ignoring.");
	 return;
      }

      IngestLog(EF_DEBUG,"ingesting %li complex tracks for time %s",
	 index->header.n_complex_tracks, 
	 AscSysTime(index->header.time, TC_Full));

      /*
       * This is the time we'll use for all chunks ingested here:
       * the current system time
       */
      TC_SysToZt(Time(0), &when);

      c_params = index->complex_params;
      s_params_ptr = index->simple_params;
      t_entry_ptr_ptr = index->track_entry;

      for (icomplex = 0; icomplex < index->header.n_complex_tracks; 
	   icomplex++)
      {
	 tdata_basic_simple_params_t *s_params = *s_params_ptr;

	 IngestLog(EF_DEBUG,
		"complex track #%d, c_track_num %li, %d simple tracks",
		icomplex, 
		c_params->complex_track_num, 
		c_params->n_simple_tracks);

	 IngestLog(EF_DEBUG,
	    " searching %li simple tracks for current track entries",
	    c_params->n_simple_tracks);

	 t_entry_ptr = *t_entry_ptr_ptr;

	 for (isimple = 0; isimple < c_params->n_simple_tracks; isimple++)
	 {
	    tdata_basic_track_entry_t *t_entry = *t_entry_ptr;

	    IngestLog(EF_DEBUG,"  simple track #%d, %d track entries",
		isimple, s_params->duration_in_scans);

	    /* 
	     * Now find the track entry we want from this simple track
	     */
	    for (ientry = 0; ientry < s_params->duration_in_scans; ientry++)
	    {
		/*
		 * Just take the first entry we find that matches the current
		 * time of this complex track
		 */
		if (t_entry->time == index->header.time)
		{
			IngestLog(EF_DEBUG,
			   "   found track entry #%d, simple track #%d, time %s",
			   ientry, isimple,
			   AscSysTime(t_entry->time, TC_Full));
	
			/* 
			 * We have found a recent track entry, so ingest it
			 */
			IngestPresentTrack(&when, (float)factor, t_entry);
	
			/*
			 * Here we would also call functions to do the 
			 * extrapolations and predictions and generate data 
			 * for the other two platforms
			 */

			break;
		}		/* else keep looking */

		t_entry++;
	    }			/* ientry */

	    t_entry_ptr++;
	    s_params++;
	 }			/* isimple */

	 t_entry_ptr_ptr++;
	 s_params_ptr++;
	 c_params++;
      }				/* icomplex */
}


/*
 * Ingest STR data from the given track entry
 * Uses the given time as the ingest time for the data chunk,
 * also stores all of the attribute data associated with 
 * the track into the DataChunk
 */
static void
IngestPresentTrack(when, factor, t_entry)
	ZebTime *when;
	float factor;
	tdata_basic_track_entry_t *t_entry;
{
	DataChunk *dc;
	long nlocns;
	Location *locns;
	ProductRecord *pr;
	PlatformId platid;
	int subid;

	/*
	 * Get our product record from the global table
	 */
	pr = &(Products[P_STR]);

	/*
	 * Get a subplatform which hopefully corresponds to a previously
	 * received simple strom track with an identical simple_track_num
	 */
	if (!GetSubPlatform(pr, t_entry->simple_track_num, &subid, &platid))
	{
		return;
	}

	dc = dc_CreateDC(DCC_Boundary);
	dc->dc_Platform = platid;
	dc_SetSampleAttr(dc, 0, AttPenupPoint, "0.0");

	/*
	 * Other attributes to store in this data chunk, unique to
	 * this storm cell
	 */
	SetFloatAttr(dc, 0, AttStormTop, 	/* storm top in km */
	   (float)(t_entry->top/factor));
	
	/*
	 * Now actually add the shape data
	 */
	nlocns = 0;
	AppendTrackEntry(&locns, &nlocns, (float)factor, t_entry);

	dc_BndAdd(dc, when, platid, locns, nlocns);
	ds_Store(dc, FALSE, 0, 0);
        TriggerDataStore(pr, when, platid);
	dc_DestroyDC(dc);
}



/*
 * Append the given track entry as a diamond shape to the locns
 * array starting at '*start'.  Return the new number of points
 * in start.  Automatically makes sure there is space in the
 * locns buffer, and appends a penup-point, 0.0.
 */
static void
AppendTrackEntry(locns, start, factor, t_entry)
	Location **locns;
	int *start;
	float factor;
	tdata_basic_track_entry_t *t_entry;
{
	float origin_lat, origin_lon;
	float center_lat, center_lon;
	float x,y;
	float rotation;  		/* degrees */
	float xradius, yradius;  	/* km */

	/*
	 * Set the center for our conversions
	 */
	origin_lat = (float)t_entry->datum_latitude/factor;
	origin_lon = (float)t_entry->datum_longitude/factor;
	cvt_Origin(origin_lat, origin_lon);

	/*
	 * Convert the center of the projected area to lat,lon
	 */
	x = (float)t_entry->projected_area_centroid_x/factor;
	y = (float)t_entry->projected_area_centroid_y/factor;
	cvt_ToLatLon(x, y, &center_lat, &center_lon);
	
	/*
	 * Convert the track entry fields from long to float using factor
	 */
	xradius = (float)t_entry->projected_area_major_radius/factor;
	yradius = (float)t_entry->projected_area_minor_radius/factor;
	rotation = 90.0 - (float)t_entry->projected_area_orientation/factor;

	/*
	 * Add the actual diamond points, +1 penup
	 */
	Diamond(locns, start, center_lat, center_lon,
		xradius, yradius, rotation);
}




static PRptr
InitProduct(pr, pdbid, rapid, code, name)
	ProductRecord *pr;
	int pdbid;
	long rapid;
	char *code;
	char *name;
/*
 * Initialize a ProductRecord.
 * Returns a pointer to the record.
 * Overwrite with new code and name values if the product
 * already exists.
 * ALL product records must be created through here to be
 * initialized correctly
 */
{
	int i;

	if ((pdbid < 1) || (pdbid > MAX_PROD_ID))
	{
	   IngestLog(EF_PROBLEM, 
	      "Illegal product id %d inserting code %s, name %s",
	      pdbid, code, name);
	   return(NULL);
	}

	   pr->pdb_id = pdbid;
	   pr->rap_id = rapid;
	   pr->next_id = 0;
	   strcpy(pr->desc,"No Description");
	   strcpy(pr->platform,PLATFORM_PREFIX);
	   strcat(pr->platform,code);
	   StrToLower(pr->platform);
	   pr->platid = ds_LookupPlatform(pr->platform);
	   pr->format = PDB_FORMAT_RAW;
	   pr->origin_lat = MHR_ORIGIN_LAT;
	   pr->origin_lon = MHR_ORIGIN_LON;
	   pr->angle = 0.0;
	   pr->last = 0;
	   pr->requested = 0;
	   pr->status = ProdStatInactive;
	   pr->raw_handler = NULL;
	   pr->line_handler = NULL;
	/*
	 * Initialize the sub-platform LRU arrays and fields
	 */
	   for (i=0; i< MaxSubplatforms; ++i)
	   {
		pr->id_numbers[i] = -1;
		pr->next_oldest[i] = i+1;
	   }
	   pr->index_oldest = 0;
	   pr->index_newest = MaxSubplatforms-1;
	   pr->next_oldest[pr->index_newest] = pr->index_oldest;
	   IngestLog(EF_DEBUG,
	    "Inserting %s %d, rap_id %d, platform %s, platid %d",
	    pr->code, pr->pdb_id, pr->rap_id, pr->platform, pr->platid);

	strncpy(pr->code, code, PROD_CODE_LEN);
	pr->code[PROD_CODE_LEN-1] = '\0';
	strncpy(pr->name, name, PROD_NAME_LEN);
	pr->name[PROD_NAME_LEN-1] = '\0';

	return(pr);
}


static void
InitStormProducts()
{
/*
 * Initialize each of the storm products with whatever
 * info we'll need
 */
	InitProduct(&Products[P_STR], 30, 10098, "STR", "StormTracks");
	InitProduct(&Products[P_STP], 31, 10099, "STP", "StormPredict");
	InitProduct(&Products[P_STF], 32, 10097, "STF", "StormForecast");
}


