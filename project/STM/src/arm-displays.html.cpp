#undef UAV
<title>ARM Display Configurations</title>
<h1>ARM Display Configurations</h1>

<p>This help topic lists some of the standard displays available with this
project.  A brief overview of each display is given here.  For details and
examples of using the display configuration, click on the name of the
configuration in the left column. </p>

<dl>

<dt><a href=satellite.html>Satellite</a>

<dd>A single constant-altitude plot (cap) window showing one of the GOES-8
channels at 4-km resolution.  As this is a <a
href=HELPDIR/horizontal-plots.html>horizontal plot</a>, any of the static
overlays, such as facility locations or the Oklahoma road map, can be
overlaid in the window.</p>

<dt><a href=smos-tseries.html>SMOS Time series</a>

<dd>Examples of time series plots using a simple dataset like SMOS.  The
left window is a <a href=HELPDIR/horizontal-plots.html>horizontal plot</a>
showing a map of state boundaries, facility locations, and a station plot
of the SMOS network.</p>

<dt><a href=sondes.html>Sonde skew-t plots</a>

<dd>This display comprises one small, <a
href=HELPDIR/horizontal-plots.html>horizontal plot</a> window and two <a
href=HELPDIR/skewt.html>skew-t</a> windows.  The horizontal plot is
sometimes called the "map" window since it basically serves as a location
reference for the sonde platforms being plotted in the skew-t windows.
Sounding sites can be selected from the map window or the <a
href=arm-iconbar.html>icon bar</a> and added to the skew-t plots.  If
sounding platforms are added to a horizontal plot, they appear as <a
href=HELPDIR/hp-aircraft.html>tracks</a>, similar to an aircraft plot.</p>

<dt>Sonde profiles

<dd>A different way of looking at sounding data.  The left window uses a
time-height <a href=HELPDIR/xy-graphcs.html>xy-graph</a> to plot wind
profiles versus height.  A background component plots a contour of a scalar
field, such as temperature or dewpoint.  The contour can be disabled or
filled, the contour field can be changed, and the contour intervals can be
adjusted using the menus attached to the contour icon.  The winds can
plotted as barbs or vectors, color-coded or monochrome, and plotted at a
specific interval using the menus under the wind vector icon.</p>

<p>The right window is a time-height profile of temperature using a
<em>pigtails</em> style of plot.  The temperature axis is fixed to the
first (lowest) point of the profile, and the axis scale is chosen
automatically to fit the profile on the plot.  A scale legend is shown in
the annotation space on the right side.  The field can be changed using the
middle menu of the annotation active area, which is located at the curve
representation (usually a line) in the upper left of the trace
annotation.</p>

<dt><a href=empty.html>Empty screen</a>

<dd>The empty display is useful for temporarily clearing space on the
screen while still leaving the main icon bar accessible.  It is also useful
as a starting place for new configurations.</p>

</dl>

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html> Zebra Help Index</i></a>
</ul>
<h6>$Id: arm-displays.html.cpp,v 1.3 1997-04-10 17:29:18 granger Exp $</h6>
