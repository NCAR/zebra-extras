/*
 * fields.c
 *
 * Field data manipulation
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80523, USA
 *
 * March 1994
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

#include "aws_ingest.h"

/*
 * Init_fields.c
 *
 * Init fields to missing value.
 */

void init_fields(long nfields,
		 float *fields_merged,
		 float *fields_prev,
		 float *fields_prev_valid)

{

  long ifield;
  
  for (ifield = 0; ifield < nfields; ifield++) {

    *fields_merged = DC_BADVAL;
    fields_merged++;

    *fields_prev = DC_BADVAL;
    fields_prev++;

    *fields_prev_valid = DC_BADVAL;
    fields_prev_valid++;

  } /* ifield */

}

/*
 * Merge_fields.c
 *
 * Module for merging fields from data at successive time, in order to
 * fill in missing data as appropriate. The latest data fields, if valid,
 * override previously determined fields.
 *
 * The incoming time series is also filtered for sudden jumps in the data.
 *
 * The objective of this routine is to create a merged_fields
 * array which is most likely to be correct. The problems 
 * encountered are (a) missing values and (b) data which is
 * incorrect but has a valid value. For example, the relative
 * humidity value sometimes drops by an order of magnitude because
 * of a transmission error in the data line, say a drop from 91%
 * to 9.1%.
 *
 * To correct for these problems data is read in from a number of
 * the most recent records. More recent data overrides less recent.
 * A sudden jump in the data (exceeding max_change) is ignored.
 *
 * The rules implemented here are as follows:
 *
 *   Set prev_valid values to DC_BADVAL - this is done in
 *   init_fields().
 *
 *   If input_val is good {
 *     if prev_valid is bad {
 *       merged_val = input_val
 *       prev_valid = input_val
 @	 prev = input_val
 *     } else {
 *       if (change < limit) {
 *         merged_val = input_val
 *         
 *     }
 *   } else {
 *     use prev_valid value 
 *   }
 * 
 *
 *   
 */

void merge_fields(long nfields,
		  float *fields_in,
		  float *fields_merged,
		  float *fields_prev,
		  float *fields_prev_valid,
		  double *max_change)


{

  long ifield;
  double diff;

  for (ifield = 0; ifield < nfields; ifield++) {
 
#ifdef CDBUG
      fprintf(stderr,"M: ifield %ld %f\n\r",ifield,*fields_in);
#endif   
    if (*fields_in != DC_BADVAL) {

      /*
       * input data is good
       */

      if (*fields_prev_valid != DC_BADVAL) {

	/*
	 * prev value is good, so if necessary we can
	 * use it instead of the input data
	 */
	
	diff = fabs((double) *fields_in - (double) *fields_prev);
	
#ifdef CDBUG
      fprintf(stderr,"M: diff/max %lf/%lf %f\n\r",
      diff,*max_change,*fields_prev);
#endif   
	if (diff < *max_change) {
	
	  /*
	   * no sudden jumps in data, so use input value
	   * and set valid value for later use
	   */

	  *fields_merged = *fields_in;
	  *fields_prev_valid = *fields_in;
	  
	} else {

	  /*
	   * sudden jump, so use prev valid value
	   */
#ifdef CDBUG
	    fprintf("Merge: Sudden jump: %f to %f using latter\n\r",
		    *fields_in, *fields_prev_valid);
#endif	    
	  
	  *fields_merged = *fields_prev_valid;

	} /* if (diff < *max_change) */

	/*
	 * set prev val to input val
	 */
#ifdef CDBUG
      fprintf(stderr,"M: setting fields_prev %f\n\r",*fields_in);
#endif   

	*fields_prev = *fields_in;

      } else {

	/*
	 * prev_valid value is bad, so we have no
	 * choice but to use the input value
	 */

	*fields_merged = *fields_in;
	*fields_prev_valid = *fields_in;
	*fields_prev = *fields_in;
	
      } /* if (*fields_prev_valid != DC_BADVAL) */

    } /* if (*fields_in != DC_BADVAL) */
    
    fields_in++;
    fields_merged++;
    fields_prev++;
    fields_prev_valid++;
    max_change++;

  } /* ifield */

}

