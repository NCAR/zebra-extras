/*		Copyright (C) 1994 by UCAR
**	University Corporation for Atmospheric Research
**		   All rights reserved
**
** No part of this work covered by the copyrights herein may be reproduced
** or used in any form or by any means -- graphic, electronic, or mechanical,
** including photocopying, recording, taping, or information storage and
** retrieval systems -- without permission of the copyright owner.
** 
** This software and any accompanying written materials are provided "as is"
** without warranty of any kind.  UCAR expressly disclaims all warranties of
** any kind, either express or implied, including but not limited to the
** implied warranties of merchantibility and fitness for a particular purpose.
** UCAR does not indemnify any infringement of copyright, patent, or trademark
** through use or modification of this software.  UCAR does not provide 
** maintenance or updates for its software.
**
** %W% %D% %T%
**
** %F% %E% %U%
**
** $Id: overlay.c,v 1.2 2008-04-16 18:26:55 granger Exp $
**
*/

/************************************************************************
*                                                                       *
*                              overlay.c				*
*                                                                       *
*************************************************************************

                        Fri Apr  1 11:48:32 1994

       Description: Routines for extracting and applying the coastline, 
                    borders, and lat/lon overlay from the RO satellite 
		    data


       See Also: ROSatImageIngest.c


       Author: C S Morse


       Modification History:


*/

# ifndef    LINT
static char RCSid[] = "$Id: overlay.c,v 1.2 2008-04-16 18:26:55 granger Exp $";
static char SCCSid[] = "%W% %D% %T%";
static char COPYRIGHTid[] = "Copyright 1994  National Center for Atmospheric Research.  All Rights Reserved.";
# endif     /* not LINT */


/*
** System include files
*/

# include <stdio.h>
# include <defs.h>
# include <sys/types.h>
/*
# include <unistd.h>
# include <errno.h>
# include <math.h>
# include <dirent.h>
# include <message.h>
# include <timer.h>
# include <string.h>
# include <sys/stat.h>
*/

/*
** Local include files
*/

# include "ro_sat_ingest.h"

/*
** Definitions / macros / types
*/

#define NVAL 1	/* number of overlay intensity values */

/*
** External references / global variables 
*/

static u_char overlay_value[NVAL] = { 1 };

/*
** Forward functions
*/

/************************************************************************
*                                                                       *
*                           extract_overlay				*
*                                                                       *
*************************************************************************

       int extract_overlay -- creates an overlay 

           Fri Apr  1 11:52:38 1994

       Method: extracts overlay pixels from a subgrid of a satellite 
               image and copies them into an array

       Inputs:

       Outputs:

       Globals:

       Author: C S Morse

       Modification History:

*/


int
extract_overlay( char *data_path, u_char *overlay_array, grid_var_t *grid )
{
    FILE *data_file;
    u_char *overlay,pixel;
    
    int maxLine,maxElement,minElement;
    int nLine,nElement;
    int oval;
    int i,j;
    

    /* Open the data file */

    data_file = fopen(data_path,"r");
    
    if (data_file == NULL) {
#ifdef USE_ZEB
	msg_ELog (EF_PROBLEM, "Error %d opening file '%s'\n", errno,
		  data_path);
#else
	fprintf(stderr,"Error opening file '%s'\n", data_path);
#endif
	return (-1);
    }

    /* Set up dimensions of grid to be read */
    maxLine = grid->sub_y0 + grid->sub_ny;
    minElement = grid->sub_x0;
    maxElement = grid->sub_x0 + grid->sub_nx;

    /* Extract the image data	*/
    overlay = overlay_array;
    
    nLine = 0;
    while (nLine < maxLine) {
	/* Just read through the first lines outside the subgrid */
	if (nLine < grid->sub_y0) {
	    for (j=0;j<grid->nx;j++) getc(data_file);
	} else {
	    nElement = 0;
	    i = 0;
	    overlay = overlay_array + ((nLine - grid->sub_y0) * grid->sub_nx);
	    while (nElement < grid->nx) {
		pixel = getc(data_file);

		/* Error checking */
		if (feof(data_file)) {
#ifdef USE_ZEB
		    msg_ELog (EF_PROBLEM, "Premature end of file '%s'\n",
			      data_path);

#else
		    fprintf(stderr,"ROSatImage Premature EOF for %s\n\r",
			  data_path);
#endif
		    fclose(data_file);
		    return(1);
		}
	      
		if (nElement >= minElement && nElement < maxElement) {  
		    oval = FALSE;
		    for (j=0;j<NVAL;j++) {
			if (pixel == overlay_value[j]) {
			    oval = TRUE;
			    break;
			}
		    }
		    overlay[i++] = (oval) ? pixel : MISSING_VAL;
		}
		nElement++;
	    }
	}
	
	nLine++;
    }
  
    fclose (data_file);
    return (0);

}

/************************************************************************
*                                                                       *
*                           apply_overlay				*
*                                                                       *
*************************************************************************

       int apply_overlay -- apply the overlay to another image

           Fri Apr  1 14:24:28 1994

       Method: use the overlay image as a mask

       Inputs: npoints	number of points in each of the arrays
               input	input data array to be overlaid
	       overlay	overlay array

       Outputs: output	output data array (can be the same as input)

       Globals: NONE

       Author: C S Morse

       Modification History:

*/

int
apply_overlay( long npoints, u_char *input, u_char *overlay, u_char *output )
{
    long i,j;
    
    for (i=0;i<npoints;i++) {
	
	output[i] = (overlay[i] != MISSING_VAL) ? overlay[i] : input[i]; 
    }
    return(0);
}


