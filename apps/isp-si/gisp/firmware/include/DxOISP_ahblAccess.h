// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

#include <stdint.h>

void     DxOISP_setRegister     (const uint32_t    uiOffset   ,        uint32_t  uiValue                         ) ;
uint32_t DxOISP_getRegister     (const uint32_t    uiOffset                                                      ) ;
void     DxOISP_getMultiRegister(const uint32_t    uiOffset   ,        uint32_t* puiDst      , uint32_t uiNbElem ) ;
void     DxOISP_setCopyRegister (const uint32_t    uiOffset   ,  const uint32_t* puiSrc      , uint32_t uiNbElem ) ;

