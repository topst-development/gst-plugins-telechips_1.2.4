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
 * SECTION:element-tc-avidemux
 *
 * FIXME:Describe tc-avidemux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tc-avidemux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_avidemux.h"
#include "gst_tc_demuxio.h"


/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE          ("video/x-msvideo")
#define STRING_DEMUX_LONGNAME       ("Telechips AVI Demuxer")
#define STRING_DEMUX_DESCRIPTION    ("Demultiplex AVI into element streams")


static GstStaticPadTemplate gs_stSinkFactory = GST_STATIC_PAD_TEMPLATE (
	"sink",									//template-name
	GST_PAD_SINK,							//direction
	GST_PAD_ALWAYS,							//presence
	GST_STATIC_CAPS ("video/x-msvideo")
    );

G_DEFINE_TYPE (GstTcAviDemux, gst_avidmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

/* GObject virtual method implementations */
static void gst_avidmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_avidmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_avidmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_avidmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestType, tcdmx_result_t * pstResult);
static gboolean gst_avidmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_avidmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_avidmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_avidmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);

/* GstTcMpgDemux private implementations */



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//

/* initialize the tc-avidemux's class */
static 
void
gst_avidmx_class_init (
	GstTcAviDemuxClass  * pstDemuxClass
	)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxClass;
	GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("");

	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overiding
	p_object_class->finalize = gst_avidmx_finalize;

	// base class overiding
	p_dmxbase_class->vmOpen    = gst_avidmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_avidmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_avidmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_avidmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_avidmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_avidmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_avidmx_vm_getparam;

	// set demux class details
	gst_tcdmx_set_class_details (p_dmxbase_class
		, STRING_DEMUX_LONGNAME
		, STRING_DEMUX_DESCRIPTION
		);

	// sink pad template
	(void)gst_tcdmx_create_sinkpad_templates (p_dmxbase_class, &gs_stSinkFactory);

	// src pad templates
	(void)gst_tcdmx_create_srcpad_templates ( p_dmxbase_class
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
gst_avidmx_init (
	GstTcAviDemux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	(void)gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	(void)gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), TCDMX_MODE_SELECTIVE);

}


static 
void
gst_avidmx_finalize (
	GObject  * pstObject
	)
{
  GstTcAviDemux *p_dmx = GST_AVIDMX (pstObject);

  GST_TRACE ("");

  //TODO: finialize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define AVI_EXT_LIB_NAME ("libtccavidmx.so")

gint gst_avidmx_load_library(GstTcAviDemux *p_avidmx);
void gst_avidmx_close_library(GstTcAviDemux *p_avidmx);

gint gst_avidmx_load_library(GstTcAviDemux *p_avidmx)
{
  gint result = 0;
  p_avidmx->pExtDLLModule = dlopen(AVI_EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if( p_avidmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_avidmx,"[AVIDMX] Load library '%s' failed: %s", AVI_EXT_LIB_NAME, dlerror());
    result = -1;
  } else {
    GST_DEBUG_OBJECT(p_avidmx,"[AVIDMX] Library '%s' Loaded", AVI_EXT_LIB_NAME);

    p_avidmx->gExtDmxFunc = dlsym(p_avidmx->pExtDLLModule, "TCC_AVI_DMX");
    if( p_avidmx->gExtDmxFunc == NULL ) {
      GST_ERROR_OBJECT(p_avidmx,"[AVIDMX] p_avidmx->gExtDmxFunc Error");
      result = -1;
    }
  }

  return result;
}

void gst_avidmx_close_library(GstTcAviDemux *p_avidmx)
{
  if( p_avidmx->pExtDLLModule != NULL){
    (void)dlclose(p_avidmx->pExtDLLModule);
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
static gboolean gst_avidmx_convert_is_avcc(gchar *pbyData, gint32 lLen);

static gint gst_avidmx_check_mismatch(GstTcDemuxBase  * pstDemuxBase, avi_dmx_handle_t *ph_dmx);

static
GstFlowReturn
gst_avidmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GstTcAviDemux *p_avidmx = GST_AVIDMX(pstDemuxBase);
	avi_dmx_handle_t	h_dmx;
	avi_dmx_result_t	res;
	avi_dmx_init_t	init_param;

	avi_dmx_info_t *p_init_info = &p_avidmx->stInitInfo;

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

	if (gst_avidmx_load_library(p_avidmx) != 0) {
		return GST_FLOW_NOT_SUPPORTED;
	}

	(void)memset ( &init_param, 0, sizeof(avi_dmx_init_t) );

	init_param.m_stMemoryFuncs = mem_func;
	init_param.m_stFileFuncs = file_func;

	init_param.m_pszOpenFileName = (gchar*)(void*)pstDemuxBase->pstSinkPad;

	init_param.m_lIoCacheSize = 4096;
	//init_param.m_ulOption = 0;					/* zero option */

	//demuxing mode
	if ( CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_SELECTIVE_DEMUXING) ){
		init_param.m_ulOption |= AVIOPT_SELECTIVE;
	}
	if ( CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_STREAMING_SOURCE) ){
		init_param.m_ulOption |= AVIOPT_STREAMING;
	}

	//seek fail handling scenario
	if (CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL)){
		init_param.m_ulOption |= AVIOPT_LAST_KEY_AFTER_SEEKFAIL;
	} else if (CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL)){
		init_param.m_ulOption |= AVIOPT_EOF_AFTER_SEEKFAIL;
	}

	{
		//--------------------------------------------------------
		// [*] AVIDOP_GET_VERSION operation
		// --------------------------------------------------------
		gchar	pVersionInfo[AVI_VER_BUFF_SIZE];
		gchar    pBuildDate[AVI_VER_BUFF_SIZE];

		res = p_avidmx->gExtDmxFunc( AVIDOP_GET_VERSION, 0, pVersionInfo, pBuildDate );
		if (res < 0){
			g_printf("get version info failed\n");
		} else {
			g_printf("%s, %s\n", pVersionInfo, pBuildDate);
		}
	}

	res = p_avidmx->gExtDmxFunc(DMXOP_OPEN, &h_dmx, &init_param, p_init_info);
	if ( res != AVIERR_NONE ) {
		res = p_avidmx->gExtDmxFunc(DMXOP_GET_LASTERROR, 0, 0, 0);
		GST_ERROR_OBJECT (p_avidmx, "avi open failed - (error: %d)", res);
		if ( res == AVIERR_DIVX_STREAM_NOT_SUPPORTED )
		{
			guchar fourcc[4];
			fourcc[0] = (guchar)(p_init_info->m_pstDefaultVideoInfo->m_ulFourCC)&0xFF;
			fourcc[1] = (guchar)(p_init_info->m_pstDefaultVideoInfo->m_ulFourCC>>8)&0xFF;
			fourcc[2] = (guchar)(p_init_info->m_pstDefaultVideoInfo->m_ulFourCC>>16)&0xFF;
			fourcc[3] = (guchar)(p_init_info->m_pstDefaultVideoInfo->m_ulFourCC>>24)&0xFF;
			g_printf (" avi open failed - This AVI library not support %c%c%c%c video\n", fourcc[0],fourcc[1],fourcc[2],fourcc[3]);
			return GST_FLOW_ERROR_NOT_SUPPORT_DIVX;
		}
		return GST_FLOW_NOT_SUPPORTED;
	}

	//mm016 - check MISMATCH_TYPE
	if ((p_init_info->m_lVideoStreamTotal) && (p_init_info->m_pstDefaultVideoInfo->m_lExtraLength))
	{
		avi_dmx_video_info_t *p_info = p_init_info->m_pstDefaultVideoInfo;

		if (p_info->m_ulFourCC == FOURCC_AVC1 || p_info->m_ulFourCC == FOURCC_avc1 ||
			p_info->m_ulFourCC == FOURCC_H264 || p_info->m_ulFourCC == FOURCC_h264 ||
			p_info->m_ulFourCC == FOURCC_X264 || p_info->m_ulFourCC == FOURCC_x264)
		{
			if ((gst_avidmx_convert_is_avcc(p_info->m_pExtraData, p_info->m_lExtraLength) > 0)
			 && (gst_avidmx_check_mismatch(pstDemuxBase, &h_dmx) > 0)) {
				GST_INFO_OBJECT (p_avidmx, "mismatch!! remove extradata. extralength %d -> 0",p_info->m_lExtraLength);
				p_info->m_lExtraLength = 0;
			}
		}
	}

	if (p_init_info->m_lAudioStreamTotal > 0){
		gst_tcdmx_set_total_audiostream(pstDemuxBase, (guint32)p_init_info->m_lAudioStreamTotal);
	}

	p_avidmx->hAviDmx = h_dmx;

	return GST_FLOW_OK;
}


static
gboolean
gst_avidmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcAviDemux *p_avidmx = GST_AVIDMX(pstDemuxBase);
	avi_dmx_handle_t	h_dmx = p_avidmx->hAviDmx;

	avi_dmx_info_t *p_init_info = &p_avidmx->stInitInfo;

	if (p_avidmx->stInitInfo.m_pstFileInfo->m_bHasIndex){
		gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);
	} else {
		gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_FALSE);
	}

	if (p_init_info->m_lVideoStreamTotal > 0) {
		avi_dmx_video_info_t *p_info = p_init_info->m_pstDefaultVideoInfo;
		tc_video_info_t info;
		info.ulFourCC       = p_info->m_ulFourCC;
		info.ulWidth        = (guint32)p_info->m_lWidth;
		info.ulHeight       = (guint32)p_info->m_lHeight;
		info.ulBitPerPixel  = (guint32)p_info->m_lBitsPerSample;
		info.ulFrameRate    = (guint32)p_info->m_lFrameRate;
		info.ulDuration     = (guint32)p_init_info->m_pstFileInfo->m_lDuration;

		gchar *p_extra_data = NULL;
		gint32 extra_data_len = 0;

		if (p_info->m_lExtraLength > 0)
		{
			p_extra_data = p_info->m_pExtraData;
			extra_data_len = p_info->m_lExtraLength;
		}

		GST_DEBUG_OBJECT (p_avidmx, "[Video] p_info->m_lStreamID=%d p_info->m_ulFourCC=%x\n",p_info->m_lStreamID,p_info->m_ulFourCC);
		(void)gst_tcdmx_register_stream_info (pstDemuxBase,
										p_info->m_lStreamID,
										TCDMX_TYPE_VIDEO,
										&info,
										p_extra_data,
										extra_data_len);
	}

	if (p_init_info->m_lAudioStreamTotal > 0) {
		avi_dmx_audio_info_t *p_info = p_init_info->m_pstDefaultAudioInfo;
		tc_audio_info_t info;
		av_sint16_t wFormatTag = (av_sint16_t)p_info->m_lFormatTag;
		memset(&info, 0, sizeof(tc_audio_info_t));
		info.ulFormatTag     = (guint32)p_info->m_lFormatTag;
		info.ulChannels      = (guint32)p_info->m_lChannels;
		info.ulBlockAlign    = (guint32)p_info->m_lBlockAlign;
		info.ulBitPerSample  = (guint32)p_info->m_lBitsPerSample;
		info.ulSampleRate    = (guint32)p_info->m_lSamplePerSec;
		info.ulSize          = (guint32)p_info->m_lAvgBytesPerSec;
		info.ulDuration      = (guint32)p_init_info->m_pstFileInfo->m_lDuration;

		if (wFormatTag == WAVE_FORMAT_PCM) {
			info.ulEndian		= 0;		//LittleEndian or Don't Care
		}

		if ((wFormatTag == WAVE_FORMAT_VORBIS) ||  /* ogg/vorbis mode */
			(wFormatTag == WAVE_FORMAT_VORBIS1) || /* ogg/vorbis mode 1 */
			(wFormatTag == WAVE_FORMAT_VORBIS2) || /* ogg/vorbis mode 2 */
			(wFormatTag == WAVE_FORMAT_VORBIS3) || /* ogg/vorbis mode 3 */
			(wFormatTag == WAVE_FORMAT_VORBIS1PLUS) || /* ogg/vorbis mode 1+ */
			(wFormatTag == WAVE_FORMAT_VORBIS2PLUS) || /* ogg/vorbis mode 2+ */
			(wFormatTag == WAVE_FORMAT_VORBIS3PLUS)    /* ogg/vorbis mode 3+ */
		 )
		{
			p_avidmx->lFlagVorbisInit = 3;
			gst_tcdmx_search_vorbis_header(p_info->m_pExtraData, p_info->m_lExtraLength, &p_avidmx->stVorbisHeaderInfo);
		}

		GST_DEBUG_OBJECT (p_avidmx, "[Audio] p_info->m_lStreamID=%d p_info->m_lFormatTag=%x\n",p_info->m_lStreamID,p_info->m_lFormatTag);
		(void)gst_tcdmx_register_stream_info (pstDemuxBase,
										p_info->m_lStreamID,
										TCDMX_TYPE_AUDIO,
										&info,
										p_info->m_pExtraData,
										p_info->m_lExtraLength);
	}

	return TCDMX_TRUE;
}

static
gboolean
gst_avidmx_vm_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestType,
	tcdmx_result_t  * pstResult
	)
{
	GstTcAviDemux *p_avidmx = GST_AVIDMX(pstDemuxBase);
	avi_dmx_handle_t	h_dmx = p_avidmx->hAviDmx;
	avi_dmx_result_t res;
	av_dmx_getstream_t get_param;
	av_dmx_outstream_t output;

	memset(&get_param, 0, sizeof(av_dmx_getstream_t));

	if(!pstDemuxBase->bPushMode)
	{
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
	}
	else {
		get_param.s32StreamType = DMXTYPE_NONE;
	}

	if (((get_param.s32StreamType == DMXTYPE_AUDIO) || (get_param.s32StreamType == DMXTYPE_NONE) )
		&& (p_avidmx->lFlagVorbisInit != 0))
	{
		switch(p_avidmx->lFlagVorbisInit)
		{
			case 3:
				p_avidmx->lFlagVorbisInit = 2;
				pstResult->pBuffer      = p_avidmx->stVorbisHeaderInfo.pData1;
				pstResult->lLength      = p_avidmx->stVorbisHeaderInfo.size1;
				break;
			case 2:
				p_avidmx->lFlagVorbisInit = 1;
				pstResult->pBuffer      = p_avidmx->stVorbisHeaderInfo.pData2;
				pstResult->lLength      = p_avidmx->stVorbisHeaderInfo.size2;
				break;
			case 1:
				p_avidmx->lFlagVorbisInit = 0;
				pstResult->pBuffer      = p_avidmx->stVorbisHeaderInfo.pData3;
				pstResult->lLength      = p_avidmx->stVorbisHeaderInfo.size3;
				break;
		}

		pstResult->ulStreamID = p_avidmx->stInitInfo.m_pstDefaultAudioInfo->m_lStreamID;
		pstResult->ulStreamType   = TCDMX_TYPE_AUDIO;
		pstResult->lTimestampMs = 0;

		return TCDMX_TRUE;
	}

	res = p_avidmx->gExtDmxFunc(DMXOP_GET_STREAM, &h_dmx, &get_param, &output);
	if ( res != AVIERR_NONE ) {
		res = p_avidmx->gExtDmxFunc(DMXOP_GET_LASTERROR, &h_dmx, 0, 0);
		GST_ERROR_OBJECT (p_avidmx, "avi demux failed - (error: %d)", res);
		return TCDMX_FALSE;
	}

	switch( output.s32StreamType ) {
		case DMXTYPE_VIDEO:
			pstResult->ulStreamType = TCDMX_TYPE_VIDEO;
			pstResult->ulStreamID = p_avidmx->stInitInfo.m_pstDefaultVideoInfo->m_lStreamID;
			break;
		case DMXTYPE_AUDIO:
			pstResult->ulStreamType = TCDMX_TYPE_AUDIO;
			pstResult->ulStreamID = p_avidmx->stInitInfo.m_pstDefaultAudioInfo->m_lStreamID;
			break;
		case DMXTYPE_SUBTITLE:
			pstResult->ulStreamType = TCDMX_TYPE_SUBTITLE;
			pstResult->ulStreamID = p_avidmx->stInitInfo.m_pstDefaultMetaInfo->m_lStreamID;
			break;
	}
//	pstResult->ulStreamID = pstResult->ulStreamType;
	pstResult->pBuffer = output.pbyStreamData;
	pstResult->lLength = output.s32StreamDataSize;
	pstResult->lTimestampMs = output.s32TimeStamp;

	GST_LOG_OBJECT (p_avidmx, "demux[ID:Type] = [%d:%d] timestamp %d size %d",pstResult->ulStreamID,pstResult->ulStreamType,pstResult->lTimestampMs,pstResult->lLength);

	return TCDMX_TRUE;
}


static
gboolean
gst_avidmx_vm_seek (
	GstTcDemuxBase  * pstDemuxBase, 
	guint32           ulFlags,
	gint32            lTargetPts, 
	gint32          * plResultPts
	)
{
	GstTcAviDemux *p_avidmx = GST_AVIDMX(pstDemuxBase);
	avi_dmx_handle_t	h_dmx = p_avidmx->hAviDmx;
	avi_dmx_result_t 	res;
	av_dmx_seek_t		param;
	av_dmx_seek_t		output;

	if(lTargetPts != 0 && p_avidmx->stInitInfo.m_pstFileInfo->m_bHasIndex == 0) {
		GST_ERROR_OBJECT (p_avidmx, "avi seek failed - no index");
		return TCDMX_FALSE;
	}

	if( h_dmx ) {
		param.s32SeekTime = lTargetPts;
		param.u32SeekMode = 0;

		res = p_avidmx->gExtDmxFunc (DMXOP_SEEK, &h_dmx, &param, &output);
		if ( res != AVIERR_NONE ) {
			res = p_avidmx->gExtDmxFunc(DMXOP_GET_LASTERROR, &h_dmx, 0, 0);
			GST_ERROR_OBJECT (p_avidmx, "avi seek failed - (error: %d)", res);
			if (res == AVIERR_END_OF_STREAM)
				return TCDMX_FALSE;
		}

		*plResultPts = output.s32SeekTime;

	}
	return TCDMX_TRUE;
}


static
gboolean
gst_avidmx_vm_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcAviDemux *p_avidmx = GST_AVIDMX(pstDemuxBase);
	gint32 result_pts;

	gst_avidmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);	//seek to 0ms

	return TCDMX_TRUE;
}

static
gboolean
gst_avidmx_vm_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcAviDemux *p_avidmx = GST_AVIDMX(pstDemuxBase);
	avi_dmx_handle_t	h_dmx = p_avidmx->hAviDmx;

	if ( h_dmx ) {
		p_avidmx->gExtDmxFunc(DMXOP_CLOSE, &h_dmx, 0, 0);
		p_avidmx->hAviDmx = 0;
		gst_avidmx_close_library(p_avidmx);
		return TCDMX_TRUE;
	}
	else
	{
		GST_ERROR_OBJECT (p_avidmx, "avi already closed");
		return TCDMX_FALSE;
	}
}

static 
gboolean 
gst_avidmx_vm_getparam (
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
		*str = g_strdup("AVI");
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
//	GstTcAviDemux private implementations
//
static
gboolean
gst_avidmx_convert_is_avcc(gchar *pbyData, gint32 lLen)
{
	gboolean ret = TCDMX_FALSE;

	if ((lLen < 7) || (pbyData == NULL)){
		return TCDMX_FALSE;
  }
	if ((pbyData[0] == 1) && ((pbyData[4] >> 2) == 0x3F) && ((pbyData[5] >> 5) == 0x07)) {
		ret = TCDMX_TRUE;
  }
	return ret;
}

static
gboolean
gst_avidmx_convert_avcc_to_annexb(gchar *pbySrc, gint32 lSrcLen, gchar *ppbyDst, gint32 *plDstLen)
{
	gboolean ret = TCDMX_FALSE;

	if ((pbySrc[0] == 1) && ((pbySrc[4] >> 2) == 0x3F) && ((pbySrc[5] >> 5) == 0x07))
	{
		gchar *p_src = pbySrc + 5;
		gchar *p_dst;
		gint32 nallen;
		gint32 i;

		p_dst = (gchar*)malloc(lSrcLen*2);
		if (p_dst == NULL) {
			GST_ERROR ("out of memory");
			return TCDMX_FALSE;
		}
		*ppbyDst = p_dst;

		gint32 nalcnt = (*p_src++) & 0x1F;
		for (i = 0; i < nalcnt; i++) {
				nallen = (guint32)((guint32)(p_src[0] << 8) | p_src[1]);
				*p_dst++ = 0;
				*p_dst++ = 0;
				*p_dst++ = 0;
				*p_dst++ = 1;
				memcpy(p_dst, p_src+2, nallen);
				p_src += nallen+2;
				p_dst += nallen;
		}

		nalcnt = *p_src++;
		for (i = 0; i < nalcnt; i++) {
				nallen = (guint32)((guint32)(p_src[0] << 8) | p_src[1]);
				*p_dst++ = 0;
				*p_dst++ = 0;
				*p_dst++ = 0;
				*p_dst++ = 1;
				memcpy(p_dst, p_src+2, nallen);
				p_src += nallen+2;
				p_dst += nallen;
		}

		*plDstLen = p_dst - *ppbyDst;
		ret = TCDMX_TRUE;
	}

	return ret;
}


static int 
gst_avidmx_check_mismatch(GstTcDemuxBase  *pstDemuxBase, avi_dmx_handle_t *ph_dmx)
{
	GstTcAviDemux *p_avidmx = GST_AVIDMX(pstDemuxBase);
	av_dmx_getstream_t get_param; 
	av_dmx_outstream_t output;
	avi_dmx_result_t res;
	guchar pby_sour[4];
	get_param.s32StreamType = DMXTYPE_VIDEO;
	get_param.s32StreamBuffSize = 3145728;
	get_param.pbyStreamBuff = g_malloc(3145728);
	if(!get_param.pbyStreamBuff)
	{
		return TCDMX_FALSE;
	}
	
	res = p_avidmx->gExtDmxFunc(DMXOP_GET_STREAM, ph_dmx, &get_param, &output);
	if ( res != AVIERR_NONE ) {
		res = p_avidmx->gExtDmxFunc(DMXOP_GET_LASTERROR, ph_dmx, 0, 0);
		if( get_param.pbyStreamBuff != 0 ) {
			g_free(get_param.pbyStreamBuff);
		}
		return -1;
	}

	pby_sour[0] = output.pbyStreamData[0];
	pby_sour[1] = output.pbyStreamData[1];
	pby_sour[2] = output.pbyStreamData[2];
	pby_sour[3] = output.pbyStreamData[3];
	
	if( res >= 0 ) {
		av_dmx_seek_t		param;
		av_dmx_seek_t		output;	
		param.s32SeekTime = 0;
		param.u32SeekMode = 0;
		
		res = p_avidmx->gExtDmxFunc (DMXOP_SEEK, ph_dmx, &param, &output);
		if ( res != AVIERR_NONE ) {
			res = p_avidmx->gExtDmxFunc(DMXOP_GET_LASTERROR, ph_dmx, 0, 0);
			if( get_param.pbyStreamBuff ) {
				g_free(get_param.pbyStreamBuff);
			}
			return -1;	
		}
	}

	if ((pby_sour[0] == 0) && (pby_sour[1] == 0) && (pby_sour[2] == 0) && (pby_sour[3] == 1)){
		res = 1;
	} else {
		res = 0;
	}

	if( get_param.pbyStreamBuff ) {
		g_free(get_param.pbyStreamBuff);
	}

	return res;
}
