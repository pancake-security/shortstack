#!/usr/bin/env bash

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

coremask=$1
iname=$2
hostname=$3
port=$4
shift;
shift;
shift;
shift;

taskset -c $coremask /local/deploy/redis-server --bind $hostname --port $port --save "" --appendonly no "$@" 2>/local/deploy/$iname.err 1>/local/deploy/$iname.out &
echo "Started KV"