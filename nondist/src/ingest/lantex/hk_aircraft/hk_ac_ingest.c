/*
 * Ingest aircraft data into the system.
 *
 * Usage:
 *	hk_ac_ingest <info_file> <platform> 
 *
 */
/*		Copyright (C) 1987-92 by UCAR
 *	University Corporation for Atmospheric Research
 *		   All rights reserved
 *
 * No part of this work covered by the copyrights herein may be reproduced
 * or used in any form or by any means -- graphic, electronic, or mechanical,
 * including photocopying, recording, taping, or information storage and
 * retrieval systems -- without permission of the copyright owner.
 * 
 * This software and any accompanying written materials are provided "as is"
 * without warranty of any kind.  UCAR expressly disclaims all warranties of
 * any kind, either express or implied, including but not limited to the
 * implied warranties of merchantibility and fitness for a particular purpose.
 * UCAR does not indemnify any infringement of copyright, patent, or trademark
 * through use or modification of this software.  UCAR does not provide 
 * maintenance or updates for its software.
 */

# include <copyright.h>
# include <errno.h>
# include <math.h>
# include <time.h>
# include "ingest.h"

# ifndef lint
MAKE_RCSID ("profs_ingest.c,v 1.2 1992/12/22 21:11:40 granger Exp")
# endif

#define DEG_TO_RAD(x)	((x)*0.017453293)
#define RAND_MAX 2147483647
#define MAX_LINE 1024
#define NFLD 1

void	DoData FP ((void));

/*
 * Global stuff
 */

FILE		*Infile;
FieldId         Fid[NFLD];
int		Nfld = NFLD;
char            Inport[MAX_LINE];
PlatformId	Plat;
Location	Loc;
double          Speed = 150.0;
double          Dirn = 0.0;

# define DC_BADVAL -9999.0

main (argc, argv)
int argc;
char **argv;

{

  long now;
  char line[MAX_LINE];

  /*
   * Basic arg check.
   */

  IngestParseOptions(&argc, argv, NULL);

  if (argc < 3) {
    printf ("Usage: %s <info-file> <platform>\n", argv[0]);
    exit (1);
  }

  /*
   * Hook into the world.
   */

  IngestInitialize ("Hk_ac_ingest");

  /*
   * More initialization.
   */

  if ((Plat = ds_LookupPlatform (argv[2])) == BadPlatform) {
    IngestLog (EF_PROBLEM, "Unknown platform: %s", argv[2]);
    exit (1);
  }

  /*
   * Open the info file and read first line
   */

  if ((Infile = fopen (argv[1], "r")) == NULL) {
    IngestLog (EF_PROBLEM, "Error %d opening '%s'", errno, 
	       argv[1]);
    exit (1);
  }

  /*
   * input serial port number
   */

  fgets (line, MAX_LINE, Infile);
  sscanf (line, "%s", Inport);
  fprintf(stderr, "Inport: %s\n", Inport);
  fclose (Infile);

  /*
   * Initialize the location (for faking data).
   */

  Loc.l_lat = 22.3000;
  Loc.l_lon = 114.1000;
  Loc.l_alt = 1.0;
  now = time(&now);
  srand((unsigned int) now);
  Fid[0] = F_Lookup ("tdry");

  /*
   * Loop forever; storing the data every 30 seconds.
   */

  while (1) {
    DoData ();
    sleep (20);
  }

  exit (0);

}

void DoData ()
     
     /*
      * Store the data
      */

{
  int		fld;
  float		data;
  ZebTime		t;
  DataChunk	*dc;
  char message[MAX_LINE];
  
  double delta_dirn, delta_deg;
  double delta_lon, delta_lat, delta_alt;
  double random_num;

  /*
   * Create and initialize the data chunk
   */

  dc = dc_CreateDC (DCC_Scalar);
  dc->dc_Platform = Plat;
  dc_SetScalarFields (dc, Nfld, Fid);
  dc_SetBadval (dc, DC_BADVAL);

  /*
   * Get the time
   */

  tl_Time (&t);

  /*
   * Fake and store the data field.
   */

  data = t.zt_MicroSec;
  dc_AddScalar (dc, &t, 0, Fid[0], &data);

  /*
   * Store lat, lon and alt.
   */

  random_num = (double) rand() / (double) RAND_MAX - 0.3;
  
  delta_alt = random_num * 0.1;
  delta_dirn = random_num * 90.0;
  Dirn += delta_dirn;
  
  delta_deg = Speed / (3600.0 * 3.0);
  delta_lon = delta_deg * cos(DEG_TO_RAD(Dirn));
  delta_lat = delta_deg * sin(DEG_TO_RAD(Dirn));
  
  Loc.l_lat += delta_lat;
  Loc.l_lon += delta_lon;
  Loc.l_alt += delta_alt;
  
  dc_SetLoc (dc, 0, &Loc);

  /*
   * Save the data chunk.
   */
  
  sprintf(message, "Stored pos %g, %g, %g",
	  Loc.l_lat, Loc.l_lon, Loc.l_alt);
  
  IngestLog (EF_INFO, message);
  ds_Store (dc, FALSE, 0, 0);

  /*
   * Clean up
   */

  dc_DestroyDC (dc);

}
