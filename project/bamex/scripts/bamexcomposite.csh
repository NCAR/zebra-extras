#!/bin/csh
# generate a BAMEX composite of NIDS sweeps

#
# Nominal time: most recent even increment of ten minutes that is at
# least five minutes ago
#
if ($#argv > 0) then
    @ time = $1
    echo "Using time" `date -u -d "1970-01-01 + $time seconds"`
else
    @ time = `date +%s`
    @ time -= 300
    @ time /= 600
    @ time *= 600
endif

#
# Make the output file name
#
set dstring = `date -u -d "1970-01-01 + $time seconds" +%Y%m%d.%H%M`
set ncfilename = /scr/js1/bamex/radar_composite/${dstring}.nc

#
# Find the NIDS files closest to this time
#

set flist = /tmp/nidscomposite.files.$$
rm -f $flist
touch $flist
chmod 666 $flist

foreach dir (/scr/js1/ldm/nexrad3/*)
    @ delta = 0
    while ($delta <= 10)
	# try time + delta * 60
	@ dtime = $time + $delta * 60
	set dstring = `date -u -d "1970-01-01 + $dtime seconds" +%Y%m%d%H%M`
	set nids_Rfile = `find $dir -name "*${dstring}.0R.nids"`
	set nids_Vfile = `find $dir -name "*${dstring}.0V.nids"`
 	if ($#nids_Rfile && $#nids_Vfile) then
 	    echo $nids_Rfile >> $flist
 	    echo $nids_Vfile >> $flist
 	    break
 	endif

	# try time - delta * 60
	@ dtime = $time - $delta * 60
	set dstring = `date -u -d "1970-01-01 + $dtime seconds" +%Y%m%d%H%M`
	set nids_Rfile = `find $dir -name "*${dstring}.0R.nids"`
	set nids_Vfile = `find $dir -name "*${dstring}.0V.nids"`
 	if ($#nids_Rfile && $#nids_Vfile) then
 	    echo $nids_Rfile >> $flist
 	    echo $nids_Vfile >> $flist
 	    break
 	endif

	@ delta++
    end
end

/code/burghart/nidstools/nidscomposite -o $ncfilename \
    -103 32 -82 49 0.01 $time `cat $flist`
rm -f $flist

echo "created $ncfilename"

echo -n "   copying to bamex-gate..."
scp -Cp $ncfilename 67.128.247.125:/data/bamex/radar_composite
if ($status) then
	echo "FAILED"
else
	echo "OK"
endif
