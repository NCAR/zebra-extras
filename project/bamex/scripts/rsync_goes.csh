#!/bin/csh
#
# sync BAMEX GOES files from Boulder
#
foreach plat (goes_1km goes_4km)
    rsync --delete --exclude="*tgz" -rtze ssh /scr/js1/bamex/$plat/ \
	bmxgate.cust.qwest.net:/data/bamex/$plat
end
