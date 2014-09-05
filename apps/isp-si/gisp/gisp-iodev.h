#ifndef __IODEV_H__
#define __IODEV_H__

/*****************************************************************************/

struct IODevice {
    int fd;
    unsigned int mapSize;
    void *mapBase;
};

/*****************************************************************************/

struct IODevice *openIODevice(const char *path);
void closeIODevice(struct IODevice *iodev);

#endif  /*__IODEV_IODEV_H__*/
