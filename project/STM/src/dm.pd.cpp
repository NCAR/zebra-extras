!
! Load, manipulate, and create our plot descriptions
!

!----------------------------------------------------------------------
! Grab the ones in the library and project pd directory
!
set libpddir concat(c$libdir,'/pd')
pddir #libpddir
pddir pd

!-----------------------------------------------------------------------
! Build the defaults database.  We want fest, class, and NWS soundings
! locations.
!
pddrop locations fest_radar defaults defaults
pddrop locations class defaults defaults
pddrop locations nws_soundings defaults defaults

!
! And of course we need our project-specific defaults, which may override
! some of the library parameter settings.
!
pddrop arm_defaults defaults defaults defaults

! Use ZEB_CACHEDIR for cache files if defined, otherwise use a cache
! directory under the project directory.
!
if (getenv("ZEB_CACHEDIR") <> "UNDEFINED")
	set cachedir getenv("ZEB_CACHEDIR")
else
	set cachedir concat(c$projdir,"/cache")
endif
parameter defaults defaults file-path #cachedir

!---------------------------------------------------------------------------
! The ubiquitous whiteclock pd
!
beginpd whiteclock
component clock
	parameter background white
	parameter foreground black
	parameter xfont '-*-new century schoolbook-bold-r-*-*-25-*-*-*-*-*-*-*'
endpd

!---------------------------------------------------------------------------
! We want the skewt templates to use overlay data mode rather than replace
!
parameter skewt-template global "add-data-mode" "overlay-mode"

! -----------------------------------------------------------------------
! Now lift some of the library pd's which we only need to tweak slightly 
! -----------------------------------------------------------------------

!
! As usual, no spaces in location lists
!
!set ef_locns 'E1,E2,E3,E4,E5,E6,E7,E8,E9,E10,E11,E12,E15,E16'
!set ef_locns concat(ef_locns,',E17,E18,E19,E20,E21,E22,E23,E24,E25')
!
! Leave E13 out since it just clutters things when CF is there, and
! all of the instruments are available through the CF menu.
!
set ef_locns 'E4,E7,E8,E9,E12,E15,E20,E22,E26'
set bf_locns 'B1,B4,B5,B6'
set all_locns concat4("C1,",ef_locns,",",bf_locns)

!
! Active EF locations
!
pdlift p_active_loc p_active_loc p_ef_locns
parameter p_ef_locns p_ef_locns label-size 12
parameter p_ef_locns p_ef_locns color green
parameter p_ef_locns p_ef_locns icon-color green
parameter p_ef_locns p_ef_locns platform #ef_locns
parameter p_ef_locns p_ef_locns require "location, ef-active-loc"
parameter p_ef_locns p_ef_locns posicon-middle-menu ef-send-menu
parameter p_ef_locns p_ef_locns posicon-left-menu minimal-cap-left-menu

!
! Active BF locations
!
pdlift p_active_loc p_active_loc p_bf_locns
parameter p_bf_locns p_bf_locns label-size 12
parameter p_bf_locns p_bf_locns color cyan
parameter p_bf_locns p_bf_locns icon-color cyan
parameter p_bf_locns p_bf_locns icon bf-locn
parameter p_bf_locns p_bf_locns platform #bf_locns
parameter p_bf_locns p_bf_locns require "location, bf-active-loc"
parameter p_bf_locns p_bf_locns posicon-middle-menu bf-send-menu
parameter p_bf_locns p_bf_locns posicon-left-menu minimal-cap-left-menu

!
! Active CF locations
!
pdlift p_active_loc p_active_loc p_cf_locns
parameter p_cf_locns p_cf_locns color HotPink
parameter p_cf_locns p_cf_locns icon-color HotPink
parameter p_cf_locns p_cf_locns icon circle
parameter p_cf_locns p_cf_locns label-size 12
parameter p_cf_locns p_cf_locns platform C1
parameter p_cf_locns p_cf_locns require "location, cf-active-loc"
parameter p_cf_locns p_cf_locns posicon-middle-menu cf-send-menu
parameter p_cf_locns p_cf_locns posicon-left-menu minimal-cap-left-menu

!
! SMOS 30-minute a1 platforms, as an irgrid platform string
!
set smos_plats "sgp30smosE9.a1,sgp30smosE20.a1,sgp30smosE15.a1,sgp30smosE13.a1"

!
! EBBR 30-minute a1 platforms
!
set ebbr_plats1 "sgp30ebbrE9.a1,sgp30ebbrE8.a1,sgp30ebbrE7.a1,sgp30ebbrE4.a1,"
set ebbr_plats2 "sgp30ebbrE26.a1,sgp30ebbrE22.a1,sgp30ebbrE20.a1,"
set ebbr_plats3 "sgp30ebbrE15.a1,sgp30ebbrE13.a1,sgp30ebbrE12.a1"
set ebbr_plats concat3(ebbr_plats1, ebbr_plats2, ebbr_plats3)

!
! Non-active overlay of all locations
!
pdlift p_loc p_loc p_all_locns
parameter p_all_locns p_all_locns platform #all_locns

!
! The generic ngm plot description
!
pdlift p_model p_model p_ngm
parameter p_ngm p_ngm platform sgpngm250X1.c1
parameter p_ngm p_ngm icon ngm-clock
parameter p_ngm p_ngm field temperature
parameter p_ngm p_ngm u-field u-component
parameter p_ngm p_ngm v-field v-component
parameter p_ngm p_ngm color cyan
parameter p_ngm p_ngm altitude-control true

!
! Now drop in our ngm-specific parameters to get
! a winds component description
!
pdlift p_model_winds p_model_winds p_ngm_winds
pddrop p_ngm p_ngm p_ngm_winds p_ngm_winds

!
! For UAV NGM platforms, we override the FieldMenu with an explicit menu, 
! which also requires another module.  And we'll always add the issue
! time platform.
!
pdlift p_ngm p_ngm p_uav_ngm
parameter p_uav_ngm p_uav_ngm icon-middle-menu ngm-field-menu
parameter p_uav_ngm p_uav_ngm posicon-middle-menu ngm-field-menu
parameter p_uav_ngm p_uav_ngm annot-left-menu ngm-field-menu
parameter p_uav_ngm p_uav_ngm annot-middle-menu ngm-field-menu
parameter p_uav_ngm p_uav_ngm require "contour, simple-color, ngm"
parameter p_uav_ngm p_uav_ngm representation contour
parameter p_uav_ngm p_uav_ngm limit-proc "contour-putup-adj"
parameter p_uav_ngm p_uav_ngm platform sgpngm250X1.ns.issue

pdlift p_uav_ngm p_uav_ngm p_uav_ngm_vector
parameter p_uav_ngm_vector p_uav_ngm_vector representation vector
parameter p_uav_ngm_vector p_uav_ngm_vector degrade 0
parameter p_uav_ngm_vector p_uav_ngm_vector "arrow-scale" "0.01"
parameter p_uav_ngm_vector p_uav_ngm_vector limit-proc "adj_arrow_scale"

!
! GOES data from the UAV project
!
pdlift p_raster p_raster p_goes_uav
parameter p_goes_uav p_goes_uav require "raster, uav-goes"
parameter p_goes_uav p_goes_uav field vis
parameter p_goes_uav p_goes_uav platform sgpgoesvis.tmp
parameter p_goes_uav p_goes_uav icon-middle-menu goes-middle

!
! For NWS mesonet, replace the field menu
!
pdlift p_station p_station p_nws_station
parameter p_nws_station p_nws_station icon-middle-menu nws-station-fields
parameter p_nws_station p_nws_station posicon-middle-menu nws-station-fields
parameter p_nws_station p_nws_station annot-left-menu nws-station-fields
parameter p_nws_station p_nws_station annot-middle-menu nws-station-fields
parameter p_nws_station p_nws_station require "station, simple-color, nws"
parameter p_nws_station p_nws_station platform sgpsfchrlyX1.a1
parameter p_nws_station p_nws_station icon sfc_hourly

pdlift p_contour p_contour p_nws_contour
parameter p_nws_contour p_nws_contour icon-middle-menu nws-surf-fields
parameter p_nws_contour p_nws_contour require "contour, simple-color, nws"
parameter p_nws_contour p_nws_contour platform sgpsfchrlyX1.a1
parameter p_nws_contour p_nws_contour field tdry
parameter p_nws_contour p_nws_contour icon grid_hourly

!
! WPDN controls altitude and uses special winds fields whether its the
! high or low mode.
!
beginpd p_wpdn
component p_wpdn
	parameter u-field u_wind_low
	parameter v-field v_wind_low
	parameter altitude-control 'true'
	parameter platform 'Dsgp60wpdnwndsX1.a1'
	parameter icon wpdn
endpd
pdlift p_station p_station p_wpdn_station
pddrop p_wpdn p_wpdn p_wpdn_station p_wpdn_station 
pdlift p_irg_winds p_irg_winds p_wpdn_winds
pddrop p_wpdn p_wpdn p_wpdn_winds p_wpdn_winds
pdlift p_contour p_contour p_wpdn_contour
pddrop p_wpdn p_wpdn p_wpdn_contour p_wpdn_contour


