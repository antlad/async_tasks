#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include "threadpool.h"

using namespace async;

std::unique_ptr<thread_pool> pool;


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


std::list<std::string> get_filenames_from_path(const std::string & path)
{
    std::cout << "searching folder from thread id = " << std::this_thread::get_id() << std::endl;
    namespace fs = boost::filesystem;
    fs::path someDir(path);
    fs::directory_iterator end_iter;

    std::list<std::string> result;

    if ( fs::exists(someDir) && fs::is_directory(someDir))
    {
      for( fs::directory_iterator dir_iter(someDir) ; dir_iter != end_iter ; ++dir_iter)
      {
        if (fs::is_regular_file(dir_iter->status()) )
        {
            result.push_back(dir_iter->path().string());
        }
      }
    }
    return result;
}

void main_async (async_ctx ctx)
{
    std::cout << "async start thread id = " << std::this_thread::get_id() << std::endl;

    std::list<std::string> filenames;
    pool->run ([&]()
    {
        filenames = get_filenames_from_path("C:\\tmp\\test_files");
    })->wait(ctx);

//    for (const std::string & filename: filenames)
//    {
//        std::cout << "file: " << filename << std::endl;
//    }
    std::vector<task_holder_ptr> tasks;
    for (int i = 0; i < 10; ++i)
    {
        tasks.push_back(pool->run(do_wait));
    }
    wait_all(tasks, ctx);

    std::cout << "async done thread id = " << std::this_thread::get_id() << std::endl;
    std::cout.flush();

    pool.reset();
}

int main()
{
    boost::asio::io_service io_service;
    pool.reset (new thread_pool (io_service, true));
    boost::asio::spawn(io_service, main_async);
    io_service.run();
    return 0;
}

