#include "fs_stuff.h"

#include <boost/filesystem.hpp>

#include <chrono>
#include <thread>

namespace fs_helper {
    
void do_wait()
{
    std::cout << "Start wait id=" << std::this_thread::get_id() << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for (std::chrono::milliseconds (1000));
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Waited " << elapsed.count() << " ms id=" << std::this_thread::get_id() << std::endl;
}    
    
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
}