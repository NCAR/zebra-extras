/*		Copyright (C) 1987,88,89,90,91 by UCAR
 *	University Corporation for Atmospheric Research
 *		   All rights reserved
 *
 * No part of this work covered by the copyrights herein may be reproduced
 * or used in any form or by any means -- graphic, electronic, or mechanical,
 * including photocopying, recording, taping, or information storage and
 * retrieval systems -- without permission of the copyright owner.
 * 
 * This software and any accompanying written materials are provided "as is"
 * without warranty of any kind.  UCAR expressly disclaims all warranties of
 * any kind, either express or implied, including but not limited to the
 * implied warranties of merchantibility and fitness for a particular purpose.
 * UCAR does not indemnify any infringement of copyright, patent, or trademark
 * through use or modification of this software.  UCAR does not provide 
 * maintenance or updates for its software.
 */
MFVERSION="$Id: Makefile.cpp,v 1.2 1991-09-26 18:03:58 gracio Exp $"

# ifdef sun
/*
 * Sun options
 */
CC=gcc
CFLAGS=  -g -O -I$(ZEBHOME)/fcc/include -I$(ZEBHOME)/rdss/include
LIBS=$(ZEBHOME)/fcc/lib/libfcc.a -lnetcdf -lrdss -lXaw -lXt -lXmu -lXext -lX11 -ltermcap -lm
# endif


BINDIR=../../bin
LIBDIR=../../lib
HDIR=../../include

OBJS = llp_ingest.o

all:	llp_ingest llp_ingest.lf

install:	llp_ingest llp_ingest.lf
	install -c llp_ingest $(BINDIR)
	install -c -m 0444 llp_ingest.lf $(LIBDIR)

include:

llp_ingest:	$(OBJS)
	$(CC) $(CFLAGS) -o llp_ingest $(OBJS) $(LIBS)

llp_ingest.lf: llp_ingest.state
	uic < make-lf

clean:
	rm -f *~ llp_ingest *.o Makefile.bak

Makefile: mf

mf:
	mv Makefile Makefile~
	cp Makefile.cpp Makefile.c
	echo "# DO NOT EDIT -- EDIT Makefile.cpp INSTEAD" > Makefile
	cc -E Makefile.c >> Makefile
	rm -f Makefile.c
	make depend

depend:
	makedepend $(CFLAGS) *.c
