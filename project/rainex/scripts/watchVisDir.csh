#!/bin/csh
# Watch for new area files in the given directory, and ingest them
# when we find them.
#
if ($#argv != 2) then
    echo "Usage: $0 <vis_dir> <frequency>"
    exit(1)
endif

set scriptDir = $0:h
set dir = $1
set sleep = $2
set plat = "goes_1km"
set logfile = "/tmp/${0:t}.log"

echo "${0}: Looking for new files for platform $plat" >> $logfile
echo "in $dir every $sleep seconds." >> $logfile

#
# we only want one of me; kill any others
#
onintr -	# ignore the INT signal we're about to send out...
killall -s INT $0:t
onintr finish	# pay attention to signals again

#
# Create a file that we use with 'find' to test for newly modified files.
#
set testfile = /tmp/$0:t_$$
cat > $testfile <<EOF
This file is used by $0:t, and exists only to have a 
modification time.  Do not delete it unless the associated 
$0:t process is dead.
EOF

#
# Go to the requested directory and keep watch for modified files,
# ingesting files when we find them.
#
unalias cd
while (1)
    foreach file (`find $dir -maxdepth 1 -newer $testfile -print`)
	if (-d $file) continue
	echo "ingesting $file" >> $logfile
	$scriptDir/ingestVis.csh $file >>& $logfile
    end
    touch $testfile
    sleep $sleep
end

finish:

rm -f $testfile
