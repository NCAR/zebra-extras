#! /usr/local/bin/perl5.001
#
# Perl script to extract the plot-time from a window's plot description
#

# Expect the plot description on stdin

$debug = 1;

%Months = ("jan", 1, "feb", 2, "mar", 3, "apr", 4, "may", 5,
	   "jun", 6, "jul", 7, "aug", 8, "sep", 9, "oct", 10,
	   "nov", 11, "dec", 12);

while (<>)
{
	last if (($date) = /^\s*plot-time\s*:\s*(\S+)\s*$/);
}

print "$date\n" if $debug;

# Now parse it for vitals like year, month, day and such

($day,$mo,$year,$hour,$min,$sec) =
    $date =~ /(\d+)-(\w+)-(\d+),(\d+):(\d+):(\d+)/;

$month = $Months{lc $mo};

printf "%02d%02d%02d.%02d\n", $year, $month, $day, $hour;


