!
! Support for active facility locations.
!

!
! Include access to the E13 instruments, since our active area is
! covering E13s active area.
!
require 'ef-active-loc'

!
! Menu for platforms available at a central facility
!
define widget cf-send-menu intmenu 'cf instrument sender'
	title 'Central Facility Instruments'
	line
	submenu 'Tower Platforms' 'tower-menu'
	submenu '50-MHz RASS' 'rass-50-menu'
	submenu '915-MHz RASS' 'rass-915-menu'
	submenu 'BSRN' 'bsrn-menu'
	submenu 'MFR' 'mfr-menu'
	entry 'SONDE (a1)' 'cf-select sgpsonde .a1 pres'
	entry 'MWRLOS (a1)' 'cf-select sgpmwrlos .a1 vap'
	line
	submenu 'E13 Instruments' ef-send-menu	
endmenu


define widget rass-50-menu intmenu 'rass platforms'
	title '50 MHz RASS Profiler'
	line
	entry 'Temperature (a0)' 'cf-select Dsgp50rwptemp .a0 temp'
	entry 'Temperature (a1)' 'cf-select Dsgp50rwptemp .a1 temp'
	entry 'Temperature (a2)' 'cf-select Dsgp50rwptemp .a2 temp'
	entry 'Temperature (b1)' 'cf-select Dsgp50rwptemp .b1 temp'
	line
	entry 'Wind (a0)' 'cf-select Dsgp50rwpwind .a0 none'
	entry 'Wind (a1)' 'cf-select Dsgp50rwpwind .a1 none'
	entry 'Wind (a2)' 'cf-select Dsgp50rwpwind .a2 none'
	entry 'Wind (b1)' 'cf-select Dsgp50rwpwind .b1 none'
endmenu
	

define widget rass-915-menu intmenu 'rass platforms'
	title '915 MHz RASS Profiler'
	line
	entry 'Temperature (a0)' 'cf-select Dsgp915rwptemp .a0'
	entry 'Temperature (a1)' 'cf-select Dsgp915rwptemp .a1'
	entry 'Temperature (a2)' 'cf-select Dsgp915rwptemp .a2'
	entry 'Temperature (b1)' 'cf-select Dsgp915rwptemp .b1'
	line
	entry 'Wind (a0)' 'cf-select Dsgp915rwpwind .a0'
	entry 'Wind (a1)' 'cf-select Dsgp915rwpwind .a1'
	entry 'Wind (a2)' 'cf-select Dsgp915rwpwind .a2'
	entry 'Wind (b1)' 'cf-select Dsgp915rwpwind .b1'
endmenu
	

define widget tower-menu intmenu 'tower platforms'
	title 'Tower Platforms'
	line
	entry 'Tower daily (a0)' 'cf-select Dsgp1440twr21x .a0 tdry'
	entry 'Tower 1-minute (a0)' 'cf-select Dsgp1twr21x .a0 tdry'
	entry 'Tower 1-minute (a1)' 'cf-select Dsgp1twr21x .a1 tdry'
	entry 'Tower 30-minute (a0)' 'cf-select Dsgp30twr21x .a0 tdry'
	entry 'Tower 30-minute (a1)' 'cf-select Dsgp30twr21x .a1 tdry'
endmenu


define widget bsrn-menu intmenu 'bsrn platforms'
	title 'BSRN Platforms'
	line
	entry 'BSRN (a0)' 'cf-select sgpbsrn .a0 nip'
	entry 'BSRN (a1)' 'cf-select sgpbsrn .a1 nip'
	entry 'BSRN Calculations (c1)' 'cf-select sgpbsrncalc .c1 nip'
endmenu


define widget mfr-menu intmenu 'mfr platforms'
	title 'Multi-filtering Radiometer'
	line
	entry 'MFR 10-meter (a1)' \
		'cf-select Dsgpmfr10m .a1 mfr_up_hemisp_broadband'
	entry 'MFR 25-meter (a1)' \
		'cf-select Dsgpmfr25m .a1 mfr_up_hemisp_broadband'
endmenu


define widget blc-menu intmenu 'blc platforms'
	title 'Bellfort Laser Ceilometer'
	line
	entry 'BLC Level a1' 'cf-select sgpblc .a1 cloud1'
!
! No a0 data
!
!	entry 'BLC Level a0' 'cf-select sgpblc .a0 cloud1'
endmenu

!
! Stuff called out of the above
!
procedure cf-select instrument string level string field string

!
! For now just force the 'icon_platform' to C1
!
	set icon_platform 'C1'
	local platform concat3(instrument,icon_platform,level)
	local dmcmd quote(concat4('PutScalar ', platform, ' ', field))
	dm #dmcmd
endprocedure


