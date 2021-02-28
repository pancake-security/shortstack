#!/usr/bin/env bash

config=$1

cat ~/zedro-eval/$config-host*.txt | grep flow_stats | wc -l
cat ~/zedro-eval/$config-host*.txt | grep flow_stats | awk '{ad+=$3; rej+=$4; unf+=$5} END {print ad,rej,unf,ad/(ad+rej)}'

rm lat.tmp

cat ~/zedro-eval/$config-host*.txt | grep flow_record | awk '$3 == "admitted" {print $11}' | sort -n > lat.tmp
nr=$(cat lat.tmp | wc -l)

paste <(echo "min") <(cat lat.tmp | head -n 1)
paste <(echo "mean") <(cat lat.tmp | awk '{sum+=$1} END {print sum/NR;}')
paste <(echo "99p") <(cat lat.tmp | head -n $((($nr*99)/100)) | tail -n 1)
paste <(echo "99.9p") <(cat lat.tmp | head -n $((($nr*999)/1000)) | tail -n 1)
paste <(echo "99.99p") <(cat lat.tmp | head -n $((($nr*9999)/10000)) | tail -n 1)
paste <(echo "max") <(cat lat.tmp | tail -n 1)

rm lat.tmp