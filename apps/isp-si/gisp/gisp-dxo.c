#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <gisp-ioctl.h>
#include <linux/videodev2.h>

#include "gisp-dxo.h"
#include "gisp-iodev.h"
#include "DxOISP.h"
#include "DxOISP_ahblAccess.h"
#include "debug.h"

/*****************************************************************************/

#define DEFAULT_IRQ_TIMEOUT             (1000)
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

/*****************************************************************************/

struct DXOPortInfo {
    const char *name;
    uint32_t hMin;
    uint32_t hMax;
    uint32_t vMin;
    uint32_t vMax;
};

struct DXOPixelFormats {
    const char *name;
    uint32_t v4l2PixelFormat;
    int format;
    int order;
    int encoding;
};

struct DXO {
    struct IODevice *io;

    int stopIRQThread;
    pthread_t irqThread;                // 

    unsigned long configFlags;
    unsigned long enabledOutput;
    ts_DxOIspCmd dxoCmd;
    ts_DxOIspStatus dxoStatus;

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

struct DXOPixelFormats dxoPixelFormats[] = {
    // 422_1P
    {
        .name = "YUV 4:2:2 packed, CbYCrY",
        .v4l2PixelFormat = V4L2_PIX_FMT_UYVY,
        .format = DxOISP_FORMAT_YUV422,
        .order = DxOISP_YUV422ORDER_UYVY,
        .encoding = DxOISP_ENCODING_YUV_601FU,
    }, {
        .name = "YUV 4:2:2 packed, CrYCbY",
        .v4l2PixelFormat = V4L2_PIX_FMT_VYUY,
        .format = DxOISP_FORMAT_YUV422,
        .order = DxOISP_YUV422ORDER_VYUY,
        .encoding = DxOISP_ENCODING_YUV_601FU,
    }, {
        .name = "YUV 4:2:2 packed, YCbYCr",
        .v4l2PixelFormat = V4L2_PIX_FMT_YUYV,
        .format = DxOISP_FORMAT_YUV422,
        .order = DxOISP_YUV422ORDER_YUYV,
        .encoding = DxOISP_ENCODING_YUV_601FU,
    }, {
        .name = "YUV 4:2:2 packed, YCrYCb",
        .v4l2PixelFormat = V4L2_PIX_FMT_YVYU,
        .format = DxOISP_FORMAT_YUV422,
        .order = DxOISP_YUV422ORDER_YVYU,
        .encoding = DxOISP_ENCODING_YUV_601FU,
    },
    // 422_2P
    {
        .name = "YUV 4:2:2 planar, Y/CbCr",
        .v4l2PixelFormat = V4L2_PIX_FMT_NV16,
        .format = DxOISP_FORMAT_YUV422,
        .order = DxOISP_YUV422ORDER_YYVU,
        .encoding = DxOISP_ENCODING_YUV_601FU,
    }, {
        .name = "YUV 4:2:2 planar, Y/CrCb",
        .v4l2PixelFormat = V4L2_PIX_FMT_NV61,
        .format = DxOISP_FORMAT_YUV422,
        .order = DxOISP_YUV422ORDER_YYUV,
        .encoding = DxOISP_ENCODING_YUV_601FU,
    },
    // FIXME: How to support 422P3 in DxO ?
#if 0
    // 422_3P
    {
        .name = "YUV 4:2:2 3-planar, Y/Cb/Cr",
        .v4l2PixelFormat = V4L2_PIX_FMT_YUV422P,
        .format = DxOISP_FORMAT_YUV422,
        .order = DxOISP_YUV422ORDER_YYUV,
        .encoding = DxOISP_ENCODING_YUV_601FU,
    },
    // 420_2P
    // 주의: NV12와 NV12M의 차이는 2Plane이 연속이냐 아니냐의 차이이다.
    {
        // NV12의 경우 plane을 두개로 하지만 실제로는 하나로 할당 받도록 처리
        .name = "YUV 4:2:0 contiguous 2-planar, Y/CbCr",
        .v4l2PixelFormat = V4L2_PIX_FMT_NV12,
        .format = DxOISP_FORMAT_YUV420,
        .order = DxOISP_YUV422ORDER_YYUV,
        .encoding = DxOISP_ENCODING_YUV_601FU,
    },
    {
        .name = "YUV 4:2:0 non-contiguous 2-planar, Y/CbCr",
        .v4l2PixelFormat = V4L2_PIX_FMT_NV12M,
    },
    {
        .name = "YUV 4:2:0 contiguous 2-planar, Y/CrCb",
        .v4l2PixelFormat = V4L2_PIX_FMT_NV21,
    },
    {
        .name = "YUV 4:2:0 non-contiguous 2-planar, Y/CrCb",
        .v4l2PixelFormat = V4L2_PIX_FMT_NV21M,
    },
    // 420_3P
    {
        .name = "YUV 4:2:0 non-contiguous 3-planar, Y/Cb/Cr",
        .v4l2PixelFormat = V4L2_PIX_FMT_YUV420M,
    },
#endif  /*0*/
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
    DxOISP_DumpStatus();
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

    do {
        userRet = 0;

        ret = waitIRQ(dxo, DEFAULT_IRQ_TIMEOUT);
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

            DxOISP_Event();

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
        }

        // If you turn on next statement, you will catch floating point
        // exception.(tifler:2014.09.03)
        //DxOISP_PostEvent();

        // user callback
        if (dxo->irqCallback)
            userRet = dxo->irqCallback(dxo->irqCallbackParam);

    } while (!dxo->stopIRQThread && userRet == 0);

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

static int getFormat(uint32_t pixelFormat)
{
    // TODO
    return DxOISP_FORMAT_YUV422;
}

static int getOrder(uint32_t pixelFormat)
{
    // TODO
    return DxOISP_YUV422ORDER_YUYV;
}

static int getEncoding(uint32_t pixelFormat)
{
    // TODO
    return DxOISP_ENCODING_YUV_601FU;
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
	dxo->dxoCmd.stSync.stControl.ucSourceSelection = 0;
	dxo->dxoCmd.stSync.stCameraControl.stAe.eMode = DxOISP_AE_MODE_MANUAL;

	DxOISP_CommandGroupOpen();
	DxOISP_CommandSetAuto(
            dxo->dxoCmd, stSync.stControl.ucSourceSelection);
	DxOISP_CommandSetAuto(
            dxo->dxoCmd, stSync.stControl.ucCalibrationSelection);
	DxOISP_CommandSetAuto(
            dxo->dxoCmd, stSync.stControl.eMode);
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

    dxo->dxoCmd.stSync.stControl.ucSourceSelection = ctrl->input;
    dxo->dxoCmd.stSync.stControl.eImageOrientation =
        (ctrl->vFlip ? 2 : 0) + (ctrl->hMirror ? 1 : 0);
    dxo->dxoCmd.stSync.stControl.isTNREnabled = ctrl->enableTNR ? 1 : 0;
    dxo->dxoCmd.stSync.stControl.usFrameRate =
        (ctrl->fpsMul << DxOISP_FPS_FRAC_PART) / ctrl->fpsDiv;
    // FIXME Change to dynamic setup
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
        outFmt->eFormat = getFormat(fmt->pixelFormat);
        outFmt->eOrder = getOrder(fmt->pixelFormat);
        outFmt->eEncoding = getEncoding(fmt->pixelFormat);
        outFmt->stCrop.usXAddrStart = fmt->crop.left;
        outFmt->stCrop.usXAddrEnd = fmt->crop.right;
        outFmt->stCrop.usYAddrStart = fmt->crop.top;
        outFmt->stCrop.usYAddrEnd = fmt->crop.bottom;
        outFmt->usSizeX = fmt->width;
        outFmt->usSizeY = fmt->height;
    }
    outFmt->isEnabled = fmt ? 1 : 0;

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
}
