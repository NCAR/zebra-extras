/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1991,1992 UCAR
 ** University Corporation for Atmospheric Research
 ** Proprietary and Confidential to UCAR
 ** Do not copy or distribute without authorization
 ** 1992/01/29 17:16:44
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/************************************************************************
 * track_server.c
 *
 * Track data routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * April 1992
 * modified May 92 for Product Server JCaron
 *
 *************************************************************************/

#include <malloc.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <values.h>

#include <tools_globals.h>
#include <err.h>
#include <utim.h>

#include <sockutil.h>
#include <servmap.h>
#include <smu.h>
#include <xdru.h>

#include "ts.h"

#define MAX_HOST_NAME_LEN 128
#define MAX_MESSAGE_LEN   10000

/*
 * file scope variables
 */
static int 	Connected = FALSE;
static int	Debug = TRUE;
static char	Host1[ MAX_HOST_NAME_LEN]; /* where server mapper is*/
static char	Host2[ MAX_HOST_NAME_LEN];
static tdata_basic_index_t Index;
static long 	Request_num = -1;
static int 	Tserver_fd = -1;
static int 	Track_data_mem_allocated = FALSE;

#define DPRINT if(Debug) printf
#define ufree free
#define umalloc malloc
#define urealloc realloc

/*************************************************************************
 * free_track_data_mem()
 *
 * frees up the track data struct arrays
 *
 *************************************************************************/

static void Free_track_data_mem()
     
    {
  	long icomplex, isimple;
  	tdata_basic_complex_params_t *c_params;
  	tdata_basic_simple_params_t **s_params_ptr;
  	tdata_basic_track_entry_t **t_entry_ptr;
  	tdata_basic_track_entry_t ***t_entry_ptr_ptr;

  	if (!Track_data_mem_allocated) 
	    return;

    	c_params = Index.complex_params;
    	s_params_ptr = Index.simple_params;
    	t_entry_ptr_ptr = Index.track_entry;
    
    	for (icomplex = 0; icomplex < Index.header.n_complex_tracks; icomplex++) 
	    {
      	    t_entry_ptr = *t_entry_ptr_ptr;

      	    for (isimple = 0; isimple < c_params->n_simple_tracks; isimple++) 
		{
		ufree((char *) *t_entry_ptr);
		t_entry_ptr++;
      		} 

      	    ufree((char *) *s_params_ptr);
      	    ufree((char *) *t_entry_ptr_ptr);
    
      	    t_entry_ptr_ptr++;
      	    s_params_ptr++;
      	    c_params++;
    	    } /* icomplex */

    	ufree((char *) Index.track_entry);
   	ufree((char *) Index.simple_params);
    	ufree((char *) Index.complex_params);

    	Track_data_mem_allocated = FALSE;
    }

/*********************************************************************
 * Read_reply.c
 *
 * Reads a reply packet from the server
 *
 * returns 0 on success, 1 on failure
 *
 *********************************************************************/

static int Read_reply(reply, from)
     tdata_reply_t *reply;
     char *from;
    {
  	SKU_header_t header;

  	if (SKU_readh(Tserver_fd, (char *) reply,
		(long) sizeof(tdata_reply_t),
		&header) < 0)
    	    return (1);

  	if (header.id != TDATA_REPLY_PACKET_ID) 
	    {
    	    ERRprintf(ERR_WARNING, "read reply from %s failure: %s: id %d != %d", 
		from, reply->text, header.id, TDATA_REPLY_PACKET_ID);
	    return (1);
	    }

  	reply->status = ntohl((u_long) reply->status);
  	return (0);
    }



/************************* Connect *************************************/
/***********************************************************************
 *
 * Request_notify()
 *
 */

static int Request_notify()
    {
  	tdata_request_t request;
  	tdata_reply_t reply;
  
  	if (!Connected)
    	    return (-1);

  	request.request = htonl((u_long) TDATA_REQUEST_NOTIFY);
  	Request_num++;

  	if (SKU_writeh(Tserver_fd, (char *) &request,
		 (long) sizeof(tdata_request_t),
		 (long) TDATA_REQUEST_PACKET_ID,
		 Request_num) < 0) 
	    {
    	    ERRprintf(ERR_WARNING, "Request_notify writeh failed");
    	    return (-1);
  	    }
  
  	if (Read_reply(&reply, "Request_notify")) 
	    {
    	    ERRprintf( ERR_WARNING, "Request_notify bad reply");
    	    return (-1);
 	    }
  
  	if (reply.status == TDATA_FAILURE) 
	    {
    	    ERRprintf(ERR_WARNING, "Request_notify TDATA FAILURE");
    	    return (-1);
  	    }
  
  	return (0);
    }

/***********************************************************************
 * Set_max_message_len()
 */

static int Set_max_message_len(len)
    long len;
    {
  	tdata_request_t request;
  	tdata_reply_t reply;
  
  	if (!Connected)
    	    return (-1);

  	request.request = htonl((u_long) TDATA_SET_MAX_MESSAGE_LEN);
  	request.u.max_message_len = htonl((u_long) len);
  	Request_num++;

  	if (SKU_writeh(Tserver_fd, (char *) &request,
		 (long) sizeof(tdata_request_t),
		 (long) TDATA_REQUEST_PACKET_ID,
		 Request_num) < 0) 
	    {
    	    ERRprintf(ERR_WARNING, "Set_max_message_len writeh failed");
    	    return (-1);
  	    }	
  
  	if (Read_reply(&reply, "set_max_len")) 
	    {
    	    ERRprintf(ERR_WARNING, "Set_max_message_len reply failed");
    	    return (-1);
  	    }
  
  	if (reply.status == TDATA_FAILURE) 
	    {
    	    ERRprintf(ERR_WARNING, "Set_max_message_len TDATA_FAILEURE");
  	    }
  
  	return (0);
    }

/***********************************************************************/
static int Verify_connect()
    {
    	tdata_request_t request;
    	tdata_reply_t reply;
  
    	if (!Connected)
    	    return (-1);

  	request.request = htonl((u_long) TDATA_VERIFY_CONNECT);
  	Request_num++;

  	if (SKU_writeh(Tserver_fd, (char *) &request,
		 (long) sizeof(tdata_request_t),
		 (long) TDATA_REQUEST_PACKET_ID,
		 Request_num) < 0) 
	    {
    	    ERRprintf(ERR_WARNING, "Verify_connect writeh failed");
            return (-1);
  	    }
  
  	if (Read_reply(&reply, "verify_connect")) 
	    {
    	    ERRprintf(ERR_WARNING, "ERROR - Verify_connect read failed");
    	    return (-1);
  	    }
  
  	if (reply.status == TDATA_FAILURE)
    	    {
	    ERRprintf(ERR_WARNING, "Verify_connect TDATA_FAILURE\n");
      	    return (-1);
  	    }
  
  	return (0);
    }


/*************************************************************************
 * Connect_to_tserver()
 *
 * Connects to track server as necessary. First checks to see if the port
 * or hostname has changed. If so, hangs up from current server and
 * makes the connection to the new one.
 *
 * Returns 0 on success, -1 on failure.
 *
 *************************************************************************/

int Connect_to_tserver(host, port)
    char *host;
    int port;
    {
  	static int current_port = -1;
  	static char current_host[MAX_HOST_NAME_LEN] = "";
  
  	/* if host or port has changed, hangup server 	*/
  	if (Connected)
    	    if (port != current_port || strcmp(host, current_host))
      		TS_hangup();
  
  	/* connect to server if required */
    	if (!Connected) 
	    {
    	    if ((Tserver_fd = SKU_open_client(host, port)) < 0) 
		{
      		ERRprintf(ERR_PROGRAM,"WARNING - Could not connect to server %s %d",
			host, port);
      		return(-1);
		}
    	    Connected = TRUE;
    
    	    /* verify the connection */
    	    if (Verify_connect())
      	    	return (-1);
    
    	    current_port = port;
    	    strcpy(current_host, host);
    
    	    /* set the max message len*/
    	    Set_max_message_len( MAX_MESSAGE_LEN);

    	    /* in realtime mode, request notification of new data */
    	    if (Request_notify())
	    	return (-1);
    	    }
  
  	return (0);
    }

/***********************************************************************
  int TS_connect( char *host1, char *host2)

  Connect to an operational track server.
  The server mapper lives on host1 and/or host2
  Return 0 on success, -1 on failure

 ************************************************************************/

int TS_connect( host1, host2)
    char *host1, *host2;
    {
	SERVMAP_request_t 	req;
	SERVMAP_info_t	  	*info;
	int		  	nservers, i;

	/* find out where the server is */
	memset( &req, 0, sizeof( SERVMAP_request_t));
	strcpy(req.server_type, SERVMAP_TYPE_STORM_TRACK);
	strcpy(req.instance, SERVMAP_INSTANCE_OPERATIONS);
	req.want_realtime = TRUE;

	/* save host names for reconnections */
	STRncopy( Host1, host1, MAX_HOST_NAME_LEN);
	STRncopy( Host2, host2, MAX_HOST_NAME_LEN);
	
	if (1 != SMU_requestInfo( &req, &nservers, &info, Host1, Host2))
	    return -1;

	for (i=0; i<nservers; i++)
	    {
	    if (0 == Connect_to_tserver( info[i].host, info[i].port))
		{
		printf("Connected to track server on %s %d\n",
			info[i].host, info[i].port);
		return 0;
		}
	    }
	
	return -1;
    }
	

/***********************************************************************/
/***********************************************************************
 *
 * TS_hangup()
 *
 */

int TS_hangup()     
   {
  	tdata_request_t request;
  	tdata_reply_t reply;

  	if (!Connected)
    	    return (0);

  	request.request = htonl((u_long) TDATA_HANGUP);
  	Request_num++;

  	if (SKU_writeh(Tserver_fd, (char *) &request,
		 (long) sizeof(tdata_request_t),
		 (long) TDATA_REQUEST_PACKET_ID,
		 Request_num) < 0) 
	    {
    	    ERRprintf(ERR_WARNING, "TS_hangup writeh");
  	    }
  
  	if (Read_reply(&reply, "hangup")) 
	    {
    	    ERRprintf(ERR_WARNING, "TS_hangup read reply failed\n");
  	    }
  	else if (reply.status == TDATA_FAILURE) 
	    {
    	    ERRprintf(ERR_WARNING, "TS_hangup TDATA_FAILURE\n");
  	    }

	TS_disconnect();
  	return (0);
    }


void TS_disconnect()
    {
  	SKU_close(Tserver_fd);
  	Free_track_data_mem();
  	Connected = FALSE;
    }


/********************************************************************/
/*********************************************************************
 * read_from_tserver_buffer()
 *
 * Reads data buffer, loading up the data as requested.
 * If necessary, reads a new packet
 *
 * returns 0 on success, -1 on failure
 *
 *********************************************************************/

static int Read_from_tserver_buffer(data, nbytes, id)
     
     char *data;
     long nbytes;
     int id;
     
   {
  	static char *read_buffer = NULL;
 	static long nbytes_buffer = 0;
  	static long nbytes_data = 0;
  	static long read_posn = 0;
  	long tot_bytes;
  	long struct_id;
  	SKU_header_t packet_header;
  	tdata_struct_header_t struct_header;
  
  	tot_bytes = nbytes + sizeof(tdata_struct_header_t);
  
  	if (read_posn + tot_bytes  > nbytes_data) 
	    {
            /* read a new packet into the buffer */
    	    if (SKU_read_header(Tserver_fd, &packet_header) < 0)
      		return (-1);
    
    	    if (packet_header.len > nbytes_buffer) 
		{
      		if (read_buffer == NULL)
		    read_buffer = umalloc ((u_int) packet_header.len);
      		else
		    read_buffer = urealloc (read_buffer, (u_int) packet_header.len);
      
      		nbytes_buffer = packet_header.len;
    		}
    
    	    if (SKU_read(Tserver_fd, read_buffer, packet_header.len, -1)
		    != packet_header.len)
      		return (-1);
    
    	    nbytes_data = packet_header.len;
    	    read_posn = 0;
  	    } /* if (read_posn + tot_bytes > nbytes_data) */
  
  	/* check struct id  */
  	bcopy(read_buffer + read_posn, (char *) &struct_header, sizeof(tdata_struct_header_t));
  
  	struct_id = ntohl((u_long) struct_header.id);
  
  	if (struct_id != id) 
	    {
            ERRprintf(ERR_WARNING, "read_from_tserver_buffer structure id %d != %d", 
	    	id, struct_id);
    	    return (-1);
  	    }
  
  	/* copy buffer to data  */
  	bcopy(read_buffer + read_posn + sizeof(tdata_struct_header_t),
		data, (int) nbytes);
  	read_posn += tot_bytes;
  
  	return (0);
    }

/***********************************************************************
 *
 * read_basic_tserver_data()
 *
 * Read basic track data set
 *
 ****************************************************************************/

static int Read_basic_tserver_data()
     
    {
  	long icomplex, isimple, ientry;

  	long plot_start_time = MAXLONG;
  	long plot_end_time = 0;
  
  	tdata_basic_header_t *header;
  	tdata_basic_complex_params_t *complex_params;
  	tdata_basic_simple_params_t *simple_params;
  	tdata_basic_track_entry_t *track_entry;
  
  	/* main header */
  	if (Read_from_tserver_buffer((char *) &Index.header,
			       (long) sizeof(tdata_basic_header_t),
			       TDATA_BASIC_HEADER_ID)) 
	    {
    	    ERRprintf( ERR_WARNING, "read_basic_tserver_data packet out of order");
    	    TS_hangup();
	    return -1;
  	    }
  
  	header = &Index.header;
  
  	XDRU_tohl((u_long *) header, (u_long) sizeof(tdata_basic_header_t));
  
  	/* allocate memory */
  
  	Index.complex_params = (tdata_basic_complex_params_t *)
    		umalloc((u_int) (header->n_complex_tracks *
		     sizeof(tdata_basic_complex_params_t)));
  
  	Index.simple_params = (tdata_basic_simple_params_t **)
    		umalloc((u_int) (header->n_complex_tracks *
		     sizeof(tdata_basic_simple_params_t *)));
  
  	Index.track_entry = (tdata_basic_track_entry_t ***)
    		umalloc((u_int) (header->n_complex_tracks *
		     sizeof(tdata_basic_track_entry_t **)));
  
    	Track_data_mem_allocated = TRUE;

 	/* complex tracks */
  	for (icomplex = 0; icomplex < header->n_complex_tracks; icomplex++) 
	    {
    	    complex_params = Index.complex_params + icomplex;
    
    	    if (Read_from_tserver_buffer((char *) complex_params,
				 (long) sizeof(tdata_basic_complex_params_t),
				 TDATA_BASIC_COMPLEX_PARAMS_ID)) 
		{
      		ERRprintf(ERR_WARNING, "read_basic_tserver_data packet out of order");
    	    	TS_hangup();
	    	return -1;
    		}
    
    	    XDRU_tohl((u_long *) complex_params,
	     	(u_long) sizeof(tdata_basic_complex_params_t));

    	    /* allocate memory */
    	    Index.simple_params[icomplex] = (tdata_basic_simple_params_t *)
		umalloc((u_int) (complex_params->n_simple_tracks *
			 sizeof(tdata_basic_simple_params_t)));
    
    	    Index.track_entry[icomplex] = (tdata_basic_track_entry_t **)
      		umalloc((u_int) (complex_params->n_simple_tracks *
		       sizeof(tdata_basic_track_entry_t *)));
    
    	    /* simple tracks */
    	    for (isimple = 0; isimple < complex_params->n_simple_tracks; isimple++) 
		{
      		simple_params = Index.simple_params[icomplex] + isimple;
      
      		if (Read_from_tserver_buffer((char *) simple_params,
				   (long) sizeof(tdata_basic_simple_params_t),
				   TDATA_BASIC_SIMPLE_PARAMS_ID)) 
		    {
		    ERRprintf( ERR_WARNING, "read_basic_tserver_data packet out of order");
    	    	    TS_hangup();
	    	    return -1;
      		    }
      
      		XDRU_tohl((u_long *) simple_params, (u_long) sizeof(tdata_basic_simple_params_t));

      		/* allocate memory */
      		Index.track_entry[icomplex][isimple] = (tdata_basic_track_entry_t *)
			umalloc((u_int) (simple_params->duration_in_scans *
			     sizeof(tdata_basic_track_entry_t)));
      
      		/* track file entries */
      		for (ientry = 0; ientry < simple_params->duration_in_scans; ientry++) 
		    {
		    track_entry = Index.track_entry[icomplex][isimple] + ientry;
	
		    if (Read_from_tserver_buffer((char *) track_entry,
				     (long) sizeof(tdata_basic_track_entry_t),
				     TDATA_BASIC_TRACK_ENTRY_ID)) 
			{
	  		ERRprintf( ERR_WARNING, "read_basic_tserver_data packet out of order");
			TS_hangup();
	    	 	return -1;
			}
	
		    XDRU_tohl((u_long *) track_entry, (u_long) sizeof(tdata_basic_track_entry_t));

		    /* keep track of start and end time for tracks */
		    if (track_entry->time < plot_start_time)
	  		plot_start_time = track_entry->time;

		    if (track_entry->time > plot_end_time)
	  		plot_end_time = track_entry->time;

      		    } /* ientry */
      
    		} /* isimple */
    
  	    } /* icomplex */

  	return (0);
    }

/***********************************************************************
 *
 * tserver_get_data()
 *
 */

static int Tserver_get_data(mode, source, track_num,
			    time, time_margin)
     
     int mode;
     int source;
     long track_num;
     long time;
     long time_margin;
     
    {
  	tdata_request_t request;
  	tdata_reply_t reply;
  
  	if (!Connected)
    	    return (-1);

  	request.request = htonl((u_long) TDATA_REQUEST_DATA);
 	request.u.mode = htonl((u_long) mode);
  	request.source = htonl((u_long) source);
  	request.track_num = htonl((u_long) track_num);
  	request.time = htonl((u_long) time);
  	request.time_margin = htonl((u_long) time_margin);
  
  	Request_num++;

  	if (SKU_writeh(Tserver_fd, (char *) &request,
		 (long) sizeof(tdata_request_t),
		 (long) TDATA_REQUEST_PACKET_ID,
		 Request_num) < 0) 
	    {
    	    ERRprintf(ERR_WARNING, "tserver_get_data writeh failed");
    	    return (-1);
  	    }
  
  	if (Read_reply(&reply, "get_data")) 
	    {
       	    ERRprintf(ERR_WARNING, "tserver_get_data reply failed");
    	    return (-1);
  	    }
  
  	if (reply.status == TDATA_FAILURE) 
	    {
    	    ERRprintf(ERR_WARNING, "tserver_get_data failure: %s", reply.text);
    	    return (-1);
  	    }
  
  	if (Read_basic_tserver_data())
    	    return (-1);
  
  	return (0);
    }

/*************************************************************************
 * TS_getdata( tdata_basic_index_t *index)
 *
 * get track data from track server
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

tdata_basic_index_t *TS_getdata()
    {
  	/* free up memory as necessary  */
  	Free_track_data_mem();

  	/* connect to the server as necessary */
  	if (!Connected)
    	    if (TS_connect( Host1, Host2))
      		return NULL;

  	/*  get the data */
  	if (Tserver_get_data( TDATA_BASIC_ONLY, TDATA_REALTIME, TDATA_ALL_AT_TIME, 0, 0))
	    return NULL;
	else
	    return &Index;

#ifdef NOTNOW
	{
	UTIMstruct utime;
	time_t	time;

	utime.year = 1992;
	utime.month = 6;
	utime.day = 11;
	utime.hour = 22;
	utime.min = 0;
	utime.sec = 0;
	time = UTIMdate_to_unix( &utime);
  	if (Tserver_get_data( TDATA_BASIC_ONLY, TDATA_ARCHIVE,
		 TDATA_ALL_AT_TIME, time, 600))

	    return NULL;
	else
	    return &Index;
	}
#endif
    }

/*********************************************************************
 * TS_check_new_data()
 *
 * Returns TRUE if new track data is available, FALSE otherwise
 *
 *********************************************************************/

static int Read_notify()
    {
  	SKU_header_t header;
  	tdata_reply_t reply;

  	/* read in the reply */

  	if (SKU_readh(Tserver_fd, (char *) &reply,
		(long) sizeof(tdata_reply_t), &header) < 0)
    	    return (-1);
  
  	/* if this is a notification packet, return successful.
   	 * Otherwise, return failure.
   	 */
  	if (header.id == TDATA_NOTIFY_PACKET_ID)
    	    return (0);
  	else
    	    return (-1);
    }


int TS_check_new_data()
    {
  	fd_set read_fd;
  	struct timeval timeout;
	int ret;
     
  	/* connect to the server as necessary */
  	if (!Connected)
    	    if (TS_connect( Host1, Host2))
      		return FALSE;

  	/* use a select to check if there is any message to be read*/
  	FD_ZERO(&read_fd);
  	FD_SET(Tserver_fd, &read_fd);
  	memset( &timeout, 0, sizeof(struct timeval));

  	if (0 >= (ret = select(FD_SETSIZE, &read_fd, (fd_set *)0, (fd_set *)0, &timeout)))
	    {
	    if (ret < 0)
		ERRprintf( ERR_WARNING, "TS select returned %d", ret);
	    return FALSE;
	    }
  
  	if (FD_ISSET(Tserver_fd, &read_fd)) 
	    {
    
    	    /* server wants to notify client that data is available */
    	    if (Read_notify()) 
		{
	      	ERRprintf( ERR_WARNING, "check_new_data read error ");
      		TS_hangup();
		return FALSE;
    		}
	    return (TRUE);
      	    }

    	return (FALSE);
    }

