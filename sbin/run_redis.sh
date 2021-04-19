#!/usr/bin/env bash

# Usage: sbin/run_redis.sh <hosts file> ..... 

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

hosts_csv=$1
shift;

kv_hosts=($(cat $hosts_csv | awk '$2 == "KV" { print $3 }'))
kv_instances=($(cat $hosts_csv | awk '$2 == "KV" { print $1 }'))
kv_cores=($(cat $hosts_csv | awk '$2 == "KV" { print $8 }'))
kv_ports=($(cat $hosts_csv | awk '$2 == "KV" { print $4 }'))

for i in "${!kv_hosts[@]}"; do 
  SERVERLIST="${kv_hosts[$i]}" $sbin/hosts.sh /local/deploy/start_redis.sh ${kv_cores[$i]} ${kv_instances[$i]} ${kv_hosts[$i]} ${kv_ports[$i]} "$@"
done