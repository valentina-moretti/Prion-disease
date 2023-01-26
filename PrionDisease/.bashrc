PATH=/bin:/usr/bin:$HOME/bin:/usr/bin/X11:/opt/bin:/usr/local/bin:/usr/local/bin/X11:.
export PATH

export GS_OPTIONS="-sPAPERSIZE=a4"
PS0=`hostname`
PS1='$PS0$PWD > '
export PS1

ulimit -c 0

umask 027

alias lpr='lpr -h'
alias lt='ls -lt | more'
alias mem='ps aux |cut -c21-24,49- |sort -r |more'
alias mon='ps aux | cut -c15-19,49- |sort -r |more'
alias A2ps='a2ps -1 --medium=A4'



if [[ -e $HOME/.chpw ]]
then
  /usr/bin/yppasswd
  /bin/rm $HOME/.chpw
fi

