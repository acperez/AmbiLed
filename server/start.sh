#!/bin/bash
ambiled_pid=$(pidof ambiled)
echo $ambiled_pid
if [ -z $ambiled_pid ]; then
    cd /home/pi/ambiled
    sudo nohup bin/ambiled &
fi
exit 0
