#
# $Id: Makefile,v 1.1 1993-12-10 01:18:28 caraher Exp $
#
# Author:
#  P.J. Caraher 1/26/93
#
# Description:
#  Makefile for the SGP modules for gp, ./modules
#
# $Log: not supported by cvs2svn $
#
#
 
MFVERSION = "$Id: Makefile,v 1.1 1993-12-10 01:18:28 caraher Exp $"

FILES = color-table contour ctr-step form-color grid interp irg_winds \
	location offsets radar range raster rings simple-color \
	simple-track skewt station track xytrace xywind

install: 
	install -c $(FILES) $(BINHOME)/../SGP/modules

release: 
	install -c $(FILES) $(DISTBIN)/../SGP/modules

Makefile:
