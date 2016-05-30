#include "ssl_stuff.h"

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <string.h>
#include <fstream>

namespace ssl_helper {
    
    
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
}