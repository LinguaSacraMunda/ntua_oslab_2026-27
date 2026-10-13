#!/bin/bash
PID="$(pgrep riddle)"
if [ ! -z $PID ]; then
    PROC="/proc/$PID/map_files"
    MEM="$(ls -la $PROC | grep '(deleted)' | awk ' { print $9 }')"
    NUM="$(sudo cat $PROC/$MEM | awk ' { print $NF } ')"
    echo $NUM
fi
