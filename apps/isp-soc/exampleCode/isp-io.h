#ifndef __ISP_IO_H__
#define __ISP_IO_H__

/*****************************************************************************/

enum {
    IODEV_ISP,
    IODEV_SIF,
    IODEV_DXO,
    IODEV_COUNT,
};

struct ISPIODevice {
    int fd;
    size_t size;
    unsigned long phys;
    void *base;
};

/*****************************************************************************/

struct ISPIODevice *openISPIODevice(int devId);
void closeISPIODevice(struct ISPIODevice *dev);

#endif  /*__ISP_IO_H__*/
