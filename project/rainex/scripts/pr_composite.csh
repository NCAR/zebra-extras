#!/bin/csh
# generate the San Juan "composite" of NIDS sweeps

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
set ncfilename = /space/data/rainex/pr_composite/${dstring}.nc

#
# Find the NIDS files closest to this time
#

set flist = /tmp/nidscomposite.files.$$
rm -f $flist
touch $flist
chmod 666 $flist

foreach dir (JUA)
    @ delta = 0
    while ($delta <= 10)
	# try time + delta * 60
	@ dtime = $time + $delta * 60
	set dstring = `date -u -d "1970-01-01 + $dtime seconds" +%Y%m%d%H%M`
	set nids_Rfile = `find /space/data/NIDS/$dir -name "*${dstring}.0Z.nids"`
 	if ($#nids_Rfile) then
 	    echo $nids_Rfile >> $flist
 	    break
 	endif

	# try time - delta * 60
	@ dtime = $time - $delta * 60
	set dstring = `date -u -d "1970-01-01 + $dtime seconds" +%Y%m%d%H%M`
	set nids_Rfile = `find /space/data/NIDS/$dir -name "*${dstring}.0Z.nids"`
 	if ($#nids_Rfile) then
 	    echo $nids_Rfile >> $flist
 	    break
 	endif

	@ delta++
    end
end

/opt/src/radarcomposite/radarcomposite -o $ncfilename -d 0.02 -t $time `cat $flist`
rm -f $flist

/opt/zebra/bin/dsrescan -f $ncfilename:t pr_composite >& /dev/null