#include <unistd.h>
#include <errno.h>

#include "file-io.h"
#include "debug.h"

/*****************************************************************************/

int safeWrite(int fd, void *buffer, size_t size)
{
    int ret;
    char *ptr;
    size_t nwrite;

    ASSERT(fd >= 0);

    nwrite = 0;
    ptr = (char *)buffer;

    while (nwrite < size) {
        ret = write(fd, &ptr[nwrite], size - nwrite);
        if (ret < 0) {
            switch (errno) {
                case EINTR:
                case EAGAIN:
                    continue;

                default:
                    DIE("write failed.");
                    break;
            }
        }
        else if (ret == 0)
            break;

        nwrite += ret;
    }

    return nwrite;
}
