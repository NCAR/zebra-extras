!
! Support for active boundary facility locations.
!

!
! Menu for platforms available at a boundary facility
!
define widget bf-send-menu intmenu 'bf instrument sender'
	title 'Boundary Facility Instruments'
	line
	entry 'Sonde (wrpn, a1)' 'bf-select sgpsondewrpn .a1 tdry'
	entry 'Sonde Wind Calc (c1)' 'bf-select sgpsondewndcalc .c1 pres'
	line
	entry 'MWR Line-of-Sight (a1)' 'bf-select sgpmwrlos .a1 vap'
	entry 'MWR 5-minute averages (c1)' 'bf-select sgp5mwravg .c1 vap'
	entry 'QME: MWR Column (c1)' 'bf-select sgpqmemwrcol .c1 mean_vap_mwr'
endmenu

!
! Stuff called out of the above
!
procedure bf-select instrument string level string field string
	local platform concat3(instrument,icon_platform,level)
	local dmcmd quote(concat4('PutScalar ', platform, ' ', field))
	dm #dmcmd
endprocedure

