/*
  This program reads a serial line for the input from the CSU chase vans
  giving latitude & longitude. The incoming format is a 17 byte packet,
  delimited by byte code 0x7 at the start and end. The baud rate is 9600.
  The data seems to be as follows:

  bytes 9, 10, 11: degrees, minutes, seconds of latitude
  bytes 12, 13, 14:  degrees, minutes, seconds of longitude
  byte 6: unit number

  R. Neitzel

  Zebra specifics added by C. Burghart
  */


#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

# include <message.h>
# include <DataStore.h>

char *PlatName = "van";

int newchar (int ifd);
void writepos (double lat, double lon);
int MsgHandler (Message *msg);

static char *usage[] =
{
    "Available options are:\n",
    "	[-i serial_port|-]\n",
    "	[-p platname]\n",
    "	[-d]\n",
    0
};

static int debug = 0;

main(int argc, char **argv)
{
    int ifd;
    int ofd = 1;			/* Default to stdout. */
    struct termios settings;
    char *dev = "/dev/ttyS1";	/* Com2 under dos. */
    int opt;
    char **ptr;
    extern char *optarg;
    extern int optind;

    while ((opt = getopt(argc,argv,"i:d:p:")) != EOF)
    {
	switch(opt)
	{
	  case 'd':		/* Allow for multiple levels. */
	    debug++;
	    break;

	  case 'i':
	    dev = optarg;
	    break;

	  case 'p':
	    PlatName = optarg;
	    break;
	  
	  case '?':
	    for (ptr = usage; *ptr; ptr++)
		fprintf(stderr,"%s",*ptr);
	    exit(1);
	    break;
	}
    }

    if (!strcmp("tty",dev))
	ifd = 0;
    else 
    {
	if ((ifd = open(dev,O_RDONLY|O_NOCTTY)) < 0)
	{
	    perror(argv[0]);
	    exit(2);
	}
	
	settings.c_iflag = IGNBRK|IGNPAR;
	settings.c_cflag = CS8|CLOCAL|CREAD;
	settings.c_lflag = NOFLSH;
	settings.c_oflag = 0;

	if (cfsetispeed(&settings,B9600))
	{
	    perror(argv[0]);
	    exit(3);
	}
	if (cfsetospeed(&settings,B9600))
	{
	    perror(argv[0]);
	    exit(3);
	}

	if (tcsetattr(ifd,TCSAFLUSH,&settings))
	{
	    perror(argv[0]);
	    exit(3);
	}
    }
/*
 * Zebra setup
 */
    zl_usy_init ();
    msg_connect (MsgHandler, "van_input");
    ds_Initialize ();
    msg_ELog (EF_INFO, "van_input: Ingesting van position");
    msg_add_fd (ifd, newchar);

    msg_await ();

    printf ("out of msg_await\n");
    exit(0);			/* We never get here. */
}


int
newchar (int ifd)
{
    unsigned char input;
    static int counter = 0;
    static double lat;
    static double lon;
    static int unit;
/*
 * Get the next char
 */
    if (read(ifd,&input,1) < 0)
    {
	msg_ELog (EF_EMERGENCY, "van_input exiting: Error %d on read", errno);
	exit(3);
    }
    printf ("%d ", input);
    fflush (stdout);
/*
 * Look for the packet start char if we don't have it
 */
    if (! counter)
    {
	if (input == 0x7)		/* Find 1st boundary. */
	    counter = 1;
	else if (debug) 
	    fprintf(stderr,"%X\n",input);

	return 0;
    }
/*
 * Otherwise, continue from our current position
 */
    else
    {
	if (debug > 1) 
	    fprintf(stderr,"Received %X\n",input);

	if (input == 0x7)
	{
	    if (counter == 16)	/* End of packet. */
		writepos(lat,lon);
	    else
		msg_ELog (EF_INFO, "Malformed packet of length %d\n",
			  counter+1);

	    counter = 0;
	    return 0;
	}
	else
	{
	    switch(counter)
	    {
	      case 0:
		msg_ELog (stderr,"Missing packet start character.\n");
		counter = 0;
		return 0;
		break;

	      case 6:		/* Is this used? */
		unit = (int)input;
		break;

	      case 9:
		lat = (double)input;
		break;

	      case 10:
		lat += (double)input / 60.0;
		break;

	      case 11:
		lat += (double)input / 3600.0;
		break;
	    /*
	     * They give us west longitudes as positive values; we convert
	     * to negative
	     */
	      case 12:
		lon = -(double)input;
		break;

	      case 13:
		lon -= (double)input / 60.0;
		break;

	      case 14:
		lon -= (double)input / 3600.0;
		break;
	    }
	    counter++;
	}
    }
    return 0;
}



void 
writepos(double lat, double lon)
{
    DataChunk *dc;
    ZebTime zt;
    Location loc;
    FieldId fid = F_Lookup ("bogus");
    static PlatformId pid = BadPlatform;
/*
 * Get our platform ID if necessary
 */
    if (pid == BadPlatform && 
	(pid = ds_LookupPlatform (PlatName)) == BadPlatform)
    {
	msg_ELog (EF_EMERGENCY, "van_input: No platform '%s' defined", 
		  PlatName);
	exit (1);
    }
/*
 * Create and fill the data chunk
 */
    dc = dc_CreateDC (DCC_Scalar);
    dc_SetScalarFields (dc, 1, &fid);
    dc->dc_Platform = pid;

    loc.l_lat = lat;
    loc.l_lon = lon;
    loc.l_alt = 0.0;
    zt.zt_Sec = time (0);
    zt.zt_MicroSec = 0;

    dc_AddScalar (dc, &zt, 0, fid, &loc.l_alt);
    dc_SetLoc (dc, 0, &loc);
/*
 * Store it and delete it
 */
    ds_Store (dc, FALSE, 0, 0);
    printf ("wrote position %.3f %.3f\n", lat, lon);
    msg_ELog (EF_DEBUG, "van_input: wrote position %.3f %.3f", lat, lon);
    
    dc_DestroyDC (dc);
}



int
MsgHandler (Message *msg)
/*
 * Deal with incoming stuff.
 */
{
    struct mh_template *mt;
    char *at;
/*
 * Make sure this is what we expect.
 */
    switch (msg->m_proto)
    {
      case MT_MESSAGE:
	mt = (struct mh_template *) msg->m_data;
	if (mt->mh_type == MH_SHUTDOWN)
	    exit (0);
	break;
      default:
	msg_ELog (EF_PROBLEM, "Strange proto %d", msg->m_proto);
	return (0);
    }
}

