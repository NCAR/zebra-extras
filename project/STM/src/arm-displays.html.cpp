#undef UAV
<title>ARM Display Configurations</title>
<h1>ARM Display Configurations</h1>

<p>This help topic lists some of the standard displays avaiable with this
project configuration.  A brief overview of each is given here.  For
details and examples of using the display configuration, click on the name
of the configuration in the left column. </p>

<dl>

<dt><a href=satellite.html>Satellite</a>

<dd>A single constant-altitude plot (cap) window showing one of the GOES-8
channels at 4-km resolution.  As this is a <a
href=HELPDIR/horizontal-plots.html>horizontal plot</a>, any of the static
overlays, such as facility locations or the Oklahoma road map, can be
overlaid in the window.  To choose the AVHRR satellite instead of GOES-8,
<a href=HELPDIR/graphics-icons.html>remove</a> the satellite component and
choose a different platform from the <a href=arm-iconbar.html>IconBar</a>
satellite menu.
		
<dt><a href=smos-tseries.html>SMOS Time series</a>

<dd>An example of time series plots using a simple dataset like SMOS.  The
left window is a <a href=HELPDIR/horizontal-plots.html>horizontal plot</a>
showing a map of state boundaries, facility locations, and a station plot
of the SMOS network.  All of the facility locations and stations are
<em>active areas</em>, meaning they are active input areas within the plot
itself.  Active areas are highlighted with a red border when the cursor
passes over them.  Press one of the mouse buttons to pop up a menu while
the area is highlighted.  The middle mouse button of an active area usually
brings up menu items which allow a specific platform to be added to another
window, as if it were chosen from one of the IconBar menus.


<dt><a href=sondes.html>Sonde skew-t plots</a>

<dd>This display comprises one small, <a
href=HELPDIR/horizontal-plots.html>horizontal plot</a> window and two <a
href=HELPDIR/skewt.html>skew-t</a> windows.  The horizontal plot is
sometimes called the "map" window since it basically serves as a location
reference for the sonde platforms being plotted in the skew-t windows.

</dl>

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html> Zebra Help Index</i></a>
</ul>
<h6>$Id: arm-displays.html.cpp,v 1.1 1995-09-22 00:16:21 granger Exp $</h6>
