//
// Simple netCDF lat/lon/alt grid class
//
# ifndef _NC_GRID_HH_
# define _NC_GRID_HH_

# include <time.h>
# include <netcdf.hh>

class bufr_cappi;

class nc_grid
{
private:
    NcFile ncfile;
    NcVar *datavar;
    NcVar *altvar;

    int nlats;
    int nlons;
    int nlevels;

    int InitializeFromBUFRCappi( const bufr_cappi* bc );
public:
    nc_grid( int nlevs, const char* filename = "grid.nc" );
    int AddBUFRCappi( const bufr_cappi* bc, int levelndx );
};

# endif // _NC_GRID_HH_
