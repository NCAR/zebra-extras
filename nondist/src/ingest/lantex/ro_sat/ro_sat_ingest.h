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
** $Id: ro_sat_ingest.h,v 1.2 2008-04-16 18:26:55 granger Exp $
**
*/

/************************************************************************
*                                                                       *
*                              ro_sat_ingest.h				*
*                                                                       *
*************************************************************************

                        Fri Apr  1 13:21:50 1994

       Description: declarations for RO Satellite Image ingest


       See Also: ROSatImageIngest.c overlay.c


       Author: C S Morse


       Modification History:


*/


/*
** System include files
*/

/*
** Local include files
*/

/*
** Definitions / macros / types
*/

/*
 * Useful stuff
 */

# define BETWEEN(x,lower,upper)	(((x)-(lower))*((x)-(upper)) <= 0)
# define DEG_TO_RAD(x)	((x)*0.017453292)
# define RAD_TO_DEG(x)	((x)*57.29577951)
# define DEG_TO_KM(x)	((x)*111.3238367) /* on a great circle */
# define OFFSET 0
# define MISSING_VAL 255

typedef struct {
  long nx, ny;
  long sub_nx, sub_ny;
  long sub_x0, sub_y0;
  double minx, miny;
  double deltax, deltay;
  double origin_lat, origin_lon;
  double altitude;
} grid_var_t;

/*
** External references / global variables 
*/

/*
** Forward functions
*/


