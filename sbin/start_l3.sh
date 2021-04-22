#!/usr/bin/env bash

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

coremask=$1
iname=$2
shift;
shift;



ulimit -n 10240

sudo /local/deploy/wondershaper/wondershaper -a ens5 -c

taskset -c $coremask /local/deploy/proxy_server l3 -h /local/deploy/hosts.csv -i $iname "$@" 2>/local/deploy/$iname.err 1>/local/deploy/$iname.out &
echo "Started L3 proxy"