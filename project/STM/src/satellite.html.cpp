#undef UAV
<title>ARM Display Configurations</title>
<h1>ARM Display Configurations</h1>


Satellite	A single constant-altitude plot (cap) window showing
		one of the GOES-8 channels at 4-km resolution.  As this
		is a cap window, any of the static overlays, such as
		facility locations or the Oklahoma road map can be overlaid
		in the window.  To choose the AVHRR satellite instead
		of GOES-8, remove the satellite component and choose
		a different platform from the IconBar satellite menu.
		
		The satellite icon has three menus, one for each mouse
		button.  The right-button menu changes characteristics of
		the satellite image, such as the type of representation:
		raster, contour, or filled contour.  It also contains a
		submenu of the available color maps and an option to
		highlight a particular range of values with a special
		color.
		
		The middle menu lists the fields available for that
		platform.  Choosing a field name changes the field which is
		being shown in the satellite image.  As the field changes,
		the color table will optimize its center and step to the
		range of data values, unless the center and step have
		already been given fixed values for the field.  The "adjust
		limits" entry in the left menu pops up a widget which
		allows the center and step to be fixed or to be calculated
		automatically.
		
		The leftmost icon in the window, with the letter "Z", is a
		special icon called the "global icon".  It represents the
		global component of the window; every graphics window has a
		global component.  The menus of this icon affect the
		general characteristics of the window, those not related to
		particular platforms.  The left-button menu calls up tools
		such as the movie controller, the overlay times widget, the
		position locator, and the image generator.  Submenus can
		expand or narrow the view of the window, change the plot
		coverage to a specific region, or reset the window to its
		original plot.  The right menu controls the size and color
		of the top annotation.

<hr>
<ul>
<li><a href=arm-tutorial.html>ARM Configuration Tutorial</a>
<li><a href=arm-index.html>ARM Help Index</a>
<li><i><a href=HELPDIR/index.html> Zebra Help Index</i></a>
</ul>
<h6>$Id: satellite.html.cpp,v 1.1 1995-09-22 00:25:49 granger Exp $</h6>
