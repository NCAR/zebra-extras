/*
 * $Id: pdb_ingest.c,v 1.2 1993-03-24 22:59:29 granger Exp $
 *
 * Ingest data products from RAP's Product Database Server, PDBS
 *
 * Type 'pdb_ingest -help' for usage info
 *
 * Uses RAP's PDBI, ERR, and SOCK packages.  ERR messages are mapped to
 * the EventLogger via err2el.c, and the PDBI interface is hidden under
 * pdblib.c convenience functions.
 *
 * The routines which actually ingest products to the DataStore from 
 * the various product formats are in handlers.c
 *
 * Also uses the ingest.c interface.
 *
 */
/*		Copyright (C) 1987-92 by UCAR
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

#if !defined(SABER) && !defined(lint)
static char rscid[] = 
   "$Id: pdb_ingest.c,v 1.2 1993-03-24 22:59:29 granger Exp $";
#endif

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <values.h>
#include <time.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/uio.h>

#include "sock.h"
#include "pdb.h"
#include "str.h"
#include "err.h"

#include "prod_structs.h"
#include "ingest.h"
#include "pdblib.h"
#include "pdb_ingest.h"
#include "pdbutils.h"
#include <copyright.h>

#define ERR_CONTROL_SETTING "OFF SLOG LOG ON STD ON USR"

#define KEY 0x18181818	/* '18' was my soccer jersey number... :-) */
/*
 * Use a global value for key so that it can be set from the
 * command line, if necessary
 */
static long Key = KEY;

#ifndef lint
MAKE_RCSID("$Id: pdb_ingest.c,v 1.2 1993-03-24 22:59:29 granger Exp $")
#endif

/*
 * General initialization and upkeeping
 */
static char *	InitializePDB FP((char *name));
static void	InitializeProductsTable();
static int	ReadProductsFile FP((char *fname));
static void	Usage FP((char *prog_name));
static void	ParseCommandLineOptions FP((int *argc, char *argv[]));
extern void	LogERRtoEL FP((int priority, char *message));
static void	Exit FP((int code));

/*
 * Product mapping functions:
 */
static struct _ProductRecord *PdbIdMap FP((int pdb_id));
static struct _ProductRecord *CodeMap FP((char *code));
static struct _ProductRecord *RapIdMap FP((long rap_id));

/*
 * Product handling
 */
static void	RequestProducts FP((char **code_list, int ncodes));
static void	RequestProductFormat FP((int format));
static void	SetProductStatus FP((int pdbid, int status));
static void	SetProductDescription FP((int pdbid, char *desc));
static int	VerifyConnection FP((char *msgname));
static struct _ProductRecord *InsertProduct 
			FP((int pdbid, char *code, char *name));

/*
 * Debugging and general info
 */
static void	DumpProductsTable FP((FILE *out));
static char *	PrintProductsTable ();

/*
 * Message and signal handlers
 */
static int	HandleResponse FP((int mtype, char *msg, int mlen));
static int	ReceiveMessages ();
static int	QueryHandler FP((char *from));
static int	DeathHandler ();
static int	MessageHandler FP((struct message *msg));
static void	signal_trap FP((int sig));
static int	poll_msg FP((int msg_fd, int waitsecs));

/*
 * Product handlers 
 */
static int	DispatchProduct FP((int mtype, char *msg, int mlen));
static int	DefaultRawHandler FP((int mtype, char *msg, int mlen));
static int	DefaultLineHandler FP((int mtype, char *msg, int mlen));
extern int	IngestPolylineProduct 
			FP((PRptr pr, int mtype, char *msg, int mlen));

/*--------------------------------------------------------------*/

# define DEFAULT_TIMEOUT 10
# define DEFAULT_MAX_SUBS 10

/*
 * Global debugging flags set from command line
 */
int MaxSubplatforms = DEFAULT_MAX_SUBS;
static char JustListProducts = (char)0;
static int GlobalProductFormat = PDB_FORMAT_RAW;
static int TimeoutSeconds = DEFAULT_TIMEOUT;
static short ConcatenatePolylines = 0;  /* 0 - Use first polyline of prod
					 * 1 - Join all polylines */

/*
 * Other global data
 */
static char *FormatString[] = { "NONE", "RAW", "LINE", "OBJS" };
static long BeginTime = 0; /* When program was starter, for stat info */

/*
 * The Map arrays:
 */
static ProductRecord *PdbIdArray[ MAX_PRODUCTS ];
static ProductRecord *PRptrMap[ MAX_PRODUCTS ] = { NULL };
static int NumProducts = 0;

/*===============================================================*/


/*------------------------------------------------------------------
 * MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN
 *------------------------------------------------------------------
 */
int main (argc, argv)
	int argc;
	char **argv;
{
	char *msgname;

	BeginTime = Time(0);
/*
 * First parse any of the general ingest options
 */
	IngestParseOptions(&argc, argv, Usage);
/*
 * Now any Pdb options
 */
	PdbParseOptions(&argc, argv, Usage);
/*
 * Get our command-line options, setting appropriate global variables
 * Only the program name and the product codes should remain
 */
	ParseCommandLineOptions(&argc, argv);
	if (JustListProducts) SetDryRun();
/*
 * Initialize usy, message, DataStore, and fields all at once
 */
	msgname = (GlobalProductFormat == PDB_FORMAT_RAW) ?
		   "PdbRawIngest" : "PdbLineIngest";
	IngestInitialize(msgname);
	msg_SetQueryHandler(QueryHandler);
	msg_DeathHandler(DeathHandler);
	/*
	 * Override Ingest's handler to perform our own death
	 * on MH_SHUTDOWN messages
	 */
	msg_AddProtoHandler( MT_MESSAGE, MessageHandler );
/*
 * Connect to PDB Server, set-up ERR module. Sends ONLY the CONNECT
 * message... Returns the name used for the connection.
 */
	msgname = InitializePDB(msgname);
/*
 * Request a products list, and get descriptions for each
 * of those prodcuts to store in our products table
 */
   	InitializeProductsTable();

/*
 * Remaining arguments should be the requested data product codes
 */
	if ((argc < 2) && !JustListProducts)
	{
		fprintf(stderr,"Error: No products requested...\n");
		Usage(argv[0]);
		Exit(1);
	}

	if (!VerifyConnection(msgname))
	{
		IngestLog(EF_EMERGENCY,
		   "Multiple connection attempts to PDBS FAILED.  Aborting.");
		Exit(1);
	}

	if (JustListProducts)
	{
		printf("Listing products table:\n");
		DumpProductsTable(stdout);
		Exit(0);
	}

/*
 * Set the format in which we will receive each product,
 * and request the products as specified on the rest of the command line
 */
	RequestProductFormat(GlobalProductFormat);
   	RequestProducts(argv+1, argc - 1 );

/*
 * Start receiving products from the products server
 */
	PdbRequest(PDB_START, 0, 0);
	IngestLog(EF_DEBUG,"--- Product Service Started at %s ---",
	   AscSysTime(BeginTime, TC_Full));
   	ReceiveMessages();

	/*
	 * All done.  Call our exit function
	 */
	Exit(0);
}



/* ParseCommandLineOptions --------------------------------------------
 *    Set global variables from command-line options, leaving only
 *    the expected file and field names in the arg list.  Must be sure
 *    to call the IngestParser for general ingest options
 */
static void
ParseCommandLineOptions(argc, argv)
	int *argc;
	char *argv[];
{
	int i;
	int nargs;

/*
 * Check for any of our own debug flags on the command line
 */
	i = 1;
	while (i < *argc)
	{
		nargs = 1;
		if (streq(argv[i],"-list"))
		{
		   JustListProducts = (char)1;
		}
		else if (streq(argv[i],"-raw"))
		{
		   GlobalProductFormat = PDB_FORMAT_RAW;
		}
		else if (streq(argv[i],"-line"))
		{
		   GlobalProductFormat = PDB_FORMAT_POLYLINE;
		}
		else if (streq(argv[i],"-passive") ||
			 streq(argv[i],"-monitor"))
		{
			fprintf(stderr,"% option not available yet... Sorry.",
				argv[i]);
		}
		else if (streq(argv[i],"-key"))
		{
		   nargs = 2;
		   Key = (unsigned)atol(argv[i+1]);
		   if (!Key) Key = KEY;
		   fprintf(stderr,"Key set to %li",Key);
		}
		else if (streq(argv[i],"-subs"))
		{
		   nargs = 2;
		   MaxSubplatforms = atoi(argv[i+1]);
		   if ((MaxSubplatforms <= 0) ||
		       (MaxSubplatforms > MAX_SUBPLATFORMS))
			MaxSubplatforms = DEFAULT_MAX_SUBS;
		   fprintf(stderr,"Using %d subplatforms\n",
			MaxSubplatforms);
		}
		else if (streq(argv[i],"-cat"))
		{
		   ConcatenatePolylines = 1;
		}
		else if (streq(argv[i],"-wait"))
		{
		   nargs = 2;
		   if (i+1 >= *argc)
		   {
			fprintf(stderr,"-wait needs an argument\n");
			Usage(argv[0]);
			exit(1);
		   }
		   TimeoutSeconds = atoi(argv[i+1]);
		}
		else
		{
		   ++i;
		   continue;
		}

		IngestRemoveOptions(argc, argv, i, nargs);
	}
}


static void
Usage(prog)
	char *prog;
{
   fprintf(stderr,
      "Usage: %s [options] [-]all [-]<code> [-]<pdbid> ...\n",prog);
   fprintf(stderr,"       %s -list\n",prog);
   fprintf(stderr,"       %s -help\n",prog);
   fprintf(stderr," where <code> is a 3-ltr product code, and\n");
   fprintf(stderr,
      "       <pdbid> is the PDBS product id, 1 - %d\n", MAX_PROD_ID);
   fprintf(stderr,"Options:\n");
   fprintf(stderr,"   %-15s List the available data products\n","-list");
   fprintf(stderr,"   %-15s Ingest raw products\n","-raw");
   fprintf(stderr,"   %-15s Ingest polyline products\n","-line");
   fprintf(stderr,"   %-15s Join all polylines in a product\n","-cat");
   fprintf(stderr,"   %-15s Timeout value, default: %d secs\n","-wait <secs>",
		DEFAULT_TIMEOUT);
   fprintf(stderr,"   %-15s Just connect and listen to server\n","-passive");
   fprintf(stderr,"   %-15s Just connect and listen to server\n","-monitor");
   fprintf(stderr,"   %-15s Maximum number of subplatforms to use\n",
		  "-subs <n>");
   fprintf(stderr,"   %-15s Change the key used for shm and sem\n","-key <k>");
   fprintf(stderr,"   %-15s 1 or 2 is added to the key, according to\n"," ");
   fprintf(stderr,"   %-15s the product format being ingested.\n"," ");
   IngestUsage();
   PdbUsage();
   fprintf(stderr,"Examples:\n");
   fprintf(stderr,"%s -log all all -11 -17 -hal -raw -key 1234\n",prog);
   fprintf(stderr,"%s -log pi -fast -sim data.lines -line all -requests\n",
	prog);
}



static char *
InitializePDB(name)
	char *name;
{
   int         err;
#  define      HOSTNAMELEN 64
   static char client[HOSTNAMELEN + 30];

   /*
    * Initialize the error module.  Disables command line options
    * for ERR by not passing argc or argv.  We want to override
    * the options here, and set up our own handler...
    */
   ERRinit(name, 0, NULL);
   ERRcontrol(ERR_CONTROL_SETTING);
   err = ERRuser(LogERRtoEL);
   ERRuserActive(err, 1); 	/* Make sure our handler is active */

   /*
    * Initialize PDB and SOCK.  Use KEY for the shm and sem keys.
    * Add the format to KEY so that we have a unique key for each
    * product format, so that a separate pdb_ingest can run for each
    * format.
    */
   sprintf(client,"%s@",name);
   gethostname(client+strlen(client),HOSTNAMELEN);
   /*
    * PdbInitAll should intialize SOCK, set up our client socket,
    * connect to the PDBS server, and send a CONNECT message
    */
   IngestLog(EF_INFO,"Connecting to PDBS with client name %s",client);
   err = PdbInitAll(client, Key+GlobalProductFormat);
   if (err != 1)
   {
	IngestLog(EF_EMERGENCY, "PdbInitAll() failed with error %d", err);
   	Exit(1);
   }

   /*
    * Trap certain signals for cleaning on interruptions:
    */
   signal(SIGINT, signal_trap);
   signal(SIGHUP, signal_trap);
   signal(SIGTERM, signal_trap);
   signal(SIGPIPE, SIG_IGN);
   /* We're not going to trap SIGPIPE because I suspect it
    * messes up the message handler...
    */

   /*
    * Once we've initialized, change our ERR settings for
    * the long haul...
    */
   ERRcontrol("OFF SLOG LOG STD ON USR");
   return(client);
}


static void
InitializeProductsTable()
/*
 * Read all of our known products from the products.pdbs file
 * and insert them with InsertProduct().
 * Request a products list in case we don't have all of them.
 * Descriptions, codes, and names received from server will
 * override whatever has been previously inserted.
 */
{
	int i;

	for (i=0; i<MAX_PRODUCTS; ++i)
		PdbIdArray[i] = NULL;
		
	PRptrMap[0] = NULL;
	NumProducts = 0;

	/*
	 * Now insert our known products
	 */
	i = ReadProductsFile("products.pdbs");
	IngestLog(EF_DEBUG,"%i products read from products.pdbs",i);

}


static int
VerifyConnection(name)
	char *name;  /* Application name to use in connect msg */
/*
 * Send a CONNECT message to the server, then send a
 * LIST_PRODUCTS message and verify a response.
 * Wait 1 minute between each attempt ....
 */
{
	int attempts = 0;

	while (++attempts)	/* Just keep trying forever */
	{
		/*
		 * First request a list to verify connection and
		 * make sure products table is complete 
		 */
		PdbRequest(PDB_LIST_PRODUCTS, (char *)0, 0);
		if (PdbVerifyResponse(TimeoutSeconds, 
				PDB_LIST_PRODUCTS, HandleResponse))
		{
			IngestLog(EF_INFO,
	   "Products list received from PDBS.  Connection established.");
			return (1);
		}

		/* Connection failed */
		IngestLog(EF_EMERGENCY,
		   "Connection attempt %i FAILED.  Trying again in 1 minute",
		   attempts);
		/*
		 * Wait no more than 60 seconds before next connection attempt,
		 * but meanwhile make sure we take care of any incoming msgs,
		 * especially of the shutdown variety
		 */
		if (!DryRun)
			poll_msg(msg_get_fd(),60);

		/* 
		 * Try a CONNECT message; this resets our settings
		 * on the server side (if it receives it) 
		 */
		if (PdbRequestString(PDB_CONNECT, name) != 1)
		{
			IngestLog(EF_EMERGENCY,
			   "PDB_CONNECT %s request failed", name);
		}
		sleep(1);
	}
	return (0);
}


/*
 * Read in the products file into the list, return # of items read.
 * Taken from John Caron's pdbs.c source
 */
static int 
ReadProductsFile(fname)
	char *fname;
{
#	define MAX_LINE 150
	int count = 0;
	FILE *fp_prod;
	char line[ MAX_LINE];
	int pdbid;
	int rapid;
	char code[PROD_CODE_LEN];
	char name[PROD_NAME_LEN];
	char desc[PROD_DESC_LEN];
	PRptr pr;
    
	fp_prod = fopen(fname,"r");
	if (fp_prod == NULL)
	{
	   ERRprintf(ERR_WARNING, "ERROR opening file %s\n", fname);
	   return 0;
	}
    
	while (NULL != fgets( line, MAX_LINE, fp_prod))
	{
	 /* remove comments */
	 if (line[0] == '#')
	    continue;
     
	   /* remove blank lines */
	 if (STRblnk( line) == 0)
	    continue;

	 /* check for too many products */
	   if (count >= MAX_PRODUCTS)
	    {
	    ERRprintf(ERR_WARNING, "too many products max=%d ..truncate\n",
	       MAX_PRODUCTS);
	      break;
	    }

	 /* parse the line : 
	  * name code pdbs_id product_id color socket_name buff_size desc
	  */
	 sscanf(line,"%s %s %i %i",name, code, &pdbid, &rapid);
	 pr = InsertProduct(pdbid, code, name);
	 pr->rap_id = rapid;

	 /* second line is the description */
	 if (NULL == fgets( desc, PROD_DESC_LEN, fp_prod))
	    break;
	 STRblnk( desc);
	 SetProductDescription(pdbid, desc);

  	 count++;
	 if (count >= MAX_PRODUCTS)
	 {
		IngestLog(EF_PROBLEM,
	   "Too many products in products file. Not enough table entries.");
		break;
	 }
	}
    
	fclose(fp_prod);
	return( count);
}



static void
RequestProductFormat(fmt)
	int fmt;
{
	static char sfmt[5];

	sprintf(sfmt,"%i",fmt);
	PdbRequestString(PDB_SET_FORMAT, sfmt);
	IngestLog(EF_INFO,"Selecting %s format from PDBS",
		FormatString[fmt]);
	/*
	 * This means all of the products will come in this
	 * format, so set that field in each product record.
	 * For now, that field is automatically set to the
	 * default in InsertProduct()
	 */
}



static PRptr
InsertProduct(pdbid, code, name)
	int pdbid;
	char *code;
	char *name;
/*
 * Initialize a ProductRecord, insert its pointers into the
 * appropriate maps, and return the pointer
 * Overwrite with new code and name values if the product
 * already exists.
 * ALL product records must be created through here to be
 * initialized correctly
 */
{
	ProductRecord *pr;
	int i;

	if ((pdbid < 1) || (pdbid > MAX_PROD_ID))
	{
	   IngestLog(EF_PROBLEM, 
	      "Illegal product id %d inserting code %s, name %s",
	      pdbid, code, name);
	   return(NULL);
	}
	if (!(pr = PdbIdArray[pdbid]))   /* Need new record */
	{
	   pr = (PRptr)malloc(sizeof(ProductRecord));

	   PdbIdArray[pdbid] = pr;
	   PRptrMap[NumProducts++] = pr;

	   pr->pdb_id = pdbid;
	   pr->rap_id = 0;
	   pr->next_id = 0;
	   strcpy(pr->desc,"No Description");
	   strcpy(pr->platform,PLATFORM_PREFIX);
	   strcat(pr->platform,code);
	   StrToLower(pr->platform);
	   pr->platid = ds_LookupPlatform(pr->platform);
	   pr->format = GlobalProductFormat;
	   pr->origin_lat = MHR_ORIGIN_LAT;
	   pr->origin_lon = MHR_ORIGIN_LON;
	   pr->angle = 0.0;
	   pr->last = 0;
	   pr->requested = 0;
	   pr->status = ProdStatInactive;
	   pr->raw_handler = DEFAULT_RAW_HANDLER;
	   pr->line_handler = DEFAULT_LINE_HANDLER;
	/*
	 * Initialize the sub-platform LRU arrays and fields
	 */
	   for (i=0; i< MaxSubplatforms; ++i)
	   {
		pr->id_numbers[i] = -1;
		pr->next_oldest[i] = i+1;
	   }
	   pr->index_oldest = 0;
	   pr->index_newest = MaxSubplatforms-1;
	   pr->next_oldest[pr->index_newest] = pr->index_oldest;
#ifdef notdef
	   IngestLog(EF_DEBUG,
	    "Inserting %s %d, rap_id %d, platform %s, platid %d",
	    pr->code, pr->pdb_id, pr->rap_id, pr->platform, pr->platid);
#endif
	}

	strncpy(pr->code, code, PROD_CODE_LEN);
	pr->code[PROD_CODE_LEN-1] = '\0';
	strncpy(pr->name, name, PROD_NAME_LEN);
	pr->name[PROD_NAME_LEN-1] = '\0';

	return(pr);
}



static void
SetProductDescription(pdbid, desc)
	int pdbid;
	char *desc;
{
	ProductRecord *pr;

	if (!(pr = PdbIdMap(pdbid)))
		return;
	strncpy(pr->desc, desc, PROD_DESC_LEN);
	pr->desc[PROD_DESC_LEN-1] = '\0';
}



static void
SetProductStatus(pdbid, status)
	int pdbid;
	int status;
{
	ProductRecord *pr = PdbIdMap(pdbid);

	if (pr)
		pr->status = (ProdStatus)status;
}
	


static PRptr
RapIdMap(rap_id)
	long rap_id;
/*
 * Returns ptr to the product record with the given rap_id.
 * Returns NULL if not found
 */
{
	PRptr *pr;

	pr = PRptrMap;
	while (*pr)
	{	
		if ((*pr)->rap_id == rap_id)
			return(*pr);
		++pr;
	}
}



static PRptr
CodeMap(code)
	char *code;
/*
 * Map a product code to its product record
 * Returns NULL if not found
 */
{
	PRptr *pr;

	pr = PRptrMap;
	while (*pr)
	{
		if (streq((*pr)->code, code))
			return(*pr);
		++pr;
	}
	return(NULL);
}


static PRptr
PdbIdMap(pdbid)
	int pdbid;
/*
 * The mapping from pdbid to a product record is just an array indexing,
 * but we also do some sanity checking here for bad pdbid's
 */
{
	if ((pdbid < 1) || (pdbid > MAX_PROD_ID))
	{
		IngestLog(EF_PROBLEM, 
		   "Bad product id %d in PdbIdMap()", pdbid);
		return(NULL);
	}
	else
	{
		return(PdbIdArray[pdbid]);
	}
}
	


static void
RequestProducts(code_list, ncodes)
	char *code_list[];
	int ncodes;
/*
 * code_list is an array of strings naming codes of products to request
 * codes can be either the 3-ltr type or pdb id's.  Any code
 * preceded by a '-' will be removed from the list of wanted products.
 * ncodes is the number of codes in code_list.
 * Each product named, and so noted in Table
 * The code "ALL" selects all products from the table
 * Codes can be in either upper or lower case
 * Ex: "all", "-act" requests all products except aircraft tracks
 */
{
	ProductRecord *pr;
	char **code;
#	define CHOICE_LEN 8
	char choice[CHOICE_LEN];
	char selections[MAX_PRODUCTS * 4];
	char rejections[MAX_PRODUCTS * 4];
	short all = 0;		/* True if ALL code should be sent */
	int pdbid;
	int reject;
	int num_requested = 0;
	int i, j;

	selections[0] = '\0';
	rejections[0] = '\0';

	for (i = 0, code = code_list; i<ncodes; ++i, ++code)
	{
		reject = 0;
		all = 0;
		pr = NULL;
		if (*code[0] == '-')
		{
			reject = 1;
			strncpy(choice, *code+1, CHOICE_LEN);
		}
		else
			strncpy(choice, *code, CHOICE_LEN);
		choice[CHOICE_LEN - 1] = '\0';

		if ((pdbid = atoi(choice)) > 0)
		{
			if (pdbid < MAX_PRODUCTS)
				pr = PdbIdMap(pdbid);
		}
		else
		{
			StrToUpper(choice);
			if (streq(choice,"ALL"))
				all = 1;
			else
				pr = CodeMap(choice);
		}
		if (!pr && !all)
		{
		   fprintf(stderr,"Unknown product code %s\n",
				  choice);
		}
		else
		{
		   if (!all)
			sprintf(choice,"%i",pr->pdb_id);
		   if (!reject)
		   {
			PdbRequestString(PDB_REQUEST_PRODUCT,choice);
			sprintf(selections+strlen(selections),
			   " %s:%i",(all)?(choice):(pr->code),
				     (all)?(0):(pr->pdb_id));
		   }
		   else
		   {	/* make sure this product won't be sent */
			PdbRequestString(PDB_STOP_PRODUCT,choice);
			sprintf(rejections+strlen(rejections),
			   " %s:%i",(all)?(choice):(pr->code),
				     (all)?(0):(pr->pdb_id));
		   }
		   if (!all) 
		   {
			if (pr->requested != (reject)?(0):(1))
			{
			   pr->requested = (reject)?(0):(1);
			   num_requested += (reject)?(-1):(1);
			}
		   }
		   else
		   {
			for (j = 0; j<NumProducts; ++j)
			{
			   if (PRptrMap[j]->requested != (reject)?(0):(1))
			   {
			      PRptrMap[j]->requested = (reject)?(0):(1);
			      num_requested += (reject)?(-1):(1);
			   }
			}
		   }
		   num_requested = (num_requested > 0) ? num_requested : 0;
		}
	}
	if (num_requested <= 0)
	{
		IngestLog(EF_EMERGENCY,"No products selected");
		Exit(1);
	}
	else
	{
		IngestLog(EF_INFO,"%i products selected:%s %s%s",
			num_requested, selections, 
			(rejections[0] == '\0')? "" : "  rejected:",
			rejections);
	}
}




/*
 * Answer queries with some info about our current status
 */
static int
QueryHandler(who)
	char *who;
{
	IngestLog(EF_DEBUG,"Answering query from %s",who);
	msg_AnswerQuery(who, PrintProductsTable() );
	msg_FinishQuery(who);
	return(0);
}



/*
 * If we lose our message connection, try to die gracefully
 */
static int
DeathHandler()
{
	SetNoMessageHandler();
	IngestLog(EF_EMERGENCY,
	   "Message socket lost! Good-bye cruel world!");
	Exit(1);
}


/*
 * Look for any MH_SHUTDOWN messages from the MT_MESSAGE proto
 */
static int
MessageHandler(msg)
	struct message *msg;
{
	struct mh_template *mh = (struct mh_template *)msg->m_data;

	if (mh->mh_type == MH_SHUTDOWN)
	{
		IngestLog(EF_INFO,"Shutdown message from %s",
				  msg->m_from);
		Exit(0);
	}
}



static void
signal_trap(sig)
	int sig;
{
	IngestLog(EF_EMERGENCY,"Trapped signal %d",sig);
	Exit(1);
}


/*
 * Try to exit cleanly from PDB Server and SOCK
 */
static void
Exit(code)
   int code;
{
   IngestLog(EF_DEBUG,"Exiting through Exit()");
   PdbRequest(PDB_DISCONNECT, (char *) 0, 0);
   SOCKexit(code);
   exit(code);
}


/*
 * Poll the message handler for any pending messages and handle them
 * with msg_incoming().  The number of seconds to block on the select()
 * is set with the 'timeout' parameter
 */
static int
poll_msg(msg_fd, timeout)
	int msg_fd;
	int timeout; /* seconds */
{
	fd_set readfds;
	int ret;
	struct timeval delay;

	delay.tv_sec = timeout;
	delay.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(msg_fd,&readfds);

	if (select(msg_fd+1, &readfds, NULL, NULL, &delay) > 0)
	{
		/* We have a message to read */
		return(msg_incoming(msg_fd));
	}
	else
		return(0);
}


/* 
 * ReceiveMessages
 * ---------------
 * Alternatively polls (non-blocking select()) the message handler
 * file descriptor and the PDB (with PdbResponse).  Any mh messages
 * are sent to msg_incoming(), while PDB responses are dispatched
 * with HandleResponse().
 *
 * If no responses are received from the product server after some timeout,
 * a warning message is logged.  At this point, it is the operators 
 * option of assuming nothing is coming in, or restarting pdb_ingest
 */
static int
ReceiveMessages()
{
	int ret = 0;
	int msg_fd;
	int secs_waiting;  /* The number of secs since last response */
#	define TIMEOUT 2   /* interval, in seconds, to wait on a pdb response */
#	define WARNING 10  /* interval, in minutes, to log a warning */

	int pdb_mtype;
	int pdb_mlen;
	char *pdb_msg;

	if (!DryRun)
		msg_fd = msg_get_fd();

	/*
	 * At the moment, we're ignoring the return values
	 * from the handlers because msg_incoming() will return
	 * 1 if a read fails (even after select() succeeded(?))
	 */
	secs_waiting = 0;
	while (1)
	{
		/* 
		 * Poll msg_fd first 
		 */
		if (!DryRun)
		{
		   ret = poll_msg(msg_fd, 0);
		   if (ret)
			IngestLog(EF_DEBUG,
			    "msg_incoming() returned %d",
			    ret);
		}

	/*
	 * NOTE: There is no delay in the msg_fd select() call so
	 * that there will be no delay between pending reads from
	 * PDB.  If a delay is added to the msg_fd poll, a do{}while
	 * loop should be added below to take care of all pending
	 * PDB messages before blocking on the message fd
	 */

		/* Next we wait on PDB */
		if (PdbResponse(TIMEOUT, 
			&pdb_mtype, &pdb_msg, &pdb_mlen) > 0)
		{
		      secs_waiting = 0;
		      /* We need to dispatch a PDB message */
		      ret = HandleResponse(pdb_mtype, pdb_msg, pdb_mlen);
		      if (ret)
			 IngestLog(EF_PROBLEM,
			    "HandleResponse() returned %d",
			    ret);
		}
		else  
		/* 
		 * Note another addition to our wait time,
		 * and send a warning every 5 minutes
		 */
		{
			secs_waiting += TIMEOUT;
			if ((secs_waiting % (WARNING*60)) >= 
				(WARNING*60)-TIMEOUT)
			{
			   IngestLog(EF_PROBLEM, 
		      "--- No response from PDBS in %i minutes!! ---",
		      		((secs_waiting+TIMEOUT) / 60));
			}
		}
	}
}


/* HandleResponse
 * --------------
 * Either handle a PDB response here or dispatch it to the
 * appropriate function
 */
static int
HandleResponse(mtype, msg, mlen)
	int mtype;
	char *msg;
	int mlen;
{
	ProductRecord *pr;
	char code[PROD_CODE_LEN];
	char name[PROD_NAME_LEN];
	char desc[PROD_DESC_LEN];
	int pdbid;
	int status;

	switch (mtype)
	{
	   /* 
	    * Response to a PDB_LIST_PRODUCTS request: 
	    */
	   case PDB_LIST_PRODUCTS:
	      {
		char *ptr;
	
		for (ptr = msg; ptr < msg + mlen; ptr += strlen(ptr) + 1)
		{
		   if (sscanf(ptr,"%i %s %s", &pdbid, code, name) == 3)
		   {
		      pr = PdbIdMap(pdbid);
		      if (!pr)  /* Product not in products file,
				 * needs a description from server */
		      {
			 IngestLog(EF_INFO,
			  "New product found: %s not in products file",
			  ptr);
			 PdbRequestString(PDB_PRODUCT_DESCRIPTION,
					  ptr);
		      }
		      InsertProduct(pdbid, code, name);
		   }
		}
	      }
	      break;
	   /* 
	    * Response to a PDB_PRODUCT_DESCRIPTION request: 
	    */
	   case PDB_PRODUCT_DESCRIPTION:
		sscanf(msg,"%i %[^\n]", &pdbid, desc);
		SetProductDescription(pdbid, desc);
		break;
	   /* 
	    * Response to a PDB_REQUEST_PRODUCT request: 
	    */
	   case PDB_STATUS:
		pdbid = 0;
		status = -1;
		if (sscanf(msg,"%i %i",&pdbid, &status) == 2)
			SetProductStatus(pdbid, status);
		IngestLog(EF_DEBUG,"Msg: %10s: Id %2d, Status %1d",
			  msg, pdbid, status);
		break;
	   case PDB_PRODUCT:
		return(DispatchProduct(mtype, msg, mlen));
	        break;
	   case PDBGET_TEST_MSG:
		IngestLog(EF_DEBUG,"Simulation Test Msg: %s",msg);
		break;
	   default:
	   /*
	    * First see if this message could be a RAP ID for
	    * a RAW format product
	    */
		pr = RapIdMap(mtype);
		if (pr)		/* We found a RAW product */
		{
		   return(DispatchRawProduct(pr, mtype, msg, mlen));
		}
		else
		   IngestLog(EF_PROBLEM,
		      "Unexpected PDB message type %d\n",mtype);
	}
	return(0);
}


static int
DefaultRawHandler(mtype, msg, mlen)
	int mtype;
	char *msg;
	int mlen;
/*
 * Simply acknowledge that there is no official handler for
 * this data product, but note this reception in 'last' field
 */
{
	IngestLog(EF_DEBUG,
	   "No handler for raw product message %d",
	   mtype);
	return(0);
}


/* ARGSUSED */
static int
DefaultLineHandler(mtype, msg, mlen)
	int mtype;
	char *msg;
	int mlen;
/*
 * Simply acknowledge that there is no official handler for
 * this data product
 */
{
	PDBproduct *pd = (PDBproduct *)msg;

	IngestLog(EF_DEBUG,
	   "No handler for polyline product type %d",
	   pd->type);
	return(0);
}



/*
 * Take a PDB_PRODUCT message, figure out what type it is and
 * what format its coming in as, and then call the appropriate
 * handler according to the ProductsTable
 */
static int
DispatchProduct(mtype, msg, mlen)
	int mtype;
	char *msg;
	int mlen;
{
	PDBproduct *pd = (PDBproduct *)msg;
	ProductRecord *pr;
	int pdbid;

	if (GlobalProductFormat == PDB_FORMAT_RAW)
	{
		/* We're not supposed to get this message while
		 * requesting raw products.... */
		IngestLog(EF_PROBLEM,
		   "Received PDB_PRODUCT message when expecting RAW data");
		/*
		 * Re-request format, but ingest this polyline anyway
		 * in case we can get something out of it
		 */
		RequestProductFormat(GlobalProductFormat);
	}

	pdbid = pd->type;
	if ((pdbid < 1) || (pdbid > MAX_PROD_ID))
	{
		IngestLog(EF_PROBLEM,"Product id %d is out of range",
			  pdbid);
		return(0);
	}

	pr = PdbIdMap(pdbid);
	if (!pr)
	{
	   IngestLog(EF_PROBLEM,"Id %d not found in ProductsTable",
			pdbid);
	   return(0);
	}
	/*
	 * Record the time of this reception, in case it is not
	 * overridden with a more appropriate value later
	 */
	pr->last = Time(0);
	/*
	 * At this point we assume the product is polyline, so ingest
	 * in case it can be useful
	 */
	IngestLog(EF_INFO,
  	   "Received %s(%hu): npts %hu, at %s",
 	   pr->code, pd->type, 
	   pd->npts, AscSysTime(Time(0), TC_Full) );
	return(IngestPolylineProduct(pr, mtype, msg, mlen));
}




int
DispatchRawProduct(pr, mtype, msg, mlen)
	ProductRecord *pr;
	int mtype;		/* Also the RapId for this product */
	char *msg;
	int mlen;
/*
 * All we're going to do for now is switch on the ProductRecord
 * RapId and call some function which will hopefully ingest the
 * data into the DataStore...
 */
{
	char buf[100];
	int ret;

	/*
	 * If we we're expecting polyline, we shouldn't get any
	 * raw products.  Note discrepancy and re-send format
	 */
	if (GlobalProductFormat == PDB_FORMAT_POLYLINE)
	{
		IngestLog(EF_PROBLEM,
		   "Got RAW product format when expecting Polyline");
		RequestProductFormat(GlobalProductFormat);
		/* We'll drop out and try to ingest this product anyway
		 */
	}
	pr->last = Time(0);
	IngestLog(EF_INFO,
   	   "Received %s(%d), pdb %d, at %s",
	   pr->code, pr->rap_id, pr->pdb_id,
	   AscSysTime(pr->last, TC_Full));

	switch(pr->rap_id)
	{
	   case 9203:
	   case 9204:
	   case 9225:
	   case 9226:
	   case 9229:
	   case 9230:
		ret = IngestExtrapolations(pr, mtype, msg, mlen);
		break;
	   case 9201:
	   case 9202:
		ret = IngestNowcasts(pr, mtype, msg, mlen);
		break;
	   case 9205:
		ret = IngestNowcastRegions(pr, mtype, msg, mlen);
		break;
	   case 9227:
	   case 9228:
	   case 9231:
	   case 9232:
		ret = IngestForecasts(pr, mtype, msg, mlen);
		break;
	   default:
		IngestLog(EF_DEBUG,"No raw handler for %s(%d)!",
			pr->code, pr->rap_id);
		ret = 0;
	}
	return(ret);
}



static void
DumpProductsTable(out)
	FILE *out;
/*
 * Dump the products table to file 'out'
 */
{
	fprintf(out, "%s", PrintProductsTable() );
}



static char *
PrintProductsTable()
/*
 * Print the products table into a static buffer
 */
{
	static char buf[MAX_PRODUCTS * 100];
	int i;
	ProductRecord *pr;
	char *ptr;
	time_t now;

	buf[0] = '\0';
	ptr = buf;

	/*
	 * Header info, simple statistics
	 */
	Time(&now);
	sprintf(ptr,"Up for %d minutes, since %s",
		(int)((now-BeginTime)/60),
		ctime(&BeginTime));
	ptr += strlen(ptr);
	sprintf(ptr,"Time now is: %s\n",ctime(&now));
	ptr += strlen(ptr);
	
	sprintf(ptr,"%3s %-4s%6s%9s:%-4s ",
		"Id","Code","RAPid","Platform","Id");
	ptr += strlen(ptr);
	sprintf(ptr,"%-3s %-4s %-5s  %-15s\n",
		"Req","Actv","Fmt","Last received:");
	ptr += strlen(ptr);

	sprintf(ptr,"%3s %-4s%6s%9s %-4s",
		"--","----","-----","--------","---");
	ptr += strlen(ptr);
	sprintf(ptr,"%-3s %-4s %-5s  %-15s\n",
		"---","----","---","--------------");
	ptr += strlen(ptr);

	for (i=0; i<NumProducts; ++i)
	{
	/*
	 * For each product, print important info
	 */
	   pr = PRptrMap[i];

	   sprintf(ptr,
		"%3i %-4s %5i %8s:%-3i ",
		pr->pdb_id, pr->code, pr->rap_id, 
		pr->platform, pr->platid);
	   ptr += strlen(ptr);

# ifdef notdef
	   sprintf(ptr,
		"%3i %-4s RAP:%5i  Name: %-20s  Platform: %-10s Id: %d\n",
		pr->pdb_id, pr->code, pr->rap_id, pr->name,
		pr->platform, pr->platid);
	   ptr += strlen(ptr);

	   sprintf(ptr,"         %-60s\n", pr->desc);
	   ptr += strlen(ptr);
	   sprintf(ptr,"         Origin: %4.2f deg lat  %4.2f deg lon",
		   pr->origin_lat, pr->origin_lon);
	   ptr += strlen(ptr);
	   sprintf(ptr,"   Grid angle: %4.2f deg\n",pr->angle);
	   ptr += strlen(ptr);
# endif

	   sprintf(ptr,"%-3s %-4s %-5s",
		(pr->requested) ? "YES" : "NO",
		(pr->status) ? "YES" : "NO",
		FormatString[pr->format]);
	   ptr += strlen(ptr);
	   sprintf(ptr,"  %s",
		(pr->last) ? ctime(&(pr->last)) : "Never\n");
	   ptr += strlen(ptr);
	}
	if (!buf[0])
	   sprintf(buf,"No valid product entries in table");
	return(buf);
}

