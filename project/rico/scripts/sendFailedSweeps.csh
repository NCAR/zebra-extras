#!/bin/csh
#
# Loop through all sweep files in /data/rico/sweeps_for_antigua/send_failed,
# and when there are no sweep files waiting in /data/rico/sweeps_for_antigua,
# move the next sweep file up so that it will be sent.
#

cd /data/rico/spol/send_failed

set logfile /tmp/${0:t}.log
echo "$0 started `date`" >> $logfile

foreach file (swp*)
    while (1)
	@ ntosend = `ls ../swp* | wc -l`
	if ($ntosend == 0) then
	    echo $file >> $logfile
	    mv $file ..
	    break
	else
	    sleep 3
	endif
    end
end
	    
	    
