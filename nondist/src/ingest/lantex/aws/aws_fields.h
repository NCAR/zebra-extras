/*
 * aws_fields.h
 *
 * field numbers for Royal Observatory, Hong Kong, automatic
 * weather station data. This is ingested from a serial stream
 * and placed into files by aws_ingest. The files are read
 * by aws_ingest to ingest them into ZEB.
 *
 * Mike Dixon, RAP, NCAR, March 1994
 *
 * field numbers for Campbell automatic weather station data.
 *
 * Cory Morse, RAP, NCAR, March 1994
 */

#ifndef aws_fields_h
#define aws_fields_h

#define N_CAMPBELL_FIELDS 14

#define PRES_CB 0
#define PRES_MAX_CB 1
#define PRES_MIN_CB 2
#define TDRY_CB 3
#define RH_CB 4
#define WSPD_CB 5
#define WDIR_CB 6
#define WDIR_SDEV_CB 7
#define WSPD_MAX_CB 8
#define WSPD_MIN_CB 9
#define WSPD_SDEV_CB 10
#define RAIN_CB 11
#define SOLRAD_CB 12
#define BATTV_CB 13

#define N_RO_FIELDS 14

#define WDIR_RO 0
#define WSPD_RO 1
#define WSPD_MAX_RO 2
#define TDRY_RO 3
#define TWET_RO 4
#define DP_RO 5
#define RH_RO 6
#define PRES_RO 7
#define CPRES0_RO 8
#define RAINA01_RO 9
#define VIS_RO 10
#define WDIR_SDEV_RO 11
#define WSPD_SDEV_RO 12
#define SOLRAD_RO 13

#endif
