#!/bin/csh
#
# start platwatch for a few platforms
#

set scriptdir = "/code/burghart/bamex/scripts"
set datadir = "/scr/js1/bamex"

#
# 60 second updates
#
foreach plat (goes_1km goes_4km radar_composite)
    #
    # execute using "at" rather than backgrounding, so that signals
    # behave as expected
    #
    echo "$scriptdir/platwatch.$plat $datadir/$plat 60 >&" \
	 "/tmp/platwatch.$plat.log" | at "now + 1 minute"
end

#
# 10 second updates
#
foreach plat (nrl_p3)
    #
    # execute using "at" rather than backgrounding, so that signals
    # behave as expected
    #
    echo "$scriptdir/platwatch.$plat $datadir/$plat 10 >&" \
	 "/tmp/platwatch.$plat.log" | at "now + 1 minute"
end
