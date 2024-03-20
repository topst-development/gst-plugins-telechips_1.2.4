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
 * SECTION:element-tc-tsdemux
 *
 * FIXME:Describe tc-tsdemux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tc-tsdemux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_tsdemux.h"
#include "gst_tc_demuxio.h"

#if defined(TC_SECURE_MEMORY_COPY)
#define RINGBUFFER_MODE_ENABLE  	(0)
#else
#define RINGBUFFER_MODE_ENABLE  	(1)
#endif

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE0         "video/mpegts"
#define CONTAINER_MIMETYPE1         "video/x-fragmented-mpegts"
#define STRING_DEMUX_LONGNAME       ("Telechips MPEG-TS Demuxer")
#define STRING_DEMUX_DESCRIPTION    ("Demultiplex MPEG-TS into element streams")


static GstStaticPadTemplate gs_stSinkFactory = GST_STATIC_PAD_TEMPLATE (
	"sink",									//template-name
	GST_PAD_SINK,							//direction
	GST_PAD_ALWAYS,							//presence
	GST_STATIC_CAPS (CONTAINER_MIMETYPE0", systemstream = (boolean) TRUE;"
                     CONTAINER_MIMETYPE1", systemstream = (boolean) TRUE;")	//caps
    );

static const guint16 gs_usSupportAudioTags[] = {
	WAVE_FORMAT_MPEGL3,
	WAVE_FORMAT_MPEGL12,
	WAVE_FORMAT_PCM,
	WAVE_FORMAT_A52,
	WAVE_FORMAT_DTS,
	WAVE_FORMAT_AAC,
	WAVE_FORMAT_MS_SWAP,
	0	// end of tags
};

static const guint32 gs_ulSupportVideoFcc[] = {
	FOURCC_MPEG,
	FOURCC_MPG2,
	FOURCC_MP4V,
	FOURCC_H264,
	GST_MAKE_FOURCC ('M', 'V', 'C', ' '),
	FOURCC_WVC1,
	GST_MAKE_FOURCC ('A', 'V', 'S', ' '),
	0	// end of fccs
};

G_DEFINE_TYPE (GstTcTsDemux, gst_tsdmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

/* GObject virtual method implementations */
static void gst_tsdmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_tsdmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_tsdmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_tsdmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestID, tcdmx_result_t * pstResult);
static gboolean gst_tsdmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_tsdmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_tsdmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_tsdmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);

/* GstTcTsDemux private implementations */
static tsd_program_t * gst_tsdmx_get_program_info (GstTcTsDemux * pstDemux, gint32 lProgramNumber);
static tsd_esinfo_t * gst_tsdmx_get_stream_info (tsd_program_t * pstProgram, gint32 lPacketID);



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//

/* initialize the tc-tsdemux's class */
static
void
gst_tsdmx_class_init (
	GstTcTsDemuxClass  * pstDemuxClass
	)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxClass;
	//GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("");

	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overiding
	p_object_class->finalize = gst_tsdmx_finalize;

	// base class overiding
	p_dmxbase_class->vmOpen    = gst_tsdmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_tsdmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_tsdmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_tsdmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_tsdmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_tsdmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_tsdmx_vm_getparam;

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
		, gst_caps_new_simple ("subpicture/x-pgs", NULL)
		, gst_caps_new_simple ("application/x-private-mpegts", NULL)
		);
}


/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static
void
gst_tsdmx_init (
	GstTcTsDemux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	(void)gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), (TCDMX_MODE_SEQUENTIAL));
#if RINGBUFFER_MODE_ENABLE
	gst_tcdmx_set_ringmode (GST_DMXBASE(pstDemux), (TCDMX_TRUE));
#endif
}


static
void
gst_tsdmx_finalize (
	GObject  * pstObject
	)
{
  GstTcTsDemux *p_dmx = GST_TSDMX (pstObject);

  GST_TRACE ("%p", p_dmx);

  //TODO: finialize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define TS_EXT_LIB_NAME ("libtcctsdmx.so")

static gint gst_tsdmx_load_library(GstTcTsDemux *p_tsdmx)
{
  gint result = 0;
  p_tsdmx->pExtDLLModule = dlopen(TS_EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if( p_tsdmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_tsdmx,"[TSDMX] Load library '%s' failed: %s", TS_EXT_LIB_NAME, dlerror());
    result = -1;
  } else {
    GST_DEBUG_OBJECT(p_tsdmx,"[TSDMX] Library '%s' Loaded", TS_EXT_LIB_NAME);

    p_tsdmx->gExtDmxFunc = dlsym(p_tsdmx->pExtDLLModule, "TCC_TS_DMX");
    if( p_tsdmx->gExtDmxFunc == NULL ) {
      GST_ERROR_OBJECT(p_tsdmx,"[TSDMX] p_tsdmx->gExtDmxFunc Error");
      result = -1;
    }
  }

  return result;
}

static void gst_tsdmx_close_library(GstTcTsDemux *p_tsdmx)
{
  if( p_tsdmx->pExtDLLModule != NULL){
    (void)dlclose(p_tsdmx->pExtDLLModule);
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
gst_tsdmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GstTcTsDemux *p_tsdmx = GST_TSDMX(pstDemuxBase);
	tsresult_t res;
	tsd_init_t init_param;
	tsd_info_t *p_init_info = &p_tsdmx->stInitInfo;
	gulong i;
	GstFlowReturn ret;

	tsd_mem_funcs_t	mem_func = {
		g_malloc,
		g_free,
		memcmp,
		memcpy,
		memmove,
		memset,
		g_realloc
		};

	tsd_file_funcs_t file_func = {
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

	if (gst_tsdmx_load_library(p_tsdmx) != 0) {
		ret = GST_FLOW_NOT_SUPPORTED;
	}
	else {
		(void)memset (&init_param, 0, sizeof(tsd_init_t));
	
		init_param.m_pszOpenFileName       = (char*)(void*)pstDemuxBase->pstSinkPad;
		init_param.m_ulSrcType             = SOURCE_TYPE_FILE;
		//init_param.m_ulTsType              = TS_FORMAT_UNKNOWN;
		//init_param.m_ulOption              = 0;
		init_param.m_ulOption              |= (gulong)TSDOPT_DONT_CHECKCRC;	// TSDOPT_CORE_STREAM_ONLY
		//init_param.m_stSecureFuncs.m_pfnSecureProc  = NULL;
		init_param.m_stMemFuncs            = mem_func;
		init_param.m_stFileFuncs           = file_func;
		//init_param.m_ulFIOBlockSizeNormal  = 0;
		//init_param.m_ulFIOBlockSizeSeek    = 0;
	
		if ( CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_STREAMING_SOURCE)) )
		{
			init_param.m_ulOption |= (gulong)TSDOPT_FAST_OPEN;
			init_param.m_ulOption |= (gulong)TSDOPT_DISABLE_INPUT_CACHE;
	
			if (!(CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_TIMESEEK_AVAILABLE) ||
			      CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_BYTESEEK_AVAILABLE))) {
				init_param.m_ulOption |= (gulong)TSDOPT_PROGRESSIVE_FILE;
			}
		}
	
		//demuxing mode
		if ( CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING)) )
		{
			p_tsdmx->ulMultipleDemuxing = 0;
			init_param.m_ulOption |= (gulong)TSDOPT_SELECTIVE;
		}
		else
		{
			if(CHECK_FLAG((pstDemuxBase->ulFlags), (TCDMX_BASE_FLAG_MULTIPLE_STEAM)))
			{
				GST_DEBUG_OBJECT (p_tsdmx, "Multiple stream demuxing enabled");
				p_tsdmx->ulMultipleDemuxing = 1;
			}
		}
	
		//seek fail handling scenario
		p_tsdmx->ulSeekFlags = TSDSEEK_DEFAULT;
		if (CHECK_FLAG((ulFlags),(TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL))) {
			p_tsdmx->ulSeekFlags |= (guint)TSDSEEK_GO_LSYNC_IF_EOS;
		} else if (CHECK_FLAG((ulFlags),(TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL))) {
			p_tsdmx->ulSeekFlags |= (guint)TSDSEEK_GO_EOS_IF_EOS;
		} else {
			GST_DEBUG_OBJECT (p_tsdmx, "Use default seek mode");
		}		
	
		res = p_tsdmx->gExtDmxFunc (TSDOP_OPEN, NULL, &init_param, p_init_info);
		if ( res == TCDMX_FALSE ) {
			tsresult_t reason = p_tsdmx->gExtDmxFunc (TSDOP_GET_LAST_ERROR, NULL, NULL, NULL);
			GST_ERROR_OBJECT (p_tsdmx, "ts open failed - (error: %ld)", reason);
			ret = GST_FLOW_NOT_SUPPORTED;
		} else {	
			p_tsdmx->stSelectInfo = p_init_info->m_stDefProgInfo;
			p_tsdmx->pstProgramList =  p_init_info->m_pstProgramInfoList;
		
			if ( CHECK_FLAG((ulFlags),(TCDMX_BASE_FLAG_STREAMING_SOURCE)) ) {
				if ( CHECK_FLAG((ulFlags),(TCDMX_BASE_FLAG_TIMESEEK_AVAILABLE)) ){
					gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);
					gst_tcdmx_set_timeseek(pstDemuxBase, TCDMX_TRUE);
					p_tsdmx->ulSeekFlags = TSDSEEK_EXTERNAL;
				}
				else if (CHECK_FLAG((ulFlags),(TCDMX_BASE_FLAG_BYTESEEK_AVAILABLE))) {
					gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);	//FIXME currently, seeking is always supported.
				}
				else {
					gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_FALSE);
				}
			}
		
			if(p_init_info->m_bIsSeekEnable != 0U)
			{
				gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);
			}
			else {
				gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_FALSE);
			}
		
			tsd_esinfo_t   esinfo;
			for(i = 0; i < p_init_info->m_pstProgramInfoList->m_ulNbES; i++)
			{
				esinfo = p_init_info->m_pstProgramInfoList->m_pstEsInfoList[i];
				if( esinfo.m_ulEsType == (gulong)ES_TYPE_AUDIO)
				{
					p_tsdmx->ulAudioStreamTotal++;
				}
			}
			if (p_tsdmx->ulAudioStreamTotal > 0U){
				gst_tcdmx_set_total_audiostream(pstDemuxBase, p_tsdmx->ulAudioStreamTotal);
			}
		
			ret = GST_FLOW_OK;
		}
	}
	return ret;
}

static
gboolean
gst_tsdmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcTsDemux *p_tsdmx = GST_TSDMX (pstDemuxBase);
	tsd_selinfo_t *p_select = &p_tsdmx->stSelectInfo;
	tsd_program_t* pProgram = p_tsdmx->pstProgramList;
	tsd_selprog_t selprog;
	tsd_selinfo_t selinfo;
	tshandle_t h_dmx = p_tsdmx->stInitInfo.m_hTsd;

	if( p_select->m_stVideoInfo.m_ulEsPID != 0U ) {
		tc_video_info_t info;
		info.ulFourCC       = (guint)p_select->m_stVideoInfo.m_lFourCC;
		info.ulWidth        = (guint)p_select->m_stVideoInfo.m_lWidth;
		info.ulHeight       = (guint)p_select->m_stVideoInfo.m_lHeight;
		info.ulBitPerPixel  = 0;
		info.ulFrameRate    = (guint)p_select->m_stVideoInfo.m_lFrameRate;
		info.ulDuration     = (guint)p_tsdmx->stSelectInfo.m_lDuration;

		(void)gst_tcdmx_register_stream_info2 (pstDemuxBase, (guint32)TCDMX_TYPE_VIDEO, (guint32)TCDMX_TYPE_VIDEO, &info,
										 NULL, 0, TCDMX_STREAM_FORMAT_BYTE_STREAM, TCDMX_STREAM_ALIGNMENT_NAL);
	}

	if (p_tsdmx->ulAudioStreamTotal > 0U) {
		if(p_tsdmx->ulMultipleDemuxing != 0U) {
			guint i;
			tsd_esinfo_t   esinfo;
			for(i = 0; i < pProgram->m_ulNbES; i++)
			{
				esinfo = pProgram->m_pstEsInfoList[i];
				if(esinfo.m_ulEsType == (gulong)ES_TYPE_AUDIO)
				{
					selprog.m_ulMultipleDemuxing = 1;
					selprog.m_ulSelectType = TSDSEL_AUDIO;
					selprog.m_ulAudioPID = esinfo.m_unInfo.stAudio.m_ulEsPID;
					p_tsdmx->gExtDmxFunc (TSDOP_SEL_PROG, h_dmx, &selprog, &selinfo);
					tc_audio_info_t info;
					memset(&info, 0, sizeof(tc_audio_info_t));
					info.ulFormatTag     = (guint)esinfo.m_unInfo.stAudio.m_ulFormatTag;
					info.ulChannels      = (guint)esinfo.m_unInfo.stAudio.m_lChannels;
					info.ulBlockAlign    = 0;
					info.ulBitPerSample  = (guint)esinfo.m_unInfo.stAudio.m_lBitPerSample;
					info.ulSampleRate    = (guint)esinfo.m_unInfo.stAudio.m_lSamplePerSec;
					info.ulSize          = 0;
					info.ulDuration      = (guint)p_tsdmx->stSelectInfo.m_lDuration;
					if (esinfo.m_unInfo.stAudio.m_ulFormatTag == (guint)WAVE_FORMAT_PCM) {
						info.ulEndian		= 1;		//BigEndian
						info.ulFormatTag	= WAVE_FORMAT_BDPCM;
					} else {
						info.ulEndian		= 0;		//LittleEndian or Don't Care
					}
					//info.pszlanguage	= esinfo.m_unInfo.stAudio.m_szLanguageCode;
					if (esinfo.m_unInfo.stAudio.m_ulSubType == (guint)AUDIO_SUBTYPE_AC3_LOSSLESS) {
						tsd_demux_config_t config;
						config.m_ulOption = (gulong)TSDOPT_CORE_STREAM_ONLY;
						info.ulFormatTag = (guint32)WAVE_FORMAT_TRUEHD_AC3;
						if (p_tsdmx->gExtDmxFunc (TSDOP_DEMUX_CONFIG, h_dmx, &config, NULL) == 0) {
							gint32 ret = p_tsdmx->gExtDmxFunc (TSDOP_GET_LAST_ERROR, h_dmx, NULL, NULL);
							GST_ERROR_OBJECT(p_tsdmx, "configuration failed (%d)", ret);
							return ret;
						}
					}
					(void)gst_tcdmx_register_stream_info (pstDemuxBase, esinfo.m_unInfo.stAudio.m_ulEsPID, (guint32)TCDMX_TYPE_AUDIO, &info,
													NULL, 0);
				}
			}
		}
		else
		{
			if( p_select->m_stAudioInfo.m_ulEsPID != 0U) {
				tc_audio_info_t info;
				memset(&info, 0, sizeof(tc_audio_info_t));
				info.ulFormatTag     = (guint)p_select->m_stAudioInfo.m_ulFormatTag;
				info.ulChannels      = (guint)p_select->m_stAudioInfo.m_lChannels;
				info.ulBlockAlign    = 0;
				info.ulBitPerSample  = (guint)p_select->m_stAudioInfo.m_lBitPerSample;
				info.ulSampleRate    = (guint)p_select->m_stAudioInfo.m_lSamplePerSec;
				info.ulSize          = 0;
				info.ulDuration      = (guint)p_tsdmx->stSelectInfo.m_lDuration;

				if (p_select->m_stAudioInfo.m_ulFormatTag == (guint)WAVE_FORMAT_PCM) {
					info.ulEndian		= 1;		//BigEndian
				} else {
					info.ulEndian		= 0;		//LittleEndian or Don't Care
				}

				//info.pszlanguage	= p_select->m_stAudioInfo.m_szLanguageCode;
				if (p_select->m_stAudioInfo.m_ulSubType == (guint32)AUDIO_SUBTYPE_AC3_LOSSLESS) {
					tshandle_t ex_h_dmx = (tshandle_t)p_tsdmx->stInitInfo.m_hTsd;
					tsd_demux_config_t config;
					config.m_ulOption = (gulong)TSDOPT_CORE_STREAM_ONLY;
					if (p_tsdmx->gExtDmxFunc (TSDOP_DEMUX_CONFIG, ex_h_dmx, &config, NULL) == 0) {
						gint32 ret = p_tsdmx->gExtDmxFunc (TSDOP_GET_LAST_ERROR, ex_h_dmx, NULL, NULL);
						GST_ERROR_OBJECT(p_tsdmx, "configuration failed (%d)", ret);
						return ret;
					}
				}

				(void)gst_tcdmx_register_stream_info (pstDemuxBase, (guint32)TCDMX_TYPE_AUDIO, (guint32)TCDMX_TYPE_AUDIO, &info,
										NULL, 0);
			}
		}
	}

	if( p_select->m_stGraphicsInfo.m_ulEsPID != 0U ) {
		tc_subtitle_info_t info = {(guint32)SUBTITLE_FORMAT_ID_HDMVPGS, {""}};
		(void)gst_tcdmx_register_stream_info (pstDemuxBase, (guint32)TCDMX_TYPE_SUBTITLE, (guint32)TCDMX_TYPE_SUBTITLE, &info,
										NULL, 0);
	}

	if( p_select->m_stSubtitleInfo.m_ulEsPID != 0U ) {
		tc_subtitle_info_t info = {(guint32)SUBTITLE_FORMAT_ID_UNKNOWN, {""}};
		(void)gst_tcdmx_register_stream_info (pstDemuxBase, (guint32)TCDMX_TYPE_SUBTITLE, (guint32)TCDMX_TYPE_SUBTITLE, &info,
										NULL, 0);
	}

	if( p_select->m_stPrivateInfo.m_ulEsPID != 0U ) {
		tc_private_info_t info = {{"application/x-private-mpegts"}};
		(void)gst_tcdmx_register_stream_info (pstDemuxBase, (guint32)TCDMX_TYPE_PRIVATE, (guint32)TCDMX_TYPE_PRIVATE, &info,
										NULL, 0);
	}

	return TCDMX_TRUE;
}

static
gboolean
gst_tsdmx_vm_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestID,
	tcdmx_result_t  * pstResult
	)
{
	GstTcTsDemux *p_tsdmx = GST_TSDMX(pstDemuxBase);
	tshandle_t h_dmx = p_tsdmx->stInitInfo.m_hTsd;
	tsresult_t res;
	tsd_getframe_t param;
	tsd_outframe_t output;
	gboolean ret;

	if( ulRequestID != (guint)TCDMX_TYPE_ANY ) {
		switch( ulRequestID ) {
			case (guint32)TCDMX_TYPE_VIDEO:
				param.m_ulStreamType = (guint32)ES_TYPE_VIDEO;
				break;
			case (guint32)TCDMX_TYPE_AUDIO:
				param.m_ulStreamType = (gulong)ES_TYPE_AUDIO;
				break;
			case (guint32)TCDMX_TYPE_SUBTITLE:
				param.m_ulStreamType = (gulong)ES_TYPE_GRAPHICS;
				break;
			case (guint32)TCDMX_TYPE_PRIVATE:
			default:
				param.m_ulStreamType = (gulong)ES_TYPE_PRIVATE;
				break;
		}
	}

	res = p_tsdmx->gExtDmxFunc (TSDOP_GET_FRAME, h_dmx, &param, &output);
	if ( res == TCDMX_FALSE ) {
		tsresult_t reason = p_tsdmx->gExtDmxFunc (TSDOP_GET_LAST_ERROR, h_dmx, NULL, NULL);
		GST_ERROR_OBJECT (p_tsdmx, "ts demux failed - (error: %ld)", reason);
		ret = TCDMX_FALSE;
	} else {
		switch( output.m_ulStreamType ) {
			case ES_TYPE_VIDEO:
				pstResult->ulStreamType = (guint)TCDMX_TYPE_VIDEO;
				break;
			case ES_TYPE_AUDIO:
				pstResult->ulStreamType = (guint)TCDMX_TYPE_AUDIO;
				break;
			case ES_TYPE_GRAPHICS:
				pstResult->ulStreamType = (guint)TCDMX_TYPE_SUBTITLE;
				break;
			case ES_TYPE_PRIVATE:	
			default:
				pstResult->ulStreamType = (guint)TCDMX_TYPE_PRIVATE;
				break;
		}
		if(output.m_ulStreamType == (guint)ES_TYPE_UNKNOWN){
			pstResult->ulStreamID = 0;
		}
		if( (p_tsdmx->ulMultipleDemuxing == 1U) && (output.m_ulStreamType == (guint)ES_TYPE_AUDIO))
		{
			pstResult->ulStreamID = output.m_ulElementPID;
		} else {
			pstResult->ulStreamID = pstResult->ulStreamType;	//TODO
		}
		pstResult->pBuffer = (gpointer)output.m_pbyDataBuff;
		pstResult->lLength = output.m_lDataLength;
		pstResult->lTimestampMs = output.m_lTimeStamp;
	
		GST_LOG_OBJECT(p_tsdmx,"demux[ID:Type] = [%d:%d] timestamp=%d length=%d",pstResult->ulStreamID,pstResult->ulStreamType,pstResult->lTimestampMs,pstResult->lLength);
		ret = TCDMX_TRUE;
	}
	return ret;
}


static
gboolean
gst_tsdmx_vm_seek (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags,
	gint32            lTargetPts,
	gint32          * plResultPts
	)
{
	GstTcTsDemux *p_tsdmx = GST_TSDMX(pstDemuxBase);
	tshandle_t h_dmx = p_tsdmx->stInitInfo.m_hTsd;
	tsresult_t res;
	tsd_seek_t param = {0,};
	tsd_seek_t result;
	gboolean ret;

	param.m_lSeekTime = lTargetPts;
	param.m_ulFlags = p_tsdmx->ulSeekFlags;
	if((ulFlags & (guint32)GST_SEEK_FLAG_SNAP_AFTER) > 0u)
	{
		param.m_ulFlags |= (gulong)TSDSEEK_TIME_FWD ;  // Fast Forward seek mode
		GST_LOG_OBJECT (p_tsdmx, "Seek mode : 0x%x  GST_SEEK_FLAG_SNAP_AFTER mode\n", ulFlags);
	}
	else if((ulFlags & (guint32)GST_SEEK_FLAG_SNAP_BEFORE) > 0u)
	{
		param.m_ulFlags |= (gulong)TSDSEEK_TIME_BWD ;  // Fast Backward seek mode
		GST_LOG_OBJECT (p_tsdmx, "Seek mode : 0x%x   GST_SEEK_FLAG_SNAP_BEFORE mode\n", ulFlags);
	}
	else
	{
		GST_LOG_OBJECT (p_tsdmx, "Seek mode : 0x%x\n", ulFlags);
	}

	res = p_tsdmx->gExtDmxFunc (TSDOP_FRAME_SEEK, h_dmx, &param, &result);
	if ( res == TCDMX_FALSE ) {
		tsresult_t reason = p_tsdmx->gExtDmxFunc (TSDOP_GET_LAST_ERROR, h_dmx, NULL, NULL);
		GST_ERROR_OBJECT (p_tsdmx, "ts seek failed - (error: %ld)", reason);
		ret = TCDMX_FALSE;
	}
	else {
		*plResultPts = result.m_lSeekTime;

		ret = TCDMX_TRUE;
	}
	return ret;
}


static
gboolean
gst_tsdmx_vm_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	//GstTcTsDemux *p_tsdmx = GST_TSDMX(pstDemuxBase);
	gint32 result_pts;

	(void)gst_tsdmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);	//seek to 0ms

	return TCDMX_TRUE;
}

static
gboolean
gst_tsdmx_vm_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcTsDemux *p_tsdmx = GST_TSDMX(pstDemuxBase);
	tshandle_t h_dmx = p_tsdmx->stInitInfo.m_hTsd;
	gboolean ret;

	if ( h_dmx != NULL ) {
		tsresult_t res = p_tsdmx->gExtDmxFunc (TSDOP_CLOSE, h_dmx, NULL, NULL);
		if( res == TCDMX_FALSE ) {
			tsresult_t reason = p_tsdmx->gExtDmxFunc (TSDOP_GET_LAST_ERROR, h_dmx, NULL, NULL);
			GST_ERROR_OBJECT (p_tsdmx, "ts close failed - (error: %ld)", reason);
		}

		(void)memset ( &p_tsdmx->stInitInfo, 0, sizeof(tsd_info_t) );
		(void)memset ( &p_tsdmx->stSelectInfo, 0, sizeof(tsd_selinfo_t) );

		gst_tsdmx_close_library(p_tsdmx);
		ret = TCDMX_TRUE;
	}
	else {
		GST_ERROR_OBJECT (p_tsdmx, "ts already closed");
		ret =  TCDMX_FALSE;
	}
	return ret;
}

static
gboolean
gst_tsdmx_vm_getparam (
	GstTcDemuxBase * pstDemuxBase,
	gint param,
	void * ret
	)
{
	GST_TRACE_OBJECT (pstDemuxBase, "");
	gboolean *ret_val = (gboolean *)ret;
	gchar **str = (gchar **)ret;
	switch( param )
	{
	case TCDEMUX_PARAM_PUSHMODE_AVAILABLE:
		*ret_val = TCDMX_FALSE;
		break;
	case TCDEMUX_PARAM_QUERY_CONTAINER_NAME:
		*str = g_strdup("MPEG-TS");
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
//	GstTcTsDemux private implementations
//
static
tsd_program_t *
gst_tsdmx_get_program_info (
	GstTcTsDemux  * pstDemux,
	gint32          lProgramNumber
	)
{
	tsd_program_t *p_list = pstDemux->stInitInfo.m_pstProgramInfoList;
	tsd_program_t *p_program = NULL;
	gint32 program_total = pstDemux->stInitInfo.m_lProgramTotal;
	gint32 i;

	for (i = 0; i < program_total; i++) {
		if( p_list->m_ulProgramNum == (gulong)lProgramNumber ){
			p_program = p_list;
		}
		p_list++;
	}

	if ( p_program == NULL ) {
		GST_ERROR_OBJECT (pstDemux, "program not is not found");
	}

	return p_program;
}


static
tsd_esinfo_t *
gst_tsdmx_get_stream_info (
	tsd_program_t  * pstProgram,
	gint32           lPacketID
	)
{
	tsd_esinfo_t *p_list = pstProgram->m_pstEsInfoList;
	tsd_esinfo_t *p_esinfo = NULL;
	gint32 program_total = (gint32)pstProgram->m_ulNbES;
	gint32 i;

	for (i = 0; i < program_total; i++) {
		if( p_list->m_ulElementPID == (gulong)lPacketID ) {
			p_esinfo = p_list;
		}
		p_list++;
	}
	if ( p_esinfo == NULL ) {
		GST_ERROR("element stream is not found");
	}

	return p_esinfo;
}

