#!/bin/bash

echoerr() { echo "$@" 1>&2; }

if [ $(influx -execute "SHOW databases" | grep -c _internal) -ne 1 ];then
    echoerr "Database '_internal' not yet present..."
    exit 1
else
    exit 0
fi
