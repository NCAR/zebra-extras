#! /bin/sh

xrdb -merge .Xresources

xsetroot -solid darkslateblue &

xhost +verdon.rap.ucar.edu
xset s 0 600

sleep 1
xclock -g +60+5 -digital -update 1 -fn "*helvetica-medium-r-normal--18*" &

xterm -g 80x20+5+0 -bg royalblue -iconic -fg white -C -sb -ls -title Console -n Console &
xterm -g 80x30-20+20 -n one &
xterm -g 80x30-20+512 -iconic -n two &

xterm -T zebra -g +100+100 -iconic -bg gray70 -e zstart juneau/syncdata &

sleep 1

exec fvwm2
