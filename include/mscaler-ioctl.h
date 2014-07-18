#ifndef __MSCALER_IOCTL_H__
#define __MSCALER_IOCTL_H__

#include <linux/videodev2.h>

/*****************************************************************************/

#define VIDIOC_MSCALER_LOCK             (BASE_VIDIOC_PRIVATE + 1)
#define VIDIOC_MSCALER_TRYLOCK          (BASE_VIDIOC_PRIVATE + 2)
#define VIDIOC_MSCALER_UNLOCK           (BASE_VIDIOC_PRIVATE + 3)

#define VIDIOC_MSCALER_SET_SCALER_COEFF (BASE_VIDIOC_PRIVATE + 4)

struct mscaler_scaler_coeff {
#define MSCALER_SCALER_COEFF_APPLY_YH_UP        (0x01)
#define MSCALER_SCALER_COEFF_APPLY_YH_DN        (0x02)
#define MSCALER_SCALER_COEFF_APPLY_YV_UP        (0x04)
#define MSCALER_SCALER_COEFF_APPLY_YV_DN        (0x08)
#define MSCALER_SCALER_COEFF_APPLY_CH_UP        (0x10)
#define MSCALER_SCALER_COEFF_APPLY_CH_DN        (0x20)
#define MSCALER_SCALER_COEFF_APPLY_CV_UP        (0x40)
#define MSCALER_SCALER_COEFF_APPLY_CV_DN        (0x80)
    int apply;              // MSCALER_SCALER_COEFF_APPLY_*
    __s16 yh_up[204];       // real 12x17 = 204
    __s16 yh_dn[204];       // real 12x17 = 204
    __s16 yv_up[86];        // real 5x17 = 85 + 1padding
    __s16 yv_dn[86];        // real 5x17 = 85 + 1padding
    __s16 ch_up[136];       // real 8x17 = 136
    __s16 ch_dn[136];       // real 8x17 = 136
    __s16 cv_up[86];        // real 5x17 = 85 + 1padding
    __s16 cv_dn[86];        // real 5x17 = 85 + 1padding
};

#define VIDIOC_MSCALER_SET_RGB_COEFF    (BASE_VIDIOC_PRIVATE + 5)

struct mscaler_rgb_coeff {
    __u16 use_user_coeff;
    __u16 pre_offset1;
    __u16 pre_offset2;
    __u16 post_offset;
    __u32 coeff0_y;
    __u32 coeff0_v;
    __u32 coeff1_u;
    __u32 coeff1_v;
    __u32 coeff2_u;
};

#define VIDIOC_MSCALER_SET_NR_COEFF     (BASE_VIDIOC_PRIVATE + 6)

struct mscaler_nr_coeff {
    __u32 use_user_coeff;
    __u32 regs[6];
};

#define VIDIOC_MSCALER_SET_RING_COEFF   (BASE_VIDIOC_PRIVATE + 7)

struct mscaler_ring_coeff {
    __u32 use_user_coeff;
    __u32 regs[4];
};


/*****************************************************************************/

#endif  /*__MSCALER_IOCTL_H__*/
