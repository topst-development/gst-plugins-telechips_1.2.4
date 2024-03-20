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

#ifndef GST_TC_DEMUX_BASE_H__
#define GST_TC_DEMUX_BASE_H__

#include <dlfcn.h>
#include <gst/gst.h>
#include <gst/base/gstadapter.h>
#include <gst/base/gstdataqueue.h>
#include "gst_tc_mediatype.h"
#include <string.h>


#define SUPPORT_PUSH_MODE			(1)
#define SUPPORT_BACKWARD_FASTPLAY	(0)

#define TCDMX_FALSE	(gboolean)(0)
#define TCDMX_TRUE	(gboolean)(1)

G_BEGIN_DECLS

GST_DEBUG_CATEGORY_EXTERN (gst_tc_demux_debug);
#define GST_CAT_DEFAULT (gst_tc_demux_debug)

/* #defines don't like whitespacey bits */
#define GST_TYPE_DMXBASE             (gst_tcdmx_get_type())
#define GST_DMXBASE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DMXBASE,GstTcDemuxBase))
#define GST_DMXBASE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DMXBASE,GstTcDemuxBaseClass))
#define GST_IS_DMXBASE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DMXBASE))
#define GST_IS_DMXBASE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DMXBASE))
#define GST_DMXBASE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), GST_TYPE_DMXBASE, GstTcDemuxBaseClass))

typedef struct _GstTcDemuxBase      GstTcDemuxBase;
typedef struct _GstTcDemuxBaseClass GstTcDemuxBaseClass;
typedef struct _GstTcDemuxPipe		GstTcDemuxPipe;

#define SET_FLAG(val, flag)    ((val) |= (guint)(flag))
#define CLEAR_FLAG(val, flag)  ((val) &= (~(guint)flag))
#define CHECK_FLAG(val, flag)  (((guint)(val) & (guint)(flag)) == ((guint)(flag)))
//#define CHECK_FLAG(val, flag)  ((val & flag) == flag)

#define GST_TCDMX_PIPE_MUTEX_LOCK(m) {G_STMT_START {                    \
  GST_LOG_OBJECT (m, "locking tlock from thread %p", g_thread_self ()); \
  if (m->pstLock != 0) {g_mutex_lock (m->pstLock);}                                              \
  GST_LOG_OBJECT (m, "locked tlock from thread %p", g_thread_self ());  \
} G_STMT_END;}

#define GST_TCDMX_PIPE_MUTEX_UNLOCK(m) {G_STMT_START {                    \
  GST_LOG_OBJECT (m, "unlocking tlock from thread %p", g_thread_self ()); \
  if (m->pstLock != 0) {g_mutex_unlock (m->pstLock);}                                              \
} G_STMT_END;}

#define GST_TCDMX_PIPE_WAIT(m) {G_STMT_START {                          \
  GST_LOG_OBJECT (m, "thread %p waiting", g_thread_self ());            \
  if ((m->pstCond != 0) && (m->pstLock != 0)) {g_cond_wait (m->pstCond, m->pstLock);}                                      \
} G_STMT_END;}

#define GST_TCDMX_PIPE_SIGNAL(m) {G_STMT_START {                        \
  GST_LOG_OBJECT (m, "signalling from thread %p", g_thread_self ());    \
  if (m->pstCond != 0) {g_cond_signal (m->pstCond);}                                              \
} G_STMT_END;}

#define STREAM_MAX	(25 * (gint)TCDMX_TYPE_TYPE_MAX)

#define TEMPLATE_NAME_AUDIO_PAD		 ("audio_%02d")
#define TEMPLATE_NAME_VIDEO_PAD		 ("video_%02d")
#define TEMPLATE_NAME_SUBTITLE_PAD	 ("subtitle_%02d")
#define TEMPLATE_NAME_PRIVATE_PAD	 ("private_%02d")

/* basic stream types */
typedef enum tcdmx_stream_type_t {
	TCDMX_TYPE_ANY   = -1,
	TCDMX_TYPE_VIDEO = 1,
	TCDMX_TYPE_AUDIO = 2,
	TCDMX_TYPE_SUBTITLE = 3,
	TCDMX_TYPE_PRIVATE = 4,
	TCDMX_TYPE_TYPE_MAX = 5
} tcdmx_stream_type_t;


/* demuxer states */
typedef enum tcdmx_state_t {
	TCDMX_STATE_CLOSED,
	TCDMX_STATE_OPENED,
	TCDMX_STATE_DEMUXING,
	TCDMX_STATE_ERROR,
} tcdmx_state_t;


/* flags about base process (GstTcDemuxBase internal flags) */
#define TCDMX_BASE_FLAG_STREAMING_SOURCE          (0x00000001U)  // (1<<0 )
#define TCDMX_BASE_FLAG_TIMESEEK_AVAILABLE        (0x00000002U)  //(1<<1 ) //src seekable flag
#define TCDMX_BASE_FLAG_BYTESEEK_AVAILABLE        (0x00000004U)  //(1<<2 ) //src seekable flag
#define TCDMX_BASE_FLAG_SEEK_ENABLE               (0x00000008U)  //(1<<3 ) //demuxer seekable
#define TCDMX_BASE_FLAG_TIMESEEK_ENABLE           (0x00000010U)  //(1<<4 )
#define TCDMX_BASE_FLAG_SELECTIVE_DEMUXING        (0x00000020U)  //(1<<5 )
#define TCDMX_BASE_FLAG_SEQUENTIAL_DEMUXING       (0x00000040U)  //(1<<6 )
#define TCDMX_BASE_FLAG_FIXED_DEMUXING_MODE       (0x00000080U)  //(1<<7 )
#define TCDMX_BASE_FLAG_MULTIPLE_STEAM            (0x00000100U)  //(1<<8 )
#define TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL   (0x00000200U)  //(1<<9 ) //go to the last sync-frame if seeking is failed
#define TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL     (0x00000400U)  //(1<<10) //go to the end of stream if seeking is failed
#define TCDMX_BASE_FLAG_SEEKING                   (0x00000800U)  //(1<<11)
#define TCDMX_BASE_FLAG_SEEK_DONE                 (0x00001000U)  //(1<<12)
#define TCDMX_BASE_FLAG_EOS                       (0x00002000U)  //(1<<13)
#define TCDMX_BASE_FLAG_DEMUX_OPENED              (0x00004000U)  //(1<<14)
#define TCDMX_BASE_FLAG_RINGMODE_ENABLE           (0x00008000U)  //(1<<15)
 
#define TCDMX_BASE_FLAG_DISABLE_VIDEO             (0x10000000u) //(1<<28)
#define TCDMX_BASE_FLAG_DISABLE_AUDIO             (0x20000000u) //(1<<29)
#define TCDMX_BASE_FLAG_DISABLE_SUBTITLE          (0x40000000u) //(1<<30)
#define TCDMX_BASE_FLAG_DISABLE_PRIVATE           (0x80000000u) //(1<<31)


/* flags about stream state */
#define STREAM_FLAG_EXPOSED         	          (0x00000001U) //(1<<0 )
#define STREAM_FLAG_DISCONTINUE			  (0x00000002U) //(1<<1 )
#define STREAM_FLAG_END_OF_STREAM		  (0x00000004U) //(1<<2 )
#define STREAM_FLAG_WAIT_SKIMMING_FRAME	        (0x00000008U) //(1<<3 )
#define STREAM_FLAG_SEGMENT_CONSUMED	          (0x00000010U) //(1<<4 )

/* seek option flags (virtual method input) */
#define TCDMX_SEEK_FLAG_NEAREST_KEY	              (1)
#define TCDMX_SEEK_FLAG_ACCURATE	                (2)

/* stream format values */
#define TCDMX_STREAM_FORMAT_BYTE_STREAM           (1)
#define TCDMX_STREAM_FORMAT_ADTS                  (2)
#define TCDMX_STREAM_FORMAT_LATM                  (3)
#define TCDMX_STREAM_FORMAT_LOAS                  (4)
#define TCDMX_STREAM_FORMAT_ADIF                  (5)
#define TCDMX_STREAM_FORMAT_AVC                   (6)

/* stream format values */
#define TCDMX_STREAM_ALIGNMENT_NONE               (1)
#define TCDMX_STREAM_ALIGNMENT_FRAME              (2)
#define TCDMX_STREAM_ALIGNMENT_AU                 (3)
#define TCDMX_STREAM_ALIGNMENT_NAL                (4)

/* GetParam Index*/
#define TCDEMUX_PARAM_PUSHMODE_AVAILABLE          (0x00000001)
#define TCDEMUX_PARAM_QUERY_CONTAINER_NAME        (0x00000002)

/* Max number of adapter */
#define TCDMX_MAX_SRC_HANDLER (8)

/* flags about skimming */
#define TCDMX_SKIMMING_DEFAULT_THRESHOLD				(2.0)
#define TCDMX_SKIMMING_DEFAULT_SEEKINTERVAL				(500)
#define TCDMX_SKIMMING_QOS_THRESHOLD					(5)

#define TCDMX_SKIMMING_MASK_OPTION						(0x0000000F)
#define TCDMX_SKIMMING_OPTION_NO_OPTION				(0x00000000)
#define TCDMX_SKIMMING_OPTION_NO_AUDIO				(0x00000001)
#define TCDMX_SKIMMING_OPTION_ADAPTIVE_MODE_CHANGE		(0x00000002)

#define TCDMX_SKIMMING_MASK_MODE				(0x000000F0)
#define TCDMX_SKIMMING_MODE_NORMAL				(0x00000000)
#define TCDMX_SKIMMING_MODE_SKIMMING_FORWARD			(0x00000010)
#define TCDMX_SKIMMING_MODE_SKIMMING_BACKWARD			(0x00000020)
#define TCDMX_SKIMMING_MODE_FASTPLAY_FORWARD			(0x00000040)//Actually nothing to do
#define TCDMX_SKIMMING_MODE_FASTPLAY_BACKWARD			(0x00000080)

#define TCDMX_SKIMMING_FASTPLAY_REACHED_BEGINNING		(0x00000100)
#define TCDMX_SKIMMING_FASTPLAY_SEGMENT_CONSUMED		(0x00000200)


/* sequence header searching */
#define MPEG4_VOL_STARTCODE_MIN				(0x00000120)      // MPEG-4
#define MPEG4_VOL_STARTCODE_MAX				(0x0000012F)      // MPEG-4
#define MPEG4_VOP_STARTCODE						(0x000001B6)      // MPEG-4
#define MPEG12_SEQHEAD_STARTCODE			(0x000001B3)      // MPEG-1/2
#define MPEG12_EXTENSION_STARTCODE		(0x000001B5)      // MPEG-1/2
#define VC1AP_SEQHEAD_STARTCODE				(0x0000010F)      // VC-1
#define AVS_SEQHEAD_STARTCODE					(0x000001B0)      // AVS
#define MAX_SEQ_HEADER_ALLOC_SIZE			(0x0007D000)		// 500KB
#define TCDMX_MAX_SEQ_SEARCH_TIME			(300)


/* Scheduling between dmx thread and main thread*/
struct _GstTcDemuxPipe
{
	GMutex					*pstLock;
	GCond					*pstCond;
	gboolean				 bEos;
	GstFlowReturn			 enSrcResult;
	GstAdapter				*pstAdapter[TCDMX_MAX_SRC_HANDLER];
	gint64					 llStartOffset[TCDMX_MAX_SRC_HANDLER];
	gint64					 llEndOffset[TCDMX_MAX_SRC_HANDLER];
	gint64					 llCurrentOffset[TCDMX_MAX_SRC_HANDLER];
	gint64					 llInUse[TCDMX_MAX_SRC_HANDLER];
	gint32					 lCurrentIOIdx;
	gint64					 ullRequred;
	guint64					 ullOffset;
};

/* stream info structure */
typedef struct tcdmx_stream_t {
	GstPad        * pstPad;
	guint32         ulStreamID;
	guint32         ulStreamType;
	guint32         ulFlags;
	gpointer        pPrivate;
	GstFlowReturn   enLastFlow;
	gint64          llLastTimestamp;
	gint64          llFrameDuration;
	gint64          llSkimmingFramePTS;

	//Anchor PTS for backward fastplay
	gint64          llCurrentAnchorPTS;
	gint64          llPreviousAnchorPTS;
	union {
		tc_audio_info_t	    stAudio;
		tc_video_info_t	    stVideo;
		tc_subtitle_info_t  stSubtitle;
		tc_private_info_t   stPrivate;
	} unInfo;
	GstBuffer     * pstCodecData;
	guint32         ulFormat;
	guint32         ulAlignment;
} tcdmx_stream_t;

/* demux result type */
typedef struct tcdmx_result_t {
	guint32         ulStreamID;
	guint32         ulStreamType;
	gpointer        pBuffer;
	gint32          lLength;
	gint32          lTimestampMs;
	gint32          lEndTimestampMs;
} tcdmx_result_t;


struct _GstTcDemuxBase
{
	GstElement element;

	GstPad          *pstSinkPad;

	tcdmx_stream_t   astStream[STREAM_MAX];
	gint32           lStreamCount;
	gint32           acTypeCount[TCDMX_TYPE_TYPE_MAX]; // count for each stream types
	gint32           acIndexMap[STREAM_MAX]; // stream type / stream id to index
	gint32           lEosCount;

	tcdmx_state_t	 enState;
	tcdmx_result_t   stDmxResult;
	guint32          ulFlags;
	gint64           llStartOffset;
	GMutex 			*pstDemuxLock;

	GstEvent        *pstCloseSegEvent;
	GstEvent        *pstNewSegEvent;

	/* segment in TIME */
	GstSegment       stSegment;
	gboolean 	     bSegmentRunning;
	gboolean		 bSeekDone;

	/* last I/O result */
	GstFlowReturn    enIoResult;
	gint64			 llLastReadPos;
	gint32			 lLastReadSize;
	gint64			 llFileSize;

	/* for Chain function*/
	gboolean		 bPushMode;
	gboolean		 bInitDone;
	GstTcDemuxPipe	 pstPipe;
	guint32			 ullAdapterNum;
	GstTask 		*pstTask;
	GMutex 			*pstTaskLock;

	//This contains video-codec name, audio-codec name, and formatter name
	GstTagList		*pstTagList;

	//Operation mode based on skimming
	guint32			 ulSkimmingFlags;
	guint32			 ulAccumQOS;
	gdouble			 fSkimmingThreshold;
	gint64			 lSkimmingInterval;

	//Stream Start event creation
	gboolean		 bHaveGroupId;
	guint32			 ulGroupId;

	//Attach codec_data at first video segment
	gboolean 		 bNeed_to_attach_codec_data;
	gboolean 		 bEnableRingMode;

	guint32			 ulTotalAudioNum;
	guint32			 request_id;
	guint32          ulDuration_ms;
	guint32			 ulSeeked_ms;
};

typedef struct vorbis_header_info_t
{
	guint8 *pData1;
	gint32 size1;
	guint8 *pData2;
	gint32 size2;
	guint8 *pData3;
	gint32 size3;
} vorbis_header_info_t;

struct _GstTcDemuxBaseClass 
{
	GstElementClass parent_class;

	GstFlowReturn (*vmOpen)    (GstTcDemuxBase *pstDmx, guint32 ulFlags);
	gboolean (*vmSetInfo) (GstTcDemuxBase *pstDmx);
	gboolean (*vmDemux)   (GstTcDemuxBase *pstDmx, guint32 ulRequestType, tcdmx_result_t * pstResult);
	gboolean (*vmSeek)    (GstTcDemuxBase *pstDmx, guint32 ulFlags, gint32 lTargetPts, gint32 *plResultPts);
	gboolean (*vmReset)   (GstTcDemuxBase *pstDmx);
	gboolean (*vmClose)   (GstTcDemuxBase *pstDmx);
	gboolean (*vmGetParam)(GstTcDemuxBase *pstDmx, gint param, void * ret);
};


GType gst_tcdmx_get_type (void);

typedef gint (*DMX_FUNC)( gint iOpCode, gulong* pHandle, void* pParam1, void* pParam2 );

void
gst_tcdmx_set_class_details (
	GstTcDemuxBaseClass * pstDemuxClass,
	const gchar               * pszDemuxLongName,
	const gchar               * pszDemuxDesc
	);

gboolean
gst_tcdmx_create_sinkpad_templates (
	GstTcDemuxBaseClass         * pstDemuxClass,
	const GstStaticPadTemplate  * pstPadTemplate
	);

gboolean
gst_tcdmx_create_srcpad_templates (
	GstTcDemuxBaseClass  * pstDemuxClass,
	GstCaps              * pstVideoCaps,
	GstCaps              * pstAudioCaps,
	GstCaps              * pstSubtitleCaps,
	GstCaps              * pstPrivateCaps
	);

gboolean
gst_tcdmx_add_sinkpad (
	GstTcDemuxBase              * pstDemux,
	const GstStaticPadTemplate  * pstPadTemplate
	);


#define TCDMX_MODE_SELECTIVE        (0x01u)	//default
#define TCDMX_MODE_SEQUENTIAL       (0x02u)
#define TCDMX_MODE_FIXED            (0x10u)
#define TCDMX_MODE_SELECTIVE_ONLY   (TCDMX_MODE_FIXED|TCDMX_MODE_SELECTIVE)
#define TCDMX_MODE_SEQUENTIAL_ONLY  (TCDMX_MODE_FIXED|TCDMX_MODE_SEQUENTIAL)

void
gst_tcdmx_set_default_demux_mode (
	GstTcDemuxBase  * pstDemux,
	guint32           ulMode
	);

void
gst_tcdmx_set_ringmode (
	GstTcDemuxBase  * pstDemux,
	gboolean 		  bEnable
	);

void
gst_tcdmx_set_seekable (
	GstTcDemuxBase  * pstDemux,
	gboolean          bSeekable
	);

void
gst_tcdmx_set_timeseek (
	GstTcDemuxBase  * pstDemux,
	gboolean          bTimeSeek
	);

gboolean
gst_tcdmx_register_stream_info (
	GstTcDemuxBase  * pstDemux,
	guint32           ulStreamID,
	guint32           ulStreamType,
	gpointer          pStreamInfo,
	gpointer          pCodecData,
	gint32            lCodecDataLen
	);

gboolean
gst_tcdmx_register_stream_info2 (
	GstTcDemuxBase  * pstDemux,
	guint32           ulStreamID,
	guint32           ulStreamType,
	gpointer          pStreamInfo,
	gpointer          pCodecData,
	gint32            lCodecDataLen,
	guint32           ulFormat,
	guint32           ulAlignment
	);

void
gst_tcdmx_search_vorbis_header (
	guchar *pData,
	guint32 size,
	vorbis_header_info_t *pHeader
	);

void
gst_tcdmx_set_total_audiostream (
	GstTcDemuxBase  * pstDemux,
	guint32           ulStreamNum
	);

G_END_DECLS

#endif /* GST_TC_DEMUX_BASE_H__ */
