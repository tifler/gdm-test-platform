#ifndef __GISP_MEDIA_H__
#define __GISP_MEDIA_H__

/*****************************************************************************/

#define MAX_NAME_LENGTH                     (64)

/*****************************************************************************/

enum MediaDeviceId {
    MEDIA_DEVICE_SENSOR_FRONT,
    MEDIA_DEVICE_SENSOR_REAR,
    MEDIA_DEVICE_CSIPHY,
    MEDIA_DEVICE_SIF,
    MEDIA_DEVICE_DXO,
    MEDIA_DEVICE_WDMA_CAPTURE,
    MEDIA_DEVICE_WDMA_VIDEO,
    MEDIA_DEVICE_WDMA_DISPLAY,
    MEDIA_DEVICE_WDMA_FACEDETECT,
    MEDIA_DEVICE_VSENSOR,
    MEDIA_DEVICE_BT,
    MEDIA_DEVICE_COUNT
};

enum MediaDeviceType {
    MEDIA_DEVICE_TYPE_SUBDEV,
    MEDIA_DEVICE_TYPE_VIDEODEV,
};

/*****************************************************************************/

struct MEDIA_DEVICE;

struct MediaDeviceInfo {
    enum MediaDeviceId id;
    enum MediaDeviceType type;
    char devNode[MAX_NAME_LENGTH];
};

/*****************************************************************************/

struct MEDIA_DEVICE *mediaInit(void);
void mediaExit(struct MEDIA_DEVICE *media);
int mediaGetInfo(struct MEDIA_DEVICE *media,
        enum MediaDeviceId id, struct MediaDeviceInfo *info);

#endif  /*__GISP_MEDIA_H__*/
