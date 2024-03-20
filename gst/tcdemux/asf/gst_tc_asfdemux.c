/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2013 nafume <<user@hostname.org>>
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
 * SECTION:element-tc-asfdemux
 *
 * FIXME:Describe tc-asfdemux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tc-asfdemux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_asfdemux.h"
#include "gst_tc_demuxio.h"


/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE          ("video/x-ms-asf")
#define STRING_DEMUX_LONGNAME       ("Telechips ASF Demuxer")
#define STRING_DEMUX_DESCRIPTION    ("Demultiplex ASF into element streams")


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

G_DEFINE_TYPE (GstTcAsfDemux, gst_asfdmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

/* GObject virtual method implementations */
static void gst_asfdmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_asfdmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_asfdmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_asfdmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestID, tcdmx_result_t * pstResult);
static gboolean gst_asfdmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_asfdmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_asfdmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_asfdmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);

/* GstTcAsfDemux private implementations */


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//


/* initialize the tc-asfdemux's class */
static 
void
gst_asfdmx_class_init (
	GstTcAsfDemuxClass  * pstDemuxClass
	)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxClass;
	GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("");
	
	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overiding
	p_object_class->finalize = gst_asfdmx_finalize;

	// base class overiding
	p_dmxbase_class->vmOpen    = gst_asfdmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_asfdmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_asfdmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_asfdmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_asfdmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_asfdmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_asfdmx_vm_getparam;
	

	// set demux class details
	gst_tcdmx_set_class_details (GST_DMXBASE_CLASS(pstDemuxClass)
		, STRING_DEMUX_LONGNAME
		, STRING_DEMUX_DESCRIPTION
		);

	// sink pad template
	gst_tcdmx_create_sinkpad_templates (GST_DMXBASE_CLASS(pstDemuxClass), &gs_stSinkFactory);

	// src pad templates
	gst_tcdmx_create_srcpad_templates ( GST_DMXBASE_CLASS (pstDemuxClass)
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
gst_asfdmx_init (
	GstTcAsfDemux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), TCDMX_MODE_SEQUENTIAL);	//TCDMX_MODE_SELECTIVE
}


static 
void
gst_asfdmx_finalize (
	GObject  * pstObject
	)
{
  GstTcAsfDemux *p_dmx = GST_ASFDMX (pstObject);

  GST_TRACE ("");

  //TODO: finialize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define ASF_EXT_LIB_NAME "libtccasfdmx.so"

gint gst_asfdmx_load_library(GstTcAsfDemux *p_asfdmx);
void gst_asfdmx_close_library(GstTcAsfDemux *p_asfdmx);

gint gst_asfdmx_load_library(GstTcAsfDemux *p_asfdmx)
{
  p_asfdmx->pExtDLLModule = dlopen(ASF_EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if( p_asfdmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_asfdmx,"[ASFDMX] Load library '%s' failed: %s", ASF_EXT_LIB_NAME, dlerror());
    return -1;
  } else {
    GST_DEBUG_OBJECT(p_asfdmx,"[ASFDMX] Library '%s' Loaded", ASF_EXT_LIB_NAME);
  }

  p_asfdmx->gExtDmxFunc = dlsym(p_asfdmx->pExtDLLModule, "TCC_ASF_DMX");
  if( p_asfdmx->gExtDmxFunc == NULL ) {
    GST_ERROR_OBJECT(p_asfdmx,"[ASFDMX] p_asfdmx->gExtDmxFunc Error");
    return -1;
  }

  return 0;
}

void gst_asfdmx_close_library(GstTcAsfDemux *p_asfdmx)
{
  if( p_asfdmx->pExtDLLModule != NULL)
    (void)dlclose(p_asfdmx->pExtDLLModule);
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
gst_asfdmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GstTcAsfDemux *p_asfdmx = GST_ASFDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx;
	av_dmx_result_t res;
	asf_dmx_init_t init_param;
	asf_dmx_info_t *p_init_info = &p_asfdmx->stInitInfo;
	asf_dmx_sel_stream_t * pstSelStream = &p_asfdmx->stSelStream;
	asf_dmx_sel_info_t * pstSelStreamInfo = &p_asfdmx->stSelStreamInfo;

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

	if (gst_asfdmx_load_library(p_asfdmx))
		return GST_FLOW_NOT_SUPPORTED;
	memset ( &init_param, 0, sizeof(asf_dmx_init_t) );

	init_param.m_stMemoryFuncs			= mem_func;
	init_param.m_stFileFuncs			= file_func;
	init_param.m_pszOpenFileName		= (char*)(void*)pstDemuxBase->pstSinkPad;
	init_param.m_lAndroidProgressive	= 0;
	init_param.m_lFileCacheSize			= 64*1024;
	init_param.m_lGetRealDuration		= 1;
	init_param.m_lSeekCacheSize			= 0;
	init_param.m_pfnTCC_ASF_DRM_Decrypt = 0;
	init_param.m_lPlayOption			= 0;

	//demuxing mode
	if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING))){
		init_param.m_lPlayOption = ASF_SELECTIVE_MODE;
	} else if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_SEQUENTIAL_DEMUXING))) {
		init_param.m_lPlayOption = ASF_SEQUENTIAL_MODE;
	}

	//seek fail handling scenario
	if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL))){
		init_param.m_lSeekOption = 2;
	} else if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL))) {
		init_param.m_lSeekOption = 1;
	}

	res = p_asfdmx->gExtDmxFunc(DMXOP_OPEN, &h_dmx, &init_param, p_init_info);
	if ( res != DMXRET_SUCCESS ) {
		GST_ERROR_OBJECT (p_asfdmx, "asf open failed - (error: %d)", res);
		return GST_FLOW_NOT_SUPPORTED;
	}

	if (init_param.m_lPlayOption == ASF_SEQUENTIAL_MODE) {
		if(CHECK_FLAG((pstDemuxBase->ulFlags), (TCDMX_BASE_FLAG_MULTIPLE_STEAM)))
		{
			GST_DEBUG_OBJECT (p_asfdmx, "Multiple stream demuxing enabled");
			pstSelStream->m_lMultipleDemuxing = 1;
		}
	}
	else
	{
		pstSelStream->m_lMultipleDemuxing = 0;
	}

	if(pstSelStream->m_lMultipleDemuxing)
	{
		int i;
		pstSelStream->m_lSelType = 0;
		if(p_init_info->m_lAudioStreamTotal)
		{
			pstSelStream->m_lSelType |= DMXTYPE_AUDIO;
			pstSelStream->m_lAudioID = 0;
			for(i = 0;i < p_init_info->m_lAudioStreamTotal;i++)
				pstSelStream->m_lAudioID |= ASF_STREAM_INDEX_(i);
		}
		res = p_asfdmx->gExtDmxFunc(DMXOP_SELECT_STREAM, &h_dmx, pstSelStream, pstSelStreamInfo);
		if(res != DMXRET_SUCCESS)
		{
			GST_ERROR_OBJECT (p_asfdmx, "asf stream selection failed - (error: %d)", (int)res);
			//Just keep demuxing in single stream mode.(do not return error)
			pstSelStream->m_lMultipleDemuxing = 0;
		}
	}

	if (p_init_info->m_lAudioStreamTotal) {
		gst_tcdmx_set_total_audiostream(pstDemuxBase, p_init_info->m_lAudioStreamTotal);
	}

	p_asfdmx->hAsfDmx = h_dmx;

	return GST_FLOW_OK;
}

static
gboolean
gst_asfdmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcAsfDemux *p_asfdmx = GST_ASFDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_asfdmx->hAsfDmx;
	asf_dmx_info_t *p_init_info = &p_asfdmx->stInitInfo;
	asf_dmx_sel_stream_t * pstSelStream = &p_asfdmx->stSelStream;
	asf_dmx_sel_info_t * pstSelStreamInfo = &p_asfdmx->stSelStreamInfo;

	GST_INFO_OBJECT (p_asfdmx, "This asf stream is %sseekable", p_init_info->m_pstFileInfo->m_lSeekable ? "":"No-");
	gst_tcdmx_set_seekable(pstDemuxBase, p_init_info->m_pstFileInfo->m_lSeekable);

	if (p_init_info->m_lVideoStreamTotal) {
		asf_dmx_video_info_t *p_info = p_init_info->m_pstDefaultVideoInfo;
		tc_video_info_t info;
		info.ulFourCC       = p_info->m_ulFourCC;
		info.ulWidth        = p_info->m_lWidth;
		info.ulHeight       = p_info->m_lHeight;
		info.ulBitPerPixel  = 0;
		info.ulFrameRate    = p_info->m_lFrameRate;
		info.ulDuration     = p_init_info->m_pstFileInfo->m_lDuration;

		(void)gst_tcdmx_register_stream_info (pstDemuxBase, 
										p_info->m_lStreamID,	//TCDMX_TYPE_VIDEO, 
										(gboolean)TCDMX_TYPE_VIDEO, 
										&info, 
										p_info->m_pExtraData, 
										p_info->m_lExtraLength);
	}

	if (p_init_info->m_lAudioStreamTotal) {
		if(pstSelStream->m_lMultipleDemuxing)
		{
			gint i;
			asf_dmx_audio_info_t *p_info;
			tc_audio_info_t info;
			for(i = 0;i < pstSelStreamInfo->m_ulNumAudioStream;i++)
			{
				p_info = &pstSelStreamInfo->m_pstAudioInfo[i];
				memset(&info, 0, sizeof(tc_audio_info_t));
				info.ulFormatTag     = p_info->m_lFormatTag;
				info.ulChannels      = p_info->m_lChannels;
				info.ulBlockAlign    = p_info->m_lBlockAlign;
				info.ulBitPerSample  = p_info->m_lBitsPerSample;
				info.ulSampleRate    = p_info->m_lSamplePerSec;
				info.ulSize          = p_info->m_lAvgBytesPerSec;// ?
				info.ulDuration      = p_init_info->m_pstFileInfo->m_lDuration;

				if (info.ulFormatTag == WAVE_FORMAT_PCM)
					info.ulEndian		= 0;		//LittleEndian or Don't Care

				if ((info.ulFormatTag == WAVE_FORMAT_VORBIS) ||  /* ogg/vorbis mode */
					(info.ulFormatTag == WAVE_FORMAT_VORBIS1) || /* ogg/vorbis mode 1 */
					(info.ulFormatTag == WAVE_FORMAT_VORBIS2) || /* ogg/vorbis mode 2 */
					(info.ulFormatTag == WAVE_FORMAT_VORBIS3) || /* ogg/vorbis mode 3 */
					(info.ulFormatTag == WAVE_FORMAT_VORBIS1PLUS) || /* ogg/vorbis mode 1+ */
					(info.ulFormatTag == WAVE_FORMAT_VORBIS2PLUS) || /* ogg/vorbis mode 2+ */
					(info.ulFormatTag == WAVE_FORMAT_VORBIS3PLUS)    /* ogg/vorbis mode 3+ */
				 ) {
					p_asfdmx->lFlagVorbisInit = 3;
					gst_tcdmx_search_vorbis_header(p_info->m_pExtraData, p_info->m_lExtraLength, &p_asfdmx->stVorbisHeaderInfo);
				}

				(void)gst_tcdmx_register_stream_info (pstDemuxBase, 
											p_info->m_lStreamID,	//TCDMX_TYPE_AUDIO, 
											(gboolean)TCDMX_TYPE_AUDIO, 
											&info, 
											p_info->m_pExtraData, 
											p_info->m_lExtraLength);
			}

		}
		else {
			asf_dmx_audio_info_t *p_info = p_init_info->m_pstDefaultAudioInfo;
			tc_audio_info_t info;
			memset(&info, 0, sizeof(tc_audio_info_t));
			info.ulFormatTag     = p_info->m_lFormatTag;
			info.ulChannels      = p_info->m_lChannels;
			info.ulBlockAlign    = p_info->m_lBlockAlign;
			info.ulBitPerSample  = p_info->m_lBitsPerSample;
			info.ulSampleRate    = p_info->m_lSamplePerSec;
			info.ulSize          = p_info->m_lAvgBytesPerSec;// ?
			info.ulDuration      = p_init_info->m_pstFileInfo->m_lDuration;

			if (info.ulFormatTag == WAVE_FORMAT_PCM)
				info.ulEndian		= 0;		//LittleEndian or Don't Care

			if ((info.ulFormatTag == WAVE_FORMAT_VORBIS) ||  /* ogg/vorbis mode */
				(info.ulFormatTag == WAVE_FORMAT_VORBIS1) || /* ogg/vorbis mode 1 */
				(info.ulFormatTag == WAVE_FORMAT_VORBIS2) || /* ogg/vorbis mode 2 */
				(info.ulFormatTag == WAVE_FORMAT_VORBIS3) || /* ogg/vorbis mode 3 */
				(info.ulFormatTag == WAVE_FORMAT_VORBIS1PLUS) || /* ogg/vorbis mode 1+ */
				(info.ulFormatTag == WAVE_FORMAT_VORBIS2PLUS) || /* ogg/vorbis mode 2+ */
				(info.ulFormatTag == WAVE_FORMAT_VORBIS3PLUS)    /* ogg/vorbis mode 3+ */
			 )		
			{
				p_asfdmx->lFlagVorbisInit = 3;
				gst_tcdmx_search_vorbis_header(p_info->m_pExtraData, p_info->m_lExtraLength, &p_asfdmx->stVorbisHeaderInfo);
			}

			(void)gst_tcdmx_register_stream_info (pstDemuxBase, 
											p_info->m_lStreamID,	//TCDMX_TYPE_AUDIO, 
											(gboolean)TCDMX_TYPE_AUDIO, 
											&info, 
											p_info->m_pExtraData, 
											p_info->m_lExtraLength);
		}
	}

	return TCDMX_TRUE;
}

static
gboolean
gst_asfdmx_vm_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestType,
	tcdmx_result_t  * pstResult
	)
{
	GstTcAsfDemux *p_asfdmx = GST_ASFDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_asfdmx->hAsfDmx;
	av_dmx_result_t res;
	av_dmx_getstream_t get_param; 
	asf_dmx_outstream_t output;

	get_param.s32StreamType = DMXTYPE_NONE;

	if( ulRequestType != TCDMX_TYPE_ANY ) {
		switch( ulRequestType ) {
			case TCDMX_TYPE_VIDEO:
				get_param.s32StreamType = DMXTYPE_VIDEO;
				break;
			case TCDMX_TYPE_AUDIO:
				get_param.s32StreamType = DMXTYPE_AUDIO;
				break;
			case TCDMX_TYPE_SUBTITLE:
				get_param.s32StreamType = DMXTYPE_SUBTITLE;
				break;
		}
	}

	if (((get_param.s32StreamType == DMXTYPE_AUDIO) || (get_param.s32StreamType == DMXTYPE_NONE) )
		&& (p_asfdmx->lFlagVorbisInit != 0))
	{
		switch(p_asfdmx->lFlagVorbisInit)
		{
			case 3:
				p_asfdmx->lFlagVorbisInit = 2;
				pstResult->pBuffer      = p_asfdmx->stVorbisHeaderInfo.pData1;
				pstResult->lLength      = p_asfdmx->stVorbisHeaderInfo.size1;
				break;
			case 2:
				p_asfdmx->lFlagVorbisInit = 1;
				pstResult->pBuffer      = p_asfdmx->stVorbisHeaderInfo.pData2;
				pstResult->lLength      = p_asfdmx->stVorbisHeaderInfo.size2;
				break;
			case 1:
				p_asfdmx->lFlagVorbisInit = 0;
				pstResult->pBuffer      = p_asfdmx->stVorbisHeaderInfo.pData3;
				pstResult->lLength      = p_asfdmx->stVorbisHeaderInfo.size3;
				break;
		}

		pstResult->ulStreamID   = p_asfdmx->stInitInfo.m_pstDefaultAudioInfo->m_lStreamID;	//TCDMX_TYPE_AUDIO;
		pstResult->lTimestampMs = 0;
		pstResult->ulStreamID = TCDMX_TYPE_AUDIO;
		GST_LOG_OBJECT(p_asfdmx,"VorbisInit[%d] length %d streamID %d ,streamType %d",(int)p_asfdmx->lFlagVorbisInit,pstResult->lLength,pstResult->ulStreamID,pstResult->ulStreamType);
		return TCDMX_TRUE;
	}
	res = p_asfdmx->gExtDmxFunc(DMXOP_GET_STREAM, &h_dmx, &get_param, &output);
	if ( res != DMXRET_SUCCESS ) {
		GST_ERROR_OBJECT (p_asfdmx, "asf demux failed - (error: %d)", (int)res);
		return TCDMX_FALSE;
	}

	switch( output.m_lStreamType ) {
		case DMXTYPE_VIDEO:
			pstResult->ulStreamType = TCDMX_TYPE_VIDEO;
			break;
		case DMXTYPE_AUDIO:
			pstResult->ulStreamType = TCDMX_TYPE_AUDIO;
			break;
		case DMXTYPE_SUBTITLE:
			pstResult->ulStreamType = TCDMX_TYPE_SUBTITLE;
			break;
	}
	pstResult->ulStreamID = output.m_iStreamIdx;	//pstResult->ulStreamType;
	pstResult->pBuffer = output.m_pbyStreamData;
	pstResult->lLength = output.m_lStreamDataSize;
	pstResult->lTimestampMs = output.m_lTimeStamp;

	GST_LOG_OBJECT(p_asfdmx,"demux[ID:Type] = [%d:%d] timestamp=%d length=%d",pstResult->ulStreamID,pstResult->ulStreamType,pstResult->lTimestampMs,pstResult->lLength);

	return TCDMX_TRUE;
}


static
gboolean
gst_asfdmx_vm_seek (
	GstTcDemuxBase  * pstDemuxBase, 
	guint32           ulFlags,
	gint32            lTargetPts, 
	gint32          * plResultPts
	)
{
	GstTcAsfDemux *p_asfdmx = GST_ASFDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_asfdmx->hAsfDmx;
	av_dmx_result_t res;
	av_dmx_seek_t	param;
	asf_dmx_outstream_t	result;

	param.s32SeekTime = lTargetPts;
	param.u32SeekMode = DMXSEEK_DEFAULT;
	
	res = p_asfdmx->gExtDmxFunc (DMXOP_SEEK, &h_dmx, &param, &result);
	if ( res != DMXRET_SUCCESS ) {
		GST_ERROR_OBJECT (p_asfdmx, "asf seek failed - (error: %d)", (int)res);
		return TCDMX_FALSE;
	}

	*plResultPts = result.m_lTimeStamp;

	return TCDMX_TRUE;
}


static
gboolean
gst_asfdmx_vm_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcAsfDemux *p_asfdmx = GST_ASFDMX(pstDemuxBase);
	gint32 result_pts;

	gst_asfdmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);	//seek to 0ms

	return TCDMX_TRUE;
}

static
gboolean
gst_asfdmx_vm_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcAsfDemux *p_asfdmx = GST_ASFDMX(pstDemuxBase);
	av_dmx_handle_t	h_dmx = p_asfdmx->hAsfDmx;

	if ( h_dmx ) {
		av_dmx_result_t res = p_asfdmx->gExtDmxFunc (DMXOP_CLOSE, &h_dmx, NULL, NULL);
		if( res != DMXRET_SUCCESS ) {
			GST_ERROR_OBJECT (p_asfdmx, "asf close failed - (error: %d)", res);
		}

		p_asfdmx->hAsfDmx = 0;
		memset ( &p_asfdmx->stInitInfo, 0, sizeof(asf_dmx_info_t) );

		gst_asfdmx_close_library(p_asfdmx);
		return TCDMX_TRUE;
	}
	else {
		GST_ERROR_OBJECT (p_asfdmx, "asf already closed");
		return TCDMX_FALSE;
	}
}

static 
gboolean 
gst_asfdmx_vm_getparam (
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
		*str = g_strdup("ASF");
		ret_val = TCDMX_TRUE;
		break;
	default:
		*ret_val = TCDMX_FALSE;
		break;
	}
	return ret_val;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstTcAsfDemux private implementations
//
