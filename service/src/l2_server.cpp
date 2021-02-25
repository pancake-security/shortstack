#include "l2_server.h"
#include "l2_handler_factory.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

std::shared_ptr<TServer> l2_server::create(std::shared_ptr<l2_proxy> proxy, int port,
                           size_t num_worker_threads, size_t num_io_threads) {
    auto clone_factory = std::make_shared<l2_handler_factory>(proxy);
    auto proc_factory = std::make_shared<l2proxyProcessorFactory>(clone_factory);
    auto socket = std::make_shared<TNonblockingServerSocket>(port);
    socket->setSendTimeout(1200000);

    std::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(num_worker_threads);
    std::shared_ptr<PosixThreadFactory> threadFactory = std::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
    threadManager->threadFactory(threadFactory);
    threadManager->start();
    auto server = std::make_shared<TNonblockingServer>(proc_factory, std::make_shared<TBinaryProtocolFactory>(), socket, threadManager);

    server->setNumIOThreads(num_io_threads);
    return server;
}
