#undef UAV
<title>Satellite Display Configuration</title>
<h1>Satellite Display Configuration</h1>

<p> The satellite display consists of a single constant-altitude plot
(<em>cap</em> or <em><a href=HELPDIR/horizontal-plots.html>horizontal
plot</a></em>) window showing one of the GOES-8 channels at 4-km
resolution.  

<h2>Adding Static Overlays</h2>

<p>Any of the static overlays, such as facility locations or the Oklahoma
road map can be overlaid onto a horizontal plot.  The static overlays can
be chosen from the <a href=arm-iconbar.html>icon bar</a> overlays icon
<img src=HELPDIR/iconbar-over.gif>.

<p>To choose the AVHRR satellite instead of GOES-8, remove the satellite
component from the graphics window.  Press the left button over the
satellite icon in the graphics window, and select the <em>Remove</em>
entry.  Choose one of the AVHRR platforms from the <em>IconBar</em>
satellite menu, then press the mouse button over the graphics window once
the cursor has turned into a cross-hair.  Change the field being shown in
the satellite image by selecting a new field from the field menu under the
satellite icon in the graphics window.</p>
		
<h2>Satellite Icon Menus</h2>

<p>The satellite icon has three menus, one for each mouse button.  The
right-button menu changes characteristics of the satellite image, such as
the type of representation: raster, contour, or filled contour.  It also
contains a submenu of the available color maps and an option to highlight a
particular range of values with a special color.</P>
		
<p>The middle menu lists the fields available for that platform.  Choosing
a field name changes the field which is being shown in the satellite image.
As the field changes, the color table will optimize its center and step to
the range of data values, unless the center and step have already been
given fixed values for the field.  The <em>adjust limits</em> entry in the
left menu pops up a widget which allows the center and step to be fixed or
to be calculated automatically.</P>

<h2>The Global Icon</h2>

<p>The leftmost icon in the graphics window <img src=HELPDIR/zeb-icon.gif>
is a special icon called the <em>global icon</em>.  It represents the
<em>global component</em> of the window; every graphics window has a global
component.  The menus of this icon affect the general characteristics of
the window, those not related to particular platforms.  The left-button
menu calls up tools such as the movie controller, the overlay times widget,
the position locator, and the image generator.  Submenus can expand or
narrow the view of the window, change the plot coverage to a specific
region, or reset the window to its original plot.  The right menu controls
the size and color of the top annotation.  The middle menu controls <a
href=HELPDIR/layout.html>layout parameters</a>.

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html> Zebra Help Index</i></a>
</ul>
<h6>$Id: satellite.html.cpp,v 1.3 1997-04-10 17:29:20 granger Exp $</h6>
