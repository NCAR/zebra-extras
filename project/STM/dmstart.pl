#!/apps/cse/bin/perl
#
# Start a Zebra display for the ARESE project
#

$NChildren = 0;
@Children = ();
$SIG{'INT'} = 'handler';
$SIG{'QUIT'} = 'handler';
$SIG{'KILL'} = 'handler';
$SIG{'TERM'} = 'handler';
$PrintOnly = 0;
$Debug = 1;

print " in dmstart, argv[1] is $ARGV[0]\n";
chdir("$ENV{'CONFIGHOME'}");
#&System("source proj_env");
$dm = "$ENV{'ZEB_TOPDIR'}/bin/dm -multiple";
if (! $ENV{'ZEB_DM'}) 
{
    $ENV{'ZEB_DM'} = "xterm -T Display_Manager_for_$ARGV[0] -g 60x15+10+720" 
	. "-sb -bg grey90 -fg purple -fn 8x13bold -e $dm ";
}

$pid = &Child ("xterm -T Tmp_EventLogger -g 80x15+540+680 -sb -e " .
	       "tail -f $ENV{'ZEB_LOGFILE'} ");

&System ("$ENV{'ZEB_TOPDIR'}/bin/zstart -dm");

kill 9, $pid;
exit 0;

#
# Provide an easy way to fork processes and return the pid.
#
sub Child
{
    local ($cmd) = @_;
    if ($PrintOnly)
    {
	print "skipping child: $cmd\n";
	return (0);
    }
    $pid = fork;
    if ($pid == 0)
    {
	print "fork: ", $cmd, "\n" if $Debug;
	exec ($cmd) || die "exec child: $cmd";
	exit 0;
    }
    print "child $NChildren pid: $pid\n" if $Debug;
    $Children[$NChildren++] = $pid;
    return $pid;
}


sub handler
{
    local($sig) = @_;

    print "Monitor caught SIG$sig -- shutting down\n";
    print "Trying to kill $NChildren child processes: ", 
	  join(' ',@Children), "\n";
    kill 9, @Children if $NChildren;
    exit;
}
    
#
# Wait for our children.  If one exits we take it out of the
# children array.  If we get a signal to exit, the handler kills
# the children left in @children and exits.
#
sub Collect
{
    local ($pid, $i);
    
    print "Collecting $NChildren children...\n" if $Debug;
    while ($NChildren)
    {
	$pid = wait;
	print "Child $pid exited with code ", $?>>8, "\n" if $Debug;
	for ($i = 0; $i < $NChildren; ++$i)
	{
	    next if ($Children[$i] != $pid);
	    splice(@Children, $i, 1);
	    --$NChildren;
	    last;
	}
    }
}

sub System 
{
    $cmd = $_[0];
    if ($PrintOnly) {
	printf "skipping: %s\n", $cmd;
	return 0;
    }
    printf "system: %s\n", $cmd if $Debug;
    system ("$cmd");
}
