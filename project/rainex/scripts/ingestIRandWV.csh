#!/bin/csh
#
# Ingest a GOES 4km IR/WV image pair
# Usage: ingestIRandWV.csh <IR_area_file> <WV_area_file>
#
if ($# != 2) then
    echo "Usage: $0 <IR_areaFile> <WV_areaFile>"
    exit 1
endif

set irAreaFile="$1"
set wvAreaFile="$2"

set initfile="/tmp/ingestIRandWV.ini"
rm -f $initfile

cat > $initfile <<EOF
set platform "goes_4km"
set kmResolution 4
file $irAreaFile ir
file $wvAreaFile wv
EOF

/opt/zebra/bin/SatIngest -init $initfile

