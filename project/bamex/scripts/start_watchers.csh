#!/bin/csh
#
# start platwatch for a few platforms
#

setenv ZEB_TOPDIR /usr/local/zebra
setenv ZEB_PROJDIR $ZEB_TOPDIR/project/bamex
source $ZEB_PROJDIR/proj_env
set scriptdir = "$ZEB_PROJDIR/scripts"

#
# 60 second updates
#
foreach plat (goes_1km goes_4km radar_composite)
    #
    # execute using "at" rather than backgrounding, so that signals
    # behave as expected
    #
    echo "$scriptdir/platwatch.$plat $DATA_DIR/$plat 60 >&" \
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
    echo "$scriptdir/platwatch.$plat $DATA_DIR/$plat 10 >&" \
	 "/tmp/platwatch.$plat.log" | at "now + 1 minute"
end
