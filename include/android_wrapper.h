#ifndef ANDROID_WRAPPER_H_INCLUDE
#define ANDROID_WRAPPER_H_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <linux/types.h>
#include <system/graphics.h>
#include <errno.h>

#define __BEGIN_DECLS
#define __END_DECLS


#define MAX_NUM_LAYERS	16

typedef struct {
	int 	fd;
	int	offset;
	int	format;
	int	bufferType;
} private_handle_t;

typedef int	buffer_handle_t;
#ifndef __cplusplus
typedef int	bool;
#endif

#define false	0
#define true	1


#define CONDITION(cond)     (__builtin_expect((cond)!=0, 0))


#define ALOGE(...) 	fprintf(stderr, " LOGE(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define ALOGI(...)	fprintf(stderr, " LOGI(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define ALOGW(...)	fprintf(stderr, " LOGW(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define ALOGD(...)	fprintf(stderr, " LOGD(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define ALOGV(...)	fprintf(stderr, " LOGD(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)

#define ALOGD_IF(cond, ...) \
    ( (CONDITION(cond)) \
    ? ((void)ALOGD(...)) \
    : (void)0 )

#define ALOGV_IF(cond, ...) \
    ( (CONDITION(cond)) \
    ? ((void)ALOGV(...)) \
    : (void)0 )

#define ALOGI_IF(cond, ...) \
    ( (CONDITION(cond)) \
    ? ((void)ALOGI(...)) \
    : (void)0 )


#define ATRACE_CALL()	fprintf(stderr, " ATRACE(%s, %s(), %d): \n", __FILE__, __FUNCTION__, __LINE__)


#define LIKELY( exp )       (__builtin_expect( (exp) != 0, true  ))
#define UNLIKELY( exp )     (__builtin_expect( (exp) != 0, false ))


#endif
