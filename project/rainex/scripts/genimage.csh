#!/bin/csh
setenv BATCH_IMAGE_SPOOL /data/rico/auto_images
setenv DS_DAEMON_HOST zebra1
setenv ZEB_TOPDIR /opt/zebra
setenv ZEB_PROJDIR $ZEB_TOPDIR/project/rico
setenv DATA_DIR /data/rico
setenv PATH /usr/X11R6/bin:$ZEB_TOPDIR/bin:$PATH  # to get Xvfb

set logfile = /tmp/${0:t}.log

# 
# Get the plot time if given, else use now
# 
if ($# == 0) then
    set datestring = `date -u`
else if ($# == 1) then
    set datestring = "$1"
else
    echo "Usage: $0 [<dateTimeString>]"
    echo 'with optional dateTimeString in a form parseable by "date"'
    exit 1
endif

set plottime = `date -u +"%d-%b-%Y,%H:%M:00" -d "$datestring"`
set yyyymmdd = `date -u +"%Y%m%d" -d "$datestring"`
set hhmm = `date -u +"%H%M" -d "$datestring"`

echo >> $logfile
echo >> $logfile
echo >> $logfile
echo "Generate images for $plottime ($yyyymmdd$hhmm)" >> $logfile

#
# plot
#
/opt/zebra/bin/zplotbatch $plottime auto dbz vr zdr >>& $logfile
