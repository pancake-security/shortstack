#!/usr/bin/env bash

# Usage: sbin/run_l2.sh <hosts file> ..... 

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

hosts_csv=$1
shift;

l2_hosts=($(cat $hosts_csv | awk '$2 == "L2" { print $3 }'))
l2_instances=($(cat $hosts_csv | awk '$2 == "L2" { print $1 }'))
l2_cores=($(cat $hosts_csv | awk '$2 == "L2" { print $5 }'))

for i in "${!l2_hosts[@]}"; do 
  SERVERLIST="${l2_hosts[$i]}" $sbin/hosts.sh /local/deploy/start_l2.sh ${l2_cores[$i]} ${l2_instances[$i]} "$@"
done