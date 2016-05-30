project(async_tasks)
set(CMAKE_CXX_STANDARD 11)
cmake_minimum_required(VERSION 3.1)
find_package(Boost 1.53 COMPONENTS thread system filesystem coroutine context REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

file(GLOB SRC_LIST
main.cpp
threadpool.h
threadpool.cpp
)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})

