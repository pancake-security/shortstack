#!/usr/bin/env bash

killall -q redis-server
sleep 2;
killall -q -9 redis-server

sudo /local/deploy/wondershaper/wondershaper -a ens5 -c
