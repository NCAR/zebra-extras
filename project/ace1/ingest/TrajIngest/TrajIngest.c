/*
 * Ingest trajectory model data
 */
# include <stdio.h>
# include <math.h>
# include <errno.h>
# include <DataStore.h>
# include <message.h>

const float BADVAL = -32768.0;
const int MAX_STEPS = 200;

/*
 * Trajectory count, and the altitude and platform name for each.
 */
# define MAXTRAJ	10

int Ntraj;

typedef struct _Trajectory
{
	PlatformId	pid;
	Location	startloc;
} Trajectory;

Trajectory	Trj[MAXTRAJ];
	

const float Alts[MAXTRAJ] = 
{
	0.5, 1.5, 3.0
};

const char *Pnames[MAXTRAJ] =
{
	"0.5km", "1.5km", "3.0km"
};

PlatformId	Plats[MAXTRAJ];

/*
 * Prototypes
 */
static int	msg_handler (void);
static void	LoadPlatInfo (char *model, char *fname);






main (int argc, char *argv[])
{
	FILE	*infile;
	int	i, ix, iy, ipres, traj, what, offset, pt, n, have_plat_info;
	int	year, month, day, hour, sign, backward;
	char	*model, *fname, line[80];
	float	latval, lonval, startlat, startlon;
	int	npts[MAXTRAJ];
	float	foffset[MAXTRAJ][MAX_STEPS];
	float	lat[MAXTRAJ][MAX_STEPS], lon[MAXTRAJ][MAX_STEPS];
	float	pres[MAXTRAJ][MAX_STEPS];
	FieldId	odim, ofld, latfld, lonfld, presfld;
	ZebTime	issue_time, this_time;
	Location	startloc;
	DataChunk	*dc;
/*
 * Backward trajectory flag and arg check
 */
	backward = (argc > 1 && ! strcmp (argv[1], "-b"));
	
	if ((backward && argc != 4) || (! backward && argc != 3))
	{
		fprintf (stderr, "Usage: %s [-b] <model> <data_file>\n", 
			 argv[0]);
		exit (1);
	}
/*
 * Our other args
 */
	if (backward)
	{
		model = argv[2];
		fname = argv[3];
	}
	else
	{
		model = argv[1];
		fname = argv[2];
	}
/*
 * Connect to Zebra, and get our list of platform id's
 */
	sprintf (line, "TrajIngest_%x", getpid ());
	msg_connect (msg_handler, line);
	usy_init ();
	ds_Initialize ();
	
/*
 * We have no issue time to start with
 */
	issue_time.zt_Sec = issue_time.zt_MicroSec = 0;
/*
 * Get platform info
 */
	LoadPlatInfo (model, fname);
/*
 * Initialize npts
 */
	for (traj = 0; traj < Ntraj; traj++)
		npts[traj] = 0;
/*
 * Open the input file
 */
	if ((infile = fopen (fname, "r")) < 0)
	{
		fprintf (stderr, "Error %d opening '%s'!\n", errno, fname);
		exit (1);
	}
/*
 * Loop through all the lines in the file
 */
	while (fgets (line, sizeof (line), infile))
	{
	/*
	 * Make sure it's a data line, i.e., it starts with "95" (kluge)
	 */
		if (strncmp (line, "95", 2))
			continue;
	/*
	 * Break up the line
	 */
		n = sscanf (line, 
			    "%2d%2d%2d%2d%6f%7f%5d%5d%5d%5d%5d%5d%5d",
			    &year, &month, &day, &hour, &latval, &lonval, 
			    &ix, &iy, &ipres, &what, &offset, &what, &traj);

		if (n != 13)
		{
			msg_ELog (EF_INFO, "Bad line '%s'", line);
			continue;
		}
	/*
	 * Our index is based at zero, not one!
	 */
		traj--;
	/*
	 * Sanity check on issue time
	 */
		TC_ZtAssemble (&this_time, year, month, day, hour, 0, 0, 0);

		if (backward) offset *= -1;
		this_time.zt_Sec -= 3600 * offset;

		if (issue_time.zt_Sec == 0)
		{
			issue_time.zt_Sec = this_time.zt_Sec;
			issue_time.zt_MicroSec = this_time.zt_MicroSec;
		}

		if (! TC_Eq (issue_time, this_time))
		{
			msg_ELog (EF_INFO, "Bad issue time. Line skipped.");
			continue;
		}
	/*
	 * OK, we'll use this point
	 */
		pt = npts[traj]++;
		lat[traj][pt] = latval;
		lon[traj][pt] = lonval;
		pres[traj][pt] = (float) ipres;
		foffset[traj][pt] = (float) offset;
	}
/*
 * For each traj, build a data chunk and stuff it out.  Be sure to write 
 * each in its own file, since we're counting on that to deal with overlap 
 * between model runs.
 */
	for (traj = 0; traj < Ntraj; traj++)
	{
		dc = dc_CreateDC (DCC_NSpace);
		dc->dc_Platform = Trj[traj].pid;

		ofld = odim = F_DeclareField ("forecast_offset", 
					      "forecast offset time", "hrs");
		dc_NSDefineDimension (dc, odim, npts[traj]);
		dc_NSDefineVariable (dc, ofld, 1, &odim, FALSE);

		latfld = F_Lookup ("f_lat");
		dc_NSDefineVariable (dc, latfld, 1, &odim, FALSE);

		lonfld = F_Lookup ("f_lon");
		dc_NSDefineVariable (dc, lonfld, 1, &odim, FALSE);

		presfld = F_Lookup ("f_pres");
		dc_NSDefineVariable (dc, presfld, 1, &odim, FALSE);

		dc_NSDefineComplete (dc);

		dc_NSAddSample (dc, &issue_time, 0, latfld, 
				(void *)(&lat[traj][0]));
		dc_NSAddSample (dc, &issue_time, 0, lonfld, 
				(void *)(&lon[traj][0]));
		dc_NSAddSample (dc, &issue_time, 0, presfld, 
				(void *)(&pres[traj][0]));
		dc_NSAddSample (dc, &issue_time, 0, ofld, 
				(void *)(&foffset[traj][0]));
	/*
	 * Set the location.  If we don't have starting location already,
	 * just extrapolate back from the first two points and find the
	 * nearest integral lat & lon. 
	 */
		if (Trj[traj].startloc.l_lat == 0.0 && 
		    Trj[traj].startloc.l_lon == 0.0)
		{
			startlat = 2 * lat[traj][0] - lat[traj][1];
			startlon = 2 * lon[traj][0] - lon[traj][1];

			sign = (startlat < 0) ? -1 : 1;
			startlat *= sign;
			startlat = (int)(startlat + 0.5);
			startlat *= sign;

			sign = (startlon < 0) ? -1 : 1;
			startlon *= sign;
			startlon = (int)(startlon + 0.5);
			startlon *= sign;

			Trj[traj].startloc.l_lat = startlat;
			Trj[traj].startloc.l_lon = startlon;
		}

		dc_SetLoc (dc, 0, &(Trj[traj].startloc));

		ds_Store (dc, TRUE, NULL, 0);
		dc_DestroyDC (dc);
	}
}




static int
msg_handler (void)
{
	msg_ELog (EF_PROBLEM, "Exiting on received message");
	exit (1);
}



static void
LoadPlatInfo (char *model, char *fname)
/*
 * Load platform info, and leave the first data line in "line"
 */
{
	int	i, t, bom, index;
	float	lat, lon, alt, what;
	char	line[80], pname[80], latlon[6];
	FILE	*infile;
/*
 * For BoM files, get the lat/lon from the last five chars of the file name,
 * and the trajectory start info is hardwired.
 */
	bom = ! strcmp (model, "bom");
	if (bom)
	{
		strcpy (latlon, fname + strlen (fname) - 5);

		Ntraj = 3;

		Trj[0].startloc.l_alt = 0.5;
		Trj[0].startloc.l_lat = Trj[0].startloc.l_lon = 0.0;
		
		Trj[1].startloc.l_alt = 1.5;
		Trj[1].startloc.l_lat = Trj[1].startloc.l_lon = 0.0;
		
		Trj[2].startloc.l_alt = 3.0;
		Trj[2].startloc.l_lat = Trj[2].startloc.l_lon = 0.0;
	}
	else
/*
 * For U.H., we read it from the file
 */
	{
	/*
	 * Open the file
	 */
		if ((infile = fopen (fname, "r")) < 0)
		{
			fprintf (stderr, "Error %d opening '%s'!\n", errno, 
				 fname);
			exit (1);
		}
		Ntraj = 0;
	/*
	 * Skip the first three lines
	 */
		for (i = 0; i < 3; i++)
		{
			if (! fgets (line, sizeof (line), infile))
			{
				msg_ELog (EF_PROBLEM, 
					  "Fewer than three lines in file!");
				exit (1);
			}
		}
	/*
	 * Read trajectory initial info lines until we hit a data line,
	 * i.e., one starting with "95" (kluge)
	 */
		while (fgets (line, sizeof (line), infile) &&
		       strncmp (line, "95", 2))
		{
		/*
		 * Pull the trajectory initial info from the line
		 */
			sscanf (line, "%f%f%f%f%f%f%f%f", &what, &what, &what,
				&what, &what, &lat, &lon, &alt);
		
			alt *= -0.001; /* convert to km MSL */

			Trj[Ntraj].startloc.l_alt = alt;
			Trj[Ntraj].startloc.l_lat = lat;
			Trj[Ntraj].startloc.l_lon = lon;

			Ntraj++;
		}
		
		fclose (infile);
	}
/*
 * Get PlatformIds
 */
	for (t = 0; t < Ntraj; t++)
	{
	/*
	 * Our "index" or subplatform number is based on the starting
	 * lat/lon of the trajectory.  For bom, we get it from the file
	 * name.  For U.H., it's based on the order of trajectories in the
	 * file
	 */
		if (bom)
		{
			if (! strcmp (latlon, "45135"))
				index = 1;
			else if (! strcmp (latlon, "50130"))
				index = 2;
			else if (! strcmp (latlon, "50135"))
				index = 3;
			else if (! strcmp (latlon, "50140"))
				index = 4;
			else if (! strcmp (latlon, "55135"))
				index = 5;
			else
			{
				msg_ELog (EF_PROBLEM, "Bad BoM latlon: %s",
					  latlon);
				exit (1);
			}
		}
		else
			index = t + 1;
	/*
	 * Build the plat name and look it up
	 */
		sprintf (pname, "%s%0.1fkm/%d", model, Trj[t].startloc.l_alt, 
			 index);
		if ((Trj[t].pid = ds_LookupPlatform (pname)) == BadPlatform)
		{
			fprintf (stderr, "Bad platform '%s'\n", pname);
			exit (1);
		}
	}
}
