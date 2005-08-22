#!/bin/csh
#
# Usage:
#    findP3Sweeps.csh <targetTime> <timeThresh> <sweep_dir> [<sweep_dir> ...]
#
# Look through the list of <sweep_dir>-s, finding sweeps at or before
# targetTime (in seconds since 1970-01-01 00:00 UTC) but within
# timeThresh seconds of targetTime. Print the names of the sweeps 
# which match.
#
# Only sweep files which match the pattern <radar_name>-<yyyymmdd>-<hhmmss>.nc
# are considered.  (This no longer applies if "dumpNOAATime" is being used 
# below, the current default).
#

#
# Add the directory where we were found to the path, since that's where
# we expect to find dumpNOAAtime
#
set mypath = `which $0`:h
set path = ($path ${mypath:h})

if ($# < 3) goto usage

@ targetTime = $1
if ($status) goto usage

@ timeThresh = $2
if ($status) goto usage

set dirs = ($argv[3-])
foreach dir ($dirs)
    if (! -d $dir) then
	echo "$dir is not a directory"
	goto usage
    endif
end

foreach dir ($dirs)
    foreach filename (`find $dir -maxdepth 1 -name "*nc"`)
# 	set datestring = `echo ${filename:t} | cut -d'-' -f2`
# 	@ yyyymmdd = $datestring >& /dev/null
# 	if ($status) continue
# 	@ YYYY = $yyyymmdd / 10000
# 	@ MM = ($yyyymmdd / 100) % 100
# 	@ DD = $yyyymmdd % 100

# 	set timestring = `echo ${filename:t:r} | cut -d'-' -f3`
# 	@ hhmmss = $timestring >& /dev/null
# 	if ($status) continue
# 	@ hh = $hhmmss / 10000
# 	@ mm = ($hhmmss / 100) % 100
# 	@ ss = $hhmmss % 100

# 	@ fileTime = \
# 	    `date -u +%s -d"${YYYY}-${MM}-${DD} ${hh}:${mm}:${ss} UTC"`

	@ fileTime = `dumpNOAATime $filename`
	@ diff = $targetTime - $fileTime
	if ($diff < 0 || $diff >= $timeThresh) continue

	echo $filename
    end
end

exit 0

usage:
    echo "Usage: $0 <time> <time_thresh> <swp_dir> [<swp_dir> ...]"
    exit 1
