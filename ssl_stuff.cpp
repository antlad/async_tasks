#include "ssl_stuff.h"

#include <openssl/sha.h>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <string.h>
#include <fstream>
#include <string>

namespace ssl_helper {
    
    
void get_file_sum(const std::string & file, std::string& rv_sum)
{
    using namespace boost::interprocess;
    std::filebuf fbuf;
    fbuf.open(file, std::ios_base::in);

    file_mapping m_file(file.c_str(), read_only);
    mapped_region region(m_file, read_only);

    void * addr       = region.get_address();
    std::size_t size  = region.get_size();
    
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512((const unsigned char*)addr, size, (unsigned char *)hash);
    char buf[SHA512_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
        sprintf(buf+i*2, "%02x", hash[i]);    
    buf[SHA512_DIGEST_LENGTH*2]=0;

    rv_sum = std::string(buf);
}
}