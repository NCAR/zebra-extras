/*
 * $Id: pdblib.c,v 1.1 1992-07-03 18:23:35 granger Exp $
 *
 * pdblib.c --- Some useful routines for interfacing with RAP's
 *		products server.  In particular, it provides
 *		shell routines which replace the PDB public
 *		routines.  These routines can vary debugging
 *		info using flags which the application sets,
 *		and they can even be used to store PDB messages
 *		and use them later to simulate server responses
 */

#if !defined(SABER) && !defined(lint)
static char rcsid[] = "$Id: pdblib.c,v 1.1 1992-07-03 18:23:35 granger Exp $";
#endif

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <values.h>
#include <time.h>

#include "str.h"
#include "err.h"
#include "sock.h"

#include "pdb.h"
#include "prod_structs.h"
#include "pdblib.h"

/* Prototype macro */
# ifndef FP
# ifdef __STDC__
#  define FP(stuff) stuff
# else
#  define FP(stuff) ()
# endif
# endif

#ifndef streq
#   define streq(s1,s2) (strcmp(s1,s2) == 0)
#endif


/*----------------------------------------------------*/

int	PdbVerifyResponse FP((int timeout, int mtype,
			      PdbMsgHandler handler));
int	SearchForResponse FP((int wait_sec, int mtype, 
				char **message, int *mess_len));
void	DumpPolylineProduct FP((FILE *file, PDBproduct *pd));
void 	DumpProductsList FP((FILE *out, char *msg, int mlen));
int	PdbResponse FP((int wait_sec, int *mtype, 
				char **message, int *mess_len));
int	PdbSimulatedResponse FP((int wait_sec, int *mtype, 
				char **message, int *mess_len));
int	PdbRequest FP((int type, char *request, int len));
int	PdbInitAll FP((char *appl_name, int key));
void	PdbUsage ();
void	PdbParseOptions FP((int *argc, char *argv[], 
			   void (*usage)(/* char *prog */)));


/*===================================================================*/
/* Global flags which control debugging and simulation capabilities  */

short PdbFlagDumpPoints = 0;	/* Show points when dumping pl prods */
short PdbFlagSimulate = 0;	/* Take input from Simulate FD in
				 * pdbget format rather than connecting
				 * to PDBS */
short PdbSimulateFD = 0;	/* File descriptor to receive simulated
				 * input from, default is stdin */
int PdbSimulateDelay = 3;	/* Delay (secs) bet simulated resp's*/
short PdbFlagSimulateRealtime = 1;
				/* Mimic timing of simulated input */
short PdbFlagEchoRequests = 0;	/* Log debug info to ERR for requests */
short PdbFlagEchoResponses = 0;	/* Log debug info to ERR for responses */

short PdbFlagStoreResponses = 0;/* Write all responses to StoreFD for
				 * later use as simulated responses */
short PdbStoreFD = 1;		/* default FD is stdout */
char *PdbServiceFile = NULL;	/* If not set from the command line,
				 * use ALG_SERVICES.  If not there,
				 * use "rap_services" */

/*===================================================================*/

/*
 * Private prototype
 */
void PdbStoreMessage FP((int fd, int mtype, char *msg, int mlen));


void
DumpProductsList(out, msg, mlen)
	FILE *out;
	char *msg;
	int mlen;
{
	char *ptr;

	for (ptr = msg; ptr < msg+mlen; ptr += strlen(ptr)+1)
		fprintf(out, "%s\n", ptr);
}


/*
 * Mimics the PDBI function PDBinitAll, but allows the user to
 * specify the services file on the command line rather than through
 * the ALG_SERVICES envariable. "rap_services" is the default if not
 * named on cmd-line or in ALG_SERVICES
 */
int
PdbInitReplaced(applic_name, key)
	char *applic_name;
	int key;
/*
 * Replaces PDBinitAll, but does not require ALG_SERVICES to be set
 */
{
	char *getenv FP((char *name));

	if (!PdbServiceFile)
	   if (!(PdbServiceFile = getenv("ALG_SERVICES")))
	      PdbServiceFile = "rap_services";
	ERRprintf(ERR_INFO, "Using services file: %s", PdbServiceFile);

	if (1 != SOCKinit( applic_name, key, /*max_servers*/ 3, 
					     /*max_clients*/ -1))
	{
	   ERRprintf(ERR_PROGRAM, "SOCKinit(%s, %i, 0, 1) failed",
		applic_name, key);
	   return (0);
	}
	if (1 != SOCKsetServiceFile( SOCK_FILE, PdbServiceFile))
	   return (0);
	if (!PDBinitProductServer(applic_name))
	{
	   ERRprintf(ERR_PROGRAM,"PDBinitProductServer() failed");
  	   return (0);
	}
	if (1 != SOCKbeginOperations(1))
	{
	   ERRprintf(ERR_PROGRAM,"SOCKbeginOperations() failed");
	   return (0);
	}
	if (1 != PdbRequestString(PDB_CONNECT, applic_name))
	{
	   ERRprintf(ERR_PROGRAM,"PDB_CONNECT request failed");
	   return (0);
	}

	return (1);
}




int
PdbInitAll(appl, key)
	char *appl;
	int key;
/*
 * If just simulating the server, don't actually call PDBinitAll
 * Rather than using PDBI's PDBinitAll, we'll mimic it here and
 * use an option rather than ALG_SERVICES environ variable to
 * set our socket services file
 */
{
	if (PdbFlagSimulate)
	{
		ERRprintf(ERR_DEBUG,"Using simulated server reponses...");
		return(1);
	}
	else
	{
		return(PdbInitReplaced(appl, key));
	}
}


int
PdbRequestString(type, request)
	int type;
	char *request;
/*
 * If just simulating, return successful unconditionally
 * If echo flag on, log some debug info to ERR,
 * including the string that was being sent.
 */
{
	int ret;
	int len;

	len = strlen(request)+1;
	if (PdbFlagEchoRequests)
		ERRprintf(ERR_DEBUG,
		   "PdbRequest(type %d, \"%s\", len %d)", 
				type, request, len);
	if (!PdbFlagSimulate)
	{
		ret = PDBrequest(type, request, len);
		if (PdbFlagEchoRequests && (ret != 1))
			ERRprintf(ERR_DEBUG,
			   "PdbRequest() returned %d",ret);
	}
	else
		ret = 1;
}


int
PdbRequest(type, request, len)
	int type;
	char *request;
	int len;
/*
 * If just simulating, return successful unconditionally
 * If echo flag on, log some debug info to ERR
 */
{
	int ret;

	if (PdbFlagEchoRequests)
		ERRprintf(ERR_DEBUG,
		   "PdbRequest(type %d, len %d)", type, len);
	if (!PdbFlagSimulate)
	{
		ret = PDBrequest(type, request, len);
		if (PdbFlagEchoRequests && (ret != 1))
			ERRprintf(ERR_DEBUG,
			   "PdbRequest() returned %d",ret);
	}
	else
		ret = 1;
}


int
PdbResponse(wait_sec, type, msg, mlen)
	int wait_sec;
	int *type;
	char **msg;
	int *mlen;
/*
 * Basically, test global flags to determine how much info
 * is logged and if simulating, use pdbget-format source
 * instead of a real call to PDBresponse.  Doesn't log
 * info about simple polls which return with 0
 */
{
	int ret;

	if (wait_sec && PdbFlagEchoResponses)
		ERRprintf(ERR_DEBUG, "PdbResponse(wait %d)", wait_sec);
	if (!PdbFlagSimulate)
		ret = PDBresponse(wait_sec, type, msg, mlen);
	else
		ret = PdbSimulatedResponse(wait_sec, type, msg, mlen);
	if (PdbFlagEchoResponses)
	{
		if (ret == 1)
		   ERRprintf(ERR_DEBUG, 
		      "PdbResponse received mtype %d, mlen %d",
		      *type, *mlen);
		else if (wait_sec)
		   ERRprintf(ERR_DEBUG, 
		      "PdbResponsse returning %d",ret);
	}
	if (ret == 1 && PdbFlagStoreResponses)
		PdbStoreMessage(PdbStoreFD, *type, *msg, *mlen);
	return(ret);
}



time_t 
Time( tloc )
	time_t *tloc;
/*
 * Replicates a call to time() which can't be used because of
 * the 'time' typedef in Zeb's defs.h
 */
{
	struct timeval tp;

	gettimeofday(&tp, NULL);
	if (tloc)
		*tloc = tp.tv_sec;
	return(tp.tv_sec);
}
	


int
PdbSimulatedResponse(wait_sec, type, msg, mlen)
	int wait_sec;
	int *type;
	char **msg;
	int *mlen;
/*
 * Read PdbPacketHeaders and subsequent PDB messages from
 * the file descriptor until eof found.  If simulating real-time
 * each message is held until the appropriate amount of time
 * has passed since the preceding message (according to the 
 * differences in their timestamps).
 */
{
	register long now;
	int fd = PdbSimulateFD;
	static PdbPacketHdr hd;
	static char buf[1024];
	static time_t next = 0;
	static time_t laststamp = 0;
	static short eof = 0;

	if (eof) 
	{
		if (wait_sec) sleep(wait_sec);
		return(0);
	}

	now = Time(0);

	if (!next)
	{
	   if (read(fd, (char *)&hd, sizeof(hd)) > 0)
	   {
		if (!laststamp) 
			laststamp = hd.timestamp;
		if (!PdbFlagSimulateRealtime)
		   next = now + PdbSimulateDelay;
		else
		   next = now + (hd.timestamp - laststamp);
		laststamp = hd.timestamp;
	   }
	   else /* that's the end of our input */
	   {
		eof = 1;
		ERRprintf(ERR_PROGRAM, "End of simulated input");
	   }
	}

	if (!next)
	{
		if (wait_sec)
			sleep(wait_sec);
		return(0);  /* nothing to receive */
	}
	else if (next <= now+wait_sec)
	{
	/*
	 * We have a header, and its time to 'receive' it 
	 */
		if (next > now)
			sleep(next - now);
		next = 0;

		read(fd, buf, hd.mlen);
		*type = hd.mtype;
		*msg = buf;
		*mlen = hd.mlen;
		return(1);
	}
	else /* have to hold on to this header and timeout */
	{
		if (wait_sec)
			sleep(wait_sec);
		return(0);
	}
}



/*
 * Build a PdbPacketHdr for this 'msg' and write both the
 * header and the msg to the given file descriptor
 */
void
PdbStoreMessage(fd, mtype, msg, mlen)
	int fd;
	int mtype;
	char *msg;
	int mlen;
{
	PdbPacketHdr hd;

	hd.mtype = mtype;
	hd.mlen = mlen;
	hd.pdb_format = 0;  /* !!! ignored --- will not be valid !!! */
	hd.timestamp = Time(0);

	write(fd, (char *)&hd, sizeof(hd));
	write(fd, msg, mlen);
}



/* SearchForResponse 
 * ----------------
 * Wait for a specific message type given by the 'type' argument.
 * If a call to PDBresponse times out in the wait_sec's given,
 * 0 will be returned, else a 1 is returned and the message and
 * mess_len arguments are filled in
 */
int
SearchForResponse(wait_sec, type, message, mess_len)
	int wait_sec;
	int type;
	char **message;
	int *mess_len;
{
	int received_type;  /* The message type returned by PDBresponse */
	int ret;	    /* Value returned by PDBresponse */

	/*
	 * Keep calling PDBresponse until it times out or
	 * the correct response is received
	 */
	do {
		ret = PdbResponse(wait_sec, &received_type,
				  message, mess_len);
		/* 
		 * For now, flag unexpected messages 
		 */
		if (received_type != type)
		{
		   printf("Unexpected PDBS message %d",received_type);
		   printf("    ...while expecting %d\n",type);
		}
	}
	while ((ret == 1) && (received_type != type));

	return(ret);
}


int
PdbVerifyResponse(wait_sec, mtype, handler)
	int wait_sec;
	int mtype;
	PdbMsgHandler handler;
/*
 * Wait for a response of type 'mtype' with a timeout of 'wait_sec'
 * All messages received are sent to the 'handler'
 * If the response is verified, return 1, else return 0
 */
{
	int received_type;
	int ret;
	char *msg;
	int mlen;

	if (PdbFlagSimulate) /* if simulating, assume response is verified */
		return(1);

	/*
	 * Keep calling PDBresponse until it times out or
	 * the correct response is received
	 */
	do {
		ret = PdbResponse(wait_sec, &received_type,
				  &msg, &mlen);
		/*
		 * If a message has been received, send it to the
		 * handler
		 */
		if (ret != 1)   	/* Response timed out */
			return(0);
		if (handler)
			(*handler)(received_type, msg, mlen);
	}
	while (received_type != mtype);

	return(1);
}



void
DumpPolylineProduct(file, pd)
   PDBproduct *pd;
   FILE       *file;
{
   int         j,
               k;
   int         npoints;
   int         x,
               y;
   double      xloc[1024],
               yloc[1024];
   PDBpt      *pt;

   if (pd->text_size > 0)
   {				/* Display labels to product */
      char       *str,
                 *txt,
                 *etxt;

      txt = pd->text;
      etxt = txt + pd->text_size;

      while (((str = PDBnextText(&txt, &x, &y)) != NULL) && (txt < etxt))
      {
	 xloc[0] = x / 100.0;	/* Current PDB scaling for coords (1/92) */
	 yloc[0] = y / 100.0;
	 fprintf(file, "Text Position  %g,%g: %s\n", xloc[0], yloc[0], str);
      }
   }


   if (pd->npts > 0)
   {				/* Display lines to product */
      fprintf(file, "Product has %d points\n", pd->npts);
      pt = pd->pts;
      npoints = 0;
      for (j = 0; j < pd->npts; j++)
      {
	 if (pt[j].x != 0x7fff)
	 {
	    xloc[npoints] = pt[j].x / 100.0;
	    yloc[npoints] = pt[j].y / 100.0;
	    npoints++;
	 }
	 else
	 {			/* End of segment indicator */
	    if (PdbFlagDumpPoints)
	       fprintf(file, "Line Segment End\n");
	    for (k = 0; k < npoints; k++)
	    {
	       if (PdbFlagDumpPoints)
		  fprintf(file, "\t %.2f, %.2f", xloc[k], yloc[k]);
	    }
	    if (PdbFlagDumpPoints)
	       fprintf(file, "\n");
	 }
      }

      if (npoints > 0)
      {				/* Plot any remaining points in segment
				 * without a terminator */
	 if (PdbFlagDumpPoints)
	    fprintf(file, "Line Segment End\n");
	 for (k = 0; k < npoints; k++)
	 {
	    if (PdbFlagDumpPoints)
	       fprintf(file, "\t %.2f, %.2f", xloc[k], yloc[k]);
	 }
	 if (PdbFlagDumpPoints)
	    fprintf(file, "\n");
      }
   }
}




void
PdbParseOptions(argc, argv, Usage)
	int *argc;
	char *argv[];
	void (*Usage)(/* char *prog */);
{
	int i,j;
	int nargs;
	int fd;

	i = 1;
	while (i < *argc)
	{
		nargs = 1;
		if (streq(argv[i],"-simulate") ||
		    streq(argv[i],"-sim"))
		{
		   nargs = 2;
		   if (i+1 < *argc)
		   {
		   	if (streq(argv[i+1],"-"))
			{
		       		PdbFlagSimulate = 1;
				PdbSimulateFD = 0;    /* stdin */
			}
			else if ((fd = open(argv[i+1],O_RDONLY)) != -1)
			{
				PdbSimulateFD = fd;
			   	PdbFlagSimulate = 1;
			}
			else
			{
				perror(argv[i+1]);
				exit(1);
			}
		   }
		   else
		   {
			fprintf(stderr,"-simulate option needs file name");
			Usage(argv[0]);
			exit(1);
		   }
		}
		else if (streq(argv[i],"-record") ||
			 streq(argv[i],"-rec"))
		{
		   nargs = 2;
		   if (i+1 < *argc)
		   {
		   	if (streq(argv[i+1],"-"))
			{
		   		PdbFlagStoreResponses = 1;
				PdbStoreFD = 1;	/* stdout */
			}
			else if ((fd = open(argv[i+1],
				      O_WRONLY | O_CREAT,
				      0666)) != -1)
			{
		   		PdbFlagStoreResponses = 1;
		   		PdbStoreFD = fd;
			}
			else
			{
				perror(argv[i+1]);
				exit(1);
			}
		   }
		   else
		   {
			fprintf(stderr,"-record option needs file name");
			Usage(argv[0]);
			exit(1);
		   }
		}
		else if (streq(argv[i],"-responses") ||
			 streq(argv[i],"-resp"))
		{
		   PdbFlagEchoResponses = 1;
		}
		else if (streq(argv[i],"-requests") ||
			 streq(argv[i],"-req"))
		{
		   PdbFlagEchoRequests = 1;
		}
		else if (streq(argv[i],"-svc"))
		{
		   nargs = 2;
		   PdbServiceFile = argv[i+1];
		}
		else if (streq(argv[i],"-fast"))
		{
		   PdbFlagSimulateRealtime = 0;
		   PdbSimulateDelay = 0;
		}
		else if (streq(argv[i],"-delay"))
		{
		   nargs = 2;
		   PdbSimulateDelay = atoi(argv[i+1]);
		   PdbFlagSimulateRealtime = 0;
		}
		else if (streq(argv[i],"-pts"))
		{
		   PdbFlagDumpPoints = 0;
		}
		else
		{
		   i++;
		   continue;
		};

		(*argc) -= nargs;
		for (j = i; j < *argc; ++j)
			argv[j] = argv[j + nargs];
	}

}


void
PdbUsage()
{
   fprintf(stderr,"Pdb Options: \n");
   fprintf(stderr,
      "-svc <file>		Set RAP network services filename\n");
   fprintf(stderr,
      "-simulate, -sim <file>	Source file for server reponses\n");
   fprintf(stderr,
      "-record, -rec <file>	Store messages\n");
   fprintf(stderr,
      "-responses, -resp	Echo PDB responses\n");
   fprintf(stderr,
      "-requests, -req		Echo PDB requests\n");
   fprintf(stderr,
      "-fast			Don't simulate 'real time' delays\n");
   fprintf(stderr,
      "-delay			Set delay (secs) between simulated responses\n");
   fprintf(stderr,
      "-pts			Show points in polyline products\n");
}

   
