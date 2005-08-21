#!/bin/csh
#
set logfile = "/space/data/logs/NOAA42_LF.log"
echo "acingest started `date`" >> $logfile

cd /space/data/NOAA42_LF
java -jar ~burghart/AC_Consumer.jar -p 54321 -3 -n NOAA42_LF >>& $logfile
