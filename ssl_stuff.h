#pragma once

#include <string>

namespace ssl_helper {
    std::string base64(const unsigned char *input, int length);
    void get_file_sum(const std::string & file, std::string& rv_sum);
}