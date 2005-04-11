#!/bin/csh
#
# FTP images under /data/rico/auto_images to the JOSS catalog
#
set imageDir = "/data/rico/auto_images"
set doneImageDir = "/data/rico/sent_images"

set logfile = "/tmp/${0:t}.log"
set ftpCmdFile = "/tmp/${0:t}.ftp"

echo >> $logfile
echo >> $logfile
echo "$0 started `date`" >> $logfile

cd $imageDir
while (1)
    #
    # Find the five latest remaining images.  Doing five at a time assures that
    # we can get the recent images over in a reasonable time, even if we 
    # generate a boatload of older images.
    #
    foreach img (`find . -name "*png" | sort -r | head -5`)
	#
	# Make sure the destination directory exists
	#
	echo "FTPing $img" >> $logfile
	set srcdir = $img:h
	mkdir -p $doneImageDir/$srcdir >>& $logfile

	#
	# Change "auto" to "research.S-Pol" in the file name
	#
	set newimg = `echo $img | sed 's/auto/research.S-Pol/g'`
	mv $img $newimg >>& $logfile
	set destimg = $doneImageDir/$newimg

	echo "cd pub/incoming/catalog/rico" > $ftpCmdFile
	echo "put $newimg $newimg:t" >> $ftpCmdFile
	ftp ftp.joss.ucar.edu < $ftpCmdFile >>& $logfile
	if (! $status) then
	    mv $newimg $destimg
	else
	    echo "FTP of $newimg failed" >> $logfile
	endif
    end
    #
    # All images sent (or attempted)!  Wait a minute and try again...
    #
    sleep 60
end
