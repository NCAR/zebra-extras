#!/bin/csh
#
# Run FAAM_Parser.py to ingest track information for the FAAM BAE-146 aircraft.
# This should be run via cron on a very regular basis.
#
set logfile = /tmp/${0:t}.log
echo $0 at `date` >> $logfile
# FAAM_Parser.py should live in the same directory as this script
set scriptdir = $0:h
$scriptdir/FAAM_Parser.py >>& $logfile
