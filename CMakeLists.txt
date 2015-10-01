project(async_tasks)
cmake_minimum_required(VERSION 2.8)
find_package(Boost 1.53 COMPONENTS thread system coroutine context REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} )
link_directories(${Boost_LIBRARY_DIRS})
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

