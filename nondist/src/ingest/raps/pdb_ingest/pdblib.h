/*
 * $Id: pdblib.h,v 1.1 1992-07-22 14:57:39 granger Exp $
 *
 * pdblib.h --- External references, prototypes and macros for
 *		use with the pdblib package
 */

#ifndef _pdblib_h_
#define _pdblib_h_

#include "pdb.h"

/* Prototype macro:
 * If using this header file with defs.h, include "defs.h" FIRST
 * This is so that non-Zeb applications don't require defs.h
 */
# ifndef FP
# ifdef __STDC__
#  define FP(stuff) stuff
# else
#  define FP(stuff) ()
# endif
# endif

/* For general convenience: */
#define PDBPT_PENUP ((short)(0x7fff))


/*----------------------------------------------------
 * Record for storing PDB messages in a file
 * This is the header which will precede each message 
 */

typedef struct _PdbPacketHdr {
	int mtype;
	int mlen;
	int pdb_format;		/* RAW, POLYLINE, etc... */
	time_t timestamp;	/* time() when message stored */
} PdbPacketHdr;

#define PDBGET_TEST_MSG 999	/* Internal test message:
				 * The message is just a string */

/*----------------------------------------------------*/

typedef int (*PdbMsgHandler)(/* int mtype, char *msg, int mlen */);

extern int	SearchForResponse FP((int wait_sec, int mtype, 
				char **message, int *mess_len));
extern int	PdbVerifyResponse FP((int timeout, int mtype,
				  PdbMsgHandler handler));
extern void	DumpPolylineProduct FP((FILE *file, PDBproduct *pd));
extern int	PdbResponse FP((int wait_sec, int *mtype, 
				char **message, int *mess_len));
extern int	PdbSimulatedResponse FP((int wait_sec, int *mtype, 
				char **message, int *mess_len));
extern int	PdbRequest FP((int type, char *request, int len));
extern int	PdbInitAll FP((char *appl_name, int key));
extern void	PdbUsage();
extern void	PdbParseOptions FP((int *argc, char *argv[],
				   void (*usage)(/* char *prog */)));

/*===================================================================*/
/* Global flags which control debugging and simulation capabilities  */

extern short PdbFlagDumpPoints;
				/* Show points when dumping pl prods */
extern short PdbFlagSimulate;
				/* Take input from Simulate FD in
 				 * pdbget format rather than connecting
 				 * to PDBS */
extern short PdbSimulateFD;	/* File descriptor to receive simulated
 				 * input from, default is stdin */
extern short PdbFlagSimulateRealtime;
 				/* Mimic timing of simulated input msgs */
extern short PdbFlagEchoRequests;	
				/* Log debug info to ERR for requests */
extern short PdbFlagEchoResponses;	
				/* Log debug info to ERR for responses */
 
extern short PdbFlagStoreResponses;
				/* Write all responses to StoreFD for
				 * later use as simulated responses */
extern short PdbStoreFD;	/* default FD is stdout */

/*===================================================================*/

#endif /* _pdblib_h_ */
