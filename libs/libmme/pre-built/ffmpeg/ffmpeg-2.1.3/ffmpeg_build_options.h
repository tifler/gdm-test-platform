#ifndef FFMPEG_BUILD_OPTIONS_H__
#define FFMPEG_BUILD_OPTIONS_H__

#define FFMPEG_OS_WIN32    100
#define FFMPEG_OS_ANDROID  101
#define FFMPEG_OS_ODYFPGA  101

#ifdef WIN32
#define FFMPEG_OS FFMPEG_OS_WIN32
#elif defined (__OMX_PLATFORM_ANDROID)
#define FFMPEG_OS FFMPEG_OS_ANDROID
#elif defined (__PLATFORM_ODY_FPGA)
#define FFMPEG_OS FFMPEG_OS_ODYFPGA
#else
#error "ERROR : Select OS"
#endif


#if (FFMPEG_OS == FFMPEG_OS_ANDROID)

//#include <assert.h>
//#include <sys/cdefs.h>
#include <sys/../time.h>

//#include "ffmpeg_config_android.h"
#include "ffmpeg_config_odyfpga.h"
#include "libavutil/internal.h"

//#define restrict

#define AV_HAVE_INCOMPATIBLE_LIBAV_ABI 0
#define FF_API_AVCODEC_OPEN 1
#define FF_API_ALLOC_CONTEXT 1

#elif (FFMPEG_OS == FFMPEG_OS_ODYFPGA)

//#include <assert.h>
//#include <sys/cdefs.h>
#include <sys/../time.h>

#include "ffmpeg_config_odyfpga.h"
#include "libavutil/internal.h"

//#define restrict

#define AV_HAVE_INCOMPATIBLE_LIBAV_ABI 0
#define FF_API_AVCODEC_OPEN 1
#define FF_API_ALLOC_CONTEXT 1

#endif

#endif