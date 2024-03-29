set(STORAGE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../libstorage/src)
set(PROXY_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../proxy/src)
set(CLIENT_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../libclient/src)
set(SERVICE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../service/src)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${STORAGE_INCLUDE} ${CLIENT_INCLUDE} ${PROXY_INCLUDE} ${BENCHMARK_INCLUDE} ${SERVICE_INCLUDE} )

add_executable(storage_interface_test
        src/storage_interface_test.cpp
        ../libstorage/src/storage_interface.h
        ../libstorage/src/redis.cpp
        ../libstorage/src/redis.h
        ../proxy/src/MurmurHash2.cpp
        ../proxy/src/MurmurHash2.h
        ../proxy/src/consistent_hash.cpp
        ../proxy/src/consistent_hash.h
        # ../libstorage/src/rocksdb.cpp
        # ../libstorage/src/rocksdb.h
        #../libstorage/src/src/memcached.cpp
        #../libstorage/src/src/memcached.h
        )
# add_dependencies(storage_interface_test cpp_redis ssdb_rocks)
add_dependencies(storage_interface_test cpp_redis)
target_link_libraries(storage_interface_test
        Threads::Threads
        ${CPP_REDIS_LIBRARIES}
        # ${SSDB_CLIENT_LIBRARY}
        # ${LEVELDB_LIBRARY}
        # ${SNAPPY_LIBRARY}
        # ${JEMALLOC_LIBRARY}
        ${CMAKE_DL_LIBS}
        ${ZLIB_LIBRARIES}
        ${BZIP2_LIBRARIES}
        ${STORAGE_INCLUDE_LIBRARIES}
        #${LIBMEMCACHED_LIBRARIES}
        )

include_directories(${PROXY_INCLUDE})

add_executable(distribution_test
        src/distribution_test.cpp
        ../proxy/src/distribution.h
        ../proxy/src/distribution.cpp
        )
target_link_libraries(distribution_test
        Threads::Threads
        ${PROXY_INCLUDE_LIBRARIES}
        )

add_executable(encryption_test
        src/encryption_test.cpp
        ../proxy/src/encryption_engine.h
        ../proxy/src/encryption_engine.cpp
        ../proxy/src/util.h
        ../proxy/src/util.cpp
        )
target_link_libraries(encryption_test
        Threads::Threads
        ${PROXY_INCLUDE_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${CMAKE_DL_LIBS}
        )

add_executable(update_cache_test
        src/update_cache_test.cpp
        ../proxy/src/update_cache.h
        ../proxy/src/update_cache.cpp
        ../proxy/src/util.h
        ../proxy/src/util.cpp
        )
target_link_libraries(update_cache_test
        Threads::Threads
        ${PROXY_INCLUDE_LIBRARIES}
        )

add_executable(pancake_test
        src/pancake_test.cpp

        ../libclient/src/client.h
        ../libclient/src/proxy_client.cpp
        ../libclient/src/proxy_client.h

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
        ../service/src/thrift_utils.h
        ../service/src/proxy_types.cpp
        ../service/src/proxy_types.h
        ../service/src/proxy_constants.cpp
        ../service/src/proxy_constants.h

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
# add_dependencies(pancake_test cpp_redis ssdb_rocks libcuckoo openssl_ep thrift_ep)
add_dependencies(pancake_test cpp_redis libcuckoo openssl_ep thrift_ep)
target_link_libraries(pancake_test
        Threads::Threads
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


add_executable(shortstack_test
        src/shortstack_test.cpp

        ../libclient/src/client.h
        ../libclient/src/proxy_client.cpp
        ../libclient/src/proxy_client.h
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
        ../service/src/thrift_utils.h
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

        ../proxy/src/MurmurHash2.cpp
        ../proxy/src/MurmurHash2.h
        ../proxy/src/consistent_hash.cpp
        ../proxy/src/consistent_hash.h

        ../proxy/src/host_info.cpp
        ../proxy/src/host_info.h

        ../libstorage/src/storage_interface.h
        ../libstorage/src/redis.h
        ../libstorage/src/redis.cpp
        # ../libstorage/src/rocksdb.cpp
        # ../libstorage/src/rocksdb.h
        #../libstorage/src/memcached.cpp
        #../libstorage/src/memcached.h
        ../service/src/thrift_utils.h)
# add_dependencies(shortstack_test cpp_redis ssdb_rocks libcuckoo openssl_ep thrift_ep)
add_dependencies(shortstack_test cpp_redis libcuckoo openssl_ep thrift_ep)
target_link_libraries(shortstack_test
        Threads::Threads
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
        
        

add_executable(shortstack_driver
        src/shortstack_driver.cpp

        ../libclient/src/client.h
        ../libclient/src/proxy_client.cpp
        ../libclient/src/proxy_client.h
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
        ../service/src/thrift_utils.h
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

        ../proxy/src/MurmurHash2.cpp
        ../proxy/src/MurmurHash2.h
        ../proxy/src/consistent_hash.cpp
        ../proxy/src/consistent_hash.h

        ../proxy/src/host_info.cpp
        ../proxy/src/host_info.h

        ../libstorage/src/storage_interface.h
        ../libstorage/src/redis.h
        ../libstorage/src/redis.cpp
        # ../libstorage/src/rocksdb.cpp
        # ../libstorage/src/rocksdb.h
        #../libstorage/src/memcached.cpp
        #../libstorage/src/memcached.h
        ../service/src/thrift_utils.h)
# add_dependencies(shortstack_driver cpp_redis ssdb_rocks libcuckoo openssl_ep thrift_ep)
add_dependencies(shortstack_driver cpp_redis libcuckoo openssl_ep thrift_ep)
target_link_libraries(shortstack_driver
        Threads::Threads
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
