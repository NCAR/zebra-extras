#!/bin/csh
#
set logfile = "/space/data/logs/nrl_p3.log"
echo "acingest started `date`" >> $logfile

cd /space/data/rainex/nrl_p3
java -jar ~burghart/AC_Consumer.NRL_P3.jar -p 33520 -n nrl_p3 -t 60 -j /opt/zebra/project/rainex/scripts/nrl_p3.vardb >>& $logfile
