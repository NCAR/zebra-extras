# include <stdio.h>
# include <math.h>
# include "nc_grid.hh"
# include "bufr_cappi.hh"


# define RAD_TO_DEG(x) ((x) * 180.0 / M_PI)
# define DEG_TO_RAD(x) ((x) * M_PI / 180.0)

const short MissingValue = (short)(-99);



nc_grid::nc_grid( int nlevs, const char* filename = "grid.nc" ) :
    ncfile( filename, NcFile::Replace )
{
    datavar = 0;
//
// Make sure we opened OK
//
    if (! ncfile.is_valid() )
    {
	fprintf( stderr, "Error opening netCDF file %s\n", filename );
	exit( 1 );
    }
//
// Use fill mode
//
    ncfile.set_fill();

    nlevels = nlevs;
}



int
nc_grid::AddBUFRCappi( const bufr_cappi* bc, int levelndx )
{
//
// If this is the first time around, initialize based on this bufr_cappi.
// Otherwise, do some sanity checking.
//
    if (! datavar)
    {
	if (! InitializeFromBUFRCappi( bc ))
	    return 0;
    }
    else
    {
	if (bc->Ny() != nlats || bc->Nx() != nlons)
	{
	    fprintf( stderr, 
		     "nc_grid: cannot add level with different dimensions\n" );
	    fprintf( stderr, "(%d x %d) != (%d x %d)\n", bc->Nx(), bc->Ny(),
		     nlons, nlats );
	    exit( 1 );
	}
	if (levelndx < 0 || levelndx >= nlevels)
	{
	    fprintf( stderr, "nc_grid: cannot add bad level %d\n", levelndx );
	    exit( 1 );
	}
    }
//
// Stuff in the data for this level, going one row at a time
//
    altvar->set_cur( levelndx );
    float level = 0.001 * bc->Level();	// convert to km
    altvar->put( &level, 1 );

    short* vals = new short[nlons];
    const float* table = bc->LookupTable();

    for (int y = 0; y < nlats; y++)
    {
	const unsigned char *dp = bc->Data() + (nlats - y - 1) * nlons;
	
	for (int x = 0; x < nlons; x++)
	    vals[x] = dp[x] ? (short)(table[dp[x]]) : MissingValue;

	datavar->set_cur( 0, levelndx, y, 0 );
	datavar->put( vals, 1, 1, 1, nlons );
    }

    ncfile.sync();

    return 1;
}


int
nc_grid::InitializeFromBUFRCappi( const bufr_cappi *bc )
{
    NcVar *var;
//
// Global missing_value attribute
//
//
// time dimension, base_time, and time_offset    
//
    NcVar *btvar = ncfile.add_var( "base_time", ncInt );
    btvar->add_att( "long_name", "Base time in Epoch" );
    btvar->add_att( "units", "seconds since 1970-1-1 0:00:00 0:00" );

    NcDim *timedim = ncfile.add_dim( "time", 1 );
    NcVar *tvar = ncfile.add_var( "time_offset", ncDouble, timedim );
    tvar->add_att( "long_name", "Time offset from base_time" );
//
// latitude, longitude, and altitude coordinate variables
//
    nlats = bc->Ny();
    nlons = bc->Nx();

    NcDim *latdim = ncfile.add_dim( "latitude", nlats );
    NcDim *londim = ncfile.add_dim( "longitude", nlons );
    NcDim *altdim = ncfile.add_dim( "altitude", nlevels );

    NcVar *latvar = ncfile.add_var( "latitude", ncFloat, latdim );
    latvar->add_att( "long_name", "North latitude" );
    latvar->add_att( "units", "deg" );

    NcVar *lonvar = ncfile.add_var( "longitude", ncFloat, londim );
    lonvar->add_att( "long_name", "East longitude" );
    lonvar->add_att( "units", "deg" );

    altvar = ncfile.add_var( "altitude", ncFloat, altdim );
    altvar->add_att( "long_name", "MSL altitude" );
    altvar->add_att( "units", "km" );
//
// The main data variable, fixed to "refl"/"Horizontal reflectivity"
//
    datavar = ncfile.add_var( "refl", ncShort, timedim, altdim, latdim, 
			      londim );
    datavar->add_att( "long_name", "Horizontal reflectivity" );
    datavar->add_att( "units", "dBZ" );
    datavar->add_att( "missing_value", MissingValue );
//
// We're done defining, so put in some data values starting with the times.
//
    long t = bc->Time();
    btvar->put( &t, 1 );

    double zero = 0.0;
    tvar->put( &zero, 1 );
//
// lats & lons
//
    const float r_earth = 6378000; // meters
    int l;

    float latstep = RAD_TO_DEG( bc->Ystep() / r_earth );
    float lat0 = bc->Lat() + latstep * 0.5 * (nlats - 1);
    for (l = 0; l < nlats; l++)
    {
	float lat = lat0 - l * latstep;

	latvar->set_cur( l );
	latvar->put( &lat, 1 );
    }
    
    float lonstep = RAD_TO_DEG( bc->Xstep() / 
				(r_earth * cos( DEG_TO_RAD( bc->Lat() ) )) );
    float lon0 = bc->Lon() - lonstep * 0.5 * (bc->Nx() - 1);
    for (l = 0; l < nlons; l++)
    {
	float lon = lon0 + l * lonstep;

	lonvar->set_cur( l );
	lonvar->put( &lon, 1 );
    }
}

