#!/bin/csh -f
#
# Run ACtracker.Consumer to ingest nrl_p3 data from port 25001
#
cd /code/burghart/ACtracker

set logfile = "/tmp/nrl_p3ingest.log"
echo "$0 started `date`" >>& $logfile

set path = (/code/burghart/java/bin /usr/local/zebra/bin $path)

set datadir = /scr/js1/bamex/nrl_p3

#
# Run the ingestor (repeating incoming packets to bamex-gate)
#
setenv CLASSPATH \
	"java/jars/ACtracker.jar:java/jars/getopt.jar:java/jars/netcdf2.jar"
java ACtracker.Consumer -p 25001 -n $datadir/nrl_p3 -t 60 \
    -r bmxgate.cust.qwest.net:25001 >>& $logfile
