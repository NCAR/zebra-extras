/**********************************************************************
 * Variety of conversion and thermodynamic routines. P.Neilley NCAR/RAP
 *
 * Converted to C - Mike Dixon, March 1994
 *
 ***************************************************************   
 * All routines assume that missing or bad data is any value
 * less than or equal to -999. All routines return -9999. if the
 * quantity is uncomputable.
 ***************************************************************
 *
 * Wind Routines:  dir = wind direction in degrees
 *                 u   = east-west wind component (arbitrary units)
 *                 v   = north-south "     "   
 *                    (= or total wind speed in some routines)
 *
 *   pr_sped(u,v)    returns wind speed given u,v
 *   pr_uwnd(v,dir)  returns u-component given speed and direction
 *   pr_vwnd(v,dir)  returns v-component given speed and direction
 *   pr_drct(u,v)    returns wind direction given u,v components
 *
 *
 * Thermodynamic Routines:
 *                 - all pressures are assumed to be in millibars.
 *                 - all temperatures(t)/dew points(td) in deg C
 *                   EXCEPT WHERE NOTED
 * 
 *  pr_vapr(td)    returns vapour pressure given td.  If t is passed
 *                   instead of td, return is saturation v.p.
 *  pr_relh(t,td)  returns relative humidity (%) given t, td.
 *  pr_thta(t,p)   returns potential temperature given t,p.
 *  pr_mixr(td,p)  returns mixing ratio given td,p.  If t is passed
 *                   instead of td, returns saturation m.r. (g/kg).
 *  pr_thte(p,t,td)returns equivalent pot. temp. given p,t,td.
 *  pt_tlcl(t,td)  returns the temperature of the lifted condensation
 *                 level (IN KELVIN) given t,td.
 *  pr_tmst(thte,p,tguess).  This routine returns the temerature of
 *                 a parcel of air with a given eqiv. pot. temp (thte)
 *                 lifted to level p.  Tguess is a first guess at
 *                 the value (or set=0, for routine determined first
 *                 guess).  Tguess in KELVIN, return is in KELVIN.
 * pr_rhdp(t,rh)   returns dewpoint given t, relative humidity.
 * pr_twet(p,t,td) returns wet-bulb t, given t,td,p.
 * pr_tvrt(p,t,td) returns virtual temp given t,td,p
 *************************************************************/

#include <math.h>

#define MISSING_VAL -999.0
#define MISSING_RET -9999.0
#define DEG_TO_RAD 0.0174532

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * prototypes
 */

double pr_vwnd(double v, double dir);
double pr_drct(double u, double v);
double pr_vapr(double t);
double pr_relh(double t, double td);
double pr_thta(double t, double p);
double pr_mixr(double td, double p);
double pr_thte(double p, double t, double td);
double pr_tlcl(double t, double td);
double pr_tmst(double tthe, double p, double tguess);
double pr_rhdp(double t, double relh);
double pr_twet(double p, double t, double td);
double pr_tvrt(double p, double t, double td);

/***************************************************************
 * Wind type routines
 */

double ps_sped(double u, double v)
     
{
  if (u > MISSING_VAL && v > MISSING_VAL) {
    return (sqrt(u * u + v * v));
  } else {
    return (MISSING_RET);
  }

}

double pr_uwnd(double v, double dir)

{

  if (v > MISSING_VAL && dir > MISSING_VAL) {
    return (-sin(dir * DEG_TO_RAD) * v);
  } else {
    return (MISSING_RET);
  }

}

double pr_vwnd(double v, double dir)

{

  if (v > MISSING_VAL && dir > MISSING_VAL) {
    return (-cos(dir * DEG_TO_RAD) * v);
  } else {
    return (MISSING_RET);
  }

}

double pr_drct(double u, double v)

{

  double dirn;
  
  if (u > MISSING_VAL && v > MISSING_VAL) {

    if (v == 0.0 && u == 0.0) {
      dirn = 0.0;
    } else {
      dirn = atan2(-u, -v) / DEG_TO_RAD;
    }

    if (dirn < 0.0)
      dirn += 360.0;

    return (dirn);

  } else {

    return (MISSING_RET);

  }

}

/*
 * Thermodynamic routines
 */

double pr_vapr(double t)

{
  if (t > MISSING_VAL) {
    return (6.112 * exp((17.67 * t)/(t + 243.5)));
   } else {
     return (MISSING_RET);
   }
}

double pr_relh(double t, double td)

{

  double ratio;

  if (t > MISSING_VAL && td > MISSING_VAL) {
    ratio = pr_vapr(td) / pr_vapr(t);
    return (100.0 * ratio);
  } else {
    return (MISSING_RET);
  }
}

double pr_thta(double t, double p)

{
  if (p > 0.0 && t > MISSING_VAL) {
    return ((t + 273.15) * pow((1000.0 / p), (2.0 / 7.0)));
  } else {
    return (MISSING_RET);
  }
}

double pr_mixr(double td, double p)

{
  double corr, e;

  if (p > 0.0 && td > MISSING_VAL) {
    corr = (1.001 + ((p - 100.0) / 900.)  * 0.0034);
    e = pr_vapr(td) * corr;
    return (0.62197 * (e / (p-e)) * 1000.0);
  } else {
    return (MISSING_RET);
  }
}

double pr_thte(double p, double t, double td)

{
  double rmix, e, thtam, thte;

  if (p > 0.0 && t > MISSING_VAL && td > MISSING_VAL) {
    rmix = pr_mixr(td, p);
    e = (2.0 / 7.0) * (1.0 - (0.001 * 0.28 * rmix));
    thtam = (t + 273.15) * pow((1000.0 / p), e);
    thte = (thtam * exp( (3.376 / pr_tlcl(t, td) -0.00254) *
			(rmix * (1.0 + 0.81 * 0.001 * rmix))));
    return (thte);
  } else {
    return (MISSING_RET);
  }
}

double pr_tlcl(double t, double td)

{
  double tk, dk;

  if (t > MISSING_VAL && td > MISSING_VAL) {
    tk = t + 273.15;
    dk = td + 273.15;
    return ((1.0 / ( 1.0 / (dk - 56.0) + log(tk / dk) / 800.0)) + 56.0);
  } else {
    return (MISSING_RET);
  }
}

double pr_tmst(double thte, double p, double tguess)

{
  long i;
  double tg, tmax, tgnup, tenu, tenup;
  double cor, tgnu;

  if (thte > 0.0 && p > 0.0 && tguess >= 0.0) {

    tg = tguess;

    if (tg == 0.0) {
      tmax = MAX(thte - 270.0, 0.0);
      tg = (thte - 0.5 * pow(tmax, 1.05)) * pow((p / 1000.0),  0.2);
    }

    tgnu = tg - 273.15;

    for (i = 0; i < 100; i++) {

      tgnup = tgnu + 1.0;
      tenu =  pr_thte (p, tgnu, tgnu);
      tenup = pr_thte (p, tgnup, tgnup);

      if ((tenu > 0.0) && ( tenup > 0.0)) {

	cor = (thte - tenu) / (tenup - tenu);
	tgnu = tgnu + cor;
	if (fabs(cor) <= 0.01 ) {
	  return (tgnu + 273.15);
	}

      } else {

	return (MISSING_RET);

      } /* if ((tenu > 0.0) ... */

    } /* i */

  } else {
    
    return (MISSING_RET);

  }

}

double pr_rhdp(double t, double relh)

{
  double eps, vapr, top, bottom;

  if (t > MISSING_VAL && relh > MISSING_VAL) {
    eps = 6.112;
    vapr = relh * pr_vapr(t) / 100.0;
    top = 243.5 * (log(eps) - log(vapr));
    bottom = log(vapr) - log(eps) - 17.67;
    return (top / bottom);
  } else {
    return (MISSING_RET);
  }
}

double pr_twet(double p, double t, double td)

{
  long iters = 0;
  double Vl = 2.5e6;
  double Cp = 1004.0;
  double dt = 9.9e9;
  double top, bottom;
  double twet, twet_new;
  double smixr, rmixr;

  if (t > MISSING_VAL && td > MISSING_VAL && p > 0.0) {
    top = t;
    bottom = td;
    while ((iters < 100) && (dt > 0.1)) {
      iters++;
      twet = (top + bottom) / 2.0;
      rmixr = pr_mixr(td, p) / 1000.0;
      smixr = pr_mixr(twet, p) / 1000.0;
      twet_new = t - (Vl / Cp) * (smixr - rmixr);
      dt = fabs(twet - twet_new);
      if (twet_new < twet) {
	top = twet;
      } else {
	bottom = twet;
      }
    } /* while */

    if ((iters < 100) || (dt < 1.0)) {
      return (twet);
    } else {
      return (MISSING_RET);
    }

  } else {

    return (MISSING_RET);

  }

}

double pr_tvrt(double p, double t, double td)

{
  double rmix, tk;
  double eps = 0.62197;

  if (t > MISSING_VAL && td > MISSING_VAL && p > 0.0) {
    rmix = pr_mixr(td, p) / 1000.0;
    tk = t + 273.15;
    return ((t + 273.15) * (1.0 + rmix / eps) / (1.0 + rmix) - 273.15);
  } else {
    return (MISSING_RET);
  }
}
