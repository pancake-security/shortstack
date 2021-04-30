#!/usr/bin/env bash

# Usage: ./init_exp.sh 2 2 2 3 2 ycsb-c-1m 1000

nl1=$1
nl2=$2
nl3=$3
rep=$4
nredis=$5
trace=$6
objsz=$7
nopushbins=$8
noredis=$9

if [[ "$nopushbins" != "nopushbins" ]]
then
    echo "Pushing bins"

    ./push_bins.sh
fi

echo "Gen + sync hosts file"
python3 gen_hosts_file.py zedro.hosts /local/deploy/hosts.csv $nl1 1 16 $nl2 1 16 $nl3 16 1 $nredis 1 1 1 $rep $rep
sbin/sync.sh /local/deploy/hosts.csv

if [[ "$noredis" != "noredis" ]]
then

    echo "Starting redis"
    sbin/hosts.sh /local/deploy/stop_redis.sh; sbin/run_redis.sh /local/deploy/hosts.csv

    echo "Init KV store + push distinfo"
    /local/deploy/proxy_server init -h /local/deploy/hosts.csv -o $objsz -t /local/deploy/$trace -d /local/deploy/distinfo.bin && sbin/sync.sh /local/deploy/distinfo.bin

    # Hack: populate all keys on all redis
    echo "Populate all keys store"
    awk 'BEGIN {for(i=0;i<2000000;i++) {print "GET "i;}}' > traces/all_labels
    /local/deploy/redis_benchmark -i -h /local/deploy/hosts.csv -t traces/all_labels -z $objsz
fi

echo "Starting proxies"
sbin/hosts.sh /local/deploy/stop_proxys.sh; 
sleep 2; 
sbin/run_l3.sh /local/deploy/hosts.csv -s 24 -c 1 -y 30; 
sleep 4; 
sbin/run_l2.sh /local/deploy/hosts.csv $rep -c 1 -y 30; 
sleep 4; 
sbin/run_l1.sh /local/deploy/hosts.csv $rep -c 1; 
sleep 4; 
/local/deploy/proxy_server manager -h /local/deploy/hosts.csv -s

