/*
** Copyright 1994  National Center for Atmospheric Research
** All Rights Reserved
**
** %W% %D% %T%
**
** %F% %E% %U%
**
** $Id: rap_realtime_archive.c,v 1.1 1997-02-07 18:03:51 granger Exp $
**
** $Log: not supported by cvs2svn $
*/

/************************************************************************
*                                                                       *
*                              rap_realtime_archive.c			*
*                                                                       *
*************************************************************************

                        Fri Apr  8 13:00:41 1994

       Description: Write out realtime aws data for RAP to grab and 
                    ingest.


       See Also:


       Author: C S Morse


       Modification History:


*/

# ifndef    LINT
static char RCSid[] = "$Id: rap_realtime_archive.c,v 1.1 1997-02-07 18:03:51 granger Exp $";
static char SCCSid[] = "%W% %D% %T%";
static char COPYRIGHTid[] = "Copyright 1994  National Center for Atmospheric Research.  All Rights Reserved.";
# endif     /* not LINT */


/*
** System include files
*/

# include <string.h>

/*
** Local include files
*/

# include "aws_ingest.h"

/*
** Definitions / macros / types
*/

/*
** External references / global variables 
*/

extern long archive_time;
extern int archive_realtime_rap;

/*
** Forward functions
*/

/************************************************************************
*                                                                       *
*                           archive_realtime				*
*                                                                       *
*************************************************************************

       int archive_realtime -- write line of raw data off to a file

           Fri Apr  8 13:04:30 1994

       Method: creates/finds file based on station name and time of
               current ingest and appends current line

       Inputs: data_file_name
               line

       Outputs: altered archive file

       Globals: archive_realtime_rap
                archive_time
		#define ARCHIVE_DIR

       Author: C S Morse

       Modification History:

*/

int
archive_realtime( char *data_file_name, char *line )
{
    char work[100],archive_file_name[100],*station,*dot;
    FILE *archive_file=NULL;
    
    /* If global flag not set, just return */
    if (!archive_realtime_rap) return(0);
    
    /* Create filename for archive file from input data filename */
    strcpy(work,data_file_name);
    station = strrchr(work,'/');
    station++;
    dot = strstr(work,".dat");
    *dot = '\0';
    sprintf(archive_file_name,"%s/aws.%ld\0",
	    ARCHIVE_DIR,archive_time);
    

    /* Open the file */

    if ((archive_file = fopen(archive_file_name,"a")) == NULL) {
	return(-1);
    }
    
    /* Write the line of data */
    fprintf(archive_file,"%6s %s",station,line);

    /* Close the archive file */
    fclose(archive_file);
    
    return(0);
    
}


/************************************************************************
*                                                                       *
*                           save_archive				*
*                                                                       *
*************************************************************************

       int save_archive -- save archives for reading

           Fri Apr  8 14:03:30 1994

       Method: move new archive files to a different directory for reading

       Inputs: NONE

       Outputs: archive file moved to new directory

       Globals: archive_realtime_rap
                #define ARCHIVE_DIR

       Author: C S Morse

       Modification History:

*/

int
save_archive()
{
    char command[256];
    
    if (!archive_realtime_rap) return(0);
    

    sprintf(command,"mv %s/aws* %s/read",ARCHIVE_DIR,ARCHIVE_DIR);

    system(command);
    return(0);
    
}



