#!/bin/bash
DIR=$(dirname "$0")
export LD_LIBRARY_PATH="$DIR/libs:/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
#echo LD_LIBRARY_PATH=$LD_LIBRARY_PATH
#echo $PATH
QT_DEBUG_PLUGINS=1 && "$DIR/bin/potaleger" "$@"
