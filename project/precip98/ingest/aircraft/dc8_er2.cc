//
// Ingestor for DC-8 or ER2 ASCII data from PRECIP98
//
# include <unistd.h>
# include <errno.h>

# include <message.h>
# include <defs.h>
# include <DataStore.h>

int MsgHandler( Message *msg );

main( int argc, char *argv[] )
{
//
// Get our platform name based on the executable name
//
    char *lastslash = strrchr (argv[0], '/');
    char *platname = lastslash ? lastslash + 1 : argv[0];
//
// Usage check
//
    if (argc != 3)
    {
	fprintf( stderr, "Usage: %s <%s_file> <yyyymmdd>\n", argv[0], 
		 platname );
	exit( 1 );
    }
//
// Zebra connection boilerplate
//
    char myname[80];
    sprintf( myname, "%s%X", platname, getpid() );

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
    PlatformId pid = ds_LookupPlatform( platname );
    if (pid == BadPlatform)
    {
        msg_ELog (EF_PROBLEM, "No '%s' platform!", platname);
        exit (1);
    };
//
// Fields
//
    FieldId flds[32];
    typedef enum 
    {
	fndx_dayhours, fndx_lat, fndx_lon, fndx_heading, fndx_alt, nflds
    } FldPositions;
    
    flds[fndx_dayhours] = F_Lookup("day_hour[][h][day hours]");
    flds[fndx_lat] = F_Lookup("latitude[lat][deg][latitude]" );
    flds[fndx_lon] = F_Lookup("longitude[lon][deg][longitude]" );
    flds[fndx_heading] = F_Lookup( "heading[][deg][heading]" );
    flds[fndx_alt] = F_Lookup( "altitude[alt][m][altitude]" );
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
    float prev_dh = 0.0;
    int repcount = 0;
    
    for (int samp = 0; fgets( line, sizeof( line ), infile ); samp++)
    {
	int nread = sscanf( line, "%f%f%f%f%f", data + 0, 
			    data + 1, data + 2, data + 3, data + 4 );

	if (nread < 5)
	{
	    samp--;
	    continue;
	}
    //
    // Kluge to force unique ER2 times, which only have .01 hour precision, 
    // and end up repeating time values
    // 
	float dayhours = data[fndx_dayhours];
	if (dayhours == prev_dh)
	    dayhours += 0.001 * ++repcount;
	else
	{
	    prev_dh = dayhours;
	    repcount = 0;
	}
	    
	float seconds = dayhours * 3600;
	ZebraTime zt = base;
	zt.zt_Sec += (long) seconds;
	zt.zt_MicroSec = (long)(1.0e6 * (seconds - (long)seconds));

	Location loc;
	loc.l_lat = data[fndx_lat];
	loc.l_lon = data[fndx_lon];
	loc.l_alt = data[fndx_alt];

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
