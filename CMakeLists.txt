cmake_minimum_required(VERSION 3.12)
project(node_gemfire)

set(CMAKE_CXX_STANDARD 11)


#TODO find gemfire/geode

include_directories(${CMAKE_JS_INC})

add_library(gemfire INTERFACE)
target_include_directories(gemfire INTERFACE SYSTEM
    /Users/sboorlagadda/workspace/node-gemfire/tmp/pivotal-gemfire-native/include
    )
target_link_libraries(gemfire INTERFACE
    /Users/sboorlagadda/workspace/node-gemfire/tmp/pivotal-gemfire-native/lib/libpivotal-gemfire.so
    )

file(GLOB SOURCE_FILES "src/*.cpp" "src/*.hpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
#target_sources(${PROJECT_NAME} INTERFACE ${SOURCE_FILES})

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

target_link_libraries(${PROJECT_NAME} gemfire)