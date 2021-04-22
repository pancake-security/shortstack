#!/usr/bin/env bash

cp -r ./bin/* /local/deploy/
cp cmakebuild/redis-stable/src/redis-server /local/deploy/
cp cmakebuild/redis-stable/src/redis-cli /local/deploy/
cp cmakebuild/redis-stable/src/redis-benchmark /local/deploy/
cp -r cmakebuild/wondershaper /local/deploy/

cp -r ./traces/* /local/deploy/
cp -r ./sbin/* /local/deploy/