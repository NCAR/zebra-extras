#!/bin/csh -f

set path = (/usr/java/j2sdk1.4.0/bin $path)

cd /scr/js1/bamex/WMI_Lear_dropsonde/ACtrack

set cp = /h/atd/maclean/work/ACtracker/java/jars

set logfile = "wmi_ingest.log"

java -cp $cp/getopt.jar:$cp/ACtracker.jar:$cp/netcdf2.jar ACtracker.Consumer -p 25003 -n WMI_Lear -t 60 -T WMI_BAMEX -r bmxgate.cust.qwest.net:25003 >& $logfile
