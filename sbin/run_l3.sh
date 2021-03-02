#!/usr/bin/env bash

# Usage: sbin/run_l3.sh <hosts file> ..... 

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

hosts_csv=$1
shift;

l3_hosts=($(cat $hosts_csv | awk '$2 == "L3" { print $3 }'))
l3_instances=($(cat $hosts_csv | awk '$2 == "L3" { print $1 }'))
l3_cores=($(cat $hosts_csv | awk '$2 == "L3" { print $5 }'))

for i in "${!l3_hosts[@]}"; do 
  SERVERLIST="${l3_hosts[$i]}" $sbin/hosts.sh taskset -c "${l3_cores[$i]}" /local/deploy/proxy_server l3 -h /local/deploy/hosts.csv -i ${l3_instances[$i]} "$@" 2>/local/deploy/${l3_instances[$i]}.err 1>/local/deploy/${l3_instances[$i]}.out &
done