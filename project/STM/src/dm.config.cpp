!
! Display manager configuration file.
!
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

message '     ...dm modules...'
require "init-config-path"

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

!
!message '     ...CF active areas ...'
!read dm.cf
!
!message '     ...EF active areas ...'
!read dm.ef
!

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
set ForceHistory true
set AutoAdvance true
set itime getenv("INITIAL_TIME")
if (itime <> "UNDEFINED")
	history all #itime
else
	history all 24-Jun-1993,12:00
endif


set config getenv("DEFAULT_CONFIG")
if (config = "UNDEFINED")
	set config "empty"
endif
display #config
