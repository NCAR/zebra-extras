#!/bin/csh -f
#
# make sure a data store is going, then run noaa_p3 to ingest 
# NOAA P3 data from port 25002
#
setenv ZEB_TOPDIR /opt/zebra

setenv ZEB_PROJDIR $ZEB_TOPDIR/project/bamex
setenv PATH $ZEB_TOPDIR/bin:$PATH

set logfile = "/tmp/noaa_p3ingest.log"
echo "$0 started `date`" >>& $logfile

$ZEB_TOPDIR/bin/zstart -n -preserve -ds $ZEB_PROJDIR >& $logfile
$ZEB_TOPDIR/bin/noaa_p3 noaa_p3 25002 >>& $logfile
