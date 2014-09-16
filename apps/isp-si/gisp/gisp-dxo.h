#ifndef __GISP_DXO_H__
#define __GISP_DXO_H__

#include <stdint.h>

/*****************************************************************************/

enum DXOInputSource {
    DXO_INPUT_SOURCE_BACK,
    DXO_INPUT_SOURCE_FRONT,
    DXO_INPUT_SOURCE_VSENSOR,
    DXO_INPUT_SOURCE_COUNT
};

enum DXOState {
    DXO_STATE_STANDBY,
    DXO_STATE_IDLE,
    DXO_STATE_PREVIEW,
    DXO_STATE_CAPTURE_A,
    DXO_STATE_CAPTURE_B,
    DXO_STATE_VIDEOREC,
    DXO_STATE_COUNT
};

enum DXOOutput {
    DXO_OUTPUT_DISPLAY,
    DXO_OUTPUT_VIDEO,
    DXO_OUTPUT_CAPTURE,
    DXO_OUTPUT_FD,
    DXO_OUTPUT_COUNT
};

/*****************************************************************************/

struct DXOSystemConfig {
    uint32_t sysFreqMul;
    uint32_t sysFreqDiv;
    uint32_t frmTimeMul;
    uint32_t frmTimeDiv;
    int needPostEvent;
    int estimateIRQ;

    // callback functions & params
    int (*printf)(const char *, ...);
    int (*pixelStuckHandler)(void);
    int (*irqCallback)(void *param);
    void *irqCallbackParam;
};

struct DXOControl {
    enum DXOInputSource input;
    int hMirror;
    int vFlip;
    int fpsMul;
    int fpsDiv;
    int enableTNR;
};

struct DXOCrop {
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
};

struct DXOOutputFormat {
    uint32_t width;
    uint32_t height;
    struct DXOCrop crop;
    uint32_t pixelFormat;       // V4L2_PIX_FMT_*
};

struct DXOPixelFormat {
    const char *name;
    const char *description;
    uint32_t v4l2PixelFormat;
    int dxoFormat;
    int dxoOrder;
    int dxoEncoding;
};

struct DXO;

/*****************************************************************************/

struct DXO *DXOInit(const struct DXOSystemConfig *conf);
void DXOExit(struct DXO *dxo);
void DXOSetControl(struct DXO *dxo, const struct DXOControl *ctrl);
void DXOSetOutputEnable(struct DXO *dxo, int mask, int enable);
int DXOSetOutputFormat(struct DXO *dxo,
        enum DXOOutput output, const struct DXOOutputFormat *fmt);
void DXORunState(struct DXO *dxo, enum DXOState state, int captureCount);
const struct DXOPixelFormat *DXOGetPixelFormatByName(const char *name);

#endif  /*__GISP_DXO_H__*/
