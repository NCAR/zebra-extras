#undef UAV
<title>ARM Display Configurations</title>
<h1>ARM Display Configurations</h1>

SMOS Time series

		An example of a time series plots on a simple dataset like
		SMOS.  The left window is a cap plot showing a map of state
		boundaries, facility locations, and a station plot of the
		SMOS network.  All of the facility locations and stations
		are "active areas", meaning they are active input areas
		within the plot itself.  Active areas are highlighted with
		a red border when the cursor passes over them.  Press one
		of the mouse buttons to pop up a menu while the area is
		highlighted.  The middle mouse button of an active area
		usually brings up menu items which allow a specific
		platform to be added to another window, as if it were
		chosen from one of the IconBar menus.

		The right window contains a two components, the first of
		which is of course the global component.  The second
		component corresponds to the four line traces being
		plotted, the same field for all four SMOS sites: E13 (CF),
		E20, E15, and E9.  The left axis scale corresponds to all
		four traces, while the bottom axis is time.  The side
		annotation labels each of the traces, showing the time of
		the trace, its color, and its representation.  In this case
		all of the representations are lines, but they can also be
		X's, crosses, points, or circles.  

		Each trace has an active area in its side annotation.  The
		left menu removes the particular platform from the trace.
		The middle menu changes the field being plotted, and the
		right menu changes the color of the trace.

		Here are a few steps to try to see how the cap and time
		series windows can interact.  First remove a single trace
		from the plot using the left menu of the active area in the
		trace's annotation.  Next, remove all of the traces by
		choosing the "remove" option from the left menu of the
		component icon in the lower left corner of the window.  The
		window is now empty.

		In the horizontal plot (cap window), move the cursor over
		the active area of the E9 facility furthest north.  There
		are actually two active areas here.  The larger is for the
		entire facility, the smaller is for a single SMOS platform.
		The SMOS menu has a single option, "Display station in
		other window", since there is only one possible platform
		(sgp30smosE9.a1).  The facility menu contains entries for
		all of the platforms at that location for which data is
		available.  On the facility menu, the SMOS platform is the
		option "30-minute (a1)" under the SMOS platforms submenu.
		Select the SMOS entry from one of these menus.  Once the
		cursor becomes a cross-hair, press the left mouse button on
		top of the time series window.  You have just added a trace
		to the time series window using the default field.  To
		choose a different field, use the middle menu of either the
		annotation active area or the second component icon.
		Additional platforms can be added to the time series by
		selecting SMOS platforms from the other facility locations
		in the horizontal plot.
		
		It is also possible to add platforms to the horizontal plot
		for comparison with the SMOS.  Choose the NWS mesonet
		station plot from the "Surface" icon of the Iconbar and
		place it in the cap window.  The NWS stations will appear
		on the plot as small crosses, just like the SMOS stations,
		including wind vectors if the winds were not bad values.
		Use the middle menu of the NWS station directly north of
		the E9 SMOS to send the NWS station's trace to the time
		series window.  Choose the ambient temperature field from
		the component icon.  Two traces should now appear in the
		time series window, one for the SMOS instrument and one for
		the NWS station.  Unfortunately, only fields with similar
		names between the two platforms can be inter-compared this
		way.  For fields with different names, a second time series
		window could be used, one for each type of platform.

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html> Zebra Help Index</i></a>
</ul>
<h6>$Id: smos-tseries.html.cpp,v 1.1 1995-09-22 00:25:51 granger Exp $</h6>
