/*
 * err2el.c --- Relays error messages from RAP's ERR package to the
 *		Zeb Event Logger.  Unfortunately, the mapping from ERR
 *		error levels to the EF_ error levels is not exactly 1-1
 *		This function must be registered as a user error handler
 *		with ERR as follows:
 *
    ERRinit(name, 0, NULL);
    ERRcontrol(ERR_CONTROL_SETTING);
    err = ERRuser(LogERRtoEL);
    ERRuserActive(err, 1);       * Make sure our handler is active *
 *
 * 		Unfortunately, this does not work real well with the SOCK
 *		library since SOCK applications fork SOCKprocess which
 *		simply inherits the flag settings and not the user handlers
 */

#if defined(SABER) || defined(lint)
#else
static char rcsid[] = "$Id: err2el.c,v 1.1 1992-07-03 18:03:13 granger Exp $";
#endif

#include "err.h"
#include "ingest.h"

extern void IngestLog();

/*
 * Log ERR messages to IngestLog
 */
void
LogERRtoEL(priority, message)
	int priority;
	char *message;
{
	int elflag;

	switch(priority)
	{
	   case ERR_ALERT:
	   case ERR_RESOURCE:
		elflag = EF_EMERGENCY;
		break;
	   case ERR_WARNING:
		elflag = EF_PROBLEM;
		break;
	   case ERR_INFO:
	   /* case ERR_DEBUG == ERR_INFO aaarrgh! */
		elflag = EF_INFO;
		break;
	   case ERR_PROGRAM:
		elflag = EF_PROBLEM;
		break;
	   default:
		elflag = EF_DEBUG;
		break;
	}
	IngestLog(elflag, message);
}

