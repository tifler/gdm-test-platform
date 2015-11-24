#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/videodev2.h>

#include "gisp-dxo.h"
#include "gisp-iodev.h"
#include "gdm-isp-ioctl.h"
#include "DxOISP.h"
#include "DxOISP_ahblAccess.h"
#include "debug.h"

/*****************************************************************************/

#define DEFAULT_IRQ_TIMEOUT             (1000)
#define DXO_SUBDEV_PATH                 "/dev/v4l-subdev2"
#define DXO_IODEV_PATH                  "/dev/gisp-ctrl-dxo"

#define DXO_WRITE(a, v) \
    (*(volatile uint32_t *)((char *)DXOBase + (a)) = ((uint32_t)(v)))
#define DXO_READ(a)     \
    (*(volatile uint32_t *)((char *)DXOBase + (a)))

/*****************************************************************************/

#define DXO_CONFIG_FLAG_OUTPUT(id)      (1 << (id))
// stSync.stControl.*(Except per port config.)
#define DXO_CONFIG_FLAG_CONTROL         (1 << 8)

/*****************************************************************************/

#define DxOISP_CommandSetAuto(v, m)       \
    DxOISP_CommandSet(DxOISP_CMD_OFFSET(m), sizeof(v.m), &(v.m))

#define MAKE_PIXEL_FORMAT(fmt)          .name = #fmt, .v4l2PixelFormat = fmt
#define ARRAY_SIZE(a)                   (sizeof(a) / sizeof(a[0]))

/*****************************************************************************/

struct DXOPortInfo {
    const char *name;
    uint32_t hMin;
    uint32_t hMax;
    uint32_t vMin;
    uint32_t vMax;
};

struct DXO {
    struct IODevice *io;

    int stopIRQThread;
    pthread_t irqThread;                // 

    unsigned long configFlags;
    unsigned long enabledOutput;
    ts_DxOIspCmd dxoCmd;
    ts_DxOIspStatus dxoStatus;

    int needPostEvent;
    int estimateIRQ;

    int (*irqCallback)(void *);
    void *irqCallbackParam;
};

/*****************************************************************************/

static unsigned char *DXOBase;
static int DXOFd;

static struct DXOPortInfo dxoPorts[] = {
    /* From F/W Integration Guide 2.4.1 */
    [DXO_OUTPUT_DISPLAY] = {
        .name = "display",
        .hMin = 176,
        .vMin = 144,
        .hMax = 1920,
        .vMax = 1080,
    },
    [DXO_OUTPUT_VIDEO] = {
        .name = "video",
        .hMin = 176,
        .vMin = 144,
        .hMax = 2400,
        .vMax = 1350,
    },
    [DXO_OUTPUT_CAPTURE] = {
        .name = "capture",
        .hMin = 176,
        .vMin = 144,
        .hMax = -1, // max sensor size
        .vMax = -1, // max sensor size
    },
    [DXO_OUTPUT_FD] = {
        .name = "face-detect",
        .hMin = 176,
        .vMin = 144,
        .hMax = 1280,
        .vMax = 720,
    },
};

// XXX WDMA assumes that DxO *ALWAYS* output as 
//     DxOISP_FORMAT_YUV422,
//     DxOISP_YUV422ORDER_YUYV,
//     DxOISP_ENCODING_YUV_601FU.
//
//     So we should not change DxO pixel format settings.
//     We should change WDMA format instead.
static struct DXOPixelFormat dxoPixelFormats[] = {
    // 422_1P
    {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_UYVY),
        .description = "YUV 4:2:2 packed, CbYCrY",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    }, {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_VYUY),
        .description = "YUV 4:2:2 packed, CrYCbY",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    }, {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_YUYV),
        .description = "YUV 4:2:2 packed, YCbYCr",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    }, {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_YVYU),
        .description = "YUV 4:2:2 packed, YCrYCb",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    },
    // 422_2P
    {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_NV16),
        .description = "YUV 4:2:2 planar, Y/CbCr",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    }, {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_NV61),
        .description = "YUV 4:2:2 planar, Y/CrCb",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    },
    // 422_3P
    {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_YUV422P),
        .description = "YUV 4:2:2 3-planar, Y/Cb/Cr",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    },
    // 420_2P
    // 주의: NV12와 NV12M의 차이는 2Plane이 연속이냐 아니냐의 차이이다.
    {
        // NV12의 경우 plane을 두개로 하지만 실제로는 하나로 할당 받도록 처리
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_NV12),
        .description = "YUV 4:2:0 contiguous 2-planar, Y/CbCr",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    },
    {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_NV12M),
        .description = "YUV 4:2:0 non-contiguous 2-planar, Y/CbCr",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    },
    {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_NV21),
        .description = "YUV 4:2:0 contiguous 2-planar, Y/CrCb",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    },
    {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_NV21M),
        .description = "YUV 4:2:0 non-contiguous 2-planar, Y/CrCb",
        .v4l2PixelFormat = V4L2_PIX_FMT_NV21M,
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    },
    // 420_3P
    {
        MAKE_PIXEL_FORMAT(V4L2_PIX_FMT_YUV420M),
        .description = "YUV 4:2:0 non-contiguous 3-planar, Y/Cb/Cr",
        .dxoFormat = DxOISP_FORMAT_YUV422,
        .dxoOrder = DxOISP_YUV422ORDER_YUYV,
        .dxoEncoding = DxOISP_ENCODING_YUV_601FU,
    },
};

static const char *state2String[] = {
    "STANDBY", 
    "IDLE",
    "PREVIEW",
    "CAPTURE_A",
    "CAPTURE_B",
    "VIDEOREC",
};

/*****************************************************************************/
// DxOISP Callback Functions

void DxOISP_setRegister(const uint32_t uiOffset, uint32_t uiValue) 
{
    DXO_WRITE(uiOffset, uiValue);
}

uint32_t DxOISP_getRegister(const uint32_t uiOffset) 
{
    return DXO_READ(uiOffset);
}

// DxO 구조상 Offset을 증가 시키지 않고 한 Register에서 계속 읽는다.
// SOC의 설명에 따르면 이 레지스터가 아마도 FIFO로 되어 있고
// 그 레지스터를 통해 여러개의 데이터를 읽어 온다고 한다.
// 따라서 Offset 증가 안하는게 문제가 되지 않는다.
void DxOISP_getMultiRegister(
        const uint32_t uiOffset, uint32_t* puiDst, uint32_t uiNbElem) 
{
    uint32_t offset = uiOffset;

    while (uiNbElem >= 4) {
        *puiDst++ = DXO_READ(offset);
        *puiDst++ = DXO_READ(offset);
        *puiDst++ = DXO_READ(offset);
        *puiDst++ = DXO_READ(offset);
        uiNbElem -= 4;
    }

    if (uiNbElem >= 2) {
        *puiDst++ = DXO_READ(offset);
        *puiDst++ = DXO_READ(offset);
        uiNbElem -= 2;
    }

    if (uiNbElem >= 1) {
        *puiDst++ = DXO_READ(offset);
        uiNbElem -= 1;
    }
    ASSERT(uiNbElem == 0);
}

void DxOISP_setCopyRegister(
        const uint32_t uiOffset, const uint32_t* puiSrc , uint32_t uiNbElem) 
{
    uint32_t offset = uiOffset;

    while (uiNbElem >= 4) {
        DXO_WRITE((offset + 0), *puiSrc++);
        DXO_WRITE((offset + 4), *puiSrc++);
        DXO_WRITE((offset + 8), *puiSrc++);
        DXO_WRITE((offset + 12), *puiSrc++);
        offset += 16;
        uiNbElem -= 4;
    }

    if (uiNbElem >= 2) {
        DXO_WRITE((offset + 0), *puiSrc++);
        DXO_WRITE((offset + 4), *puiSrc++);
        offset += 8;
        uiNbElem -= 2;
    }

    if (uiNbElem >= 1) {
        DXO_WRITE(offset, *puiSrc++);
        offset += 4;
        uiNbElem -= 1;
    }
    ASSERT(uiNbElem == 0);
}

int DxOISP_Printf(const char *format, ...)
{
    int val;
    va_list args;
    va_start(args, format);
    val = vfprintf(stdout, format, args);
    va_end(args);
    return val;
}

void DxOISP_PixelStuckHandler(void)
{
    DBG("Sensor Error.");
    //DxOISP_DumpStatus();
}

/*****************************************************************************/

static int waitIRQ(struct DXO *dxo, int timeout)
{
    int ret;

    ret = ioctl(dxo->io->fd, ISP_CTRL_IOC_WAITINT, &timeout);

    return ret;
}

static void *irqHandlerThread(void *arg)
{
    int ret;
    int userRet;
    int event_cnt = 0;
    uint8_t i2c_data;
    struct DXO *dxo = (struct DXO *)arg;
    extern int initdone;
    struct timeval prevIRQTime;
    struct timeval currIRQTime;
    unsigned int diff;

    memset(&prevIRQTime, 0, sizeof(prevIRQTime));

    while (!dxo->stopIRQThread) {

        ret = waitIRQ(dxo, DEFAULT_IRQ_TIMEOUT);

        if (dxo->estimateIRQ) {
            gettimeofday(&currIRQTime, (struct timezone *)0);

            if (prevIRQTime.tv_sec > 0) {
                diff = (currIRQTime.tv_sec - prevIRQTime.tv_sec) * 1000000;
                diff += (currIRQTime.tv_usec - prevIRQTime.tv_usec);
                DBG("IRQ Interval = %u usec", diff);
            }
            prevIRQTime = currIRQTime;
        }

        if (ret < 0) {
            break;
        }
        else if (ret == 0) {
            DBG("Timeout.");
            continue;
        }

        //DBG("IRQ=0x%x", ret);

        if (ret & 0x18000) {
#if 0
            if (initdone && event_cnt < 2) {
                do {
                    i2c_data = 0x42;
                    I2C_Write(0x3008, &i2c_data, 1);
                    i2c_data = 0;
                    I2C_Read(0x3008, &i2c_data, 1);
                    if(i2c_data != 0x42)
                        ISP_PRINTF("i2cm stop error\n");
                } while( i2c_data != 0x42);
            }
#endif  /*0*/

            if (dxo->estimateIRQ) {
                ESTIMATE_START("DxOISP_Event()");
                DxOISP_Event();
                ESTIMATE_STOP();
            }
            else {
                DxOISP_Event();
            }

#if 0
            if (initdone && event_cnt < 2) {
                do {
                    i2c_data = 0x02;
                    I2C_Write(0x3008, &i2c_data, 1);
                    i2c_data = 0;
                    I2C_Read(0x3008, &i2c_data, 1);
                    if(i2c_data != 0x02)
                        ISP_PRINTF("i2cm start error\n");
                } while( i2c_data != 0x02);

                event_cnt++;
            }
#endif  /*0*/

            if (dxo->needPostEvent) {
                // If you turn on next statement, you will catch floating point
                // exception.(tifler:2014.09.03)
                DxOISP_PostEvent();
            }
        }

        // user callback
        if (dxo->irqCallback) {
            userRet = dxo->irqCallback(dxo->irqCallbackParam);
            if (userRet != 0) {
                DBG("User callback return non-zero value. Stop irq handler.");
                break;
            }
        }
    }

    return dxo;
}

static void startIRQHandler(struct DXO *dxo)
{
    int ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    dxo->stopIRQThread = 0;
    ret = pthread_create(&dxo->irqThread, &attr, irqHandlerThread, dxo);
    ASSERT(ret == 0 && "IRQ Thread Create failed.");

    pthread_attr_destroy(&attr);
}

static void stopIRQHandler(struct DXO *dxo)
{
    int ret;
    dxo->stopIRQThread = 1;
    ret = pthread_join(dxo->irqThread, NULL);
    ASSERT(ret == 0);
}

static struct DXOPixelFormat *getPixelFormat(uint32_t pixelFormat)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(dxoPixelFormats); i++) {
        if (dxoPixelFormats[i].v4l2PixelFormat == pixelFormat) {
            DBG("DXO Found PixelFormat = %s", dxoPixelFormats[i].name);
            return &dxoPixelFormats[i];
        }
    }

    return NULL;
}

/*****************************************************************************/

struct DXO *DXOInit(const struct DXOSystemConfig *conf)
{
    struct DXO *dxo;
    uint8_t mode;
    uint16_t frmTime;
	uint32_t sysFreq;

    dxo = (struct DXO *)calloc(1, sizeof(*dxo));
    ASSERT(dxo);

    dxo->io = openIODevice(DXO_IODEV_PATH);
    DXOBase = dxo->io->mapBase;
    DXOFd = dxo->io->fd;

    dxo->needPostEvent = (conf->needPostEvent ? 1 : 0);
    dxo->estimateIRQ = (conf->estimateIRQ ? 1 : 0);

    startIRQHandler(dxo);

    sysFreq = (conf->sysFreqMul << 
            DxOISP_SYSTEM_CLOCK_FRAC_PART) / conf->sysFreqDiv;

	DxOISP_Init(sysFreq);

    frmTime = (conf->frmTimeMul << 
            DxOISP_MIN_INTERFRAME_TIME_FRAC_PART) / conf->frmTimeDiv;

	DxOISP_CommandSet(
            DxOISP_CMD_OFFSET(stAsync.stSystem.usMinInterframeTime),
            sizeof(frmTime), &frmTime);

	dxo->dxoCmd.stSync.stControl.eMode = DxOISP_MODE_IDLE;
	dxo->dxoCmd.stSync.stControl.ucCalibrationSelection = 0;
	dxo->dxoCmd.stSync.stControl.ucSourceSelection = conf->sensorId;
	dxo->dxoCmd.stSync.stCameraControl.stAfk.eMode = DxOISP_AFK_MODE_MANUAL;
	dxo->dxoCmd.stSync.stCameraControl.stAe.eMode = DxOISP_AE_MODE_MANUAL;
	dxo->dxoCmd.stSync.stCameraControl.stAwb.eMode = DxOISP_AWB_MODE_MANUAL;
	dxo->dxoCmd.stSync.stCameraControl.stFlash.eMode = DxOISP_FLASH_MODE_OFF;
	dxo->dxoCmd.stSync.stCameraControl.stAf.eMode = DxOISP_AF_MODE_MANUAL;

	DxOISP_CommandGroupOpen();
	DxOISP_CommandSetAuto(
            dxo->dxoCmd, stSync.stControl.ucSourceSelection);
	DxOISP_CommandSetAuto(
            dxo->dxoCmd, stSync.stControl.ucCalibrationSelection);
	DxOISP_CommandSetAuto(
            dxo->dxoCmd, stSync.stControl.eMode);
	DxOISP_CommandSetAuto(
            dxo->dxoCmd, stSync.stCameraControl);
	DxOISP_CommandGroupClose();
	
	do {
		DxOISP_StatusGroupOpen() ;
		DxOISP_StatusGet(
                DxOISP_STATUS_OFFSET(stSync.stSystem.eMode),
                sizeof(mode), &mode);
		DxOISP_StatusGroupClose();
	} while(mode != DxOISP_MODE_IDLE);

    DBG("DxO entered idle state.");

    return dxo;
}

void DXOExit(struct DXO *dxo)
{
    typeof(dxo->dxoStatus.stSync.stSystem.eMode) mode;

    ASSERT(dxo);

	dxo->dxoCmd.stSync.stControl.eMode = DxOISP_MODE_IDLE;
	dxo->dxoCmd.stSync.stCameraControl.stAe.eMode = DxOISP_AE_MODE_MANUAL;

	DxOISP_CommandGroupOpen();
	DxOISP_CommandSetAuto(dxo->dxoCmd, stSync.stControl.eMode);
	DxOISP_CommandGroupClose();
	
	do {
		DxOISP_StatusGroupOpen() ;
		DxOISP_StatusGet(
                DxOISP_STATUS_OFFSET(stSync.stSystem.eMode),
                sizeof(mode), &mode);
		DxOISP_StatusGroupClose();
	} while(mode != DxOISP_MODE_IDLE);

    stopIRQHandler(dxo);

    DXOBase = NULL;
    DXOFd = -1;
    closeIODevice(dxo->io);

    free(dxo);
}

void DXOSetControl(struct DXO *dxo, const struct DXOControl *ctrl)
{
    ASSERT(dxo);
    ASSERT(ctrl);

    //dxo->dxoCmd.stSync.stControl.ucSourceSelection = ctrl->input;
    dxo->dxoCmd.stSync.stControl.eImageOrientation =
        (ctrl->vFlip ? 2 : 0) + (ctrl->hMirror ? 1 : 0);
    dxo->dxoCmd.stSync.stControl.isTNREnabled = ctrl->enableTNR ? 1 : 0;
    dxo->dxoCmd.stSync.stControl.usFrameRate =
        (ctrl->fpsMul << DxOISP_FPS_FRAC_PART) / ctrl->fpsDiv;
    // TODO Need to change for dynamic setup
    dxo->dxoCmd.stSync.stControl.ucInputDecimation = 1;
    dxo->dxoCmd.stSync.stControl.ucCalibrationSelection = 0;
    dxo->dxoCmd.stSync.stControl.ucNbCaptureFrame = 1;
    dxo->dxoCmd.stSync.stControl.usZoomFocal = 1 << DxOISP_ZOOMFOCAL_FRAC_PART;
    dxo->dxoCmd.stSync.stControl.usFrameToFrameLimit = 0xffff;
    dxo->configFlags |= DXO_CONFIG_FLAG_CONTROL;
}

int DXOSetOutputFormat(struct DXO *dxo,
        enum DXOOutput output, const struct DXOOutputFormat *fmt)
{
    struct DXOPixelFormat *pixelFormat;
    struct DXOPortInfo *port = &dxoPorts[output];
    typeof(dxo->dxoCmd.stSync.stControl.stOutput[0]) *outFmt;

    ASSERT(dxo);
    ASSERT(fmt);

    outFmt = &dxo->dxoCmd.stSync.stControl.stOutput[output];
    if (fmt) {
        if (fmt->width < port->hMin ||
                fmt->width > port->hMax ||
                fmt->height < port->vMin ||
                fmt->height > port->vMax) {
            ERR("Port[%s] size limit exceeded.", port->name);
            return -1;
        }
        pixelFormat = getPixelFormat(fmt->pixelFormat);
        ASSERT(pixelFormat);
        outFmt->eFormat = pixelFormat->dxoFormat;
        outFmt->eOrder = pixelFormat->dxoOrder;
        outFmt->eEncoding = pixelFormat->dxoEncoding;
        outFmt->stCrop.usXAddrStart = fmt->crop.left;
        outFmt->stCrop.usXAddrEnd = fmt->crop.right;
        outFmt->stCrop.usYAddrStart = fmt->crop.top;
        outFmt->stCrop.usYAddrEnd = fmt->crop.bottom;
        DBG("Crop(XS:%u, XE:%u, YS:%u, YE:%u)", 
                outFmt->stCrop.usXAddrStart,
                outFmt->stCrop.usXAddrEnd,
                outFmt->stCrop.usYAddrStart,
                outFmt->stCrop.usYAddrEnd);
        outFmt->usSizeX = fmt->width;
        outFmt->usSizeY = fmt->height;
    }

    dxo->configFlags |= DXO_CONFIG_FLAG_OUTPUT(output);

    DBG("Port[%s] formated to (%dx%d)",
            port->name, outFmt->usSizeX, outFmt->usSizeY);

    return 0;
}

void DXOSetOutputEnable(struct DXO *dxo, int mask, int enable)
{
    int i;
    typeof(dxo->dxoCmd.stSync.stControl.stOutput[0]) *outFmt;

    ASSERT(dxo);

    for (i = 0; i < DXO_OUTPUT_COUNT; i++) {
        if (mask & (1 << i)) {
            outFmt = &dxo->dxoCmd.stSync.stControl.stOutput[i];
            if (enable & (1 << i)) {
                outFmt->isEnabled = 1;
                dxo->enabledOutput |= (1 << i);
            }
            else {
                outFmt->isEnabled = 0;
                dxo->enabledOutput &= ~(1 << i);
            }
        }
    }
}

void DXORunState(struct DXO *dxo, enum DXOState state, int captureCount)
{
    typeof(dxo->dxoStatus.stSync.stSystem.eMode) mode;

    ASSERT(dxo);

    DBG("***** Changing DxO State From %s To %s",
            state2String[dxo->dxoCmd.stSync.stControl.eMode],
            state2String[state]);

    dxo->dxoCmd.stSync.stControl.eMode = state;
    if (state == DXO_STATE_CAPTURE_A || state == DXO_STATE_CAPTURE_B)
        dxo->dxoCmd.stSync.stControl.ucNbCaptureFrame = captureCount;
    else
        dxo->dxoCmd.stSync.stControl.ucNbCaptureFrame = 1;

    DxOISP_CommandGroupOpen() ;
	DxOISP_CommandSetAuto(dxo->dxoCmd, stSync.stControl);
    DxOISP_CommandSetAuto(dxo->dxoCmd, stSync.stCameraControl);
    DxOISP_CommandGroupClose() ;

	do {
		DxOISP_StatusGroupOpen() ;
		DxOISP_StatusGet(
                DxOISP_STATUS_OFFSET(stSync.stSystem.eMode),
                sizeof(mode), &mode);
		DxOISP_StatusGroupClose();
	} while(mode != state);

    DBG("***** DXO State Changed To %s", state2String[state]);
}

enum DXOState DXOGetState(struct DXO *dxo)
{
    typeof(dxo->dxoStatus.stSync.stSystem.eMode) mode;

    DxOISP_StatusGroupOpen() ;
    DxOISP_StatusGet(
            DxOISP_STATUS_OFFSET(stSync.stSystem.eMode),
            sizeof(mode), &mode);
    DxOISP_StatusGroupClose();

    return (enum DXOState)mode;
}

const struct DXOPixelFormat *DXOGetPixelFormatByName(const char *name)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(dxoPixelFormats); i++) {
        if (strcmp(dxoPixelFormats[i].name, name) == 0)
            return &dxoPixelFormats[i];
    }

    return NULL;
}
