#ifndef SOCK_WAS_INCLUDED
#define SOCK_WAS_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>

/* types of sockets */
#define SOCK_INET AF_INET
#define SOCK_UNIX AF_UNIX
#define SOCK_SHMEM 0x100 /* not implemented */

/* Notify parameters */
#define SOCK_NOTIFY_TIMEOUT 1
#define SOCK_NOTIFY_CONNECT 2
#define SOCK_NOTIFY_LOSTMSG 3

/* SOCKsetServiceFile() parameters */
#define SOCK_ENV  0
#define SOCK_FILE 1
#define SOCK_OPS  2

/* exported functions */

extern int SOCKinit( /* char *applic, key_t key, int max_servers, int max_clients */);
extern void SOCKexit( /* void */ );

extern int SOCKnotify( /* int sid, int what, int param, void Notify() */ );
extern int SOCKsetServiceFile( /* int where, char *name */);
extern int SOCKbeginOperations( /* void */ );


extern int SOCKclient ( /* char *service, int buffer_size, int retry_secs */);
extern int SOCKclientNamed ( /* char *service, int sock_type, char *name, 
				int port, int buffer_size, int retry_secs */);

extern int SOCKserver( /* char *service, int buffer_size, int wait_sec,
			  int max_conns */);
extern int SOCKserverNamed( /* char *service, int sock_type, char *name, 
		int port, int buffer_size, int wait_sec, int max_conns */);


extern int SOCKreadMessage( /* int *sid, int *record_type, long *seqno, 
	char **message, int *message_nbytes, int wait_sec */);

extern int SOCKwriteMessage( /* int sid, int record_type, char *message, 
			       int message_nbytes */);


#endif
