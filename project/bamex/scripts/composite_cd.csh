#!/bin/csh
#
# Write a CD of BAMEX tar'red & gzip'ped radar composites
#

if ($#argv != 1) then
    echo "Usage: $0 <start_yyyymmdd>"
    exit 1
endif

#
# Only root or members of the cdwrite group can run this successfully
#
set user = `id -un`
set group = `id -gn`
if ($user != "root" && $group != "cdwrite") then
    echo "This script can only be run by root, or users in the 'cdwrite' group"
    exit 1
endif

#
# Remove any old file list and create an empty new one
#
set filelist = /tmp/${0:t}.files
rm -f $filelist
touch $filelist
chmod 666 $filelist

#
# Start is the given date
#
@ start = $1

#
# Max CD size and current remainder
#
@ maxsize = 700 * 1024 # KB on CD
if ($?MIN_CDSIZE) then
    @ maxremainder = $maxsize - $MIN_CDSIZE
else
    @ maxremainder = 200000
endif
@ remainder = $maxsize

#
# From the given start day, move forward until we run out of files
# or until we can no longer fit another file on the disk
#
cd /scr/js1/bamex
@ day = 0
while (1)
    @ testend = `date -d"$start + $day days" +"%Y%m%d"`
    set nextfile = "composite.$testend.tgz"
    if (! -f $nextfile) then
	if ($day == 0) then
	    echo "No file for $start"
	    exit 1
	else if ($remainder < $maxremainder) then
	    break
	else
	    echo "Not enough days from $start to fill a CD yet"
	    exit 1
	endif
    endif
    @ nextsize = `ls -s $nextfile | cut -f1 -d" "`
    if ($nextsize > $remainder) break
    echo $nextfile >> $filelist
    @ end = $testend
    @ remainder -= $nextsize
    @ day++
end

@ disksize = $maxsize - $remainder
echo "Writing from $start to $end ($disksize KB)"

#
# Create a volume id with the start and end month/day
#
@ startmonth = ($start / 100) % 100
@ startday = $start % 100
@ endmonth = ($end / 100) % 100
@ endday = $end % 100
set volid = "BAMEX composites $startmonth/$startday - $endmonth/$endday"

#
# Now run mkisofs and cdrecord to burn the disk
#
echo -n "Insert a blank CD and type <Enter>: "
set junk = $<

mkisofs -r -J -pad -quiet -V "$volid" -path-list $filelist | \
 cdrecord dev=0,3,0 speed=40 fs=16m driveropts=burnfree gracetime=2 -v -eject -

rm -f $filelist
