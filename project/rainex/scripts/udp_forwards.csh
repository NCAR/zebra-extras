#!/bin/csh
#
# start udp_forward-s for a bunch of ports, if they aren't running already
# (this can be run via cron on a very regular basis)
#

foreach port (33520 54320 54321 54324 54325 54328)
    ps auxw | fgrep udp_forward | fgrep -q $port
    if (! $status) continue	# already running

    #
    # restart udp_forward for this port
    #
    echo "Starting udp_forward for $port @ " `date`
    echo "~/udp_forward/udp_forward $port rainex3:$port" | at now
end
