# Shortstack

(README Work in Progress)

Shortstack is a scalable and fault-tolerant proxy architecture that hides access patterns to cloud storage. It provides linear throughput scalability and provable security guarantees.   

### Requirements

```
sudo apt-get update
sudo apt -y install build-essential libbz2-dev zlib1g-dev cmake libssl-dev
```

Download and build that latest stable version of redis
```
./build_redis.sh
```

### Build instructions

```
git clone https://github.com/pancake-security/shortstack
cd shortstack
mkdir -p cmakebuild
cd cmakebuild
cmake ..
make
```

### Running on single box

Start KV

```
sudo sysctl -w vm.overcommit_memory=1
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
./cmakebuild/redis-stable/src/redis-server --bind 127.0.0.1 --port 6379 --save "" --appendonly no
```

Initialize KV and produce distribution information

```
./bin/proxy_server init -h ./singlebox.csv -o 1000 -t traces/helloworld -d ./distinfo.bin
```

Start L3 proxy

```
./bin/proxy_server l3 -h ./singlebox.csv -i l3 -s 1 -c 1 -y 1
```

Start L2 proxy

```
./bin/proxy_server l2 -h ./singlebox.csv -d distinfo.bin -i l2 -c 1 -y 1
```

Start L1 proxy

```
./bin/proxy_server l1 -h ./singlebox.csv -d distinfo.bin -i l1 -c 1 -y 1 -f
```

Initialize proxys

```
./bin/proxy_server manager -h ./singlebox.csv -s
```






