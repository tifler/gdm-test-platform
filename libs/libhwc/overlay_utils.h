#ifndef GDM_DSS_OVERLAY_UTILS_H
#define GDM_DSS_OVERLAY_UTILS_H

enum eGdmDssFlags {
    OV_GDM_FLAGS_NONE = 0,
    OV_GDM_PIPE_FORCE_DMA = GDM_OV_PIPE_FORCE_DMA,
    OV_GDM_DEINTERLACE = MDP_DEINTERLACE,
    OV_GDM_SECURE_OVERLAY_SESSION = GDM_SECURE_OVERLAY_SESSION,
    OV_GDM_SOURCE_ROTATED_90 = GDM_SOURCE_ROTATED_90,
    OV_GDM_BACKEND_COMPOSITION = GDM_BACKEND_COMPOSITION,
    OV_GDM_BLEND_FG_PREMULT = GDM_BLEND_FG_PREMULT,
    OV_GDM_FLIP_H = MDP_FLIP_LR,
    OV_GDM_FLIP_V = MDP_FLIP_UD,
    OV_GDM_PP_EN = MDP_OVERLAY_PP_CFG_EN,
};


enum eMdpPipeType {
    OV_GDM_PIPE_RGB = 0,
    OV_GDM_PIPE_VG,
    OV_GDMPIPE_DMA,
    OV_GDMPIPE_ANY, //Any
};



/* values for copybit_set_parameter(OVERLAY_TRANSFORM) */
enum eTransform {
    /* No rot */
    OVERLAY_TRANSFORM_0 = 0x0,
    /* flip source image horizontally 0x1 */
    OVERLAY_TRANSFORM_FLIP_H = HAL_TRANSFORM_FLIP_H,
    /* flip source image vertically 0x2 */
    OVERLAY_TRANSFORM_FLIP_V = HAL_TRANSFORM_FLIP_V,
    /* rotate source image 180 degrees
     * It is basically bit-or-ed  H | V == 0x3 */
    OVERLAY_TRANSFORM_ROT_180 = HAL_TRANSFORM_ROT_180,
    /* rotate source image 90 degrees 0x4 */
    OVERLAY_TRANSFORM_ROT_90 = HAL_TRANSFORM_ROT_90,
    /* rotate source image 90 degrees and flip horizontally 0x5 */
    OVERLAY_TRANSFORM_ROT_90_FLIP_H = HAL_TRANSFORM_ROT_90 |
                                      HAL_TRANSFORM_FLIP_H,
    /* rotate source image 90 degrees and flip vertically 0x6 */
    OVERLAY_TRANSFORM_ROT_90_FLIP_V = HAL_TRANSFORM_ROT_90 |
                                      HAL_TRANSFORM_FLIP_V,
    /* rotate source image 270 degrees
     * Basically 180 | 90 == 0x7 */
    OVERLAY_TRANSFORM_ROT_270 = HAL_TRANSFORM_ROT_270,
    /* rotate invalid like in Transform.h */
    OVERLAY_TRANSFORM_INV = 0x80
};


enum eBlending {
    OVERLAY_BLENDING_UNDEFINED = 0x0,
    /* No blending */
    OVERLAY_BLENDING_OPAQUE,
    /* src.rgb + dst.rgb*(1-src_alpha) */
    OVERLAY_BLENDING_PREMULT,
    /* src.rgb * src_alpha + dst.rgb (1 - src_alpha) */
    OVERLAY_BLENDING_COVERAGE,
};


#ifdef __cplusplus
extern "C"
{
#endif




#ifdef __cplusplus
}
#endif


#endif
