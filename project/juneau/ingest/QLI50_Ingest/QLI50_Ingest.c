/*
 * Ingest data from a Vaisala QLI50 box
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
# define MAG_TO_TRUE(x,badval)	\
(((x)==(badval)) ? (badval) : fmod ((x) + 28.75, 360.0))

# define KNOTS_TO_MS(x,badval)	(((x)==(badval)) ? (badval) : ((x) * 0.51479))
# define MS_TO_KNOTS(x,badval)	(((x)==(badval)) ? (badval) : ((x) * 1.9425))

/*
 * The bad value flag in the Vaisala message
 */
# define QLI50_BADVAL	999.	/* real value unknown at this time... */

/*
 * The anemometers
 */
struct anem
{
    char	id;
    char	*raw_platname;
    PlatformId	raw_pid;
    char	*platname;
    PlatformId	pid;
    Location	loc;
} Anem[] = 
  {
      { 'C', "raw_mt_roberts", 0, "mt_roberts", 0, 
	{ 58.3000, -134.38333, 0.0 } },
      { 'X', "raw_two", 0, "two", 0, { 58.35416667, -134.56775, 0.0 } },
  };


# define N_ANEM	1

/*
 * Variables for our statistics.  Speed sums are kept in m/s, since that's
 * what we get from the A/D.
 */
int	NSpds[N_ANEM], NDirs[N_ANEM];
int	NTemps[N_ANEM], NRHs[N_ANEM], NPressures[N_ANEM];
double	SpdSum[N_ANEM], USum[N_ANEM], VSum[N_ANEM];
double	TempSum[N_ANEM], RHSum[N_ANEM], PresSum[N_ANEM];
double	SpdMin[N_ANEM], SpdMax[N_ANEM];
ZebTime	NextWrite[N_ANEM];

/*
 * Our input serial lines
 */
FILE	*LineIn[N_ANEM];

/*
 * Field IDs
 */
FieldId	WS, WSmax, WSmin, WS_k, WSmax_k, WSmin_k, WD, WD_mag, T, RH, P;

/*
 * Prototypes
 */
static int	msg_handler (struct message *msg);
static int	newdata (int fd);
static void	DeclareFields (void);
static void	DoStats (int which, ZebTime *dtime, double spd, double dir_mag,
			 double temp, double rh, double pres);
static void	WriteStats (int which, ZebTime *dtime, double spd, 
			    double minspd, double maxspd, double dir_mag,
			    double temp, double rh, double pres);
static void	WriteRaw (int which, ZebTime *dtime, double spd, 
			  double dir_mag, double temp, double rh, double pres);





main (int argc, char *argv[])
{
    char	ourname[40];
    int	i;
    
    struct timeval	nulltime = {0,0};
    
    if (argc != (1 + N_ANEM))
    {
	fprintf (stderr, "Usage: %s <tty0> <tty1> <tty2>\n", argv[0]);
	exit (1);
    }
    
    sprintf (ourname, "QLI50_%x", getpid());
    
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
	NSpds[i] = NDirs[i] = NTemps[i] = NRHs[i] = NPressures[i] = 0;
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

	if ((Anem[i].raw_pid = ds_LookupPlatform (Anem[i].raw_platname)) == 
	    BadPlatform)
	{
	    msg_ELog (EF_EMERGENCY, "Bad platform name '%s'", 
		      Anem[i].raw_platname);
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
    static char	str[128] = "";
    char	which[8];
    int		i, ndx;
    double	spd = QLI50_BADVAL, dir_mag = QLI50_BADVAL;
    double	temp = QLI50_BADVAL, rh = QLI50_BADVAL, pres = QLI50_BADVAL;
    double	junk;
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
 * ID, spd, direction, temperature, rh, and pressure.  There's a trick
 * here, since the first character of the line is an ASCII SOH (\001), so
 * we start the sscanf one character into the line.
 */
    fgets (str + strlen (str), sizeof (str), LineIn[i]);
    if (str[strlen(str) - 1] != '\n')
	return (0);

    if (! strcmp (str, "\n"))
    {
	str[0] = '\0';
        return (0);
    }
    

    if (sscanf (str + 1, "%s %lf %lf %lf %lf %lf %lf %lf", which, &temp, 
		&rh, &pres, &junk, &junk, &dir_mag, &spd) < 8)
    {
	msg_ELog (EF_INFO, "Bad QLI50 data '%s' ignored on fd %d", str,
		  fd);

	str[0] = '\0';
	return (0);
    }

    for (ndx = 0; ndx < N_ANEM; ndx++)
	if (which[0] == Anem[ndx].id)
	    break;
    
    if (ndx == N_ANEM)
    {
	msg_ELog (EF_PROBLEM, "Data dropped for unknown QLI50 ID '%c'", 
		  which[0]);
	str[0] = '\0';
	return (0);
    }

    WriteRaw (ndx, &dtime, spd, dir_mag, temp, rh, pres);
    DoStats (ndx, &dtime, spd, dir_mag, temp, rh, pres);

    str[0] = '\0';
    return (0);
}



static void
DoStats (int which, ZebTime *dtime, double spd, double dir_mag, 
	 double temp, double rh, double pres)
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
	double	mean_spd, mean_dir_mag, mean_temp, mean_rh, mean_pres;

	if (NSpds[which])
	    mean_spd = SpdSum[which] / NSpds[which];
	else
	    mean_spd = SpdMin[which] = SpdMax[which] = QLI50_BADVAL;

	if (NDirs[which])
	{
	    mean_dir_mag = 90.0 - 
		RAD_TO_DEG (atan2 (VSum[which] / NDirs[which], 
				   USum[which] / NDirs[which]));
	    if (mean_dir_mag < 0.0 && mean_dir_mag != QLI50_BADVAL)
		mean_dir_mag += 360.0;
	}
	else
	    mean_dir_mag = QLI50_BADVAL;

	if (NTemps[which])
	    mean_temp = TempSum[which] / NTemps[which];
	else
	    mean_temp = QLI50_BADVAL;

	if (NRHs[which])
	    mean_rh = RHSum[which] / NRHs[which];
	else
	    mean_rh = QLI50_BADVAL;

	if (NPressures[which])
	    mean_pres = PresSum[which] / NPressures[which];
	else
	    mean_pres = QLI50_BADVAL;

	WriteStats (which, &(NextWrite[which]), mean_spd, SpdMin[which], 
		    SpdMax[which], mean_dir_mag, mean_temp, mean_rh, 
		    mean_pres);

	NSpds[which] = NDirs[which] = NTemps[which] = NRHs[which] = 0;
	NPressures[which] = 0;

    /*
     * Trigger the next write at the beginning of the next minute
     */
	NextWrite[which].zt_Sec = 60 * ((dtime->zt_Sec / 60) + 1);
    }

/*
 * Update wind speed sum, min, and max.
 */
    if (spd != QLI50_BADVAL)
    {
	if (! NSpds[which])
	{
	    SpdSum[which] = 0.0;
	    SpdMin[which] = SpdMax[which] = spd;
	}

	SpdSum[which] += spd;

	if (spd < SpdMin[which])
	    SpdMin[which] = spd;

	if (spd > SpdMax[which])
	    SpdMax[which] = spd;

	NSpds[which]++;
    }

/*
 * Update the direction sums (we break it into u and v)
 */
    if (dir_mag != QLI50_BADVAL)
    {
	if (! NDirs[which])
	    USum[which] = VSum[which] = 0.0;

	USum[which] += cos (DEG_TO_RAD (90.0 - dir_mag));
	VSum[which] += sin (DEG_TO_RAD (90.0 - dir_mag));

	NDirs[which]++;
    }
/*
 * Update temperature, rh, and pressure sums
 */
    if (temp != QLI50_BADVAL)
    {
	if (! NTemps[which])
	    TempSum[which] = 0.0;

	TempSum[which] += temp;
	NTemps[which]++;
    }

    if (rh != QLI50_BADVAL)
    {
	if (! NRHs[which])
	    RHSum[which] = 0.0;

	RHSum[which] += rh;
	NRHs[which]++;
    }

    if (pres != QLI50_BADVAL)
    {
	if (! NPressures[which])
	    PresSum[which] = 0.0;

	PresSum[which] += pres;
	NPressures[which]++;
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
 * Wind direction magnetic and true
 */
    WD_mag = F_DeclareField ("wdir_mag", "wind direction (magnetic)", 
			     "degrees");
    WD = F_DeclareField ("wdir", "wind direction", "degrees");
/*
 * Temperature, rh, and pressure
 */
    T = F_DeclareField ("temp", "temperature", "degC");
    RH = F_DeclareField ("rh", "relative humidity", "%");
    P = F_DeclareField ("pres", "pressure", "mb");
}


	
static void
WriteStats (int which, ZebTime *dtime, double spd, double minspd,
	    double maxspd, double dir_mag, double temp, double rh, double pres)
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
	FieldId	flds[11];

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
	flds[8] = T;
	flds[9] = RH;
	flds[10] = P;
	dc_SetupFields (dc, 11, flds);
    }

/*
 * Set the platform
 */
    dc->dc_Platform = Anem[which].pid;
    dc_SetStaticLoc (dc, &(Anem[which].loc));

/*
 * Switch over to the data chunk's bad value
 */
    if (spd == QLI50_BADVAL)
	spd = badval;

    if (minspd == QLI50_BADVAL)
	minspd = badval;

    if (maxspd == QLI50_BADVAL)
	maxspd = badval;

    if (dir_mag == QLI50_BADVAL)
	dir_mag = badval;

    if (temp == QLI50_BADVAL)
	temp = badval;

    if (rh == QLI50_BADVAL)
	rh = badval;

    if (pres == QLI50_BADVAL)
	pres = badval;
/*
 * Derive the fields we need to and stuff the real values into the chunk
 */
    val = (float) spd;
    dc_AddScalar (dc, dtime, 0, WS, &val);

    val = (float) maxspd;
    dc_AddScalar (dc, dtime, 0, WSmax, &val);

    val = (float) minspd;
    dc_AddScalar (dc, dtime, 0, WSmin, &val);
    
    val = MS_TO_KNOTS (spd, badval);
    dc_AddScalar (dc, dtime, 0, WS_k, &val);
    
    val = MS_TO_KNOTS (maxspd, badval);
    dc_AddScalar (dc, dtime, 0, WSmax_k, &val);
    
    val = MS_TO_KNOTS (minspd, badval);
    dc_AddScalar (dc, dtime, 0, WSmin_k, &val);

    val = (float) dir_mag;
    dc_AddScalar (dc, dtime, 0, WD_mag, &val);
    
    val = MAG_TO_TRUE (dir_mag, badval);
    dc_AddScalar (dc, dtime, 0, WD, &val);

    val = (float) temp;
    dc_AddScalar (dc, dtime, 0, T, &val);

    val = (float) rh;
    dc_AddScalar (dc, dtime, 0, RH, &val);

    val = (float) pres;
    dc_AddScalar (dc, dtime, 0, P, &val);

/*
 * Write it
 */
    ds_Store (dc, 0, 0, 0);
    
    msg_ELog (EF_DEBUG, "Wrote to platform %s", Anem[which].platname);
}




static void
WriteRaw (int which, ZebTime *dtime, double spd, double dir_mag,
	  double temp, double rh, double pres)
{
    static DataChunk	*dc = 0;
    static float	badval;
    float	val;

/*
 * Create our data chunk if necessary
 */
    if (! dc)
    {
	FieldId	flds[5];

	dc = dc_CreateDC (DCC_Scalar);
	badval = dc_GetBadval (dc);

	flds[0] = WS;
	flds[1] = WD_mag;
	flds[2] = T;
	flds[3] = RH;
	flds[4] = P;
	dc_SetupFields (dc, 5, flds);
    }

/*
 * Set the platform
 */
    dc->dc_Platform = Anem[which].raw_pid;
    dc_SetStaticLoc (dc, &(Anem[which].loc));
    
/*
 * Switch over to the data chunk's bad value
 */
    if (spd == QLI50_BADVAL)
	spd = badval;

    if (dir_mag == QLI50_BADVAL)
	dir_mag = badval;
    
    if (temp == QLI50_BADVAL)
	temp = badval;
    
    if (rh == QLI50_BADVAL)
	rh = badval;
    
    if (pres == QLI50_BADVAL)
	pres = badval;
    
/*
 * Stuff the values into the chunk
 */
    val = (float) spd;
    dc_AddScalar (dc, dtime, 0, WS, &val);
    
    val = (float) dir_mag;
    dc_AddScalar (dc, dtime, 0, WD_mag, &val);
    
    val = (float) temp;
    dc_AddScalar (dc, dtime, 0, T, &val);
    
    val = (float) rh;
    dc_AddScalar (dc, dtime, 0, RH, &val);
    
    val = (float) pres;
    dc_AddScalar (dc, dtime, 0, P, &val);
    
/*
 * Write it
 */
    ds_Store (dc, 0, 0, 0);
    msg_ELog (EF_DEBUG, "Wrote to platform %s", Anem[which].raw_platname);
}

