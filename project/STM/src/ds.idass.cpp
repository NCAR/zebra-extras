
!=======================================================================
! IDASS '93 IOP ISS Platforms
!=======================================================================

!
! ISSish stuff.  Profiler and RASS are identical, but might as well keep
! them separate.  These classes are distinct from the Profiler and RASS
! classes above since ISS uses a different netCDF layout.
!
class ISS-Profiler
	organization	1dgrid
	filetype	netcdf
	maxsamples	100
	daysplit
endclass

class ISS-RASS
	organization	1dgrid
	filetype	netcdf
	maxsamples	100
	daysplit
endclass

class ISS-Sounding
      	organization	scalar
	filetype	netcdf
	maxsamples	5000
	mobile
endclass

instance ISS-Sounding sounding/saltfork sounding/dodge sounding/kingfisher 
instance ISS-Sounding sounding/kingman sounding/norman sounding/pawhuska 
instance ISS-Sounding sounding/sgp sounding/topeka

instance ISS-RASS rass/saltfork_r rass/redrock_r

instance ISS-Profiler prof/saltfork_h prof/redrock_h prof/redrock_l 
instance ISS-Profiler prof/redrockw_h prof/redrockw_l 
instance ISS-Profiler prof/redrockr_h prof/redrockr_l 
instance ISS-Profiler prof/redrockp_h prof/redrockp_l 
instance ISS-Profiler prof/redrock_mom

!
! Single surface stations
!
class Surface
      	organization	scalar
	filetype	netcdf
	maxsamples	720
endclass

instance Surface surf/saltfork_s surf/redrock_s 

