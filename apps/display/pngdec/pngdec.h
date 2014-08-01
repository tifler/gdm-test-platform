#ifndef PNGDEC_INCLUDE_H
#define PNGDEC_INCLUDE_H

struct image_desc {
	int width;
	int height;
	int pixelformat;
	int pitch;
	void *data;
	int size;
	int flags;
	int fd;
	struct ion_handle *handle;
	int shared_fd;
};

/*
 * @internal
 *
 * Encodes format constants in the following way (bit 31 - 0):
 *
 * lkjj:hhgg | gfff:eeed | cccc:bbbb | baaa:aaaa
 *
 * a) pixelformat index<br>
 * b) effective color (or index) bits per pixel of format<br>
 * c) effective alpha bits per pixel of format<br>
 * d) alpha channel present<br>
 * e) bytes per "pixel in a row" (1/8 fragment, i.e. bits)<br>
 * f) bytes per "pixel in a row" (decimal part, i.e. bytes)<br>
 * g) smallest number of pixels aligned to byte boundary (minus one)<br>
 * h) multiplier for planes minus one (1/4 fragment)<br>
 * j) multiplier for planes minus one (decimal part)<br>
 * k) color and/or alpha lookup table present<br>
 * l) alpha channel is inverted
 */
#define DFB_SURFACE_PIXELFORMAT( index, color_bits, alpha_bits, has_alpha,     \
                                 row_bits, row_bytes, align, mul_f, mul_d,     \
                                 has_lut, inv_alpha )                          \
     ( (((index     ) & 0x7F)      ) |                                         \
       (((color_bits) & 0x1F) <<  7) |                                         \
       (((alpha_bits) & 0x0F) << 12) |                                         \
       (((has_alpha ) ? 1 :0) << 16) |                                         \
       (((row_bits  ) & 0x07) << 17) |                                         \
       (((row_bytes ) & 0x07) << 20) |                                         \
       (((align     ) & 0x07) << 23) |                                         \
       (((mul_f     ) & 0x03) << 26) |                                         \
       (((mul_d     ) & 0x03) << 28) |                                         \
       (((has_lut   ) ? 1 :0) << 30) |                                         \
       (((inv_alpha ) ? 1 :0) << 31) )

/*
 * Pixel format of a surface.
 */
typedef enum {
     DSPF_UNKNOWN   = 0x00000000,  /* unknown or unspecified format */

     /* 16 bit  ARGB (2 byte, alpha 1@15, red 5@10, green 5@5, blue 5@0) */
     DSPF_ARGB1555  = DFB_SURFACE_PIXELFORMAT(  0, 15, 1, 1, 0, 2, 0, 0, 0, 0, 0 ),

     /* 16 bit   RGB (2 byte, red 5@11, green 6@5, blue 5@0) */
     DSPF_RGB16     = DFB_SURFACE_PIXELFORMAT(  1, 16, 0, 0, 0, 2, 0, 0, 0, 0, 0 ),

     /* 24 bit   RGB (3 byte, red 8@16, green 8@8, blue 8@0) */
     DSPF_RGB24     = DFB_SURFACE_PIXELFORMAT(  2, 24, 0, 0, 0, 3, 0, 0, 0, 0, 0 ),

     /* 24 bit   RGB (4 byte, nothing@24, red 8@16, green 8@8, blue 8@0) */
     DSPF_RGB32     = DFB_SURFACE_PIXELFORMAT(  3, 24, 0, 0, 0, 4, 0, 0, 0, 0, 0 ),

     /* 32 bit  ARGB (4 byte, alpha 8@24, red 8@16, green 8@8, blue 8@0) */
     DSPF_ARGB      = DFB_SURFACE_PIXELFORMAT(  4, 24, 8, 1, 0, 4, 0, 0, 0, 0, 0 ),

     /*  8 bit alpha (1 byte, alpha 8@0), e.g. anti-aliased glyphs */
     DSPF_A8        = DFB_SURFACE_PIXELFORMAT(  5,  0, 8, 1, 0, 1, 0, 0, 0, 0, 0 ),

     /* 16 bit   YUV (4 byte/ 2 pixel, macropixel contains CbYCrY [31:0]) */
     DSPF_YUY2      = DFB_SURFACE_PIXELFORMAT(  6, 16, 0, 0, 0, 2, 0, 0, 0, 0, 0 ),

     /*  8 bit   RGB (1 byte, red 3@5, green 3@2, blue 2@0) */
     DSPF_RGB332    = DFB_SURFACE_PIXELFORMAT(  7,  8, 0, 0, 0, 1, 0, 0, 0, 0, 0 ),

     /* 16 bit   YUV (4 byte/ 2 pixel, macropixel contains YCbYCr [31:0]) */
     DSPF_UYVY      = DFB_SURFACE_PIXELFORMAT(  8, 16, 0, 0, 0, 2, 0, 0, 0, 0, 0 ),

     /* 12 bit   YUV (8 bit Y plane followed by 8 bit quarter size U/V planes) */
     DSPF_I420      = DFB_SURFACE_PIXELFORMAT(  9, 12, 0, 0, 0, 1, 0, 2, 0, 0, 0 ),

     /* 12 bit   YUV (8 bit Y plane followed by 8 bit quarter size V/U planes) */
     DSPF_YV12      = DFB_SURFACE_PIXELFORMAT( 10, 12, 0, 0, 0, 1, 0, 2, 0, 0, 0 ),

     /*  8 bit   LUT (8 bit color and alpha lookup from palette) */
     DSPF_LUT8      = DFB_SURFACE_PIXELFORMAT( 11,  8, 0, 1, 0, 1, 0, 0, 0, 1, 0 ),

     /*  8 bit  ALUT (1 byte, alpha 4@4, color lookup 4@0) */
     DSPF_ALUT44    = DFB_SURFACE_PIXELFORMAT( 12,  4, 4, 1, 0, 1, 0, 0, 0, 1, 0 ),

     /* 32 bit  ARGB (4 byte, inv. alpha 8@24, red 8@16, green 8@8, blue 8@0) */
     DSPF_AiRGB     = DFB_SURFACE_PIXELFORMAT( 13, 24, 8, 1, 0, 4, 0, 0, 0, 0, 1 ),

     /*  1 bit alpha (1 byte/ 8 pixel, most significant bit used first) */
     DSPF_A1        = DFB_SURFACE_PIXELFORMAT( 14,  0, 1, 1, 1, 0, 7, 0, 0, 0, 0 ),

     /* 12 bit   YUV (8 bit Y plane followed by one 16 bit quarter size Cb|Cr [7:0|7:0] plane) */
     DSPF_NV12      = DFB_SURFACE_PIXELFORMAT( 15, 12, 0, 0, 0, 1, 0, 2, 0, 0, 0 ),

     /* 16 bit   YUV (8 bit Y plane followed by one 16 bit half width Cb|Cr [7:0|7:0] plane) */
     DSPF_NV16      = DFB_SURFACE_PIXELFORMAT( 16, 16, 0, 0, 0, 1, 0, 0, 1, 0, 0 ),

     /* 16 bit  ARGB (2 byte, alpha 2@14, red 5@9, green 5@4, blue 4@0) */
     DSPF_ARGB2554  = DFB_SURFACE_PIXELFORMAT( 17, 14, 2, 1, 0, 2, 0, 0, 0, 0, 0 ),

     /* 16 bit  ARGB (2 byte, alpha 4@12, red 4@8, green 4@4, blue 4@0) */
     DSPF_ARGB4444  = DFB_SURFACE_PIXELFORMAT( 18, 12, 4, 1, 0, 2, 0, 0, 0, 0, 0 ),

     /* 16 bit  RGBA (2 byte, red 4@12, green 4@8, blue 4@4, alpha 4@0) */
     DSPF_RGBA4444  = DFB_SURFACE_PIXELFORMAT( 19, 12, 4, 1, 0, 2, 0, 0, 0, 0, 0 ),

     /* 12 bit   YUV (8 bit Y plane followed by one 16 bit quarter size Cr|Cb [7:0|7:0] plane) */
     DSPF_NV21      = DFB_SURFACE_PIXELFORMAT( 20, 12, 0, 0, 0, 1, 0, 2, 0, 0, 0 ),

     /* 32 bit  AYUV (4 byte, alpha 8@24, Y 8@16, Cb 8@8, Cr 8@0) */
     DSPF_AYUV      = DFB_SURFACE_PIXELFORMAT( 21, 24, 8, 1, 0, 4, 0, 0, 0, 0, 0 ),

     /*  4 bit alpha (1 byte/ 2 pixel, more significant nibble used first) */
     DSPF_A4        = DFB_SURFACE_PIXELFORMAT( 22,  0, 4, 1, 4, 0, 1, 0, 0, 0, 0 ),

     /*  1 bit alpha (3 byte/  alpha 1@18, red 6@12, green 6@6, blue 6@0) */
     DSPF_ARGB1666  = DFB_SURFACE_PIXELFORMAT( 23, 18, 1, 1, 0, 3, 0, 0, 0, 0, 0 ),

     /*  6 bit alpha (3 byte/  alpha 6@18, red 6@12, green 6@6, blue 6@0) */
     DSPF_ARGB6666  = DFB_SURFACE_PIXELFORMAT( 24, 18, 6, 1, 0, 3, 0, 0, 0, 0, 0 ),

     /*  6 bit   RGB (3 byte/   red 6@12, green 6@6, blue 6@0) */
     DSPF_RGB18     = DFB_SURFACE_PIXELFORMAT( 25, 18, 0, 0, 0, 3, 0, 0, 0, 0, 0 ),

     /*  2 bit   LUT (1 byte/ 4 pixel, 2 bit color and alpha lookup from palette) */
     DSPF_LUT2      = DFB_SURFACE_PIXELFORMAT( 26,  2, 0, 1, 2, 0, 3, 0, 0, 1, 0 ),

     /* 16 bit   RGB (2 byte, nothing @12, red 4@8, green 4@4, blue 4@0) */
     DSPF_RGB444    = DFB_SURFACE_PIXELFORMAT( 27, 12, 0, 0, 0, 2, 0, 0, 0, 0, 0 ),

     /* 16 bit   RGB (2 byte, nothing @15, red 5@10, green 5@5, blue 5@0) */
     DSPF_RGB555    = DFB_SURFACE_PIXELFORMAT( 28, 15, 0, 0, 0, 2, 0, 0, 0, 0, 0 ),

     /* 16 bit   BGR (2 byte, nothing @15, blue 5@10, green 5@5, red 5@0) */
     DSPF_BGR555    = DFB_SURFACE_PIXELFORMAT( 29, 15, 0, 0, 0, 2, 0, 0, 0, 0, 0 ),

     /* 16 bit  RGBA (2 byte, red 5@11, green 5@6, blue 5@1, alpha 1@0) */
     DSPF_RGBA5551  = DFB_SURFACE_PIXELFORMAT( 30, 15, 1, 1, 0, 2, 0, 0, 0, 0, 0 ),

     /* 24 bit full YUV planar (8 bit Y plane followed by an 8 bit Cb and an
        8 bit Cr plane) */
     DSPF_YUV444P   = DFB_SURFACE_PIXELFORMAT( 31, 24, 0, 0, 0, 1, 0, 0, 2, 0, 0 ),

     /* 24 bit  ARGB (3 byte, alpha 8@16, red 5@11, green 6@5, blue 5@0) */
     DSPF_ARGB8565  = DFB_SURFACE_PIXELFORMAT( 32, 16, 8, 1, 0, 3, 0, 0, 0, 0, 0 ),

     /* 32 bit  AVYU 4:4:4 (4 byte, alpha 8@24, Cr 8@16, Y 8@8, Cb 8@0) */
     DSPF_AVYU      = DFB_SURFACE_PIXELFORMAT( 33, 24, 8, 1, 0, 4, 0, 0, 0, 0, 0 ),

     /* 24 bit   VYU 4:4:4 (3 byte, Cr 8@16, Y 8@8, Cb 8@0) */
     DSPF_VYU       = DFB_SURFACE_PIXELFORMAT( 34, 24, 0, 0, 0, 3, 0, 0, 0, 0, 0 ),

     /*  1 bit alpha (1 byte/ 8 pixel, LEAST significant bit used first) */
     DSPF_A1_LSB    = DFB_SURFACE_PIXELFORMAT( 35,  0, 1, 1, 1, 0, 7, 0, 0, 0, 0 ),

     /* 16 bit   YUV (8 bit Y plane followed by 8 bit 2x1 subsampled V/U planes) */
     DSPF_YV16      = DFB_SURFACE_PIXELFORMAT( 36, 16, 0, 0, 0, 1, 0, 0, 1, 0, 0 ),

     /* 32 bit  ABGR (4 byte, alpha 8@24, blue 8@16, green 8@8, red 8@0) */
     DSPF_ABGR      = DFB_SURFACE_PIXELFORMAT( 37, 24, 8, 1, 0, 4, 0, 0, 0, 0, 0 ),

     /* 32 bit RGBAF (4 byte, red 8@24, green 8@16, blue 8@8, alpha 7@1, flash 1@0 */
     DSPF_RGBAF88871 = DFB_SURFACE_PIXELFORMAT( 38, 24, 7, 1, 0, 4, 0, 0, 0, 0, 0 ),

     /*  4 bit   LUT (1 byte/ 2 pixel, 4 bit color and alpha lookup from palette) */
     DSPF_LUT4      = DFB_SURFACE_PIXELFORMAT( 39,  4, 0, 1, 4, 0, 1, 0, 0, 1, 0 ),

     /*  16 bit   LUT (1 byte alpha and 8 bit color lookup from palette) */
     DSPF_ALUT8     = DFB_SURFACE_PIXELFORMAT( 40,  8, 8, 1, 0, 2, 0, 0, 0, 1, 0 )

} DFBSurfacePixelFormat;

/* Number of pixelformats defined */
#define DFB_NUM_PIXELFORMATS            41

/* These macros extract information about the pixel format. */
#define DFB_PIXELFORMAT_INDEX(fmt)      (((fmt) & 0x0000007F)      )

#define DFB_COLOR_BITS_PER_PIXEL(fmt)   (((fmt) & 0x00000F80) >>  7)

#define DFB_ALPHA_BITS_PER_PIXEL(fmt)   (((fmt) & 0x0000F000) >> 12)

#define DFB_PIXELFORMAT_HAS_ALPHA(fmt)  (((fmt) & 0x00010000) !=  0)

#define DFB_BITS_PER_PIXEL(fmt)         (((fmt) & 0x007E0000) >> 17)

#define DFB_BYTES_PER_PIXEL(fmt)        (((fmt) & 0x00700000) >> 20)

#define DFB_BYTES_PER_LINE(fmt,width)   (((((fmt) & 0x007E0000) >> 17) * (width) + 7) >> 3)

#define DFB_PIXELFORMAT_ALIGNMENT(fmt)  (((fmt) & 0x03800000) >> 23)

#define DFB_PLANE_MULTIPLY(fmt,height)  ((((((fmt) & 0x3C000000) >> 26) + 4) * (height)) >> 2)

#define DFB_PIXELFORMAT_IS_INDEXED(fmt) (((fmt) & 0x40000000) !=  0)

#define DFB_PLANAR_PIXELFORMAT(fmt)     (((fmt) & 0x3C000000) !=  0)

#define DFB_PIXELFORMAT_INV_ALPHA(fmt)  (((fmt) & 0x80000000) !=  0)

#define DFB_COLOR_IS_RGB(fmt)           \
     (((fmt) == DSPF_ARGB1555)     ||   \
      ((fmt) == DSPF_RGB16)        ||   \
      ((fmt) == DSPF_RGB24)        ||   \
      ((fmt) == DSPF_RGB32)        ||   \
      ((fmt) == DSPF_ARGB)         ||   \
      ((fmt) == DSPF_RGB332)       ||   \
      ((fmt) == DSPF_AiRGB)        ||   \
      ((fmt) == DSPF_ARGB2554)     ||   \
      ((fmt) == DSPF_ARGB4444)     ||   \
      ((fmt) == DSPF_RGBA4444)     ||   \
      ((fmt) == DSPF_ARGB1666)     ||   \
      ((fmt) == DSPF_ARGB6666)     ||   \
      ((fmt) == DSPF_RGB18)        ||   \
      ((fmt) == DSPF_RGB444)       ||   \
      ((fmt) == DSPF_RGB555)       ||   \
      ((fmt) == DSPF_BGR555)       ||   \
      ((fmt) == DSPF_RGBAF88871))

#define DFB_COLOR_IS_YUV(fmt)           \
     (((fmt) == DSPF_YUY2)         ||   \
      ((fmt) == DSPF_UYVY)         ||   \
      ((fmt) == DSPF_I420)         ||   \
      ((fmt) == DSPF_YV12)         ||   \
      ((fmt) == DSPF_NV12)         ||   \
      ((fmt) == DSPF_NV16)         ||   \
      ((fmt) == DSPF_NV21)         ||   \
      ((fmt) == DSPF_AYUV)         ||   \
      ((fmt) == DSPF_YUV444P)      ||   \
      ((fmt) == DSPF_AVYU)         ||   \
      ((fmt) == DSPF_VYU)          ||   \
      ((fmt) == DSPF_YV16))

typedef struct {
     unsigned char             a;   /* alpha channel */
     unsigned char             r;   /* red channel */
     unsigned char             g;   /* green channel */
     unsigned char             b;   /* blue channel */
} DFBColor;


#endif // PNGDEC_INCLUDE_H
