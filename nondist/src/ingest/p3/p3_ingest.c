/*
 * P-3 data ingest.
 */
/*		Copyright (C) 1987,88,89,90,91 by UCAR
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
static char *rcsid = "$Id: p3_ingest.c,v 1.1 1992-09-11 06:41:16 granger Exp $";

# include <copyright.h>
# include <stdio.h>
# include <errno.h>
# include <math.h>
# include <fcntl.h>
# include <sgtty.h>

# include <defs.h>
# include <message.h>
# include <timer.h>
# include <DataStore.h>


# ifdef __STDC__
	void	Connect (char *, char *);
	void	Login ();
	void	GetPort (char *);
	void	Dial (char *);
	char *	ReadLine (void);
	void	Die (void);
	int	MsgHandler (Message *);
	void	ProcessData (char *);
	void	DobjSetup (void);	
	void	StoreData (void);
	void	GetData (void);
	void	WritePort (char *);
	int	ReadPort (char *, int);
	void	LoseData ();
	void	Logout ();
# else
	void	Connect ();
	void	Login ();
	void	GetPort ();
	void	Dial ();
	char *	ReadLine ();
	void	Die ();
	int	MsgHandler ();
	void	ProcessData ();
	void	DobjSetup ();
	void	StoreData ();
	void	GetData ();
	void	WritePort ();
	int	ReadPort ();
	void	LoseData ();
	void	Logout ();
# endif

# define M_PER_FT 0.30480
# define DEG_TO_RAD(x)	((x) * 0.017453292)

# define USERNAME "NWSHUR\r"
# define PASSWORD "IMNMDM\r"
# define COMMAND  "DO MSG LIST_ID 3\r"
# define BYE	  "BYE\r"
# define ENDODATA "NNNN"
# define ENDOSEGMENTS "DOWNLOAD COMPLETE"
# define PLATFORMID "752025C2"
# define OBSCURECODE "URNT40"
# define PHONENUM "18048240105"
# define DIALPORT "/dev/cua0"

bool GoodPlat = TRUE;
/*
 * Dialing and connecting.
 */
char *Port, *Number;
int Fd;
FILE *FpFd;

/*
 * Data object.
 */
# define NUMFIELD 8
# define MAXPOINT 100
# define MAXSEGMENT 300
DataObject Dobj[MAXSEGMENT];
static int	NumSeg;
static float	P3_Data[MAXSEGMENT][NUMFIELD][MAXPOINT];
static Location Locs[MAXSEGMENT][MAXPOINT];
static time	Times[MAXSEGMENT][MAXPOINT];

int FirstLine = TRUE, SecondLine = FALSE, ThirdLine = FALSE;

char Buffer[500];

char **Argv;
int Argc;

main (argc, argv)
int argc; 
char **argv;
{
	int frequency;
/*
 * Save the arguments.
 */
	Argv = argv;
	Argc = argc;
/*
 * The usual sanity checking.
 */
	if (argc != 2 && argc != 3)
	{
		printf ("Usage: p3_ingest frequency [in_file]\n");
		exit (1);
	}
/* 
 * Hook in.
 */
	usy_init ();
	if (! msg_connect (MsgHandler, "P3Ingest"))
	{
		printf ("Unable to connect to message handler\n");
		exit (1);
	}
/*
 * Initializations.
 */
	ds_Initialize ();
	DobjSetup ();
/*
 * Every so many minutes, get some data.
 */
	frequency = atoi (argv[1]);
	tl_AddRelativeEvent (Login, 0, 0, frequency*INCFRAC);
/*
 * Keep running until we're told to die.
 */
	msg_await ();
}




void
GetData ()
/*
 * Get lines of data and process them. 
 */
{
	char *line;
/*
 * Set the current number of segments.
 */
	NumSeg = 0;
/*
 * Read a line of data from the serial line.
 */
	line = ReadLine ();
/* 
 * While there is data coming in process it.
 */
	while (strstr(line, ENDOSEGMENTS) == NULL)
	{
		if (GoodPlat) 
		{
			ProcessData (line);
			line = ReadLine ();
		}
		else LoseData ();
	}
}


void
LoseData ()
/*
 * Not interested in this data so get rid of it.
 */
{
	char *line;

	line = ReadLine ();
	msg_ELog (EF_DEBUG, "Dropping a line from another plane");
	while (strncmp (line, ENDODATA, sizeof (ENDODATA)) != 0)
	{
		line = ReadLine ();
		msg_ELog (EF_DEBUG, "Dropping a line from another plane");
	}
	line = ReadLine ();
	GoodPlat = TRUE;
	FirstLine = TRUE;
}


void
Login ()
/*
 * Call up and log in.
 */
{
	char buff[2], string[80];
/*
 * Dial the port if we weren't given an input file name
 */
	if (Argc < 3)
	{
	/*
	 * Connect to the port.
	 */
		Connect (Port = DIALPORT, Number = PHONENUM);
	/*
	 * Get the thing's attention.
	 */
		sleep (10);
		WritePort ("\r");
	/*
	 * Read Username:
	 */
		buff[0] = '\0';
		ReadPort (buff + 1, 1);
		while (strncmp (buff, "e:", 2))
		{
			buff[0] = buff[1];
			ReadPort (buff + 1, 1);
		}
	/*
	 * Write the username.
	 */
		WritePort (USERNAME);
		msg_ELog (EF_DEBUG, "Typed username.");
	/*
	 * Read Password:
	 */
		ReadPort (buff, 1);
		while (buff[0] != ':')
			ReadPort (buff, 1);
	/*
	 * Write the password.
	 */
		WritePort (PASSWORD);
		msg_ELog (EF_DEBUG, "Typed password.");
	/*
	 * Read all the junk up to the prompt.
	 */
		ReadPort (buff, 1);
		while (buff[0] != '>')
			ReadPort (buff, 1);
		msg_ELog (EF_INFO, "Logged in.");
	/*
	 * Write the command.
	 */
		WritePort (COMMAND);
		msg_ELog (EF_DEBUG, "Typed command.");
	/*
	 * Read up to the >> prompt.
	 */
		buff[0] = '\0';
		ReadPort (buff + 1, 1);
		while (strncmp (buff, ">>", 2))
		{
			buff[0] = buff[1];
			ReadPort (buff + 1, 1);
		}
	/*
	 * Yes we want the data.
	 */
		WritePort ("Y\r");	
		msg_ELog (EF_DEBUG, "Typed Y.");
		sleep (2);
	/*
	 * Read back the echoed 'Y'
	 */
		ReadPort (buff, 1);
		while (buff[0] != 'Y')
			ReadPort (buff, 1);
	}
	else
	{
	/*
	 * Open the test data file.
	 */
		if ((Fd = open (Argv[2], O_RDWR, 0)) < 0)
		{
			msg_ELog (EF_PROBLEM, "Error %d opening %s", 
				errno, Argv[2]);
			exit (1);
		}
	}
/*
 * Start reading the data.
 */
	GetData ();
/*
 * When we're done getting data store it and logout.
 */
	msg_ELog (EF_INFO, "Storing data.");
	StoreData ();
	msg_ELog (EF_INFO, "Logging out.");
	if (Argc < 3)
		Logout ();

}



void
Logout ()
/*
 * Logout of the remote system.
 */
{
	WritePort (BYE);
	sleep (10);
	ReadPort (Buffer, 500);
}




void
Die ()
/*
 * Die gracefully.
 */
{
	if (Argc < 3)
		Logout ();

	close (Fd);
	exit (1);
}



void
Connect (port, number)
char *port, *number;
/*
 * Make a connection to this port.
 */
{
/*
 * Get the port.
 */
	GetPort (port);
/*
 * Now dial it.
 */
	Dial (number);
}




void
GetPort (port)
char *port;
/*
 * Open and tweak the serial port.
 */
{
	struct sgttyb  tbuf;
	unsigned bits = LDECCTQ;
/*
 * Open the port.
 */
	if ((Fd = open (port, O_RDWR, 0)) < 0)
	{
		msg_ELog (EF_PROBLEM, "Error %d opening %s", errno, port);
		exit (1);
	}
	FpFd = fdopen (Fd, "r+");
/*
 * Tweak it.
 */
	tbuf.sg_flags = CBREAK | EVENP | TANDEM;
	tbuf.sg_ispeed = tbuf.sg_ospeed = B2400;
/*
 * Store the new parameters.
 */
	if (ioctl (Fd, TIOCSETP, &tbuf) != 0)
	{
		msg_ELog (EF_EMERGENCY, "Ioctl error %d", errno);
		DoSound ("bark");
		DoSound ("bark");
		DoSound ("bark");
		exit (1);
	}
	if (ioctl (Fd, TIOCLBIS, &bits) != 0)
	{
		msg_ELog (EF_EMERGENCY, "Ioctl error %d", errno);
		DoSound ("bark");
		DoSound ("bark");
		DoSound ("bark");
		exit (1);
	}
}




void WritePort (stuff)
char *stuff;
/*
 * Write this stuff.
 */
{
	write (Fd, stuff, strlen (stuff));
}



int
ReadPort (dest, len)
char *dest;
int len;
/*
 * Read something back.
 */
{
	int nc, i;

	nc = read (Fd, dest, len);
# ifdef notdef
	for (i = 0; i < nc; i++)
		printf ("%c", dest[i]);
# endif
	return (nc);
}





void
Dial (number)
char *number;
/*
 * Work until something comes in.
 */
{
	while (! DoDial (number))
		sleep (20);		/* Should check msg */
}



int
DoDial (number)
char *number;
/*
 * Try to make a connection to this number.
 */
{
	char buf[80];
	int reply, i;
/*
 * Put the modem in the state we want.
 */
	WritePort ("AT H0 V0 E0\r");
	sleep (5);
	(void) ReadPort (buf, 80);	/* Clean out junk */
/*
 * Tell it to dial.
 */
	WritePort ("ATDT");
	WritePort (number);
	WritePort ("\r");
/*
 * Get something back and see what it thinks.
 */
	while (TRUE)
	{
	/*
	 * Read a reply up to \r
	 */
		buf[0] = '\0';
		i = 0;
		while (TRUE)
		{
			if (ReadPort (buf + i, 1) == 1)
				if (buf[i++] == '\r')
					break;
		}

		sscanf (buf, "%d", &reply);
	/*
	 * Deal with it
	 */
		switch (reply)
		{
		    case 3:
			msg_ELog (EF_PROBLEM, "No carrier detected.");
			return (FALSE);
		    case 4:
			msg_ELog (EF_PROBLEM, "Command error.");
			return (FALSE);
		    case 6:
			msg_ELog (EF_PROBLEM, "No dialtone.");
			return (FALSE);
		    case 8:
			msg_ELog (EF_PROBLEM, "No answer.");
			return (FALSE);
		    case 10:
			msg_ELog (EF_INFO, "Connected (%d)", reply);
			return (TRUE);
		    case 52:
			msg_ELog (EF_INFO, "Ring.");
			break;
		    default:
			msg_ELog (EF_PROBLEM, "Didn't expect to get (%d).",
				reply);
			return (FALSE);
		}
	}
}




char *
ReadLine ()
/*
 * Read in one line of data.
 */
{
	int nread;
/*
 * Skip past stuff until we hit the real data.
 */
	ReadPort (Buffer, 1);
	while (Buffer[0] <= '\040')
		ReadPort (Buffer, 1);	
/*
 * Now look for newlines indicating that whole lines have been read.
 */
	nread = 1;
	while ((Buffer[nread-1] != '\r') && (Buffer[nread-1] != '\n'))
	{
		ReadPort (Buffer + nread, 1);
		nread++;
	}

	Buffer[nread-1] = '\0';
	msg_ELog (EF_DEBUG, "line: '%s'", Buffer);
	return (Buffer);
}




int
MsgHandler (msg)
Message *msg;
/*
 * Input!
 */
{
/*
 * Everything is assumed to be a message handler event.
 */	
	if (msg->m_proto == MT_MESSAGE)
	{
		struct mh_template *tmpl = (struct mh_template *) msg->m_data;
		if (tmpl->mh_type == MH_SHUTDOWN)
			Die ();
	}
	return (0);
}
	



void
DobjSetup ()
/*
 * Initial data object setup for P-3 data.
 */
{
	int i;

 	if ((Dobj[0].do_id = ds_LookupPlatform ("p3")) == BadPlatform)
	{
		msg_ELog (EF_EMERGENCY, "P3 platform unknown");
		DoSound ("bark");
		DoSound ("bark");
		DoSound ("bark");
		exit (1);
	}
	for (i = 0; i < MAXSEGMENT; i++)
	{
		Dobj[i].do_id = Dobj[0].do_id;
		Dobj[i].do_org = OrgScalar;
		Dobj[i].do_badval = -99999;
		Dobj[i].do_flags = 0;
		Dobj[i].do_nfield = NUMFIELD;
		Dobj[i].do_fields[0] = "wdir";
		Dobj[i].do_fields[1] = "wspd";
		Dobj[i].do_fields[2] = "dval";
		Dobj[i].do_fields[3] = "tdry";
		Dobj[i].do_fields[4] = "dp";
		Dobj[i].do_fields[5] = "trans";
		Dobj[i].do_fields[6] = "u_wind";
		Dobj[i].do_fields[7] = "v_wind";
		Dobj[i].do_data[0] = P3_Data[i][0];
		Dobj[i].do_data[1] = P3_Data[i][1];
		Dobj[i].do_data[2] = P3_Data[i][2];
		Dobj[i].do_data[3] = P3_Data[i][3];
		Dobj[i].do_data[4] = P3_Data[i][4];
		Dobj[i].do_data[5] = P3_Data[i][5];
		Dobj[i].do_data[6] = P3_Data[i][6];
		Dobj[i].do_data[7] = P3_Data[i][7];
		Dobj[i].do_aloc = Locs[i];
		Dobj[i].do_times = Times[i];
	}
}





void
ProcessData (line)
char *line;
/*
 * Deal with a line of P-3 data.
 */
{
	int n[8], pt;
	static int month, day, year;
	char platform[9], yr[3], daynum[4];
	char code[20];
	float wdir, wspd;
	time t;
	Location loc;
/*
 * Are we expecting the first line?
 */
	if (FirstLine)
	{
	/*
	 * See if its our platform. 
	 */
		strncpy (platform, line, 8);
		platform[8] = '\0';
		if (strcmp (platform, PLATFORMID) == 0)
		{
		/*
		 * It's our platform, get the date.
		 */
			msg_ELog (EF_DEBUG, "It's our platform");
			GoodPlat = TRUE;
			strncpy (yr, line + 8, 2);
			yr[2] = '\0';
			strncpy (daynum, line + 10, 3);
			daynum[3] = '\0';
			ud_dayn_to_date (atoi (daynum), atoi (yr), 
				&month, &day);
			year = atoi (yr);
		/*
		 * We've seen the first line and now want the second line.
		 */
			FirstLine = FALSE;
			SecondLine = TRUE;
			Dobj[NumSeg].do_npoint = 0;
		}
		else 
		{
			GoodPlat = FALSE;
			msg_ELog (EF_DEBUG, "Not our platform '%s'", platform);
		}
	}
	else if (SecondLine)
	{
	/*
	 * Look for our obscure code in the second line.
	 */
		SecondLine = FALSE;
		strncpy (code, line, 6);
		code[6] = '\0';
		if (strcmp (code, OBSCURECODE) != 0)
		{
			msg_ELog (EF_DEBUG, "NOT our obscure code '%s'", 
				code);
			GoodPlat = FALSE;
		}
		else 
		{
			ThirdLine = TRUE;
			msg_ELog (EF_DEBUG, "It's our obscure code");
		}
	}
	else if (ThirdLine)
	{
	/*
	 * Just get rid of the third line.
	 */
		ThirdLine = FALSE;
	}
	else if (strncmp (line, ENDODATA, sizeof (ENDODATA)) == 0)
	{
	/*
	 * We've reached the end of the data so increment the number
	 * of segments, and start looking for the first line of the 
	 * next segment of data.
	 */
		NumSeg++;
		if (NumSeg >= MAXSEGMENT)
		{
			msg_ELog (EF_EMERGENCY, "Too many segments!");
			DoSound ("bark");
			DoSound ("bark");
			DoSound ("bark");
			exit (1);
		}

		GoodPlat = TRUE;
		FirstLine = TRUE;
	}
	else 	/* a real data line */
	{
	/*
	 * Try to scan out some numbers.
	 */
		if (sscanf (line, "%d %d %d %d %d %d %d %d", n, n + 1, 
			n + 2, n + 3, n + 4, n + 5, n + 6, n + 7) < 8)
		{
			msg_ELog (EF_PROBLEM, "Bad P-3 line: '%s'", line);
			return;
		}

		t.ds_yymmdd = year*10000 + month*100 + day;
		t.ds_hhmmss = n[0];

		pt = Dobj[NumSeg].do_npoint;

		if (pt == 0)
			Dobj[NumSeg].do_begin = t;

		Dobj[NumSeg].do_end = t;

		Times[NumSeg][pt] = t;
	/*
	 * Wind direction and speed (speed is reported in knots, so 
	 * convert to m/s)
	 */
		wdir = P3_Data[NumSeg][0][pt] = (float)(n[5] / 1000);
		wspd = P3_Data[NumSeg][1][pt] = (float)(n[5] % 1000) * 0.51479;
	/*
	 * D-value (whatever that is)
	 */
		P3_Data[NumSeg][2][pt] = (float)(n[4] * M_PER_FT);
	/*
	 * Temperature and dewpoint
	 */
		P3_Data[NumSeg][3][pt] = n[6] * 0.1;
		P3_Data[NumSeg][4][pt] = n[7] * 0.1;
	/*
	 * We have to put in a transponder code, so just use 1
	 */
		P3_Data[NumSeg][5][pt] = 0001;
	/*
	 * Derive u and v wind components from wind speed and direction
	 */
		P3_Data[NumSeg][6][pt] = wspd * 
			cos (DEG_TO_RAD (90.0 - wdir));
		P3_Data[NumSeg][7][pt] = wspd * 
			sin (DEG_TO_RAD (90.0 - wdir));
	/*
	 * Location
	 */
		loc.l_lat = n[1] / 100 + (float)(n[1] % 100) / 60.0;	
		loc.l_lon = -(n[2] / 100 + (float)(n[2] % 100) / 60.0);	
		loc.l_alt = n[3] * M_PER_FT;	
		Locs[NumSeg][pt] = loc;
	/*
	 * Finish up
	 */
		Dobj[NumSeg].do_npoint++;
	}
}





void
StoreData ()
/*
 * Store the P-3 data in correct time order (hopefully).
 */
{
	int i, j;

	for (i = NumSeg - 1; i >= 0; i--)
	{
	/*
	 * Fixup any weird times by subtracting a day.
	 */
		if (Dobj[i].do_begin.ds_hhmmss > Dobj[i].do_end.ds_hhmmss)
			pmu_dsub (&Dobj[i].do_begin.ds_yymmdd,
				  &Dobj[i].do_begin.ds_hhmmss, 100);

		for (j = 0; j < Dobj[i].do_npoint; j++)
			if ((Dobj[i].do_times + j)->ds_hhmmss > 
			     Dobj[i].do_end.ds_hhmmss)
			{
				pmu_dsub (&(Dobj[i].do_times + j)->ds_yymmdd,
					   &(Dobj[i].do_times + j)->ds_hhmmss, 
				    	   100);
			}
	/*
	 * Store the segment.
	 */
		ds_PutData (&Dobj[i], FALSE);
	}
}
