# ifndef _BUFR_CAPPI_HH_
# define _BUFR_CAPPI_HH_

# include <stdio.h>
# include <time.h>

class bufr_cappi
{
public:
    bufr_cappi( const char* decbufr_txt, const char* decbufr_img );
    ~bufr_cappi( void );
    const unsigned char* Data( void ) const { return data; }
    int Nx( void ) const { return nx; }
    int Ny( void ) const { return ny; }
    float Xstep( void ) const { return xstep; }
    float Ystep( void ) const { return xstep; }
    const float* LookupTable( void ) const { return table; }
    float Lat( void ) const { return lat; }
    float Lon( void ) const { return lon; }
    float Alt( void ) const { return alt; }
// CAPPI level in meters
    float Level( void ) const { return level; }
// Unix time
    time_t Time( void ) const;
// double our effective resolution by replicating values in x and y
    void DoubleRes( void );
// double our array size by centering the data in a new array that's otherwise
// zero-filled
    void DoubleSize( void );

private:
    const static int badint = -999;
    const static float badfloat = -99999.0;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int nx;
    int ny;
    float xstep;	// m
    float ystep;	// m
    float table[256];	// lookup table
    float lat;
    float lon;
    float alt;		// site altitude (m)
    float level;	// cappi altitude (m MSL)

    unsigned char *data;

    void InitFromDecbufrTxt( const char* decbufr_txt );
    int NextIVal( FILE *txtfile );
    float NextFVal( FILE *txtfile );
};

# endif // _BUFR_CAPPI_HH_
