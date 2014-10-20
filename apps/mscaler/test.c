#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "mscaler.h"
#include "iniparser.h"
#include "ion_api.h"
#include "debug.h"

/******************************************************************************
 *
 * 기능 테스트 시나리오
 * 
 * 1. 각 포맷/크기별 소스 이미지 준비 
 * 2. 1번의 이미지들과 대응하는 정보 테이블(크기/포맷) 생성 
 * 3. 2번의 각 엔트리 별로 Crop/Scale을 달리한 정보 테이블 생성 
 * 4. 각 조합으로 테스트 수행
 * 
 * # FPGA상에 이미지와 정보데이터를 어떻게 올릴 것인가?
 *
 *
 * 성능 테스트 시나리오
 *
 * (TBD)
 *
 *****************************************************************************/

#define ION_DEVICE                      "/dev/ion"
#define ARRAY_SIZE(a)                   (sizeof(a) / sizeof(a[0]))
#define MAKE_COLOR_FORMAT(fmt)  \
    .name = #fmt, .pixelformat = fmt

#define TC_NAME_LENGTH                  (256)

/*****************************************************************************/

// 사용할 소스 이미지 정보
struct TCImageFormat {
    char *path;
    struct MScalerImageFormat fmt;
};

struct TCEntry {
    char *name;
    int disabled;
    int storeEachPlane;
    int dontSend;
    int dontRemove;
    int randomCount;
    int loopCount;
    struct TCImageFormat src;
    struct TCImageFormat dst;
};

/*****************************************************************************/

static MScalerHandle *hScaler;
static int ionFd = -1;

static struct mscaler_scaler_coeff scaler_coeff = {
    .apply = 0x00,
    .yh = {
        0,    0,    0,    0,    0, 1024,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    6,  -24, 1020,   28,   -6,    0,    0,    0,    0,
        0,    0,    0,   12,  -47, 1015,   57,  -13,    0,    0,    0,    0,
        0,    0,    0,   17,  -68, 1007,   89,  -22,    1,    0,    0,    0,
        0,    0,    0,   21,  -86,  994,  123,  -30,    2,    0,    0,    0,
        0,    0,    0,   24, -102,  979,  159,  -39,    3,    0,    0,    0,
        0,    0,    0,   27, -115,  960,  197,  -49,    4,    0,    0,    0,
        0,    0,    0,   29, -126,  938,  236,  -59,    6,    0,    0,    0,
        0,    0,    0,   31, -135,  911,  277,  -68,    8,    0,    0,    0,
        0,    0,    0,   32, -142,  884,  319,  -78,    9,    0,    0,    0,
        0,    0,    0,   32, -147,  853,  362,  -88,   12,    0,    0,    0,
        0,    0,    0,   32, -149,  820,  405,  -98,   14,    0,    0,    0,
        0,    0,    0,   31, -150,  785,  449, -107,   16,    0,    0,    0,
        0,    0,    0,   30, -149,  747,  494, -116,   18,    0,    0,    0,
        0,    0,    0,   29, -147,  707,  538, -124,   21,    0,    0,    0,
        0,    0,    0,   27, -143,  667,  581, -131,   23,    0,    0,    0,
        0,    0,    0,   25, -138,  625,  625, -138,   25,    0,    0,    0
    },
    .yv = {
         -111,  623,  623, -111,    0,
         -104,  577,  665, -116,    2,
          -97,  532,  707, -121,    3,
          -89,  485,  747, -124,    5,
          -81,  440,  784, -126,    7,
          -73,  395,  821, -127,    8,
          -64,  350,  853, -125,   10,
          -56,  307,  884, -122,   11,
          -48,  265,  913, -117,   11,
          -40,  225,  937, -110,   12,
          -32,  186,  959, -101,   12,
          -25,  150,  978,  -90,   11,
          -19,  115,  994,  -76,   10,
          -13,   83, 1007,  -61,    8,
           -8,   53, 1016,  -43,    6,
           -3,   25, 1021,  -22,    3,
            0,    0, 1024,    0,    0
    },
    .ch = {
        0,    0,    0, 1024,    0,    0,    0,    0,
        0,    6,  -24, 1020,   28,   -6,    0,    0,
        0,   12,  -47, 1015,   57,  -13,    0,    0,
        0,   17,  -68, 1007,   89,  -22,    1,    0,
        0,   21,  -86,  994,  123,  -30,    2,    0,
        0,   24, -102,  979,  159,  -39,    3,    0,
        0,   27, -115,  960,  197,  -49,    4,    0,
        0,   29, -126,  938,  236,  -59,    6,    0,
        0,   31, -135,  911,  277,  -68,    8,    0,
        0,   32, -142,  884,  319,  -78,    9,    0,
        0,   32, -147,  853,  362,  -88,   12,    0,
        0,   32, -149,  820,  405,  -98,   14,    0,
        0,   31, -150,  785,  449, -107,   16,    0,
        0,   30, -149,  747,  494, -116,   18,    0,
        0,   29, -147,  707,  538, -124,   21,    0,
        0,   27, -143,  667,  581, -131,   23,    0,
        0,   25, -138,  625,  625, -138,   25,    0
    },

    .cv = {
         -111,  623,  623, -111,    0,
         -104,  577,  665, -116,    2,
          -97,  532,  707, -121,    3,
          -89,  485,  747, -124,    5,
          -81,  440,  784, -126,    7,
          -73,  395,  821, -127,    8,
          -64,  350,  853, -125,   10,
          -56,  307,  884, -122,   11,
          -48,  265,  913, -117,   11,
          -40,  225,  937, -110,   12,
          -32,  186,  959, -101,   12,
          -25,  150,  978,  -90,   11,
          -19,  115,  994,  -76,   10,
          -13,   83, 1007,  -61,    8,
           -8,   53, 1016,  -43,    6,
           -3,   25, 1021,  -22,    3,
            0,    0, 1024,    0,    0
    },
};

static struct mscaler_rgb_coeff rgb_coeff = {
    .use_user_coeff = 1,
    .pre_offset1 = 16,
    .pre_offset2 = 128,
    .post_offset = 16,
    .coeff0_y = 76284,
    .coeff0_v = 104595,
    .coeff1_u = 25625,
    .coeff1_v = 53281,
    .coeff2_u = 132252,
};

/*****************************************************************************/

static unsigned int str2PixelFormat(const char *name)
{
    struct MScalerPixelFormat *fmt;
    fmt = MScalerGetPixelFormatByName(name);
    ASSERT(fmt);
    return fmt->pixelformat;
}

static const char *pixelFormat2String(unsigned int pixelformat)
{
    struct MScalerPixelFormat *fmt;
    fmt = MScalerGetPixelFormat(pixelformat);
    ASSERT(fmt);
    return fmt->name;
}

static int isYUV(unsigned int pixelformat)
{
    switch (pixelformat) {
        case V4L2_PIX_FMT_RGB32:
        case V4L2_PIX_FMT_BGR32:
        case V4L2_PIX_FMT_RGB565:
            return 0;

        default:
            break;
    }

    return 1;
}

struct IonHelper {
    int fd;
    unsigned int flags;
    unsigned int heapMask;
    struct ion_handle *handle[3];
};

static int allocContigMemory(
        struct MScalerImageData *data, const struct MScalerBufferInfo *bufInfo)
{
    int i;
    int mapFd;
    int ret = 0;
    struct IonHelper *ion;

    ion = calloc(1, sizeof(*ion));
    ASSERT(ion);

    ion->flags = 0;
    //ion->heapMask = (1 << ION_DISPLAY_CARVEOUT);
    ion->heapMask = (1 << ION_HEAP_TYPE_CARVEOUT);
    ion->fd = ion_open();
    ASSERT(ion->fd > 0);

    data->planes = bufInfo->planes;

    // allocate memory from ion device for each planes
    for (i = 0; i < data->planes; i++) {
        DBG("plane[%d] Size = %d", i, bufInfo->planeSizes[i]);
        ret = ion_alloc(ion->fd, bufInfo->planeSizes[i],
                0, ion->heapMask, ion->flags, &ion->handle[i]);
        if (ret) {
            ERR("ion_alloc() failed.");
            break;
        }

        // 여기서 맵핑된 메모리 공간에 소스 이미지를 로딩한다.
        ret = ion_map(ion->fd, ion->handle[i], bufInfo->planeSizes[i],
                PROT_READ | PROT_WRITE, MAP_SHARED, 0,
                (unsigned char **)&data->plane[i].base, &mapFd);
        if (ret) {
            ERR("ion_map() failed.");
            break;
        }

        // 다른 process와 share를 하지 않고 이 프로세스에서만 사용되므로
        // mapFd를 사용한다.
#if 0
        ret = ion_share(ionFd, ionHandle, &data->plane[i].fd);
        if (ret) {
            ERR("ion_share() failed.");
            break;
        }
#else
        data->plane[i].fd = mapFd;
#endif  /*0*/

        data->plane[i].length = bufInfo->planeSizes[i];
    }

    if (ret) {
        for (--i ; i >= 0; i--) {
            close(data->plane[i].fd);
            data->plane[i].fd = 0;
            ion_free(ion->fd, ion->handle[i]);
        }
        ion_close(ion->fd);
        free(ion);
    }
    else {
        data->priv = ion;
    }

    return ret;
}

static int freeContigMemory(struct MScalerImageData *data)
{
    int i;
    struct IonHelper *ion = data->priv;

    DBG("freeContigMemory");

    ASSERT(ion);

    for (i = 0; i < data->planes; i++) {
        munmap(data->plane[i].base, data->plane[i].length);
        close(data->plane[i].fd);
        ion_free(ion->fd, ion->handle[i]);
        data->plane[i].base = NULL;
        data->plane[i].length = 0;
        data->plane[i].fd = -1;
    }

    ion_close(ion->fd);
    free(ion);
    data->priv = NULL;

    return 0;
}

static int safeRead(int fd, void *buffer, size_t size)
{
    int ret;
    char *ptr;
    size_t nread;

    ASSERT(fd > 0);

    nread = 0;
    ptr = (char *)buffer;

    while (nread < size) {
        ret = read(fd, &ptr[nread], size - nread);
        if (ret < 0) {
            switch (errno) {
                case EINTR:
                case EAGAIN:
                    continue;

                default:
                    DIE("read failed.");
                    break;
            }
        }
        else if (ret == 0)
            break;

        nread += ret;
    }

    return nread;
}

static int safeWrite(int fd, void *buffer, size_t size)
{
    int ret;
    char *ptr;
    size_t nwrite;

    ASSERT(fd > 0);

    nwrite = 0;
    ptr = (char *)buffer;

    while (nwrite < size) {
        ret = write(fd, &ptr[nwrite], size - nwrite);
        if (ret < 0) {
            switch (errno) {
                case EINTR:
                case EAGAIN:
                    continue;

                default:
                    DIE("write failed.");
                    break;
            }
        }
        else if (ret == 0)
            break;

        nwrite += ret;
    }

    return nwrite;
}

static void readImagePlane(int fd,
        void *bufptr, struct TCImageFormat *img, int plane)
{
    int i;
    int ret = 0;
    int line;
    int nread = 0;
    struct MScalerPlaneInfo pi;
    char *ptr = (char *)bufptr;

    MScalerGetPlaneInfo(&img->fmt, plane, &pi);

    // 각 컴포넌트별로 읽어야 한다.
    for (i = 0; i < pi.comps; i++) {
        for (line = 0; line < pi.height[i]; line++) {
            ret = safeRead(fd, ptr, pi.bpl[i]);
            ASSERT(ret == pi.bpl[i]);

            nread += ret;
            ptr += pi.stride[i];
        }
    }

    DBG("Read Size = %d", nread);
}

static void writeImagePlane(int fd,
        void *bufptr, struct TCImageFormat *img, int plane)
{
    int i;
    int ret = 0;
    int line;
    int nwrite = 0;
    struct MScalerPlaneInfo pi;
    char *ptr = (char *)bufptr;

    ASSERT(fd > 0);
    ASSERT(bufptr);

    MScalerGetPlaneInfo(&img->fmt, plane, &pi);

    for (i = 0; i < pi.comps; i++) {
        for (line = 0; line < pi.height[i]; line++) {
            ret = safeWrite(fd, ptr, pi.bpl[i]);
            ASSERT(ret == pi.bpl[i]);

            nwrite += ret;
            ptr += pi.stride[i];
        }
    }

    DBG("Write Size = %d", nwrite);
}

// SRC 이미지 로딩시에도 Stride에 맞게 로딩해야 한다.
// 즉 Width 가 798일경우 Stride는 800이므로(420-3P경우) 2Byte 패딩이 있어야
// 한다.
static int loadImage(struct MScalerImageData *data, struct TCImageFormat *img)
{
    int i;
    int fd;
    int ret;
    struct stat st;
    unsigned int bufSize;

    ASSERT(data->planes > 0);

    bufSize = 0;

    for (i = 0; i < data->planes; i++)
        bufSize += data->plane[i].length;

    ret = stat(img->path, &st);
    if (ret)
        DIE("%s: stat() failed.", strerror(errno));

    if (st.st_size > bufSize)
        DIE("Image size is too big.(expect=%u, size=%llu)",
                bufSize, (unsigned long long)st.st_size);

    fd = open(img->path, O_RDONLY);
    if (fd < 0) {
        DIE("%s: open failed.", img->path);
    }

    // fill image data to each plane buffer
    for (i = 0; i < data->planes; i++)
        readImagePlane(fd, data->plane[i].base, img, i);

    close(fd);

    return 0;
}

static void getDstPlanePath(const struct TCEntry *tc,
        char *buffer, int bufsize, int plane)
{
    snprintf(buffer, bufsize - 1, "%s-Plane-%d.%s",
            tc->dst.path, plane, isYUV(tc->dst.fmt.pixelformat) ? "yuv" : "rgb");
}

static void getDstPath(char *buffer, int size, const struct TCEntry *tc)
{
    int stride;
    struct MScalerPlaneInfo pi;

    ASSERT(buffer);
    ASSERT(tc);
    ASSERT(size > 0);

    MScalerGetPlaneInfo(&tc->dst.fmt, 0, &pi);

    if (pi.stride[0] >= tc->dst.fmt.width * 2)
        stride = tc->dst.fmt.width;
    else
        stride = pi.stride[0];

    snprintf(buffer, size - 1, "%s.%s.%dx%d.%s",
            basename(tc->src.path), 
            pixelFormat2String(tc->dst.fmt.pixelformat),
            stride,
            //pi.stride[0],
            tc->dst.fmt.height,
            (isYUV(tc->dst.fmt.pixelformat) ? "yuv" : "rgb"));
}

static void storeImage(struct TCEntry *tc, struct MScalerImageData *data)
{
    int i;
    char path[256];
    int fd[3] = { 0, 0, 0 };

    if (!tc->dst.path) {
        tc->dst.path = calloc(1, TC_NAME_LENGTH);
        ASSERT(tc->dst.path);
        getDstPath(tc->dst.path, TC_NAME_LENGTH, tc);
    }

    if (tc->storeEachPlane) {
        // Save each planes
        for (i = 0; i < data->planes; i++) {
            getDstPlanePath(tc, path, sizeof(path), i);
            fd[i] = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
            ASSERT(fd[i] > 0);

            writeImagePlane(fd[i], data->plane[i].base, &tc->dst, i);

            close(fd[i]);
        }
    }

    // Composition file save
    fd[0] = open(tc->dst.path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    ASSERT(fd[0] > 0);

    for (i = 0; i < data->planes; i++) {
        writeImagePlane(fd[0], data->plane[i].base, &tc->dst, i);
    }

    close(fd[0]);
}

static void __sendImage(const char *path)
{
    char buffer[256];
    DBG("Sending image: %s", path);
    sprintf(buffer, "sz -y %s", path);
    system(buffer);
}

static void sendImage(const struct TCEntry *tc, struct MScalerImageData *data)
{
    int i;
    char path[256];
    
    if (tc->storeEachPlane) {
        for (i = 0; i < data->planes; i++) {
            getDstPlanePath(tc, path, sizeof(path), i);
            __sendImage(path);
        }
    }
    __sendImage(tc->dst.path);
}

static void __removeImage(const char *path)
{
    char buffer[256];
    DBG("Remove image: %s", path);
    sprintf(buffer, "rm -rf %s", path);
    system(buffer);
}

static void removeImage(const struct TCEntry *tc, struct MScalerImageData *data)
{
    int i;
    char path[256];
    
    if (tc->storeEachPlane) {
        for (i = 0; i < data->planes; i++) {
            getDstPlanePath(tc, path, sizeof(path), i);
            __removeImage(path);
        }
    }
    __removeImage(tc->dst.path);
}

static int doTestCase(struct TCEntry *tc)
{
    int ret;
    int loopCount;
    struct MScalerImageFormat srcFormat, dstFormat;
    struct MScalerImageData srcData, dstData;
    struct MScalerBufferInfo srcBufInfo, dstBufInfo;

    TRACE_LINE;

    ASSERT(tc);
    ASSERT(tc->src.path);

    DBG("========================================");
    DBG(" TEST %s ", tc->name);

    srcFormat = tc->src.fmt;

    ret = MScalerLock(hScaler, -1);
    if (ret) {
        ERR("MScalerLock() failed.");
        goto done;
    }

    ret = MScalerSetScalerCoeff(hScaler, &scaler_coeff);
    if (ret) {
        ERR("MScalerSetScalerCoeff() failed.");
        goto err1;
    }

    ret = MScalerSetRGBCoeff(hScaler, &rgb_coeff);
    if (ret) {
        ERR("MScalerSetRGBCoeff() failed.");
        goto err1;
    }

    DBG("Src: size=(%d x %d) crop=(%d, %d, %d, %d) pixelformat=0x%x",
            srcFormat.width, srcFormat.height,
            srcFormat.crop.top, srcFormat.crop.left,
            srcFormat.crop.width, srcFormat.crop.height,
            srcFormat.pixelformat);

    dstFormat = tc->dst.fmt;

    DBG("Dst: size=(%d x %d) crop=(%d, %d, %d, %d) pixelformat=0x%x",
            dstFormat.width, dstFormat.height,
            dstFormat.crop.top, dstFormat.crop.left,
            dstFormat.crop.width, dstFormat.crop.height,
            dstFormat.pixelformat);

    ret = MScalerSetImageFormat(hScaler, &srcFormat, &dstFormat);
    if (ret) {
        ERR("MScalerSetImageFormat() failed.");
        goto err1;
    }

    ret = MScalerGetBufferInfo(hScaler, &srcBufInfo, 0);
    if (ret) {
        ERR("MScalerGetBufferInfo(src) failed.");
        goto err1;
    }

    ret = MScalerGetBufferInfo(hScaler, &dstBufInfo, 1);
    if (ret) {
        ERR("MScalerGetBufferInfo(src) failed.");
        goto err1;
    }

    ret = allocContigMemory(&srcData, &srcBufInfo);
    if (ret) {
        DBG("allocContigMemory(srcData) failed.");
        goto err1;
    }

    ret = allocContigMemory(&dstData, &dstBufInfo);
    if (ret) {
        DBG("allocContigMemory(dstData) failed.");
        goto err2;
    }

    ret = loadImage(&srcData, &tc->src);
    if (ret) {
        DBG("loadImage() failed.");
        goto err3;
    }

    // -----
    ESTIMATE_START("TEST ESTIMATION");
    // -----

    loopCount = tc->loopCount;

    do {
        ret = MScalerSetImageData(hScaler, &srcData, &dstData);
        if (ret) {
            DBG("MScalerSetImageData() failed.");
            goto err3;
        }

        ret = MScalerRun(hScaler);
        if (ret) {
            DBG("MScalerRun() failed.");
            goto err4;
        }

        ret = MScalerWaitDone(hScaler, -1);
        if (ret) {
            DBG("MScalerWaitDone() failed.");
            goto err4;
        }

        loopCount--;

    } while(loopCount > 0);

    MScalerStop(hScaler);

    MScalerUnlock(hScaler);

    // -----
    ESTIMATE_STOP();
    // -----

    storeImage(tc, &dstData);

    if (!tc->dontSend)
        sendImage(tc, &dstData);

    if (!tc->dontRemove)
        removeImage(tc, &dstData);

    freeContigMemory(&dstData);
    freeContigMemory(&srcData);

    DBG("%s: Success.", tc->name);
    DBG("========================================");

    return 0;

err4:
    MScalerCancel(hScaler);

err3:
    freeContigMemory(&dstData);

err2:
    freeContigMemory(&srcData);

err1:
    MScalerUnlock(hScaler);

done:
    DBG("%s: Failed.", tc->name);
    DBG("========================================");
    return ret;
}

static struct TCEntry *buildTCTable(dictionary *dict)
{
    int i;
    char *tmp;
    char *pixfmt;
    char *section;
    char key[256];
    int count;
    struct TCEntry *e;
    struct TCEntry *table;

    count = iniparser_getnsec(dict);
    ASSERT(count > 1);

    table = calloc(count, sizeof(struct TCEntry));
    ASSERT(table);

    for (i = 1; i < count; i++) {
        e = &table[i - 1];

        section = iniparser_getsecname(dict, i);
        ASSERT(section);

        e->name = section;

        sprintf(key, "%s:Disable", section);
        e->disabled = iniparser_getint(dict, key, 0);

        if (e->disabled)
            continue;

        sprintf(key, "%s:StoreEachPlane", section);
        e->storeEachPlane = iniparser_getint(dict, key, 0);
        
        sprintf(key, "%s:DoNotSend", section);
        e->dontSend = iniparser_getint(dict, key, 0);

        sprintf(key, "%s:DoNotRemove", section);
        e->dontRemove = iniparser_getint(dict, key, 0);

        sprintf(key, "%s:RandomCount", section);
        e->randomCount = iniparser_getint(dict, key, 0);

        sprintf(key, "%s:LoopCount", section);
        e->loopCount = iniparser_getint(dict, key, 0);

        sprintf(key, "%s:SrcPath", section);
        e->src.path = iniparser_getstring(dict, key, NULL);
        ASSERT(e->src.path);

        sprintf(key, "%s:SrcPixelFormat", section);
        pixfmt = iniparser_getstring(dict, key, NULL);
        ASSERT(pixfmt);
        e->src.fmt.pixelformat = str2PixelFormat(pixfmt);
        ASSERT(e->src.fmt.pixelformat != 0);

        sprintf(key, "%s:SrcWidth", section);
        e->src.fmt.width = iniparser_getint(dict, key, 0);
        ASSERT(e->src.fmt.width > 0 && !(e->src.fmt.width & 1));

        sprintf(key, "%s:SrcHeight", section);
        e->src.fmt.height = iniparser_getint(dict, key, 0);
        ASSERT(e->src.fmt.height > 0 && !(e->src.fmt.height & 1));

        sprintf(key, "%s:SrcCropTop", section);
        e->src.fmt.crop.top = iniparser_getint(dict, key, 0);
        ASSERT(e->src.fmt.crop.top >= 0 && !(e->src.fmt.crop.top & 1));

        sprintf(key, "%s:SrcCropLeft", section);
        e->src.fmt.crop.left = iniparser_getint(dict, key, 0);
        ASSERT(e->src.fmt.crop.left >= 0 && !(e->src.fmt.crop.left & 1));

        sprintf(key, "%s:SrcCropWidth", section);
        e->src.fmt.crop.width = iniparser_getint(dict, key, e->src.fmt.width);
        ASSERT(e->src.fmt.crop.width <= e->src.fmt.width && 
                !(e->src.fmt.crop.width & 1));

        sprintf(key, "%s:SrcCropHeight", section);
        e->src.fmt.crop.height = iniparser_getint(dict, key, e->src.fmt.height);
        ASSERT(e->src.fmt.crop.height <= e->src.fmt.height && 
                !(e->src.fmt.crop.height & 1));

        // dst ================================================================
        
        sprintf(key, "%s:DstPixelFormat", section);
        pixfmt = iniparser_getstring(dict, key, pixfmt);
        ASSERT(pixfmt);
        e->dst.fmt.pixelformat = str2PixelFormat(pixfmt);

        if (!e->randomCount) {

            int width, height;
            int ratioHMul, ratioHDiv;
            int ratioVMul, ratioVDiv;

            sprintf(key, "%s:DstRatioHMul", section);
            ratioHMul = iniparser_getint(dict, key, 0);

            sprintf(key, "%s:DstRatioHDiv", section);
            ratioHDiv = iniparser_getint(dict, key, 0);

            sprintf(key, "%s:DstRatioVMul", section);
            ratioVMul = iniparser_getint(dict, key, 0);

            sprintf(key, "%s:DstRatioVDiv", section);
            ratioVDiv = iniparser_getint(dict, key, 0);

            if (ratioHMul * ratioHDiv) {
                e->dst.fmt.width = e->src.fmt.crop.width * ratioHMul / ratioHDiv;
                e->dst.fmt.width = (
                        (e->dst.fmt.width + ((1 << MSCALER_HALIGN_ORDER) - 1))
                        >> MSCALER_HALIGN_ORDER) << MSCALER_HALIGN_ORDER;
            }

            if (ratioVMul * ratioVDiv) {
                e->dst.fmt.height = e->src.fmt.crop.height * ratioVMul / ratioVDiv;
                e->dst.fmt.height = (
                        (e->dst.fmt.height + ((1 << MSCALER_VALIGN_ORDER) - 1))
                        >> MSCALER_VALIGN_ORDER) << MSCALER_VALIGN_ORDER;
            }

            sprintf(key, "%s:DstWidth", section);
            width = iniparser_getint(dict, key, 0);
            if (width > 0) {
                width = ((width + ((1 << MSCALER_HALIGN_ORDER) - 1))
                        >> MSCALER_HALIGN_ORDER) << MSCALER_HALIGN_ORDER;
                e->dst.fmt.width = width;
            }

            ASSERT(e->dst.fmt.width > 0);

            sprintf(key, "%s:DstHeight", section);
            height = iniparser_getint(dict, key, 0);
            if (height > 0) {
                height = ((height + ((1 << MSCALER_VALIGN_ORDER) - 1))
                        >> MSCALER_VALIGN_ORDER) << MSCALER_VALIGN_ORDER;
                e->dst.fmt.height = height;
            }

            ASSERT(e->dst.fmt.height > 0);

            sprintf(key, "%s:DstPath", section);
            tmp = iniparser_getstring(dict, key, NULL);
            if (tmp) {
                e->dst.path = strdup(tmp);
            }
        }

#if 0
        sprintf(key, "%s:DstPath", section);
        tmp = iniparser_getstring(dict, key, NULL);
        if (tmp) {
            e->dst.path = strdup(tmp);
        }
        else {
            e->dst.path = calloc(1, 256);
            ASSERT(e->dst.path);
            sprintf(e->dst.path, "%s.%s.%dx%d.%s",
                    basename(e->src.path), pixfmt,
                    e->dst.fmt.width, e->dst.fmt.height,
                    isYUV(e->dst.fmt.pixelformat) ? "yuv" : "rgb");

        }
#endif  /*0*/
    }

    return table;
}

// 랜덤 크기를 생성한다.
static void genRandomSize(
        unsigned int srcWidth, unsigned int srcHeight,
        float hMaxRatio, float vMaxRatio, float hMinRatio, float vMinRatio,
        unsigned int *dstWidth, unsigned int *dstHeight)
{
    float hRatio;
    float vRatio;
    long int mul, div;

    ASSERT(dstWidth);
    ASSERT(dstHeight);

retry:
    for ( ; ; ) {
        mul = random();
        div = random();
        hRatio = (float)mul / (float)div;
        if (hRatio <= hMaxRatio && hRatio >= hMinRatio)
            break;
    }

    vRatio = hRatio;

    *dstWidth = srcWidth * hRatio;
    *dstHeight = srcHeight * vRatio;

    if (*dstWidth < 64 || *dstHeight < 64)
        goto retry;
}

static struct TCEntry *genRandomTest(const struct TCEntry *tc, int id)
{
    struct TCEntry *randTC = NULL;

    ASSERT(tc->randomCount > 0);
    ASSERT(tc->dst.path == NULL);

    randTC = calloc(1, sizeof(*randTC));
    ASSERT(randTC);

    *randTC = *tc;

    randTC->name = calloc(1, TC_NAME_LENGTH);
    ASSERT(randTC->name);
    snprintf(randTC->name, TC_NAME_LENGTH - 1, "%s-Random-%04d", tc->name, id);

    genRandomSize(
            randTC->src.fmt.width, randTC->src.fmt.height,
            8.0F, 8.0F, 0.125F, 0.125F,
            &randTC->dst.fmt.width, &randTC->dst.fmt.height);

    //randTC->dst.fmt.width = ((randTC->dst.fmt.width + 1) >> 1) << 1;
    randTC->dst.fmt.width = ((randTC->dst.fmt.width + 7) >> 3) << 3;
    randTC->dst.fmt.height = ((randTC->dst.fmt.height + 1) >> 1) << 1;

    DBG("RandomSize = (%d, %d)", randTC->dst.fmt.width, randTC->dst.fmt.height);

    // TODO random pixelformat

    randTC->dst.path = calloc(1, TC_NAME_LENGTH);
    ASSERT(randTC->dst.path);

    getDstPath(randTC->dst.path, TC_NAME_LENGTH, randTC);
#if 0
    snprintf(randTC->dst.path, TC_NAME_LENGTH - 1, "%s.%s.%dx%d.%s",
            basename(randTC->src.path), 
            pixelFormat2String(randTC->dst.fmt.pixelformat),
            randTC->dst.fmt.width,
            randTC->dst.fmt.height,
            isYUV(randTC->dst.fmt.pixelformat) ? "yuv" : "rgb");
#endif  /*0*/

    return randTC;
}

static void freeRandomTest(struct TCEntry *tc)
{
    free(tc->name);
    free(tc->dst.path);
    free(tc);
}

static void runTest(struct TCEntry *table, int count)
{
    int i, j;
    int ret;
    struct TCEntry *tc;

    for (i = 0; i < count; i++) {
        tc = &table[i];
        if (tc->disabled) {
            DBG("========================================");
            DBG("Test %s Disabled. Skip.", tc->name);
            DBG("========================================");
            continue;
        }

        if (tc->randomCount > 0) {
            struct TCEntry *randTC;
            srandom(time(NULL));
            for (j = 0; j < tc->randomCount; j++) {
                randTC = genRandomTest(tc, j);
                ret = doTestCase(randTC);
                freeRandomTest(randTC);
                if (ret)
                    break;
            }
        }
        else {
            doTestCase(tc);
        }
    }
}

static void usage(int argc, char **argv)
{
    fprintf(stderr, "MScaler Test Case Launcher.\n");
    fprintf(stderr, "Usage: %s <*.ini file path>\n", argv[0]);
}

/*****************************************************************************/

int main(int argc, char **argv)
{
    char *node;
    int testCount;
    dictionary *dict;
    struct TCEntry *table;

    if (argc != 2) {
        usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    dict = iniparser_load(argv[1]);
    if (!dict) {
        fprintf(stderr, "%s: INI Parse failed.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    testCount = iniparser_getnsec(dict) - 1;
    ASSERT(testCount > 0);

    node = iniparser_getstring(dict, "global:ScalerDevice", NULL);
    ASSERT(node);

    hScaler = MScalerOpen(node);
    ASSERT(hScaler);

    ionFd = ion_open();
    ASSERT(ionFd > 0);

    table = buildTCTable(dict);
    ASSERT(table);

    // Execute Test Cases
    runTest(table, testCount);

    MScalerClose(hScaler);
    close(ionFd);

    return 0;
}

