//
// Ingestor for Citation ASCII data from PRECIP98
//
# include <unistd.h>
# include <errno.h>

# include <message.h>
# include <defs.h>
# include <DataStore.h>

int MsgHandler( Message *msg );

main( int argc, char *argv[] )
{
    if (argc != 3)
    {
	fprintf( stderr, "Usage: %s <citation_file> <yyyymmdd>\n", argv[0] );
	exit( 1 );
    }
//
// Zebra connection boilerplate
//
    char myname[80];
    sprintf( myname, "citation%X", getpid() );

    if (! msg_connect( MsgHandler, myname ))
    {
	fprintf( stderr, "Error connecting with Zebra message system\n" );
	exit( 1 );
    }

    if (! ds_Initialize())
    {
	msg_ELog( EF_PROBLEM, "Error initializing datastore" );
	exit( 1 );
    }
//
// Platform
//
    PlatformId pid = ds_LookupPlatform( "citation" );
    if (pid == BadPlatform)
    {
        msg_ELog (EF_PROBLEM, "No 'citation' platform!");
        exit (1);
    };
//
// Fields
//
    FieldId flds[32];
    typedef enum 
    {
	dayseconds, gps_lat, gps_lon, ins_lat, ins_lon, alt1, alt2,
	airspeed, temp, dp, unknown, nflds
    } FldPositions;
    
    flds[dayseconds] = F_Lookup("dayseconds[][s][day seconds]");
    flds[gps_lat] = F_Lookup("gps_lat[lat][deg][GPS latitude]" );
    flds[gps_lon] = F_Lookup("gps_lon[lon][deg][GPS longitude]" );
    flds[ins_lat] = F_Lookup("ins_lat[lat][deg][INS latitude]" );
    flds[ins_lon] = F_Lookup("ins_lon[lon][deg][INS longitude]" );
    flds[alt1] = F_Lookup( "alt1[alt][m][altitude 1]" );
    flds[alt2] = F_Lookup( "alt2[alt][m][altitude 2]" );
    flds[airspeed] = F_Lookup( "airspeed[][m][airspeed]" );
    flds[temp] = F_Lookup( "temp[T][degC][temperature]" );
    flds[dp] = F_Lookup( "dp[dp][degC][dewpoint]" );
    flds[unknown] = F_Lookup( "unknown[][][Unknown]" );
//
// Data chunk initialization
//
    float badval = -9999.9;

    DataChunk *dc = dc_CreateDC( DCC_Scalar );
    dc_SetPlatform( dc, pid );
    dc_SetScalarFields( dc, nflds, flds );
    dc_SetLocAltUnits( dc, AU_mMSL );
    dc_SetBadval( dc, badval );
//
// Figure out our base time
//
    int yyyymmdd = atol( argv[2] );
    if (yyyymmdd < 19700101)
    {
	msg_ELog( EF_PROBLEM, "Bad date '%s'.  I need <yyyymmdd>.", argv[2] );
	exit( 1 );
    }

    int year = yyyymmdd / 10000;
    int month = (yyyymmdd / 100) % 100;
    int day = yyyymmdd % 100;

    ZebraTime base;
    TC_ZtAssemble( &base, year, month, day, 0, 0, 0, 0 );
//
// Open the input file
//
    FILE *infile = fopen( argv[1], "r" );
    if (! infile)
    {
	msg_ELog( EF_PROBLEM, "Error %d opening input file '%s'", argv[1] );
	exit( 1 );
    }
//
// For each line...
//
    char line[256];
    float *data = new float[nflds];
    float prevseconds = 0.0;
    
    for (int samp = 0; fgets( line, sizeof( line ), infile ); samp++)
    {
	int nread = sscanf( line, "%f%f%f%f%f%f%f%f%f%f%f", data + 0, 
			    data + 1, data + 2, data + 3, data + 4, data + 5, 
			    data + 6, data + 7, data + 8, data + 9, 
			    data + 10 );

	if (nread < 11)
	{
	    samp--;
	    continue;
	}
	
	data[ins_lon] *= -1;	// Make 'em *WEST* longitudes, dammit!
	data[gps_lon] *= -1;

	float seconds = data[dayseconds];
    //
    // Look out for day rollover
    //
	if (seconds < prevseconds)
	    base.zt_Sec += 86400;

	prevseconds = seconds;
	    
	ZebraTime zt = base;
	zt.zt_Sec += (long) seconds;
	zt.zt_MicroSec = (long)(1.0e6 * (seconds - (long)seconds));

	Location loc;
	loc.l_lat = data[ins_lat];
	loc.l_lon = data[ins_lon];
	loc.l_alt = data[alt1];

	for (int i = 0; i < nflds; i++)
	    dc_AddScalar( dc, &zt, samp, flds[i], data + i );

	dc_SetLoc (dc, samp, &loc);
    }


    ds_Store( dc, 1, NULL, 0 );

    delete[] data;
    dc_DestroyDC( dc );
}

int
MsgHandler( Message *msg )
{
}
