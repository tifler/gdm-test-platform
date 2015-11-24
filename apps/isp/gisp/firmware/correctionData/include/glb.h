// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2011 - (All rights reserved)
// ============================================================================

#ifndef _GLB_H_
#define _GLB_H_

#ifndef INC
	#define INC(f) <f>
#endif
#include <stdint.h>

extern void ProcNS(bin_V)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      void     * const
);

extern void ProcNS(zoom_INVV_mkBuf)(
	const void     * const
,	const uint16_t * const
,	const uint16_t
,	const uint16_t * const
,	const uint16_t
);

extern void ProcNS(zoom_OCT_mkBuf)(
	const void     * const
,	const uint16_t * const
,	const uint16_t
,	const uint16_t * const
,	const uint16_t
);

extern void ProcNS(zoom_DD_mkBuf)(
	const void     * const
,	const uint16_t * const
,	const uint16_t
,	const uint16_t * const
,	const uint16_t
);

extern void ProcNS(zoom_V_mkBuf)(
	const void     * const
,	const uint16_t * const
,	const uint16_t
,	const uint16_t * const
,	const uint16_t
);

extern void ProcNS(zoom_USM_mkBuf)(
	const void     * const
,	const uint16_t * const
,	const uint16_t
,	const uint16_t * const
,	const uint16_t
);

extern void ProcNS(conv_INVV_bi)(
	const uint8_t
,	const void     * const
,	      void     *
);

extern void ProcNS(zoom_INVV_bi)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(zoom_OCT_bi)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(conv_OCT_bi)(
	const uint8_t
,	const void    * const
,	      void    * const
);

extern void ProcNS(zoom_INVV)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(zoom_OCT)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(zoom_DD)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(zoom_DD_bi)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(zoom_V)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(zoom_V_bi)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(conv_DD_bi)(
	const uint8_t
,	const void    * const
,	      void    * const
);

extern void ProcNS(conv_V_bi)(
	const uint8_t
,	const void    * const
,	      void    * const
);

extern void ProcNS(conv_V_CLS)(
	      void    * const
);

extern void ProcNS(fmt_HDRF)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	      int16_t  * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_HDRT)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(zoom_USM)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	const uint16_t
,	      uint16_t * const
,	const uint16_t
,	      void     * const
);

extern void ProcNS(insert_USM)(
	const void     * const
,	const uint16_t
,	const uint16_t
,	      void     * const
);

extern void ProcNS(fmt_USM)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_SY)(
	const int16_t  * const
,	const uint16_t
,	const uint16_t
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_V)(
	const void * const
,	      void * const
);

extern void ProcNS(fmt_DO)(
	const void     * const
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	      void     * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
);

extern void ProcNS(daf_insert_WB)(
	const uint16_t
,	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(insert_pedestal)(
	const uint16_t
,	       int16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_YF)(
	const uint16_t * const
,	const uint16_t * const
,	      int16_t  * const
);

extern void ProcNS(fmt_NM_NoDiag)(
	int16_t  *
,	int16_t  *
,	uint16_t *
,	uint16_t
,	uint16_t
);

extern void ProcNS(fmt_BP)(
	const uint16_t
,	      int16_t  * const
);

extern void ProcNS(fmt_BP_4C)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
);

extern void ProcNS(pedestal_BP)(
	const uint16_t
,	      int16_t  * const
);

extern void ProcNS(pedestal_BP_4C)(
	const uint16_t
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
);

extern void ProcNS(tune_ADP)(
	const uint16_t
,	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_CF)(
	const uint16_t
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_DN)(
	const uint16_t * const
,	const uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_GF)(
	const int16_t * const
,	const int16_t * const
,	      int16_t  * const
,	      int16_t  * const
);

extern void ProcNS(fmt_GF_soshy2)(
	const int16_t * const
,	const int16_t * const
,	      int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
);

extern void ProcNS(fmt_M)(
	const uint16_t
,	const uint16_t
,	const int16_t * const
,	      int16_t * const

);

extern void ProcNS(fmt_NM)(
	const uint16_t
,	const uint16_t
,	const int16_t  * const
,	const uint16_t * const
,	const uint16_t * const
,	      int16_t  * const
);

extern void ProcNS(tune_SIFI)(
	const uint16_t
,	const uint16_t
,	      int16_t  * const
,	      int16_t  * const
,	      uint16_t * const
);

extern void ProcNS(insert_scales_SIFI)(
	const uint16_t
,	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_TC)(
	const int16_t * const
,	const int16_t * const
,	const int16_t * const
,	const int16_t * const
,	const int16_t * const
,	const int16_t * const
,	      int16_t * const
,	      int16_t * const
,	      int16_t * const
);

extern void ProcNS(fmt_DD)(
	const uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(insert_AA_MultiPhase)(
	uint16_t const
,	void     const *const
,	uint16_t const
,	uint16_t const
,	uint16_t const
,	uint16_t       *const
,	uint8_t        *const
,	uint8_t        *const
);

extern void ProcNS(insert_RIY)(
	const void     * const
,	const void     * const
,	const void     * const
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(tune_N)(
	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(insert_WB)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(insert_WB_CLS)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(insert_WB_noGain)(
	const uint16_t
,	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(tune_TC_B)(
	const int16_t
,	      int16_t *
,	      int16_t *
,	      int16_t *
,	      int16_t *
,	      int16_t *
,	      int16_t *
);

extern void ProcNS(tune_TC_C)(
	const int16_t
,	      int16_t * const
,	      int16_t * const
,	      int16_t * const
,	      int16_t * const
,	      int16_t * const
,	      int16_t * const
);

extern void ProcNS(tune_RGB2YCbCr)(
	const int16_t
,	      int16_t * const
);

extern void ProcNS(tune_NM)(
	const uint16_t
,	      int16_t  * const
,	      uint16_t * const
,	      uint16_t * const
,	const uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(fmt_T)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const int16_t  * const
,	const int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      uint16_t * const
);

extern void ProcNS(tune_fmt_T)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	const int16_t  * const
,	const int16_t  * const
,	      int16_t  * const
,	      int16_t  * const
,	      uint8_t  * const
);

extern void ProcNS(tune_RCRF)(
	const uint16_t
,	const int16_t  * const
,	const uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
);

extern void ProcNS(tune_CF)(
	const uint16_t
,	      uint16_t * const
,	      uint16_t * const
,	const uint16_t * const
,	const uint16_t * const
,	      uint16_t * const
,	      uint16_t * const
);

extern void ProcNS(tune_V)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	const uint16_t
,	      void     * const
);

extern void ProcNS(tune_V_noGain)(
	const uint16_t
,	const uint16_t
,	const uint16_t
,	      void     * const
);

extern void ProcNS(tune_DN)(
	const uint16_t
,	      uint16_t * const
,	const uint16_t * const
);

extern void ProcNS(tune_GF)(
	uint16_t const
,	uint16_t const
,	int16_t        *const
,	int16_t        *const
);

extern void ProcNS(tune_GF_soshy2)(
	uint16_t const
,	uint16_t const
,	int16_t        *const
,	int16_t        *const
,	int16_t        *const
);

extern void ProcNS(insert_ACLS)(
	uint8_t  const        i_aclsActivated
,	void     const *const i_clsMaps
,	uint16_t const        i_xEnd
,	uint16_t const        i_xStart
,	uint16_t const        i_yEnd
,	uint16_t const        i_yStart
,	uint8_t  const        i_hMirror
,	uint8_t  const        i_vFlip
,	uint16_t const        i_xMaxSize
,	uint16_t const        i_yMaxSize
,	void           *const io_mapVRaw
);

extern void ProcNS(insert_Bypass)(
	uint32_t  const i_uiBypassTF
,	uint32_t  const i_uiBypassSensor
,	uint32_t *const o_uiBypass
);

extern void ProcNS(nextY_OCT)(
	void const *const
,	void       *const
);

extern void ProcNS(nextY_V)(
	void const *const
,	void       *const
);

extern void ProcNS(nextY_DD)(
	void const *const
,	void       *const
);

extern void ProcNS(geom_OCT)(
	void     const *const
,	uint16_t const
,	void     const *const
,	void     const *const
,	void           *const
);

extern void ProcNS(geom_V)(
	void     const *const
,	uint16_t const
,	void     const *const
,	void     const *const
,	void           *const
);

extern void ProcNS(geom_DD)(
	void     const *const
,	uint16_t const
,	void     const *const
,	void     const *const
,	void           *const
);

#endif // _GLB_H_
