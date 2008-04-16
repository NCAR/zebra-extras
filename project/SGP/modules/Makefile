#
# $Id: Makefile,v 1.2 2008-04-16 18:26:54 granger Exp $
#
# Author:
#  P.J. Caraher 1/26/93
#
# Description:
#  Makefile for the SGP modules for gp, ./modules
#
 
MFVERSION = "$Id: Makefile,v 1.2 2008-04-16 18:26:54 granger Exp $"

FILES = color-table contour ctr-step form-color grid interp irg_winds \
	location offsets radar range raster rings simple-color \
	simple-track skewt station track xytrace xywind

install: 
	install -c $(FILES) $(BINHOME)/../SGP/modules

release: 
	install -c $(FILES) $(DISTBIN)/../SGP/modules

Makefile:
