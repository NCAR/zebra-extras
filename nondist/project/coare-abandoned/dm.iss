!
! Code for dealing with ISS active areas.
!


procedure IssAddSounding station string window string
!
! Make this the sounding trace.
!
	!!! changed by mindy to the correct path
	!local realplat concat(station,'/omega')
	local realplat concat('sounding/',station)
	!!!
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
		local realplat concat(station,'/surf')
		ShipScalar #realplat 'tdry' #window
	else
		message 'Yup, this (#) is another case I can not handle' rep
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

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!! everything from here down was added by mindy

procedure MyIssAddXygraph station string window string
!
! Add to an xygraph.
!

!
! First we gotta figure out what sort of xygraph this is.
!
!!!       local comp NthComp(window, 1)
     local comps pd_complist(window)
     foreach comp #comps
        local rep PDParam(window, comp, 'representation')
        if (rep = 'wind')
                local otherrep 'wind'
                local realplat concat(station,'/prof915h')
                parameter #window #comp platform #realplat
        elseif (rep = 'simple')
                local realplat concat(station,'/surf')
                ShipScalar #realplat 'tdry' #window
        elseif (rep = 'obs')
                local otherrep 'rass'
                local realplat concat(station,'/rass915')
                parameter #window #comp platform #realplat
        elseif (rep = 'contour')
                local crep 'contour'
                local crepcomp comp
        elseif (rep <> "(Undefined)")
                message 'Yup, this (#) is another case I can not handle' rep
                beep
                return
        endif
     endfor
        if (crep <> "(Undefined)")
        if (crep = 'contour')
            if (otherrep = 'wind')
                local realplat concat(station,'/prof915h')
                parameter #window #crepcomp platform #realplat
            elseif (otherrep = 'rass')
                local realplat concat(station,'/rass915')
                parameter #window #crepcomp platform #realplat
            endif
        endif
        endif
endprocedure

procedure MineIssAddXygraph station string window string
!
! Add to an xygraph.
!

!
! First we gotta figure out what sort of xygraph this is.
!
     local comps pd_complist(window)
     foreach comp #comps
        local rep PDParam(window, comp, 'representation')
        if (rep = 'simple')
           if (substring(comp,'sound'))
                local realplat concat('sounding/',station)
                parameter #window #comp platform #realplat
           elseif (substring(comp,'wind'))
                local realplat concat(station,'/prof915h')
                parameter #window #comp platform #realplat
           elseif (substring(comp,'rass'))
                local realplat concat(station,'/rass915')
                parameter #window #comp platform #realplat
           endif
        elseif (rep <> "(Undefined)")
                message 'Yup, this (#) is another case I can not handle' rep
                beep
                return
        endif
     endfor
endprocedure


!
! Make a window display a given platform.  This is back-invoked from gp.
!
procedure MyTweakIss station string
!
! OK, look at the plot type of the destination window and branch out from
! there.
!
                IssAddSounding #station skew-t
                MyIssAddXygraph #station rass
                MyIssAddXygraph #station wind
endprocedure

procedure MineTweakIss station string
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
                beep    ! what can we do here??
                return
        elseif (ptype = 'skewt')
                IssAddSounding #station #target_win
        elseif (ptype = 'xygraph')
                MineIssAddXygraph #station #target_win
        else
                message 'Huh?  Funky plot type #' ptype
                beep
        endif
endprocedure

!
! Make a window display a given platform.  This is back-invoked from gp.
!
procedure NewTweakIss station string
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
		MyIssAddXygraph #station #target_win
	else
		message 'Huh?  Funky plot type #' ptype
		beep
	endif
endprocedure
