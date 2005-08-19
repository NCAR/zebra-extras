#!/bin/csh

#
# Make sure the ACtracker.Consumer stuff is running for each aircraft
#
foreach ac ("noaa_42" "noaa_43" "nrl_p3")
    ps auxw | fgrep -v grep | fgrep "ACtracker.Consumer" | fgrep -q $ac
    if (! $status) continue	# already running
    echo $scriptdir/acingest_${ac}.csh  | at now
end
