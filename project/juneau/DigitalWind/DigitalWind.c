/*
 * Digital wind display for Juneau runway anemometers
 *
 *		Copyright (C) 1996 by UCAR
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
# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <sys/types.h>
# include <time.h>

# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/Intrinsic.h>
# include <X11/StringDefs.h>
# include <X11/Shell.h>
# include <X11/Xaw/Form.h>
# include <X11/Xaw/Label.h>

# include <message.h>
# include <DataStore.h>

int	Interval = 1;	/* update interval in seconds */
int	StatPeriod = 60;	/* statistics period in seconds */
XtAppContext	Appc;
PlatformId	Pid;

Widget	Wspd, WspdMin, WspdMax, Wdir;

String FallbackResources[] =
{
    "*background: lightblue",
    "*borderWidth: 0",
    "*justify: right",
    "*Form.borderWidth: 4",
    "*Form.defaultDistance: 0",
    "*font: -*-helvetica-medium-r-normal--24-*-*-*-*-*-*-*",
    "*wspd.font: -*-helvetica-medium-r-normal--40-*-*-*-*-*-*-*",
    "*wdir.font: -*-helvetica-medium-r-normal--40-*-*-*-*-*-*-*",
    "*wspdMin.foreground: gray40",
    "*wspdMax.foreground: gray40",
    "*wspdMin.internalHeight: 10",
    "*wspdMax.internalHeight: 10",
    "*wsUnits.internalHeight: 10",
    "*wdUnits.internalHeight: 10",
    0
};


static int	msg_handler (struct message *msg);
static void	Update (XtPointer client_data, XtIntervalId *timer);
static void	XZebraHandler (XtPointer client_data, int *source, 
			       XtInputId *id);



main (int argc, char **argv)
{
	Widget		top, form, title, wsunits, wdunits;
	String		name;
	XtTranslations	table;
	char	ourname[40];

	/*
	 * Connect to the message handler; we're a Zebra application!
	 */
	sprintf (ourname, "DigitalWind_%x", getpid());
  
	if (! msg_connect (msg_handler, ourname))
	{
	    fprintf (stderr, "%s (%s): unable to connect to message handler\n",
		     ourname, argv[0]);
	    exit (1);
	}

	usy_init ();
	ds_Initialize ();

	/*
	 * Connect to X; we're also an X application!
	 */
	top = XtOpenApplication (&Appc, "DigitalWind", NULL, 0, &argc, argv, 
				 FallbackResources, sessionShellWidgetClass, 
				 NULL, 0);
	XtVaGetValues (top, XtNtitle, &name, 0);

	/*
	 * Get our platform
	 */
	if (argc != 2)
	{
	    fprintf (stderr, "Usage: %s <platform>\n", argv[0]);
	    exit (1);
	}

	if ((Pid = ds_LookupPlatform (argv[1])) == BadPlatform)
	{
	    fprintf (stderr, "Bad platform: %s\n", argv[1]);
	    exit (1);
	}

	/*
	 * Build the display
	 */
	form = XtVaCreateManagedWidget ("form", formWidgetClass, top, 0);
	title = XtVaCreateManagedWidget ("title", labelWidgetClass, form,
					 XtNlabel, name, 0);
	WspdMin = XtVaCreateManagedWidget ("wspdMin", labelWidgetClass, form,
					   XtNfromVert, title, 
					   XtNlabel, "999", 0);
	Wspd = XtVaCreateManagedWidget ("wspd", labelWidgetClass, form,
					XtNfromVert, title, 
					XtNfromHoriz, WspdMin,
					XtNlabel, "999", 0);
	wsunits = XtVaCreateManagedWidget ("wsUnits", labelWidgetClass, form,
					   XtNfromVert, title, 
					   XtNfromHoriz, Wspd,
					   XtNlabel, "kts", 0);
	WspdMax = XtVaCreateManagedWidget ("wspdMax", labelWidgetClass, form,
					   XtNfromVert, title,
					   XtNfromHoriz, wsunits,
					   XtNlabel, "999", 0);
	Wdir = XtVaCreateManagedWidget ("wdir", labelWidgetClass, form,
					XtNfromVert, Wspd,
					XtNfromHoriz, WspdMin,
					XtNlabel, "999", 0);
	wdunits = XtVaCreateManagedWidget ("wdUnits", labelWidgetClass, form,
					   XtNfromVert, Wspd, 
					   XtNfromHoriz, Wdir,
					   XtNlabel, "\260mag.", 0);
	XtRealizeWidget (top);

	/*
	 * Give the Zebra message file descriptor to X, so we can react when
	 * Zebra messages arrive.
	 */
	XtAppAddInput (Appc, msg_get_fd(), (XtPointer) XtInputReadMask, 
		       (XtInputCallbackProc) XZebraHandler, 0);

	/*
	 * Update the display and set an interval timer
	 */
	Update (NULL, NULL);

	/*
	 * Now just loop forever
	 */
	XtAppMainLoop (Appc);
	exit (0);
}



static void
Update (XtPointer client_data, XtIntervalId *timer)
{
    static FieldId	flds[2], wsfld = BadField, wdfld = BadField;
    static int		nflds;

    char	label[16];
    int		npts, i;
    float	ws, wd, ws_max, ws_min, badval;
    time_t	now = time (0);
    ZebTime	btime, etime;
    DataChunk	*dc;

    /*
     * Lookup our fields if necessary
     */
    if (wsfld == BadField)
    {
	wsfld = flds[0] = F_Lookup ("wspd_k");
	wdfld = flds[1] = F_Lookup ("wdir_mag");
	nflds = 2;
    }
	
    /*
     * Get the last StatPeriod seconds of data from our anemometer
     */
    btime.zt_Sec = now - StatPeriod;
    btime.zt_MicroSec = 0;
    if (! ds_DataTimes (Pid, &btime, 1, DsAfter, &btime))
    {
	msg_ELog (EF_DEBUG, "No data!");
	dc = 0;
    }
    else
    {
	etime.zt_Sec = now + 1;
	etime.zt_MicroSec = 0;
	ds_DataTimes (Pid, &etime, 1, DsBefore, &etime);

	if (! (dc = ds_Fetch (Pid, DCC_Scalar, &btime, &etime, flds, nflds, 
			      0, 0)))
	    msg_ELog (EF_PROBLEM, "Failed to get promised data!");
    }

    badval = dc ? dc_GetBadval (dc) : -999.;
    npts = dc ? dc_GetNSample (dc) : 0;

    /*
     * Find the max and min wind speed over the period
     */
    ws_min = badval;
    ws_max = badval;

    for (i = 0; i < npts; i++)
    {
	if ((ws = dc_GetScalar (dc, i, wsfld)) == badval)
	    continue;

	if (ws_min == badval || ws < ws_min)
	    ws_min = ws;

	if (ws_max == badval || ws > ws_max)
	    ws_max = ws;
    }

    /*
     * Get the most recent speed and direction
     */
    ws = dc ? dc_GetScalar (dc, npts-1, wsfld) : badval;
    wd = dc ? dc_GetScalar (dc, npts-1, wdfld) : badval;

    /*
     * Done with the DataChunk
     */
    dc_DestroyDC (dc);

    /*
     * Put the values into the display
     */
    if (ws == badval)
	strcpy (label, "---");
    else
	sprintf (label, "%d", (int)(ws + 0.5));
    XtVaSetValues (Wspd, XtNlabel, label, 0);

    if (ws_min == badval)
	strcpy (label, "---");
    else
	sprintf (label, "%d", (int)(ws_min + 0.5));
    XtVaSetValues (WspdMin, XtNlabel, label, 0);

    if (ws_max == badval)
	strcpy (label, "---");
    else
	sprintf (label, "%d", (int)(ws_max + 0.5));
    XtVaSetValues (WspdMax, XtNlabel, label, 0);

    if (wd == badval)
	strcpy (label, "---");
    else
	sprintf (label, "%03d", (int)wd);
    XtVaSetValues (Wdir, XtNlabel, label, 0);

    /*
     * Set a timer to get us called again in Interval seconds
     */
    XtAppAddTimeOut (Appc, Interval * 1000, (XtTimerCallbackProc) Update, 0);
}




static int
msg_handler (struct message *msg)
{
	struct mh_template *tm = (struct mh_template *) msg->m_data;
/*
 * Just branch out on the message type.
 */
	switch (msg->m_proto)
	{
	/*
	 * Message handler stuff.  The only thing we know how to deal
	 * with now is SHUTDOWN.
	 */
	   case MT_MESSAGE:
	   	if (tm->mh_type == MH_SHUTDOWN)
			exit (0);
		msg_ELog (EF_PROBLEM, "Unknown MESSAGE proto type: %d",
			tm->mh_type);
		break;
	 /*
	  * Data store.
	  */
	    case MT_DATASTORE:
	    	ds_DSMessage (msg);
		break;
	}
	return (0);
}



static void
XZebraHandler (XtPointer client_data, int *source, XtInputId *id)
{
    msg_incoming (*source);
}
