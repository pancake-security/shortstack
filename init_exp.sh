#!/usr/bin/env bash

# Usage: ./init_exp.sh 2 3 ycsb-c-1m 1000

x=$1
rep=$2
trace=$3
objsz=$4

echo "Pushing bins"

./push_bins.sh

echo "Gen + sync hosts file"
python3 gen_hosts_file.py zedro.hosts /local/deploy/hosts.csv $1 1 16 $1 1 16 $1 16 1 $x 1 1 1 $rep $rep
sbin/sync.sh /local/deploy/hosts.csv

echo "Starting redis"
sbin/hosts.sh /local/deploy/stop_redis.sh; sbin/run_redis.sh /local/deploy/hosts.csv

echo "Init KV store + push distinfo"
/local/deploy/proxy_server init -h /local/deploy/hosts.csv -o $objsz -t /local/deploy/$trace -d /local/deploy/distinfo.bin && sbin/sync.sh /local/deploy/distinfo.bin

echo "Starting proxies"
sbin/hosts.sh /local/deploy/stop_proxys.sh; 
sleep 2; 
sbin/run_l3.sh /local/deploy/hosts.csv -s 24 -c 1 -y 30; 
sleep 4; 
sbin/run_l2.sh /local/deploy/hosts.csv $rep -c 1 -y 30; 
sleep 2; 
sbin/run_l1.sh /local/deploy/hosts.csv $rep -c 1; 
sleep 4; 
/local/deploy/proxy_server manager -h /local/deploy/hosts.csv -s

