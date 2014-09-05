static void mainLoop(
        struct GCamera *gcam, struct GDMBuffer **buffers, int count, int display)
{
    int ret;
    int idx;
    struct pollfd pollfd;

    if (display) {
        struct GDMDispFormat fmt;

        gcam->display = GDispOpen(DISPLAY_SOCK_PATH, 0);
        ASSERT(gcam->display);

        fmt.pixelformat = GCAM_PIXEL_FORMAT;
        fmt.width = DISPLAY_WIDTH;
        fmt.height = DISPLAY_HEIGHT;
        GDispSetFormat(gcam->display, &fmt);
    }

    pollfd.fd = gcam->port[GCAM_PORT_DIS].fd;
    pollfd.events = POLLIN;
    pollfd.revents = 0;

    for ( ; ; ) {
        ret = poll(&pollfd, 1, 1000);
        if (ret < 0) {
            perror("poll");
            exit(0);
        }
        else if (ret == 0) {
            DBG("Timeout.");
            continue;
        }

        idx = v4l2_dqbuf(pollfd.fd, 3);
        if (idx < 0) {
            perror("v4l2_dqbuf");
            exit(0);
        }

        if (gcam->display)
            GDispSendFrame(gcam->display, buffers[idx], 1000);

        ret = v4l2_qbuf(gcam->port[GCAM_PORT_DIS].fd,
                DISPLAY_WIDTH, DISPLAY_HEIGHT, buffers[idx], idx);

        if (ret) {
            DBG("=====> QBUF(%d) FAILED <=====", idx);
        }
        DBG("Buffer Index = %d", idx);
    }
}

/*****************************************************************************/

//int initdone;

int main(int argc, char **argv)
{
    int i;
    int ret;
    int planes;
    struct GCamera *gcam;
    struct v4l2_format fmt;
    struct v4l2_pix_format_mplane *pixmp;
    unsigned int planeSizes[3];
    struct GDMBuffer **buffers;
    struct DXOSystemConfig conf;
    struct DXOControl ctrl;
    struct DXOOutputFormat dxoFmt;
    struct SIFConfig sifConf;
    struct ISP *isp;
    struct SIF *sif;
    struct DXO *dxo;

    parseOption(argc, argv);

    buffers = calloc(opt.buffers, sizeof(*buffers));

    gcam = GCamOpen();
    ASSERT(gcam);

    // V4L2 init
    ret = v4l2_enum_fmt(gcam->port[GCAM_PORT_DIS].fd, GCAM_PIXEL_FORMAT);
    ASSERT(ret == 0);

    ret = v4l2_s_fmt(gcam->port[GCAM_PORT_DIS].fd,
            DISPLAY_WIDTH, DISPLAY_HEIGHT, GCAM_PIXEL_FORMAT, &fmt);
    ASSERT(ret == 0);

    ret = v4l2_reqbufs(gcam->port[GCAM_PORT_DIS].fd, opt.buffers);
    DBG("reqbufs result = %d", ret);

    pixmp = &fmt.fmt.pix_mp;
    planes = pixmp->num_planes;
    for (i = 0; i < planes; i++)
        planeSizes[i] = pixmp->plane_fmt[i].sizeimage;

    for (i = 0; i < opt.buffers; i++) {
        buffers[i] = allocContigMemory(planes, planeSizes, 0);
        ASSERT(buffers[i]);
    }

    for (i = 0; i < opt.buffers; i++) {
        ret = v4l2_qbuf(gcam->port[GCAM_PORT_DIS].fd,
                DISPLAY_WIDTH, DISPLAY_HEIGHT, buffers[i], i);
        DBG("v4l2_qbuf = %d", ret);
    }

    memset(&conf, 0, sizeof(conf));
    conf.sysFreqMul = 32;
    conf.sysFreqDiv = 1;
    conf.frmTimeMul = 4;
    conf.frmTimeDiv = 1;

    isp = ISPInit();
    sif = SIFInit();
    dxo = DXOInit(&conf);

    sifConf.width = SENSOR_WIDTH;
    sifConf.height = SENSOR_HEIGHT;
    SIFSetConfig(sif, &sifConf);

    ctrl.input = DXO_INPUT_SOURCE_FRONT;
    ctrl.hMirror = 0;
    ctrl.vFlip = 0;
    ctrl.enableTNR = 0;
    ctrl.fpsMul = 8;
    ctrl.fpsDiv = 1;
    DXOSetControl(dxo, &ctrl);

    dxoFmt.width = DISPLAY_WIDTH;
    dxoFmt.height = DISPLAY_HEIGHT;
    dxoFmt.pixelFormat = V4L2_PIX_FMT_UYVY;
    dxoFmt.crop.left = 0;
    dxoFmt.crop.top = 0;
    dxoFmt.crop.right = DISPLAY_WIDTH - 1;
    dxoFmt.crop.bottom = DISPLAY_HEIGHT - 1;
    DXOSetOutputFormat(dxo, DXO_OUTPUT_DISPLAY, &dxoFmt);

    DXOSetOutputEnable(dxo, 1 << DXO_OUTPUT_DISPLAY, 1 << DXO_OUTPUT_DISPLAY);
    DXORunState(dxo, DXO_STATE_PREVIEW, 0);

//    initdone = 1;

    if (!opt.v4l2)
        pause();

    v4l2_streamon(gcam->port[GCAM_PORT_DIS].fd);
    DBG("======= STREAM ON =======");

    mainLoop(gcam, buffers, opt.buffers, opt.display);

    v4l2_streamoff(gcam->port[GCAM_PORT_DIS].fd);

    DXOExit(dxo);
    SIFExit(sif);
    ISPExit(isp);

    GCamClose(gcam);

    return 0;
}

struct STREAM *streamOpen(int port)
{
    struct STREAM *stream;

    ASSERT(port >= 0);
    ASSERT(port < GISP_PORT_COUNT);

    stream = (struct STREAM *)calloc(1, sizeof(*stream));
    ASSERT(stream);

    snprintf(path, sizeof(path) - 1, "/dev/video%d", port + 1);
    stream->fd = open(path, O_RDWR);
    ASSERT(stream->fd > 0);

    return stream;
}

void streamClose(struct STREAM *stream)
{
    ASSERT(stream);
    ASSERT(stream->fd > 0);
    close(stream->fd);
    free(stream);
}

int streamSetFormat(struct STREAM *stream,
        uint32_t width, uint32_t height, uint32_t pixelformat)
{
    int ret;

    stream->width = width;
    stream->height = height;
    sttream->pixelformat = pixelformat;

    ret = v4l2_enum_fmt(stream->fd, stream->pixelformat);
    ASSERT(ret == 0);

    ret = v4l2_s_fmt(stream->fd,
            stream->width, stream->height, stream->pixelformat, &stream->fmt);
    ASSERT(ret == 0);

    return 0;
}

int streamGetBufferSize(struct STREAM *stream, uint32_t planeSizes[3])
{
    struct v4l2_pix_format_mplane *pixmp;

    pixmp = &stream->fmt.fmt.pix_mp;
    planes = pixmp->num_planes;
    for (i = 0; i < planes; i++)
        planeSizes[i] = pixmp->plane_fmt[i].sizeimage;

    return planes;
}

int streamSetExternalBuffers(
        struct STREAM *stream, uint32_t bufferCount, struct GDMBuffer **buffers)
{
    int i;

    stream->bufferCount = v4l2_reqbufs(stream->fd, bufferCount);
    ASSERT(stream->bufferCount >= 0);

    for (i = 0; i < stream->bufferCount; i++) {
        stream->buffers[i] = buffers[i];
        ret = v4l2_qbuf(
                stream->fd, stream->width, stream->height, buffers[i], i);
        ASSERT(ret == 0);
    }
    
    return stream->bufferCount;
}

int streamStart(struct STREAM *stream)
{
}
