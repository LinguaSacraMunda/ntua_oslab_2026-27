#!/bin/bash
PID="$(pgrep riddle)"
if [ -n $PID ]; then
    PROC="/proc/$PID/map_files"
    MEM="$(ls -la "$PROC" | awk '/\(deleted\)/ { print $9 }')"
    sudo cat "$PROC/$MEM" | awk '{ print $NF }' | tr -cd '[:alnum:]'
    echo
fi
