#include "option.h"
#include "iniparser.h"
#include "gisp/gisp-dxo.h"
#include "debug.h"

/*****************************************************************************/

#define DEFAULT_BUFFER_COUNT            (1)
#define safeFree(p)                     do { if (p) free(p); } while (0)
#define MAKE_COLOR_FORMAT(fmt)          .name = #fmt, .pixelformat = fmt

#define ARRAY_SIZE(a)                   (sizeof(a) / sizeof(a[0]))

/*****************************************************************************/

static uint32_t str2PixelFormat(const char *name)
{
    const struct DXOPixelFormat *dxoPixelFormat;
    dxoPixelFormat = DXOGetPixelFormatByName(name);
    ASSERT(dxoPixelFormat);
    return dxoPixelFormat->v4l2PixelFormat;
}

static uint32_t str2State(const char *name)
{
    uint32_t state = DXO_STATE_PREVIEW;

    if (strcasecmp(name, "video") == 0)
        state = DXO_STATE_VIDEOREC;

    return state;
}

static int str2PortId(const char *portName)
{
    int i;
    int portId = -1;
    static const char *__portName[] = {
        "capture", "video", "display", "facedetect",
    };

    if (portName) {
        for (i = 0; i < ARRAY_SIZE(__portName); i++) {
            if (strcasecmp(portName, __portName[i]) == 0) {
                portId = i;
                break;
            }
        }

        if (portId < 0) {
            DBG("PortName [%s] is unknown. Ignored.", portName);
        }
    }

    return portId;
}

/*****************************************************************************/

static void parseGlobal(dictionary *dict, struct Option *option)
{
    char *str;
    option->global.sysClkMul = iniparser_getint(dict, "Global:SysClkMul", 32);
    option->global.sysClkDiv = iniparser_getint(dict, "Global:SysClkDiv", 0);
    option->global.display = iniparser_getint(dict, "Global:Display", 0);
    option->global.videoEncode = iniparser_getint(dict, "Global:VideoEncode", 0);
    option->global.captureEncode = iniparser_getint(dict, "Global:CaptureEncode", 0);
    option->global.needPostEvent = iniparser_getint(dict, "Global:NeedPostEvent", 0);
    option->global.estimateIRQ = iniparser_getint(dict, "Global:EstimateIRQ", 0);
    option->global.showFPS = iniparser_getint(dict, "Global:ShowFPS", 0);
    str = iniparser_getstring(dict, "Global:RunState", "preview");
    option->global.runState = str2State(str);
    str = iniparser_getstring(dict, "Global:BT601", NULL);
    option->global.bt601PortId = str2PortId(str);
}

static void parseSensor(dictionary *dict, struct Option *option)
{
    option->sensor.id = iniparser_getint(dict, "Sensor:Id", 0);

    option->sensor.width = iniparser_getint(dict, "Sensor:Width", 0);
    ASSERT(option->sensor.width > 0);
    
    option->sensor.height = iniparser_getint(dict, "Sensor:Height", 0);
    ASSERT(option->sensor.height > 0);

    option->sensor.fps = iniparser_getint(dict, "Sensor:FPS", 0);
    ASSERT(option->sensor.fps > 0);
}

static void parsePort(dictionary *dict, struct Option *option, int index)
{
    char key[256];
    char *pixelFormat;
    static const char *portName[] = {
        "Capture", "Video", "Display", "FaceDetect"
    };

    struct OptionPort *port = &option->port[index];

    snprintf(key, sizeof(key) - 1, "Port-%s:Disable", portName[index]);
    port->disable = iniparser_getint(dict, key, 0);
    DBG("Port-%s:Disable = %d", portName[index], port->disable);

    if (port->disable)
        return;

    snprintf(key, sizeof(key) - 1, "Port-%s:Width", portName[index]);
    port->width = iniparser_getint(dict, key, option->sensor.width);

    snprintf(key, sizeof(key) - 1, "Port-%s:Height", portName[index]);
    port->height = iniparser_getint(dict, key, option->sensor.height);

    snprintf(key, sizeof(key) - 1, "Port-%s:CropTop", portName[index]);
    port->cropTop = iniparser_getint(dict, key, 0);

    snprintf(key, sizeof(key) - 1, "Port-%s:CropLeft", portName[index]);
    port->cropLeft = iniparser_getint(dict, key, 0);

    ASSERT(option->sensor.width > 0);
    snprintf(key, sizeof(key) - 1, "Port-%s:CropRight", portName[index]);
    port->cropRight = iniparser_getint(dict, key, option->sensor.width - 1);
    ASSERT(port->cropRight > 0);
    ASSERT(port->cropRight < option->sensor.width);

    ASSERT(option->sensor.height > 0);
    snprintf(key, sizeof(key) - 1, "Port-%s:CropBottom", portName[index]);
    port->cropBottom = iniparser_getint(dict, key, option->sensor.height - 1);
    ASSERT(port->cropBottom > 0);
    ASSERT(port->cropBottom < option->sensor.height);

    snprintf(key, sizeof(key) - 1, "Port-%s:PixelFormat", portName[index]);
    pixelFormat = iniparser_getstring(dict, key, NULL);
    ASSERT(pixelFormat);
    port->pixelFormat = str2PixelFormat(pixelFormat);

    snprintf(key, sizeof(key) - 1, "Port-%s:BufferCount", portName[index]);
    port->bufferCount = iniparser_getint(dict, key, DEFAULT_BUFFER_COUNT);

    snprintf(key, sizeof(key) - 1, "Port-%s:WriteYUV", portName[index]);
    port->writeYUV = iniparser_getint(dict, key, 0);

    snprintf(key, sizeof(key) - 1, "Port-%s:EnableEffect", portName[index]);
    port->enableEffect = iniparser_getint(dict, key, 0);
}

static void parseDisplay(dictionary *dict, struct Option *option)
{
    char *tmp;
    option->display.scale = iniparser_getint(dict, "Display:Scale", 0);
    tmp = iniparser_getstring(dict, "Display:SocketPath", NULL);
    ASSERT(tmp);
    option->display.unixPath = strdup(tmp);
}

static void parseVideoEncoder(dictionary *dict, struct Option *option)
{
    int i;
    char *tmp;
    static const char *fmtString[] = { "h264", "mpeg4", "h263" };

    tmp = iniparser_getstring(dict, "VideoEncoder:BasePath", NULL);
    ASSERT(tmp);
    option->videoEncoder.basePath = strdup(tmp);
    ASSERT(option->videoEncoder.basePath);

    option->videoEncoder.format = -1;
    tmp = iniparser_getstring(dict, "VideoEncoder:Format", NULL);
    ASSERT(tmp);
    for (i = 0; i < ARRAY_SIZE(fmtString); i++) {
        if (strcasecmp(tmp, fmtString[i]) == 0)
            option->videoEncoder.format = i;
    }
    ASSERT(option->videoEncoder.format >= 0);
}

static void parseJPEGEncoder(dictionary *dict, struct Option *option)
{
    char *tmp;

    tmp = iniparser_getstring(dict, "JPEGEncoder:BasePath", NULL);
    ASSERT(tmp);
    option->jpegEncoder.basePath = strdup(tmp);
    ASSERT(option->jpegEncoder.basePath);

    option->jpegEncoder.captureCount =
        iniparser_getint(dict, "JPEGEncoder:CaptureCount", 1);

    option->jpegEncoder.ratio = iniparser_getint(dict, "JPEGEncoder:Ratio", 70);
}

/*****************************************************************************/

struct Option *createOption(const char *iniPath)
{
    int i;
    dictionary *dict;
    struct Option *option;

    ASSERT(iniPath);

    dict = iniparser_load(iniPath);
    ASSERT(dict);

    option = calloc(1, sizeof(*option));
    ASSERT(option);

    parseGlobal(dict, option);
    parseSensor(dict, option);

    for (i = 0; i < STREAM_PORT_COUNT; i++)
        parsePort(dict, option, i);

    if (option->global.display)
        parseDisplay(dict, option);

    if (option->global.videoEncode)
        parseVideoEncoder(dict, option);

    if (option->global.captureEncode)
        parseJPEGEncoder(dict, option);

    iniparser_freedict(dict);

    return option;
}

void deleteOption(struct Option *option)
{
    ASSERT(option);
    safeFree(option->display.unixPath);
    safeFree(option->videoEncoder.basePath);
    safeFree(option->jpegEncoder.basePath);
    safeFree(option);
}
