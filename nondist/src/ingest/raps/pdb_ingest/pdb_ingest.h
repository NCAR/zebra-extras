/*
 * $Id: pdb_ingest.h,v 1.2 1993-03-24 23:00:28 granger Exp $
 *
 * Global type defines and macros for pdb_ingest modules,
 * particularly the ProductRecord type definition and the
 * MAX_SUBPLATFORMS macro and MaxSubplatforms global variable
 */

#ifndef _pdb_ingest_h_
#define _pdb_ingest_h_

#include "pdblib.h"

/*===============================================================*/
/* Products Table:
 * This array, indexed by product id (NOT the 4-digit record id),
 * holds a ProductRecord for every known product (and then some).
 * Each product has a status, origin (usually MHR), 3-char code,
 * name, description, current input format (RAW or POLYLINE),
 * and two function pointers for handling either format.
 * The gist of the product info will be read in from the 
 * products.pdbs text file.  This file is especially important
 * for RAW format since it provides the 4-digit RAP packet id for
 * the raw structure to be ingested.  Products are inserted into the
 * table with InsertProduct(), and info about the product can be
 * changed or added as it comes in (such as getting a description).
 *
 * Three 'maps' exist for accessing product records, each returns
 * a pointer to a product record, or null if one not found: 
 *
 * 1) PdbIdMap  - An array indexed by the PDBS id
 *
 * 2) PRptrMap   - A NULL terminated array of PDBS pointers in order
 *		  of the PDBS id, ideal for traversing all products
 *
 * 3) RapIdMap()  A function which returns the product record ptr
 *		  given the 4-digit RAP id
 *
 * The first two map arrays will be updated automatically by
 * InsertProduct().  All operations on product records will be
 * done with pointers to a record.
 */

/*
 * These have been taken from John Caron's (RAP) PDBS code
 */
#define PROD_CODE_LEN	4	/* Includes '\0' */
#define PROD_DESC_LEN	256
#define PROD_NAME_LEN	16
#define PROD_PLAT_LEN	16

#define MAX_PROD_ID	40
#define MAX_PRODUCTS	(MAX_PROD_ID+1)
#define MAX_SUBPLATFORMS 20

typedef enum { 
	   ProdStatInactive = 0, 
	   ProdStatActive = 1 
	}
	ProdStatus; 	/* Coincides with server codes */

typedef struct _ProductRecord {

	int pdb_id;	/* This product's id, and its tbl index */
	long rap_id;	/* The 4-digit RAP id */
	long next_id;	/* subid of most recent product of this type */
	char code[PROD_CODE_LEN];
	char name[PROD_NAME_LEN];
	char desc[PROD_DESC_LEN];
	char platform[PROD_PLAT_LEN]; /* platform name in Zeb */
	PlatformId platid;	/* The Zeb DataStore identity */

	/* To mimic ideal behavior, a LRU replacement strategy is used
	 * to assign platform subids to incoming product id numbers (which
	 * don't necessarily fall into the range of sub id's).
	 *
	 * id_numbers[] stores the id_number last used with a subid, -1 if none
	 * next_oldest[] points to the oldest id_num, i.e. the array slot
	 * which will be used for the next product with a new id_num
	 */
	long id_numbers[MAX_SUBPLATFORMS];
	short next_oldest[MAX_SUBPLATFORMS];
	short index_newest;
	short index_oldest;
	ProdStatus status;
	int format;	/* For the day the server can differentiate */
	float origin_lat;
	float origin_lon;
	float angle;	/* Origin of product grid, usually MHR */
	time_t last;	/* Last time data received for this prod */
	short requested;/* non-zero if this product is being requested */

	/*___ The PDB_PRODUCT msg handlers ___*/

	PdbMsgHandler raw_handler;
	PdbMsgHandler line_handler;

} ProductRecord, *PRptr;

/* Says John Caron of RAP:
 * All origins are from Mile High:
 * generally x,y are in km * 100, where positive y axis is True North.
 */

#define MHR_ORIGIN_LON (-104.7568) /* long - MHR */
#define MHR_ORIGIN_LAT (39.8783)   /* lat - MHR */

/* Useful defines: */

#define DEFAULT_PROD_FORMAT PDB_FORMAT_POLYLINE
#define DEFAULT_RAW_HANDLER (PdbMsgHandler)DefaultRawHandler
#define DEFAULT_LINE_HANDLER (PdbMsgHandler)IngestPolylineProduct
/*
 * This is the string which will prefix product codes to
 * form the platform name
 */
#define PLATFORM_PREFIX "rap_"
/* Maximum number of subids to give any pdb product */
extern int MaxSubplatforms;

#endif

