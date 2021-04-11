set(STORAGE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../libstorage/src)
set(CLIENT_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../libclient/src)
set(PROXY_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../proxy/src)
set(SERVICE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../service/src)
set(SERVER_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${STORAGE_INCLUDE} ${CLIENT_INCLUDE} ${PROXY_INCLUDE} ${SERVICE_INCLUDE} ${SERVER_INCLUDE} )


add_executable(proxy_server
        src/proxy_server.cpp

        ../libclient/src/client.h
        ../libclient/src/proxy_client.cpp
        ../libclient/src/proxy_client.h

        ../service/src/thrift_server.cpp
        ../service/src/thrift_server.h
        ../service/src/thrift_handler_factory.cpp
        ../service/src/thrift_handler_factory.h
        ../service/src/thrift_handler.cpp
        ../service/src/thrift_handler.h
        ../service/src/thrift_response_client_map.cpp
        ../service/src/thrift_response_client_map.h
        ../service/src/thrift_response_client.cpp
        ../service/src/thrift_response_client.h
        ../service/src/pancake_thrift_response.cpp
        ../service/src/pancake_thrift_response.h
        ../service/src/pancake_thrift.cpp
        ../service/src/pancake_thrift.h
        ../service/src/proxy_types.cpp
        ../service/src/proxy_types.h
        ../service/src/proxy_constants.cpp
        ../service/src/proxy_constants.h

        ../service/src/l2proxy.h
        ../service/src/l2proxy.cpp
        ../service/src/l2_handler_factory.h
        ../service/src/l2_handler_factory.cpp
        ../service/src/l2_handler.h
        ../service/src/l2_handler.cpp
        ../service/src/l2_server.h
        ../service/src/l2_server.cpp

        ../service/src/l3proxy.h
        ../service/src/l3proxy.cpp
        ../service/src/l3_handler_factory.h
        ../service/src/l3_handler_factory.cpp
        ../service/src/l3_handler.h
        ../service/src/l3_handler.cpp
        ../service/src/l3_server.h
        ../service/src/l3_server.cpp

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

        ../proxy/src/consistent_hash.cpp
        ../proxy/src/consistent_hash.h
        ../proxy/src/dist_info_constants.cpp
        ../proxy/src/dist_info_constants.h
        ../proxy/src/dist_info_types.cpp
        ../proxy/src/dist_info_types.h
        ../proxy/src/distribution_info.cpp
        ../proxy/src/distribution_info.h
        ../proxy/src/host_info.cpp
        ../proxy/src/host_info.h
        ../proxy/src/l1_proxy.cpp
        ../proxy/src/l1_proxy.h
        ../proxy/src/l2_interface.cpp
        ../proxy/src/l2_interface.h
        ../proxy/src/l2_proxy.cpp
        ../proxy/src/l2_proxy.h
        ../proxy/src/l3_interface.cpp
        ../proxy/src/l3_interface.h
        ../proxy/src/l3_proxy.cpp
        ../proxy/src/l3_proxy.h
        ../proxy/src/MurmurHash2.cpp
        ../proxy/src/MurmurHash2.h
        ../proxy/src/initializer.h
        ../proxy/src/initializer.cpp

        ../proxy/src/block_id_parser.h
        ../proxy/src/block_id_parser.cpp
        ../proxy/src/chain_request_client.h
        ../proxy/src/chain_request_client.cpp
        ../proxy/src/chain_response_client.h
        ../proxy/src/chain_response_client.cpp
        ../proxy/src/chain_module.h
        ../proxy/src/chain_module.cpp
        
        ../libstorage/src/storage_interface.h
        ../libstorage/src/redis.h
        ../libstorage/src/redis.cpp
        ../libstorage/src/dummy_kv.h
        ../libstorage/src/dummy_kv.cpp
        # ../libstorage/src/rocksdb.cpp
        # ../libstorage/src/rocksdb.h
        #../libstorage/src/memcached.cpp
        #../libstorage/src/memcached.h
        ../service/src/thrift_utils.h)
# add_dependencies(proxy_server cpp_redis ssdb_rocks libcuckoo openssl_ep thrift_ep)
add_dependencies(proxy_server cpp_redis libcuckoo openssl_ep thrift_ep spdlog)
target_link_libraries(proxy_server
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
        #${LIBMEMCACHED_LIBRARIES}
        ${SPDLOG_LIBRARIES}
        ${PROXY_INCLUDE_LIBRARIES}
        ${STORAGE_INCLUDE_LIBRARIES}
        ${BENCHMARK_INCLUDE_LIBRARIES}
        ${SERVICE_INCLUDE_LIBRARIES}
        ${CLIENT_INCLUDE_LIBRARIES}
        Threads::Threads)
