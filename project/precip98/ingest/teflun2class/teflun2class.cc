//
// Convert a NASA PRECIP98 TEFLUN "mtx" sounding file into new CLASS format, 
// deriving altitude along the way
//
# include <stdio.h>
# include <stdlib.h>
# include <errno.h>
# include <string.h>
# include <math.h>
extern "C"
{
# include <met_formulas.h>
}


    

void write_header( FILE *outfile, const char *infile_name, 
		   const char *site_name, int year, int month, int day, 
		   int hour, int minute, float lat, float lon, float alt );
void fdd_dp( float *buf, float **dbufs, int npts, float badval );
void fdd_alt( float *buf, float **dbufs, int npts, float badval );


char *Header[] = {
    "Data Type:                         TEFLUN -> CLASS conversion",
    "Project ID:                        PRECIP98",
    "Launch Site Type/Site ID:          ",
    "Launch Location (lon,lat,alt):     ",
    "GMT Launch Time (y,m,d,h,m,s):     ",
    "Sonde Type/ID/Sensor ID/Tx Freq:   unknown",
    "Met Processor/Met Smoothing:       unknown",
    "Winds Type/Processor/Smoothing:    unknown",
    "Pre-launch Surface Obs Source:     unknown",
    "System Operator/Comments:          converted from file ",
    "/",
    "/",
    " Time  Press  Temp  Dewpt  RH    Uwind  Vwind  Wspd  Dir   dZ       Lon       Lat     Rng   Ang    Alt    Qp   Qt   Qh   Qu   Qv   Quv",
    "  sec    mb     C     C     %     m/s    m/s   m/s   deg   m/s      deg       deg      km   deg     m     mb   C    %    m/s  m/s  m/s",
    "------ ------ ----- ----- ----- ------ ------ ----- ----- ----- ---------- --------- ----- ----- ------- ---- ---- ---- ---- ---- ----"
};




main( int argc, char *argv[] )
{
//
// Arg check
//
    if (argc != 6)
    {
	fprintf( stderr, "Usage: %s <mtx_file> <site> <lat> <lon> <alt(m)>\n",
		 argv[0] );
	exit( 1 );
    }

    char *infile_name = argv[1];
    char *site_name = argv[2];

    char *ok;

    float lat = strtod( argv[3], &ok );
    if (! ok)
    {
	fprintf( stderr, "Bad latitude '%s'\n", argv[3] );
	exit( 1 );
    }

    float lon = strtod( argv[4], &ok );
    if (! ok)
    {
	fprintf( stderr, "Bad longitude '%s'\n", argv[4] );
	exit( 1 );
    }

    float site_alt = strtod( argv[5], &ok );
    if (! ok)
    {
	fprintf( stderr, "Bad altitude '%s'\n", argv[5] );
	exit( 1 );
    }
//
// Verify that we got the mtx file, then tease out the date/time from the 
// file name
//
    if (strcmp( infile_name + strlen( infile_name ) - 4, ".mtx" ))
    {
	fprintf( stderr, "The input file name must end in '.mtx'\n" );
	exit( 1 );
    }
    
    int mmddhhmm;
    char *tail = strrchr( infile_name, '/' );
    if (tail)
	tail++;	// skip the '/'
    else
	tail = infile_name;
    
    sscanf( tail, "v%d", &mmddhhmm );

    int year = 1998;	// fixed, since we have nothing to clue us in...
    int month = mmddhhmm / 1000000;
    int day = (mmddhhmm / 10000) % 100;
    int hour = (mmddhhmm / 100) % 100;
    int minute = mmddhhmm % 100;
//
// Open the input file...
//
    FILE *infile;
    if (!(infile = fopen( infile_name, "r" )))
    {
	fprintf( stderr, "Error (%s) opening file %s\n", strerror( errno ),
		 infile_name );
	exit( 1 );
    }
//
// ...and the output file
//
    FILE *outfile;
    char outfile_name[40];
    sprintf( outfile_name, "%s.%4d%02d%02d.%02d%02d00.class", site_name,
	     year, month, day, hour, minute );
    if (!(outfile = fopen( outfile_name, "w")))
    {
	fprintf( stderr, "Error (%s) opening output file '%s'\n", 
		 strerror( errno ), outfile_name );
	exit( 1 );
    }
//
// Skip the first 9 lines of the input file
//
    char line[160];
    int pt;

    for (pt = 0; pt < 9; pt++)
	fgets( line, sizeof( line ), infile );
    
//
// Grab all the data
//
    const int arraylen = 10000;
    float time[arraylen], pres[arraylen], temp[arraylen], rh[arraylen];

    for (pt = 0; fgets( line, sizeof( line ), infile ); pt++)
    {
	if (pt == arraylen)
	{
	    fprintf( stderr, "Can't handle > %d points!\n", arraylen );
	    exit( 1 );
	}
	sscanf( line, "%f%f%f%f", time + pt, pres + pt, temp + pt, rh + pt );
    }

    fclose( infile );

    int npts = pt;
//
// Derive dp and then alt
//
    float *dbufs[5];
    float dp[arraylen];
    float alt[arraylen];
    float mtxbadval = -999.99;

    dbufs[0] = temp;
    dbufs[1] = rh;
    fdd_dp( dp, dbufs, npts, mtxbadval );

    dbufs[0] = dp;
    dbufs[1] = temp;
    dbufs[2] = pres;
    fdd_alt( alt, dbufs, npts, mtxbadval );
//
// Adjust alts to MSL
//
    for (pt = 0; pt < npts; pt++)
    {
	if (alt[pt] != mtxbadval)
	    alt[pt] += site_alt;
    }
//
// Change the mtx bad values into CLASS bad values
//
    for (pt = 0; pt < npts; pt++)
    {
	float timebadval = 9999.0;
	float presbadval = 9999.0;
	float tempbadval = 999.0;
	float rhbadval = 999.0;
	float dpbadval = 999.0;
	float altbadval = 99999.0;
	
	if (time[pt] == mtxbadval)
	    time[pt] = timebadval;

	if (pres[pt] == mtxbadval)
	    pres[pt] = presbadval;

	if (temp[pt] == mtxbadval)
	    temp[pt] = tempbadval;

	if (rh[pt] == mtxbadval)
	    rh[pt] = rhbadval;

	if (dp[pt] == mtxbadval)
	    dp[pt] = dpbadval;

	if (alt[pt] == mtxbadval)
	    alt[pt] = altbadval;
    }
//
// Write the output file header
//
    write_header( outfile, infile_name, site_name, year, month, day, hour, 
		  minute, lat, lon, site_alt );
//
// Write the data
//
    for (int i = 0; i < npts; i++)
    {
	fprintf( outfile, "%6.1f %6.1f %5.1f %5.1f %5.1f ", time[i], pres[i], 
		 temp[i], dp[i], rh[i] );
	fprintf( outfile, "999.0 999.0 999.0 999.0 99.0 999.0 999.0 999.0 " );
	fprintf( outfile, "999.0 %7.1f 99.0 99.0 99.0 99.0 99.0 99.0\n", 
		 alt[i] );
    }

    fclose( outfile );
}



void
write_header( FILE *outfile, const char *infile_name, const char *site_name, 
	      int year, int month, int day, int hour, int minute, 
	      float lat, float lon, float alt )
{
    for (int i = 0; i < sizeof( Header ) / sizeof( *Header ); i++)
    {
	fprintf( outfile, "%s", Header[i] );
    //
    // Add sounding-specific info where necessary
    //
	switch (i)
	{
	//
	// site name
	//
	  case 2:
	    fprintf( outfile, site_name );
	    break;
	//
	// lat/lon
	//
	  case 3:
	    {
		char ew = (lon > 0.0) ? 'E' : 'W';
		char ns = (lat > 0.0) ? 'N' : 'S';
		int latdeg = (int)(fabs( lat ));
		float latmin = 60.0 * (fabs( lat ) - latdeg);
		int londeg = (int)(fabs( lon ));
		float lonmin = 60.0 * (fabs( lon ) - londeg);
		fprintf( outfile, "%d %.2f' %c, %d %.2f' %c, %.4f, %.4f, %d",
			 londeg, lonmin, ew, latdeg, latmin, ns, lon, lat, 
			 (int)alt );
	    }
	    break;
	//
	// date/time
	//
	  case 4:
	    fprintf( outfile, "%d, %02d, %02d, %02d:%02d:00", year, month,
		     day, hour, minute );
	    break;
	//
	// source file
	//
	  case 9:
	    fprintf( outfile, infile_name );
	    break;
	}
	fprintf( outfile, "\n" );
    }
}

    

# define T_K	273.15
# define R_D	287.
# define G_0	9.80665

# define DEG_TO_RAD(x)	((x) * 0.017453292)
# define RAD_TO_DEG(x)	((x) * 57.29577951)



void
fdd_alt(float *buf, float **dbufs, int npts, float badval)
//
// altitude derivation routine
//
// IMPORTANT NOTE:  The altitudes returned are not MSL!  The heights returned
// are relative to the first good point (essentially AGL).
//
{
	float	*dp = dbufs[0], *temp = dbufs[1], *pres = dbufs[2];
	float	e, vt, pres_prev = badval, alt_prev, vt_prev;
	int	i;
// 
// This derivation is adapted from equation 2.24 of Wallace and
// Hobbs' "Atmospheric Science" (1977) using a simple estimation
// of the integral.
//
	for (i = 0; i < npts; i++)
	{
		if (dp[i] == badval || temp[i] == badval || pres[i] == badval)
			buf[i] = badval;
		else
		{
		//
		// Get the vapor pressure and virtual temperature
		//
			e = e_from_dp (dp[i] + T_K);
			vt = t_v (temp[i] + T_K, pres[i], e);
		//
		// Assign the altitude (the first good point is set to
		// 0 km altitude
		//
			if (pres_prev == badval)
				buf[i] = 0.0;
			else
				buf[i] = alt_prev + R_D / G_0 * 0.5 * 
					(vt / pres[i] + vt_prev / pres_prev) * 
					(pres_prev - pres[i]);

			pres_prev = pres[i];
			vt_prev = vt;
			alt_prev = buf[i];
		}
	}
}




void
fdd_dp (float *buf, float **dbufs, int npts, float badval)
//
// dewpoint derivation routine
//
{
	float	*temp = dbufs[0], *rh = dbufs[1];
	float	e;
	int	i;

	for (i = 0; i < npts; i++)
	{
		if (temp[i] == badval || rh[i] == badval)
			buf[i] = badval;
		else
		{
			e = rh[i] / 100.0 * e_sw (temp[i] + T_K);
			buf[i] = dewpoint (e) - T_K;
		}
	}
}
