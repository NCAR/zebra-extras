#!/bin/csh
#
# Ingest a GOES vis 1km image
# Usage: ingestVis.csh <area_file>
#
if ($# != 1) then
    echo "Usage: $0 <areaFile>"
    exit 1
endif

set areaFile="$1"

set initfile="/tmp/ingestVis.ini"
rm -f $initfile

cat > $initfile <<EOF
set platform "goes_1km"
set kmResolution 1
file $areaFile vis
EOF

/opt/zebra/bin/SatIngest -init $initfile

