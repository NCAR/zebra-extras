# include <math.h>
# include <message.h>
# include <DataStore.h>

# define KNOTS_TO_MS(x,badval)	(((x)==(badval)) ? (badval) : ((x) * 0.51479))
/* 28.75 degree mean magnetic deviation near Juneau (1985) */
# define MAG_TO_TRUE(x,badval)	(((x)==(badval)) ? (badval) : fmod ((x) + 28.75, 360.0))

/*
 * Generate "normal" units met fields from the Eagle Crest and Sheep Mtn 
 * anemometers.  They give us wind speed in knots and magnetic wind 
 * direction rather than true.
 */

static int msg_handler (struct message *msg);
static void newdata(PlatformId pid, int index, ZebTime *zt, int nsample,
		    UpdCode ucode);




main (int argc, char *argv[])
{
  char	ourname[40];

  sprintf (ourname, "VaisalaDer_%x", getpid());
  
  if (! msg_connect (msg_handler, ourname))
    {
      fprintf (stderr, "%s (%s): unable to connect to message handler\n",
	       ourname, argv[0]);
      exit (1);
    }

  usy_init ();
  ds_Initialize ();
  
  ds_RequestNotify (ds_LookupPlatform ("eagle_crest"), 0, newdata);
  ds_RequestNotify (ds_LookupPlatform ("sheep_mtn"), 0, newdata);

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
  static int	wsmax_k, wsmin_k, ws_k, wssd_k;
  static int	wsmax, wsmin, ws, wssd;
  static int	wd, wdsd, wd_mag;
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
      wsmax_k = flds[nflds++] = 
	F_DeclareField ("wspd_max_k", "1 minute max wind speed", "knots");
      
      wsmin_k = flds[nflds++] = 
	F_DeclareField ("wspd_min_k", "1 minute min wind speed", "knots");
      
      ws_k = flds[nflds++] = 
	F_DeclareField ("wspd_k", "1 minute avg wind speed", "knots");
      
      wssd_k = flds[nflds++] = 
	F_DeclareField ("wspd_sdev_k", "1 minute wind speed std dev", "knots");
      
      /*
       * Wind speed fields in m/s
       */
      wsmax = flds[nflds++] = 
	F_DeclareField ("wspd_max", "1 minute max wind speed", "m/s");
     
      wsmin = flds[nflds++] = 
	F_DeclareField ("wspd_min", "1 minute min wind speed", "m/s");
      
      ws = flds[nflds++] = 
	F_DeclareField ("wspd", "1 minute avg wind speed", "m/s");
      
      wssd = flds[nflds++] = 
	F_DeclareField ("wspd_sdev", "1 minute wind speed std dev", "m/s");
      /*
       * Wind direction magnetic, true, and std dev
       */
      wd_mag = flds[nflds++] = 
	F_DeclareField ("wdir_mag", "wind direction (magnetic)", "degrees");
      
      wd = flds[nflds++] = 
	F_DeclareField ("wdir", "wind direction", "degrees");

      wdsd = flds[nflds++] = 
	F_DeclareField ("wdir_sdev", "wind direction std dev", "degrees");
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
  val = KNOTS_TO_MS (dc_GetScalar (dc, 0, wsmax_k), badval);
  dc_AddScalar (dc, zt, 0, wsmax, &val);
  
  val = KNOTS_TO_MS (dc_GetScalar (dc, 0, wsmin_k), badval);
  dc_AddScalar (dc, zt, 0, wsmin, &val);
  
  val = KNOTS_TO_MS (dc_GetScalar (dc, 0, ws_k), badval);
  dc_AddScalar (dc, zt, 0, ws, &val);

  val = KNOTS_TO_MS (dc_GetScalar (dc, 0, wssd_k), badval);
  dc_AddScalar (dc, zt, 0, wssd, &val);

  val = MAG_TO_TRUE (dc_GetScalar (dc, 0, wd_mag), badval);
  dc_AddScalar (dc, zt, 0, wd, &val);
  /*
   * Ship the chunk out to the added value platform
   */
  dc->dc_Platform = (pid == ds_LookupPlatform ("eagle_crest")) ? 
    ds_LookupPlatform ("eagle_crest_x") : ds_LookupPlatform ("sheep_mtn_x");
  ds_Store (dc, 0, 0, 0);

  msg_ELog (EF_DEBUG, "Wrote to platform %s", 
	    ds_PlatformName (dc->dc_Platform));

  dc_DestroyDC (dc);
}

