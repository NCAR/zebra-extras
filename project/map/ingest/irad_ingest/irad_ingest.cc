# include <stdio.h>
# include <string.h>
# include "bufr_cappi.hh"
# include "nc_grid.hh"



main( int argc, char *argv[] )
{
    int nlevels = argc - 2;
    char **prefixes = argv + 1;
    const char* outfile = argv[argc - 1];
//
// Usage
//
    if (argc < 2)
    {
	fprintf( stderr, 
		 "Usage: %s <in_prefix1> [<in_prefix2> ...] <out_file>\n",
		 argv[0] );
	exit( 1 );
    }
//
// Loop through the file prefixes we were given (one per CAPPI level)
//
    bufr_cappi** bc_list = new bufr_cappi*[nlevels];
    
    int lvl;
    
    for (lvl = 0; lvl < nlevels; lvl++)
    {
	printf( "Processing %s...\n", prefixes[lvl] );
    //
    // Build a bufr_cappi using the decbufr text and image files with this
    // prefix
    //
	char txtfile[128];
	char imgfile[128];

	sprintf( txtfile, "%s.txt", prefixes[lvl] );
	sprintf( imgfile, "%s.img", prefixes[lvl] );
	bc_list[lvl] = new bufr_cappi( txtfile, imgfile );
    //
    // Verify that all files match enough to be in the same volume
    //
	if (lvl == 0)
	    continue;
    //
    // We can apply a data doubling kluge if this image
    // has the same spatial size but half resolution...
    //
	if (bc_list[lvl]->Nx() == bc_list[0]->Nx() / 2 &&
	    bc_list[lvl]->Ny() == bc_list[0]->Ny() / 2 &&
	    bc_list[lvl]->Xstep() == bc_list[0]->Xstep() * 2 &&
	    bc_list[lvl]->Ystep() == bc_list[0]->Ystep() * 2)
	{
	    bc_list[lvl]->DoubleRes();
	    fprintf( stderr, "data doubling applied for %s\n", prefixes[lvl] );
	}
    //
    // There's also an array size doubling kluge we can apply if the
    // resolution is the same but the spatial size is halved...
    //
	if (bc_list[lvl]->Nx() == bc_list[0]->Nx() / 2 &&
	    bc_list[lvl]->Ny() == bc_list[0]->Ny() / 2 &&
	    bc_list[lvl]->Xstep() == bc_list[0]->Xstep() &&
	    bc_list[lvl]->Ystep() == bc_list[0]->Ystep())
	{
	    bc_list[lvl]->DoubleSize();
	    fprintf( stderr, "array padding applied for %s\n", prefixes[lvl] );
	}
    //
    // Verify that we now have something that will merge in properly
    // (Kluge: we now allow time differences up to ten minutes between 
    // the first level and any other levels)
    //
	time_t timediff = abs (bc_list[lvl]->Time() - bc_list[0]->Time());

	if (bc_list[lvl]->Nx() != bc_list[0]->Nx() ||
	    bc_list[lvl]->Ny() != bc_list[0]->Ny() ||
	    timediff > 600)
	{
	    fprintf( stderr, "Image size or time mismatch for %s\n",
		     prefixes[lvl] );
	    exit( 1 );
	}
    //
    // Make sure that the CAPPI levels are in increasing order
    //
	if (bc_list[lvl]->Level() <= bc_list[lvl-1]->Level())
	{
	    fprintf( stderr, "CAPPI level decreases at %s\n", prefixes[lvl] );
	    exit( 1 );
	}
    }

    nc_grid *ncg = new nc_grid( nlevels, outfile );
    for (lvl = 0; lvl < nlevels; lvl++)
	ncg->AddBUFRCappi( bc_list[lvl], lvl );

    printf( "done\n" );
}
