#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "gisp-iodev.h"
#include "gisp-ioctl.h"
#include "debug.h"

/*****************************************************************************/

struct IODevice *openIODevice(const char *path)
{
    int ret;
    struct IODevice *io;
    struct gdm_isp_iodev_info info;

    ASSERT(path);

    io = (struct IODevice *)calloc(1, sizeof(*io));
    ASSERT(io);

    io->fd = open(path, O_RDWR);
    ASSERT(io->fd > 0 && "Open failed");

    ret = ioctl(io->fd, ISP_CTRL_IOC_GET_DEVINFO, &info);
    ASSERT(ret == 0);

    io->mapSize = info.size;
    io->mapBase = mmap(NULL,
            io->mapSize, PROT_READ|PROT_WRITE, MAP_SHARED, io->fd, 0);
    ASSERT(io->mapBase != MAP_FAILED && "Map failed.");
    DBG("IODEV[%s] Base=%p Size=0x%x", path, io->mapBase, io->mapSize);

    return io;
}

void closeIODevice(struct IODevice *io)
{
    int ret;

    ASSERT(io);
    ASSERT(io->fd > 0);
    ASSERT(io->mapBase != MAP_FAILED);

    ret = munmap(io->mapBase, io->mapSize);
    ASSERT(ret == 0);

    close(io->fd);
    free(io);
}

