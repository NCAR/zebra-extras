#
# $Id: Makefile,v 1.3 1993-12-10 04:04:49 caraher Exp $
#
# Author:
#  P.J. Caraher 1/26/93
#
# $Log: not supported by cvs2svn $
# Revision 1.2  1993/12/09  21:40:02  caraher
# Take out the line that executes "co RCS/*"
#
# Revision 1.1  1993/12/09  21:32:18  caraher
# Initial revision
#
#
 
MFVERSION = "$Id: Makefile,v 1.3 1993-12-10 04:04:49 caraher Exp $"

FILES = defaults.pd instrument.pd instrument2.pd instrument3.pd instrument4.pd \
	instrument5.pd kansas.pd mesonet.pd ok_bounds.pd ok_ops-ctr.pd ok_sonde1.pd \
	oklahoma_rivers.pd skewt.pd state_lines.pd whiteclock.pd sounding.pd

install: 
	install -c $(FILES) $(BINHOME)/../SGP/pd

release: 
	install -c $(FILES) $(DISTBIN)/../SGP/pd

Makefile:


