#!/bin/csh
#
# start platwatch-es for a few platforms, if they aren't running already
# (this can be run via cron on a very regular basis)
#

if (! $?ZEB_TOPDIR) then
    echo "ZEB_TOPDIR must be set!"
    exit 1
endif

setenv ZEB_PROJDIR $ZEB_TOPDIR/project/rico
source $ZEB_PROJDIR/proj_env
set scriptdir = "$ZEB_PROJDIR/scripts"

foreach plat_and_update ("spol 10" "c130 10" "uw_kingair 10")
    set plat = `echo $plat_and_update | cut -f1 -d" "`
    ps auxw | fgrep -v grep | fgrep -q "platwatch.$plat"
    if (! $status) continue	# already running
    $scriptdir/start_watcher.csh $plat_and_update >& /dev/null
end

#
# Make sure the ACtracker.Consumer stuff is running for each aircraft
#
foreach ac ("c130" "uw_kingair")
    ps auxw | fgrep -v grep | fgrep "ACtracker.Consumer" | fgrep -q $ac
    if (! $status) continue	# already running
    echo $scriptdir/acingest_${ac}.csh  | at now
end

#
# Watch GOES data directories and ingest new files
#
foreach scriptAndDir ("watchVisDir.csh /data/ldm/images/sat/GOES-12/1km/VIS" \
	      "watchIRandWVDirs.csh /data/ldm/images/sat/GOES-12/4km/IR")
    set script = `echo $scriptAndDir | cut -f1 -d" "`
    ps auxw | fgrep -v grep | fgrep -q $script
    if (! $status) continue	# already running
    echo $scriptdir/$scriptAndDir 60 | at now
end

#
# Make sure FTP image transfer to the JOSS catalog is happening
#
ps auxw | fgrep -v grep | fgrep -q ftpImages.csh
if (!$status) then
    echo $scriptdir/ftpImages.csh | at now
endif
