#undef UAV
<title>The ARM Icon Bar </title>
<h1>The ARM Icon Bar </h1>

<p> This help topic talks about the specific functions available from the
icon bar in the ARM project configuration.  If you are not familiar with
the general purpose of the icon bar, you can click <a
href=HELPDIR/iconbar.html> here </a> for more information or choose a topic
from the iconbar help menu. <img src=HELPDIR/iconbar-help.gif> </p>

<p> For all of the iconbar icons, there is only one menu attached to each
icon.  All three mouse buttons call up the same menu on one icon.  This is
different from the icon menus in the individual graphics windows, since
those icons usually have unique menus for each mouse button.  However,
everything else about the iconbar and window menus is similar.  Menu
entries followed by an ellipsis call up a widget to query for further
information.  An arrow in the left column of an entry indicates a submenu,
which can be opened by dragging the mouse to the right half of the entry.
Entries with neither an ellipsis nor an arrow take action immediately.  In
some menus the left column of an entry may contain a star, meaning that
entry has been enabled or is currently active.</P>

<h2>The Icon Bar menus</h2>

<h3><img src=HELPDIR/iconbar-help.gif>Help</h3>

<p> This menu lists help topics which start the on-line help system on a
particular topic.  If the Web browser (Mosaic or Netscape) has already been
started by a previous help topic, then choosing a new topic causes the
existing browser to immediately visit the new topic page.  If the browser
has not been started yet it will be started automatically once the first
help topic is picked.</P>

<p> If you are looking for help on a particular topic which cannot be found
in the help menu, select the <a href=HELPDIR/index.html> help index </a> to
see if the topic is listed there.</P>

<p> For the ARM project, the last few entries in the help menu point to WWW
servers of particular interest to ARM project participants: the ARM home
page, ARM datasets information, and the ARM archive home page.</p>

<h3><img src=HELPDIR/tools-icon.gif><a href=tools.html>Tools</a></h3>

<p>The <a href=tools.html>tools</a> menu contains entries for popup widgets and
external applications which assist in managing data, controlling the time
and operation of the Zebra session, and creating and modifying display
configurations.</p>


<h3><img src=HELPDIR/configs-icon.gif>The Configs Menu</h3>

<p>The configs menu changes the current display to one of several standard
display configurations.  Drag down the menu and select one of the displays,
and that display will become the new current configuration.  The current
configuration, if among those in the menu, will have a star to the left of
the menu entry.  Switch to the <a href=empty.html>empty</a> configuration
to clear the screen of windows and leave only the icon bar displayed.
Other configurations are described in the <a href=arm-displays.html>ARM
display index</a>.</p>

<h3><img src=HELPDIR/iconbar-sat.gif>Platform Menus...</h3>

<p>Each of the platform icons contains a menu of platforms for a particular
category.  For example, all of the satellite platforms can be found under
the satellite icon.  Each menu entry selects a platform to add to an
existing graphics window.  After selecting a platform entry, the cursor
turns into a cross-hair, indicating that a graphics window needs to be
chosen to which to add the platform.  Select the graphics window by
pressing the mouse button over the desired window.  The platform will be
added to that window's plot in a representation appropriate for the
platform and window.  For example, if the chosen platform is a sonde, it
will be represented by a track in a constant-altitude plot, by a skew-t
plot in a skew-t window, or as a line trace in a time-series window.  For
the skew-t plots, the only acceptable platforms are sondes.  For a sonde
winds or temperatures profile, the chosen platform overrides the existing
platform, and the plot will begin displaying data from the chosen platform.
</p>

<p>Almost any platform can be added to a time series plot, including
aircrafts, instruments, soundings, and profilers.  The time series will try
to plot a default field, but usually the field will have to be chosen from
the field menu once the platform has been added to the plot.</p>


<h3><img src=HELPDIR/iconbar-over.gif>The Overlays Menu</h3>

<p>The overlays menu lists <em>static overlays</em>, fixed-position
information which can be overlaid in a <a
href=HELPDIR/horizontal-plots.html>constant-altitude plot</a>, including
facility locations, maps, roads, range rings, grids, and so on.  Adding an
overlay to a window is similar to adding a platform, except the overlay
does not correspond to an actual data platform, nor does it have any
representation except on a constant-altitude plot.</P>

<p>The following overlays are available in the ARM project
configuration.</p>

<dl>

<dt>State lines <dd>State boundaries for the entire U.S.

<dt>Cities <dd>Locations of cities labelled by name

<dt>Roads <dd>Roads and interstates for a region centered on the
Oklahoma-Kansas border.

<dt>Road labels <dd>Labels for some of the roads in the immediate vicinity
of Blackwell-Tonkawa airport.

<dt>Rivers <dd>Rivers, irrigation, creeks, and drainage systems for the
same area covered by the roads overlay.

<dt>Railroads <dd>Railroad tracks for the OK-KS area.

<dt>Central Facility <dd>Active location for the central and E13 factilities.

<dt>Extended Facilities <dd>Active locations for all of the operational
extended facilities.

<dt>Boundary Facilities <dd>Active locations for boundary facilities B1, B4,
B5, and B6.

<dt>Radar range rings<dd>Range rings for one of three origins in the
Oklahoma reigon: OKC, FDR, and CIM.

<dt>X-Y Grid <dd>Horizontal and vertical grid lines or crosses for
the kilometer grid in the projection coordinate space.

<dt>Lat/Lon Grid <dd>Lines of latitude and longitude with variable spacing.

<dt>Window Bounds <dd>Add the bounds of one window, either a <a
href=HELPDIR/horizontal-plots.html>horizontal plot</a> or <a
href=HELPDIR/cross-section.html>cross-section</a>, to a horizontal plot.
Select the source plot with the first crosshair cursor, and the destination
plot, which will show the boundaries of the source plot , with the second
crosshair.

</dl>

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html> Zebra Help Index</i></a>
</ul>
<h6>$Id: arm-iconbar.html.cpp,v 1.3 1997-04-10 17:29:19 granger Exp $</h6>
