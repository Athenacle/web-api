#include "utils.h"

#include <fcntl.h>
#include <unistd.h>

#include <openssl/err.h>
#include <sys/stat.h>

#ifdef UNIX_HAVE_MMAP
#include <sys/mman.h>
#endif

using namespace cppmhd;

std::atomic_uint32_t UniversalReturnObject::count(0);


SerializableObject::~SerializableObject() {}

UniversalReturnObject::~UniversalReturnObject() {}

UniversalReturnObject::UniversalReturnObject()
{
    value_ = count++;
}

void* readfile(const std::string& fn, size_t& size)
{
    struct stat st;
    void* ptr = nullptr;
    int fd = open(fn.c_str(), O_RDONLY);
    if (fd < 0) {
        return nullptr;
    }
    if (fstat(fd, &st) == 0) {
#ifdef UNIX_HAVE_MMAP
        ptr = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        size = st.st_size;
#endif
    }
    close(fd);
    return ptr;
}


void closefile(void* ptr, size_t size)
{
#ifdef UNIX_HAVE_MMAP
    munmap(ptr, size);
#endif
}


void setup_openssl()
{
    ERR_load_ERR_strings();
    ERR_load_crypto_strings();
}