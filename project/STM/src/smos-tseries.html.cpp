#undef UAV
<title>SMOS Time Series</title>
<h1>SMOS Time Series</h1>

<p>The left window is a <a href=HELPDIR/horizontal-plot.html>horizontal
plot</a> showing a map of state boundaries, facility locations, and a
station plot of the SMOS network.  All of the facility locations and
stations are <em>active areas</em>, meaning they are active input areas
within the plot itself.  Active areas are highlighted with a red border
when the cursor passes over them.  Press one of the mouse buttons to pop up
a menu while the area is highlighted.  The middle mouse button of an active
area usually brings up menu items which allow a specific platform to be
added to another window, as if it were chosen from one of the IconBar
menus.<p>

<p>The right window contains a two components, the first of which is the
<em>global component</em>.  The second component corresponds to the four
line traces being plotted, the same field for all four SMOS sites: E13
(CF), E20, E15, and E9.  The left axis scale corresponds to all four
traces, while the bottom axis is time.  The side annotation labels each of
the traces, showing the time of the trace, its color, and its
representation.  In this case all of the representations are lines, but
they can also be Xs, crosses, points, or circles.</P>

<p>Each trace has an active area in its side annotation.  The left menu
removes the particular platform from the trace.  The middle menu changes
the field being plotted, and the right menu changes the color of the trace.
</p>

<h2>Examples</h2>

<p>Here are a few steps to try to see how the horizontal and time series
windows can interact.  First, remove a single trace from the time series
plot using the left menu of the active area in the trace annotation.
Next, remove all of the traces by choosing the <em>Remove</em> option from the
left menu of the component icon in the lower left corner of the window.
The window is now empty.</p>

<p> In the horizontal plot, move the cursor over the active area of the E9
facility furthest north.  There are actually two active areas here.  The
larger is for the entire facility, the smaller is for a single SMOS
instrument platform.  The SMOS menu has a single option, <em>Display
station in other window</em>, since there is only one possible platform
(sgp30smosE9.a1).  The facility menu contains entries for all of the
platforms at that location for which data is available.  On the facility
menu, the SMOS platform is the option <em>30-minute (a1)</em> under the
SMOS platforms submenu.  Select the SMOS entry from one of these menus.
Once the cursor becomes a cross-hair, press the left mouse button on top of
the time series window.  You have just added a trace to the time series
window using the default field.  To choose a different field, use the
middle menu of either the annotation active area or the second component
icon.  Additional platforms can be added to the time series by selecting
SMOS platforms from the other facility locations in the horizontal plot.
</p>
		
<p>It is also possible to add platforms to the horizontal plot for
comparison with the SMOS.  Choose the NWS mesonet station plot from the
<em>Surface</em> icon of the <a href=arm-iconbar.html>iconbar</a> and place
it in the horizontal plot.  The NWS stations will appear on the plot as
small crosses, just like the SMOS stations, including wind vectors for
those winds which are not bad values.  Use the middle menu of the NWS
station directly north of the E9 SMOS to send the NWS station's trace to
the time series window.  Then choose the ambient temperature field from the
time series component icon.  Two traces should now appear in the time
series window, one for the SMOS instrument and one for the NWS station.
Unfortunately, only fields with similar field names between the two
platforms can be inter-compared this way.  For fields with different names,
a second time series window could be used, one for each type of
platform.</p>

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html> Zebra Help Index</i></a>
</ul>
<h6>$Id: smos-tseries.html.cpp,v 1.2 1995-09-28 13:56:33 granger Exp $</h6>
