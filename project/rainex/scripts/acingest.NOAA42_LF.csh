#!/bin/csh
#
set logfile = "/space/data/logs/NOAA42_LF.log"
echo "acingest started `date`" >> $logfile

set scriptdir = /opt/zebra/project/rainex/scripts

cd /space/data/NOAA42_LF
java -jar $scriptdir/AC_Consumer.jar -p 54321 -3 -n NOAA42_LF >>& $logfile
