XCOMM !/bin/csh -f
XCOMM
XCOMM Environment variable settings specific to ARM
XCOMM $Id: proj_env.cpp,v 1.5 1997-08-19 21:56:54 granger Exp $

#include "config.h"

echo "Reading proj_env"

setenv HOST `uname -n`
if ( ! $?DISPLAY ) then
	setenv DISPLAY localhost:0
else
	set display=`echo $DISPLAY | sed -e 's/:.*//'`
	set screen=`echo $DISPLAY | sed -e 's/^.*://'`
	if ( x$display == x || x$display == x$HOST ) then
		setenv DISPLAY localhost:$screen
	endif
endif
echo "DISPLAY set to $DISPLAY"

XCOMM Simple configuration settings used by zstart
set dm="dm -single "
if (! $?ZEB_DM) setenv ZEB_DM \
   "xterm -T DisplayManager -bg grey90 -fg purple -fn 8x13bold -e $dm "
if (! $?ZEB_EVENTLOGGER) \
	setenv ZEB_EVENTLOGGER 'EventLogger -w'

setenv ZEBHOME $ZEB_TOPDIR
setenv CONFIGHOME CFG_PROJECTDIR
setenv ZEB_PROJDIR $CONFIGHOME
setenv DATA_DIR CFG_DEFAULTDATADIR

if ( ! $?ZEB_CACHEDIR ) \
	setenv ZEB_CACHEDIR $CONFIGHOME/cache

