/*
 * Field mill data ingest.
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


static char *rcsid = "$Id: fm_ingest.c,v 2.1 1991-09-16 21:31:20 burghart Exp $";

# include <copyright.h>
# include <defs.h>
# include <message.h>
# include <timer.h>
# include <DataStore.h>
# include "SLGrabber.h"



/*
 * The following table is used to map lines onto different types of 
 * stuff.  The field mill line returns a lot of different data, and
 * we need to get the right thing into the right place.
 */

# ifdef __STDC__
	void	InitFMill (char *);
# else
	void	InitFMill ();
# endif

struct init_line
{
	char *il_begin;		/* Beginning of the line	*/
	void (*il_func)();	/* Function to begin ingest	*/
} InitLines[] =
{
	{ "FIELD MILL GRID",	InitFMill	},
	{ 0,			0		}
};


/*
 * If we are currently in a particular type of data, this function is
 * the one to handle input.
 */

void (*LineFunc) () = 0;

/*
 * Stuff for field mill ingest.
 */
# define FMXRES 25
# define FMYRES 25
DataObject FMDobj;
float FMGrid[FMXRES * FMYRES];
int FMXpos, FMYpos;
time FMTime;
bool FMBlank = FALSE;		/* Blank line expected	*/


/*
 * Other forward funcs.
 */
# ifdef __STDC__
	int	MsgHandler (Message *);
	void	NewData	(char *);
	void	FMData (char *);
	void	SetupFMill (void);
	void	FMDone (void);
# else
# endif



main (argc, argv)
int argc;
char **argv;
{
/*
 * Hook up.
 */
	usy_init ();
	if (! msg_connect (MsgHandler, "FMIngest"))
	{
		printf ("Unable to connect to message handler\n");
		exit (1);
	}
	ds_Initialize ();
/*
 * Initializations.
 */
	SetupFMill ();
/*
 * Wait for lines.
 */
	msg_await ();
}




int
MsgHandler (msg)
Message *msg;
/*
 * Input!
 */
{
/*
 * Look for serial data.
 */
	if (msg->m_proto == MT_SLDATA)
		NewData (((sldata *) msg->m_data)->sl_data);
/*
 * Everything else is assumed to be a message handler event.
 */	
	else if (msg->m_proto == MT_MESSAGE)
	{
		struct mh_template *tmpl = (struct mh_template *) msg->m_data;
		if (tmpl->mh_type == MH_SHUTDOWN)
			Die ();
	}
	return (0);
}
	



void
NewData (line)
char *line;
/*
 * A line of serial data has arrived.
 */
{
	int len, i;
/*
 * If there is currently a data function, we're in the middle of something
 * and we need to just pass it off.
 */
	if (LineFunc)
	{
		(*LineFunc) (line);
		return;
	}
/*
 * Otherwise we need to try to match something.
 */
	for (i = 0; InitLines[i].il_begin; i++)
		if (! strncmp (InitLines[i].il_begin, line,
					strlen (InitLines[i].il_begin)))
			break;
	if (InitLines[i].il_begin)
		(*InitLines[i].il_func) (line);
}




void
SetupFMill ()
/*
 * Initial setup for field mills.
 */
{
	RGrid *rg = &FMDobj.do_desc.d_rgrid;
/*
 */
 	if ((FMDobj.do_id = ds_LookupPlatform ("field-mill")) == BadPlatform)
	{
		msg_ELog (EF_EMERGENCY, "Field mill platform unknown");
		exit (1);
	}
	FMDobj.do_org = Org2dGrid;
	FMDobj.do_npoint = 1;
	FMDobj.do_nbyte = FMXRES * FMYRES * sizeof (float);
/* 
 * Hardwired grid info.
 */
	rg->rg_nX = FMXRES;
	rg->rg_nY = FMYRES;
	rg->rg_nZ = 0;
	rg->rg_Xspacing = rg->rg_Yspacing = 1.389;	/* yes, really */
/*
 * Locations.
 */
	FMDobj.do_loc.l_lat = 28.39107;
	FMDobj.do_loc.l_lon = -80.84413;
/*
 * Other stuff.
 */
	FMDobj.do_times = &FMTime;
	FMDobj.do_data[0] = FMGrid;
	FMDobj.do_nfield = 1;
	FMDobj.do_fields[0] = "fstrength";
	FMDobj.do_flags = 0;
	FMDobj.do_badval = -99999;
}





void
InitFMill (line)
char *line;
/*
 * Get started on field mill data.
 */
{
	int h, m, s;
/*
 * Parse out the begin time.
 *
 * FIELD MILL GRID         13:26:00  07/13/1991
 */
	tl_GetTime (&FMDobj.do_begin);
	if (sscanf (line + 20, "%d:%d:%d", &h, &m, &s) < 3)
	{
		msg_ELog (EF_PROBLEM, "Bad FMILL init line: '%s'", line);
		return;
	}
	FMDobj.do_begin.ds_hhmmss = h*10000 + m*100 + s;
	FMDobj.do_times[0] = FMDobj.do_end = FMDobj.do_begin;
/*
 * Now we just set up our line function.
 */
	FMXpos = FMYpos = 0;
	FMBlank = FALSE;
	LineFunc = FMData;
}



int
Die ()
{
	exit (0);
}

void
FMData (line)
char *line;
/*
 * Deal with a line of field mill data.
 */
{
	int n[5], i;
/*
 * If we're expecting a blank line, check for it.
 */
	if (FMBlank)
	{
		if (strlen (line) > 10)
		{
			msg_ELog (EF_PROBLEM,"Got '%s' instead of blank",line);
			LineFunc = 0;
		}
		FMBlank = FALSE;
		return;
	}
/*
 * Try to scan out some numbers.
 */
	if (sscanf (line, "%d %d %d %d %d", n, n + 1, n + 2, n + 3, n + 4) < 5)
	{
		msg_ELog (EF_PROBLEM, "Bad fmill line: '%s'", line);
		LineFunc = 0;
		return;
	}
/*
 * Store them.
 */
	for (i = 0; i < 5; i++)
		FMGrid[FMXpos*FMXRES + FMYpos++] = ((float) n[i])/1000.0;
/*
 * See where we are.
 */
	if (FMYpos >= FMYRES)
	{
		FMXpos++;
		FMYpos = 0;
		if (FMXpos >= FMXRES)
			FMDone ();
		else
			FMBlank = TRUE;
	}
}





void
FMDone ()
/*
 * We're finished with field mill data.
 */
{
	ds_PutData (&FMDobj, FALSE);
	LineFunc = 0;
}
