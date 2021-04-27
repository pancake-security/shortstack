#!/usr/bin/env bash

# Usage: sbin/run_l3.sh <hosts file> ..... 

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

hosts_csv=$1
shift;

l3_hosts=($(cat $hosts_csv | awk '$2 == "L1" { print $3 }'))
l3_instances=($(cat $hosts_csv | awk '$2 == "L1" { print $1 }'))
l3_cores=($(cat $hosts_csv | awk '$2 == "L1" { print $8 }'))

for i in "${!l3_hosts[@]}"; do 
  SERVERLIST="${l3_hosts[$i]}" $sbin/hosts.sh /local/deploy/start_enc.sh ${l3_cores[$i]} ${l3_instances[$i]} "$@"
done