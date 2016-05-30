#pragma once

#include <string>
#include <vector>

namespace fs_helper {

struct file_item
{
    std::string file_name;
    std::string sha512;
};
void do_wait();

std::vector<file_item> get_file_items_from_path(const std::string & path);

}