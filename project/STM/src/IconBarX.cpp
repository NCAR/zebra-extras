!
! IconBar for external datasets, like satellites and models.
!

#ifdef notdef
!	menu model bitmap model
!		title 'Model Grids'
!		line
!		entry 'RUC Contours' \
!			'require ModelAdd; ModelAdd sgprucsX1.a1 pt yellow'
!		entry 'RUC Wind Vectors' \
!		   'require ModelAdd; ModelWindsAdd sgprucsX1.a1 yellow 50'
!		entry 'RUC Analysis Contours' \
!		   'require ModelAdd; ModelAdd sgprucsanalX1.a1 pt yellow'
!		entry 'RUC Analysis Winds' \
!		   'require ModelAdd; ModelWindsAdd sgprucsanalX1.a1 yellow 50'
!		line
!		entry 'MM5 Contours' \
!			'require ModelAdd; ModelAdd mm5ncar pres green'
!		entry 'MM5 Wind Vectors' \
!			'require ModelAdd; ModelWindsAdd mm5ncar green 50'
!		entry 'MM5 Surface Contours' \
!		   'require ModelAdd; SfcModelAdd mm5ncar_sfc soil_temp brown'
!		entry 'MM5 Surface Winds' \
!		   'require ModelAdd; SfcModelWindsAdd mm5ncar_sfc brown 50'
!		line
!		entry 'NGM Contours' 'sendout p_ngm pick'
!		entry 'NGM Wind Vectors' 'sendout p_ngm_winds pick'
!	endmenu
#endif /* notdef */

#ifdef notdef
!	menu satellite bitmap satellite
!		title 'Satellite Images'
!		line
!		entry 'GOES-8 5-channel (4 km)' \
!		   'putc2 p_raster platform sgpgoes8X1.a1 field gvar_ch1'
!		entry 'GOES-8 1-channel vis (1 km)' \
!		   'putc2 p_raster platform sgpgoes8visX1.a1 field gvar_ch1'
!		line
!		entry 'AVHRR-12 Albedo and Brightness' \
!		   'putc2 p_raster platform sgpavhrr12X1.a1 field avhrr_ch1'
!		entry 'AVHRR-12 Radiances' \
!		   'putc2 p_raster platform sgpavhrr12radX1.a1 field avhrr_ch3'
!		entry 'AVHRR-14 Albedo and Brightness' \
!		   'putc2 p_raster platform sgpavhrr14X1.a1 field avhrr_ch1'
!		entry 'AVHRR-14 Radiances' \
!		   'putc2 p_raster platform sgpavhrr14radX1.a1 field avhrr_ch3'
!		line
!		entry 'GOES-Minnis' 'sendout c_goes_minnis pick'
!	endmenu
#endif /* notdef */


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


#ifndef UAV
define widget wpdn-menu intmenu 'wpdn plots'
	title 'Wind Profiler Demo Network'
	line
	entry 'Station Plot - Low Mode Winds' \
	   'putc2 p_wpdn_station u-field u_wind_low v-field v_wind_low'
	entry 'Station Plot - High Mode Winds' \
	   'putc2 p_wpdn_station u-field u_wind_hi v-field v_wind_hi'
	line
	entry 'Gridded Wind Vectors (Low Mode)' \
	   'putc2 p_wpdn_winds u-field u_wind_low v-field v_wind_low'
	entry 'Gridded Wind Vectors (Low Mode)' \
	   'putc2 p_wpdn_winds u-field u_wind_hi v-field v_wind_hi'
	line
	entry 'Consensus Contours - Low Mode' \
	   'putc1 p_wpdn_contour field cons_n_low'
	entry 'Consensus Contours - High Mode' \
	   'putc1 p_wpdn_contour field cons_n_hi'
endmenu
#endif

define widget kansas-hourly intmenu 'kansas'
	title 'Kansas Hourly Mesonet'
	line
	entry 'Station Plot' \
	   'putc2 p_station platform sgpksuhrlymesoX1.b1 icon sfc_hourly'
	entry 'Contour Plot' \
	   'putc2 p_contour platform sgpksuhrlymesoX1.b1 icon grid_hourly'
	entry 'Gridded Wind Vectors' \
   'putc2 p_irg_winds platform sgpksuhrlymesoX1.b1 icon grid_hourly_vectors'
endmenu

define widget kansas-daily intmenu 'kansas'
	title 'Kansas Daily Mesonet'
	line
	entry 'Station Plot' \
	   'putc2 p_station platform sgpksudlymesoX1.b1 icon meso_stations'
	entry 'Contour Plot' \
	   'putc2 p_contour platform sgpksudlymesoX1.b1 icon mesonet'
	entry 'Gridded Wind Vectors' \
	   'putc2 p_irg_winds platform sgpksudlymesoX1.b1 icon meso_winds'
endmenu

define widget oklahoma05a0 intmenu 'oklahoma'
	title 'Oklahoma Mesonet 5-minute (a0)'
	line
	entry 'Station Plot' \
	   'putc2 p_station platform sgp05okmX1.a0 icon sfc_5min'
	entry 'Contour Plot' \
	   'putc2 p_contour platform sgp05okmX1.a0 icon grid_5min'
	entry 'Gridded Wind Vectors' \
	   'putc2 p_irg_winds platform sgp05okmX1.a0 icon grid_5min_vectors'
endmenu

define widget oklahoma05a1 intmenu 'oklahoma'
	title 'Oklahoma Mesonet 5-minute (a1)'
	line
	entry 'Station Plot' \
	   'putc2 p_station platform sgp05okmX1.a1 icon sfc_5min'
	entry 'Contour Plot' \
	   'putc2 p_contour platform sgp05okmX1.a1 icon grid_5min'
	entry 'Gridded Wind Vectors' \
	   'putc2 p_irg_winds platform sgp05okmX1.a1 icon grid_5min_vectors'
endmenu

define widget oklahoma15a1 intmenu 'oklahoma'
	title 'Oklahoma Mesonet 15-minute (a1)'
	line
	entry 'Station Plot' \
	   'putc2 p_station platform sgp15okmX1.a1 icon meso_stations'
	entry 'Contour Plot' \
	   'putc2 p_contour platform sgp15okmX1.a1 icon mesonet'
	entry 'Gridded Wind Vectors' \
	   'putc2 p_irg_winds platform sgp15okmX1.a1 icon meso_winds'
endmenu

define widget oklahoma15a0 intmenu 'oklahoma'
	title 'Oklahoma Mesonet 15-minute (a0)'
	line
	entry 'Station Plot' \
	   'putc2 p_station platform sgp15okmX1.a0 icon meso_stations'
	entry 'Contour Plot' \
	   'putc2 p_contour platform sgp15okmX1.a0 icon mesonet'
	entry 'Gridded Wind Vectors' \
	   'putc2 p_irg_winds platform sgp15okmX1.a0 icon meso_winds'
endmenu

define widget nwssurf intmenu 'nws surface mesonet'
	title 'NWS Surface Mesonet'
	line
	entry 'Station Plot' 'sendout p_nws_station pick'
	entry 'Contour Plot' 'putc1 p_nws_contour representation contour'
	entry 'Gridded Wind Vectors' \
   'putc2 p_irg_winds platform sgpnwssurfX1.a1 icon meso_winds'
endmenu
