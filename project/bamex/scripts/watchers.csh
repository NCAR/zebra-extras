#!/bin/csh
#
# start platwatch-es for a few platforms, if they aren't running already
# (this can be run via cron on a very regular basis)
#

if (! $?ZEB_TOPDIR) then
    echo "ZEB_TOPDIR must be set!"
    exit 1
endif

setenv ZEB_PROJDIR $ZEB_TOPDIR/project/bamex
source $ZEB_PROJDIR/proj_env
set scriptdir = "$ZEB_PROJDIR/scripts"

foreach plat_and_update ("goes_1km 60" "goes_4km 60" "radar_composite 60" \
			 "nrl_p3 10" "lear 10")
    set plat = `echo $plat_and_update | cut -f1 -d" "`
    ps auxw | fgrep -v grep | fgrep -q "platwatch.$plat"
    if (! $status) continue	# already running
    $scriptdir/start_watcher.csh $plat_and_update >& /dev/null
end
