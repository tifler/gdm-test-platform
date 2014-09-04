/*
 * Copyright (c) 2009 Mans Rullgard <mans@mansr.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "ffmpeg_build_options.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/fft.h"
#include "libavcodec/rdft.h"
#include "libavcodec/synth_filter.h"

void ff_fft_permute_neon(FFTContext *s, FFTComplex *z);
void ff_fft_calc_neon(FFTContext *s, FFTComplex *z);

#if (HAVE_NEON == 1)
void fft4_neon(FFTContext *s, FFTComplex *z);
void fft8_neon(FFTContext *s, FFTComplex *z);
void fft16_neon(FFTContext *s, FFTComplex *z);
void fft32_neon(FFTContext *s, FFTComplex *z);
void fft64_neon(FFTContext *s, FFTComplex *z);
void fft128_neon(FFTContext *s, FFTComplex *z);
void fft256_neon(FFTContext *s, FFTComplex *z);
void fft512_neon(FFTContext *s, FFTComplex *z);
void fft1024_neon(FFTContext *s, FFTComplex *z);
void fft2048_neon(FFTContext *s, FFTComplex *z);
void fft4096_neon(FFTContext *s, FFTComplex *z);
void fft8192_neon(FFTContext *s, FFTComplex *z);
void fft16384_neon(FFTContext *s, FFTComplex *z);
void fft32768_neon(FFTContext *s, FFTComplex *z);
void fft65536_neon(FFTContext *s, FFTComplex *z);
#endif

void ff_imdct_half_vfp(FFTContext *s, FFTSample *output, const FFTSample *input);

void ff_imdct_calc_neon(FFTContext *s, FFTSample *output, const FFTSample *input);
void ff_imdct_half_neon(FFTContext *s, FFTSample *output, const FFTSample *input);
void ff_mdct_calc_neon(FFTContext *s, FFTSample *output, const FFTSample *input);

void ff_rdft_calc_neon(struct RDFTContext *s, FFTSample *z);

av_cold void ff_fft_init_arm(FFTContext *s)
{
    int cpu_flags = av_get_cpu_flags();

    if (have_vfp(cpu_flags)) {
#if CONFIG_MDCT
        if (!have_vfpv3(cpu_flags))
            s->imdct_half   = ff_imdct_half_vfp;
#endif
    }

    if (have_neon(cpu_flags)) {
#if CONFIG_FFT
        s->fft_permute  = ff_fft_permute_neon;
        s->fft_calc     = ff_fft_calc_neon;
#endif
#if CONFIG_MDCT
        s->imdct_calc   = ff_imdct_calc_neon;
        s->imdct_half   = ff_imdct_half_neon;
        s->mdct_calc    = ff_mdct_calc_neon;
        s->mdct_permutation = FF_MDCT_PERM_INTERLEAVE;
#endif
    }
}

#if CONFIG_RDFT
av_cold void ff_rdft_init_arm(RDFTContext *s)
{
    int cpu_flags = av_get_cpu_flags();

    if (have_neon(cpu_flags))
        s->rdft_calc    = ff_rdft_calc_neon;
}
#endif

#if (HAVE_NEON == 1)
const void* fft_tab_neon[]={
    /*.word */fft4_neon,
    /*.word */fft8_neon,
    /*.word */fft16_neon,
    /*.word */fft32_neon,
    /*.word */fft64_neon,
    /*.word */fft128_neon,
    /*.word */fft256_neon,
    /*.word */fft512_neon,
    /*.word */fft1024_neon,
    /*.word */fft2048_neon,
    /*.word */fft4096_neon,
    /*.word */fft8192_neon,
    /*.word */fft16384_neon,
    /*.word */fft32768_neon,
    /*.word */fft65536_neon,
};
#endif