/*
 * Quickie hack to get in and store the limits of a radar's scanning.
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

# include <X11/Intrinsic.h>
# include <X11/StringDefs.h>
# include <X11/Shell.h>
# include <X11/Xaw/Form.h>
# include <X11/Xaw/Text.h>
# include <X11/Xaw/Label.h>
# include <X11/Xaw/Command.h>
# include <X11/Xaw/AsciiText.h>

# include "defs.h"
# include "copyright.h"
# include <message.h>
# include <DataStore.h>
# include <dm.h>
MAKE_RCSID ("$Id: RadarLimits.c,v 2.3 1991-12-05 20:34:36 corbet Exp $")


static XtAppContext Appc;
Widget Top, Form, Shell, Left, Right, Last, Wm;
bool Visible = FALSE;
bool Override = TRUE;

char LString[40], RString[40];

DataObject DObj;


# ifdef __STDC__
	static int MsgHandler (Message *);
	static void Store ();
	static int xevent (int);
	static void CreateWidget (char *);
	static void wm ();
	static void CreateDataObject (char *);
# else
	static int MsgHandler ();
	static void Store ();
	static int xevent ();
	static void CreateWidget ();
	static void wm ();
	static void CreateDataObject ();
# endif

main (argc, argv)
int argc;
char **argv;
{
	Arg args[5];
	int n;
	char *platform;
/*
 * Hook in.
 */
	if (! msg_connect (MsgHandler, "RadarLimits"))
		exit (1);
	usy_init ();
	ds_Initialize ();
/*
 * Get set up with the toolkit.
 */
	Top = XtAppInitialize (&Appc, "RadarLimits", NULL, 0, &argc, argv,
		NULL, NULL, 0);
	platform = (argc > 1) ? argv[1] : "cp2";
/*
 * Create our shell.
 */
	n = 0;
	XtSetArg (args[n], XtNinput, True);	n++;
	XtSetArg (args[n], XtNoverrideRedirect, True);	n++;
	Shell = XtCreatePopupShell ("Radar Limits", topLevelShellWidgetClass,
		Top, args, n);
/*
 * Create the rest of the widget.
 */
	CreateWidget (platform);
/*
 * Create the static parts of our data object.
 */
	CreateDataObject (platform);
/*
 * Tell msglib about our X connection.
 */
	msg_add_fd (XConnectionNumber (XtDisplay (Shell)), xevent);
/*
 * Now we just wait.
 */
	reconfig (100, 600, 300, 70);
	sync ();
	xevent (0);
	msg_await ();
}





static void
CreateWidget (platform)
char *platform;
/*
 * Create the widget itself.
 */
{
	char string[80];
	Arg args[15];
	Widget w, label, title;
	int n;
/*
 * Put a form inside it.
 */
	Form = XtCreateManagedWidget ("form", formWidgetClass, Shell, args, 0);
/*
 * Start with a label.
 */
	sprintf (string, "Radar azimuth limits for %s", platform);
	n = 0;
	XtSetArg (args[n], XtNlabel, string);		n++;
	XtSetArg (args[n], XtNfromHoriz, NULL);		n++;
	XtSetArg (args[n], XtNfromVert, NULL);		n++;
	title = XtCreateManagedWidget("title",labelWidgetClass, Form, args, n);
/*
 * The dm/wm button.
 */
	n = 0;
	XtSetArg (args[n], XtNlabel, "Ctl: DM"  );	n++;
	XtSetArg (args[n], XtNfromHoriz, title);	n++;
	XtSetArg (args[n], XtNfromVert, NULL);		n++;
	Wm = XtCreateManagedWidget ("ctl", commandWidgetClass, Form, args, n);
	XtAddCallback (Wm, XtNcallback, wm, 0);
/*
 * Another label for the left limit.
 */
	n = 0;
	XtSetArg (args[n], XtNlabel, "From"  );		n++;
	XtSetArg (args[n], XtNfromHoriz, NULL);		n++;
	XtSetArg (args[n], XtNfromVert, title);		n++;
	label =XtCreateManagedWidget("lLeft", labelWidgetClass, Form, args, n);
/*
 * The text widget for the left limit.
 */
	n = 0;
	XtSetArg (args[n], XtNfromHoriz, label);	n++;
	XtSetArg (args[n], XtNfromVert, title); 		n++;
	XtSetArg (args[n], XtNdisplayPosition, 0); 	n++;
	XtSetArg (args[n], XtNinsertPosition, 0); 	n++;
	XtSetArg (args[n], XtNresize, XawtextResizeNever); n++;
	XtSetArg (args[n], XtNtype, XawAsciiString); 	n++;
	XtSetArg (args[n], XtNuseStringInPlace, True); 	n++;
	XtSetArg (args[n], XtNlength, 40); 		n++;
	XtSetArg (args[n], XtNstring, LString);		n++;
	XtSetArg (args[n], XtNleftMargin, 5);		n++;
	XtSetArg (args[n], XtNeditType, XawtextEdit);	n++;
	Left = XtCreateManagedWidget ("Left", asciiTextWidgetClass, Form,
			args, n);
/*
 * Intervening label.
 */
	n = 0;
	XtSetArg (args[n], XtNlabel, "deg to"  );	n++;
	XtSetArg (args[n], XtNfromHoriz, Left);		n++;
	XtSetArg (args[n], XtNfromVert, title);		n++;
	w = XtCreateManagedWidget ("lRight", labelWidgetClass, Form, args, n);
/*
 * The text widget for the right limit.
 */
	n = 0;
	XtSetArg (args[n], XtNfromHoriz, w);	n++;
	XtSetArg (args[n], XtNfromVert, title);		n++;
	XtSetArg (args[n], XtNdisplayPosition, 0); 	n++;
	XtSetArg (args[n], XtNinsertPosition, 0); 	n++;
	XtSetArg (args[n], XtNresize, XawtextResizeNever); n++;
	XtSetArg (args[n], XtNtype, XawAsciiString); 	n++;
	XtSetArg (args[n], XtNuseStringInPlace, True); 	n++;
	XtSetArg (args[n], XtNlength, 10); 		n++;
	XtSetArg (args[n], XtNstring, RString);		n++;
	XtSetArg (args[n], XtNleftMargin, 5);		n++;
	XtSetArg (args[n], XtNeditType, XawtextEdit);	n++;
	Right = XtCreateManagedWidget ("Right", asciiTextWidgetClass, Form,
			args, n);
/*
 * And something to finish.
 */
	n = 0;
	XtSetArg (args[n], XtNlabel, "deg."  );	n++;
	XtSetArg (args[n], XtNfromHoriz, Right);		n++;
	XtSetArg (args[n], XtNfromVert, title);		n++;
	w = XtCreateManagedWidget ("lEnd", labelWidgetClass, Form, args, n);
/*
 * The store button
 */
	n = 0;
	XtSetArg (args[n], XtNlabel, "Store"  );	n++;
	XtSetArg (args[n], XtNfromHoriz, w);		n++;
	XtSetArg (args[n], XtNfromVert, title);		n++;
	w = XtCreateManagedWidget ("Store", commandWidgetClass, Form, args, n);
	XtAddCallback (w, XtNcallback, Store, 0);
/*
 * A label widget to say when we last stored something.
 */
	n = 0;
	XtSetArg (args[n], XtNlabel, " "  );	n++;
	XtSetArg (args[n], XtNfromHoriz, NULL);		n++;
	XtSetArg (args[n], XtNfromVert, Left);		n++;
	Last = XtCreateManagedWidget ("last", labelWidgetClass, Form, args, n);
}



static int
xevent (fd)
int fd;
/*
 * Deal with an Xt event.
 */
{
	XEvent event;
/*
 * Deal with events as long as they keep coming.
 */
 	while (XtAppPending (Appc))
	{
		XtAppNextEvent (Appc, &event);
		XtDispatchEvent (&event);
	}
	return (0);
}





static int
MsgHandler (msg)
struct message *msg;
/*
 * Log a client event.
 */
{
/*
 * Maybe it's a display manager message.
 */
	if (msg->m_proto == MT_DISPLAYMGR)
		dm_msg (msg->m_data);
/*
 * Everything else is assumed to be a message handler event.
 */	
	else if (msg->m_proto == MT_MESSAGE)
	{
		struct mh_template *tmpl = (struct mh_template *) msg->m_data;
		if (tmpl->mh_type == MH_SHUTDOWN)
			exit (0);
	}
	return (0);
}








sync ()
/*
 * Synchronize with the window system.
 */
{
	XSync (XtDisplay (Top), False);
}





reconfig (x, y, w, h)
int x, y, w, h;
/*
 * Reconfigure the window.
 */
{
	Arg args[5];

	XtSetArg (args[0], XtNx, x);
	XtSetArg (args[1], XtNy, y);
	XtSetArg (args[2], XtNwidth, w);
	XtSetArg (args[3], XtNheight, h);
	XtSetValues (Shell, args, 4);
/* 
 * If they can't see us yet, make it so now.
 */
	if (! Visible)
	{
		XtPopup (Shell, XtGrabNone);
		Visible = TRUE;
	}
	sync ();
}





static void
wm ()
/*
 * Try to change override redirect.
 */
{
	Arg args[2];
/*
 * If the window is up, take it down.
 */
	if (Visible)
		XtPopdown (Shell);
/*
 * Set the parameter.
 */
	Override = ! Override;
	XtSetArg (args[0], XtNoverrideRedirect, Override);
	XtSetValues (Shell, args, 1);
/*
 * Set the label on the command widget too.
 */
	if (Override)
		XtSetArg (args[0], XtNlabel, "Ctl: DM");
	else
		XtSetArg (args[0], XtNlabel, "Ctl: WM");
	XtSetValues (Wm, args, 1);
/*
 * Put the window back if it was before.
 */
	if (Visible)
		XtPopup (Shell, XtGrabNone);
}




dm_msg (dmsg)
struct dm_msg *dmsg;
/*
 * Deal with a DM message.
 */
{
	struct dm_hello reply;
/*
 * See what we got.
 */
	switch (dmsg->dmm_type)
	{
	/*
	 * Maybe it's a DM scoping us out.
	 */
	   case DM_HELLO:
	   	reply.dmm_type = DM_HELLO;
		msg_send ("Displaymgr", MT_DISPLAYMGR, FALSE, &reply, 
			sizeof (reply));
		break;
	/*
	 * Maybe it's a reconfig.
	 */
	   case DM_RECONFIG:
		if (Override)
		   	reconfig (dmsg->dmm_x, dmsg->dmm_y, dmsg->dmm_dx,
				dmsg->dmm_dy);
		break;
	/*
	 * They might want us to go away entirely.
	 */
	   case DM_SUSPEND:
	   	if (Visible)
		{
			Visible = FALSE;
			XtPopdown (Shell);
		}
		break;
	}
}





static void
SetStatus (s)
char *s;
/*
 * Set our status widget.
 */
{
	Arg args[2];

	XtSetArg (args[0], XtNlabel, s);
	XtSetValues (Last, args, 1);
	sync ();
}



static void
Store ()
/*
 * Store the result.
 */
{
	char s[80];
	time t;
/*
 * Decode the values.
 */
	if (! sscanf (LString, "%f", DObj.do_data[0]))
	{
		SetStatus ("Bad LEFT limit");
		return;
	}
	else if (! sscanf (RString, "%f", DObj.do_data[1]))
	{
		SetStatus ("Bad RIGHT limit");
		return;
	}
/*
 * Get the time and store everything.
 */
	tl_GetTime (&t);
	DObj.do_begin = DObj.do_end = DObj.do_times[0] = t;
	ds_PutData (&DObj, FALSE);
/*
 * Time for a message.
 */
	sprintf (s, "Last stored %d:%02d:%02d", t.ds_hhmmss/10000,
		(t.ds_hhmmss/100) % 100, t.ds_hhmmss % 100);
	SetStatus (s);
}





static void
CreateDataObject (platform)
char *platform;
/*
 * Make the data object for this platform.
 */
{
	char pname[80];
/*
 * Create the real platform name and look it up.
 */
	strcpy (pname, platform);
	strcat (pname, "-az-limits");
	if ((DObj.do_id = ds_LookupPlatform (pname)) == BadPlatform)
	{
		msg_ELog (EF_EMERGENCY, "No az lims platform %s", pname);
		exit (1);
	}
/*
 * Start to fill everything in.
 */
	DObj.do_org = OrgScalar;
	DObj.do_npoint = 1;
	DObj.do_nbyte = 8;
	DObj.do_times = ALLOC (time);
	DObj.do_data[0] = (float *) malloc (2*sizeof (float));
	DObj.do_data[1] = DObj.do_data[0] + 1;
	DObj.do_nfield = 2;
	DObj.do_fields[0] = "left";
	DObj.do_fields[1] = "right";
	DObj.do_flags = 0;
	DObj.do_badval = -9999.0;
}
