/*
 * Quick and dirty (if not filthy) routine to pull in ACE-1 ASCII
 * soundings.
 */


# include <stdio.h>
# include <string.h>
# include <math.h>
# include <defs.h>
# include <message.h>
# include <DataStore.h>

# define BADFLAG -99999.9
# define LINELEN 80		/* more than enough */
# define D2R(deg) ((deg)*M_PI/180.0)
# define R2D(rad) ((rad)*180.0/M_PI)

/*
 * The file formats we understand.
 */
typedef enum { FF_ADELAIDE, FF_HOBART } FileFormat;

/*
 * The arrays in which we store the data.  It is kept this way so that we
 * can easily reset things if we find that we are still plowing through
 * the junk lines that appear at the beginning of the sounding files.
 */
# define MAXPOINT 2048 /* Sample data maxes out at 350 */
float DeltaTime[MAXPOINT];	/* Time since beginning 	*/
float Range[MAXPOINT];		/* Range from station		*/
float Azimuth[MAXPOINT];	/* Azimuth to balloon		*/
float Elevation[MAXPOINT];	/* Elevation to baloon		*/
float WSpd[MAXPOINT];		/* Speed			*/
float WDir[MAXPOINT];		/* Direction			*/
float Height[MAXPOINT];		/* Balloon height		*/
float Pres[MAXPOINT];		/* Pressure			*/
ZebTime Time[MAXPOINT];
int NPoints = 0;

/*
 * Mappings of weird location codes into platform names and formats.
 */
struct _CodeMap
{
	int cm_code;		/* The code in the file name	*/
	char *cm_pname;		/* The platform name		*/
	FileFormat cm_format;	/* The format of the files	*/
} CodeMap [] =
{
  	{ 1100,	"sounding/adelaide",	FF_ADELAIDE },
	{ 1101,	"sounding/laverton",	FF_ADELAIDE },
	{ 1102,	"sounding/hobart",	FF_HOBART }
};
# define NMAP (sizeof CodeMap/sizeof (struct _CodeMap))

/*
 * Field ID's.
 */
# define NFIELD 7
FieldId Fids[NFIELD];

/*
 * Forwards.
 */
int ExtractInfo FP ((char *, PlatformId *, ZebTime *, FileFormat *));
int DecodeLine FP ((char *, FileFormat));
int DecodeAdelaide FP ((char *));
int DecodeHobart FP ((char *));
void TimeCheck FP ((void));
void BreakCheck FP ((void));
void CalculateWinds FP ((void));
void MakeTimes FP ((ZebTime *));
void MakePressures FP ((void));
DataChunk *MakeDC FP ((void));


/*
 * Your standard, boring message handler.
 */
MsgHandler ()
{ }



/*
 * ace-sounding files...
 */
main (argc, argv)
int argc;
char **argv;
{
	FILE *fp;
	DataChunk *dc;
	PlatformId pid;
	ZebTime begintime;
	char line[LINELEN];
	FileFormat format;
/*
 * Get ourselves set up.
 */
	usy_init ();
	msg_connect (MsgHandler, "ace-sounding");
	ds_Initialize ();
/*
 * This is ugly, but what the hell.  Get all of the field ID's.
 */
	Fids[0] = F_DeclareField ("range", "Balloon range", "km");
	Fids[1] = F_DeclareField ("azimuth", "Balloon azimuth", "deg");
	Fids[2] = F_DeclareField ("elevation", "Balloon elevation", "deg");
	Fids[3] = F_DeclareField ("height", "Balloon height", "km");
	Fids[4] = F_DeclareField ("wspd", "Wind speed", "m/s");
	Fids[5] = F_DeclareField ("wdir", "Wind direction", "deg");
	Fids[6] = F_DeclareField ("pres", "Pressure", "mb");
/*
 * Plow through the files.
 */
	for (argc--, argv++; argc > 0; argc--, argv++)
	{
		NPoints = 0;
	/*
	 * Open the file, and extract the platform and time info from the
	 * file name.
	 */
		if ((fp = fopen (*argv, "r")) == NULL)
		{
			perror (*argv);
			continue;
		}
		if (! ExtractInfo (*argv, &pid, &begintime, &format))
			continue;
	/*
	 * Plow through it.
	 */
		while (fgets (line, LINELEN, fp) != NULL)
		{
		/*
		 * Attempt to extract the information from the line.  If
		 * we do not succeed, assume we are still plowing through
		 * the junk lines at the beginning, and start over.
		 */
			if (! DecodeLine (line, format))
			{
				NPoints = 0;
				continue;
			}
		/*
		 * The other thing that can happen is that the time goes
		 * backward.  That, too, indicates that we have junk lines,
		 * but in this case we want to keep the new stuff.
		 */
			TimeCheck ();
		}
	/*
	 * If this is a hobart sounding, calculate the winds.
	 */
		if (format == FF_HOBART)
			CalculateWinds ();
	/*
	 * Get rid of the file, and see if we need to truncate out
	 * data after the balloon breaks.  Also create real time values.
	 */
		fclose (fp);
		BreakCheck ();
		MakeTimes (&begintime);
		MakePressures ();
	/*
	 * Make our DataChunk and store it.
	 */
		dc = MakeDC ();
		dc->dc_Platform = pid;
		ds_Store (dc, TRUE, 0, 0);
		dc_DestroyDC (dc);
	}
}





int
ExtractInfo (fname, pid, begintime, format)
char *fname;
PlatformId *pid;
ZebTime *begintime;
FileFormat *format;
/*
 * Figure out just what we're dealing with here.
 */
{
	int i, code;
	char *fnbegin;
	struct date_st fdate;
/*
 * Find the beginning of the file name, and see if we can't decode out the
 * parts.
 */
	if ((fnbegin = strrchr (fname, '/')) != 0)
		fnbegin++;	/* just past slash */
	else
		fnbegin = fname;
	if (sscanf (fnbegin, "%d.%d.%d", &code, &fdate.ds_yymmdd,
			&fdate.ds_hhmmss) != 3)
	{
		fprintf (stderr, "Unable to decode fname '%s'\n", fname);
		return (FALSE);
	}
/*
 * Look up the code in the map.
 */
	for (i = 0; i < NMAP; i++)
		if (CodeMap[i].cm_code == code)
			break;
	if (i >= NMAP)
	{
		fprintf (stderr, "Funky code %d in file name %s\n",code,fname);
		return (FALSE);
	}
/*
 * Which platform is this?
 */
	if ((*pid = ds_LookupPlatform (CodeMap[i].cm_pname)) == BadPlatform)
	{
		fprintf (stderr, "Can't lookup platform %s\n",
				CodeMap[i].cm_pname);
		return (FALSE);
	}
/*
 * Return the info.
 */
	TC_UIToZt (&fdate, begintime);
	*format = CodeMap[i].cm_format;
	return (TRUE);
}





int
DecodeLine (line, format)
char *line;
FileFormat format;
/*
 * Extract the numbers out of this line.
 */
{
	switch (format)
	{
	    case FF_ADELAIDE:
		return (DecodeAdelaide (line));

	    case FF_HOBART:
		return (DecodeHobart (line));
	}
}



int
DecodeAdelaide (line)
char *line;
/*
 * Decode an adelaide-format line.
 */
{
	int junk, ndecoded, np = NPoints;
/*
 * Just try to do it as a massive decode.
 */
	ndecoded = sscanf (line, "%d %f %f %f %f %f %f %f", &junk,
			DeltaTime + np, Range + np, Azimuth + np,
			Elevation + np, WDir + np, WSpd + np, Height + np);
/*
 * Now see what came back.  If we got all eight items, it's a good line.
 */
	if (ndecoded == 8)
	{
		Height[np] /= 1000.0;	/* Convert to km	*/
# ifdef notdef
		if ((WDir[np] -= 180.0) < 0)	/* Wind from -> wind to */
			WDir[np] += 360.0;
# endif
		DeltaTime[np] *= 60;	/* to seconds */
		NPoints++;
		return (TRUE);
	}
/*
 * If instead we got five, then this is a funky line without wind info.
 * Chris says we don't want these, so to hell with it.  We return success,
 * but do not increment NPoints to keep this data.
 *
 * Otherwise it's a complete failure.
 */
	return (ndecoded == 5);
}



int
DecodeHobart (line)
char *line;
/*
 * Decode a Hobart-format line.
 */
{
	int np = NPoints, ndecoded, t;
/*
 * Try the decode.  The "+ 2" skips a leading ^M and space.
 */
	ndecoded = sscanf (line + 2, "TM: %f RG: %f EL: %f AZ: %f",
			DeltaTime + np, Range + np, Elevation + np,
			Azimuth + np);
	if (ndecoded != 4)
		return (FALSE);
/*
 * We got the stuff.  Tweak the data into the right units.  The time,
 * as it happens, is in HHMMSS format, so we have to fix that too.
 */
	t = DeltaTime[np];
	DeltaTime[np] = (t/10000)*3600 + ((t/100) % 100)*60 + (t % 100);
	Range[np] /= 1000.0;
	Elevation[np] /= 100.0;
	Azimuth[np] /= 100.0;
	NPoints++;
	return (TRUE);
}





void
TimeCheck ()
/*
 * See if the time is going backwards.
 */
{
	int np = NPoints - 1;
	
	if (NPoints > 1 && DeltaTime[np - 1] > DeltaTime[np])
	{
		printf ("Timecheck: NP %d times %.2f %.2f\n", NPoints,
				DeltaTime[np - 1], DeltaTime[np]);
		DeltaTime[0] = DeltaTime[np];
		Range[0] = Range[np];
		Azimuth[0] = Azimuth[np];
		Elevation[0] = Elevation[np];
		WSpd[0] = WSpd[np];
		WDir[0] = WDir[np];
		Height[0] = Height[np];
		NPoints = 1;
	}
}





void
BreakCheck ()
/*
 * See if we have data after the balloon pops.  Note we need to be sure we
 * have heights by now.
 */
{
	int p;

	for (p = NPoints/2; p < NPoints; p++)
		if (Height[p] < Height[p - 1])
		{
			printf ("POP! At pt %d/%d, Height from %.2f to %.2f\n",
					p, NPoints, Height[p - 1], Height[p]);
			NPoints = p;
			return;
		}
}




void
CalculateWinds ()
/*
 * Figure out what the winds were doing from what the motion of the
 * balloon was.  I am sure there must be a more graceful way to
 * do this, but I'm not having an elegant sort of day.
 */
{
	int p;
	float rp1, rp2;	/* 1 = prev point, 2 = current */
	float dx, dy;
/*
 * The first point is special.
 */
	WSpd[0] = WDir[0] = BADFLAG;
	Height[0] = Range[0]*sin (D2R (Elevation[0]));
/*
 * Now do the rest of them.
 */
	for (p = 1; p < NPoints; p++)
	{
	/*
	 * Height is easy so we'll just do it now.
	 */
		Height[p] = Range[p]*sin (D2R (Elevation[p]));
	/*
	 * Project the range of the points onto the horizontal plane.
	 */
		rp1 = Range[p - 1]*cos (D2R (Elevation[p - 1]));
		rp2 = Range[p]*cos (D2R (Elevation[p]));
	/*
	 * Figure how far the balloon moved in the X and Y directions.  The
	 * assumption here is that azimuth is the usual clockwise-from-north
	 * convention.
	 */
		dx = rp2*sin (D2R (Azimuth[p])) - rp1*sin (D2R (Azimuth[p-1]));
		dy = rp2*cos (D2R (Azimuth[p])) - rp1*cos (D2R (Azimuth[p-1]));
	/*
	 * Now we can get speed and direction.  No need to negate the
	 * direction, since we have a direct motion vector, not u and v
	 * values.
	 */
		WSpd[p] = hypot (dx, dy)*1000.0/(DeltaTime[p]-DeltaTime[p-1]);
		if ((WDir[p] = R2D (atan2 (dx, dy))) < 0)
			WDir[p] += 360.0;
	}
}




DataChunk *
MakeDC ()
/*
 * Turn our arrays into a data chunk.
 */
{
	DataChunk *dc = dc_CreateDC (DCC_Scalar);
/*
 * Set things up, and add the data.
 */
	dc_SetScalarFields (dc, NFIELD, Fids);
	dc_SetBadval (dc, BADFLAG);
	dc_AddMultScalar (dc, Time, 0, NPoints, F_Lookup ("range"), Range);
	dc_AddMultScalar (dc, Time, 0, NPoints, F_Lookup ("azimuth"), Azimuth);
	dc_AddMultScalar (dc, Time, 0, NPoints, F_Lookup ("elevation"),
			Elevation);
	dc_AddMultScalar (dc, Time, 0, NPoints, F_Lookup ("height"), Height);
	dc_AddMultScalar (dc, Time, 0, NPoints, F_Lookup ("wspd"), WSpd);
	dc_AddMultScalar (dc, Time, 0, NPoints, F_Lookup ("wdir"), WDir);
	dc_AddMultScalar (dc, Time, 0, NPoints, F_Lookup ("pres"), Pres);
	return (dc);
}





void
MakeTimes (begin)
ZebTime *begin;
/*
 * Fix up the time array.
 */
{
	int p;

	for (p = 0; p < NPoints; p++)
	{
		Time[p] = *begin;
		Time[p].zt_Sec += (int) DeltaTime[p];
		Time[p].zt_MicroSec = DeltaTime[p] - (int) DeltaTime[p];
	}
}




void
MakePressures ()
/*
 * Calculate pressures based on the ICAO standard atmosphere
 */
{
	int	p;
	float	alt, exponent;
	float	R_d = 287.05;	/* Gas constant for dry air */
	float	G = 9.80665;	/* gravitational acceleration */

	for (p = 0; p < NPoints; p++)
	{
	/*
	 * U.S. Standard Atmosphere (1976) calculations.
	 *
	 * Formulae from "Fundamentals of Atmospheric Dynamics and 
	 * Thermodynamics" by C.A. Riegel, World Scientific, 1992, pp. 334-335
	 *
	 * U.S. Standard Atmosphere (1976) below 20 gpkm (54.747 mb):
	 *
	 *	a) surface temperature = 288.15 K
	 *	b) surface pressure = 1013.25 mb
	 *	c) g = 9.80665 m/s/s (constant)
	 *	d) lapse rate in the troposphere = 6.5 K/gpkm (0.0065 K/m)
	 *	e) tropopause at 11 gpkm  (226.317 mb)
	 *	f) the lower stratosphere is isothermal at a temperature of 
	 *         216.56 K
	 *
	 */
		alt = Height[p];
		
		if (alt <= 11.0)
		{
			exponent = G / (0.0065 * R_d);
			Pres[p] = 1013.25 * 
				pow ((288.15 - 6.5 * alt) / 288.15, exponent);
		}
		else if (alt <= 20.0)
		{
			exponent = -G / (216.65 * R_d) * 1000 * (alt - 11.0);
			Pres[p] = 226.317 * exp (exponent);
		}
		else
			Pres[p] = BADFLAG;
	}
}
