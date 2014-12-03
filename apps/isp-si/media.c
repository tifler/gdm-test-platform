#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include "media.h"
#include "debug.h"

/*****************************************************************************/

#define V4L_DIR_PATH            "/sys/devices/platform/gdm-isp/video4linux"
#define ARRAY_SIZE(a)           (sizeof(a) / sizeof(a[0]))

/*****************************************************************************/

struct MEDIA_DEVICE {
    struct MediaDeviceInfo devices[MEDIA_DEVICE_COUNT];
    unsigned int exists;
};

static const char *mediaName[] = {
    "GISP SENSOR FRONT",
    "GISP SENSOR REAR",
    "GISP CSIPHY",
    "GISP SIF",
    "GISP DXO",
    "GISP WDMA CAPTURE",
    "GISP WDMA VIDEO",
    "GISP WDMA DISPLAY",
    "GISP WDMA FACEDETECT",
    "GISP VSENSOR",
    "GISP BT",
};

/*****************************************************************************/

static void readNameFile(const char *path, char *buffer, int length)
{
    int fd;
    ssize_t size;

    memset(buffer, 0, length);
    fd = open(path, O_RDONLY);
    ASSERT(fd > 0);
    size = read(fd, buffer, length);
    ASSERT(size > 0);
    // truncate newline
    buffer[size - 1] = 0;
    close(fd);
}

static int lookupMediaName(const char *name)
{
    int id;

    for (id = 0; id < ARRAY_SIZE(mediaName); id++) {
        //DBG("mediaName[%d]=[%s], name=[%s]", id, mediaName[id], name);
        if(strcmp(mediaName[id], name) == 0)
            return id;
    }

    DBG("Not found %s", name);
    return -1;
}

/*****************************************************************************/

struct MEDIA_DEVICE *mediaInit(void)
{
    int id;
    DIR *root;
    char name[MAX_NAME_LENGTH];
    char path[256];
    struct dirent *entry;
    struct MEDIA_DEVICE *media;

    media = calloc(1, sizeof(*media));
    ASSERT(media);

    root = opendir(V4L_DIR_PATH);
    ASSERT(root);

    while ((entry = readdir(root)) != NULL) {
        if (entry->d_name[0] != 'v')
            continue;
        snprintf(path, sizeof(path) - 1, 
                V4L_DIR_PATH "/%s/name", entry->d_name);
        //DBG("Path=%s", path);
        readNameFile(path, name, sizeof(name));
        //DBG("%s: %s", path, name);

        id = lookupMediaName(name);
        if (id < 0)
            continue;

        // not allowd duplication
        ASSERT(!(media->exists & (1 << id)));

        media->devices[id].id = id;
        if (strstr(entry->d_name, "subdev") != NULL)
            media->devices[id].type = MEDIA_DEVICE_TYPE_SUBDEV;
        else
            media->devices[id].type = MEDIA_DEVICE_TYPE_VIDEODEV;
        strcpy(media->devices[id].devNode, entry->d_name);
        media->exists |= (1 << id);

        DBG("[%s:%d] type=%d node=%s", name, id,
                media->devices[id].type, media->devices[id].devNode);
    }

    closedir(root);

    return media;
}

void mediaExit(struct MEDIA_DEVICE *media)
{
    ASSERT(media);
    free(media);
}

int mediaGetInfo(struct MEDIA_DEVICE *media,
        enum MediaDeviceId id, struct MediaDeviceInfo *info)
{
    ASSERT(media);

    if (!(media->exists & (1 << id))) {
        DBG("Not exist entry.(%s:%d)", mediaName[id], id);
        return -ENOENT;
    }

    memcpy(info, &media->devices[id], sizeof(struct MediaDeviceInfo));
    return 0;
}
