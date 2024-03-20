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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gst_tc_demuxio.h"

GST_DEBUG_CATEGORY_STATIC (gst_tc_demuxio_debug);
#define GST_CAT_DMXIO (gst_tc_demuxio_debug)

static GType gst_tc_demuxio_type = (guint)0;

typedef struct
{
  GstTcDemuxIo tcdemuxio;

  GstStructure *structure;
} GstTcDemuxIoImpl;


#define USE_MALLOC_INSTEAD_OF_SLIZE (0)
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Telechips demuxer I/O class
//
static GstTcDemuxIo *gst_tc_demuxio_copy (GstTcDemuxIo * pstIO);
static void gst_tc_demuxio_free (GstTcDemuxIo * pstIO);

void
g_priv_gst_tc_demuxio_initialize (void)
{
  /* the GstMiniObject types need to be class_ref'd once before it can be
    done from multiple threads; */
  // see http://bugzilla.gnome.org/show_bug.cgi?id=304551

	(void)gst_tc_demuxio_get_type ();

	GST_DEBUG_CATEGORY_INIT (
		gst_tc_demuxio_debug,
		"tcdemux-io",
		GST_DEBUG_FG_BLACK,
		"Debugging info for the telechips demuxer I/O");
	gst_tc_demuxio_type = gst_tc_demuxio_get_type();
}

#define _do_init \
{ \
  GST_DEBUG_CATEGORY_INIT (\
	  gst_tc_demuxio_debug, \
	  "tcdemux-io", \
	  GST_DEBUG_FG_BLACK, \
      "Debugging info for the telechips demuxer I/O"); \
  gst_tc_demuxio_type = g_define_type_id; \
}

//G_DEFINE_TYPE_WITH_CODE (GstTcDemuxIo, gst_tc_demuxio, G_TYPE_BOXED, _do_init);
GST_DEFINE_MINI_OBJECT_TYPE (GstTcDemuxIo, gst_tc_demuxio);

static void
gst_tc_demuxio_init (GstTcDemuxIo * pstIO)
{
	GST_CAT_LOG_OBJECT (GST_CAT_DMXIO, pstIO, "init");

	gst_mini_object_init (GST_MINI_OBJECT_CAST (pstIO), 0, gst_tc_demuxio_type,
      (GstMiniObjectCopyFunction) gst_tc_demuxio_copy, NULL,
      (GstMiniObjectFreeFunction) gst_tc_demuxio_free);
}


static GstTcDemuxIo *
gst_tc_demuxio_copy (GstTcDemuxIo * pstIO)
{
	GST_TRACE("%p", pstIO);
	return NULL;
}

static void
gst_tc_demuxio_free (GstTcDemuxIo * pstIO)
{
  g_return_if_fail (pstIO != NULL);

  GST_CAT_LOG_OBJECT (GST_CAT_DMXIO, pstIO, "finalize");

  if( pstIO->pstSinkPad != NULL ){
	  gst_object_unref(pstIO->pstSinkPad);
  }
}

static inline void
gst_tc_demuxio_unref (GstTcDemuxIo *pstIO)
{
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (pstIO));
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Telechips demuxer I/O helpers
//
static gboolean
gst_tc_demuxio_set_srcpad (GstTcDemuxIo *pstIO, GstPad *pstSinkPad)
{
	gint32 i;
	gint64 size;
	GstFormat format = GST_FORMAT_BYTES;

	if (pstIO->pstSinkPad != NULL){
		gst_object_unref (pstIO->pstSinkPad);
	}

	pstIO->pstSinkPad = pstSinkPad;
	pstIO->pstDemux = GST_DMXBASE (GST_PAD_PARENT (pstSinkPad));

	pstIO->nAdapterIdx = -1;
	for(i = 0;i < TCDMX_MAX_SRC_HANDLER;i++)
	{
		if(pstIO->pstDemux->pstPipe.llInUse[i] == 0)
		{
			pstIO->pstDemux->pstPipe.llInUse[i] = 1;
			pstIO->nAdapterIdx = i;
			break;
		}
	}
	if(pstIO->nAdapterIdx == -1)
	{
		GST_CAT_WARNING_OBJECT (GST_CAT_DMXIO, pstIO, "Failed to get adapter");
		return FALSE;
	}

#if 0//V1.0
	if (gst_pad_query_peer_duration (pstSinkPad, &format, &size) == TRUE && format == GST_FORMAT_BYTES)
#else
	if (gst_pad_peer_query_duration (pstSinkPad, format, &size) > 0)
#endif
	{
		pstIO->llFileSize = size;
		pstIO->pstDemux->llFileSize = size;
		GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "size: %" G_GINT64_FORMAT "bytes (%d MB)", size, (gint32)(size/1048576));
	}
	else {
		pstIO->pstDemux->llFileSize = 0;
		GST_CAT_WARNING_OBJECT (GST_CAT_DMXIO, pstIO, "size is unknown");
		if(gst_pad_is_linked(pstSinkPad) == FALSE)
		{
			GST_CAT_WARNING_OBJECT (GST_CAT_DMXIO, pstIO, "Sink pad is not linked.");
			return FALSE;
		}
	}

	pstIO->pstDemux->llLastReadPos = 0;
	pstIO->pstDemux->pstPipe.llCurrentOffset[pstIO->nAdapterIdx] = 0;
	return TCDMX_TRUE;
}

static GstTcDemuxIo*
gst_tc_demuxio_new (void)
{
#if 0//V1.0
	GstTcDemuxIo *p_newio = (GstTcDemuxIo *) gst_mini_object_new (gst_tc_demuxio_type);
#else
	#if USE_MALLOC_INSTEAD_OF_SLIZE
	GstTcDemuxIo *p_newio = malloc(sizeof(GstTcDemuxIo));
	#else
	GstTcDemuxIo *p_newio = g_slice_new0(GstTcDemuxIo);//g_slice_new0(GstTcDemuxIo);
	#endif
#endif
	GST_CAT_LOG_OBJECT (GST_CAT_DMXIO, p_newio, "new");

	gst_tc_demuxio_init(p_newio);

	return GST_TC_DEMUXIO_CAST (p_newio);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Telechips demuxer I/O callbacks
//

void* tcdmx_io_open (const char *pPad, const char *unused)
{
	GstQuery * query;
	gboolean pull_mode;
	GstPad *p_pad = GST_PAD_CAST (pPad);
	GstTcDemuxIo *p_newio;

	p_newio = gst_tc_demuxio_new ();

	GST_TRACE("unused : %s", unused);
	GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, p_newio, "open");

	query = gst_query_new_scheduling();
	if (gst_pad_peer_query (p_pad, query) == 0) {
		gst_query_unref (query);
		p_newio->bPullMode = FALSE;
	}
	else
	{
		pull_mode = gst_query_has_scheduling_mode_with_flags (query,
						GST_PAD_MODE_PULL, GST_SCHEDULING_FLAG_SEEKABLE);
		gst_query_unref (query);

		if (pull_mode == 0){
			p_newio->bPullMode = TCDMX_FALSE;
		}else{
			p_newio->bPullMode = TCDMX_TRUE;
		}
	}

	(void)gst_object_ref (p_pad);
	if(gst_tc_demuxio_set_srcpad (p_newio, p_pad) == FALSE)
	{
		(void)tcdmx_io_close(p_newio);
		return NULL;
	}

	return p_newio;
}

unsigned long
tcdmx_io_read(void *pBuffer, unsigned long ulSize, unsigned long ulCount, void *pstIO)
{
	GstTcDemuxIo *p_io = GST_TC_DEMUXIO_CAST (pstIO);
	GstBuffer *p_buffer = NULL;
	guint32 read_byte;
	GstFlowReturn ret;
	gint available;
	GstMapInfo stMapInfo;

	GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "size: %ld, offset : %ld", ulSize*ulCount, p_io->llCurrentPos);

	if(p_io->bPullMode != 0)//Pull Mode
	{
		p_io->pstDemux->enIoResult = ret = gst_pad_pull_range (p_io->pstSinkPad, (guint64)p_io->llCurrentPos, ulSize*ulCount, &p_buffer);
		p_io->pstDemux->llLastReadPos = p_io->llCurrentPos;
		if (ret != GST_FLOW_OK) {
			p_io->pstDemux->lLastReadSize = 0;
			GST_CAT_ERROR_OBJECT (GST_CAT_DMXIO, pstIO, "gst_pad_pull_range() - ret: %d", ret);
			return 0;
		}

		(void)gst_buffer_map(p_buffer, &stMapInfo, GST_MAP_READ);
		GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "read: %d", (int)stMapInfo.size);
		(void)memcpy(pBuffer, stMapInfo.data, (guint)stMapInfo.size);

		p_io->llCurrentPos += (int)stMapInfo.size;
		read_byte = stMapInfo.size;
		p_io->pstDemux->lLastReadSize = (gint)stMapInfo.size;

		gst_buffer_unmap(p_buffer, &stMapInfo);
		gst_buffer_unref(p_buffer);

		return read_byte / ulSize;
	}
	else//Push Mode
	{
		guint size = ulSize * ulCount;
		GstEvent * pstEvent;
		gboolean is_OutOfRange;
		GstTcDemuxPipe *pPipe = &p_io->pstDemux->pstPipe;

		GST_TCDMX_PIPE_MUTEX_LOCK (pPipe);

		pPipe->llCurrentOffset[p_io->nAdapterIdx] = p_io->llCurrentPos;

		//Only in case that interesting data is not in range, we perform seeking to target position
		//otherwise, just wait until you get what you want.
		GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO,
							   "[IDX : %d][Start : %lld][End : %lld][Cur : %lld]",
								p_io->nAdapterIdx,
								pPipe->llStartOffset[p_io->nAdapterIdx],
								pPipe->llEndOffset[p_io->nAdapterIdx],
								pPipe->llCurrentOffset[p_io->nAdapterIdx]);

		if(pPipe->lCurrentIOIdx != p_io->nAdapterIdx)
		{
			is_OutOfRange = (pPipe->llStartOffset[p_io->nAdapterIdx] == -1) ||
						   (pPipe->llStartOffset[p_io->nAdapterIdx] > pPipe->llCurrentOffset[p_io->nAdapterIdx]) || 
						  ((pPipe->llEndOffset[p_io->nAdapterIdx] - pPipe->llCurrentOffset[p_io->nAdapterIdx]) < size);
		}
		else
		{
			is_OutOfRange = (pPipe->llStartOffset[p_io->nAdapterIdx] == -1) ||
						   (pPipe->llStartOffset[p_io->nAdapterIdx] > pPipe->llCurrentOffset[p_io->nAdapterIdx]) || 
						   ((pPipe->llEndOffset[p_io->nAdapterIdx] - pPipe->llCurrentOffset[p_io->nAdapterIdx] + GST_TC_MAX_IO_POOL_SIZE) < size);
		}

		if(is_OutOfRange != 0)
		{
			GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "gst_event_new_seek() to %lld[OutOfRange : %d][InstanceChange : %d][Instance : %d]", 
								  (is_OutOfRange)?pPipe->llCurrentOffset[p_io->nAdapterIdx]:pPipe->llEndOffset[p_io->nAdapterIdx], 
								  is_OutOfRange,
								  pPipe->lCurrentIOIdx != p_io->nAdapterIdx,
								  p_io->nAdapterIdx);
			pstEvent = gst_event_new_seek (1.0, 
										   GST_FORMAT_BYTES, 
										   GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH, 
										   GST_SEEK_TYPE_SET, 
										   pPipe->llCurrentOffset[p_io->nAdapterIdx], 
										   GST_SEEK_TYPE_NONE, 
										   -1);

			//Initialize the adapter
			//Start filling another adapter
			pPipe->lCurrentIOIdx = p_io->nAdapterIdx;
			//Reset all anchors
			pPipe->llStartOffset[p_io->nAdapterIdx] = -1;
			pPipe->llEndOffset[p_io->nAdapterIdx] = -1;
			gst_adapter_clear(pPipe->pstAdapter[p_io->nAdapterIdx]);
			pPipe->enSrcResult = GST_FLOW_FLUSHING;
			GST_TCDMX_PIPE_MUTEX_UNLOCK (pPipe);
			if(gst_pad_push_event (p_io->pstSinkPad, pstEvent) == FALSE)
			{
				GST_CAT_ERROR_OBJECT (GST_CAT_DMXIO, pstIO, "Failed to send Seek Event");
				return 0;
			}

			GST_TCDMX_PIPE_MUTEX_LOCK (pPipe);
		}

#ifdef STACK_MORE_THAN_NEEDED

		//If required data is in adapter, skip filling adapter
		if((pPipe->llEndOffset[p_io->nAdapterIdx] - pPipe->llCurrentOffset[p_io->nAdapterIdx]) < size)
		{
			//If filling is necessary, fill up more than we want
			while ( ((pPipe->llEndOffset[p_io->nAdapterIdx] - pPipe->llCurrentOffset[p_io->nAdapterIdx]) < size + GST_TC_MAX_IO_POOL_SIZE) && 
					!pPipe->bEos) {
				GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO,
								   "Start : %lld, End : %lld, Cur : %lld",
									pPipe->llStartOffset[p_io->nAdapterIdx],
									pPipe->llEndOffset[p_io->nAdapterIdx],
									pPipe->llCurrentOffset[p_io->nAdapterIdx]);
				pPipe->ullRequred = size + GST_TC_MAX_IO_POOL_SIZE;
				GST_TCDMX_PIPE_SIGNAL (pPipe);
				GST_TCDMX_PIPE_WAIT (pPipe);
			}
		}

#else//STACK_MORE_THAN_NEEDED

		while ( ((pPipe->llEndOffset[p_io->nAdapterIdx] - pPipe->llCurrentOffset[p_io->nAdapterIdx]) < size) && 
				(pPipe->bEos == 0)) {
			GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO,
							   "Start : %lld, End : %lld, Cur : %lld",
								pPipe->llStartOffset[p_io->nAdapterIdx],
								pPipe->llEndOffset[p_io->nAdapterIdx],
								pPipe->llCurrentOffset[p_io->nAdapterIdx]);
			pPipe->ullRequred = (long long)size;
			GST_TCDMX_PIPE_SIGNAL (pPipe);
			GST_TCDMX_PIPE_WAIT (pPipe);
		}

#endif//STACK_MORE_THAN_NEEDED

		available = pPipe->llEndOffset[p_io->nAdapterIdx] - pPipe->llCurrentOffset[p_io->nAdapterIdx];
		size = MIN (available, size);
		GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "[IDX:%d][Avail : %d ( %lld - %lld )][size : %d]", 
							  p_io->nAdapterIdx, available, pPipe->llEndOffset[p_io->nAdapterIdx], pPipe->llCurrentOffset[p_io->nAdapterIdx], size);
		if (size > 0) {
			gsize adapter_offset = (gsize)(pPipe->llCurrentOffset[p_io->nAdapterIdx] - pPipe->llStartOffset[p_io->nAdapterIdx]);
			gst_adapter_copy (pPipe->pstAdapter[p_io->nAdapterIdx], 
							  (gpointer)pBuffer, 
							  adapter_offset, 
							  (gsize)size);
			pPipe->llCurrentOffset[p_io->nAdapterIdx] += (gint64)size;
			pPipe->ullRequred = 0;
			p_io->llCurrentPos = pPipe->llCurrentOffset[p_io->nAdapterIdx];
			GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "Just got %d bytes from adapter", size);
		}
		//Flush beginning of adapter to maintain the size of buffer in adapter
		if(pPipe->llEndOffset[p_io->nAdapterIdx] - pPipe->llStartOffset[p_io->nAdapterIdx] > GST_TC_MAX_IO_BUFFERING_SIZE)
		{
			gint64 flush_size = (pPipe->llEndOffset[p_io->nAdapterIdx] - pPipe->llStartOffset[p_io->nAdapterIdx]) - GST_TC_MAX_IO_BUFFERING_SIZE;
			gst_adapter_flush(pPipe->pstAdapter[p_io->nAdapterIdx], (gsize)flush_size);
			pPipe->llStartOffset[p_io->nAdapterIdx] += flush_size;
			GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "Flushing %ld bytes from adapter", flush_size);
		}
		GST_TCDMX_PIPE_MUTEX_UNLOCK (pPipe);
		return size;
	}
}

gint32 
tcdmx_io_seek(void *pstIO, long lOffset, gint32 lOrigin)
{
	gint64	new_pos = (gint64)lOffset;

	GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "[offset: %ld][origin: %d]", lOffset, lOrigin);

	return tcdmx_io_seek64(pstIO, new_pos, lOrigin);
}

gint32 
tcdmx_io_seek64(void *pstIO, gint64 llOffset, gint32 lOrigin)
{
	GstTcDemuxIo *p_io = GST_TC_DEMUXIO_CAST (pstIO);
	gint64	new_pos;
	gint32 ret = -1;

	GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "[offset: %"G_GINT64_FORMAT"][origin: %d]", llOffset, lOrigin);

	if(p_io->llFileSize != 0)
	{
		switch (lOrigin)
		{
		case 0:
			new_pos = llOffset;
			if ((llOffset >= 0) && (llOffset <= p_io->llFileSize)){
				ret = 0;
			}
			break;

		case 1: 
			new_pos = p_io->llCurrentPos + llOffset;
			if ((new_pos >= 0) && (new_pos <= p_io->llFileSize)){
				ret = 0;
			}
			break;

		case 2: 
			new_pos = p_io->llFileSize + llOffset;
			if ((new_pos >= 0) && (new_pos <= p_io->llFileSize)){
				ret = 0;
			}
			break;

		default:
			ret = -1;
			break;
		}
	}
	else
	{
		switch (lOrigin)
		{
		case 0:
			new_pos = llOffset;
			if (llOffset >= 0){
				ret = 0;
			}
			break;

		case 1: 
			new_pos = p_io->llCurrentPos + llOffset;
			if (new_pos >= 0){
				ret = 0;
			}
			break;

		case 2: 
			ret = -1;
			break;

		default:
			break;
		}
	}

	if(ret == -1)
	{
		GST_CAT_ERROR_OBJECT (GST_CAT_DMXIO, pstIO, "[offset: %"G_GINT64_FORMAT"][origin: %d][filesize: %"G_GINT64_FORMAT"]", llOffset, lOrigin, p_io->llFileSize);
	}
	else
	{
		//Update current position, this is not a real read point
		p_io->llCurrentPos = new_pos;
	}
	return ret;
}

long
tcdmx_io_tell (void *pstIO)
{
	GstTcDemuxIo *p_io = GST_TC_DEMUXIO_CAST(pstIO);
	GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "pos: %d (%d MB)", (gint32)p_io->llCurrentPos, (gint32)(p_io->llCurrentPos/1048576));
	return (gint32)(p_io->llCurrentPos);
}

gint64 
tcdmx_io_tell64 (void *pstIO)
{
	GstTcDemuxIo *p_io = GST_TC_DEMUXIO_CAST(pstIO);
	GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "pos: %"G_GINT64_FORMAT" (%d MB)", p_io->llCurrentPos, (gint32)(p_io->llCurrentPos/1048576));
	return p_io->llCurrentPos;
}

gint32 
tcdmx_io_close (void *pstIO)
{
	GstTcDemuxIo *p_io = GST_TC_DEMUXIO_CAST(pstIO);
	GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "close");

	if (p_io->nAdapterIdx != -1){
		//Return adapter
		p_io->pstDemux->pstPipe.llInUse[p_io->nAdapterIdx] = 0;
	}
#if USE_MALLOC_INSTEAD_OF_SLIZE
	free(p_io);
#else
	g_slice_free1(sizeof(GstTcDemuxIo), p_io);
#endif
	return 0;
}

unsigned long
tcdmx_io_eof (void *pstIO)
{
	GstTcDemuxIo *p_io = GST_TC_DEMUXIO_CAST(pstIO);
	unsigned long ret;
	
	if(p_io->llFileSize != 0)
	{
		GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "eof: %d", (p_io->llCurrentPos >= p_io->llFileSize));
		ret = (p_io->llCurrentPos >= p_io->llFileSize) ? 1u : 0u;
	}
	else
	{
		GST_CAT_DEBUG_OBJECT (GST_CAT_DMXIO, pstIO, "eof: %d", p_io->pstDemux->pstPipe.bEos);
		ret = (unsigned long)p_io->pstDemux->pstPipe.bEos;
	}
	return ret;
}

unsigned long
tcdmx_io_flush (void *pstIO)
{
	GST_TRACE("%p", pstIO);
	return 0;
}

unsigned long
tcdmx_io_write (const void *pBuffer, guint32 ulSize, guint32 ulCount, void *pstIO)
{
	GST_TRACE("%p, %d, %d, %p", pBuffer, ulSize, ulCount, pstIO);
	return 0;
}

gint32 
tcdmx_io_unlink (const char *pstIO)
{
	GST_TRACE("%p", pstIO);
	return 0;
}

