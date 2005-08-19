#!/bin/csh
#
# Usage: make_composite [<unix_time>]
#
# Generate a composite of NOAA P3 LF radar data, using sweeps in the
# five minutes before the given time (which defaults to now)
#

#
# Add the directory where we were found to the path, since that's where
# we expect to find findP3Sweeps.csh
#
set mypath = `which $0`:h
set path = ($path ${mypath:h})

@ span = 300 # five minute time span for composite

if ($# > 0) then
    @ compositeTime = $1 >> /dev/null
    if ($status) then
	echo "Bad time: $1"
	goto usage
    endif
else
    #
    # Default to (near) now, backing up so we're on an even multiple of 'span'
    #
    @ curTime = `date -u +%s`
    @ leftover = $curTime % $span
    echo "leftover " $leftover
    @ compositeTime = $curTime - $leftover
endif

set sweeps = `findP3Sweeps.csh $compositeTime $span /space/data/NOAA42_LF /space/data/NOAA43_LF`

if ("$sweeps" == "") then
    exit 0
endif

set filename = \
    `date -u +"%Y%m%d.%H%M.nc" -d"1970-01-01 UTC +$compositeTime seconds"`

/opt/src/radarcomposite/radarcomposite -o $filename -t $compositeTime $sweeps
