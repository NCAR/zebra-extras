#! /localbin/perl5.001
#
# Rename image-cranker files into ofps format.  This means
# skip files whose name already has only two hour digits, and for the
# rest, parse the file name for the date and time, subtract 12 hours,
# and rename to the new time using only hour digits.
#
# WARNING: Doesn't account for leap years.

@Months = (31, 28, 31, 30, 31, 30, 31, 30, 31, 31, 30, 31);
$debug = 0;

if (@ARGV == 0)
{
    print "$0 file [file ...]\n";
    print "All files must have no path component, just the name.\n";
}

foreach $file (@ARGV)
{
    if ($file =~ /\//)
    {
	print "File $file has a path, files must be in current directory\n";
	exit(1);
    }
    next if $file =~ /^\d{6}\.\d{2}\.\w+\.gif$/;

    # Try to get a time from this file
    ($year,$month,$day,$hour,$plat) =
	($file =~ /^(\d\d)(\d\d)(\d\d)\.(\d\d)\d\d\d\d\.(\w+)\.gif$/);
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
    # Construct the new name
    $newname = sprintf ("%02d%02d%02d.%02d.%s.gif", $year, $month, $day, 
			$hour, $plat);
    print "Renaming $file: $newname\n" if $debug;
    $cmd = "mv $file $newname";
    print "$cmd\n" if $debug;
    system("$cmd") unless $debug;
}

