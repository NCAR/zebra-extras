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
MFVERSION="$Id: Makefile.cpp,v 1.2 1993-03-24 22:56:50 granger Exp $"
MFDESC="$Desc: Makefile.cpp for RAP data products and storm tracks ingestors$"

# include "../../../include/config.h"

# ifdef sun
/*
 * Sun options
 */
CC = CCompiler
/* 
 * DO NOT COMPILE WITH -O WITH GCC!!!  THERE IS A BUG IN GCC -O WHICH 
 * WHICH CRASHES PDB_INGEST!!!
 */
CCOPTIONS = -g
CFLAGS = $(CCOPTIONS) IncludeDirs -I../include
LIBTOOLS = ../lib/libtools.a
LIBTOOL2 = ../lib/libtool2.a
LIBS = $(LIBTOOLS) $(LIBTOOL2) ZebLibrary MiscLibs XLibraries CDFLibrary
# endif

LIBC_MP = /usr/local/src/mprof/libc_mp.a

GCCLIB = "/usr/local/lib/gcc-lib/sparc-sun-sunos4.1.2/2.2/libgcc.a"

TESTOBJS = get_x_def.o free.o str_parse.o 

OBJS = ingest.o pdblib.o pdbutils.o handlers.o err2el.o

TSOBJS = tstracks.o ingest.o pdbutils.o shapes.o err2el.o ts.o

PRGS = pdbget pdbecho pdbtest pdb_ingest pdbping

SRCS = $(OBJS:.o=.c)

all:	pdb_ingest

install:	

include:

pdbget: pdbget.o $(LIBTOOLS)
	$(CC) $(CFLAGS) -o $@ pdbget.o $(LIBS)

pdbecho: pdbecho.o $(LIBTOOLS)
	$(CC) $(CFLAGS) -o $@ pdbecho.o $(LIBS)

pdbping: pdbping.o $(LIBTOOLS)
	$(CC) $(CFLAGS) -o $@ pdbping.o $(LIBS)

pdbtest: pdbtest.o $(TESTOBJS) $(LIBTOOLS)
	$(CC) $(CFLAGS) -o $@ pdbtest.o $(TESTOBJS) $(LIBS)

pdb_ingest:  pdb_ingest.o $(OBJS) $(LIBTOOLS)
	$(CC) $(CFLAGS) -o $@ $@.o $(OBJS) $(LIBS)

tstracks:  $(TSOBJS) $(LIBTOOLS) $(LIBTOOL2)
	$(CC) $(CFLAGS) -o $@ $(TSOBJS) $(LIBS)

clean::
	rm -f tstracks

ccenter_tstracks:  $(TSOBJS) $(LIBTOOLS) $(LIBTOOL2)
	#setopt ansi
	#load $(CFLAGS) $(TSOBJS:.o=.c) -G -Bstatic $(LIBS) $(GCCLIB) 
	#link

ccenter: $(OBJS)
	#setopt ansi
	#load $(CFLAGS) pdb_ingest.c $(SRCS) -G -Bstatic $(LIBS) $(GCCLIB) 
	#link

clean::
	rm -f $(PRGS)
	rm -f *~ *.o Makefile.bak

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
