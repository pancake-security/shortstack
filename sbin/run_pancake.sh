#!/usr/bin/env bash

# Usage: sbin/run_l1.sh <hosts file> ..... 

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

hosts_csv=$1
shift;
rep_factor=1;

for ((replica=$rep_factor-1; replica>=0; replica--)); do
  l1_hosts=($(cat $hosts_csv | awk -v r=$replica '($2 == "L1" && $5 == r) { print $3 }'))
  l1_instances=($(cat $hosts_csv | awk -v r=$replica '($2 == "L1" && $5 == r) { print $1 }'))
  l1_cores=($(cat $hosts_csv | awk -v r=$replica '($2 == "L1" && $5 == r) { print $8 }'))

  for i in "${!l1_hosts[@]}"; do 
    SERVERLIST="${l1_hosts[$i]}" $sbin/hosts.sh /local/deploy/start_pancake.sh ${l1_cores[$i]} ${l1_instances[$i]} "$@"
  done

  sleep 5;
done;

