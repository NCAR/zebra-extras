/*
 * pdbget.c --- Reads products from the PDB Server and write's them
 *		to standard out to be piped into a file
 *
 * Options -raw or -pl to specify the mode to get (raw or polyline)
 *         -test produces test messages, but cannot generate data
 *
 * It's possible to create tests with multiple modes using
 * pdbget -pl -test -raw -test
 * To get just test messages and no PDB data, use -end at end of line
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <values.h>
#include <time.h>

#include "sock.h"
#include "pdb.h"
#include "str.h"

#include "pdblib.h"

void    pdbs_exit();
int	pdb_get_response();
int	pdb_response();
int	pdb_request();
int	pdb_dispatch();
void dump_list_products_msg();
int pdb_receive();
void pdb_write_msg();
void TestMessages();

int FDout = 1;	/* The fd we will use for output */

int ProductFormat = PDB_FORMAT_POLYLINE;


int
main(argc, argv)
   int         argc;
   char       *argv[];
{
   int         err;
   int i;

   for (i = 1; i<argc; ++i)
   {
	if (!strcmp(argv[i],"-raw"))
		ProductFormat = PDB_FORMAT_RAW;
	else if (!strcmp(argv[i],"-pl"))
		ProductFormat = PDB_FORMAT_POLYLINE;
	else if (!strcmp(argv[i],"-test"))
		TestMessages();
	else if (!strcmp(argv[i],"-end"))
		exit(0);
   }

   /*
    * Initialize the error module
    */
   ERRinit(argv[0], argc, argv);

   /*
    * Initialize PDB and SOCK.  Use our pid for the key value. ALG_SERVICES
    * env variable must be set to the services file
    */
   err = PDBinitAll(argv[0], getpid());
   fprintf(stderr,"PDBinitAll: returned %d\n", err);
   if (err < 1)
      exit(1);

   /*
    * Trap certain signals for cleaning on interruptions:
    */
   signal(SIGINT, pdbs_exit);
   signal(SIGHUP, pdbs_exit);
   signal(SIGTERM, pdbs_exit);
   signal(SIGPIPE, pdbs_exit);

   /*
    * Set format 
    */
   if (PDBrequest(PDB_SET_FORMAT, ProductFormat, 0) < 1)
   {
	fprintf(stderr,"PDBrequest(PDB_SET_FORMAT, %d) failed\n",
		ProductFormat);
	pdbs_exit(1);
   }
   
   if (PDBrequest(PDB_REQUEST_PRODUCT, "ALL", 4) < 1)
   {
	fprintf(stderr,"PDBrequest(PDB_REQUEST_PRODUCT, ALL) failed\n");
	pdbs_exit(1);
   }
   
   if (PDBrequest(PDB_START, (char *)0, 0) < 1)
   {
	fprintf(stderr,"PDBrequest(PDB_START) failed\n");
	pdbs_exit(1);
   }

   /*
    * Now just sit and receive whatever the PDBS sends to us
    */
   pdb_receive(300);
   fprintf(stderr,"No response from PDBS for 5 minutes...\n");
   fprintf(stderr,"%s exiting.\n",argv[0]);

   pdbs_exit(1);
}



void
TestMessages()
{
   char	test[30];
   static int j = 0;
   int i;

   /*
    * Test the pdb_write_msg function
    */
   for (i=j; i<j+5; i++)
   {
      sprintf(test,"Message %d  %i",j+i,(i+j)*(i+j));
      pdb_write_msg(FDout, PDBGET_TEST_MSG, test, strlen(test)+1);
      sleep(1);
   }
   j += i;
}



void
pdbs_exit(code)
   int         code;
{
   SOCKexit(code);
   exit(code);
}



int
pdb_request(type, req, len)
	int type;
	char *req;
	int len;
{
	int ret;

	printf("PDBrequest: type %d  length %d\n",
		type, len);
	ret = PDBrequest(type, req, len);
	if (ret < 1)
		printf("PDBrequest: error, returned %d\n",ret);
	return(ret);
}



int
pdb_response(wait_sec, type, msg, mlen)
	int wait_sec;
	int *type;
	char **msg;
	int *mlen;
{
	int ret;

	printf("PDBresponse: ");
	printf("Wait: %d\n",wait_sec);
	ret = PDBresponse(wait_sec, type, msg, mlen);
	if (ret > 0)
		printf("PDBresponse: msg type: %d  length: %d\n",
			*type, *mlen);
	else
		printf("PDBresponse: error: returned %d\n",ret);
	return(ret);
}



int
pdb_receive(waitsecs)
	int waitsecs;
{
	int ret;

	int pdb_mtype;
	int pdb_mlen;
	char *pdb_msg;

	while ((ret = PDBresponse(waitsecs, 
			   	  &pdb_mtype, 
				  &pdb_msg, &pdb_mlen)) > 0)
	{
	   pdb_dispatch(pdb_mtype, pdb_msg, pdb_mlen);
	}
	return(ret);
}


/* pdb_dispatch
 * ------------
 * Either handle a PDB response here or dispatch it to the
 * appropriate function
 */
int
pdb_dispatch(mtype, msg, mlen)
	int mtype;
	char *msg;
	int mlen;
{
	PDBproduct *pd;
	char *ptr;

	switch (mtype)
	{
	   /* 
	    * Response to a PDB_LIST_PRODUCTS request: 
	    */
	   case PDB_LIST_PRODUCTS:
		dump_list_products_msg(msg, mlen);
		break;
	   /* 
	    * Response to a PDB_PRODUCT_DESCRIPTION request: 
	    */
	   case PDB_PRODUCT_DESCRIPTION:
		fprintf(stderr,"Product description:\n%s\n",msg);
		break;
	   /* 
	    * Response to a PDB_REQUEST_PRODUCT request: 
	    */
	   case PDB_STATUS:
		fprintf(stderr,"Status message: %s\n",msg);
		break;

	   case PDB_PRODUCT:
		if (ProductFormat == PDB_FORMAT_POLYLINE)
		{
	           pd = (PDBproduct *) msg;
		   fprintf(stderr, "Incoming Product. ID: %d\n", pd->type);
		}
	        break;
	   default:
		fprintf(stderr,"Unexpected PDB response type %d\n",mtype);
	}
	/*
	 * For ALL messages received, we will write a copy of the
	 * message to our FDout
	 */
	pdb_write_msg(FDout, mtype, msg, mlen);
}


/*
 * Dump a LIST_PRODUCTS message to stderr.
 * msg is not modified in any way
 */
void
dump_list_products_msg(msg, mlen)
	char *msg;
	int mlen;
{
	char *ptr;

	for (ptr = msg; ptr < msg + mlen; ptr += strlen(ptr) + 1)
		fprintf(stderr,"%s\n", ptr);
}


void
pdb_write_msg(fd, mtype, msg, mlen)
	int fd;
	int mtype;
	char *msg;
	int mlen;
{
	PdbPacketHdr hd;

	hd.mtype = mtype;
	hd.mlen = mlen;
	hd.pdb_format = ProductFormat;
	hd.timestamp = time(0);

	write(fd, (char *)&hd, sizeof(hd));
	write(fd, msg, mlen);
}


