/*
 *  Ingest NATS data.
 */
/*              Copyright (C) 1987,88,89,90,91 by UCAR
 *      University Corporation for Atmospheric Research
 *                 All rights reserved
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
# include <fcntl.h>
# include <sgtty.h>
# include <unistd.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/resource.h>
# include <signal.h>
# include <string.h>

# include <ui.h>
# include <defs.h>
# include <message.h>
# include <timer.h>
# include <DataStore.h>

# define STRLEN 30
# define BADVAL -32768

typedef char NameStr[STRLEN];

FILE	*Of;
int	Df, Nf;
int	NameSize, NumData, NumOurs = -1;
char	DataFile[STRLEN], NameFile[STRLEN], OurFile[STRLEN];
char	*NameArray, Platform[STRLEN];
NameStr	*OurArray;
NameStr	Latitude, Longitude, Altitude;
DataObject	Dobj;

int	Die (), ReadData (), GetParams (), Setup (), Search ();
void	StoreData (), SnarfLoop (); 


main (argc, argv)
int	argc;
char	**argv;
{
	int	i;
/*
 * Parameters check. 
 */
	argc--;
	argv++;
	if (argc < 6)
	{
		printf ("Usage: nats_ingest platform datafile namefile namesize numdata ournamefile\n");
		exit (1);
	}
/*
 * Hook into the world.
 */	
	usy_init ();
	msg_connect (Die, "Nats_Ingest");
	ds_Initialize ();

	signal (SIGINT, Die);
	signal (SIGTERM, Die);
/*
 * Process parameters.
 */
	if (! GetParams (argv)) exit (1);
	if (! Setup ()) exit (1);
/*
 * Initialize some of our data object.
 */
	Dobj.do_org = OrgScalar;
	Dobj.do_badval = BADVAL;
	Dobj.do_flags = 0;
	Dobj.do_nfield = NumOurs;
	for (i = 0; i < NumOurs; i++)
	{	
		Dobj.do_fields[i] = OurArray[i];
		Dobj.do_data[i] = ALLOC(float);
	}
	Dobj.do_aloc = ALLOC(Location);
/*
 * Start getting data.
 */
	SnarfLoop ();
}


int
Setup ()
{
	int	i, size;
/*
 * Get the name array.
 */
	size = NumData * NameSize * sizeof (char);
	if ((NameArray = (char *) malloc (size)) == NULL)
	{
		msg_ELog (EF_EMERGENCY, "Can't get space for NameArray.");
		return (FALSE);
	}
	if (read (Nf, NameArray, size) != size)
	{
		msg_ELog (EF_EMERGENCY, "Error reading NameArray.");
		return (FALSE);
	}
	for (i = 0; i < NumData * NameSize; i++)
		NameArray[i] = tolower (NameArray[i]);
/*
 * Get our array.
 */
	fscanf (Of, "%d", &NumOurs);	
	if (NumOurs <= 0)
	{
		msg_ELog (EF_EMERGENCY, "No names in our file.");
		return (FALSE);
	}
	else if (NumOurs >= MAXFIELD)
	{
		msg_ELog (EF_PROBLEM, "Too many names in our file, using %d.",
			MAXFIELD);
		NumOurs = MAXFIELD;
	}
	fscanf (Of, "%s", Latitude);
	fscanf (Of, "%s", Longitude);
	fscanf (Of, "%s", Altitude);
	size = NumOurs * sizeof (NameStr);
	if ((OurArray = (NameStr *) malloc (size)) == NULL)
	{
		msg_ELog (EF_EMERGENCY, "Can't get space for OurArray.");
		return (FALSE);
	}
	for (i = 0; i < NumOurs; i++)
	{
		if (fscanf (Of, "%s", OurArray[i]) <= 0)
		{
			msg_ELog (EF_EMERGENCY, "Can't read OurArray.");
			return (FALSE);
		}
	}
	return (TRUE);	 
}


int
GetParams (argv)
char	**argv;
/*
 * Verify that the parameters make sense.
 */
{
/*
 * Get the platform name.
 */
	strcpy (Platform, argv[0]);
	msg_ELog (EF_DEBUG, "Platform %s", Platform);
	if ((Dobj.do_id = ds_LookupPlatform (Platform)) == BadPlatform)
	{
		msg_ELog (EF_PROBLEM, "Unknown platform '%s'", Platform);
		return (FALSE);
	}
/*
 * See if we can open the data file.
 */
	strcpy (DataFile, argv[1]);
	msg_ELog (EF_DEBUG, "DataFile %s", DataFile);
	if ((Df = open (DataFile, O_RDONLY)) < 0)
	{
		msg_ELog (EF_EMERGENCY, "Can't open %s (%d).", DataFile,
			errno);
		return (FALSE);
	}
/*
	if (lseek (Df, 0, SEEK_END) == -1)
	{
		msg_ELog (EF_EMERGENCY, "Can't lseek in %s (%d).", DataFile,
			errno);
		return (FALSE);
	}	
*/
/*
 * See if we can open the names file.
 */
	strcpy (NameFile, argv[2]);
	msg_ELog (EF_DEBUG, "NameFile %s", NameFile);
	if ((Nf = open (NameFile, O_RDONLY)) < 0)
	{
		msg_ELog (EF_EMERGENCY, "Can't open %s (%d).", NameFile,
			errno);
		return (FALSE);
	}
/*
 * See if we can get the name size.
 */
	msg_ELog (EF_DEBUG, "NameSize %s", argv[3]);
	if ((NameSize = atoi (argv[3])) < 1)
	{
		msg_ELog (EF_EMERGENCY, "Bad name size %d.", NameSize);
		return (FALSE);
	}
/*
 * See if we can get the number of data values in one package.
 */
	msg_ELog (EF_DEBUG, "NumData %s", argv[4]);
	if ((NumData = atoi (argv[4])) < 1)
	{
		msg_ELog (EF_EMERGENCY, "Bad num data %d.", NumData);
		return (FALSE);
	}
/*
 * See if we can open our names file.
 */
	strcpy (OurFile, argv[5]);
	msg_ELog (EF_DEBUG, "OurFile %s", OurFile);
	if ((Of = fopen (OurFile, "r")) == NULL)
	{
		msg_ELog (EF_EMERGENCY, "Can't open %s (%d).", OurFile,
			errno);
		return (FALSE);
	}
	return (TRUE);
}


void
SnarfLoop ()
/*
 * Loop forever reading data from the file.
 */
{
	int	numread, size;
	float	*buffer;

	size = NumData * sizeof (float);
	if ((buffer = (float *) malloc (size)) == NULL)
	{
		msg_ELog (EF_PROBLEM, "Can't get space for buffer.");
		exit (0);
	}
	while (1)
	{
		if ((numread = ReadData (Df, buffer, size)) == size)
		{
			StoreData (buffer);
		}
		else if (numread < 0)
			msg_ELog (EF_DEBUG, "Error reading %d (%d).", numread,
				errno);
		else msg_ELog (EF_DEBUG, "Read %d.", numread);
	}
}


int
Die ()
/*
 * Finish gracefully.
 */
{
	msg_ELog (EF_DEBUG, "Dying...");
	close (Nf);
	fclose (Of);
	close (Df);
	exit (0);
}



void
StoreData (buffer)
float	*buffer;
/*
 * Put the data in the data store.
 */
{
	time	t;
	int	i, itsat;
/*
 * Get the time.
 */	
	tl_GetTime (&t);
	Dobj.do_begin =  Dobj.do_end = t;
	Dobj.do_times = &t;
	Dobj.do_npoint = 1;
/*
 * Get the location.
 */
	itsat = Search (Latitude, NameArray, NumData, NameSize);
	if (itsat < 0)
	{
		msg_ELog (EF_PROBLEM, "Can't find '%s'.", Latitude);
		return;	
	}
	Dobj.do_aloc->l_lat = buffer[itsat];
	msg_ELog (EF_DEBUG, "latitude = %f", Dobj.do_aloc->l_lat);

	itsat = Search (Longitude, NameArray, NumData, NameSize);
	if (itsat < 0)
	{
		msg_ELog (EF_PROBLEM, "Can't find '%s'.", Longitude);
		return;	
	}
	Dobj.do_aloc->l_lon = buffer[itsat];
	msg_ELog (EF_DEBUG, "longitude = %f", Dobj.do_aloc->l_lon);

	itsat = Search (Altitude, NameArray, NumData, NameSize);
	if (itsat < 0)
	{
		msg_ELog (EF_PROBLEM, "Can't find '%s'.", Altitude);
		return;	
	}
	Dobj.do_aloc->l_alt = buffer[itsat];
	msg_ELog (EF_DEBUG, "altitude = %f", Dobj.do_aloc->l_alt);
/*
 * Fill in the fields.
 */
	for (i = 0; i < NumOurs; i++)
	{
		itsat = Search (OurArray[i], NameArray, NumData, NameSize);
		if (itsat < 0)
		{
			msg_ELog (EF_PROBLEM, "Can't find '%s'.",
				OurArray[i]);
			continue;	
		}
		Dobj.do_data[i][0] = buffer[itsat];	
		msg_ELog (EF_DEBUG, "%s = %f",OurArray[i],Dobj.do_data[i][0]);
	}
/*
 * Store it.
 */
	msg_ELog (EF_DEBUG, "Storing data.");
	ds_PutData (&Dobj, FALSE);
}


int 
ReadData (fd, buffer, n)
int	fd;
int	n;
char	*buffer;
/*
 * Keep reading chars till n of them get into buf, return # read in.
 */
{
	int	i, j, k;
  
	i = n;
	for (j = 0; j < n; )
	{
		if ((k = read (fd, (buffer + j), i)) < 0) return (k);
		if (k == 0) break;
		j += k;
		i -= k;
	}
	return (j);
}

 
int 
Search (name, array, alen, slen)
char	name[];		/* string to search for */
char	*array;		/* char array to search */
int	alen;		/* number of items in array to be searched */
int	slen;		/* length of string to compare */
/*
 * Search the array for name.  Return its index in the array.
 */
{
	int	i;
	char	*temparray;

	temparray = array;
	for(i = 0; i < alen; i++)
	{
		if (strncmp (name, temparray, slen) == 0) 
			return (i);
		temparray += slen;
	}
	return (-1);
}




