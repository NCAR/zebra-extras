#!/bin/csh
#
# start platwatch for a given platform
#

if ($#argv != 2) then
    echo "Usage: $0 <platform> <update_seconds>"
    exit 1
endif

if (! $?ZEB_TOPDIR) then
    echo "ZEB_TOPDIR must be set!"
    exit 1
endif

setenv ZEB_PROJDIR $ZEB_TOPDIR/project/bamex
source $ZEB_PROJDIR/proj_env

set plat = $1
set update = $2

set platwatch = "$ZEB_PROJDIR/scripts/platwatch.$plat"
set platdir = "$DATA_DIR/$plat"

#
# execute using "at" rather than backgrounding, so that signals
# behave as expected
#
echo "$platwatch $platdir $update >&" "/tmp/platwatch.$plat.log" | at now
