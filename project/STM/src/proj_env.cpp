XCOMM !/bin/csh -f
XCOMM
XCOMM Environment variable settings specific to ARM
XCOMM $Id: proj_env.cpp,v 1.3 1997-06-02 22:55:31 granger Exp $

#include "config.h"

echo "Reading proj_env"

XCOMM Simple configuration settings used by zstart
set dm="dm -single "
if (! $?ZEB_DM) setenv ZEB_DM \
   "xterm -T DisplayManager -bg grey90 -fg purple -fn 8x13bold -e $dm "
if (! $?ZEB_EVENTLOGGER) \
	setenv ZEB_EVENTLOGGER 'EventLogger -w'

setenv ZEBHOME $ZEB_TOPDIR
setenv HOST `uname -n`
setenv CONFIGHOME CFG_PROJECTDIR
setenv ZEB_PROJDIR $CONFIGHOME
setenv DATA_DIR CFG_DEFAULTDATADIR

if ( ! $?ZEB_CACHEDIR ) \
	setenv ZEB_CACHEDIR $CONFIGHOME/cache

