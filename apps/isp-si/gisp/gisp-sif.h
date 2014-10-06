#ifndef __GISP_SIF_H__
#define __GISP_SIF_H__

#include <stdint.h>

/*****************************************************************************/

struct SIF;

struct SIFConfig {
    uint32_t id;
    uint32_t width;
    uint32_t height;
    uint32_t fps;
};

/*****************************************************************************/

struct SIF *SIFInit(int useBT601);
void SIFExit(struct SIF *sif);

void SIFSetConfig(struct SIF *sif, const struct SIFConfig *conf);

#endif  /*__GISP_SIF_H__*/
