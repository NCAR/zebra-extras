#!/bin/csh
#
set logfile = "/space/data/logs/noaa_42.log"
echo "acingest started `date`" >> $logfile

set scriptdir = /opt/zebra/project/rainex/scripts

cd /space/data/rainex/noaa_42
java -jar $scriptdir/AC_Consumer.jar -p 54320 -k -n noaa_42 -t 60 >>& $logfile
