XCOMM !/bin/csh -f
XCOMM
XCOMM Environment variable settings specific to ARM
XCOMM $Id: proj_env.cpp,v 1.1 1997-02-04 14:56:18 granger Exp $

echo "Reading proj_env"

XCOMM Simple configuration settings used by zstart
set dm="dm -single "
if (! $?ZEB_DM) setenv ZEB_DM \
   "xterm -T DisplayManager -bg grey90 -fg purple -fn 8x13bold -e $dm "
if (! $?ZEB_EVENTLOGGER) \
	setenv ZEB_EVENTLOGGER 'EventLogger -w'

setenv ZEBHOME $ZEB_TOPDIR
setenv HOST `uname -n`
setenv ZEB_PROJECT ARMProject
setenv CONFIGHOME ProjectDir
setenv ZEB_PROJDIR $CONFIGHOME

XCOMM This is mostly to keep zstart quiet.
XCOMM DATAHOME will take precedence in ds.config
XCOMM
setenv DATA_DIR DefaultDataDir

XCOMM Force multiple option on dm command line
XCOMM
XCOMM setenv ZEB_DM_CONFIG '-multiple dm.config'

