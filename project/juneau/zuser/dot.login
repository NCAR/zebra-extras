##################################################################
#
#         Read in after the .cshrc file at login time.
#         Not read in for subsequent shells.  For setting up
#         terminal and global environment characteristics.
#
##################################################################


echo " "

setenv CC gcc
setenv EDITOR "emacs -nw"
setenv MANPATH /usr/man:/usr/X11R6/man:/usr/local/man

#         terminal characteristics for remote terminals:
#         Leave lines for all but your remote terminal commented
#         out (or add a new line if your terminal does not appear).

# disable tcsh autologout
unset autologout
  
setenv XFILESEARCHPATH /usr/X11R6/lib/X11/%T/%N%S

stty erase '^h'
stty eof  '^d'
stty kill '^k'
if (($TERM == "linux") || ($TERM == "console")) then
   startx
   sleep 1
   logout
endif
