#################################################################
#
#         .cshrc file           Santiago Newbery
#
#         initial setup file for both interactive and noninteractive
#         C-Shells
#
#################################################################
#echo ".cshrc..."
umask 002
if ( ${?OPENWINHOME} == 0 ) then        
  setenv OPENWINHOME /usr/openwin
endif                           

#         set up search path
#  1. directories for local disks
#  2. add directories for network commands (/usr/local/bin == /localbin)
# note:   /zeb/bin must come before /bin/X11 to get the right imake
#  3. add directories for zeb, nexus & other apps
set path = ( /bin /sbin /usr/sbin /usr/bin /usr/local/bin /usr/X11R6/bin \
$OPENWINHOME/bin /zebra/bin /usr/bin/X11 ~/bin .)
# auto-complete filenames in cshell
set filec

#         cd path

#         skip remaining setup if not an interactive shell

if ($?USER == 0 || $?prompt == 0) exit
#echo ".cshrc interactive..."

#          settings  for interactive shells

set history=40
set ignoreeof
set savehist=40
set prompt="`uname -n`:$cwd>"
set time=100

#         aliasess for interactive shells
alias pwd       'echo $cwd'
alias del       'rm -i'
alias cd        'cd \!*; set prompt="`uname -n`:$cwd>" '
alias pushd     'pushd \!*; set prompt="`uname -n`:$cwd>" '
alias pd        pushd
alias popd      'popd \!*; set prompt="`uname -n`:$cwd>" '
alias h         'history \!* | head -39 | more'
alias 
        clear
alias md        mkdir
alias rd        rmdir

alias .         'echo $cwd'
alias ..        'set dot=$cwd;cd ..'
alias ,         'cd $dot '
alias l         'ls -alF \!*'
alias ll        'ls -alF | more'
alias lg        'ls -alFg | more'
alias llr       'ls -alFR | more'
alias la        'ls -a'
alias lf        'ls -F'
alias lt        'ls -lrt'
alias legz      'gunzip -c \!* | less'
alias j         'jobs -l'
alias bye       'logout'
alias hog       'du | sort -b -n -r +0 -1 | head -40'
alias psm       'ps -aux | more '
alias psg       'ps -aux | grep \!* '
alias pss       'ps -aux | sort -b -n -r +4 -5 | more'
alias mroe      more
alias m         more
alias smsgs     'more /var/adm/messages'
alias senv      'env | sort | more'
alias set-disp  'setenv DISPLAY unix:0'
alias df        'df -k'
alias du        'du -k'
alias last      'last | more'
alias forecast  'telnet downwind.sprl.umich.edu 3000'

# X aliases ----------------------------------
# alias xinit xinit --  -ar1 400       # keybd delay (-ar2 n  sets repeat rate.)
alias console   'xterm -g 62x20+5+0 -bg royalblue -fg white -C -sb -ls -title Console -e tcsh &'
alias xterm1    'xterm -g 80x50+5-5 +vb -j -sf -title xterm_1 -n xterm_1 -bg LightCyan -fg midnightblue &'
alias xterm2    'xterm -g 80x23-5-5 +vb -j -title xterm_2 -n xterm_2 -bg LightBlue -fg black &'
alias xterm3    'xterm -g 80x30-5+105 +vb -j -sb -name xterm_3 -bg white -fg black &'
alias xed       'xedit -bg aquamarine \!*; chmod ugo-x \!*'
alias xed1      'xedit -bg lightyellow -fg darkslateblue  \!*; chmod ugo-x \!*'
alias rxwin     'xterm -g 90x40+200+200 +vb -j -title \!* -n \!* -e telnet \!* &'
alias xload     'xload -g 150x50-5+0  -fg red -update 2 -jumpscroll 1 &'
alias xclock    'xclock -g -1+56 -digital -chime -update 1 -bg yellow &'
alias xbiff     'xbiff -g 50x50-1+55  -file /var/mail/newbery -bg green -fg red &'
alias xman      'xman  -notopbox -pagesize 850x600+200+200 -iconic &'
alias em1       'emacs -i  -bg aquamarine \!*'
alias em2       'emacs -i  -bg lightyellow -fg darkslateblue \!*'
alias em3       'emacs -i  -bg rgb:00/1b/c0  -fg yellow \!*'
alias rs        'eval resize'

if (`uname -n` == "jnu_aops") then
        alias zebra 'xterm -T zebra -g +100+100 -iconic -bg gray70 -e zstart -dm -preserve juneau &'
else
        alias zebra 'xterm -T zebra -g +100+100 -iconic -bg gray70 -e zstart juneau &'
endif

