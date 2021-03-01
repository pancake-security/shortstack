#!/bin/bash

cd cmakebuild
wget http://download.redis.io/redis-stable.tar.gz
tar xvzf redis-stable.tar.gz
cd redis-stable

make CFLAGS="-static" EXEEXT="-static" LDFLAGS="-I/usr/local/include/"

cd ../../