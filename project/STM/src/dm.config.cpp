! $Id: dm.config.cpp,v 1.9 1997-06-02 22:55:26 granger Exp $
!
! ------------------------------------------------------------------------
! DO NOT EDIT THIS FILE if it resides in the top level of the project
! configuration.  This file is generated from a cpp source file in the
! ./src directory!
! ------------------------------------------------------------------------

#include "config.h"

!
! Display manager configuration file.
!

set undef "UNDEFINED"

!
! Load in our plot descriptions.
!
message 'Loading:'
message '     ...plot descriptions...'
read "dm.pd"

!
! Kludgery for dm modules -- add commands, primarily.
!
set DmModPath 'dmmodules'

message '     ...modules...'
require "init-config-path"

set TimeFile './dconfig/Showcases'
set UseXHelp false
set mosaicpath CFG_BROWSERPATH

!
! Add our color table directory to the path.
!
set CTablePath concat('colortables,',CTablePath)

!
! Most of the interesting stuff is in other files -- if you looked here,
! you lose!
!
message '     ...widgets...'
read dm.widgets

message '     ...ISS active areas ...'
read dm.iss

#ifdef notdef
message '     ...CF active areas ...'
read dm.cf

message '     ...EF active areas ...'
read dm.ef
#endif

message '     ...button maps...'
read ButtonMaps

message '     ...display configurations ...'
read DisplayConfigs

message '     ...the icon bar...'
read IconBar
message 'ready!'

!
! When and how long to sleep when creating windows.
! OpenWindows => 1,5	X => 4,1
set sleepafter	2
set sleepfor	4

!
! Come up with an interesting display.
!
set SoundEnabled false
set AutoAdvance true

#ifdef REALTIME
set ForceHistory false
realtime all
#else
set ForceHistory true
set itime getenv("INITIAL_TIME")
if (itime <> undef)
	history all #itime
else
	history all 24-Jun-1993,12:00
endif
#endif

set config getenv("DEFAULT_CONFIG")
if (config = undef)
	set config "empty"
endif
display #config
