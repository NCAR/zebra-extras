#
# $Id: Makefile,v 1.1 1993-12-09 21:31:30 caraher Exp $
#
# Author:
#  P.J. Caraher 1/26/93
#
# $Log: not supported by cvs2svn $
#
 
MFVERSION = "$Id: Makefile,v 1.1 1993-12-09 21:31:30 caraher Exp $"

FILES = ButtonMaps ButtonMaps.defs ColorTables ColorTables.defs Display.all \
	DisplayConfigs IconBar dm.config dmstart ds.config gp.init msgstart

install: 
	/apps/rcs/bin/co $(FILES)
	chmod ugo+x dmstart msgstart
	install -c $(FILES) $(BINHOME)/../SGP
	(cd pd; make install)

release: 
	/apps/rcs/bin/co $(FILES)
	chmod ugo+x dmstart msgstart
	install -c $(FILES) $(DISTBIN)/../SGP
	(cd pd; make release)

Makefile:


