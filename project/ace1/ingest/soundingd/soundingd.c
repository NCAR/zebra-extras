# include <sys/un.h>
# include <stdio.h>
# include <fcntl.h>
# include <string.h>
# include <signal.h>
# include <errno.h>
# include <math.h>	/* for fabs() */
# include <sys/select.h>

# include <netdb.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>

# define TRUE	1
# define FALSE	0

int	ListenFd = -1;
int	Timeout = FALSE;
int	PipeBroke = FALSE;

/*
 * Prototypes
 */
static void	sig_handler (int sig);
static inline void	swap4 (char *bytes);
static inline void	swap2 (char *bytes);




main (int argc, char *argv[])
{
	int	i, cfd, port, failcount, nbytes, nwrote;
	int	ListenFd, client_fd, out_fd;
	long	t;
	fd_set	fdset;
	char	inbuf[128], fname[128], tstring[16];
/*
 * We expect a port number to listen on
 */
	if (argc != 2 || (port = atoi (argv[1])) == 0)
	{
		fprintf (stderr, "Usage: %s <port_num>\n", argv[0]);
		exit (1);
	}
/*
 * Do all our time stuff in GMT
 */
	putenv ("TZ=GMT");
	t = time (0);
	fprintf (stderr, "Sounding daemon started %s\n", ctime (&t));
/*
 * Catch broken pipes, since they happen every time a client disconnects
 */
	signal (SIGPIPE, sig_handler);
	signal (SIGALRM, sig_handler);
	signal (SIGINT, sig_handler);
/*
 * Open a socket for our server port, trying again every second until we 
 * actually get it
 */
	failcount = 0;

	while ((ListenFd = open_server_sock (port)) < 0)
	{
		if (! failcount++)
			fprintf (stderr, "Waiting for server socket to clear");

		fprintf (stderr, ".");
		fflush (stderr);
		sleep (1);
	}

	if (failcount)
		fprintf (stderr, "\n");
/*
 * Now loop forever, waiting for clients on our port and writing out what 
 * they give us.
 */	
	while (1)
	{
	/*
	 * Wait for a client
	 */
		FD_ZERO (&fdset);
		FD_SET (ListenFd, &fdset);
		select (FD_SETSIZE, &fdset, NULL, NULL, NULL);
		if ((client_fd = get_next_client (ListenFd)) >= 0)
		{
			t = time (0);
			fprintf (stderr, "Client connect @ %s", ctime (&t));
		}
		else
			continue;
	/*
	 * Now read from our client and write to our output file until
	 * they stop sending stuff
	 */
		out_fd = -1;

		while (1)
		{
		/*
		 * Allow up to five minutes between successive lines of input
		 * (Allow infinite time if we're between soundings)
		 */
			if (out_fd >= 0)
				alarm (300);
			else
				alarm (0);
		/*
		 * Wait until there's something to read (or we get a signal)
		 */
			FD_ZERO (&fdset);
			FD_SET (client_fd, &fdset);
			select (FD_SETSIZE, &fdset, NULL, NULL, NULL);
		/*
		 * Did a signal break us out of the select?
		 */
			if (Timeout || PipeBroke)
			{
				t = time (0);
				if (Timeout)
					fprintf (stderr, 
						 "Sounding time out @ %s\n",
						 ctime (&t));

				if (PipeBroke)
					fprintf (stderr, 
						 "Client broke pipe @ %s\n",
						 ctime (&t));
				close (out_fd);
				out_fd = -1;

				Timeout = FALSE;
				PipeBroke = FALSE;

				continue;
			}
		/*
		 * Data are waiting, so read 'em
		 */
			nbytes = read (client_fd, inbuf, sizeof (inbuf));

			if (nbytes <= 0)
			{
				if (nbytes < 0)
				{
					fprintf (stderr, 
					    "Error %d reading from client\n",
					     errno);
				}
				
				t = time (0);
				fprintf (stderr, "Disconnect @ %s\n", 
					 ctime (&t));
				
				close (client_fd);

				if (out_fd)
					close (out_fd);

				break;
			}
		/*
		 * If no output file yet, open a new output file, with the
		 * current time as part of its name.
		 */
			if (out_fd < 0)
			{
				t = time (0);
				fprintf (stderr,
					 "Begin new file @ %s", ctime (&t));

				cftime (tstring, "%y%m%d.%H%M%S", &t);
				sprintf (fname, "%d.%s.raw", port, tstring);

				out_fd = open (fname, O_WRONLY | O_CREAT, 
					       0775);
				if (out_fd < 0)
				{
					fprintf (stderr, 
					     "Error %d opening file '%s'\n",
					     errno, fname);
					exit (1);
				}
			}
		/*
		 * write out what we just read
		 */
			if ((nwrote = write (out_fd, inbuf, nbytes)) <= 0)
				fprintf (stderr, 
					 "Error %d writing %d bytes to '%s'\n",
					 errno, nwrote);
		}
	}
}




/*
 * open_server_sock:	Open up an Internet stream socket for use as a server
 *			and puts it into listen mode.
 *	Returns proto filedescriptor:  use this in next accept call
 *	OR :  	-1 = Could not open socket
 *		-2 = Could not bind to specified port
 *		-3 = Could not fill in socket name 
 *		-4 = Could not listen 
 *
 * Written by F. Hage 2/88
 * Latest Update :
 */
int
open_server_sock (port)
int	port;
{	
	int	sfd, ival, size, name_len;
	static struct sockaddr_in loc_soc;	/* local socket info */

	/*  get a file destriptor for the connection to the remote port */
	if ((sfd = socket (AF_INET,SOCK_STREAM,0)) == -1) 
	{
		fprintf (stderr, "Could not set up socket\n");
		return(-1);
	}

	/* establish non-blocking I/O */
	fcntl (sfd, F_SETFL, O_NDELAY);
	 
	loc_soc.sin_port = htons (port);
	loc_soc.sin_family = AF_INET;
	loc_soc.sin_addr.s_addr = htonl (INADDR_ANY);

	/* bind to a local port */
	if (bind (sfd, (struct sockaddr *) &loc_soc, sizeof (loc_soc)) < 0) 
	{
		close (sfd);
		return (-2);
	}

	/* retrieve the name of (information about) the local socket */
	name_len = sizeof(loc_soc);
	if (getsockname (sfd, (struct sockaddr *) &loc_soc, &name_len) < 0) 
	{
		fprintf (stderr, "Getsockname failure\n");
		return (-3);
	}

	if (listen(sfd,5) < 0 ) 
	{
		fprintf (stderr, "Listening failure \n");
		return (-4);
	}
	return (sfd);
}



/*
 * get_next_client: Gets the next client and waits until client has connected 
 */
int
get_next_client (sfd)
int	sfd; /* proto filedescriptor of socket in listen mode */
{
	int	name_len;
	int	sockfd;			/* socket file descriptor */
	
	static union sunion 
	{
		struct sockaddr_in sin;
		struct sockaddr_un sund;
	} sadd;
	
	name_len = sizeof(struct sockaddr_in);
	if ((sockfd = accept (sfd, (struct sockaddr *) &sadd, &name_len)) < 0) 
		return (-1);
/* 
 * establish blocking I/O - otherwise our write() calls return
 * "Operation would block", rather than just waiting briefly
 */
	fcntl (sockfd, F_SETFL, 0);
	
	return(sockfd);
}



static void
sig_handler (int sig)
/*
 * Deal with various signals
 */
{
	switch (sig)
	{
	    case SIGPIPE:
		PipeBroke = TRUE;
		break;
	    case SIGALRM:
		Timeout = TRUE;
		break;
	    case SIGINT:
		fprintf (stderr, "Exiting on SIGINT\n");
		if (ListenFd >= 0)
			close (ListenFd);
		exit (1);
	}
/*
 * Set handlers again under SVR4
 */
	signal (SIGPIPE, sig_handler);
	signal (SIGALRM, sig_handler);
}
