project(async_tasks)
set(CMAKE_CXX_STANDARD 11)
cmake_minimum_required(VERSION 3.1)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.53 COMPONENTS system filesystem coroutine context REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

set(SRC_LIST
main.cpp
threadpool.h
threadpool.cpp
ssl_stuff.cpp
ssl_stuff.h
fs_stuff.cpp
fs_stuff.h
)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})

