project(async_tasks)
cmake_minimum_required(VERSION 2.8)
find_package(Boost 1.53 COMPONENTS thread system filesystem coroutine context log program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} )
link_directories(${Boost_LIBRARY_DIRS})

file(GLOB SRC_LIST
*.cpp
*.h
md5/*.hh
md5/*.cc
)

add_executable(${PROJECT_NAME} ${SRC_LIST})

