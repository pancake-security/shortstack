#!/usr/bin/env bash

# Usage: sbin/run_l1.sh <hosts file> ..... 

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

hosts_csv=$1
shift;

l1_hosts=($(cat $hosts_csv | awk '$2 == "L1" { print $3 }'))
l1_instances=($(cat $hosts_csv | awk '$2 == "L1" { print $1 }'))
l1_cores=($(cat $hosts_csv | awk '$2 == "L1" { print $8 }'))

for i in "${!l1_hosts[@]}"; do 
  SERVERLIST="${l1_hosts[$i]}" $sbin/hosts.sh /local/deploy/start_l1.sh ${l1_cores[$i]} ${l1_instances[$i]} "$@"
done