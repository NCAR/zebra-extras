!
! Support for active facility locations.
!

!
! Menu for platforms available at an extended facility
!
define widget ef-send-menu intmenu 'ef instrument sender'
	title 'Extended Facility Instruments'
	line
	submenu 'SMOS Platforms' 'ef-smos-menu'
	submenu 'EBBR Platforms' 'ef-ebbr-menu'
	entry 'SIROS (a1)' 'ef-select Dsgpsiros .a1 hemisp_broadband'
endmenu

define widget ef-smos-menu intmenu 'ef smos platforms'
	title 'SMOS Platforms'
	line
	entry '1-minute (a0)' 'ef-select sgp1smos .a0 temp'
	entry '1-minute (a1)' 'ef-select sgp1smos .a1 temp'
	entry '30-minute (a0)' 'ef-select sgp30smos .a0 temp'
	entry '30-minute (a1)' 'ef-select sgp30smos .a1 temp'
endmenu

define widget ef-ebbr-menu intmenu 'ef ebbr platforms'
	title 'EBBR Platforms'
	line
	entry '5-minute (a0)' 'ef-select sgp5ebbr .a0 temp'
	entry '15-minute (a1)' 'ef-select sgp15ebbr .a1 temp'
	entry '30-minute (a1)' 'ef-select sgp30ebbr .a1 temp'
endmenu

!
! Stuff called out of the above
!
procedure ef-select instrument string level string field string

!
! It's possible we're called from the Central Facility, in which case
! we are actually platform E13.
!
	if (icon_platform = 'CF')
		set icon_platform 'E13'
	elseif (icon_platform = 'C1')
		set icon_platform 'E13'
	endif
	local platform concat3(instrument,icon_platform,level)
	local dmcmd quote(concat4('PutScalar ', platform, ' ', field))
	dm #dmcmd
endprocedure

