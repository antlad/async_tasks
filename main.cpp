#include "threadpool.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

using namespace async;

std::unique_ptr<thread_pool> pool;
std::string scan_folder;

void do_wait()
{
    std::cout << "Start wait id=" << std::this_thread::get_id() << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for (std::chrono::milliseconds (1000));
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Waited " << elapsed.count() << " ms id=" << std::this_thread::get_id() << std::endl;
}

std::string base64(const unsigned char *input, int length)
{
    BIO *bmem, *b64;
    BUF_MEM *bptr;
    
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    
    std::string result;
    result.resize(bptr->length);

    memcpy((void *)result.data(), bptr->data, bptr->length - 1);
    result[bptr->length -1] = 0;
    BIO_free_all(b64);
    
    return result;
}

void get_file_sum(const std::string & file, std::string& rv_sum)
{
    using namespace boost::interprocess;
    std::filebuf fbuf;
    fbuf.open(file, std::ios_base::in);

    file_mapping m_file(file.c_str(), read_only);
    mapped_region region(m_file, read_only);

    void * addr       = region.get_address();
    std::size_t size  = region.get_size();
    
    char hash[SHA512_DIGEST_LENGTH];
    SHA512((const unsigned char*)addr, size, (unsigned char *)hash);
    
    rv_sum = base64((const unsigned char *)hash, SHA512_DIGEST_LENGTH);
}

struct file_item
{
    std::string file_name;
    std::string sha512;
};

std::vector<file_item> get_file_items_from_path(const std::string & path)
{
    std::cout << "searching folder from thread id = " << std::this_thread::get_id()<< std::endl;
    namespace fs = boost::filesystem;
    fs::path someDir(path);
    fs::directory_iterator end_iter;

    std::vector<file_item> result;

    if (!fs::exists(someDir) || !fs::is_directory(someDir))
        return result;
    
    for( fs::directory_iterator dir_iter(someDir) ; dir_iter != end_iter ; ++dir_iter)
    {
        if (!fs::is_regular_file(dir_iter->status()) )
            continue;
    
        file_item item;
        item.file_name = dir_iter->path().string();
        result.push_back(item);
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
        timer.expires_from_now(std::chrono::milliseconds(1000));
        timer.async_wait(ctx);
        std::cout << i++ << " secs. tick 1000 ms thread id = " << std::this_thread::get_id() << std::endl;
    }
}

void main_async (async_ctx ctx)
{
    std::cout << "async start thread id = " << std::this_thread::get_id() << std::endl;

    boost::asio::spawn(pool->io(), tick_timer);
    auto fileslist = get_file_items_from_path(scan_folder);

    std::vector<task_holder_ptr> tasks;
    for (auto& item: fileslist)
    {
        tasks.push_back(pool->run([&](){
            get_file_sum(item.file_name, item.sha512);
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
        pool.reset(new thread_pool (io_service, true, 20));
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

