#!/bin/csh
set path = (/opt/zebra/bin $path)
setenv BATCH_IMAGE_SPOOL /data/bamex/auto_images
setenv DS_DAEMON_HOST zebra1
setenv ZEB_PROJDIR bamex
setenv PATH /usr/X11R6/bin:$PATH  # to get Xvfb when we're started from cron

# 
# use now as the plot time
# 
set plottime = `date -u +"%d-%b-%Y,%H:%M"`

#
# move the plot location to the current aircraft location
#
dc_shiftloc --olat 38.54517 --olon -89.83517 --platform nrl_p3 \
            --dc     /opt/zebra/project/dconfig/p3_image.dc \
	    --out_dc /opt/zebra/project/dconfig/p3_image_tmp.dc
#
# and plot
#
zplotbatch $plottime ac_image_tmp x >& /tmp/${0:t}.log
