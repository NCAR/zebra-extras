!======================================================================
! Mesonets
!======================================================================

!
! Every mesonet platform must be a composite irregular grid, and the
! filetype will usually be netcdf.  Maxsamples is up to the subclass.
!
class Mesonet
	abstract
	organization	irgrid
	filetype	netcdf
	composite
endclass Mesonet

#ifndef UAV
!=======================================================================
! Kansas Mesonet
!=======================================================================

class KSMesonet Mesonet
	maxsamples	600
endclass

class KSMesonet-Station KSMesonet
	organization	scalar
endclass

class KSMesonet-Hourly KSMesonet
	subplats KSMesonet-Station kh1 kh2 kh3 kh4
endclass

class KSMesonet-Daily KSMesonet
	subplats KSMesonet-Station kd1 kd2 kd3 kd4
endclass

instance KSMesonet-Hourly sgpksuhrlymesoX1.b1
instance KSMesonet-Daily sgpksudlymesoX1.b1

#endif /* ! UAV */

!======================================================================
! NWS hourly surface network
!======================================================================

class NWSMesonet Mesonet
	maxsamples	24
	daysplit
endclass

class NWS-Station NWSMesonet
	organization	scalar
endclass

subplats NWSMesonet NWS-Station 3km adm bvo cnk cnu csm ddc eha emp end flv
subplats NWSMesonet NWS-Station foe fri fsi gag gck gld hbr hlc hut hys iab
subplats NWSMesonet NWS-Station ict ixd lbl lts mhk mlc ojc okc p28 pnc pwa
subplats NWSMesonet NWS-Station rsl sln tik top tul wdg

instance NWSMesonet sgpnwssurfX1.a1

#ifndef UAV
!====================================================================
! Oklahoma Mesonet
!====================================================================

class OKMesonet Mesonet
endclass

class OKMesonet-Station OKMesonet
	organization	scalar
endclass

subplats OKMesonet OKMesonet-Station ADAX ALTU ALVA ANTL
subplats OKMesonet OKMesonet-Station ARDM ARNE BEAV BESS
subplats OKMesonet OKMesonet-Station BIXB BLAC BOIS BOWL
subplats OKMesonet OKMesonet-Station BREC BRIS BBOW BUFF
subplats OKMesonet OKMesonet-Station BURB BURN BUTL BYAR
subplats OKMesonet OKMesonet-Station CALV CAMA CENT CHAN
subplats OKMesonet OKMesonet-Station CHER CHEY CHIC CLAR
subplats OKMesonet OKMesonet-Station CLAY CLOU COOK COPA
subplats OKMesonet OKMesonet-Station DURA ELRE ERIC EUFA
subplats OKMesonet OKMesonet-Station FAIR FORA FTCB FREE
subplats OKMesonet OKMesonet-Station GOOD GRAN GUTH HASK
subplats OKMesonet OKMesonet-Station HINT HOBA HOLL HOOK
subplats OKMesonet OKMesonet-Station HUGO IDAB JAYX KENT
subplats OKMesonet OKMesonet-Station KETC KING LAHO LANE
subplats OKMesonet OKMesonet-Station MADI MANG MARE MARS
subplats OKMesonet OKMesonet-Station MAYR MCAL MEDF MEDI
subplats OKMesonet OKMesonet-Station MIAM MINC MTHE NEWK
subplats OKMesonet OKMesonet-Station NORM NOWA OILT OKEM
subplats OKMesonet OKMesonet-Station OKMU PAUL PAWN PERK
subplats OKMesonet OKMesonet-Station PRYO PUTN REDR RETR
subplats OKMesonet OKMesonet-Station RING SALL SEIL SHAW
subplats OKMesonet OKMesonet-Station SKIA SLAP SPEN STIG
subplats OKMesonet OKMesonet-Station STIL STUA SULP TAHL
subplats OKMesonet OKMesonet-Station TALI TIPT TISH TULL
subplats OKMesonet OKMesonet-Station VINI WALT WASH WATO
subplats OKMesonet OKMesonet-Station WAUR WEAT WEBB WEST
subplats OKMesonet OKMesonet-Station WILB WIST WOOD WYNO
subplats OKMesonet OKMesonet-Station NINN ACME APAC

class OKMesonet-05 OKMesonet
	maxsamples	144
endclass

class OKMesonet-15 OKMesonet
	maxsamples	144
endclass

instance OKMesonet-05 sgp05okmX1.a0 sgp05okmX1.a1
instance OKMesonet-15 sgp15okmX1.a0 sgp15okmX1.a1

#endif /* ! UAV */

!=========================================================================
! Satellite
!
! All satellite images are regular grids in HDF from the SeaSpace converter
!
class Satellite
	directory	satellite
	instancedir	subdirclass
	inheritdir	copy
	organization	2dgrid
	filetype	hdf
endclass

class SatelliteOverlay Satellite
	organization	image
endclass

instance Satellite sgpavhrr12X1.a1 sgpavhrr12radX1.a1
instance Satellite sgpavhrr14X1.a1 sgpavhrr14radX1.a1
instance Satellite sgpgoes8X1.a1 sgpgoes8visX1.a1

#ifdef notdef	/* testing purposes only; no longer used */

instance Satellite g7ir8merc g7irmerc g7radir8merc g7radirmerc g7vismerc
instance Satellite n9avhrrmerc n9avhrrradmerc

!
! Overlay images, which all have only byte fields
!
instance SatelliteOverlay avhrr_sgp.state_lines avhrr_sgp.lat_lon
instance SatelliteOverlay goes_ir_sgp.state_lines goes_ir_sgp.lat_lon
instance SatelliteOverlay goes_vis_sgp.state_lines goes_vis_sgp.lat_lon

#endif

#ifdef notdef
!======================================================================
! Arkansas red basin river precipitation
!======================================================================

!instance ? sgpabrfcpcpX1.c1
#endif

#ifndef UAV
!======================================================================
! GOES Minnis
!======================================================================

class GOES-Minnis
	organization	nspace
	filetype	netcdf
endclass

instance GOES-Minnis sgpgoesminnisX1.c1
#endif /* ! UAV */

!======================================================================
! Wind Profiler Demonstration Network
!======================================================================

class WPDN-Profiler Profiler
endclass

class WPDN Mesonet
!
! When plotting...
!
	organization	irgrid
!
! When creating...
!
!	organization	nspace

	subplats WPDN-Profiler  1  2  3  4  5  6  7  8  9 10 11 12 13 14
	subplats WPDN-Profiler 15 16 17 18 19 20 21 22 23 24 25 26 27 28
	subplats WPDN-Profiler 29 30 31 32 33 34
endclass

instance WPDN Dsgp60wpdnwndsX1.a1

#endif /* ! UAV */

#ifndef UAV
!======================================================================
! Models
!======================================================================

class ModelGrid
	organization	3dgrid
	filetype	netCDF
	maxsamples	4
	model
	daysplit
endclass

class NGM ModelGrid
endclass

instance NGM sgpngm250X1.c1 Dsgpngm250derivX1.c1

class ETA ModelGrid
endclass

instance ETA sgpeta90X1.c1

class GRIBModel
	organization	nspace
	filetype	grib
	model
endclass

!
! Surface grids are handled as separate platforms which share the
! same files as the volume platforms.  Hence the copyparent attribute.
!
class GRIBSurface
	organization	nspace
	filetype	grib_sfc
	instancedir	copyparent
	model
endclass

instance GRIBModel sgprucsX1.a1 sgprucsanalX1.a1

instance GRIBModel mm5ncar
subplats mm5ncar GRIBSurface mm5ncar_sfc

#endif /* ! UAV */

#ifndef UAV
!=====================================================================
! MAPS
!=====================================================================

class MAPS
	organization	scalar
	filetype	netCDF
	maxsamples	8
	daysplit
endclass

class MAPS-All MAPS
	maxsamples	1
endclass

instance MAPS Dsgpmaps60X1.c1
instance MAPS-All Dsgpallmaps60X1.c1

#endif /* ! UAV */


#ifdef notdef
!
! NGM model
!
platform sgpngm250X1.ns.issue
	organization	nspace
	filetype	netCDF
	maxsamples	4
	daysplit
endplatform

platform sgpngm250X1.ns.valid
	organization	nspace
	filetype	netCDF
	maxsamples	4
	daysplit
endplatform
#endif

