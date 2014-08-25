// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "isp-io.h"
#include "debug.h"

/*****************************************************************************/

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
#if 1
#if 1
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
#else
    uint32_t i;

    for (i = 0; i < uiNbElem; i++) {
        *puiDst++ = DXO_READ(uiOffset);
    }
#endif  /*0*/
#else
    memcpy(puiDst, (void *)(DXOBase + uiOffset), uiNbElem * sizeof(uint32_t));
#endif 
}

void DxOISP_setCopyRegister(
        const uint32_t uiOffset, const uint32_t* puiSrc , uint32_t uiNbElem) 
{
#if 1
#if 1
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
#else
    uint32_t i;

    for (i = 0; i < uiNbElem; i++) {
        DXO_WRITE(uiOffset + i * sizeof(uint32_t), *puiSrc++);
    }
#endif  /*0*/
#else
    memcpy((void *)(DXOBase + uiOffset), puiSrc, uiNbElem * sizeof(uint32_t));
#endif
}
