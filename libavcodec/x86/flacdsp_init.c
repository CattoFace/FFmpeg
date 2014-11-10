/*
 * Copyright (c) 2014 James Almer
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

#include "libavcodec/flacdsp.h"
#include "libavutil/x86/cpu.h"
#include "config.h"

void ff_flac_lpc_32_sse4(int32_t *samples, const int coeffs[32], int order,
                         int qlevel, int len);
void ff_flac_lpc_32_xop(int32_t *samples, const int coeffs[32], int order,
                        int qlevel, int len);

void ff_flac_enc_lpc_16_sse4(int32_t *, const int32_t *, int, int, const int32_t *,int);

av_cold void ff_flacdsp_init_x86(FLACDSPContext *c, enum AVSampleFormat fmt,
                                 int bps)
{
#if HAVE_YASM
    int cpu_flags = av_get_cpu_flags();

#if CONFIG_FLAC_DECODER
    if (EXTERNAL_SSE4(cpu_flags)) {
        if (bps > 16)
            c->lpc = ff_flac_lpc_32_sse4;
    }
    if (EXTERNAL_XOP(cpu_flags)) {
        if (bps > 16)
            c->lpc = ff_flac_lpc_32_xop;
    }
#endif

#if CONFIG_FLAC_ENCODER
    if (EXTERNAL_SSE4(cpu_flags)) {
        if (CONFIG_GPL && bps == 16)
            c->lpc_encode = ff_flac_enc_lpc_16_sse4;
    }
#endif
#endif /* HAVE_YASM */
}
