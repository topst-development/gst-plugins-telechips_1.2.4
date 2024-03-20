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

#ifndef GST_TC_DEMUX_IO_H__
#define GST_TC_DEMUX_IO_H__

#include <gst/gst.h>
#include <fcntl.h>
#include "gst_tc_demux_base.h"

G_BEGIN_DECLS

typedef struct _GstTcDemuxIo GstTcDemuxIo;
typedef struct _GstTcDemuxIoClass GstTcDemuxIoClass;

#define GST_TYPE_TC_DEMUXIO                         (gst_tc_demuxio_get_type())
#define GST_IS_TC_DEMUXIO(obj)                      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_TC_DEMUXIO))
#define GST_IS_TC_DEMUXIO_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_TC_DEMUXIO))
#define GST_TC_DEMUXIO_GET_CLASS(obj)               (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_TC_DEMUXIO, GstTcDemuxIoClass))
#define GST_TC_DEMUXIO(obj)                         (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_TC_DEMUXIO, GstTcDemuxIo))
#define GST_TC_DEMUXIO_CLASS(klass)                 (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_TC_DEMUXIO, GstTcDemuxIoClass))
#define GST_TC_DEMUXIO_CAST(obj)                    ((GstTcDemuxIo *)(obj))

//If byte seek interval is smaller than this, don't seek.
//Just read data to flush.
#define GST_TC_MAX_IO_POOL_SIZE			(1 * 1024 * 1024)
#define GST_TC_MAX_IO_BUFFERING_SIZE	(3 * 1024 * 1024)

#define TCDMX_IO_SEEK_NONE		(0)
#define TCDMX_IO_SEEK_NORMAL	(1)
#define TCDMX_IO_SEEK_FORCED	(2)

struct _GstTcDemuxIo
{
	GstMiniObject     mini_object;

	GstPad			* pstSinkPad;
	GstTcDemuxBase  * pstDemux;
	
	gint64			  llCurrentPos;
	gint64			  llSkipSize;
	gint64			  llFileSize;
	gboolean		  bPullMode;
	gint32			  bSeekPending;

	//This is only for push mode
	gint32			  nAdapterIdx;
};

GType gst_tc_demuxio_get_type (void);


void* tcdmx_io_open(const char *pPad, const char *unused);
unsigned long tcdmx_io_read(void *pBuffer, unsigned long ulSize, unsigned long ulCount, void* pstIO);
gint32 tcdmx_io_seek(void *pstIO, long lOffset, gint32 lOrigin);
gint32 tcdmx_io_seek64(void *pstIO, gint64 llOffset, gint32 lOrigin);
long tcdmx_io_tell(void *pstIO);
gint64 tcdmx_io_tell64(void *pstIO);
gint32 tcdmx_io_close(void *pstIO);
unsigned long tcdmx_io_eof(void *pstIO);
unsigned long tcdmx_io_flush(void *pstIO);
unsigned long tcdmx_io_write(const void *pBuffer, guint32 ulSize, guint32 ulCount, void *pstIO);
gint32 tcdmx_io_unlink(const char *pstIO);


G_END_DECLS

#endif /* GST_TC_DEMUX_IO_H__ */
