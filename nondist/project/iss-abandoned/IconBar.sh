#! /bin/sh
# $Id: IconBar.sh,v 1.2 2000-06-07 23:04:35 granger Exp $
#
# This is not UI code but a shell script which generates UI code
# for an IconBar and writes it to stdout.  It is up to dm.config
# to run this script (thereby passing its environment variables
# as well) and 'read' the output.
#
# Many of the menus are boilerplate, so those are easy to generate...

# Generate a list of platforms from the datastore daemon
platforms=/tmp/ds.iconbar
dsdump -a -n | grep Platform | awk '{ print $2; }' > $platforms

# First the opening
cat <<EOF

require "cfg-tools"
require "template-menu"

define widget dslistings intmenu 'dsdwidget listings'
	title 'Platform Subsets'
	line
	entry 'All Platforms' \
	   'shell "dsdwidget -a -t All\ Platforms &"'
	entry 'Profiler Platforms' \
	   'shell "dsdwidget -a -t Profiler\ Platforms prof915 &"'
	entry 'Sounding Platforms' \
	   'shell "dsdwidget -a -t Sounding\ Platforms class &"'
	entry 'Surface Platforms' \
	   'shell "dsdwidget -a -t Surface\ Platforms surf &"'
endmenu
EOF

# Build the list of display configs from the templates for the known platforms
sites=`cat $platforms | sed -e 's,/[^/]*$,,' | egrep 'iss.$' | uniq`
configs=`(cd dconfig/template; \
          tmake ../../tmp/configs '' iss-batch $sites)`

# Echo command to read those configs into dm
echo read \'tmp/configs\'

# We can't delete the files from here because dm has not yet processed
# this file, obviously...

cat <<EOF
!
! Project iconbar
!
define widget iconbar menubar "iconbar"
	noheader
EOF

# Next the Help Menu
cat <<HELP
	menu help bitmap iconbar-help
		title 'Zebra Help Access'
		line
		entry 'Introduction' 'help intro'
		entry 'On-line Help Index' 'help index'
		entry 'Starting Zebra' 'help start'
		entry 'Shutting Down' 'help shut-down'
		entry 'Keys and Mouse Buttons' 'help keys'
		entry 'The Icon Bar' 'help iconbar'
		entry 'Editing Display Configurations' 'help editing-configs'
		line
		entry 'Horizontal plots' 'help horizontal-plots'
		entry 'Skew-t plots' 'help skewt'
		entry 'XYGraph plots' 'help xy-graphs'
		entry 'Cross section plots' 'help cross-section'
	endmenu
HELP


# Tools menu
#
# Limit the available tools in 'operator' mode
#
cat <<TOOLS
	menu tools bitmap tools-big
		title 'Tools'
		line
		entry 'Time Control Tool...' 'popup time'
		entry 'List All Platforms' \
		   'shell "dsdwidget -a -t All\ Platforms &"'
TOOLS

if [ ${OPERATOR_MODE:-0} = 0 ]; then
cat <<'TOOLS'
		submenu 'List Platforms by Categories' dslistings
		entry 'Printing Tool...' 'popup hardcopy'
		entry 'Configuration Tool...' 'popup ConfigEdit'
		entry 'XTerm Shell Tool' 'shell "xterm -T ZebraShell -bg grey90 -fg purple -fn 8x13bold -e zstart -shell &"'
		entry 'Data Store Management' 'shell "dsmanage &"'
		line
		entry 'Begin new configuration...' 'popup newconfig'
		submenu 'Add new window' 'template-menu'
		entry 'Kill window by cursor' 'point-and-shoot'
		entry 'Add window named...' 'popup NewWindow'
		entry 'Kill window named...' 'popup ZorchWindow'
		line
		entry 'Save this configuration' 'cfgsave #dm$config'
		entry 'Save this configuration as...' 'popup SaveConfig'
		entry 'Restore configuration' 'cfg-restore'
TOOLS
fi

cat <<TOOLS
		line
		entry 'Event Logger' 'shell "EventLogger -w &"'
		line
		entry 'Shutdown Zebra' 'shutdown'
	endmenu
TOOLS


cat <<'CONFIGS'
	menu configs bitmap configs
		title 'Display configurations'
		line
CONFIGS


for c in $configs END ; do
    if [ $c != END ] ; then
	echo entry \'$c\' \'display $c\' \('dm$config' = \'$c\'\)
    fi
done

cat <<'CONFIGS'
		line
		entry 'Empty screen' 'display empty' (dm$config = 'empty')
	endmenu
CONFIGS

# Echo entries for each surface platform we have
cat <<SURF
	menu surface bitmap surface
	     	title 'Surface observations'
		line
SURF
#		entry 'ISS Surface Stations' \
#      'putc3 p_station platform #isssurf active-icon false icon iss-iss-small'
#		line

	grep surf $platforms | while read plat; do \
		echo entry \'$plat\' \'PutScalar $plat \"[T]\"\' 
	done
	echo "	endmenu"

# Echo entries for class platforms
cat <<CLASS
	menu sounding bitmap sounding
		title 'Soundings'
		line
CLASS
	grep class $platforms | while read plat; do
	    echo entry \'$plat\' \'PutScalar $plat \"[T]\"\'
	done
	echo "	endmenu"


cat <<RASS
	menu rass bitmap iss-rass
		title 'RASS'
		line
RASS
	grep rass $platforms | while read plat; do
		echo entry \'$plat\' \'PutScalar $plat tv\'
	done
	echo "  endmenu"

cat <<PROF
	menu prof bitmap iss-prof
		title 'Profilers (High Mode)'
		line
PROF
	grep prof915h $platforms | while read plat; do
		echo entry \'$plat\' \'PutScalar $plat wind\'
	done
	echo 'line'
	echo 'entry "Profilers (Low Mode)" beep'
	echo 'line'

	grep prof915l $platforms | while read plat; do
		echo entry \'$plat\' \'PutScalar $plat wind\'
	done
	echo 'endmenu'

cat <<OVERLAYS
	menu 'overlays' bitmap overlays
		title 'Static Overlays'
		line
		entry 'US Map' \
		      'putc3 p_map platform us color brown icon map'
		entry 'World Map' \
		      'putc3 p_map platform world color brown icon map'
		entry 'ISS Stations' 'SendLocs station'
		entry 'Profiler locations' 'SendLocs profiler'
		entry 'RASS locations' 'SendLocs rass'
		entry 'Sounding locations' 'SendLocs sounding'
		line
		entry 'X-Y Grid (km)' \
			'putc3 p_kmgrid x-spacing 300 y-spacing 300 color cyan'
		entry 'Lat/Lon Grid' \
			'putc3 p_llgrid x-spacing 300 y-spacing 300 color cyan'
	endmenu
OVERLAYS
echo 'enddef'

#
# Location variables for long stuff.  DO NOT PUT SPACES IN THESE STRINGS or
# unpleasant things will happen to you.
#

listplats()
{
    plist=""
    comma=""
    egrep $1 $2 | while read p ; do
	plist="$plist$comma$p"
	echo $plist
	comma="," ; done | tail -1
}

echo set isssurf \'`listplats surface_met $platforms`\'

# Soundings are one of those cases where it would be nice to be able
# to dump by platform class, since there is not necessarily anything
# discriminating in the platform name.
#
echo set sndlocs \'`listplats class $platforms`\'

echo set rasslocs \'`listplats rass $platforms`\'

# Profilers.  Don't bother with both high and low, since they tend to have
# similar locations.
#
echo set proflocs \'`listplats prof915l $platforms`\'

cat <<EOF
!
! Add an iss location overlay if one isn't already there.
!
procedure SendLocs type string
	if (type = 'sounding')
		parameter p_active_loc p_active_loc location-time observation
		putc3 p_active_loc platform #SndLocs icon sounding-small \
			time-label true
	elseif (type = 'profiler')
		putc2 p_active_loc platform #ProfLocs icon prof-sm
	elseif (type = 'rass')
		putc2 p_active_loc platform #RASSLocs icon rass-sm
	elseif (type = 'station')
	        putc2 p_station platform #isssurf icon iss-iss-small
	endif
	parameter p_active_loc p_active_loc location-time point
!
! Now we have to drop in an ISS overlay too, if one doesn't already 
! exist.
!
	if (target_win = "none")
		return
	endif
	parameter #target_win global plot-hold true
	local comps pd_complist(target_win)
	foreach comp #comps
	!
	! Key on the iss-loc icon, which is kludgy but easy.
	!
		if (PDParam(target_win,comp,'icon') = 'iss-loc')
			parameter #target_win global plot-hold false
			return
		endif
	endfor
	sendout p_iss_loc #target_win
	parameter #target_win global plot-hold false
endprocedure


EOF

# rm -f $platforms
