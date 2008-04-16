#
# $Id: Makefile,v 1.4 2008-04-16 18:26:55 granger Exp $
#
# Author:
#  P.J. Caraher 1/26/93
#
 
MFVERSION = "$Id: Makefile,v 1.4 2008-04-16 18:26:55 granger Exp $"

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
