!
! Code for dealing with ISS active areas.
!
! $Id: dm.iss,v 1.2 1994-06-21 17:16:38 granger Exp $

procedure IssAddSounding station string window string
!
! Make this the sounding trace.
!
	local realplat concat(station,'/class')
!
! Get the second component name, then tweak the platform.
!
	local comp NthComp(window, 1)
	if (comp <> "(Undefined)")
		parameter #window #comp platform #realplat
	else
		beep
		message 'I can not cope with this case yet'
	endif
endprocedure




procedure IssAddXygraph station string window string
!
! Add to an xygraph.
!

!
! First we gotta figure out what sort of xygraph this is.
!
	local comp NthComp(window, 1)
	local rep PDParam(window, comp, 'representation')
	if (rep = 'wind') 
		local realplat concat(station,'/prof915h')
		parameter #window #comp platform #realplat
	elseif (rep = 'simple')
		local realplat concat(station,'/surface_met')
		ShipScalar #realplat 'tdry' #window
	else
		message 'This rep (#) is a case I cannot handle' rep
		beep 
		return
	endif
endprocedure




!
! Make a window display a given platform.  This is back-invoked from gp.
!
procedure TweakIss station string
!
! Get the target window.
!
	set target_win "none"
	pickwin target_win
	if (target_win = 'none')
		beep
		return
	endif
!
! OK, look at the plot type of the destination window and branch out from
! there.
!
	local ptype PDParam(target_win, 'global', 'plot-type')
	if (ptype = 'CAP' or ptype = 'cap')
		beep	! what can we do here??
		return
	elseif (ptype = 'skewt')
		IssAddSounding #station #target_win
	elseif (ptype = 'xygraph')
		IssAddXygraph #station #target_win
	else
		message 'Huh?  Funky plot type #' ptype
		beep
	endif
endprocedure


