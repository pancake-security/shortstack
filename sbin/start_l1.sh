#!/usr/bin/env bash

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

coremask=$1
iname=$2
shift;
shift;

killall proxy_server

taskset -c $coremask /local/deploy/proxy_server l1 -h /local/deploy/hosts.csv -d /local/deploy/distinfo.bin -i $iname "$@" 2>/local/deploy/$iname.err 1>/local/deploy/$iname.out &
echo "Started L1 proxy"