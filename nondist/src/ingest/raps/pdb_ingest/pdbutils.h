
#ifndef _pdb_utils_h_
#define _pdb_utils_h_

extern Location *GetLocnsBuffer FP((unsigned long npts));
extern char *AscSysTime FP((long systime, TimePrintFormat fmt));
extern char *GetSubPlatform FP((ProductRecord *pr,
				long id_num, int *subid,
				PlatformId *platid));
extern void TriggerDataStore FP((
	ProductRecord *pr, ZebTime *when, PlatformId platid));
extern void TimerTriggerCall FP(( ZebTime *zt, void *param));
extern void SetIntAttr FP(( DataChunk *dc, int s, char *att, int val));
extern void SetFloatAttr FP(( DataChunk *dc, int s, char *att, float val));
extern void SetNowcCenterAttr FP(( DataChunk *dc, short x, short y));
extern void AppendNowcLocations FP((
	Location **locns,  /* Array to append to, may be moved */
	int start,	   /* Where to start adding at */
	Nowc_pt *shortpts, /* {short x; short y;}  */
	int  npts  	   /* # pts to add from shortpts */ ));

extern long Ctime6 FP(( Time6 tim ));
extern char *St6_time FP(( Time6 tim));
extern char *Stime6 FP(( Time6	tim ));

extern void StrToUpper FP((char *str));
extern void StrToLower FP((char *str));
extern time_t Time FP(( time_t *tptr ));
extern short CheckIdNum FP(( ProductRecord *pr, long idnum ));


/*----------------------------------------------------------------*/
/* Some common attribute names which are used for data products
 * to provide some kind of consistency among attributes: 	  */

#define AttLineType "line-type"			/* Integers */
#define AttColor "color"
#define AttId "id"
#define AttSubid "subid"
#define AttRegionId "region-id"
#define AttRegionType "region-type"
#define AttNowcastType "nowcast-type"

#define AttStartTime "start-time"		/* ASCII-expanded times */
#define AttGenerateTime "generate-time"
#define AttExpireTime "expire-time"

#define AttCenterLat "center_lat"		/* Floats */
#define AttCenterLon "center_lon"
#define AttPenupPoint "penup-point"
#define AttProbability "probability"
#define AttDbz "dbz"
#define AttPeriod "period"
#define AttStormTop "top(km)"
#define AttStormBase "base(km)"
#define AttStormVolume "volume(km3)"
#define AttStormAreaMean "area-mean(km2)"
#define AttStormRainFlux "rain-flux(m3/s)"
#define AttStormMass "mass(ktons)"
#define AttStormDxDt "dx-dt(km/hr)"
#define AttStormDyDt "dy-dt(km/hr)"
#define AttStormDvDt "dvol-dt(km3/hr)"
#define AttStormDaDt "darea-dt(km2/hr)"

/*
 * To convert i16 coords in decameters to float km
 */
#define FKM(c) ((float)(((float)(c))/100.0))

#endif /* ndef _pdb_utils_h_ */

