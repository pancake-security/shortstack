#!/usr/bin/env bash

killall redis-server
sleep 2;
killall -9 redis-server

sudo /local/deploy/wondershaper/wondershaper -a ens5 -c