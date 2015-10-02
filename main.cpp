#include <iostream>
#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "threadpool.h"
#include "md5/md5.hh"

using namespace async;

std::unique_ptr<thread_pool> pool;


void do_wait()
{
    std::cout << "processing frame " << std::endl;
    //begin running in other thread
    std::cout << "Start wait id=" << std::this_thread::get_id() << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for (std::chrono::milliseconds (1000));
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Waited " << elapsed.count() << " ms id=" << std::this_thread::get_id() << std::endl;
        //end running in other thread
}

void get_file_md5(const std::string & file, std::string & rv_md5)
{
    using namespace boost::interprocess;
    //Create a file
    std::filebuf fbuf;
    fbuf.open(file, std::ios_base::in);
    file_mapping m_file(file.c_str(), read_only);

    //Map the whole file in this process
    mapped_region region
    (m_file
    ,read_only
    );

    //Get the address of the mapped region
    void * addr       = region.get_address();
    std::size_t size  = region.get_size();

    rv_md5 = md5sum(addr, size);
}

struct file_item
{
    std::string file_name;
    std::string md5;
};

std::vector<file_item> get_file_items_from_path(const std::string & path)
{
    std::cout << "searching folder from thread id = " << std::this_thread::get_id()<< std::endl;
    namespace fs = boost::filesystem;
    fs::path someDir(path);
    fs::directory_iterator end_iter;

    std::vector<file_item> result;

    if ( fs::exists(someDir) && fs::is_directory(someDir))
    {
      for( fs::directory_iterator dir_iter(someDir) ; dir_iter != end_iter ; ++dir_iter)
      {
        if (fs::is_regular_file(dir_iter->status()) )
        {
            file_item item;
            item.file_name = dir_iter->path().string();
            result.push_back(item);
        }
      }
    }
    return result;
}
void tick_timer (async_ctx ctx)
{
    int i = 0;
    for(;;)
    {
        if (!pool)
            return;
        boost::asio::system_timer timer(pool->io());
        timer.expires_from_now(std::chrono::milliseconds(0100));
        timer.async_wait(ctx);
        std::cout << i++ << " secs. tick 1000 ms thread id = " << std::this_thread::get_id() << std::endl;
    }
}



void main_async (async_ctx ctx)
{
    std::cout << "async start thread id = " << std::this_thread::get_id() << std::endl;

    boost::asio::spawn(pool->io(), tick_timer);

    std::vector<file_item> fileitems;
    pool->run ([&]()
    {
        fileitems = get_file_items_from_path("C:\\tmp\\test_files");
    })->wait(ctx);

    std::vector<task_holder_ptr> tasks;
    for (file_item & item: fileitems)
    {
        tasks.push_back(pool->run([&]()
        {
            get_file_md5(item.file_name, item.md5);
        }));
    }
    wait_all(tasks, ctx);

    std::cout << "async done thread id = " << std::this_thread::get_id() << std::endl;
//    for (file_item & item: fileitems)
//    {
//        std::cout << "file_name " << item.file_name << " md5 " << item.md5 << std::endl;
//    }

    pool.reset();
}

int main()
{
//    std::string md5;
//    get_file_md5("C:\\tmp\\test_files\\file1", md5);
//    std::cout << md5 << std::endl;
//    return 0;
    boost::asio::io_service io_service;
    pool.reset(new thread_pool (io_service, true, 20));
    boost::asio::spawn(io_service, main_async);
    io_service.run();
    return 0;
}

