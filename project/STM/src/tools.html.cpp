#undef UAV
<title>The Tools Menu</title>
<h1>ARM Iconbar Tools Menu</h1>

<img src=HELPDIR/tools-icon.gif>

<p>The tools menu provides access to a variety of tools, widgets, utility
programs, and functions; each menu entry is described below.</p>

<dl>
<dt><b>Time Control Tool</b>

<dd>The <a href=HELPDIR/historytime.html> time widget </a> changes the time
of one or all windows, and switches between realtime and history modes.

<dt><b>Print Tool</b>

<dd>The <a href=HELPDIR/printer.html> print widget </a> allows one window
or the whole screen to be dumped to a file or to a printer, in color, mono
PostScript, or any of several other printer formats.

<dt><b>Configuration Tool</b>

<dd>The <a href=HELPDIR/editing-configs.html> configuration tool </a>
contains access to all of the usual configuration functions: <em>New
Configuration</em>, <em>New Window</em>, <em>Delete Window</em>, and
<em>Save Configuration</em>.  This widget can be bypassed by choosing
configuration functions directly from the tools menu (below).

<dt><b>XTerm Shell Tool</b>

<dd>Starts an xterm window and C-shell with the Zebra run-time environment.
This means important environment variables such as <tt>ZEB_TOPDIR</tt>, the
path, the X resource directory, and <tt>ZEB_SOCKET</tt> will be correctly
set to run Zebra clients with the current Zebra session.

<dt><b>Data Listings</b>

<dd>A menu of platform categories which can be listed by the <a
href=HELPDIR/dsdwidget.html> data store widget </a> (program
<tt>dsdwidget</tt>).  Each menu entry calls up an alphabetical listing of
the platforms in the given category and the data available in each.  For
the full ARM dataset, it is helpful to be able to limit the listings to a
particular category of interest.  However, for the real-time UAV programs
there are few enough platforms that the <em>All Platforms</em> category will
usually be sufficient.

<dt><b>Data Store Management</b>

<dd> Start the <a href=HELPDIR/dsmanage.html> data store manager </a>
(program <tt>dsmanage</tt>).  This tool manages the task of moving data
files to and from the disk (the on-line data store).  It uses indexes of
archive tapes to know what times and platforms are available on each tape.
A graphical interface allows data files to be easily selected by times and
pulled off of the tape, while another interface selects data files for
removal to free disk space.

<dt><b>Begin new configuration...</b>

<dd>Call up a widget which creates a new configuration by copying an
existing one (by default the current configuration).

<dt><b>Add new window</b>

<dd>This is a submenu of templates for windows which can be created
on-the-fly.  The chosen window appears with a preset size and location near
the middle of the screen.  From there the window can be moved and resized
as desired.  Many of the templates include particular platforms; those that
do not, such as the <em>Time series</em> and <em>Constant altitude</em>
entries, are generic window types to which platforms can be added as
needed.

<dt><b>Kill window by cursor</b>

<dd>Select a graphics window for deletion.  When the cursor turns into a
crosshair, press the left mouse button over a graphics window to kill that
window and delete it from the configuration.  The kill can be aborted by
clicking on the root window or a window which is not a graphics process.
This menu item is a convenient, direct access to the configuration <a
href=HELPDIR/ec-delete.html> kill window </a> function, and as such is more
dangerous.  Use with caution.

<dt><b>Add window named...</b>

<dd>Brings up the <a href=HELPDIR/ec-add.html> add window </a> widget
without going through the configuration tool.  The name of the new window
can be chosen and entered into the widget, as opposed to letting the window
template menu automatically choose a name.

<dt><b>Kill window named...</b>

<dd>Bring up the <a href=HELPDIR/ec-delete.html> kill window widget </a>
from the menu instead of through the configuration tool.  This is a safer
method of killing a window, since the window name can be typed or selected
with a cursor, and actually killing the named window requires an extra
button push.

<dt><b>Save this configuration</b>

<dd>Save the current configuration in the default save directory
(./dconfig) under its current name.  Essentially this option immediately
updates the configuration file on the disk with what is currently displayed
on the screen.

<dt><b>Save this configuration as...</b>

<dd>Calls up the <a href=HELPDIR/ec-save.html> save configuration widget
</a>.  Using the save widget allows the current configuration to be saved
under a different name, at which point the new name becomes the current
configuration.

<dt><b>Restore to original</b>

<dd>Restores the current configuration to its original layout.  This does
not restore the plots within the individual windows.  Plot windows can be
restored with the <em>Reset window</em> option under the <em>Window
operations</em> submenu of the global icon.<img src=HELPDIR/zeb-icon.gif>

</dl>

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html>Zebra Help Index</i></a>
</ul>
<h6>$Id: tools.html.cpp,v 1.1 1995-09-22 00:16:29 granger Exp $</h6>

