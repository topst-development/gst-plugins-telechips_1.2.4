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

/**
 * SECTION:element-tc-flvdemux
 *
 * FIXME:Describe tc-flvdemux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tc-flvdemux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_flvdemux.h"
#include "gst_tc_demuxio.h"


/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE          ("video/x-flv")
#define STRING_DEMUX_LONGNAME       ("Telechips FLV Demuxer")
#define STRING_DEMUX_DESCRIPTION    ("Demultiplex FLV into element streams")


static GstStaticPadTemplate gs_stSinkFactory = GST_STATIC_PAD_TEMPLATE (
	"sink",									//template-name
	GST_PAD_SINK,							//direction
	GST_PAD_ALWAYS,							//presence
	GST_STATIC_CAPS (CONTAINER_MIMETYPE)	//caps
    );

static const guint16 gs_usSupportAudioTags[] = {
	WAVE_FORMAT_MPEGL3,
	WAVE_FORMAT_AAC,
	0	// end of tags
};

static const guint32 gs_ulSupportVideoFcc[] = {
	FOURCC_FLV1,
	GST_MAKE_FOURCC ('S', 'C', 'R', '1'),
	GST_MAKE_FOURCC ('S', 'C', 'R', '2'),
	FOURCC_VP60,
	GST_MAKE_FOURCC ('V', 'P', '6', 'A'),
	FOURCC_H264,
	0	// end of fccs
};

G_DEFINE_TYPE (GstTcFlvDemux, gst_flvdmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

/* GObject virtual method implementations */
static void gst_flvdmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_flvdmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_flvdmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_flvdmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestID, tcdmx_result_t * pstResult);
static gboolean gst_flvdmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_flvdmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_flvdmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_flvdmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);

/* GstTcFlvDemux private implementations */



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//
/* initialize the tc-flvdemux's class */
static 
void
gst_flvdmx_class_init (
	GstTcFlvDemuxClass  * pstDemuxClass
	)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxClass;
	GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("");

	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overiding
	p_object_class->finalize = gst_flvdmx_finalize;

	// base class overiding
	p_dmxbase_class->vmOpen    = gst_flvdmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_flvdmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_flvdmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_flvdmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_flvdmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_flvdmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_flvdmx_vm_getparam;

	// set demux class details
	gst_tcdmx_set_class_details (GST_DMXBASE_CLASS(pstDemuxClass)
		, STRING_DEMUX_LONGNAME
		, STRING_DEMUX_DESCRIPTION
		);

	// sink pad template
	(void)gst_tcdmx_create_sinkpad_templates (GST_DMXBASE_CLASS(pstDemuxClass), &gs_stSinkFactory);

	// src pad templates
	(void)gst_tcdmx_create_srcpad_templates ( GST_DMXBASE_CLASS (pstDemuxClass)
		, gst_tc_create_video_template_caps (gs_ulSupportVideoFcc)
		, gst_tc_create_audio_template_caps (gs_usSupportAudioTags)
		, NULL
		, NULL
		);
}


/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static 
void
gst_flvdmx_init (
	GstTcFlvDemux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	(void)gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), TCDMX_MODE_SELECTIVE);
}


static 
void
gst_flvdmx_finalize (
	GObject  * pstObject
	)
{
  GstTcFlvDemux *p_dmx = GST_FLVDMX (pstObject);

  GST_TRACE ("");

  //TODO: finialize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define FLV_EXT_LIB_NAME ("libtccflvdmx.so")

static gint gst_flvdmx_load_library(GstTcFlvDemux *p_flvdmx)
{
  gint result = 0;
  p_flvdmx->pExtDLLModule = dlopen(FLV_EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if( p_flvdmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_flvdmx,"[FLVDMX] Load library '%s' failed: %s", FLV_EXT_LIB_NAME, dlerror());
    result = -1;
  } else {
    GST_DEBUG_OBJECT(p_flvdmx,"[FLVDMX] Library '%s' Loaded", FLV_EXT_LIB_NAME);

    p_flvdmx->gExtDmxFunc = dlsym(p_flvdmx->pExtDLLModule, "TCC_FLV_DMX");
    if( p_flvdmx->gExtDmxFunc == NULL ) {
      GST_ERROR_OBJECT(p_flvdmx,"[FLVDMX] p_flvdmx->gExtDmxFunc Error");
      result = -1;
    }
  }

  return result;
}

static void gst_flvdmx_close_library(GstTcFlvDemux *p_flvdmx)
{
  if( p_flvdmx->pExtDLLModule != NULL){
    (void)dlclose(p_flvdmx->pExtDLLModule);
  }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstElement virtual method implementations
//


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstPad virtual method implementations
//


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstTcDemuxBase virtual method implementations
//
static
GstFlowReturn
gst_flvdmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GstTcFlvDemux *p_flvdmx = GST_FLVDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx;
	av_dmx_result_t res;
	flv_dmx_init_t init_param;
	flv_dmx_info_t *p_init_info = &p_flvdmx->stInitInfo;
	GstFlowReturn ret;

	av_memory_func_t mem_func = { 
		g_malloc, 
		g_free, 
		memcmp, 
		memcpy, 
		memmove, 
		memset, 
		g_realloc 
		};

	av_file_func_t file_func = {
		tcdmx_io_open, 
		tcdmx_io_read, 
		tcdmx_io_seek, 
		tcdmx_io_tell, 
		tcdmx_io_close, 
		tcdmx_io_eof, 
		tcdmx_io_flush, 
		NULL, 
		NULL, 
		tcdmx_io_seek64, 
		tcdmx_io_tell64
		};

	if (gst_flvdmx_load_library(p_flvdmx) == 0) {

		(void)memset ( &init_param, 0, sizeof(flv_dmx_init_t) );

		init_param.m_stMemoryFuncs			= mem_func;
		init_param.m_stFileFuncs			= file_func;
		init_param.m_pszOpenFileName		= (char*)(void*)pstDemuxBase->pstSinkPad;
		//init_param.m_ulOption				= 0;
		//init_param.m_ulFIOBlockSizeNormal	= 0;
		//init_param.m_ulFIOBlockSizeSeek		= 0;

		//demuxing mode
		if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING))){
			init_param.m_ulOption |= (guint)FLVOPT_SELECTIVE;
		}

		//seek fail handling scenario
		if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL))){
			p_flvdmx->ulSeekFlags = DMXSEEK_GO_LSYNC_IF_EOS;
		} else if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL))) {
			p_flvdmx->ulSeekFlags = DMXSEEK_GO_EOS_IF_EOS;
		} else {
			p_flvdmx->ulSeekFlags = 0;
		}

		res = p_flvdmx->gExtDmxFunc(DMXOP_OPEN, &h_dmx, &init_param, p_init_info);
		if ( res != DMXRET_SUCCESS ) {
			res = p_flvdmx->gExtDmxFunc(DMXOP_GET_LASTERROR, NULL, NULL, NULL);
			GST_ERROR_OBJECT (p_flvdmx, "flv open failed - (error: %d)", res);
			ret = GST_FLOW_NOT_SUPPORTED;
		} else {
			p_flvdmx->hFlvDmx = h_dmx;

		 	if (( !CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_STREAMING_SOURCE) ) ||
			    ( p_init_info->m_pstFileInfo->m_ulSeekMode == (guint)SEEK_BY_META_INDEX )) { 
				gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);
				p_flvdmx->bSeekEnable = TCDMX_TRUE;
			} else {
				gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_FALSE);
				p_flvdmx->bSeekEnable = TCDMX_FALSE;
			}
	
			if (p_init_info->m_ulAudioStreamTotal > 0U) {
				gst_tcdmx_set_total_audiostream(pstDemuxBase, 1);
			}
			ret = GST_FLOW_OK;
		}
	} else {
		ret = GST_FLOW_NOT_SUPPORTED;
	}
	return ret;
}

static
gboolean
gst_flvdmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcFlvDemux *p_flvdmx = GST_FLVDMX((pstDemuxBase));
	av_dmx_handle_t	h_dmx = p_flvdmx->hFlvDmx;

	flv_dmx_info_t *p_init_info = &p_flvdmx->stInitInfo;

	if (p_init_info->m_ulVideoStreamTotal > 0U) {
		flv_dmx_video_info_t *p_info = p_init_info->m_pstDefaultVideoInfo;
		tc_video_info_t info;
		info.ulFourCC       = p_info->m_ulFourCC;
		info.ulWidth        = p_info->m_ulWidth;
		info.ulHeight       = p_info->m_ulHeight;
		info.ulBitPerPixel  = 0;
		info.ulFrameRate    = p_info->m_ulFrameRate;
		info.ulDuration     = p_init_info->m_pstFileInfo->m_ulDuration;

		(void)gst_tcdmx_register_stream_info (pstDemuxBase, 
										(gboolean)TCDMX_TYPE_VIDEO, 
										(gboolean)TCDMX_TYPE_VIDEO, 
										&info, 
										p_info->m_pbyCodecPrivate, 
										(gint32)p_info->m_ulCodecPrivateSize);
	}

	if (p_init_info->m_ulAudioStreamTotal > 0U) {
		flv_dmx_audio_info_t *p_info = p_init_info->m_pstDefaultAudioInfo;
		tc_audio_info_t info;
		memset(&info, 0, sizeof(tc_audio_info_t));
		info.ulFormatTag     = p_info->m_ulFormatTag;
		info.ulChannels      = p_info->m_ulChannels;
		info.ulBlockAlign    = 0;
		info.ulBitPerSample  = p_info->m_ulBitsPerSample;
		info.ulSampleRate    = p_info->m_ulSamplePerSec;
		info.ulSize          = 0;
		info.ulDuration      = p_init_info->m_pstFileInfo->m_ulDuration;

		
		if (info.ulFormatTag == (guint)WAVE_FORMAT_PCM){
			info.ulEndian		= 0;		//LittleEndian or Don't Care
		}

		(void)gst_tcdmx_register_stream_info (pstDemuxBase, 
										(gboolean)TCDMX_TYPE_AUDIO, 
										(gboolean)TCDMX_TYPE_AUDIO, 
										&info, 
										p_info->m_pbyCodecPrivate, 
										(gint32)p_info->m_ulCodecPrivateSize);
	}

	return TCDMX_TRUE;
}

static
gboolean
gst_flvdmx_vm_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestID,
	tcdmx_result_t  * pstResult
	)
{
	GstTcFlvDemux *p_flvdmx = GST_FLVDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_flvdmx->hFlvDmx;
	av_dmx_result_t res;
	av_dmx_getstream_t get_param; 
	gboolean ret = TCDMX_TRUE;

	if( ulRequestID == (guint)TCDMX_TYPE_VIDEO ) {
		get_param.s32StreamType = DMXTYPE_VIDEO;
	} else if (ulRequestID == (guint)TCDMX_TYPE_AUDIO ) {
		get_param.s32StreamType = DMXTYPE_AUDIO;
	} else if (ulRequestID == (guint)TCDMX_TYPE_SUBTITLE ) {
		get_param.s32StreamType = DMXTYPE_SUBTITLE;
	} else {
		ret = TCDMX_FALSE;
	}

   if (ret != TCDMX_FALSE) {
	av_dmx_outstream_t output;
	res = p_flvdmx->gExtDmxFunc(DMXOP_GET_STREAM, &h_dmx, &get_param, &output);
	if ( res != DMXRET_SUCCESS ) {
		res = p_flvdmx->gExtDmxFunc(DMXOP_GET_LASTERROR, &h_dmx, NULL, NULL);
		GST_ERROR_OBJECT (p_flvdmx, "flv demux failed - (error: %d)", res);
		ret = TCDMX_FALSE;
	} else {
		gint out_streamtype = output.s32StreamType;
		if (out_streamtype == DMXTYPE_VIDEO) {
			pstResult->ulStreamType = (guint)TCDMX_TYPE_VIDEO;
		} else if (out_streamtype == DMXTYPE_AUDIO) {
			pstResult->ulStreamType = (guint)TCDMX_TYPE_AUDIO;
		} else {
			pstResult->ulStreamType = (guint)TCDMX_TYPE_SUBTITLE;
		}

		pstResult->ulStreamID = pstResult->ulStreamType;
		pstResult->pBuffer = output.pbyStreamData;
		pstResult->lLength = output.s32StreamDataSize;
		pstResult->lTimestampMs = output.s32TimeStamp;

		GST_LOG_OBJECT(p_flvdmx,"demux[ID:Type] = [%d:%d] timestamp=%d length=%d",pstResult->ulStreamID,pstResult->ulStreamType,pstResult->lTimestampMs,pstResult->lLength);
	}
   }
	return ret;
}


static
gboolean
gst_flvdmx_vm_seek (
	GstTcDemuxBase  * pstDemuxBase, 
	guint32           ulFlags,
	gint32            lTargetPts, 
	gint32          * plResultPts
	)
{
	GstTcFlvDemux *p_flvdmx = GST_FLVDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_flvdmx->hFlvDmx;
	av_dmx_result_t res;
	av_dmx_seek_t	param;
	av_dmx_seek_t	result;
	gboolean ret;

	if (p_flvdmx->bSeekEnable == TCDMX_FALSE) {
		GST_ERROR_OBJECT (p_flvdmx, "flv seek - not supported");
		ret = TCDMX_FALSE;
	} else {
		param.s32SeekTime  = lTargetPts;
		param.u32SeekMode = p_flvdmx->ulSeekFlags;
	
		res = p_flvdmx->gExtDmxFunc (DMXOP_SEEK, &h_dmx, &param, &result);
		if ( res != DMXRET_SUCCESS ) {
			res = p_flvdmx->gExtDmxFunc(DMXOP_GET_LASTERROR, &h_dmx, NULL, NULL);
			GST_ERROR_OBJECT (p_flvdmx, "flv seek failed - (error: %d)", res);
			ret = TCDMX_FALSE;
		} else {
			*plResultPts = result.s32SeekTime;
			ret = TCDMX_TRUE;
		}
	}
	return ret;
}


static
gboolean
gst_flvdmx_vm_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcFlvDemux *p_flvdmx = GST_FLVDMX(pstDemuxBase);
	gint32 result_pts;

	(void)gst_flvdmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);	//seek to 0ms

	return TCDMX_TRUE;
}

static
gboolean
gst_flvdmx_vm_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcFlvDemux *p_flvdmx = GST_FLVDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_flvdmx->hFlvDmx;
	gboolean ret;
	if ( h_dmx != NULL ) {
		av_dmx_result_t res = p_flvdmx->gExtDmxFunc (DMXOP_CLOSE, &h_dmx, NULL, NULL);
		if( res != DMXRET_SUCCESS ) {
			res = p_flvdmx->gExtDmxFunc(DMXOP_GET_LASTERROR, &h_dmx, NULL, NULL);
			GST_ERROR_OBJECT (p_flvdmx, "flv close failed - (error: %d)", res);
		}

		p_flvdmx->hFlvDmx = 0;
		(void)memset ( &p_flvdmx->stInitInfo, 0, sizeof(flv_dmx_info_t) );

		gst_flvdmx_close_library(p_flvdmx);
		ret = TCDMX_TRUE;
	}
	else {
		GST_ERROR_OBJECT (p_flvdmx, "flv already closed");
		ret = TCDMX_FALSE;
	}
	return ret;
}

static 
gboolean 
gst_flvdmx_vm_getparam (
	GstTcDemuxBase * pstDemuxBase, 
	gint param,
	void * ret
	)
{
	gboolean *ret_val = (gboolean *)ret;
	gchar **str = (gchar **)ret;
	switch( param )
	{
	case TCDEMUX_PARAM_PUSHMODE_AVAILABLE:
		*ret_val = TCDMX_TRUE;
		break;
	case TCDEMUX_PARAM_QUERY_CONTAINER_NAME:
		*str = g_strdup("FLV");
		break;
	default:
		*ret_val = TCDMX_FALSE;
		break;
	}
	return TCDMX_TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstTcFlvDemux private implementations
//
