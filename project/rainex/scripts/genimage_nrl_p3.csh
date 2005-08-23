#!/bin/csh
setenv BATCH_IMAGE_SPOOL /space/data/nrl_images
setenv DS_DAEMON_HOST rainex3
setenv ZEB_TOPDIR /opt/zebra
setenv ZEB_PROJDIR $ZEB_TOPDIR/project/rainex
source $ZEB_PROJDIR/proj_env	# to get ZEB_SOCKET
setenv PATH /usr/X11R6/bin:$ZEB_TOPDIR/bin:$PATH  # to get Xvfb

set logfile = /space/data/logs/${0:t}.log
mv -f $logfile $logfile.old
touch $logfile
chmod 777 $logfile

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
    set realtime = 0
else
    #
    # default to now
    #
    set zebradate = `date -u +"%d-%b-%Y,%H:%M:00"`
    set realtime = 1
endif

#
# From the Zebra time, change the comma to a space to make a 
# more standard date string, then convert it into a Unix time
#
set datestring = `echo $zebradate | sed 's/,/ /g'`
@ plottime = `date +%s -d"$datestring UTC"`

#
# If we're in real-time mode, bump the plot time back ten seconds, 
# to avoid tickling a DS bug with dc_shiftloc/platloc
#
if ($realtime) @ plottime -= 10

#
# move the plot location to the current aircraft location
#
zstart -n -preserve -ds $ZEB_PROJDIR >> $logfile

@ try = 0
while (1)
    @ try++
    dc_shiftloc --olat 25.000 --olon -70.000 --platform nrl_p3 \
	    --time $plottime \
	    --size 400 \
	    --dc     $ZEB_PROJDIR/dconfig/LFComposite_template.dc \
	    --out_dc $ZEB_PROJDIR/dconfig/LFComposite.dc >> $logfile

    if ($status) then
	echo "dc_shiftloc error\!"
	if ($try > 5) then
	    echo "Exiting after $try tries"
	    exit 1
	endif
    else
	break
    endif
end

#
# Make a file to mark the current time, so we can find any new
# images
#
set timetest = $BATCH_IMAGE_SPOOL/.time
rm -f $timetest
date > $timetest
chmod 777 $timetest

#
# Plot
#
zplotbatch $zebradate LFComposite auto >>& $logfile

#
# If we're not running in real time, exit now
#
if (! $realtime) exit 0

#
# Give our image some time to get completed
#
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
    #
    # Make this image available via LDM
    #
    pushd $new_img:h >> /dev/null
    /usr/local/ldm/bin/pqinsert -f EXP $new_img:t >>& $logfile
    if (! $status) then
	echo "Image $new_img:t inserted in LDM queue at " `date` >> $logfile
    else
	echo "Image $new_img:t LDM insert failed at " `date` >> $logfile
	echo "Image $new_img:t LDM insert failed at " `date`
    endif
    popd >> /dev/null
    #
    # Make a "latest.png" link to the latest image
    #
    rm -f latest.png
    ln -s $new_img latest.png
else
    echo "$0: No image available after $try tries" >> $logfile
    echo "$0: No image available after $try tries"
endif
