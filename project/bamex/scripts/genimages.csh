#!/bin/csh
onintr finishup

setenv BATCH_IMAGE_SPOOL /scr/hallertau/burghart/bamex_images
setenv ZEB_TOPDIR /net/local_lnx/zebra
setenv ZEB_PROJDIR $ZEB_TOPDIR/project/bamex
setenv DATA_DIR /scr/js1/bamex
setenv ZEB_SOCKET /tmp/zebra.$$.socket
setenv PATH /usr/X11R6/bin:$ZEB_TOPDIR/bin:$PATH	# to get Xvfb

set logfile = /tmp/${0:t}.log

@ starttime = 1053388800	# 20 May 2003
@ endtime = 1057622400		# 8 July 2003
@ time = $starttime

zstart -n -preserve -ds $ZEB_PROJDIR >& $logfile

while ($time < $endtime)
    #
    # If this is the start of a new day, remove unneeded files, and 
    # unpack today's
    #
    set daystring = \
	`date -u +"%Y%m%d" -d"1 Jan 1970 00:00 UTC +$time seconds"`

    @ yesterday = $time - 86400
    set yesterdaystring = \
	`date -u +"%Y%m%d" -d"1 Jan 1970 00:00 UTC +$yesterday seconds"`
    echo "New day $daystring"

    foreach plat (radar_composite goes_4km)
	cd $DATA_DIR/$plat
	# remove all netCDF files except for yesterday's
	rm -f `ls *nc | fgrep -v $yesterdaystring`
	tar xvfz *$daystring*.tgz
	dsrescan $plat
    end

    set batchfile = "/tmp/batch.list"
    rm -f $batchfile
    while ($time <= $endtime)
	# 
	# use now as the plot time
	# 
	set zebratime = \
	 `date -u +"%d-%b-%Y,%H:%M:00" -d"1 Jan 1970 00:00 UTC +$time seconds"`
	if (! $?batchstart) set batchstart = $zebratime
	set batchend = $zebratime

	#
	# add this time to the batch list
	#
	echo $zebratime image_template auto >> $batchfile
	#
	# increment by ten minutes
	#
	@ time += 600
	if (($time % 86400) == 0) break
    end
    echo batch plotting from $batchstart to $batchend
    unset batchstart
    cat $batchfile | zplotbatch - >>& $logfile
end

sleep 10

finishup:
zstop

