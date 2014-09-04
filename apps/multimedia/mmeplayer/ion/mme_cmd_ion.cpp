#include "mme_shell.h"
#include "ion_api.h"
#include <sys/mman.h>

int mme_command_ion_test1(int argc, char* argv[]) {

    int fd;
    int shared_fd;
    int ret;
    const int bufsize = 1024*1024;
    unsigned char* pbuf;

    fd = ion_open();
    if(fd >= 0) {

        ret = ion_alloc_fd(fd, bufsize, 0, ION_HEAP_CARVEOUT_MASK, 0, &shared_fd);
        ion_close(fd);

        if(shared_fd > 0) {

            pbuf = (unsigned char*)MMP_DRIVER_MMAP(NULL, bufsize, (PROT_READ | PROT_WRITE), MAP_SHARED, shared_fd, 0);
            if(pbuf != NULL) {
                memset(pbuf, 0xAA, bufsize);
            }

            ion_close(shared_fd);
        }
    }

    return 0;
}

