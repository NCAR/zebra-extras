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
MFVERSION="$Id: Makefile.cpp,v 1.4 1992-04-30 15:43:26 granger Exp $"

# include "../../include/config.h"

# ifdef sun
/*
 * Sun options
 */
CC=CCompiler
CFLAGS= CCOptions IncludeDirs
LIBS=ZebLibrary CDFLibrary MiscLibs
XLIBS=XLibraries
# endif

OBJS = llp_ingest.o

all:	llp_ingest llp_ingest.lf

install:	llp_ingest llp_ingest.lf
	install -c llp_ingest D_BINDIR
	install -c -m 0444 llp_ingest.lf D_LIBDIR

include:

llp_ingest:	$(OBJS)
	$(CC) $(CFLAGS) -o llp_ingest $(OBJS) $(LIBS) $(XLIBS)

llp_ingest.lf: llp_ingest.state
	uic < make-lf

clean:
	rm -f *~ llp_ingest *.o Makefile.bak llp_ingest.lf

Makefile: mf

mf:
	mv Makefile Makefile~
	cp Makefile.cpp Makefile.c
	echo "# DO NOT EDIT -- EDIT Makefile.cpp INSTEAD" > Makefile
	cc -E -DMAKING_MAKEFILE Makefile.c | cat -s >> Makefile
	rm -f Makefile.c
	make depend

depend:
	makedepend $(CFLAGS) *.c
