#!/bin/csh -f
#
# Run ACtracker.Consumer to ingest WMI Lear data from port 25003, and
# run a Zebra platform directory watcher to keep the datastore
# updated.
#

set platform = lear
set type = WMI_BAMEX	# ADS, WMI_BAMEX, or DC8
set datadir = /data/bamex/$platform
set path = (/opt/java/bin $path)

set logfile = "/tmp/${platform}_ingest.log"
echo "$0 started `date`" >> $logfile

cd $datadir

set cp = /opt/src/ACtracker.old/java/jars

#
# Run a directory watcher to force dsrescans as new data are written.
# Use "at", since just running and backgrounding the platwatch script 
# just doesn't work...
#
at "now + 1 minute" <<EOF
    /opt/zebra/project/bamex/scripts/platwatch.$platform $datadir 10 >> \
	$logfile 2>&1
EOF

#
# Run the ingestor
#
java -cp $cp/getopt.jar:$cp/ACtracker.jar:$cp/netcdf2.jar \
    ACtracker.Consumer -p 25003 -n $platform -t 60 -T $type >>& $logfile