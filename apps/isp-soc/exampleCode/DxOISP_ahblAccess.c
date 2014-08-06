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

#include "isp.h"
#include "ioctl.h"
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

// FIXME: 현재 DxO 레지스터 맵 상에 hole이 존재한다.
//        이 hole을 access하면 data abort가 발생한다.(ex: 0xa0d4)
//        이를 ISP팀에서는 읽지 않는것으로 해결중이다.
//        H/W 문제로 보이며, 이 문제는 반드시 해결되어야 한다.
void DxOISP_getMultiRegister(
        const uint32_t uiOffset, uint32_t* puiDst, uint32_t uiNbElem) 
{
#if 1
#if 1
    // 왜 uiOffset이 const인지 도무지 이해가 안가지만 선언이 그러니 따른다.
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
    // 왜 uiOffset이 const인지 도무지 이해가 안가지만 선언이 그러니 따른다.
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
