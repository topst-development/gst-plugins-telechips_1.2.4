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
 * SECTION:element-tc-oggdemux
 *
 * FIXME:Describe tc-oggdemux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tc-oggdemux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_oggdemux.h"
#include "gst_tc_demuxio.h"


/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE		    ("video/ogg; audio/ogg; application/ogg")
#define STRING_DEMUX_LONGNAME     ("Telechips OGG Demuxer")
#define STRING_DEMUX_DESCRIPTION  ("Demultiplex OGG into element streams")


static GstStaticPadTemplate gs_stSinkFactory = GST_STATIC_PAD_TEMPLATE (
	"sink",									//template-name
	GST_PAD_SINK,							//direction
	GST_PAD_ALWAYS,							//presence
	GST_STATIC_CAPS (CONTAINER_MIMETYPE)	//caps
    );

G_DEFINE_TYPE (GstTcOggDemux, gst_oggdmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

/* GObject virtual method implementations */
static void gst_oggdmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_oggdmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_oggdmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_oggdmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestID, tcdmx_result_t * pstResult);
static gboolean gst_oggdmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_oggdmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_oggdmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_oggdmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);


/* GstTcOggDemux private implementations */
#define	LARGE_STREAM_BUF_SIZE	(1024*1024*2)
static guint8	pLargeStramBuff[LARGE_STREAM_BUF_SIZE];

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//


/* initialize the tc-oggdemux's class */
static 
void
gst_oggdmx_class_init (
	GstTcOggDemuxClass  * pstDemuxClass
	)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxClass;
	GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("");

	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overiding
	p_object_class->finalize = gst_oggdmx_finalize;

	// base class overiding
	p_dmxbase_class->vmOpen    = gst_oggdmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_oggdmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_oggdmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_oggdmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_oggdmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_oggdmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_oggdmx_vm_getparam;

	// set demux class details
	gst_tcdmx_set_class_details (GST_DMXBASE_CLASS(pstDemuxClass)
		, STRING_DEMUX_LONGNAME
		, STRING_DEMUX_DESCRIPTION
		);

	// sink pad template
	(void)gst_tcdmx_create_sinkpad_templates (GST_DMXBASE_CLASS(pstDemuxClass), &gs_stSinkFactory);

	// src pad templates
	(void)gst_tcdmx_create_srcpad_templates ( GST_DMXBASE_CLASS (pstDemuxClass)
		, GST_CAPS_ANY
		, GST_CAPS_ANY
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
gst_oggdmx_init (
	GstTcOggDemux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	(void)gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), TCDMX_MODE_SELECTIVE_ONLY);
}


static 
void
gst_oggdmx_finalize (
	GObject  * pstObject
	)
{
  GstTcOggDemux *p_dmx = GST_OGGDMX (pstObject);

  GST_TRACE ("");

  //TODO: finialize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define OGG_EXT_LIB_NAME ("libtccoggdmx.so")

static gint gst_oggdmx_load_library(GstTcOggDemux *p_oggdmx)
{
  gint result = 0;
  p_oggdmx->pExtDLLModule = dlopen(OGG_EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if( p_oggdmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_oggdmx,"[OGGDMX] Load library '%s' failed: %s", OGG_EXT_LIB_NAME, dlerror());
    result = -1;
  } else {
    GST_DEBUG_OBJECT(p_oggdmx,"[OGGDMX] Library '%s' Loaded", OGG_EXT_LIB_NAME);
  
    p_oggdmx->gExtDmxFunc = dlsym(p_oggdmx->pExtDLLModule, "TCC_OGG_DMX");
    if( p_oggdmx->gExtDmxFunc == NULL ) {
      GST_ERROR_OBJECT(p_oggdmx,"[OGGDMX] p_oggdmx->gExtDmxFunc Error");
      result = -1;
    }
  }

  return result;
}

static void gst_oggdmx_close_library(GstTcOggDemux *p_oggdmx)
{
  if( p_oggdmx->pExtDLLModule != NULL){
    (void)dlclose(p_oggdmx->pExtDLLModule);
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
gst_oggdmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GstTcOggDemux *p_oggdmx = GST_OGGDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx;
	av_dmx_result_t res;
	ogg_dmx_init_t init_param;
	ogg_dmx_info_t *p_init_info = &p_oggdmx->stInitInfo;
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

	if (gst_oggdmx_load_library(p_oggdmx) == 0) {

		(void)memset (&init_param, 0, sizeof(ogg_dmx_init_t));
	
		init_param.m_sCallbackFuncMem			= mem_func;
		init_param.m_sCallbackFuncFile			= file_func;
		init_param.m_pOpenFileName       = (char*)(void*)pstDemuxBase->pstSinkPad;
		init_param.m_uiFileCacheSize = 1024*4;
	
		//demuxing mode
		init_param.m_uiOpenOption = OGG_OPENOPT_SELECTIVE;
		p_oggdmx->lFlagMultiDemux = 0;
		if(CHECK_FLAG((pstDemuxBase->ulFlags), (TCDMX_BASE_FLAG_MULTIPLE_STEAM)))
		{
			GST_DEBUG_OBJECT (p_oggdmx, "Multiple stream demuxing enabled");
			init_param.m_uiOpenOption |= (guint32)OGG_OPENOPT_MULTIDEMUX;
			p_oggdmx->lFlagMultiDemux = 1;
		}
	
		//seek fail handling scenario
		if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL))) {
			init_param.m_uiSeekEndOption = 2;
		} else if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL))) {
			init_param.m_uiSeekEndOption = 1;
		} else {
			init_param.m_uiSeekEndOption = 0;
		}
	
		res = p_oggdmx->gExtDmxFunc( DMXOP_OPEN, &h_dmx, &init_param, p_init_info );
	
		if ( res != DMXRET_SUCCESS ) {
			GST_ERROR_OBJECT (p_oggdmx, "ogg open failed - (error: %d)", res);
			ret = GST_FLOW_NOT_SUPPORTED;
		} else {	
			if (p_init_info->m_sAudioInfo.m_iTotalNumber > 0){
				gst_tcdmx_set_total_audiostream(pstDemuxBase, (guint32)p_init_info->m_sAudioInfo.m_iTotalNumber);
			}
			p_oggdmx->hOggDmx = h_dmx;
		
			ret = GST_FLOW_OK;
		}
	} else {
		ret = GST_FLOW_NOT_SUPPORTED;
	}
	return ret;
}

static
gboolean
gst_oggdmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcOggDemux *p_oggdmx = GST_OGGDMX (pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_oggdmx->hOggDmx;

	ogg_dmx_info_t *p_init_info = &p_oggdmx->stInitInfo;

	gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);

	if( p_init_info->m_sVideoInfo.m_iFourCC > 0 ) {
		tc_video_info_t info;
		info.ulFourCC       = (guint32)p_init_info->m_sVideoInfo.m_iFourCC;
		info.ulWidth        = (guint32)p_init_info->m_sVideoInfo.m_iWidth;
		info.ulHeight       = (guint32)p_init_info->m_sVideoInfo.m_iHeight;
		info.ulBitPerPixel  = 0;
		info.ulFrameRate    = (guint32)p_init_info->m_sVideoInfo.m_iFrameRate;
		info.ulDuration     = (guint32)p_init_info->m_sFileInfo.m_iRunningtime;

		(void)gst_tcdmx_register_stream_info (pstDemuxBase, 
										(guint32)p_init_info->m_sVideoInfo.m_iStreamID,	//TCDMX_TYPE_VIDEO, 
										(guint32)TCDMX_TYPE_VIDEO,
										&info, 
										p_init_info->m_sVideoInfo.m_pExtraData, 
										p_init_info->m_sVideoInfo.m_iExtraDataLen);
	}

	if( p_init_info->m_sAudioInfo.m_iTotalNumber > 0) {
		if (p_oggdmx->lFlagMultiDemux != 0) {
			gint i;
			ogg_dmx_audio_info_t *pAudioInfo;
			for(i=0; i<p_init_info->m_sAudioInfo.m_iTotalNumber; i++)
			{
				tc_audio_info_t info;
				memset(&info, 0, sizeof(tc_audio_info_t));
				pAudioInfo = &p_init_info->m_pstAudioInfoList[i];
				info.ulFormatTag     = (guint32)pAudioInfo->m_iFormatId;
				info.ulChannels      = (guint32)pAudioInfo->m_iChannels;
				info.ulBlockAlign    = 0;
				info.ulBitPerSample  = (guint32)pAudioInfo->m_iBitsPerSample;
				info.ulSampleRate    = (guint32)pAudioInfo->m_iSamplePerSec;
				info.ulSize          = 0;
				info.ulDuration      = (guint32)p_init_info->m_sFileInfo.m_iRunningtime;

				if (info.ulFormatTag == (guint32)WAVE_FORMAT_PCM){
					info.ulEndian		= 0;		//LittleEndian or Don't Care
				}

				(void)gst_tcdmx_register_stream_info (pstDemuxBase,
											(guint32)pAudioInfo->m_iStreamID,	//TCDMX_TYPE_AUDIO,
											(gboolean)TCDMX_TYPE_AUDIO,
											&info,
											pAudioInfo->m_pExtraData,
											pAudioInfo->m_iExtraDataLen);
			}
		}
		else {
			tc_audio_info_t info;
			memset(&info, 0, sizeof(tc_audio_info_t));
			info.ulFormatTag     = (guint32)p_init_info->m_sAudioInfo.m_iFormatId;
			info.ulChannels      = (guint32)p_init_info->m_sAudioInfo.m_iChannels;
			info.ulBlockAlign    = 0;
			info.ulBitPerSample  = (guint32)p_init_info->m_sAudioInfo.m_iBitsPerSample;
			info.ulSampleRate    = (guint32)p_init_info->m_sAudioInfo.m_iSamplePerSec;
			info.ulSize          = 0;
			info.ulDuration      = (guint32)p_init_info->m_sFileInfo.m_iRunningtime;

			if (info.ulFormatTag == (guint32)WAVE_FORMAT_PCM){
				info.ulEndian		= 0;		//LittleEndian or Don't Care
			}

			(void)gst_tcdmx_register_stream_info (pstDemuxBase,
										(guint32)p_init_info->m_sAudioInfo.m_iStreamID,	//TCDMX_TYPE_AUDIO,
										(gboolean)TCDMX_TYPE_AUDIO,
										&info,
										p_init_info->m_sAudioInfo.m_pExtraData,
										p_init_info->m_sAudioInfo.m_iExtraDataLen);
		}
	}

	return TCDMX_TRUE;
}

static
gboolean
gst_oggdmx_vm_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestType,
	tcdmx_result_t  * pstResult
	)
{
	GstTcOggDemux *p_oggdmx = GST_OGGDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_oggdmx->hOggDmx;
	av_dmx_result_t res;
	ogg_dmx_input_t get_param;
	gboolean ret;

	if( ulRequestType == (guint32)TCDMX_TYPE_ANY ) {
		GST_DEBUG_OBJECT(p_oggdmx,"not support TCDMX_TYPE_ANY");
		ret = TCDMX_FALSE;
	} else {
		if ( ulRequestType == (guint32)TCDMX_TYPE_VIDEO) {
			get_param.m_ulStreamType = (guint32)DMXTYPE_VIDEO;
			ret = TCDMX_TRUE;
		}
		else if ( ulRequestType == (guint32)TCDMX_TYPE_AUDIO) {
			get_param.m_ulStreamType = (guint32)DMXTYPE_AUDIO;
			ret = TCDMX_TRUE;
		}
		else {
			GST_DEBUG_OBJECT(p_oggdmx,"not support ulRequestType(%d)",ulRequestType);
			ret = TCDMX_FALSE;
		}

		if (ret != TCDMX_FALSE)
		{
			ogg_dmx_output_t output;
			get_param.m_ulStreamBuffSize	=	LARGE_STREAM_BUF_SIZE;
			get_param.m_pbyStreamBuff		=	pLargeStramBuff;
			get_param.m_iStreamIdx			=	pstDemuxBase->request_id;
		
			res = p_oggdmx->gExtDmxFunc(DMXOP_GET_STREAM, &h_dmx, &get_param, &output);
			if ( res != DMXRET_SUCCESS ) {
				GST_ERROR_OBJECT (p_oggdmx, "ogg demux failed - (error: %d)", res);
				ret = TCDMX_FALSE;
			}
			else {
				switch( output.m_ulStreamType ) {
					case DMXTYPE_VIDEO:
						pstResult->ulStreamType = (guint32)TCDMX_TYPE_VIDEO;
						break;
					case DMXTYPE_AUDIO:
						pstResult->ulStreamType = (guint32)TCDMX_TYPE_AUDIO;
						break;
					default:
						GST_DEBUG_OBJECT(p_oggdmx,"not support output.m_ulStreamType(%d)",output.m_ulStreamType);
						break;
				}
			
				pstResult->ulStreamID = output.m_iStreamIdx;
				pstResult->pBuffer = output.m_pbyStreamData;
				pstResult->lLength = (gint)output.m_ulStreamDataSize;
				pstResult->lTimestampMs = (gint)output.m_ulTimeStamp;
			
				GST_LOG_OBJECT(p_oggdmx,"demux[ID:Type] = [%d:%d] timestamp=%d length=%d",pstResult->ulStreamID,pstResult->ulStreamType,pstResult->lTimestampMs,pstResult->lLength);
			}
		}
	}
	return ret;

}


static
gboolean
gst_oggdmx_vm_seek (
	GstTcDemuxBase  * pstDemuxBase, 
	guint32           ulFlags,
	gint32            lTargetPts, 
	gint32          * plResultPts
	)
{
	GstTcOggDemux *p_oggdmx = GST_OGGDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_oggdmx->hOggDmx;
	av_dmx_result_t res;
	ogg_dmx_seek_t	param;
	ogg_dmx_output_t	result;
	gboolean ret;

	param.s32SeekTime = lTargetPts;
	param.u32SeekMode = OGG_SEEKMODE_ABSOLUTE_TIME;

	res = p_oggdmx->gExtDmxFunc (DMXOP_SEEK, &h_dmx, &param, &result);
	if ( res != DMXRET_SUCCESS ) {
		GST_ERROR_OBJECT (p_oggdmx, "ogg seek failed - (error: %d)", res);
		ret = TCDMX_FALSE;
	} else {

		*plResultPts = (gint)result.m_ulTimeStamp;
		GST_DEBUG_OBJECT(p_oggdmx,"[SEEK] res %d target pts: %d seek pts: %d ",res, lTargetPts,result.m_ulTimeStamp);

		ret = TCDMX_TRUE;
	}
	return ret;
}


static
gboolean
gst_oggdmx_vm_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcOggDemux *p_oggdmx = GST_OGGDMX(pstDemuxBase);
	gint32 result_pts;

	(void)gst_oggdmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);	//seek to 0ms

	return TCDMX_TRUE;
}

static
gboolean
gst_oggdmx_vm_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcOggDemux *p_oggdmx = GST_OGGDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_oggdmx->hOggDmx;
	gboolean ret;

	if ( h_dmx != 0U) {
		av_dmx_result_t res = p_oggdmx->gExtDmxFunc (DMXOP_CLOSE, &h_dmx, NULL, NULL);
		if( res != DMXRET_SUCCESS ) {
			GST_ERROR_OBJECT (p_oggdmx, "ogg close failed - (error: %d)", res);
		}

		p_oggdmx->hOggDmx = 0;
		(void)memset ( &p_oggdmx->stInitInfo, 0, sizeof(ogg_dmx_info_t) );

		(void)gst_oggdmx_close_library(p_oggdmx);
		ret = TCDMX_TRUE;
	}
	else {
		GST_ERROR_OBJECT (p_oggdmx, "ogg already closed");
		ret = TCDMX_FALSE;
	}
	return ret;
}

static 
gboolean 
gst_oggdmx_vm_getparam (
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
		*ret_val = TCDMX_FALSE;
		break;
	case TCDEMUX_PARAM_QUERY_CONTAINER_NAME:
		*str = g_strdup("OGG");
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

