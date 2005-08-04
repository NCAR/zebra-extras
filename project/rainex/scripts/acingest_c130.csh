#!/bin/csh
#
set logfile = "/data/rico/acingest.c130.log"
echo "acingest started `date`" >> $logfile

setenv JARDIR /home/burghart/janet_ACtracker/java/jars
setenv CLASSPATH "$JARDIR/ACtracker.jar:$JARDIR/getopt.jar:$JARDIR/netcdf2.jar"
setenv PATH "/opt/j2re1.4.1_02/bin:$PATH"

java ACtracker.Consumer -p 33520 -n /data/rico/c130/c130 -t 60 -j /opt/zebra/project/rico/scripts/c130.vars >>& $logfile
