/*
 * $Id: pdbping.c,v 1.1 1992-07-03 18:22:46 granger Exp $
 *
 * pdbping.c -- Do a simple check to see if the PDB server is alive
 *
 * Optional 1st argument gives seconds to wait for timeout
 */

#include <math.h>		/* System */
#include <string.h>
#include <values.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include "sock.h"
#include "pdb.h"
#include "str.h"

#include <stdio.h>

void    pdbs_exit();
int	pdb_get_response();
int	pdb_response();
int	pdb_request();
int	pdb_dispatch();
void	dump_list_products_msg();
void	print_polyline_product();


/*----------------------------------------------------------------
  Re-define some of the PDB calls to log messages on stdout     */

#define _PDBrequest(type,req,len) pdb_request(type,req,len)
#define _PDBresponse(wait,type,msg,len) \
	pdb_response(wait,type,msg,len)

#define DEBUG2 0

/*--------------------------------------------------------------*/


int
main(argc, argv)
   int         argc;
   char       *argv[];
{
   int 		waitsecs;
   int         err;
   int         mtype;
   int         mlen;
   char       *buf,
              *ptr;

   /*
    * Initialize the error module
    */
   ERRinit(argv[0], argc, argv);

   /*
    * Initialize PDB and SOCK.  Use our pid for the key value. ALG_SERVICES
    * env variable must be set to the services file
    */
   err = PDBinitAll(argv[0], getpid());
   if (err < 1)
   {
      printf("Error: PDBinitAll returned %d\n", err);
      pdbs_exit(1);
   }

   /*
    * Trap certain signals for cleaning on interruptions:
    */
   signal(SIGINT, pdbs_exit);
   signal(SIGHUP, pdbs_exit);
   signal(SIGTERM, pdbs_exit);
   signal(SIGPIPE, pdbs_exit);

   if (argc>2)
   {
      fprintf(stderr,"Usage: pdbping [waitsecs]\n");
      pdbs_exit(1);
   }

   if (argc == 2)
      waitsecs = atoi(argv[1]);
   else
      waitsecs = 10;  /* default */

   /*
    * A simple test: get a listing of the products
    */
   err = _PDBrequest(PDB_LIST_PRODUCTS, (char *) 0, 0);

   err = _PDBresponse(waitsecs, &mtype, &buf, &mlen);
   if (err <= 0)
      fprintf(stderr,"No response from PDB Server\n");
   else if (mtype == PDB_LIST_PRODUCTS)
      dump_list_products_msg(buf, mlen);
   else
   {
      fprintf(stderr,"Received unexpected message type %d",mtype);
      fprintf(stderr,"   in response to PDB_LIST_PRODUCTS\n");
   }

   /*
    * All done.  Send disconnect, then call our exit function
    */
   _PDBrequest(PDB_DISCONNECT, (char *) 0, 0);
   pdbs_exit(0);
}



void
pdbs_exit(code)
   int         code;
{
   SOCKexit(code);
   exit(code);
}



/* pdb_get_response 
 * ----------------
 * Wait for a specific message type given by the 'type' argument.
 * If a call to PDBresponse times out in the wait_sec's given,
 * 0 will be returned, else a 1 is returned and the message and
 * mess_len arguments are filled in
 */
int
pdb_get_response(wait_sec, type, message, mess_len)
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
		ret = _PDBresponse(wait_sec, &received_type,
				  message, mess_len);
		/* For now, flag unexpected messages */
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




/* pdb_await
 * ---------
 * Alternatively polls (non-blocking select()) the message handler
 * file descriptor and the PDB (with PDBresponse).  Any mh messages
 * are sent to msg_incoming(), while PDB responses are dispatched
 * with pdb_dispatch().
 */
int
pdb_await()
{
	int ret = 1;

	int pdb_mtype;
	int pdb_mlen;
	char *pdb_msg;

	while (ret)
	{
		/* Next we poll PDB */
		if (PDBresponse(0, &pdb_mtype, &pdb_msg, &pdb_mlen) > 0)
		{
		   /* We need to dispatch a PDB message */
		   ret = pdb_dispatch(pdb_mtype, pdb_msg, pdb_mlen);
		}
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
		printf("%s\n",msg);
		break;
	   /* 
	    * Response to a PDB_REQUEST_PRODUCT request: 
	    */
	   case PDB_STATUS:

		break;

	   case PDB_PRODUCT:
	        pd = (PDBproduct *) msg;
		fprintf(stderr, "Incoming Product. ID: %d\n", pd->type);
		ptr = (char *) (msg + sizeof(PDBproduct) + 
				(pd->npts * sizeof(PDBpt)));
		if ((ptr + pd->text_size) > (msg + mlen))
		{
		     fprintf(stderr, 
			"Problem with PDB Message - Incompatible lengths\n");
		}
		else
		     print_polyline_product(pd, stderr);
	        break;

	   default:
		printf("Unexpected PDB response type %d\n",mtype);

	}
}


/*
 * Dump a LIST_PRODUCTS message to stdout.
 * msg is not modified in any way
 */
void
dump_list_products_msg(msg, mlen)
	char *msg;
	int mlen;
{
	char *ptr;

	for (ptr = msg; ptr < msg + mlen; ptr += strlen(ptr) + 1)
		printf("%s\n", ptr);
}



void
print_polyline_product(pd, file)
   PDBproduct *pd;
   FILE       *file;
{
   int         i,
               j,
               k;
   int         found,
               npoints;
   int         x,
               y;
   long        tm;
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
	    if (DEBUG2)
	       fprintf(file, "Line Segment End\n");
	    for (k = 0; k < npoints; k++)
	    {
	       if (DEBUG2)
		  fprintf(file, "\t %.2f, %.2f", xloc[k], yloc[k]);
	    }
	    if (DEBUG2)
	       fprintf(file, "\n");
	 }
      }

      if (npoints > 0)
      {				/* Plot any remaining points in segment
				 * without a terminator */
	 if (DEBUG2)
	    fprintf(file, "Line Segment End\n");
	 for (k = 0; k < npoints; k++)
	 {
	    if (DEBUG2)
	       fprintf(file, "\t %.2f, %.2f", xloc[k], yloc[k]);
	 }
	 if (DEBUG2)
	    fprintf(file, "\n");
      }
   }
}


#ifdef notdef


/************************************************************************
 * HANDLE_MESSAGE: Route the information in the incomming message to the
 *		 correct place.
 */

void
handle_message(sid, msg_type, seq_no, message, mess_len)
   int         sid;		/* server Id */
   int         msg_type;
   int         seq_no;		/* secquence number */
   char       *message;		/* The message */
   int         mess_len;	/* The message length */
{
   int         i,
               j;
   int         id,
               stat;		/* Product ID and status */
   int         found;		/* indicates correct info was found */
   server_info_t *server;

   PDBpt      *pt_ptr;
   PDBproduct *pd;
   PDBproduct *prod;
   product_t  *prodi;

   char       *ptr;
   char        token[128];


   found = 0;
   for (i = 0; i < gd.num_servers && found == 0; i++)
   {				/* Look through all server structs for a
				 * match */
      if (gd.server[i].server_id == sid)
      {
	 found = 1;
	 server = &(gd.server[i]);
	 switch (server->server_type)
	 {
	 case PRODUCT_SERVER:
	    switch (msg_type)
	    {
	    case PDB_CONNECT:
	       server->server_status = SERVER_UP;
	       break;

	    case PDB_DISCONNECT:
	       server->server_status = SERVER_DOWN;
	       break;

	    case PDB_PRODUCT:
	       pd = (PDBproduct *) message;
	       if (gd.debug1)
		  fprintf(stderr, "Incomming Product. ID: %d\n", pd->type);
	       found = 0;
	       for (i = 0; i < gd.num_products && found == 0; i++)
	       {
		  if (pd->type == gd.prod[i].product_id)
		  {
		     found = 1;
		     prod = &(gd.prod[i].pdb);
		     prodi = &(gd.prod[i]);
		  }
	       }
	       if (found)
	       {
		  pt_ptr = (PDBpt *) (message + sizeof(PDBproduct));
		  ptr = (char *) (message + sizeof(PDBproduct) + (pd->npts * sizeof(PDBpt)));
		  if ((ptr + pd->text_size) > (message + mess_len))
		  {
		     fprintf(stderr, "Problem with PDB Message - Incompatible lengths\n");
		  }
		  else
		  {
		     update_pdb_product(prod, pd, i, pt_ptr, ptr);
		     print_product(prod, stderr);
		  }
	       }
	       break;

	    case PDB_LIST_PRODUCTS:
	       server->server_status = SERVER_UP;
	       /* Find product ID's for PROD_CLIENT'd desired products */
	       match_product_list(message, mess_len);
	       break;

	    case PDB_PRODUCT_DESCRIPTION:
	       ptr = message;
	       if (STRtokn(&ptr, token, 128, " ") != NULL);	/* get ID token */
	       id = atoi(token);

	       found = 0;
	       for (i = 0; i < gd.num_products && found == 0; i++)
	       {
		  if (gd.prod[i].product_id == id)
		  {
		     found = 1;
		     prod = &(gd.prod[i].pdb);
		     strncpy(gd.prod[i].prod_description, ptr, 1024);	/* Copy description of
									 * product */
		  }
	       }

	       break;

	    case PDB_STOP_PRODUCT:	/* Currently never sent */
	       break;

	    case PDB_START:	/* Current never sent */
	       break;

	    case PDB_STOP:	/* Current never sent */
	       break;

	    case PDB_STATUS:
	       set_product_status(message);
	       break;

	    case PDB_DB_PRODUCT:
	       break;

	    case PDB_DB_GETTIME:
	       break;

	    case PDB_DB_REQUEST_PRODUCT:
	       break;

	    case PDB_DB_STOP_PRODUCT:
	       break;

	    case PDB_DB_GET:
	       break;
	    }
	 }
      }
   }
}

#endif



