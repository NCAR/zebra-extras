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


#ifdef notdef
==========================================================================
Platform menus: Chooses a platform from a menu to add to an existing
graphics window.  After selecting a platform entry, the cursor turns into a
cross-hair and waits for a graphics window to be selected by pressing the
mouse button over that window.  The chosen platform will be added to that
window's plot in a representation appropriate for the platform and window.
For example, if the chosen platform is a sonde, it will be represented by a
track in a constant-altitude plot, by a skew-t plot in a skew-t window, or
as a line trace in a time-series window.

==========================================================================
Overlays menu: The overlays menu lists "static overlays", fixed-position
information which can be overlaid in a constant-altitude plot, such as
facility locations, maps, roads, radar range rings, grids, and so on.
Adding an overlay to a window is similar to adding a platform, except the
overlay does not correspond to an actual data platform, nor does it have
any representation except on a constant-altitude plot.

==========================================================================
Configs menu: Changes the current display to one of the "pre-supplied" or
saved configurations.


#endif

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html> Zebra Help Index</i></a>
</ul>
<h6>$Id: arm-iconbar.html.cpp,v 1.1 1995-09-22 00:16:23 granger Exp $</h6>
