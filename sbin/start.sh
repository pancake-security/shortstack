#!/usr/bin/env bash

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

echo 1024 | sudo tee /sys/devices/system/node/node*/hugepages/hugepages-2048kB/nr_hugepages
echo "Inited hugepages"

flow_size=$1
num_flows=$2
load=$3
reject_factor=$4
param_k=$5

sudo /local/deploy/zedro send $flow_size $num_flows $load $reject_factor $param_k 2>/local/deploy/out.err 1>/local/deploy/out.txt &
echo "Started zedro"