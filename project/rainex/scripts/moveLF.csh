#!/bin/csh
#
# Move NOAA LF radar files more than 1 day old to a different directory.
# This is useful to keep the LF directories a bit cleaner, and hence to
# make findP3Sweeps.csh run much faster.
#
foreach dir (/space/data/NOAA42_LF /space/data/NOAA43_LF)
    mkdir -p $dir/old
    #
    # Find non-directories which have not been modified in the
    # last 24 hours, and move them into the "old" subdirectory
    #
    find $dir -maxdepth 1 ! -type d -and ! -mtime 0 \
	-exec mv {} $dir/old \;
end
