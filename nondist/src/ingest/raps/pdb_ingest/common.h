/*
 * $Id: common.h,v 1.2 1992-07-22 14:58:20 granger Exp $
 *
 * The commonly included files for PDB handlers and ingestors
 * 'byte' symbol defined as unsigned char must be undef'ed here
 * since it is type-defined in Zeb's defs.h
 */

#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>	/* fd_set */

#include <tools_globals.h>
#undef byte
#include <err.h>
#include <font.h>
#include <mem.h>
#include <pjg.h>
#include <sock.h>
#include <str.h>

#include "ts.h"
#include "prod_structs.h"
#include "pdb.h"

#define DEG_TO_RAD 0.017453293
#define NMH_MS  1.9438445   /* Nautical Miles per hour per meters/sec */

