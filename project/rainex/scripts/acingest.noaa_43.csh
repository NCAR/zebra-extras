#!/bin/csh
#
set logfile = "/space/data/logs/noaa_43.log"
echo "acingest started `date`" >> $logfile

set scriptdir = /opt/zebra/project/rainex/scripts

cd /space/data/rainex/noaa_43
java -jar $scriptdir/AC_Consumer.jar -p 54324 -k -n noaa_43 -t 60 >>& $logfile
