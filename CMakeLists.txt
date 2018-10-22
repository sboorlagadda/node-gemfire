cmake_minimum_required(VERSION 3.12)
project(node_gemfire)

set(CMAKE_CXX_STANDARD 11)


#TODO find gemfire/geode

add_library(gemfire INTERFACE)
target_include_directories(gemfire INTERFACE SYSTEM
        /Users/sboorlagadda/workspace/geode-native/install/include
    )
target_link_libraries(gemfire INTERFACE
        /Users/sboorlagadda/workspace/geode-native/install/lib/libapache-geode.dylib
    )



include(node_modules/@mapbox/cmake-node-module/module.cmake)

add_node_module(node_gemfire
    NAN_VERSION 2.11.1
    MINIMUM_NODE_ABI 48
    )

target_sources(node_gemfire INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/binding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cache_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cache_factory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/conversions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/conversions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dependencies.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dependencies.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/event_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/event_stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/events.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/exceptions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/functions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/functions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gemfire_worker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gemfire_worker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/region.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/region.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/region_event_listener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/region_event_listener.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/region_event_registry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/region_event_registry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/region_shortcuts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/region_shortcuts.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/result_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/result_stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/select_results.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/select_results.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/streaming_result_collector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/streaming_result_collector.hpp
    )

target_link_libraries(node_gemfire INTERFACE
    gemfire
    )