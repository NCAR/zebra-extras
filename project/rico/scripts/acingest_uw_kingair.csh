#!/bin/csh
#
set logfile = "/data/rico/acingest.uw_kingair.log"
echo "acingest started `date`" >> $logfile

setenv JARDIR /home/burghart/janet_ACtracker/java/jars
setenv CLASSPATH "$JARDIR/ACtracker.jar:$JARDIR/getopt.jar:$JARDIR/netcdf2.jar"
setenv PATH "/opt/j2sdk1.4.2_06/bin:$PATH"

java ACtracker.Consumer -p 33521 -n /data/rico/uw_kingair/uw_kingair \
	-t 60 -j /opt/zebra/project/rico/scripts/uw_kingair.vars \
	-r 128.117.80.208:33521 >>& $logfile
