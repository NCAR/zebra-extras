#!/bin/csh
#
# Look at the latest GOES8 4km vis/IR pair and ingest them if we haven't
# already
#
set path = (/code/burghart/zebra/bin $path)

set lastvis = `ls /scr/js1/ldm/goes8_4km/*0.65um | tail -1`
set lastir = `echo $lastvis | sed 's/0.65/10.7/g'`
if (! -f $lastir) then
    echo "no IR file"
    exit 0
endif
    
#
# get the time string in a form parseable by "date"
#
set timestring = `areadump $lastvis | cut -d' ' -f1 | sed 's/,/ /g'`

#
# See if we have a raster file for that time; if no raster file, then 
# ingest the images
#
set rfile = /scr/js1/bamex/goes8_4km/goes8_4km.`date -u +%Y%m%d.%H%M%S -d "$timestring"`.rf
if (-f $rfile) exit 0

echo Ingesting $timestring GOES8 images
/code/burghart/bamex/scripts/ingest_goes8.csh $lastvis $lastir >& /dev/null
