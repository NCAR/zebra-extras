#!/bin/csh
#
# start platwatch-es for a few platforms, if they aren't running already
# (this can be run via cron on a very regular basis)
#
if (! $?ZEB_TOPDIR) then
    echo "ZEB_TOPDIR must be set!"
    exit 1
endif

setenv ZEB_PROJDIR $ZEB_TOPDIR/project/rainex
source $ZEB_PROJDIR/proj_env
set scriptdir = "$ZEB_PROJDIR/scripts"

#
# Make sure data dir watchers are running so that we get updates in
# Zebra displays
#
foreach plat_and_update ("noaa_42 10" "noaa_43 10" "nrl_p3 10")
    set plat = `echo $plat_and_update | cut -f1 -d" "`
    ps auxw | fgrep -v grep | fgrep -q "platwatch.$plat"
    if (! $status) continue	# already running
    $scriptdir/start_watcher.csh $plat_and_update >& /dev/null
end

#
# Make sure the ACtracker.Consumer stuff is running for each aircraft
#
foreach ac ("noaa_42" "NOAA42_LF" "noaa_43" "NOAA43_LF" "nrl_p3")
    ps auxw | fgrep -v grep | fgrep "AC_Consumer" | fgrep -q $ac
    if (! $status) continue	# already running
    echo "Starting ingester for $ac at " `date`
    echo $scriptdir/acingest.${ac}.csh  | at now
end
