#!/bin/tcsh
#
# On occasion, the archiver will fail, with some Zebra components still
# running.  Then, the "new_zebra" parameter in Chris's startup file will
# not be set, and the archiver will not be restarted.
#
# Note that the archiver will not start if a file called /ops/run/arch.out
# exists.  This file must be removed before re-initiating archiver.  
# Additionally, a more rigorous test should be done to determine Archiver
# status.  Issue 'mstatus', and grep for Archiver.  If the archiver is
# running, mstatus should return a zero value (non-error).
#
#  (I'm not sure if the environment variables need to be set, but can't
#  see that they'd hurt anything.

if (! $?ZEBRA_MASTER) then
	echo "You must set the ZEBRA_MASTER environment variable!"
	exit 1
endif

if ( `hostname` == $ZEBRA_MASTER ) then
    setenv XAPPLRESDIR /zebra/lib/resources
    setenv ZEB_MESSAGE "message -internet"
    /zebra/bin/Archiver -f /dev/st0 -g +5-5 -time 60 \
		    -database /ops/run/ArcDB \
		    -tapelimit 3500000 \
		    -exclude spol >& /ops/run/arch.out &
    sleep 2
else
    echo "You need to run this on $ZEBRA_MASTER"
    exit 0
endif

exit
