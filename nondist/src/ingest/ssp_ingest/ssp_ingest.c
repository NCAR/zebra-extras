/*
 * ssp_ingest.c - ingest sample SSP data via N-space DataChunk
 * Created: Thu Apr 13 13:58:45 1995 (Sean T. Moore)
 */

#include <ingest.h>

# define INGEST_NAME    "ssp_ingest"
# define PLATFORM       "sspch1"
# define NWAVELEN       55
# define DATAFILE       "test-data"
# define NSAMPLE        1	/* save this many to datachunk before ingesting */
# define SLEEPTIME      10	/* seconds */
# define BADVALUE	-99999.989
# define PROBLEM        1
# define LAT           35.0
# define LON           -120.0
# define ALT           228.0

# define TRUNC_SECONDS

/*
 * Declare Routines.
 */
double drand48();
void srand48();
int printf();
int fscanf();
int fclose();
int sleep();

/*
 * Declare Globals.
 */

/*
 * Program
 */

int main (argc, argv)
     int	argc;
     char	**argv;
{
  /* ----------------------- declarations ------------------- */
  int	i, j, n, k;
  FILE	*fptr;
  int FIRST = TRUE;

  float wave_data[NWAVELEN];
  float ssprad[NWAVELEN];
  float ssplw;
  float sspsw;

  DataChunk	*dc = NULL;	
  ZebTime	when;		/* time of a sample */
  Location	locn;		/* location of a sample */
  FieldId	wave_id, ssprad_id, ssplw_id, sspsw_id;
  PlatformId	pid;

  char *progname  = argv[0];
  locn.l_lat = LAT + 0.01;
  locn.l_lon = LON + 0.001;
  locn.l_alt = ALT + 1.0;

  /* ----------------------------- main body ------------------ */

  /*
   * Check arguments.
   */

  IngestParseOptions(&argc, argv, IngestUsage);

  /*
   * Initialization.
   */

  IngestInitialize(INGEST_NAME);

  /*
   * Check the platform (Zeb DataStore must be running)
   */
  if ((pid = ds_LookupPlatform (PLATFORM)) == BadPlatform) {
    IngestLog (EF_EMERGENCY, "Bad platform '%s'.", PLATFORM);
    exit (PROBLEM);
  }
      
  /* 
   * Read the sample dummy data
   */
  if ((fptr = fopen (DATAFILE, "r")) == NULL) {
    printf("Could not open %s\n",DATAFILE);
    exit(1);
  }
  printf ("scanning... \n");
  for (i = 0; i < NWAVELEN; i++) {
    if (fscanf(fptr, "%d %f %f", &n, &wave_data[i], &ssprad[i]) == EOF) {
      printf("not enough data \n");
      exit(1);
    }
  }
  /* set the seed for random numbers */
  srand48(ssprad[0]);

  /*
   * Loop forever, generating dummy data and saving to datastore
   */
  while (TRUE) {
    
    /*
     * Create a datachunk for this platform
     */
    dc = dc_CreateDC(DCC_NSpace);
    dc->dc_Platform = ds_LookupPlatform(PLATFORM);
    
    /*
     * Define wave dimension and the wave field with the same FieldId
     */
    wave_id = F_DeclareField("wave", "Wavelength (nm)", "nm");
    dc_NSDefineDimension(dc, wave_id, NWAVELEN);
    dc_NSDefineVariable(dc, wave_id, 1, &wave_id, /*is_static*/TRUE);
    
    /*
     * Define fields that are a function of time and wave 
     */
    ssprad_id = F_DeclareField("ssprad", 
			       "Spectral Radiance (W/(m2 sr um))",
			       "W/(m2 sr um)");
    dc_NSDefineVariable(dc, ssprad_id, 1, &wave_id, /*not_static*/FALSE);
    
    /* 
     * Define fields that are a function of time only
     */
    ssplw_id = F_DeclareField("ssplw", 
			      "Longwave Broadband Radiance (W/(m2 sr))",
			      "W/(m2 sr)");
    dc_NSDefineVariable(dc, ssplw_id, 0, NULL, /*not_static*/FALSE);
    sspsw_id = F_DeclareField("sspsw", 
			      "Shortwave Broadband Radiance (W/(m2 sr))",
			      "W/(m2 sr)");
    dc_NSDefineVariable(dc, sspsw_id, 0, NULL, /*not_static*/FALSE);
    
    /*
     * Close out the Data Chunk definition.
     * Data type defaults to float unless otherwise set.
     */
    dc_NSDefineComplete(dc);
    dc_SetBadval (dc, BADVALUE);
    
    
    /*
     * Add the static (time-independent) data
     */
    dc_NSAddStatic(dc, wave_id, (void *)wave_data);
    
    /*
     * Add the dynamic (time-dependent) data
     * Hint of data size now, to prevent multiple malloc's
     */
    dc_HintNSamples (dc, NSAMPLE, TRUE);
    for (i=0; i < NSAMPLE; ++i) {
      sleep(SLEEPTIME);
      for (j = 0; j < NWAVELEN / 4; j++) {
	k = (int) (drand48() * NWAVELEN) ;
	ssprad[k] = ssprad[k] + (float)((drand48() - 0.5) * 100.0);
      }
      ssplw = ssprad[0] + (float)((drand48() - 0.5) * 20.0);
      sspsw = ssprad[NWAVELEN-1] + (float)((drand48() - 0.5) * 10.0);
      locn.l_lat = locn.l_lat + 0.01;
      locn.l_lon = locn.l_lon + 0.001;
      locn.l_alt = locn.l_alt + 1.0;
      tl_Time (&when);		/* use current system time */
#ifdef TRUNC_SECONDS
      when.zt_MicroSec = 0;
#endif
      dc_NSAddSample(dc, &when, i, ssprad_id, ssprad);
      dc_NSAddSample(dc, &when, i, ssplw_id, &ssplw);
      dc_NSAddSample(dc, &when, i, sspsw_id, &sspsw);
      dc_SetLoc (dc, i, &locn);
    }
    /*
     * Store away datachunk and we are done
     */
    ds_StoreBlocks (dc, FIRST, NULL, 0);
    FIRST = FALSE;
    dc_DestroyDC (dc);
  }
}
