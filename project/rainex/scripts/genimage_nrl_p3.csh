#!/bin/csh
setenv BATCH_IMAGE_SPOOL /space/data/nrl_images
setenv DS_DAEMON_HOST rainex3
setenv ZEB_TOPDIR /opt/zebra
setenv ZEB_PROJDIR $ZEB_TOPDIR/project/rainex
source $ZEB_PROJDIR/proj_env	# to get ZEB_SOCKET
setenv PATH /usr/X11R6/bin:$ZEB_TOPDIR/bin:$PATH  # to get Xvfb

set logfile = /tmp/${0:t}.log

#
# Arg check
#
if ($# > 1) then
    echo "Usage: ${0:t} [dd-mmm-yyyy,hh:mm]"
    exit 1
endif

# 
# use now as the plot time, if none was given
# 
if ($# == 1) then
    set zebradate = $1
    set realtime = 1
else
    set zebradate = `date -u +"%d-%b-%Y,%H:%M:00"`
    set realtime = 0
endif

#
# From the Zebra time, change the comma to a space to make a 
# more standard date string, then convert it into a Unix time
#
set datestring = `echo $zebradate | sed 's/,/ /g'`
set plottime = `date +%s -d"$datestring UTC"`

#
# move the plot location to the current aircraft location
#
zstart -n -preserve -ds $ZEB_PROJDIR >& $logfile

dc_shiftloc --olat 25.000 --olon -70.000 --platform noaa_42 \
	    --time $plottime \
	    --dc     $ZEB_PROJDIR/dconfig/LFComposite_template.dc \
	    --out_dc $ZEB_PROJDIR/dconfig/LFComposite.dc >& $logfile

if ($status) then
	echo Exiting on dc_shiftloc error
	exit 1
endif

#
# Make a file to mark the current time, so we can find any new
# images
#
set timetest = $BATCH_IMAGE_SPOOL/.time
rm -f $timetest
date > $timetest

#
# Plot
#
zplotbatch $zebradate LFComposite auto >>& $logfile

#
# make a link to the latest image
#
if (! $realtime)
    exit 0

sleep 10		# wait for the new png to be ready
cd $BATCH_IMAGE_SPOOL

#
# look a few times until we find the new image available or we give up
#
@ try = 0
while ($try < 3)
   set new_img = `find . -name "*auto.png" -newer $timetest | sort | tail -1`
   if ($new_img != "") break
   @ try++
   sleep 5
end

if ($new_img != "") then
	mv -f latest-4.png latest-5.png
	mv -f latest-3.png latest-4.png
	mv -f latest-2.png latest-3.png
	mv -f latest-1.png latest-2.png
	mv -f latest.png latest-1.png
	ln -s $new_img latest.png
else
	echo "No image available after $try tries"
endif
