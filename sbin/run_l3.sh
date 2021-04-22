#!/usr/bin/env bash

# Usage: sbin/run_l3.sh <hosts file> ..... 

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

hosts_csv=$1
replica=$2
shift;

l3_hosts=($(cat $hosts_csv | awk -v r=$replica '($2 == "L3" && $5 == r) { print $3 }'))
l3_instances=($(cat $hosts_csv | awk '($2 == "L3" && $5 == r) { print $1 }'))
l3_cores=($(cat $hosts_csv | awk '($2 == "L3" && $5 == r) { print $8 }'))

for i in "${!l3_hosts[@]}"; do 
  SERVERLIST="${l3_hosts[$i]}" $sbin/hosts.sh /local/deploy/start_l3.sh ${l3_cores[$i]} ${l3_instances[$i]} "$@"
done