#!/bin/csh
#
# Script to ingest flight tracks from Flight Explorer.  Run this script
# via cron every five minutes to grab the latest file from the Flight
# Explorer ftp site and ingest it.
#
# You must have a ~/.netrc file containing the following line:
#
#	machine 65.165.242.49 login ncar001 password ncarsuper
#
# in order to run this from cron, since this will allow ftp to log in
# automatically.
#


#
# Get the time at the previous five minute mark
#
@ posixtime = `date +%s`
@ posixtime /= 300
@ posixtime *= 300

#
# Make filenames from the time (file names at Flight Explorer 
# use EST/EDT timezone)
#
setenv TZ "EST5EDT"
set hhmm = `date +%H%M -d"1-Jan-1970 00:00 UTC +$posixtime seconds"`

set last_tzfile = NCARTZ$hhmm.txt
set tmpfile = /tmp/$last_tzfile

rm -f $tmpfile

#
# Get the TZ file from the ftp site
#
/usr/bin/ftp 65.165.242.49 <<EOF
    cd ncar
    get $last_tzfile $tmpfile
EOF

#
# If we didn't get a file, there's a problem
#
if (! -f $tmpfile) then
    echo "error FTPing $last_tzfile"
    exit 1
endif

if (-z $tmpfile) then
    rm $tmpfile
    exit 0
endif

#
# Ingest the file we grabbed
#
setenv ZEB_TOPDIR /opt/zebra
$ZEB_TOPDIR/bin/FlightExplorer \
    $ZEB_TOPDIR/project/bamex/scripts/FlightExplorer_map.bamex $tmpfile

#
# "archive" this file
#
cat $tmpfile >> /data/bamex/FlightExplorer.data
rm $tmpfile
