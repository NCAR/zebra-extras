/*		Copyright (C) 1987,88,89,90,91 by UCAR
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
# include <defs.h>
# include <message.h>
# include <timer.h>
# include <DataStore.h>

# define STRLEN	30
# define BADVAL -32768
# define BUFLEN	1000
# define MAXOURS 10
# define KM_PER_NM (6080.0 / (5280.0 * 0.621))
# define M_PER_FT 0.30480
# define A_FEW 20
# define PI 3.141592654
# define R_EARTH 6372.0


/*
 * Global stuff for the LLP ingest module.
 */
char	Modem[STRLEN], DialOut[STRLEN];
int	BaudRate; 
float	RadarLat, RadarLon;

/*
 * File descriptor of device file where black box is.
 */
int	Fd;

/*
 * The data object to store.
 */
DataObject	Dobj;

/*
 * Command constants go here.
 */
# define AIC_GO		1
# define AIC_DIAL	2
