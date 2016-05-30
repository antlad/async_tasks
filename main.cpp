#include "threadpool.h"
#include "fs_stuff.h"
#include "ssl_stuff.h"

#include <boost/asio.hpp>

#include <boost/asio/system_timer.hpp>

#include <iostream>
#include <thread>

using namespace async;

std::unique_ptr<thread_pool> pool;
std::string scan_folder;

void tick_timer (async_ctx ctx)
{
    int i = 0;
    for(;;)
    {
        if (!pool)
            return;
        boost::asio::system_timer timer(pool->io());
        timer.expires_from_now(std::chrono::milliseconds(1000));
        timer.async_wait(ctx);
        std::cout << i++ << " secs. tick 1000 ms thread id = " << std::this_thread::get_id() << std::endl;
    }
}

void main_async (async_ctx ctx)
{
    std::cout << "async start thread id = " << std::this_thread::get_id() << std::endl;

    boost::asio::spawn(pool->io(), tick_timer);
    auto fileslist = fs_helper::get_file_items_from_path(scan_folder);

    std::vector<task_holder_ptr> tasks;
    for (auto& item: fileslist)
    {
        tasks.push_back(pool->run([&](){
            ssl_helper::get_file_sum(item.file_name, item.sha512);
        }));
    }
    wait_all(tasks, ctx);

    for (const auto& item: fileslist)
        std::cout << "file_name " << item.file_name << " sha512 " << item.sha512 << std::endl;

    pool.reset();
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
            throw std::runtime_error("invalid args");
        
        scan_folder = std::string(argv[1]);
        
        boost::asio::io_service io_service;
        pool.reset(new thread_pool (io_service, true, std::thread::hardware_concurrency()));
        boost::asio::spawn(io_service, main_async);
        io_service.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    catch(...)
    {
        std::cerr << "unknown exception, aborting" << std::endl;
        exit(1);
    }
    return 0;
}

