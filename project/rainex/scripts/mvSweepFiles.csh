#!/bin/csh
#
# Move sweep files older than the specified number of hours into directories
# under the given destination directory named by date (YYYYMMDDhhmmss)
#
set srcdir = /data/rico/spol
set destdir = /data/rico/spol

#
# Make a list of sweeps older than the specified time in the source directory
#
@ hours = 24
@ minutes = $hours * 60
set oldswps = `find $srcdir -maxdepth 1 -name "swp*" -mmin +$minutes`

foreach swp ($oldswps)
    @ yyyymmdd = `echo $swp | cut -f2 -d"." | head -c7` + 19000000
    mkdir -p $destdir/$yyyymmdd
    mv $swp $destdir/$yyyymmdd/
end

/opt/zebra/bin/dsrescan spol > /dev/null
