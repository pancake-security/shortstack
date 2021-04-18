#!/usr/bin/env bash

config=$1

cat data/$config-client*.lat | awk -F',' '{l1+=$2; l2+=$3; l3_get+=$4; l3_to_crypto+=$5; l3_crypto += $6; l3_put += $7; l3_to_resp += $8} END {print l1/NR, l2/NR, l3_get/NR, l3_to_crypto/NR, l3_crypto/NR, l3_put/NR, l3_to_resp/NR}'