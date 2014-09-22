#ifndef __OPTION_H__
#define __OPTION_H__

#include <stdint.h>
#include "stream.h"

/*****************************************************************************/

struct Option {
    struct OptionGlobal {
        int sysClkMul;
        int sysClkDiv;
        int display;
        int videoEncode;
        int captureEncode;
        int runState;
        int needPostEvent;
        int estimateIRQ;
    } global;

    struct OptionSensor {
        uint32_t id;
        uint32_t width;
        uint32_t height;
        uint32_t fps;
    } sensor;

    struct OptionPort {
        int disable;
        uint32_t width;
        uint32_t height;
        uint32_t cropTop;
        uint32_t cropLeft;
        uint32_t cropBottom;
        uint32_t cropRight;
        uint32_t pixelFormat;
        uint32_t bufferCount;
    } port[STREAM_PORT_COUNT];

    struct OptionDisplay {
        char *unixPath;
        int scale;
    } display;

    struct OptionVideoEncoder {
        char *basePath;
        int format;
    } videoEncoder;

    struct OptionJPEGEncoder {
        char *basePath;
        uint32_t captureCount;
        uint32_t ratio;
    } jpegEncoder;
};

/*****************************************************************************/

struct Option *createOption(const char *iniPath);
void deleteOption(struct Option *option);

#endif  /*__OPTION_H__*/
