# include <math.h>
# include <message.h>
# include <DataStore.h>

/* knots to m/s */
# define MS_TO_KNOTS(x,badval)	(((x)==(badval)) ? (badval) : ((x) * 1.9425))
/* 28.75 degree mean magnetic deviation near Juneau (1985) */
# define TRUE_TO_MAG(x,badval)	(((x)==(badval)) ? (badval) : fmod ((x)+331.25,360.0))

/*
 * Generate "aviation useful" units met fields from the Handar anemometers.  
 * The raw file has wind speeds in m/s and wind direction true rather than 
 * magnetic.
 */

static int msg_handler (struct message *msg);
static void newdata(PlatformId pid, int index, ZebTime *zt, int nsample,
		    UpdCode ucode);




main (int argc, char *argv[])
{
  char	ourname[40];

  sprintf (ourname, "HandarDer_%x", getpid());
  
  if (! msg_connect (msg_handler, ourname))
    {
      fprintf (stderr, "%s (%s): unable to connect to message handler\n",
	       ourname, argv[0]);
      exit (1);
    }

  usy_init ();
  ds_Initialize ();
  
  ds_RequestNotify (ds_LookupPlatform ("baf1"), 0, newdata);
  ds_RequestNotify (ds_LookupPlatform ("fedb"), 0, newdata);

  msg_await ();
}



static int
msg_handler (struct message *msg)
{
	struct mh_template *tm = (struct mh_template *) msg->m_data;
/*
 * Just branch out on the message type.
 */
	switch (msg->m_proto)
	{
	/*
	 * Message handler stuff.  The only thing we know how to deal
	 * with now is SHUTDOWN.
	 */
	   case MT_MESSAGE:
	   	if (tm->mh_type == MH_SHUTDOWN)
			exit (0);
		msg_ELog (EF_PROBLEM, "Unknown MESSAGE proto type: %d",
			tm->mh_type);
		break;
	 /*
	  * Data store.
	  */
	    case MT_DATASTORE:
	    	ds_DSMessage (msg);
		break;
	}
	return (0);
}



static void
newdata (PlatformId pid, int index, ZebTime *zt, int nsample, UpdCode ucode)
/*
 * A data available notification has arrived.
 */
{
  static int	ws_k, ws10_k, ws60_k, ws, ws10, ws60;
  static int	wd, wd_mag, tdry;
  static FieldId	flds[32];
  static int		nflds = 0;
  DataChunk	*dc;
  float		val, badval;
  /*
   * Initialize our field list if necessary
   */
  if (! nflds)
    {
      /*
       * Wind speed fields in knots
       */
      ws_k = flds[nflds++] = 
	F_DeclareField ("wspd_k", "2 minute avg wind speed", "knots");
     
      ws10_k = flds[nflds++] = 
	F_DeclareField ("wspd_10_k", "10 minute avg wind speed", "knots");
      
      ws60_k = flds[nflds++] = 
	F_DeclareField ("wspd_60_k", "60 minute avg wind speed", "knots");
      
      /*
       * Wind speed fields in m/s
       */
      ws = flds[nflds++] = 
	F_DeclareField ("wspd_02", "2 minute avg wind speed", "m/s");
     
      ws10 = flds[nflds++] = 
	F_DeclareField ("wspd_10", "10 minute avg wind speed", "m/s");
      
      ws60 = flds[nflds++] = 
	F_DeclareField ("wspd_60", "60 minute avg wind speed", "m/s");
      
      /*
       * Wind direction, magnetic and true
       */
      wd_mag = flds[nflds++] = 
	F_DeclareField ("wdir_mag", "wind direction (magnetic)", "degrees");
      
      wd = flds[nflds++] = 
	F_DeclareField ("wdir", "wind direction", "degrees");

      tdry = flds[nflds++] = 
	F_DeclareField ("tdry", "temperature", "C");
    }
  /*
   * Get the new data
   */
  if (! (dc = ds_Fetch (pid, DCC_Scalar, zt, zt, flds, nflds, 0, 0)))
    {
      msg_ELog (EF_PROBLEM, "Failed to get promised data!");
      return;
    }

  badval = dc_GetBadval (dc);
  /*
   * Derive the fields we need to and stuff the real values into the chunk
   */
  val = MS_TO_KNOTS (dc_GetScalar (dc, 0, ws), badval);
  dc_AddScalar (dc, zt, 0, ws_k, &val);

  val = MS_TO_KNOTS (dc_GetScalar (dc, 0, ws10), badval);
  dc_AddScalar (dc, zt, 0, ws10_k, &val);

  val = MS_TO_KNOTS (dc_GetScalar (dc, 0, ws60), badval);
  dc_AddScalar (dc, zt, 0, ws60_k, &val);

  val = TRUE_TO_MAG (dc_GetScalar (dc, 0, wd), badval);
  dc_AddScalar (dc, zt, 0, wd_mag, &val);
  /*
   * Ship the chunk out to the added value platform
   */
  dc->dc_Platform = (pid == ds_LookupPlatform ("fedb")) ? 
    ds_LookupPlatform ("fedb_x") : ds_LookupPlatform ("baf1_x");
  ds_Store (dc, 0, 0, 0);

  msg_ELog (EF_DEBUG, "Wrote to platform %s", 
	    ds_PlatformName (dc->dc_Platform));

  dc_DestroyDC (dc);
}

