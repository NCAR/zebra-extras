/*
 * Ingest data from a number of Electric Speed Indicator Corp. A/D boxes
 * built for Juneau airport runway anemometers.
 */
# include <stdio.h>
# include <math.h>
# include <sys/time.h>
# include <unistd.h>

# include <message.h>
# include <DataStore.h>

# define DEG_TO_RAD(x)	((x)*0.017453292)
# define RAD_TO_DEG(x)	((x)*57.29577951)

/* 
 * 28.75 degree mean magnetic deviation near Juneau (1985 datum) 
 */
# define TRUE_TO_MAG(x,badval)	\
	(((x)==(badval)) ? (badval) : fmod ((x) + 331.25, 360.0))
# define KNOTS_TO_MS(x,badval)	(((x)==(badval)) ? (badval) : ((x) * 0.51479))

/*
 * The ESI A/D box puts in 990 for bad values
 */
# define ESI_BADVAL	990.

/*
 * The anemometers
 */
struct anem
{
    char	*platname;
    PlatformId	pid;
    Location	loc;
} Anem[] = 
{
    { "runway_west", 0, { 58.35966667, -134.5958611, 0.0 } },
    { "runway_mid", 0, { 58.35416667, -134.56775, 0.0 } },
    { "runway_east", 0, { 58.35377778, -134.5521389, 0.0 } }
};

struct anem RawAnem[] = 
{
    { "raw_runway_west", 0, { 58.35966667, -134.5958611 } },
    { "raw_runway_mid", 0, { 58.35416667, -134.56775 } },
    { "raw_runway_east", 0, { 58.35377778, -134.5521389 } }
};


# define N_ANEM	3

/*
 * Variables for our statistics.  Speed sums are kept in knots, since that's
 * what we get from the A/D.
 */
int	NSpds[N_ANEM], NDirs[N_ANEM];
double	SpdSum[N_ANEM], USum[N_ANEM], VSum[N_ANEM];
double	SpdMin[N_ANEM], SpdMax[N_ANEM];
ZebTime	NextWrite[N_ANEM];

/*
 * Our input serial lines
 */
FILE	*LineIn[N_ANEM];

/*
 * Field IDs
 */
FieldId	WS, WSmax, WSmin, WS_k, WSmax_k, WSmin_k, WD, WD_mag;

/*
 * Prototypes
 */
static int	msg_handler (struct message *msg);
static int	newdata (int fd);
static void	DeclareFields (void);
static void	DoStats (int which, ZebTime *dtime, double spd_kts, 
			 double dir_true);
static void	WriteStats (int which, ZebTime *dtime, double spd_kts, 
			    double min_kts, double max_kts, double dir_true);
static void	WriteRaw (int which, ZebTime *dtime, double spd_kts, 
			  double dir_true);





main (int argc, char *argv[])
{
    char	ourname[40];
    int	i;
    
    struct timeval	nulltime = {0,0};
    
    if (argc != 4)
    {
	fprintf (stderr, "Usage: %s <tty0> <tty1> <tty2>\n", argv[0]);
	exit (1);
    }
    
    sprintf (ourname, "ESI_%x", getpid());
    
    if (! msg_connect (msg_handler, ourname))
    {
	fprintf (stderr, "%s (%s): unable to connect to message handler\n",
		 ourname, argv[0]);
	exit (1);
    }
    
    usy_init ();
    ds_Initialize ();

    /*
     * Do field declarations
     */
    DeclareFields ();

    /*    
     * Initialize some of the per-anemometer stuff
     */
    for (i = 0; i < N_ANEM; i++)
    {
	NSpds[i] = NDirs[i] = 0;
	NextWrite[i].zt_Sec = 0;

	/*
	 * Get the platform id
	 */
	if ((Anem[i].pid = ds_LookupPlatform (Anem[i].platname)) == 
	    BadPlatform)
	{
	    msg_ELog (EF_EMERGENCY, "Bad platform name '%s'", 
		      Anem[i].platname);
	    exit (1);
	}

	if ((RawAnem[i].pid = ds_LookupPlatform (RawAnem[i].platname)) == 
	    BadPlatform)
	{
	    msg_ELog (EF_EMERGENCY, "Bad platform name '%s'", 
		      RawAnem[i].platname);
	    exit (1);
	}

	/*
	 * Open the associated serial line and add the file descriptor to
	 * those watched for action
	 */
	if (! (LineIn[i] = fopen (argv[i+1], "r")))
	{
	    msg_ELog (EF_EMERGENCY, "Unable to open serial line '%s'", 
		      argv[i+1]);
	    exit (1);
	}

	msg_add_fd (fileno (LineIn[i]), newdata);
    }
    
    /*	
     * Event loop
     */
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
      case MT_MESSAGE:
	/*
	 * Message handler stuff.  The only thing we know how to deal
	 * with now is SHUTDOWN.
	 */
	if (tm->mh_type == MH_SHUTDOWN)
	    exit (0);
	msg_ELog (EF_PROBLEM, "Unknown MESSAGE proto type: %d",
		  tm->mh_type);
	break;
      case MT_DATASTORE:
	/*
	 * Data store.
	 */
	ds_DSMessage (msg);
	break;
    }
    return (0);
}




static int
newdata (int fd)
/*
 * New stuff is waiting on one of our serial lines
 */
{
    char	str[64];
    int		i, which;
    double	spd_kts = ESI_BADVAL, dir_true = ESI_BADVAL;
    struct timezone	tz_gmt = { 0, DST_NONE };
    struct timeval	current;
    ZebTime	dtime;

    /*
     * Figure out which line has data waiting for us
     */
    for (i = 0; i < N_ANEM; i++)
	if (fd == fileno (LineIn[i]))
	    break;

    if (i == N_ANEM)
    {
	msg_ELog (EF_PROBLEM, "Unexpected fd %d in newdata", fd);
	return (0);
    }

    /*
     * Use the current time for this datum
     */
    gettimeofday (&current, &tz_gmt);
    dtime.zt_Sec = current.tv_sec;
    dtime.zt_MicroSec = current.tv_usec;

    /*	
     * Read the line.  Ignore it if it's blank, otherwise break it into 
     * anemometer ID, spd, and direction
     */
    fgets (str, sizeof (str), LineIn[i]);

    if (! strcmp (str, "\n"))
        return (0);

    if (sscanf (str, "%d ,%lf ,%lf", &which, &spd_kts, &dir_true) < 3)
    {
	msg_ELog (EF_INFO, "Bad anemometer data '%s' ignored on fd %d", str,
		  fd);
	return (0);
    }

    which--;	/* They start the IDs at one; we start them at zero */

    WriteRaw (which, &dtime, spd_kts, dir_true);
    DoStats (which, &dtime, spd_kts, dir_true);
    return (0);
}



static void
DoStats (int which, ZebTime *dtime, double spd_kts, double dir_true)
{
    /*
     * Initialize the next write time if necessary
     */
    if (! NextWrite[which].zt_Sec)
	NextWrite[which].zt_Sec = 60 * ((dtime->zt_Sec / 60) + 1);

    /*
     * Write out our current statistical values if we've passed 
     * the next write time
     */
    if (! TC_LessEq (*dtime, NextWrite[which]))
    {
	double	spd_kts;
	double	dir_true;

	if (NSpds[which])
	    spd_kts = SpdSum[which] / NSpds[which];
	else
	    spd_kts = SpdMin[which] = SpdMax[which] = ESI_BADVAL;

	if (NDirs[which])
	{
	    dir_true = 90.0 - RAD_TO_DEG (atan2 (VSum[which] / NDirs[which], 
						 USum[which] / NDirs[which]));
	    if (dir_true < 0.0 && dir_true != ESI_BADVAL)
		dir_true += 360.0;
	}
	else
	    dir_true = ESI_BADVAL;

	WriteStats (which, &(NextWrite[which]), spd_kts, SpdMin[which], 
		    SpdMax[which], dir_true);

	NSpds[which] = NDirs[which] = 0;

	/*
	 * Trigger the next write at the beginning of the next minute
	 */
	NextWrite[which].zt_Sec = 60 * ((dtime->zt_Sec / 60) + 1);
    }

    /*
     * Update wind speed sum, min, and max.
     */
    if (spd_kts != ESI_BADVAL)
    {
	if (! NSpds[which])
	{
	    SpdSum[which] = 0.0;
	    SpdMin[which] = SpdMax[which] = spd_kts;
	}

	SpdSum[which] += spd_kts;

	if (spd_kts < SpdMin[which])
	    SpdMin[which] = spd_kts;

	if (spd_kts > SpdMax[which])
	    SpdMax[which] = spd_kts;

	NSpds[which]++;
    }

    /*
     * Update the direction sums (we break it into u and v)
     */
    if (dir_true != ESI_BADVAL)
    {
	if (! NDirs[which])
	    USum[which] = VSum[which] = 0.0;

	USum[which] += cos (DEG_TO_RAD (90.0 - dir_true));
	VSum[which] += sin (DEG_TO_RAD (90.0 - dir_true));

	NDirs[which]++;
    }
}

	
    
static void
DeclareFields (void)
{
    /*
     * Wind speed fields in knots
     */
    WSmax_k = F_DeclareField ("wspd_max_k", "1 minute max wind speed", 
			      "knots");
    WSmin_k = F_DeclareField ("wspd_min_k", "1 minute min wind speed", 
			      "knots");
    WS_k = F_DeclareField ("wspd_k", "1 minute avg wind speed", "knots");
    
    /*
     * Wind speed fields in m/s
     */
    WSmax = F_DeclareField ("wspd_max", "1 minute max wind speed", "m/s");
    WSmin = F_DeclareField ("wspd_min", "1 minute min wind speed", "m/s");
    WS = F_DeclareField ("wspd", "1 minute avg wind speed", "m/s");
    
    /*
     * Wind direction magnetic, true, and std dev
     */
    WD_mag = F_DeclareField ("wdir_mag", "wind direction (magnetic)", 
			     "degrees");
    WD = F_DeclareField ("wdir", "wind direction", "degrees");
}


	
static void
WriteStats (int which, ZebTime *dtime, double spd_kts, double min_kts,
	    double max_kts, double dir_true)
{
    static DataChunk	*dc = 0;
    static float	badval;
    Location	loc;
    float	val;

    /*
     * Create our data chunk if necessary
     */
    if (! dc)
    {
	FieldId	flds[8];

	dc = dc_CreateDC (DCC_Scalar);
	badval = dc_GetBadval (dc);

	flds[0] = WS;
	flds[1] = WSmax;
	flds[2] = WSmin;
	flds[3] = WS_k;
	flds[4] = WSmax_k;
	flds[5] = WSmin_k;
	flds[6] = WD;
	flds[7] = WD_mag;
	dc_SetupFields (dc, 8, flds);
    }

    /*
     * Set the platform
     */
    dc->dc_Platform = Anem[which].pid;
    dc_SetStaticLoc (dc, &(Anem[which].loc));

    /*
     * Switch over to the data chunk's bad value
     */
    if (spd_kts == ESI_BADVAL)
      spd_kts = badval;

    if (min_kts == ESI_BADVAL)
      min_kts = badval;

    if (max_kts == ESI_BADVAL)
      max_kts = badval;

    if (dir_true == ESI_BADVAL)
      dir_true = badval;
    
    /*
     * Derive the fields we need to and stuff the real values into the chunk
     */
    val = (float) spd_kts;
    dc_AddScalar (dc, dtime, 0, WS_k, &val);

    val = (float) max_kts;
    dc_AddScalar (dc, dtime, 0, WSmax_k, &val);

    val = (float) min_kts;
    dc_AddScalar (dc, dtime, 0, WSmin_k, &val);
    
    val = KNOTS_TO_MS (spd_kts, badval);
    dc_AddScalar (dc, dtime, 0, WS, &val);
    
    val = KNOTS_TO_MS (max_kts, badval);
    dc_AddScalar (dc, dtime, 0, WSmax, &val);
    
    val = KNOTS_TO_MS (min_kts, badval);
    dc_AddScalar (dc, dtime, 0, WSmin, &val);

    val = (float) dir_true;
    dc_AddScalar (dc, dtime, 0, WD, &val);
    
    val = TRUE_TO_MAG (dir_true, badval);
    dc_AddScalar (dc, dtime, 0, WD_mag, &val);

    /*
     * Write it
     */
    ds_Store (dc, 0, 0, 0);
    
    msg_ELog (EF_DEBUG, "Wrote to platform %s", Anem[which].platname);
}




static void
WriteRaw (int which, ZebTime *dtime, double spd_kts, double dir_true)
{
    static DataChunk	*dc = 0;
    static float	badval;
    float	val;

    /*
     * Create our data chunk if necessary
     */
    if (! dc)
    {
	FieldId	flds[3];

	dc = dc_CreateDC (DCC_Scalar);
	badval = dc_GetBadval (dc);

	flds[0] = WS_k;
	flds[1] = WD;
	flds[2] = WD_mag;
	dc_SetupFields (dc, 3, flds);
    }

    /*
     * Set the platform
     */
    dc->dc_Platform = RawAnem[which].pid;
    dc_SetStaticLoc (dc, &(RawAnem[which].loc));
    
    /*
     * Switch over to the data chunk's bad value
     */
    if (spd_kts == ESI_BADVAL)
      spd_kts = badval;

    if (dir_true == ESI_BADVAL)
      dir_true = badval;
    
    /*
     * Stuff the values into the chunk
     */
    val = (float) spd_kts;
    dc_AddScalar (dc, dtime, 0, WS_k, &val);
    
    val = (float) dir_true;
    dc_AddScalar (dc, dtime, 0, WD, &val);
    
    val = TRUE_TO_MAG (dir_true, badval);
    dc_AddScalar (dc, dtime, 0, WD_mag, &val);

    /*
     * Write it
     */
    ds_Store (dc, 0, 0, 0);
    msg_ELog (EF_DEBUG, "Wrote to platform %s", RawAnem[which].platname);
}

