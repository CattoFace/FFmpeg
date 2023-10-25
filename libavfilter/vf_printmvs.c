
/*
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 * Copyright (c) 2014 Clément Bœsch <u pkh me>
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

/**
 * @file
 * Codec debug viewer filter.
 *
 * All the MV drawing code from Michael Niedermayer is extracted from
 * libavcodec/mpegvideo.c.
 *
 * TODO: segmentation
 */

#include "avfilter.h"
#include "internal.h"
#include "libavutil/imgutils.h"
#include "libavutil/motion_vector.h"
#include "libavutil/opt.h"
#include "libavutil/video_enc_params.h"
#include "qp_table.h"
#include "video.h"

typedef struct PrintMVsContext {
  const AVClass *class;
} PrintMVsContext;

#define OFFSET(x) offsetof(PrintMVsContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM | AV_OPT_FLAG_VIDEO_PARAM
#define CONST(name, help, val, unit)                                           \
  { name, help, 0, AV_OPT_TYPE_CONST, {.i64 = val}, 0, 0, FLAGS, unit }

static const AVOption printmvs_options[] = {{nullptr}};

AVFILTER_DEFINE_CLASS(printmvs);

static int filter_frame(AVFilterLink *inlink, AVFrame *frame) {
  AVFilterContext *ctx = inlink->dst;
  AVFilterLink *outlink = ctx->outputs[0];
  AVFrameSideData *sd =
      av_frame_get_side_data(frame, AV_FRAME_DATA_MOTION_VECTORS);
  if (sd) {
    int i;
    const AVMotionVector *mvs = (const AVMotionVector *)sd->data;
    char printed = 0;
    for (i = 0; i < sd->size / sizeof(*mvs); i++) {
      const AVMotionVector *mv = &mvs[i];
      if (mv->source == -1 &&
          ((mv->motion_y != 0) ||
           (mv->motion_scale == 0 && mv->src_y == mv->dst_y))) {
        if (printed) {
          printf(",");
        } else {
          printed = 1;
        }
        printf("%.2f, %.2f, %d, %d",
               mv->motion_x != 0
                   ? (mv->dst_x + ((float)mv->motion_x) / mv->motion_scale)
                   : (float)mv->src_x,
               mv->motion_y != 0
                   ? (mv->dst_y + ((float)mv->motion_y) / mv->motion_scale)
                   : (float)mv->src_y,
               mv->dst_x, mv->dst_y);
      }
    }
  }
  printf("\n");
  return ff_filter_frame(outlink, frame);
}

static int config_input(AVFilterLink *inlink) { return 0; }

static const AVFilterPad printmvs_inputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO,
        .filter_frame = filter_frame,
        .config_props = config_input,
    },
};

const AVFilter ff_vf_printmvs = {
    .name = "printmvs",
    .description = NULL_IF_CONFIG_SMALL("Print extracted motion vectors."),
    .priv_size = sizeof(PrintMVsContext),
    FILTER_INPUTS(printmvs_inputs),
    FILTER_OUTPUTS(ff_video_default_filterpad),
    .priv_class = &printmvs_class,
};
