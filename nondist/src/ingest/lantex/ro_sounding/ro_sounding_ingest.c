/*
 * Ingest RO Sounding ASCII data into the system.
 *
 * Usage:
 *	ro_sounding_ingest <platform> <data file>
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
# include "ingest.h"

# ifndef lint
MAKE_RCSID ("ro_sounding_ingest.c,v 1.2 1992/12/22 21:11:40 granger Exp")
# endif

# define DEG_TO_RAD(x)	((x)*0.017453293)

void	DoData FP ((void));

/*
 * Global stuff
 */
# define MAX_FIELD 40
# define MAX_PLAT 100

FILE		*Infile;

FieldId		Fid[MAX_FIELD];
char		Scratch[256];
PlatformId	Plat;

# define DC_BADVAL -9999.0




main (argc, argv)
int argc;
char **argv;
{
/*
 * Basic arg check.
 */
	IngestParseOptions(&argc, argv, NULL);
	if (argc < 3)
	{
		printf ("Usage: %s <platform> <data_file>\n", argv[0]);
		exit (1);
	}
/*
 * Hook into the world.
 */
	IngestInitialize ("ROSoundingIngest");
/*
 * More initialization.
 */
	if ((Plat = ds_LookupPlatform (argv[1])) == BadPlatform)
	{
		IngestLog (EF_PROBLEM, "Unknown platform: ", argv[1]);
		exit (1);
	}
/*
 * Open the data file.
 */
	if ((Infile = fopen (argv[2], "r")) == NULL)
	{
		IngestLog (EF_PROBLEM, "Error %d opening '%s'", errno, 
			argv[2]);
		return;
	}
/*
 * Get the data. 
 */
	DoData ();
}



void
DoData ()
/*
 * Read and store the data
 */
{
	int	year, month, day, hour, tmp, junk, test, sample;
	int	NumField;
	float	pres, tdry, dp, u_wind, v_wind, alt;
	ZebTime	t;
	DataChunk	*dc;
	Location	loc;
/*
 * Declare the fields
 */
	Fid[0] = F_Lookup ("pres");
	Fid[1] = F_Lookup ("tdry");
	Fid[2] = F_Lookup ("dp");
	Fid[3] = F_Lookup ("u_wind");
	Fid[4] = F_Lookup ("v_wind");
	NumField = 5;
/*
 * Create and initialize the data chunk
 */
	dc = dc_CreateDC (DCC_Scalar);
	dc->dc_Platform = Plat;
	dc_SetScalarFields (dc, NumField, Fid);
	dc_SetBadval (dc, DC_BADVAL);
/*
 * Read and discard the first number in the file. 
 */
	fscanf (Infile, "%d", &tmp);
/*
 * Read the date.
 */
	fscanf (Infile, "%d %d %d %d", &year, &month, &day, &hour);
	TC_ZtAssemble (&t, year, month, day, hour, 0, 0, 0);
/*
 * Now read through the entire file getting the fields and storing them
 * in the data chunk. 
 */	
	sample = 0;
	while (fscanf (Infile, "%f %f %f %f %f %f", &pres, &tdry, &dp, &u_wind,
		&v_wind, &alt) == 6)
	{
	/*
	 * Read the rest of the line in the input file.
	 */
		fscanf (Infile, "%f %f %f", &tmp, &junk, &test);
	/*
	 * Store the fields.
	 */
		dc_AddScalar (dc, &t, sample, Fid[0], &pres);
		dc_AddScalar (dc, &t, sample, Fid[1], &tdry);
		dc_AddScalar (dc, &t, sample, Fid[2], &dp);
		dc_AddScalar (dc, &t, sample, Fid[3], &u_wind);
		dc_AddScalar (dc, &t, sample, Fid[4], &v_wind);
	/*
	 * Store the location.
	 */
		loc.l_lat = 27.3092;
		loc.l_lon = 114.1728;
		if (sample == 0) loc.l_alt = 0.0;  /* Start on the ground */
		else loc.l_alt = alt;
		dc_SetLoc (dc, sample, &loc);
	/*
	 * Increment the time a bit and the sample number.
	 */
		t.zt_Sec += 1;
		sample++;
	}
/*
 * Store the data chunk.
 */
	ds_Store (dc, FALSE, 0, 0);
/*
 * Clean up
 */
	dc_DestroyDC (dc);
}

