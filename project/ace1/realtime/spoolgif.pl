#! /usr/local/bin/perl5.001
#
# spoolgif.pl <msgname> <window_name> <platform_key>
#
# Generate a gif image of the given window, and
# based on the platform key or name, decide how to name
# it and where to place it.  So far we recognize these
# platforms:
#
# iss1/rass915:
# iss1/prof915h: Use begin-time to name the file, placing it
#                in $data/iss1_gif.
# iss4/rass915:
# iss4/prof915h: Same as above, but place files in $data/iss4_gif
#
# BoM:           Same time as plot, placed in $data/traj_gif

$debug = 0;

# The data spooling directory
$data = "/data";

# Best make sure we got all our args and we recognize the platform 
if ($#ARGV != 2)
{
    print "Usage: $0 <msgname> <window_name> <platform>\n";
    exit 1;
}
($msgname, $window_name, $platkey) = @ARGV;

if ($platkey =~ /bom|BoM/)
{
    $plat = "BoM";
    $dir = "traj_gif";
}
elsif (($plat) = ($platkey =~ /iss4\/(\w+)/))
{
    $dir = "iss4_gif";
}
elsif (($plat) = ($platkey =~ /iss1\/(\w+)/))
{
    $dir = "iss1_gif";
}
else
{
    print "Unknown platform key $platkey.\n";
    exit 1;
}


# First thing we need to do is prepare to make the GIF image.  Get
# the plot time from the window with zquery.

%Months = ("jan", 1, "feb", 2, "mar", 3, "apr", 4, "may", 5,
	   "jun", 6, "jul", 7, "aug", 8, "sep", 9, "oct", 10,
	   "nov", 11, "dec", 12);

# Rename image-cranker files into ofps format.  This means
# skip files whose name already has only two hour digits, and for the
# rest, parse the file name for the date and time, subtract 12 hours,
# and rename to the new time using only hour digits.
#
# WARNING: Doesn't account for leap years.
#
sub subtime
{
    my @Months = (31, 28, 31, 30, 31, 30, 31, 30, 31, 31, 30, 31);
    my ($year, $month, $day, $hour) = @_;

    print "$year $month $day,$hour - 12:00 = " if $debug;
    $hour -= 12;
    if ($hour < 0) {
	$hour += 24;
	$day--;
	if ($day < 1) {
	    $month--;
	    if ($month < 1) {
		$month += 12;
		$year--;
	    }
	    $day += $Months[$month-1];
	}
    }
    print "$year $month $day,$hour\n" if $debug;
    return ($year, $month, $day, $hour);
}


open(QUERY, "zquery $msgname |") || die "could not zquery $msgname\n";

# We could put in a check here for the series-span, to find out how
# much we should subtract from the plot time to get the OFPS file time,
# but it doesn't seem that useful yet.

while (<QUERY>)
{
	last if (($date) = /^\s*plot-time\s*:\s*(\S+)\s*$/);
}
close(QUERY);
print "$date\n" if $debug;

# Now parse it for vitals like year, month, day and such
($day,$mo,$year,$hour,$min,$sec) =
    $date =~ /(\d+)-(\w+)-(\d+),(\d+):(\d+):(\d+)/;

$month = $Months{lc $mo};
$year %= 100;

printf "%02d%02d%02d.%02d\n", $year, $month, $day, $hour if $debug;

# Subtract 12 hours if this is an iss platform
if ($platkey =~ /iss(1|4)/)
{
    ($year, $month, $day, $hour) = &subtime($year, $month, $day, $hour);
}

# Construct the new name
$filename = sprintf ("%02d%02d%02d.%02d.%s.gif", $year, $month, $day, 
		     $hour, $plat);

print "Filename: $filename\n" if $debug;

# Now just do the dump

$temp = "/tmp/image.$$.pnm";
$cmd = "xwd -name $window_name | xwdtopnm > $temp";
print "$cmd\n";
system ("$cmd") unless $debug;
$cmd = "convert $temp $data/$dir/$filename";
print "$cmd\n";
system ("$cmd") unless $debug;
system ("rm -f $temp") unless $debug;

