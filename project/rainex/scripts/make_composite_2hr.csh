#!/bin/csh
#
# Usage: make_composite_2hr [<unix_time>]
#
# Generate a composite of NOAA P3 LF radar data, using sweeps in the
# two hours before the given time (which defaults to now)
#

#
# Add the directory where we were found to the path, since that's where
# we expect to find findP3Sweeps.csh
#
set mypath = `which $0`
set path = ($path ${mypath:h})

@ span2hr = 7200 # time span for two hour composite

if ($# > 0) then
    @ compositeTime2hr = $1 >> /dev/null
    if ($status) then
        echo "Bad time: $1"
        goto usage
    endif
else
    #
    # Default to (near) now, backing up so we're on an even multiple of 'span'
    #
    @ curTime2hr = `date -u +%s`
#    @ leftover = $curTime % $span
#    @ compositeTime = $curTime - $leftover
    @ compositeTime2hr = $curTime2hr
endif

set sweeps2hr = `findP3Sweeps.csh $compositeTime2hr $span2hr /space/data/NOAA42_LF /space/data/NOAA43_LF`

if ($%sweeps2hr == 0) then
    exit 0
endif

#
# Make file name of the form <yyyymmdd>.<hhmm>.nc
#
set filename2hr = \
    `date -u +"%Y%m%d.%H%M.nc" -d"1970-01-01 UTC +$compositeTime2hr seconds"`

#
# Create the composite from the sweeps we found, and dsrescan it
#
/opt/src/radarcomposite/radarcomposite \
    -o /space/data/rainex/noaa_lf_composite_2hr/$filename2hr -t $compositeTime2hr $sweeps2hr
/opt/zebra/bin/dsrescan -f $filename2hr noaa_lf_composite_2hr >& /dev/null
