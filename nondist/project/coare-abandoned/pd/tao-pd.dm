
beginpd c_tao
component c_tao
	parameter platform tao
	parameter representation contour
	parameter field 	sst
	parameter contour-sst-center 29
	parameter contour-sst-step 0.3
	parameter sst-range-min 20
	parameter sst-range-max 40
	parameter ta-range-min 0
	parameter ta-range-max 50
	parameter rh-range-min 0
	parameter rh-range-max 100
	parameter color-mono true
	parameter color 	cyan
	parameter label-blanking false
	parameter color-table cook16_bk
	parameter icon-middle-menu tao-middle
	parameter icon 	buoy
	parameter trigger-global true
	parameter trigger tao
endpd

!
! Fancy generic plot description intended to work with filled or line 
! contours.  Uses line contours by default.  Platform and field need to be
! filled in.
!
beginpd c_tao_contour
global
	parameter pd-name	"override me"
	parameter comment	"this pd read from dm ui file"
component c_tao_contour
	parameter platform 	tao
	parameter representation 	contour
	parameter field 		tdry
	parameter icon-left-menu 	standard-cap-left-menu
	parameter icon-middle-menu FieldMenu
	parameter icon-right-menu contour-right
	parameter require 	'contour, simple-color'
	parameter scale-mode 	auto
	parameter color-table 	Contour
	parameter color 		yellow
	parameter icon 		tao
	parameter do-labels 	true
	parameter label-blanking 	false
	parameter limit-proc 	contour-putup-adj
	parameter max-fill 	5
	parameter radius 		6
	parameter grid-method 	barnes
endpd


!
! Fancy generic plot description intended to work with filled or line 
! contours.  Uses line contours by default.  Platform and field need to be
! filled in.
!
beginpd
component c_tao_fcontour
	parameter platform 	tao
	parameter representation 	filled-contour
	parameter field 		tdry
	parameter icon-left-menu 	standard-cap-left-menu
	parameter icon-middle-menu FieldMenu
	parameter icon-right-menu contour-right
	parameter require 	'contour, simple-color'
	parameter scale-mode 	auto
	parameter color-table 	Contour
	parameter color 		yellow
	parameter icon 		tao
	parameter do-labels 	true
	parameter label-blanking 	false
	parameter limit-proc 	contour-putup-adj
	parameter max-fill 	5
	parameter radius 		6
	parameter grid-method 	barnes
endpd


beginpd
component c_tao_winds
	parameter arrow-scale 0.01
	parameter color red
	parameter grid-method barnes
	parameter icon tao_winds
	parameter icon-left-menu standard-cap-left-menu
	parameter icon-right-menu irg-winds-right
	parameter limit-proc adj_arrow_scale
	parameter line-width 2
	parameter max-fill 5
	parameter platform tao
	parameter radius 6
	parameter representation vector
	parameter require 'irg_winds, simple-color'
	parameter u-field u_wind
	parameter v-field v_wind
	parameter x-points 30
	parameter y-points 30
endpd
