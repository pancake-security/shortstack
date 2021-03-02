#!/usr/bin/env bash

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

coremask=$1
iname=$2
shift;
shift;

taskset -c $coremask /local/deploy/proxy_server l2 -h /local/deploy/hosts.csv -d /local/deploy/distinfo.bin -i $iname "$@" 2>/local/deploy/$iname.err 1>/local/deploy/$iname.out &
echo "Started L2 proxy"