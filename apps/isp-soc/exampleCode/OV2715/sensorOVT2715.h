// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================

/*****************************************************************************
 * Defined Functions:
 *                    - DxOSensor_OVT2715_Initialize()
 *                    - DxOSensor_OVT2715_Uninitialize()
 *                    - DxOSensor_OVT2715_GetStartGrp()
 *                    - DxOSensor_OVT2715_Get()
 *                    - DxOSensor_OVT2715_GetEndGrp()
 *                    - DxOSensor_OVT2715_SetStartGrp()
 *                    - DxOSensor_OVT2715_Set()
 *                    - DxOSensor_OVT2715_SetEndGrp()
 *                    - DxOSensor_OVT2715_Fire()
 *
 *****************************************************************************/
#ifndef __SENSOR_OVT2715_H__
#define __SENSOR_OVT2715_H__
 
#include <stdint.h>


void     DxOSensor_OVT2715_Initialize      ( void );
void     DxOSensor_OVT2715_Uninitialize    ( void );

void     DxOSensor_OVT2715_GetStartGrp     ( void );
void     DxOSensor_OVT2715_Get             ( uint16_t usOffset, uint16_t usSize, void* pBuf );
void     DxOSensor_OVT2715_GetEndGrp       ( void );


void     DxOSensor_OVT2715_SetStartGrp     ( void );
void     DxOSensor_OVT2715_Set             ( uint16_t usOffset, uint16_t usSize, void* pBuf );
void     DxOSensor_OVT2715_SetEndGrp       ( void );

void     DxOSensor_OVT2715_Fire            ( void );


#endif //__SENSOR_OVT2715_H__
/****************************************************************************/
/****************************************************************************/
/* END OF FILE                                                              */
/****************************************************************************/
/****************************************************************************/
