#!/bin/csh
if ($#argv != 2) then
    echo "Usage: $0 <vis_areafile> <ir_areafile>"
    exit 1
endif

setenv ZEB_TOPDIR /code/burghart/zebra
set path = (${ZEB_TOPDIR}/bin $path)

set visfile = $1
set irfile = $2

set initfile = "/tmp/satingest.ini"

cat > $initfile <<EOF
limits 32.0 -102.0 46.0 -82.0
set platform "goes8_4km"
set originLat 39.0
set kmResolution 4
file $visfile vis
file $irfile ir
EOF

setenv ZEB_PROJDIR /code/burghart/bamex
setenv DATA_DIR /scr/js1/bamex/satingest
setenv ZEB_SOCKET /tmp/zebra_${0:t}.socket
setenv ZEB_EVENTLOGGER "EventLogger -n -f $DATA_DIR/${0:t}.log"
zstart -dsonly
SatIngest -init $initfile
zstop
rm -f $initfile
