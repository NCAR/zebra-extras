#
# $Id: Makefile,v 1.1 1993-12-09 21:32:18 caraher Exp $
#
# Author:
#  P.J. Caraher 1/26/93
#
# $Log: not supported by cvs2svn $
#
 
MFVERSION = "$Id: Makefile,v 1.1 1993-12-09 21:32:18 caraher Exp $"

FILES = defaults.pd instrument.pd instrument2.pd instrument3.pd instrument4.pd \
	instrument5.pd kansas.pd mesonet.pd ok_bounds.pd ok_ops-ctr.pd ok_sonde1.pd \
	oklahoma_rivers.pd skewt.pd state_lines.pd whiteclock.pd

install: 
	/apps/rcs/bin/co $(FILES)
	install -c $(FILES) $(BINHOME)/../SGP/pd

release: 
	/apps/rcs/bin/co $(FILES)
	install -c $(FILES) $(DISTBIN)/../SGP/pd

Makefile:


