#include <boost/asio.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include "threadpool.h"

using namespace async;

std::unique_ptr<thread_pool> db_pool;


void do_wait()
{
    //begin running in other thread
    std::cout << "Start wait id=" << std::this_thread::get_id() << std::endl;
    std::cout.flush();
    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for (std::chrono::milliseconds (1000));
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Waited " << elapsed.count() << " ms id=" << std::this_thread::get_id() << std::endl;
    std::cout.flush();
    //end running in other thread
}

void main_async (async_ctx ctx)
{
    std::cout << "async start thread id = " << std::this_thread::get_id() << std::endl;
    std::cout.flush();
    int waiting_time = 999;
    int some_value_from_database = 0;
    db_pool->run (do_wait);
    db_pool->run ([&]()
    {
        //begin running in other thread
        std::cout << "Start wait id=" << std::this_thread::get_id() << std::endl;
        std::cout.flush();
        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for (std::chrono::milliseconds (waiting_time));
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "Waited " << elapsed.count() << " ms id=" << std::this_thread::get_id() << std::endl;
        some_value_from_database = 1000;
        std::cout.flush();
        //end running in other thread
    });
    db_pool->run_and_wait (do_wait, ctx);
    std::cout << "value from db " << some_value_from_database << std::endl;
    std::cout << "async done thread id = " << std::this_thread::get_id() << std::endl;
    std::cout.flush();

    db_pool.reset();
}

int main()
{
    boost::asio::io_service io_service;
    db_pool.reset (new thread_pool (io_service, true));
    boost::asio::spawn(io_service, main_async);
    io_service.run();
    return 0;
}

