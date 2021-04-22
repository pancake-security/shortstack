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


sudo sysctl -w net.core.somaxconn=1024
sudo sysctl -w vm.overcommit_memory=1
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
ulimit -n 10240

sudo /local/deploy/wondershaper/wondershaper -a ens5 -c
sudo /local/deploy/wondershaper/wondershaper -a ens5 -u 1000000

taskset -c $coremask /local/deploy/redis-server --bind $hostname --port $port --save "" --appendonly no "$@" 2>/local/deploy/$iname.err 1>/local/deploy/$iname.out &
echo "Started KV"