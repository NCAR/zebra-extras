#
# $Id: Makefile,v 1.1 1993-12-10 01:17:14 caraher Exp $
#
# Author:
#  P.J. Caraher 1/26/93
#
# Description:
#  Makefile for the SGP color tables directory ./colortables
#
# $Log: not supported by cvs2svn $
#
#
 
MFVERSION = "$Id: Makefile,v 1.1 1993-12-10 01:17:14 caraher Exp $"

FILES = 16gray Rainbow cook16 hotcold smallrevgray AllWhite RevRainbow \
	cook32 newgray tseries Contour Skew-t fivegray refcontour wild \
	ContourMono SmallRainbow fivegrey revgray wild_new Winds gcook16 \
	scook xygraph

install: 
	install -c $(FILES) $(BINHOME)/../SGP/colortables

release: 
	install -c $(FILES) $(DISTBIN)/../SGP/colortables

Makefile:
