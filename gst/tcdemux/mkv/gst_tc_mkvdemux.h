/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2013 Telechips Audio/Video Algorithm Group <AValgorithm@telechips.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GST_TC_MKV_DEMUX_H__
#define GST_TC_MKV_DEMUX_H__

#include <gst/gst.h>
#include "gst_tc_demux_base.h"
#include "mkv_demuxer.h"

G_BEGIN_DECLS

/* #defines don't like whitespace bits */
#define GST_TYPE_MKVDMX              (gst_mkvdmx_get_type())
#define GST_MKVDMX(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MKVDMX,GstTcMkvDemux))
#define GST_MKVDMX_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MKVDMX,GstTcMkvDemuxClass))
#define GST_IS_MKVDMX(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MKVDMX))
#define GST_IS_MKVDMX_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MKVDMX))

typedef struct _GstTcMkvDemux      GstTcMkvDemux;
typedef struct _GstTcMkvDemuxClass GstTcMkvDemuxClass;

//////////////////////////////////////////////////////////////////////////
//
// buffers
//
typedef struct cdmx_buff_t
{
	// for stream buffer
	guchar	*pbyStreamBuff;
	gulong 	 ulStreamBuffSize;
} cdmx_buff_t;

typedef struct vorbis_header_t
{
    guint   u32StreamID;
    guint   u32NumPackets;
    guint   u32InitDone;
    guchar *pbyData[3];
    guint   u32DataSize[3];
} vorbis_header_t;


typedef struct mkv_dmx_inst_t
{
    // common
    cdmx_buff_t             stCdmxBuff;

    /* vorbis info */
    vorbis_header_t        *pstVorbisHeaders;
    guint            u32VorbisTotalCount;
    guint            u32VorbisIndex;
    guint            u32AudioIndex;

    // for MKV demuxer
    mkv_dmx_handle_t        hDmxHandle;
    mkv_dmx_init_t          stDmxInit;
    mkv_dmx_info_t          stDmxInfo;
    mkv_dmx_metadata_t      stDmxMeta;
    mkv_dmx_getstream_t     stDmxGetStream;
    mkv_dmx_outstream_t     stDmxOutStream;
    mkv_dmx_seek_t          stDmxSeek;
    mkv_dmx_error_t         stDmxError;
    mkv_dmx_getavc_t        stDmxGetAvc;
    mkv_dmx_outavc_t        stDmxOutAvc;
    av_result_t (*pfnDmxLib) ( gulong ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
    void                    *pvLibHandle;

} mkv_dmx_inst_t;


struct _GstTcMkvDemux
{
    GstTcDemuxBase element;

    void* pExtDLLModule;
    DMX_FUNC gExtDmxFunc;

// mkv instance
    mkv_dmx_inst_t stInst;
    gpointer        pstSourceHandler;
};

struct _GstTcMkvDemuxClass
{
  GstTcDemuxBaseClass parent_class;
};

GType gst_mkvdmx_get_type (void);

G_END_DECLS

#endif /* GST_TC_MKV_DEMUX_H__ */
