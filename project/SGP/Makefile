#
# $Id: Makefile,v 1.3 1993-12-10 03:58:55 caraher Exp $
#
# Author:
#  P.J. Caraher 1/26/93
#
# $Log: not supported by cvs2svn $
# Revision 1.2  1993/12/09  21:39:05  caraher
# Take out the line which executes "co RCS/*"
#
# Revision 1.1  1993/12/09  21:31:30  caraher
# Initial revision
#
#
 
MFVERSION = "$Id: Makefile,v 1.3 1993-12-10 03:58:55 caraher Exp $"

FILES = ButtonMaps ButtonMaps.defs ColorTables ColorTables.defs Display.all \
	DisplayConfigs IconBar dm.config dmstart ds.config gp.init msgstart \
	Display.skewt

install: 
	chmod ugo+x dmstart msgstart
	install -c $(FILES) $(BINHOME)/../SGP
	(cd pd; $(MAKE) $(MFLAGS) install)
	(cd modules; $(MAKE) $(MFLAGS) install)
	(cd colortables; $(MAKE) $(MFLAGS) install)
	(cd maps; $(MAKE) $(MFLAGS) install)

release: 
	chmod ugo+x dmstart msgstart
	install -c $(FILES) $(DISTBIN)/../SGP
	(cd pd; $(MAKE) $(MFLAGS) release)
	(cd modules; $(MAKE) $(MFLAGS) release)
	(cd colortables; $(MAKE) $(MFLAGS) release)
	(cd maps; $(MAKE) $(MFLAGS) release)

Makefile:
