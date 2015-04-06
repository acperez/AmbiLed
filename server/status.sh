#!/bin/bash
ambiled_pid=$(pidof ambiled)
if [ -z $ambiled_pid ]; then
    echo -n false
    exit 0
fi
echo -n true
