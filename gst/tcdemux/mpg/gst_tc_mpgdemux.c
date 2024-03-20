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
 * SECTION:element-tc-mpgdemux
 *
 * FIXME:Describe tc-mpgdemux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tc-mpgdemux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_mpgdemux.h"
#include "gst_tc_demuxio.h"

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE          ("video/mpeg")
#define STRING_DEMUX_LONGNAME       ("Telechips MPG Demuxer")
#define STRING_DEMUX_DESCRIPTION    ("Demultiplex MPG into element streams")


static GstStaticPadTemplate gs_stSinkFactory = GST_STATIC_PAD_TEMPLATE (
	"sink",									//template-name
	GST_PAD_SINK,							//direction
	GST_PAD_ALWAYS,							//presence
	GST_STATIC_CAPS ("video/mpeg, "
        "mpegversion = (int) { 1, 2 }, "
        "systemstream = (boolean) TRUE;" "video/x-cdxa")
    );

static const guint16 gs_usSupportAudioTags[] = {
	WAVE_FORMAT_MPEGL12,
	WAVE_FORMAT_MPEGL3,
	WAVE_FORMAT_AAC,
	WAVE_FORMAT_DTS,
	WAVE_FORMAT_A52,
	WAVE_FORMAT_PCM,
	WAVE_FORMAT_MS_SWAP,
	0	// end of tags
};

static const guint32 gs_ulSupportVideoFcc[] = {
	FOURCC_MPEG,
	FOURCC_H264,
	FOURCC_MP4V,
	0	// end of fccs
};

G_DEFINE_TYPE (GstTcMpgDemux, gst_mpgdmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

/* GObject virtual method implementations */
static void gst_mpgdmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_mpgdmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_mpgdmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mpgdmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestID, tcdmx_result_t * pstResult);
static gboolean gst_mpgdmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_mpgdmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mpgdmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mpgdmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);

/* GstTcMpgDemux private implementations */



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//
/* initialize the tc-mpgdemux's class */
static
void
gst_mpgdmx_class_init (
	GstTcMpgDemuxClass  * pstDemuxClass
	)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxClass;
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("%p", pstDemuxClass);

	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overiding
	p_object_class->finalize = gst_mpgdmx_finalize;

	// base class overiding
	p_dmxbase_class->vmOpen    = gst_mpgdmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_mpgdmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_mpgdmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_mpgdmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_mpgdmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_mpgdmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_mpgdmx_vm_getparam;

	// set demux class details
	gst_tcdmx_set_class_details (p_dmxbase_class
		, STRING_DEMUX_LONGNAME
		, STRING_DEMUX_DESCRIPTION
		);

	// sink pad template
	(void)gst_tcdmx_create_sinkpad_templates (p_dmxbase_class, &gs_stSinkFactory);

	// src pad templates
	(void)gst_tcdmx_create_srcpad_templates ( p_dmxbase_class
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
gst_mpgdmx_init (
	GstTcMpgDemux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	(void)gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), TCDMX_MODE_SEQUENTIAL);
}


static
void
gst_mpgdmx_finalize (
	GObject  * pstObject
	)
{
  GstTcMpgDemux *p_dmx = GST_MPGDMX (pstObject);

  GST_TRACE ("");

  //TODO: finialize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define EXT_LIB_NAME ("libtccmpgdmx.so")

static gint gst_mpgdmx_load_library(GstTcMpgDemux *p_mpgdmx)
{
  gint result = 0;
  p_mpgdmx->pExtDLLModule = dlopen(EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if( p_mpgdmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_mpgdmx,"[MPGDMX] Load library '%s' failed: %s", EXT_LIB_NAME, dlerror());
    result = -1;
  } else {
    GST_DEBUG_OBJECT(p_mpgdmx,"[MPGDMX] Library '%s' Loaded", EXT_LIB_NAME);

    p_mpgdmx->gExtDmxFunc = dlsym(p_mpgdmx->pExtDLLModule, "TCC_MPG_DEC");
    if( p_mpgdmx->gExtDmxFunc == NULL ) {
      GST_ERROR_OBJECT(p_mpgdmx,"[MPGDMX] p_mpgdmx->gExtDmxFunc Error");
      result = -1;
    }
  }

  return result;
}

static void gst_mpgdmx_close_library(GstTcMpgDemux *p_mpgdmx)
{
  if( p_mpgdmx->pExtDLLModule != NULL){
    (void)dlclose(p_mpgdmx->pExtDLLModule);
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

//Temporary buffer for output data concatenating
#define MPG_TEMP_OUTPUT_BUF_SIZE	(1024 * 1024 * 3)

static
GstFlowReturn
gst_mpgdmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	guint32 i;
	GstTcMpgDemux *p_mpgdmx = GST_MPGDMX(pstDemuxBase);
	mpg_handle_t	h_dmx;
	mpg_result_t	res;
	mpg_dec_init_t	init_param;
	mpg_dec_info_t *p_init_info = &p_mpgdmx->stInitInfo;
	mpg_dec_output_t * p_output = &p_mpgdmx->stOutput;
	mpg_dec_sel_stream_t * pstSelStream = &p_mpgdmx->stSelStream;
	mpg_dec_sel_info_t * pstSelStreamInfo = &p_mpgdmx->stSelStreamInfo;
	GstFlowReturn ret;

	mpg_callback_func_t callback_func = {
		g_malloc,
		g_malloc,
		g_free,
		g_free,
		memcpy,
		memset,
		g_realloc,
		memmove,
		tcdmx_io_open,
		tcdmx_io_read,
		tcdmx_io_read,
		tcdmx_io_seek,
		tcdmx_io_tell,
		tcdmx_io_seek64,
		tcdmx_io_tell64,
		tcdmx_io_write,
		tcdmx_io_close,
		tcdmx_io_unlink,
		tcdmx_io_eof,
		tcdmx_io_flush
		};

	if (gst_mpgdmx_load_library(p_mpgdmx) == 0) {

		(void)memset ( &init_param, 0, sizeof(mpg_dec_init_t) );
		(void)memset ( p_output, 0, sizeof(mpg_dec_output_t) );

		p_mpgdmx->pTempOutput = g_malloc(MPG_TEMP_OUTPUT_BUF_SIZE);
		if(p_mpgdmx->pTempOutput != NULL)
		{
			init_param.m_CallbackFunc = callback_func;
			init_param.m_pOpenFileName = (gchar*)(void*)pstDemuxBase->pstSinkPad;
			init_param.m_ReturnDuration = 1;

			//demuxing mode
			if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING))) {
				init_param.m_GetStreamMode = MPG_MODE_INTERLEAVED;
				p_mpgdmx->bNeedDefragment = TCDMX_FALSE;
			}
			else { 
				init_param.m_GetStreamMode = MPG_MODE_SEQUENTIAL;
				p_mpgdmx->bNeedDefragment = TCDMX_TRUE;
			}	
	
			//seek fail handling scenario
			if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL))){
				init_param.m_ulFlags |= (guint)MPGOPT_SEEK_LAST_KEY_AFTER_SEEK_ERROR;
			} else if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL))) {
				init_param.m_ulFlags |= (guint)MPGOPT_SEEK_SEND_EOF_AFTER_SEEK_ERROR;
			} else {
				init_param.m_ulFlags |= (guint)MPGOPT_SEEK_PREV_POS_AFTER_SEEK_ERROR;
			}

			if ( (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_TIMESEEK_AVAILABLE)) ||
				 !CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_BYTESEEK_AVAILABLE))) ) {
				init_param.m_ReturnDuration = 0;
			}
	
			init_param.m_FileCacheSize = 1024 * 1024;
	
			res = p_mpgdmx->gExtDmxFunc(MPG_OP_INIT, &h_dmx, &init_param, p_init_info);
			if ( res != ERR_NONE ) {
				GST_ERROR_OBJECT (p_mpgdmx, "mpg open failed - (error: %d)", res);
				ret = GST_FLOW_NOT_SUPPORTED;
			} else {	
				// Only in case of sequential mode, multiple stream can be demuxed.
				if(init_param.m_GetStreamMode == MPG_MODE_SEQUENTIAL){
					if(CHECK_FLAG((pstDemuxBase->ulFlags), (TCDMX_BASE_FLAG_MULTIPLE_STEAM))){
						GST_LOG_OBJECT (p_mpgdmx, "Multiple stream demuxing enabled");
						pstSelStream->m_ulMultipleDemuxing = 1;
					}
				} else {
					pstSelStream->m_ulMultipleDemuxing = 0;
				}
		
				if(pstSelStream->m_ulMultipleDemuxing == 1U){
					pstSelStream->m_ulSelType = 0;
					if(p_init_info->m_uiTotalAudioNum > 0U){
						pstSelStream->m_ulSelType |= (guint)PACKET_AUDIO;
						pstSelStream->m_ulAudioID = 0;
						for(i = 0;i < p_init_info->m_uiTotalAudioNum;i++) {
							(pstSelStream->m_ulAudioID) |= (MPG_STREAM_INDEX_(i));
						}
					}
					if(p_init_info->m_uiTotalVideoNum > 0U){
						pstSelStream->m_ulSelType |= (guint)PACKET_VIDEO;
						pstSelStream->m_ulVideoID = 0;
						for(i = 0;i < p_init_info->m_uiTotalVideoNum;i++) {
							(pstSelStream->m_ulVideoID) |= (MPG_STREAM_INDEX_(i));
						}
					}
					res = p_mpgdmx->gExtDmxFunc(MPG_OP_SELSTREAM, &h_dmx, pstSelStream, pstSelStreamInfo);
					if(res != ERR_NONE) {
						GST_ERROR_OBJECT (p_mpgdmx, "mpg stream selection failed - (error: %d)", res);
						//Just keep demuxing in single stream mode.(do not return error)
						pstSelStream->m_ulMultipleDemuxing = 0;
					}
				}
		
				p_mpgdmx->hMpgDmx = h_dmx;
			
				if ( CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_TIMESEEK_AVAILABLE)) ) {
					gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);
					gst_tcdmx_set_timeseek(pstDemuxBase, TCDMX_TRUE);
				}
				else {
					if(p_init_info->m_Runningtime != 0U) {
						gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);
					} else {
						gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_FALSE);
					}
				}
			
				if (p_init_info->m_uiTotalAudioNum > 0U) {
					gst_tcdmx_set_total_audiostream(pstDemuxBase, p_init_info->m_uiTotalAudioNum);
				}
		
				ret = GST_FLOW_OK;
			}	
		} else {
			GST_ERROR_OBJECT (p_mpgdmx, "mpg buffer allocation failed)");
			ret = GST_FLOW_NOT_SUPPORTED;
		}
	}
	else {
		ret = GST_FLOW_NOT_SUPPORTED;
	}
	return ret;
}

static
gboolean
gst_mpgdmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	guint32 i;
	GstTcMpgDemux *p_mpgdmx = GST_MPGDMX(pstDemuxBase);
	mpg_handle_t	h_dmx = p_mpgdmx->hMpgDmx;

	mpg_dec_info_t *p_init_info = &p_mpgdmx->stInitInfo;
	mpg_dec_sel_stream_t * pstSelStream = &p_mpgdmx->stSelStream;
	mpg_dec_sel_info_t * pstSelStreamInfo = &p_mpgdmx->stSelStreamInfo;

	if(pstSelStream->m_ulMultipleDemuxing == 1U)
	{
		tc_video_info_t vinfo;
		tc_audio_info_t ainfo;
		mpg_video_info_t *p_vinfo;
		mpg_audio_info_t *p_ainfo;

		for(i = 0;i < pstSelStreamInfo->m_ulNumVideoStream;i++)
		{
			p_vinfo = pstSelStreamInfo->m_pstVideoInfo[i];
			vinfo.ulFourCC       = p_vinfo->m_FourCC;
			vinfo.ulWidth        = p_vinfo->m_Width;
			vinfo.ulHeight       = p_vinfo->m_Height;
			vinfo.ulBitPerPixel  = 0;
			vinfo.ulFrameRate    = p_vinfo->m_FrameRate;
			vinfo.ulDuration     = p_init_info->m_Runningtime;

			if ((p_vinfo->m_FourCC == FOURCC_mpeg) || (p_vinfo->m_FourCC == FOURCC_mp4v)) {
				GST_INFO_OBJECT (p_mpgdmx, "video fourcc is mpeg/mp4v, set ring-mode");
				gst_tcdmx_set_ringmode (p_mpgdmx, (TCDMX_TRUE));
			}

			(void)gst_tcdmx_register_stream_info2 (pstDemuxBase,
											 p_vinfo->m_Index,
											 (gboolean)TCDMX_TYPE_VIDEO,
											 &vinfo,
											 NULL,
											 0,
											 TCDMX_STREAM_FORMAT_BYTE_STREAM,
											 TCDMX_STREAM_ALIGNMENT_NONE);
		}
		for(i = 0;i < pstSelStreamInfo->m_ulNumAudioStream;i++)
		{
			memset(&ainfo, 0, sizeof(tc_audio_info_t));
			p_ainfo = pstSelStreamInfo->m_pstAudioInfo[i];
			ainfo.ulFormatTag     = p_ainfo->m_FormatId;
			ainfo.ulChannels      = p_ainfo->m_Channels;
			ainfo.ulBlockAlign    = 0;
			ainfo.ulBitPerSample  = p_ainfo->m_BitsPerSample;
			ainfo.ulSampleRate    = p_ainfo->m_SamplePerSec;
			ainfo.ulSize          = 0;
			ainfo.ulDuration      = p_init_info->m_Runningtime;

			if (p_ainfo->m_FormatId == (guint)WAVE_FORMAT_PCM)
			{
				ainfo.ulFormatTag	= WAVE_FORMAT_DVDPCM;
				ainfo.ulEndian		= 1;		//BigEndian
			}
			else {
				ainfo.ulEndian		= 0;		//LittleEndian or Don't Care
			}
			(void)gst_tcdmx_register_stream_info (pstDemuxBase,
											p_ainfo->m_Index,
											(gboolean)TCDMX_TYPE_AUDIO,
											&ainfo,
											NULL,
											0);
		}
	}
	else
	{
		if (p_init_info->m_uiTotalVideoNum > 0U) {
			mpg_video_info_t *p_info = &(p_init_info->m_pszVideoInfo[p_init_info->m_uiDefaultVideo]);
			tc_video_info_t info;
			info.ulFourCC       = p_info->m_FourCC;
			info.ulWidth        = p_info->m_Width;
			info.ulHeight       = p_info->m_Height;
			info.ulBitPerPixel  = 0;
			info.ulFrameRate    = p_info->m_FrameRate;
			info.ulDuration     = p_init_info->m_Runningtime;

			if(p_mpgdmx->bNeedDefragment == TCDMX_TRUE)
			{
				(void)gst_tcdmx_register_stream_info2 (pstDemuxBase,
											 p_info->m_Index,
											 (gboolean)TCDMX_TYPE_VIDEO,
											 &info,
											 NULL,
											 0,
											 TCDMX_STREAM_FORMAT_BYTE_STREAM,
											 TCDMX_STREAM_ALIGNMENT_NONE);
			}
			else
			{
				(void)gst_tcdmx_register_stream_info (pstDemuxBase,
											p_info->m_Index,
											(gboolean)TCDMX_TYPE_VIDEO,
											&info,
											NULL,
											0);
			}
		}

		if (p_init_info->m_uiTotalAudioNum > 0U) {
			mpg_audio_info_t *p_info = &(p_init_info->m_pszAudioInfo[p_init_info->m_uiDefaultAudio]);
			tc_audio_info_t info;
			memset(&info, 0, sizeof(tc_audio_info_t));
			info.ulFormatTag     = p_info->m_FormatId;
			info.ulChannels      = p_info->m_Channels;
			info.ulBlockAlign    = 0;
			info.ulBitPerSample  = p_info->m_BitsPerSample;
			info.ulSampleRate    = p_info->m_SamplePerSec;
			info.ulSize          = 0;
			info.ulDuration      = p_init_info->m_Runningtime;

			if (p_info->m_FormatId == (guint)WAVE_FORMAT_PCM)
			{
				info.ulFormatTag	= WAVE_FORMAT_DVDPCM;
				info.ulEndian		= 1;		//BigEndian
			}
			else {
				info.ulEndian		= 0;		//LittleEndian or Don't Care
			}
			(void)gst_tcdmx_register_stream_info (pstDemuxBase,
											p_info->m_Index,
											(gboolean)TCDMX_TYPE_AUDIO,
											&info,
											NULL,
											0);
		}
	}

	return TCDMX_TRUE;
}

static
gboolean
gst_mpgdmx_vm_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestID,
	tcdmx_result_t  * pstResult
	)
{
	GstTcMpgDemux *p_mpgdmx = GST_MPGDMX(pstDemuxBase);
	mpg_handle_t	h_dmx = p_mpgdmx->hMpgDmx;
	mpg_result_t res;
	mpg_dec_input_t get_param;
	mpg_dec_output_t * output = &p_mpgdmx->stOutput;
	guchar *	pTempBuf = p_mpgdmx->pTempOutput;
	gint pTotalLength;
	gboolean ret;

	res = p_mpgdmx->gExtDmxFunc(MPG_OP_GETSTREAM, &h_dmx, &get_param, output);
	if ( res != ERR_NONE ) {
		GST_ERROR_OBJECT (p_mpgdmx, "mpg demux failed - (error: %d)", res);
		ret = TCDMX_FALSE;
	} else {
		(void)memcpy(pTempBuf, output->m_pData, (guint)output->m_iDataLength);
		pTotalLength = output->m_iDataLength;

		switch( output->m_iType ) {
			case PACKET_VIDEO:
				pstResult->ulStreamType = (guint)TCDMX_TYPE_VIDEO;
				break;
			case PACKET_AUDIO:
				pstResult->ulStreamType = (guint)TCDMX_TYPE_AUDIO;
				break;
			case PACKET_SUBTITLE:
			default:
				pstResult->ulStreamType = (guint)TCDMX_TYPE_SUBTITLE;
				break;
		}
		pstResult->ulStreamID = output->m_iStreamIdx;
		pstResult->pBuffer = pTempBuf;
		pstResult->lLength = pTotalLength;
		pstResult->lTimestampMs = (gint)output->m_iRefTime;

		GST_LOG_OBJECT(p_mpgdmx,"demux[ID:Type] = [%d:%d] timestamp=%d length=%d",pstResult->ulStreamID,pstResult->ulStreamType,pstResult->lTimestampMs,pstResult->lLength);

		ret = TCDMX_TRUE;
	}

	return ret;
}


static
gboolean
gst_mpgdmx_vm_seek (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags,
	gint32            lTargetPts,
	gint32          * plResultPts
	)
{
	GstTcMpgDemux *p_mpgdmx = GST_MPGDMX(pstDemuxBase);
	mpg_handle_t		h_dmx = p_mpgdmx->hMpgDmx;
	mpg_result_t		res;
	mpg_dec_input_t		param;
	mpg_dec_output_t	* result = &p_mpgdmx->stOutput;
	gboolean ret;

	GST_DEBUG_OBJECT (p_mpgdmx, "Seeking to %d ms", lTargetPts);
	param.m_iSeekTimeMSec  = lTargetPts;
	param.m_iSeekMode = SEEK_RESET;
	param.m_iSeekMode |= (guint32)SEEK_MODE_ABSOLUTE;
	if((ulFlags & (guint32)GST_SEEK_FLAG_SNAP_AFTER) != 0u)
	{
		param.m_iSeekMode |= (guint32)SEEK_DIR_FWD ;  // Fast Forward seek mode
		GST_LOG_OBJECT (p_mpgdmx, "Seek mode : 0x%x  GST_SEEK_FLAG_SNAP_AFTER mode\n", ulFlags);
	}
	else if((ulFlags & (guint32)GST_SEEK_FLAG_SNAP_BEFORE) != 0u)
	{
		param.m_iSeekMode |= (guint32)SEEK_DIR_BWD ;  // Fast Backward seek mode
		GST_LOG_OBJECT (p_mpgdmx, "Seek mode : 0x%x   GST_SEEK_FLAG_SNAP_BEFORE mode\n", ulFlags);
	}
	else
	{
		param.m_iSeekMode |= (guint32)SEEK_DIR_FWD ;
		GST_LOG_OBJECT (p_mpgdmx, "Seek mode : 0x%x\n", ulFlags);
	}

	res = p_mpgdmx->gExtDmxFunc (MPG_OP_SEEK, &h_dmx, &param, result);
	if ( res != ERR_NONE ) {
		GST_ERROR_OBJECT (p_mpgdmx, "mpg seek failed - (error: %d)", res);
		ret = TCDMX_FALSE;
	} else {
		*plResultPts = (gint32)result->m_iRefTime;
		ret = TCDMX_TRUE;
	}
	return ret;
}


static
gboolean
gst_mpgdmx_vm_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcMpgDemux *p_mpgdmx = GST_MPGDMX(pstDemuxBase);
	gint32 result_pts;

	(void)gst_mpgdmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);	//seek to 0ms

	return TCDMX_TRUE;
}

static
gboolean
gst_mpgdmx_vm_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcMpgDemux *p_mpgdmx = GST_MPGDMX(pstDemuxBase);
	mpg_handle_t	h_dmx = p_mpgdmx->hMpgDmx;
	gboolean ret;

	if(p_mpgdmx->pTempOutput != NULL)
	{
		g_free(p_mpgdmx->pTempOutput);
		p_mpgdmx->pTempOutput = NULL;
	}

	if ( h_dmx != 0U ) {
		mpg_dec_input_t		param;
		mpg_dec_output_t	* result = &p_mpgdmx->stOutput;
		mpg_result_t res = p_mpgdmx->gExtDmxFunc (MPG_OP_CLOSE, &h_dmx, &param, result);
		if( res != ERR_NONE ) {
			GST_ERROR_OBJECT (p_mpgdmx, "mpg close failed - (error: %d)", res);
		}

		p_mpgdmx->hMpgDmx = 0;
		(void)memset ( &p_mpgdmx->stInitInfo, 0, sizeof(mpg_dec_info_t) );

		gst_mpgdmx_close_library(p_mpgdmx);
		ret = TCDMX_TRUE;
	}
	else {
		GST_ERROR_OBJECT (p_mpgdmx, "mpg already closed");
		ret = TCDMX_FALSE;
	}
	return ret;
}

static
gboolean
gst_mpgdmx_vm_getparam (
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
		*str = g_strdup("MPEG-PS");
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
//	GstTcMpgDemux private implementations
//
