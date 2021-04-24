set(STORAGE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../libstorage/src)
set(CLIENT_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../libclient/src)
set(PROXY_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../proxy/src)
set(SERVICE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../service/src)
set(BENCHMARK_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${STORAGE_INCLUDE} ${CLIENT_INCLUDE} ${PROXY_INCLUDE} ${BENCHMARK_INCLUDE} ${SERVICE_INCLUDE} )


add_executable(proxy_benchmark
        src/proxy_benchmark.cpp

        ../libclient/src/client.h
        ../libclient/src/proxy_client.cpp
        ../libclient/src/proxy_client.h
        ../libclient/src/async_proxy_client.cpp
        ../libclient/src/async_proxy_client.h
        ../libclient/src/command_response_reader.cpp
        ../libclient/src/command_response_reader.h
        ../libclient/src/shortstack_client.h
        ../libclient/src/shortstack_client.cpp

        ../service/src/pancake_thrift_response.cpp
        ../service/src/thrift_response_client.cpp
        ../service/src/thrift_response_client.h
        ../service/src/thrift_response_client_map.cpp
        ../service/src/thrift_response_client_map.h
        ../service/src/thrift_server.cpp
        ../service/src/thrift_server.h
        ../service/src/thrift_handler_factory.cpp
        ../service/src/thrift_handler_factory.h
        ../service/src/thrift_handler.cpp
        ../service/src/thrift_handler.h
        ../service/src/pancake_thrift.cpp
        ../service/src/pancake_thrift.h
        ../service/src/proxy_types.cpp
        ../service/src/proxy_types.h
        ../service/src/proxy_constants.cpp
        ../service/src/proxy_constants.h

        ../service/src/block_request_service.h
        ../service/src/block_request_service.cpp
        ../service/src/block_response_service.h
        ../service/src/block_response_service.cpp

        ../service/src/l3proxy.h
        ../service/src/l3proxy.cpp

        ../proxy/src/pancake_proxy.cpp
        ../proxy/src/pancake_proxy.h
        ../proxy/src/proxy.h
        ../proxy/src/operation.h
        ../proxy/src/distribution.h
        ../proxy/src/distribution.cpp
        ../proxy/src/queue.h
        ../proxy/src/update_cache.h
        ../proxy/src/update_cache.cpp
        ../proxy/src/encryption_engine.h
        ../proxy/src/encryption_engine.cpp
        ../proxy/src/util.h
        ../proxy/src/util.cpp

        ../proxy/src/host_info.cpp
        ../proxy/src/host_info.h

        ../proxy/src/MurmurHash2.cpp
        ../proxy/src/MurmurHash2.h
        ../proxy/src/consistent_hash.cpp
        ../proxy/src/consistent_hash.h

        ../libstorage/src/storage_interface.h
        ../libstorage/src/redis.h
        ../libstorage/src/redis.cpp
        # ../libstorage/src/rocksdb.cpp
        # ../libstorage/src/rocksdb.h
        #../libstorage/src/memcached.cpp
        #../libstorage/src/memcached.h
        ../service/src/thrift_utils.h)
# add_dependencies(proxy_benchmark cpp_redis ssdb_rocks libcuckoo openssl_ep thrift_ep)
add_dependencies(proxy_benchmark cpp_redis libcuckoo openssl_ep thrift_ep)
target_link_libraries(proxy_benchmark
        ${THRIFTNB_LIBRARY}
        ${THRIFT_LIBRARY}
        ${LIBEVENT_LIBRARY}
        ${CPP_REDIS_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        # ${SSDB_CLIENT_LIBRARY}
        # ${LEVELDB_LIBRARY}
        # ${SNAPPY_LIBRARY}
        # ${JEMALLOC_LIBRARY}
        ${CMAKE_DL_LIBS}
        ${ZLIB_LIBRARIES}
        ${BZIP2_LIBRARIES}
        #${LIBCUCKOO_LIBRARIES}
        #${LIBMEMCACHED_LIBRARIES}
        ${PROXY_INCLUDE_LIBRARIES}
        ${STORAGE_INCLUDE_LIBRARIES}
        ${BENCHMARK_INCLUDE_LIBRARIES}
        ${SERVICE_INCLUDE_LIBRARIES}
        ${CLIENT_INCLUDE_LIBRARIES}
        Threads::Threads)


add_executable(shortstack_benchmark
        src/shortstack_benchmark.cpp

        ../libclient/src/client.h
        ../libclient/src/proxy_client.cpp
        ../libclient/src/proxy_client.h
        ../libclient/src/async_proxy_client.cpp
        ../libclient/src/async_proxy_client.h
        ../libclient/src/command_response_reader.cpp
        ../libclient/src/command_response_reader.h
        ../libclient/src/shortstack_client.h
        ../libclient/src/shortstack_client.cpp

        ../service/src/pancake_thrift_response.cpp
        ../service/src/thrift_response_client.cpp
        ../service/src/thrift_response_client.h
        ../service/src/thrift_response_client_map.cpp
        ../service/src/thrift_response_client_map.h
        ../service/src/thrift_server.cpp
        ../service/src/thrift_server.h
        ../service/src/thrift_handler_factory.cpp
        ../service/src/thrift_handler_factory.h
        ../service/src/thrift_handler.cpp
        ../service/src/thrift_handler.h
        ../service/src/pancake_thrift.cpp
        ../service/src/pancake_thrift.h
        ../service/src/proxy_types.cpp
        ../service/src/proxy_types.h
        ../service/src/proxy_constants.cpp
        ../service/src/proxy_constants.h

        ../service/src/l3proxy.h
        ../service/src/l3proxy.cpp

        ../service/src/block_request_service.h
        ../service/src/block_request_service.cpp
        ../service/src/block_response_service.h
        ../service/src/block_response_service.cpp

        ../proxy/src/pancake_proxy.cpp
        ../proxy/src/pancake_proxy.h
        ../proxy/src/proxy.h
        ../proxy/src/operation.h
        ../proxy/src/distribution.h
        ../proxy/src/distribution.cpp
        ../proxy/src/queue.h
        ../proxy/src/update_cache.h
        ../proxy/src/update_cache.cpp
        ../proxy/src/encryption_engine.h
        ../proxy/src/encryption_engine.cpp
        ../proxy/src/util.h
        ../proxy/src/util.cpp

        ../proxy/src/host_info.cpp
        ../proxy/src/host_info.h

        ../proxy/src/MurmurHash2.cpp
        ../proxy/src/MurmurHash2.h
        ../proxy/src/consistent_hash.cpp
        ../proxy/src/consistent_hash.h

        ../libstorage/src/storage_interface.h
        ../libstorage/src/redis.h
        ../libstorage/src/redis.cpp
        # ../libstorage/src/rocksdb.cpp
        # ../libstorage/src/rocksdb.h
        #../libstorage/src/memcached.cpp
        #../libstorage/src/memcached.h
        ../service/src/thrift_utils.h)
# add_dependencies(proxy_benchmark cpp_redis ssdb_rocks libcuckoo openssl_ep thrift_ep)
add_dependencies(shortstack_benchmark cpp_redis libcuckoo openssl_ep thrift_ep spdlog)
target_link_libraries(shortstack_benchmark
        ${THRIFTNB_LIBRARY}
        ${THRIFT_LIBRARY}
        ${LIBEVENT_LIBRARY}
        ${CPP_REDIS_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        # ${SSDB_CLIENT_LIBRARY}
        # ${LEVELDB_LIBRARY}
        # ${SNAPPY_LIBRARY}
        # ${JEMALLOC_LIBRARY}
        ${CMAKE_DL_LIBS}
        ${ZLIB_LIBRARIES}
        ${BZIP2_LIBRARIES}
        #${LIBCUCKOO_LIBRARIES}
        #${LIBMEMCACHED_LIBRARIES}
        ${PROXY_INCLUDE_LIBRARIES}
        ${STORAGE_INCLUDE_LIBRARIES}
        ${BENCHMARK_INCLUDE_LIBRARIES}
        ${SERVICE_INCLUDE_LIBRARIES}
        ${CLIENT_INCLUDE_LIBRARIES}
        ${SPDLOG_LIBRARIES}
        Threads::Threads)



add_executable(redis_benchmark
        src/redis_benchmark.cpp

        # ../libclient/src/client.h
        # ../libclient/src/proxy_client.cpp
        # ../libclient/src/proxy_client.h
        # ../libclient/src/async_proxy_client.cpp
        # ../libclient/src/async_proxy_client.h
        # ../libclient/src/command_response_reader.cpp
        # ../libclient/src/command_response_reader.h
        # ../libclient/src/shortstack_client.h
        # ../libclient/src/shortstack_client.cpp

        # ../service/src/pancake_thrift_response.cpp
        # ../service/src/thrift_response_client.cpp
        # ../service/src/thrift_response_client.h
        # ../service/src/thrift_response_client_map.cpp
        # ../service/src/thrift_response_client_map.h
        # ../service/src/thrift_server.cpp
        # ../service/src/thrift_server.h
        # ../service/src/thrift_handler_factory.cpp
        # ../service/src/thrift_handler_factory.h
        # ../service/src/thrift_handler.cpp
        # ../service/src/thrift_handler.h
        # ../service/src/pancake_thrift.cpp
        # ../service/src/pancake_thrift.h
        # ../service/src/proxy_types.cpp
        # ../service/src/proxy_types.h
        # ../service/src/proxy_constants.cpp
        # ../service/src/proxy_constants.h

        # ../service/src/l3proxy.h
        # ../service/src/l3proxy.cpp

        # ../service/src/block_request_service.h
        # ../service/src/block_request_service.cpp
        # ../service/src/block_response_service.h
        # ../service/src/block_response_service.cpp

        # ../proxy/src/pancake_proxy.cpp
        # ../proxy/src/pancake_proxy.h
        # ../proxy/src/proxy.h
        # ../proxy/src/operation.h
        # ../proxy/src/distribution.h
        # ../proxy/src/distribution.cpp
        # ../proxy/src/queue.h
        # ../proxy/src/update_cache.h
        # ../proxy/src/update_cache.cpp
        # ../proxy/src/encryption_engine.h
        # ../proxy/src/encryption_engine.cpp
        # ../proxy/src/util.h
        # ../proxy/src/util.cpp

        ../proxy/src/host_info.cpp
        ../proxy/src/host_info.h

        ../proxy/src/MurmurHash2.cpp
        ../proxy/src/MurmurHash2.h
        ../proxy/src/consistent_hash.cpp
        ../proxy/src/consistent_hash.h

        ../libstorage/src/storage_interface.h
        ../libstorage/src/redis.h
        ../libstorage/src/redis.cpp)
        # ../libstorage/src/rocksdb.cpp
        # ../libstorage/src/rocksdb.h
        #../libstorage/src/memcached.cpp
        #../libstorage/src/memcached.h
        # ../service/src/thrift_utils.h
        # )
# add_dependencies(proxy_benchmark cpp_redis ssdb_rocks libcuckoo openssl_ep thrift_ep)
add_dependencies(redis_benchmark cpp_redis libcuckoo openssl_ep thrift_ep spdlog)
target_link_libraries(redis_benchmark
        ${THRIFTNB_LIBRARY}
        ${THRIFT_LIBRARY}
        ${LIBEVENT_LIBRARY}
        ${CPP_REDIS_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        # ${SSDB_CLIENT_LIBRARY}
        # ${LEVELDB_LIBRARY}
        # ${SNAPPY_LIBRARY}
        # ${JEMALLOC_LIBRARY}
        ${CMAKE_DL_LIBS}
        ${ZLIB_LIBRARIES}
        ${BZIP2_LIBRARIES}
        #${LIBCUCKOO_LIBRARIES}
        #${LIBMEMCACHED_LIBRARIES}
        ${PROXY_INCLUDE_LIBRARIES}
        ${STORAGE_INCLUDE_LIBRARIES}
        ${BENCHMARK_INCLUDE_LIBRARIES}
        ${SERVICE_INCLUDE_LIBRARIES}
        ${CLIENT_INCLUDE_LIBRARIES}
        ${SPDLOG_LIBRARIES}
        Threads::Threads)




add_executable(pancake_benchmark
        src/pancake_benchmark.cpp

        ../libclient/src/client.h
        ../libclient/src/proxy_client.cpp
        ../libclient/src/proxy_client.h
        ../libclient/src/async_proxy_client.cpp
        ../libclient/src/async_proxy_client.h
        ../libclient/src/command_response_reader.cpp
        ../libclient/src/command_response_reader.h
        ../libclient/src/pancake_client.h
        ../libclient/src/pancake_client.cpp

        ../service/src/pancake_thrift_response.cpp
        ../service/src/thrift_response_client.cpp
        ../service/src/thrift_response_client.h
        ../service/src/thrift_response_client_map.cpp
        ../service/src/thrift_response_client_map.h
        ../service/src/thrift_server.cpp
        ../service/src/thrift_server.h
        ../service/src/thrift_handler_factory.cpp
        ../service/src/thrift_handler_factory.h
        ../service/src/thrift_handler.cpp
        ../service/src/thrift_handler.h
        ../service/src/pancake_thrift.cpp
        ../service/src/pancake_thrift.h
        ../service/src/proxy_types.cpp
        ../service/src/proxy_types.h
        ../service/src/proxy_constants.cpp
        ../service/src/proxy_constants.h

        ../service/src/l3proxy.h
        ../service/src/l3proxy.cpp

        ../service/src/block_request_service.h
        ../service/src/block_request_service.cpp
        ../service/src/block_response_service.h
        ../service/src/block_response_service.cpp

        ../proxy/src/pancake_proxy.cpp
        ../proxy/src/pancake_proxy.h
        ../proxy/src/proxy.h
        ../proxy/src/operation.h
        ../proxy/src/distribution.h
        ../proxy/src/distribution.cpp
        ../proxy/src/queue.h
        ../proxy/src/update_cache.h
        ../proxy/src/update_cache.cpp
        ../proxy/src/encryption_engine.h
        ../proxy/src/encryption_engine.cpp
        ../proxy/src/util.h
        ../proxy/src/util.cpp

        ../proxy/src/host_info.cpp
        ../proxy/src/host_info.h

        ../proxy/src/MurmurHash2.cpp
        ../proxy/src/MurmurHash2.h
        ../proxy/src/consistent_hash.cpp
        ../proxy/src/consistent_hash.h

        ../libstorage/src/storage_interface.h
        ../libstorage/src/redis.h
        ../libstorage/src/redis.cpp
        # ../libstorage/src/rocksdb.cpp
        # ../libstorage/src/rocksdb.h
        #../libstorage/src/memcached.cpp
        #../libstorage/src/memcached.h
        ../service/src/thrift_utils.h)
# add_dependencies(proxy_benchmark cpp_redis ssdb_rocks libcuckoo openssl_ep thrift_ep)
add_dependencies(pancake_benchmark cpp_redis libcuckoo openssl_ep thrift_ep spdlog)
target_link_libraries(pancake_benchmark
        ${THRIFTNB_LIBRARY}
        ${THRIFT_LIBRARY}
        ${LIBEVENT_LIBRARY}
        ${CPP_REDIS_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        # ${SSDB_CLIENT_LIBRARY}
        # ${LEVELDB_LIBRARY}
        # ${SNAPPY_LIBRARY}
        # ${JEMALLOC_LIBRARY}
        ${CMAKE_DL_LIBS}
        ${ZLIB_LIBRARIES}
        ${BZIP2_LIBRARIES}
        #${LIBCUCKOO_LIBRARIES}
        #${LIBMEMCACHED_LIBRARIES}
        ${PROXY_INCLUDE_LIBRARIES}
        ${STORAGE_INCLUDE_LIBRARIES}
        ${BENCHMARK_INCLUDE_LIBRARIES}
        ${SERVICE_INCLUDE_LIBRARIES}
        ${CLIENT_INCLUDE_LIBRARIES}
        ${SPDLOG_LIBRARIES}
        Threads::Threads)