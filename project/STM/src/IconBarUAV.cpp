! $Id: IconBarUAV.cpp,v 1.1 1997-06-02 22:55:22 granger Exp $
!
! UAV-specific iconbar menus, configs, and platforms



#ifndef UAV
!		entry 'MM5 model' 'display mm5' (dm$config = 'mm5')
!		entry 'MM5 cross sections' 'display mm5-xs' \
!		      (dm$config = 'mm5-xs')
!		entry 'Time-height and soundings' 'display iss-timeht' \
!			(dm$config = 'iss-timeht')
!		entry 'Sounding cross sections' 'display snd_xsect' \
!		        (dm$config = 'snd_xsect')
!		entry 'AERI LBL Clouds' 'display aericlouds' \
!			(dm$config = 'aericlouds')
!		entry 'RUC model' 'display ruc' (dm$config = 'ruc')
!		entry 'Oklahoma Mesonet' 'display ok-mesonet' \
!			(dm$config = 'ok-mesonet')
!		entry 'Wind Profiler Demo Network' 'display wpdn' \
!			(dm$config = 'wpdn')
#endif
!		entry 'AVHRR/GOES Satellite' 'display satellite' \
!			(dm$config = 'satellite')
!		entry 'UDF Track' 'display uav-track' (dm$config = 'uav-track')
!		entry 'NWS Surface Mesonet' \
!			'display Mesonet' (dm$config = 'Mesonet')


!	menu uav bitmap aircraft
!		title 'UAV Platforms'
!		line
!		entry 'UAV Instrument Track' 'PutScalar uav met_temp'
!		entry 'UAV Telemetry Track' \
!		   'PutScalar sgpuavtrack alt'
#ifdef notdef
!		entry 'UAV TDDR Time Series' \
!		   'PutScalar DsgpuavtddrUg1.a1 tddr_opd'
!		line
!		entry 'GOES vis (UAV project)' \
!		   'putc2 p_goes_uav platform sgpgoesvis.tmp field vis'
!		entry 'GOES ir (UAV project)' \
!		   'putc2 p_goes_uav platform sgpgoesir.tmp field ir'
!		line
!		submenu 'NGM plots from UAV' ngm-uav-menu
#endif
!	endmenu


	menu overlays
		entry 'ISS Sounding locations' \
			'require ISS-Send-Locs; ISS-Send-Locs sounding'
		entry 'ISS Profiler locations' \
			'require ISS-Send-Locs; ISS-Send-Locs profiler'
		entry 'ISS RASS locations' \
			'require ISS-Send-Locs; ISS-Send-Locs rass'

#ifdef notdef
		submenu 'VOR/DME' 'VORmenu'
		entry 'CLASS stations' 'putc0 c_sounding_loc'
		entry 'NWS soundings' 'putc0 c_nwssounding_loc'
		entry 'Profilers' 'putc0 c_profiler_loc'
		line
		submenu 'VOR/DME Range Rings' VORRings 
#endif

	endmenu

#ifndef UAV
define widget uav-config-menu intmenu 'UAV configurations'
	title 'UAV Configurations'
	line
	entry 'UDF Track' 'display uav-track' (dm$config = 'uav-track')
#ifdef notdef
	entry 'NGM Model' 'display NGM' (dm$config = 'NGM')
#endif
	entry '915 MHz Profiles' \
		'display Profiler915' (dm$config = 'Profiler915')
	entry '50 MHz Profiles' \
		'display Profiler50' (dm$config = 'Profiler50')
	entry 'NWS Surface Mesonet' 'display Mesonet' (dm$config = 'Mesonet')
endmenu
#endif

#ifndef UAV
	entry 'IDASS IOP Platforms' \
   'shell "dsdwidget -a -t NCAR\ IOP prof/ sounding/ surf/ wpdn mm5ncar &"'
	line
#endif


#ifdef notdef
define widget ngm-uav-menu intmenu 'ngm plots for the uav ngm platforms'
	title 'Nested Grid Model (UAV)'
	line
	entry 'Line Contours' \
		'putc1 p_uav_ngm representation "contour"' 
	entry 'Filled Contours' \
		'putc1 p_uav_ngm representation "filled-contour"' 
	entry 'Wind Vectors' \
		'sendout p_uav_ngm_vector pick'
endmenu
#endif


!	menu surface bitmap surface
!		title 'Surface Observations'
!		line
!		submenu 'NWS Mesonet' nwssurf
!#ifndef UAV
!		submenu 'Tower Platforms' tower-menu
!!		submenu 'Kansas Hourly' kansas-hourly
!!		submenu 'Kansas Daily' kansas-daily
!!		submenu 'Oklahoma 5-minute (a0)' oklahoma05a0
!!		submenu 'Oklahoma 5-minute (a1)' oklahoma05a1
!!		submenu 'Oklahoma 15-minute (a0)' oklahoma15a0
!!		submenu 'Oklahoma 15-minute (a1)' oklahoma15a1
!		line
!!		submenu 'Wind Profiler Demo Network' 'wpdn-menu'
!		submenu 'EBBR Instruments' 'ebbr-irgrid-menu'
!!		entry 'ISS Surface data' \
!!      'putc3 p_station platform #isssurf active-icon false icon iss-iss-small'
!#endif
!		submenu 'SMOS Instruments' 'smos-irgrid-menu'
!#ifdef notdef
!!		line
!!		entry 'ISS Surface data' 'ISS-Send-Stations'
!!		line
!!		entry 'Arkansas River Basin Precip' 'beep'
!#endif
!	endmenu



#ifdef UAV
		entry 'MFR 25m Central Facility (a1)' \
		   'PutScalar Dsgpmfr25mC1.a1 mfr_up_hemisp_broadband'
		entry 'CF Bellfort Laser Ceilometer (a1)' \
		   'PutScalar sgpblcC1.a1 cloud1'
		line
		submenu 'SIROS' siros
		submenu 'MWR Central Facility' mwr-cf-menu
#else
!		submenu 'AERI Central Facility' aeri-menu
!		line
!		submenu 'MWR Central Facility' mwr-cf-menu
!		submenu 'MWR Boundary Facility One' mwr-b1-menu
!		submenu 'MWR Boundary Facility Four' mwr-b4-menu
!		submenu 'MWR Boundary Facility Five' mwr-b5-menu
!		line
!		submenu 'MFRSR' mfrsr-menu
!		line
		submenu 'SIROS' siros
!		line
!		entry 'MFR 10m Central Facility (a1)' \
!		   'PutScalar Dsgpmfr10mC1.a1 mfr_up_hemisp_broadband'
!		entry 'MFR 25m Central Facility (a1)' \
!		   'PutScalar Dsgpmfr25mC1.a1 mfr_up_hemisp_broadband'
!		line
!		entry 'BSRN Central Facility (a1)' \
!		   'PutScalar sgpbsrnC1.a1 nip'
!		entry 'CF Whole Sky Imager' \
!		   'PutScalar sgpwsicloudC1.c1 mean_cloud_height'
!		entry 'CF Bellfort Laser Ceilometer (a1)' \
!		   'PutScalar sgpblcC1.a1 cloud1'
#endif


!	menu uav bitmap aircraft
!		title 'UAV Platforms'
!		line
!		entry 'UAV Instrument Track' 'PutScalar uav met_temp'
!		entry 'UAV Telemetry Track' \
!		   'PutScalar sgpuavtrack alt'
#ifdef notdef
!		entry 'UAV TDDR Time Series' \
!		   'PutScalar DsgpuavtddrUg1.a1 tddr_opd'
!		line
!		entry 'GOES vis (UAV project)' \
!		   'putc2 p_goes_uav platform sgpgoesvis.tmp field vis'
!		entry 'GOES ir (UAV project)' \
!		   'putc2 p_goes_uav platform sgpgoesir.tmp field ir'
!		line
!		submenu 'NGM plots from UAV' ngm-uav-menu
#endif
!	endmenu


#ifndef UAV
define widget uav-config-menu intmenu 'UAV configurations'
	title 'UAV Configurations'
	line
	entry 'UDF Track' 'display uav-track' (dm$config = 'uav-track')
#ifdef notdef
	entry 'NGM Model' 'display NGM' (dm$config = 'NGM')
#endif
	entry '915 MHz Profiles' \
		'display Profiler915' (dm$config = 'Profiler915')
	entry '50 MHz Profiles' \
		'display Profiler50' (dm$config = 'Profiler50')
	entry 'NWS Surface Mesonet' 'display Mesonet' (dm$config = 'Mesonet')
endmenu
#endif

#ifdef notdef
define widget goes-7-menu intmenu 'GOES-7 images'
	title 'GOES-7 Images'
	line
	entry 'GOES-7 Visible' \
	   'putc2 p_satellite platform g7vismerc field vas_visible'
	entry 'GOES-7 Channel 5 brightness' \
	   'putc2 p_satellite platform g7irmerc field vas_ir5'
	entry 'GOES-7 Channel 8 brightness' \
	   'putc2 p_satellite platform g7ir8merc field vas_ir8'
	entry 'GOES-7 Channel 12 brightness' \
	   'putc2 p_satellite platform g7irmerc field vas_ir12'
	entry 'GOES-7 Channel 5 radiances' \
	   'putc2 p_satellite platform g7radirmerc field vas_ir5'
	entry 'GOES-7 Channel 8 radiances' \
	   'putc2 p_satellite platform g7radir8merc field vas_ir8'
	entry 'GOES-7 Channel 12 radiances' \
	   'putc2 p_satellite platform g7radirmerc field vas_ir12'
endmenu
#endif /* notdef */

