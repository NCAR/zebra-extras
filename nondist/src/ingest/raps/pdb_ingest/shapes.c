/*
 * shapes.c --- Draw some simple shapes in a locations array for
 *		boundary data
 *
 * $Id: shapes.c,v 1.1 1993-06-28 05:37:11 granger Exp $
 */


#include "defs.h"
#include "math.h"

#if !defined(lint) && !defined(SABER)
static char rcsid[] =
   "$Id: shapes.c,v 1.1 1993-06-28 05:37:11 granger Exp $";
#endif

extern Location *GetLocnsBuffer FP(( unsigned long num ));
extern bool cvt_Origin FP((double lat, double lon));
extern void cvt_ToLatLon FP((double x, double y, float *lat, float *lon));

void
Diamond(lbuf, start, x, y, xradius, yradius, rotation)
	Location **lbuf;	/* diamond will be put in */
	int *start;		/*    *lbuf[*start ... *start+4] */
	float x, y;		/* lat, lon of center of diamond */
	float xradius;		/* len (km) of xradius (bef. rotation) */
	float yradius;		/* len (km) of yradius (bef. rotation) */
	float rotation;		/* angle (deg) of rotation */
{
	float dx, dy;
	double radians;
	Location *locns;

	radians = rotation * 3.1415927 / 180.0;

	cvt_Origin(x,y);  /* Set our ogigin for converting
			   * dx,dy offsets to lat,lon */

	/*
	 * Make sure Location buffer has enough room for 4 corners,
	 * plus one to close shape, and one penup-point
	 */
	*lbuf = GetLocnsBuffer(*start + 6);
	locns = *lbuf + *start;

	/* Ends of x-axis: */
	dx = xradius*(float)cos(radians);
	dy = xradius*(float)sin(radians);
	cvt_ToLatLon(dx,dy,&locns[0].l_lat, &locns[0].l_lon);
	cvt_ToLatLon(-dx,-dy,&locns[2].l_lat, &locns[2].l_lon);

	/* Ends of y-axis: */
	dx = 0 - yradius*(float)sin(radians);
	dy = yradius*(float)cos(radians);
	/*
	 * The eqns above are equivalent to rotating the points an additional
	 * 90 deg since these pts are rotated from the horizontal axis:
	 * cos(a + pi/2) = -sin(a)
	 * sin(a + pi/2) = cos(a)
	 */
	cvt_ToLatLon(dx,dy,&locns[1].l_lat, &locns[1].l_lon);
	cvt_ToLatLon(-dx,-dy,&locns[3].l_lat, &locns[3].l_lon);

	locns[5].l_lat = 0.0;	/* Set the penup-point with 0.0 */
	locns[5].l_lon = 0.0;
	locns[5].l_alt = 0.0;
	locns[4] = locns[0];	/* Complete the shape */
	*start += 5;		/* Update end of lbuf array index */
}



