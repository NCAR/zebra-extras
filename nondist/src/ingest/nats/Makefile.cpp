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

# include "../../include/config.h"

# ifdef sun
/*
 * Sun options
 */
CC=CCompiler
CFLAGS=CCOptions -I$(FCCINC) -I$(RDSSINC)
LIBS= ZebLibrary -lnetcdf -lrdss -lXaw -lXmu -lXt -lXext -lX11 -ltermcap -lm
# endif


OBJS = nats_ingest.o

all:	nats_ingest nats

install:	nats_ingest
	install -c nats_ingest D_BINDIR

include:

nats_ingest:	$(OBJS)
	$(CC) $(CFLAGS) -o nats_ingest $(OBJS) $(LIBS)

nats:	nats.o
	$(CC) $(CFLAGS) -o nats nats.o $(LIBS)

clean:
	rm -f *~ nats_ingest *.o Makefile.bak nats

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
