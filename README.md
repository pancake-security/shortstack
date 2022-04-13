# Shortstack

Shortstack is a scalable and fault-tolerant proxy architecture that hides access patterns to cloud storage. It provides linear throughput scalability and provable security guarantees.   

### Requirements

This README assumes an Ubuntu Linux OS (tested on Ubuntu 16.04 and 18.04). Shortstack's source code is portable since it relies mostly on the C++ standard library, and it's external dependencies (e.g. OpenSSL, Thrift) are available on most platforms. We plan to update the README with instructions for other platforms in the future.

In order to build shortstack from source, the following dependencies need to be installed:

```
sudo apt-get update
sudo apt -y install build-essential libbz2-dev zlib1g-dev cmake libssl-dev
```


### Build instructions

The following will clone and build Shortstack:

```
git clone https://github.com/pancake-security/shortstack
cd shortstack
mkdir -p cmakebuild
cd cmakebuild
cmake ..
make
cd ..
```

Shortstack interfaces with a backend key-value store. We currently support Redis (extensions for other key-value stores can easily be added. Feel free to reach out with specific requests) 
The following will download and build that latest stable version of redis (to be called from root directory of the repo)

```
./build_redis.sh
```

### Running on single box

Here is a quick-start guide on how to run Shortstack locally where each of the proxy servers, the key-value store, and the client run as separate processes on the same machine. In this example, we will run the simples possible Shortstack configuration with 1 L1, 1 L2, and 1 L3 proxy server.

Various components in Shortstack rely on a host config file which lists information (e.g. IP address, ports) about all the nodes in the setup. For this example, we provide a sample config file `singlebox.csv`, that you can directly use.

The following steps need to be performed in order (all from the root directory of the repo):

1. Start the key-value store

```
./cmakebuild/redis-stable/src/redis-server --bind 127.0.0.1 --port 6379 --save "" --appendonly no
```

2. Initialize the key-value store and produce distribution information (that is later needed to initialize proxy servers). 

```
./bin/proxy_server init -h ./singlebox.csv -o 1000 -t traces/helloworld -d ./distinfo.bin
```

3. Start the L3 proxy. You should see a "Proxy server is reachable" output message if successful.

```
./bin/proxy_server l3 -h ./singlebox.csv -i l3 -s 1 -c 1 -y 1
```

4. Start the L2 proxy. You should see a "Proxy server is reachable" output message if successful.

```
./bin/proxy_server l2 -h ./singlebox.csv -d distinfo.bin -i l2 -c 1 -y 1
```

5. Start L1 proxy. You should see a "Proxy server is reachable" output message if successful.

```
./bin/proxy_server l1 -h ./singlebox.csv -d distinfo.bin -i l1 -c 1 -y 1 -f
```

6. Initialize the proxys

```
./bin/proxy_server manager -h ./singlebox.csv -s
```

7. Now that all the proxys have been started up and initialized, we can run a client program the performs GET/PUT operations. The below command will run a client that executes the operations from the following simple trace: `traces/helloworld`, which performs PUT operations on 5 keys, and then performs GET operations on the same keys to retrieve their values. 

```
./bin/shortstack_driver -h ./singlebox.csv -t traces/helloworld
```

If successful, you should see the values returned by the GET operations read "hello world shortstack is working".





