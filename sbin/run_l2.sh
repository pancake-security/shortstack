#!/usr/bin/env bash

# Usage: sbin/run_l2.sh <hosts file> ..... 

sbin="`dirname "$0"`"
sbin="`cd "$sbin"; pwd`"

hosts_csv=$1
shift;
rep_factor=$1;
shift;

for ((replica=$rep_factor-1; replica>=0; replica--)); do
  l2_hosts=($(cat $hosts_csv | awk -v r=$replica '($2 == "L2" && $5 == r) { print $3 }'))
  l2_instances=($(cat $hosts_csv | awk -v r=$replica '($2 == "L2" && $5 == r) { print $1 }'))
  l2_cores=($(cat $hosts_csv | awk -v r=$replica '($2 == "L2" && $5 == r) { print $8 }'))

  for i in "${!l2_hosts[@]}"; do 
    SERVERLIST="${l2_hosts[$i]}" $sbin/hosts.sh /local/deploy/start_l2.sh ${l2_cores[$i]} ${l2_instances[$i]} "$@"
  done

  sleep 5;
done;
