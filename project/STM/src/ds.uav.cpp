!======================================================================
! Other things that will have unique data paths
!======================================================================
!
! UAV aircraft tracks
!
platform sgpuavtrack
	organization	scalar
	filetype	netcdf
	maxsamples	(6*3600)
	mobile
endplatform

#ifdef notdef
!
! UAV on-board radiometer package; files lack location information so their
! data are only good for xygraphs and not tracks.
!
platform DsgpuavtddrUg1.a1
	organization	nspace
	filetype	netcdf
	maxsamples	65000
endplatform
#endif

!======================================================================
! MRC platforms
!======================================================================

class AircraftTrack
	organization	scalar
	filetype	netcdf
	maxsamples	21600
	mobile
endclass

instance AircraftTrack uav-now uav chase-now chase

class Boundary
	filetype	zeb
	organization	outline
endclass	

instance Boundary moa ifp

