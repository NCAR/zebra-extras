/*
 *  Ingest aircraft data from the LLP line.
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

static char *rcsid = "$Id: llp_ingest.c,v 1.2 1992-08-13 00:53:41 granger Exp $";

# include <copyright.h>
# include <stdio.h>
# include <errno.h>
# include <math.h>
# include <fcntl.h>
# include <sgtty.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/resource.h>
# include <signal.h>
# include <string.h>

# include "llp_ingest.h"
# include "SLGrabber.h"

/*
 * Buffering parameters, so that we don't put each flash into the
 * data store separately.
 */
# define BUFFERTIME 10
# define MAXKEEP 20

static int	NewMessage ();
int		DoData ();
static void	Go (), StoreData(), Timeout (); 

/*
 * Flash data.
 */
static float	Strikes[MAXKEEP];
static Location	Locs[MAXKEEP];
static time	Times[MAXKEEP];

int Timer_id = -1;

Die ()
/*
 * Finish gracefully.
 */
{
	msg_ELog (EF_DEBUG, "Dying...");
	ui_finish ();
	exit (0);
}


main (argc, argv)
int	argc;
char	**argv;
{
	SValue	v;
/*
 * Initialize.
 */
	usy_init ();
	msg_connect (NewMessage, "LLP_Ingest");
	ds_Initialize ();
/*
 * Fill in the data object.
 */
	if ((Dobj.do_id = ds_LookupPlatform ("lightning")) == BadPlatform)
	{
		msg_ELog (EF_PROBLEM, "Unknown platform lightning.");
		exit (1);
	}
	Dobj.do_org = OrgScalar;
	Dobj.do_badval = BADVAL;
	Dobj.do_flags = 0;
	Dobj.do_nfield = 1;
	Dobj.do_fields[0] = "strikes";
	Dobj.do_data[0] = Strikes;
	Dobj.do_npoint = 0;
	Dobj.do_aloc = Locs;
	Dobj.do_times = Times;
	
	signal (SIGINT, Die);
	signal (SIGTERM, Die);
/*
 * Pull in data.
 */
 	Go ();
}




static int
NewMessage (msg)
Message *msg;
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
	




static void
Go ()
/*
 * Start reading data from the modem.
 */
{
	msg_await ();
}


int
NewData (line)
char *line;
/*
 * Handler which deals with the data.
 */
{
	Location	loc;
	int h, m, s, latd, latm, lats, lond, lonm, lons, nstrike, ijunk;
	float junk;
	time t;

/* 21:13:21.90    29:09:58  -81:10:50  -27.1   2     5,4,1,3 */
/*
 * Parse it out.
 */
	if (line[0] == '\r')
		strcpy (line, line + 1);
 	if (sscanf (line, "%d:%d:%d.%d %d:%d:%d %d:%d:%d %f %d",
			&h, &m, &s, &ijunk, &latd, &latm, &lats, &lond, &lonm,
			&lons, &junk, &nstrike) != 12)
	{
		msg_ELog (EF_INFO, "Bad LLP line '%s'", line);
		return;
	}
/*
 * Stuff the data into our structure.
 */
	tl_GetTime (&t);		/* Get day	*/
	t.ds_hhmmss = h*10000 + m*100 + s;
	if (Dobj.do_npoint == 0)
		Dobj.do_begin = t;
	else if (t.ds_hhmmss == Times[Dobj.do_npoint - 1].ds_hhmmss)
	{
		msg_ELog (EF_DEBUG, "Time drop :-(");
		return;
	}
	Times[Dobj.do_npoint] = t;
	Strikes[Dobj.do_npoint] = (float) nstrike;
	loc.l_lat = latd + ((float) latm)/60.0 + ((float) lats)/3600.0;
	loc.l_lon = lond - ((float) lonm)/60.0 + ((float) lons)/3600.0;
	Dobj.do_aloc[Dobj.do_npoint] = loc;
	msg_ELog (EF_DEBUG, "%d strikes at %.4f %.4f, %d", nstrike, loc.l_lon,
		loc.l_lat, t.ds_hhmmss);
/*
 * If this is the first point, we need to fire up a timer request.
 */
	Dobj.do_npoint++;
	if (Dobj.do_npoint == 1)
		Timer_id = tl_AddRelativeEvent (Timeout, 0, BUFFERTIME*INCFRAC,
				0);
/*
 * Otherwise if this one has filled the buffer, we need to dump it.
 */
	else if (Dobj.do_npoint == MAXKEEP)
		StoreData ();
/*
 * Otherwise we wait.
 */
	return (1);
}




static void
Timeout (t, junk)
time *t;
int junk;
/*
 * Time to store our stuff.
 */
{
	Timer_id = -1;
	if (Dobj.do_npoint > 0)
		StoreData ();
}




static void
StoreData ()
/*
 * Put the aircraft data in the data store.
 */
{
	msg_ELog (EF_DEBUG, "Store %d flashes", Dobj.do_npoint);
	Dobj.do_end = Times[Dobj.do_npoint - 1];

	ds_PutData (&Dobj, FALSE);
	Dobj.do_npoint = 0;
}
