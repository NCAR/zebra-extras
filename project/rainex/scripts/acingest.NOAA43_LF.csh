#!/bin/csh
#
set logfile = "/space/data/logs/NOAA43_LF.log"
echo "acingest started `date`" >> $logfile

set scriptdir = /opt/zebra/project/rainex/scripts

cd /space/data/NOAA43_LF
java -jar $scriptdir/AC_Consumer.jar -p 54325 -3 -n NOAA43_LF >>& $logfile
