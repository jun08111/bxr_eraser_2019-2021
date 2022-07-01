# .bashrc

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# Uncomment the following line if you don't like systemctl's auto-paging feature:
# export SYSTEMD_PAGER=

# User specific aliases and functions
export LD_LIBRARY_PATH=/APP/bluexg-server-1.0/lib
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.292.b10-1.el7_9.x86_64
export JRE_HOME=$JAVA_HOME/jre

export DEV_HOME=`echo $HOME/Blue-X-ray-Eraser/1.server`
alias dinc="cd $DEV_HOME/inc"
alias dbin="cd $DEV_HOME/bin"
alias dcfg="cd $DEV_HOME/conf"
alias ssrc="cd $DEV_HOME/src/bxr_server"
alias psrc="cd $DEV_HOME/src/bxr_parser"
alias usrc="cd $DEV_HOME/src/bxr_utils"

export BXRG_HOME=`echo /APP/bluexg-server-1.0`
alias lib="cd $BXRG_HOME/lib"
alias cfg="cd $BXRG_HOME/conf"
alias bin="cd $BXRG_HOME/bin"
alias log="cd $BXRG_HOME/log"

export PATH=$PATH:$BXRG_HOME/bin

