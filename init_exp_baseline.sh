#!/usr/bin/env bash

# Usage: ./init_exp.sh 2 2 2 3 2 ycsb-c-1m 1000

x=$1
trace=$2
objsz=$3

echo "Pushing bins"

./push_bins.sh

echo "Gen + sync hosts file"
python3 gen_hosts_file.py zedro.hosts /local/deploy/hosts.csv $x 1 16 0 1 16 0 16 1 $x 1 1 1 1 0
sbin/sync.sh /local/deploy/hosts.csv

echo "Starting redis"
sbin/hosts.sh /local/deploy/stop_redis.sh; sbin/run_redis.sh /local/deploy/hosts.csv

echo "Init KV store"
/local/deploy/redis_benchmark -i -h /local/deploy/hosts.csv -t traces/$trace -z $objsize

echo "Starting proxies"
sbin/hosts.sh /local/deploy/stop_proxys.sh; 
sleep 2; 
sbin/run_enc.sh /local/deploy/hosts.csv -s 24; 
sleep 4; 


