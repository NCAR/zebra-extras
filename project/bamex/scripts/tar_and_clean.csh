#!/bin/csh
#
# Tar and compress yesterday's files, and remove files from two days
# ago.  This should be run once a day via cron.
#
if (! $?ZEB_TOPDIR) then
    echo "ZEB_TOPDIR must be set!"
    exit 1
endif
 
setenv ZEB_PROJDIR $ZEB_TOPDIR/project/bamex
source $ZEB_PROJDIR/proj_env

cd $DATA_DIR

# yesterday and two days ago, in form YYYYMMDD
set yesterday = `date -u -d "yesterday 00:00 UTC" +%Y%m%d`
set twodaysago = `date -u -d "yesterday 00:00 UTC -24 hours" +%Y%m%d`

foreach source (radar_composite goes_1km goes_4km)
    cd $source
    #
    # tar up yesterday's files
    #
    tar cfz ${source}.${yesterday}.tgz ${yesterday}*nc
    #
    # remove files from two days ago (as long as we find the archive
    # for that day)
    #
    if (-f ${source}.${twodaysago}.tgz) then
	rm -f ${twodaysago}*nc
#	ssh syrah /code/burghart/zebra/bin/dsrescan $source
	$ZEB_TOPDIR/bin/dsrescan $source > /dev/null
    else
	echo "No $source archive exists for $twodaysago; not cleaning."
    endif
    cd ..
end
