/*
 * $Id: pdbecho.c,v 1.1 1992-07-03 18:22:16 granger Exp $
 *
 * A support program for testing PDB programs with raw, pre-recorded data.
 * The data is stored with the -record option in the pdblib.c interface.
 * This program either describes the data in text format, or filters specific
 * products and echoes them raw.  The input is expected to be in 
 * PdbPacketHdr format (defined in pdblib.h).  See pdbecho -help
 * for specific option information.
 *
 */
/* Options:
 * -realtime	Emulate real-time delays between packets
 * -purge	Filter out non-data packets from stream
 * -t<n>	Remove this type from stream
 * +t<n>	Include this type in stream, and exclude others not
 *		explicitly included
 *
 * See usage()
 */

#if !defined(SABER) && !defined(lint)
static char rcsid[] = "$Id";
#endif

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "pdblib.h"   /* For the header structure */
#include "pdb.h"
#include "prod_structs.h"

void IdentifyProduct();

int
main(argc, argv)
   int argc;
   char *argv[];
{
	int realtime = 0;
	int purge = 0;
	int exclude[64];
	int nexclude = 0;
	int include[64];
	int ninclude = 0;
	int echo_binary = 0;

	int fd = 0;	/* stdin */
	int fdout = 1;	/* stdout */
	PdbPacketHdr hd;
	char msg[10000];
	PDBproduct *pd = (PDBproduct *)msg;
	int ptype;
	time_t last = 0;
	int i,j;
	void usage();

	/*
	 * Remove all arguments from command line so that only
	 * filenames (if any) remain
	 */
	j = 1;
	while (j < argc)
	{
	   if (!strcmp(argv[j],"-realtime"))
		realtime = 1;
	   else if (!strcmp(argv[j],"-purge"))
		purge = 1;
	   else if (!strncmp(argv[j],"-t",2))
		exclude[nexclude++] = atoi(argv[j]+2);
	   else if (!strncmp(argv[j],"+t",2))
		include[ninclude++] = atoi(argv[j]+2);
	   else if (!strcmp(argv[j],"-help") ||
		    !strcmp(argv[j],"-h"))
		usage(argv[0]);
	   else
	   	{ ++j; continue; }
	   for (i=j+1; i<argc; ++i) argv[i-1] = argv[i];
	   --argc;
	}
	if ((ninclude > 0) || (nexclude > 0) || (purge))
		echo_binary = 1;

	/*
	 * If filenames left on command line, use those,
	 * else use stdin.  All output is to stdout.
	 */
	j = 1;
	do {

	   if (argc < 2) fd = 0;	/* Take from stdin */
	   else
	   {
		fd = open(argv[j++], O_RDONLY);
		if (fd<0)
		{
			perror(argv[j-1]);
			exit(1);
		}
	   }

	   while (read(fd, (char *)&hd, sizeof(hd)))
	   {
		if (realtime && !purge)
		{
			if (!last) 
				last = hd.timestamp;
			else if (hd.timestamp > last)
				sleep(hd.timestamp - last);
		}
		read(fd, msg, hd.mlen);
		/*
		 * Remove bad packets
		 */
		if ((hd.mtype<1) || (hd.mtype>100000))
			continue;
		/* 
		 * First ascertain whether this packet is data or not
		 * If not, check for purging.
		 */
		if ((hd.mtype == 10) || (hd.mtype > 1000))
		{
			/* We have data, so filter it, if necessary */
			ptype = (hd.mtype > 1000) ? (hd.mtype) : (pd->type);
	
			/*
			 * If there are any includes, then we only pass data
			 * messages whose types match one of the types in the
			 * include array
			 */
			if (ninclude > 0)
			{
				/* Continue if this type is not in array */
				for (i = 0; i < ninclude; ++i)
					if (include[i] == ptype)
						break;
				if (i >= ninclude)
					continue;
			}
			else
			{
				/*
				 * Otherwise we just reject any that 
				 * match any types in the exclude array
				 */
				for (i = 0; i < nexclude; ++i)
					if (exclude[i] == ptype)
						break;
				if (i < nexclude)
					continue;
			}

		}
		else	/* Have non-data, see if we should exclude it */
		{
		   if (purge)
			continue;
		}
		/* 
		 * We now have a packet that we know we should echo.
		 * Determine if we echo in binary or describe in text
		 */
		if (echo_binary)
		{
			write(fdout,(char *)&hd,sizeof(hd));
			write(fdout,msg,hd.mlen);
		}
		else
		{
			printf("Message type: %d,",hd.mtype);
			printf("  length: %d",hd.mlen);
			printf("  Timestamp: %s",
				asctime(gmtime(&(hd.timestamp))));
			if (hd.mtype == PDBGET_TEST_MSG)
			   printf("Message: %s\n",msg);
			else if (hd.mtype == PDB_PRODUCT)
			   IdentifyProduct(hd.mtype, msg, hd.mlen);
			else if (hd.mtype > 1000)
			   printf("Raw product, rap ID: %d\n",hd.mtype);
		}

	   } /* while read(fd) */

	   close(fd);

	} while (j<argc);  /* Do for each filename on cmd-line */

	exit(0);
}


void
IdentifyProduct(mtype, msg, mlen)
	int mtype;
	char *msg;
	int mlen;
{
	PDBproduct *pd = (PDBproduct *)msg;

	/* PDB_FORMAT_POLYLINE */
	printf("Polyline %hu: txt %hu, npts %hu, id %li, subid %li, start %s",
	       pd->type, pd->text_size, pd->npts, pd->id, 
	       pd->subid, ctime(&pd->start_time));
}



void
usage(prog)
	char *prog;
{
printf("%s [options] [filenames ...]\n",prog);
printf("	-realtime	Mimic realtime delays, off by default\n");
printf("	-purge		Exclude all non-data messages\n");
printf("	-t<n>		Exclude this data type from output\n");
printf("	+t<n>		Include this data type, while excluding all\n");
printf("			others not likewise explicitly included\n");
printf("	-h,-help	Show this usage information\n");
printf("\n-purge, -t, and +t options echo raw binary product messages.\n");
printf("Otherwise, input will be described in text output.\n");
printf("<n> is a product id, 1-?, for polyline product types, or a RAP\n");
printf("4-digit product id for raw format.\n");
printf("If no filenames are given, data will be taken from standard input\n");
printf("-realtime is only effective in the interpretive mode\n");
exit(0);
}


