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
 * SECTION:element-tc-mp4demux
 *
 * FIXME:Describe tc-mp4demux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tcmp4demux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_mp4demux.h"
#include "gst_tc_demuxio.h"


/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE          ("video/qt-pcm; video/mp4; video/quicktime; video/mj2; audio/x-m4a; application/x-3gp")
#define STRING_DEMUX_LONGNAME       ("Telechips MP4 Demuxer")
#define STRING_DEMUX_DESCRIPTION    ("Demultiplex MP4 into element streams")


static GstStaticPadTemplate gs_stSinkFactory = GST_STATIC_PAD_TEMPLATE (
	"sink",									//template-name
	GST_PAD_SINK,							//direction
	GST_PAD_ALWAYS,							//presence
	GST_STATIC_CAPS (CONTAINER_MIMETYPE)
    );

G_DEFINE_TYPE (GstTcMp4Demux, gst_mp4dmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

/* GObject virtual method implementations */
static void gst_mp4dmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_mp4dmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_mp4dmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mp4dmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestID, tcdmx_result_t * pstResult);
static gboolean gst_mp4dmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_mp4dmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mp4dmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mp4dmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);

/* GstTcMpgDemux private implementations */



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//
/* initialize the tc-mp4demux's class */
static
void
gst_mp4dmx_class_init (
	GstTcMp4DemuxClass  * pstDemuxClass
	)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxClass;
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("%p", pstDemuxClass);

	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overiding
	p_object_class->finalize = gst_mp4dmx_finalize;

	// base class overiding
	p_dmxbase_class->vmOpen    = gst_mp4dmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_mp4dmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_mp4dmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_mp4dmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_mp4dmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_mp4dmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_mp4dmx_vm_getparam;

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
		, GST_CAPS_ANY
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
gst_mp4dmx_init (
	GstTcMp4Demux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	(void)gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), TCDMX_MODE_SELECTIVE);
}


static
void
gst_mp4dmx_finalize (
	GObject  * pstObject
	)
{
  GstTcMp4Demux *p_dmx = GST_MP4DMX (pstObject);

  GST_TRACE ("");

  //TODO: finialize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define MP4_EXT_LIB_NAME ("libtccmp4dmx.so")

static gint gst_mp4dmx_load_library(GstTcMp4Demux *p_mp4dmx)
{
  gint result = 0;
  p_mp4dmx->pExtDLLModule = dlopen(MP4_EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if( p_mp4dmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_mp4dmx,"[MP4DMX] Load library '%s' failed: %s", MP4_EXT_LIB_NAME, dlerror());
    result =  -1;
  } else {
    GST_DEBUG_OBJECT(p_mp4dmx,"[MP4DMX] Library '%s' Loaded", MP4_EXT_LIB_NAME);

    p_mp4dmx->gExtDmxFunc = dlsym(p_mp4dmx->pExtDLLModule, "TCC_MP4_DMX");
    if( p_mp4dmx->gExtDmxFunc == NULL ) {
      GST_ERROR_OBJECT(p_mp4dmx,"[MP4DMX] p_mp4dmx->gExtDmxFunc Error");
      result =  -1;
    }
  }

  return result;
}

static void gst_mp4dmx_close_library(GstTcMp4Demux *p_mp4dmx)
{
  if( p_mp4dmx->pExtDLLModule != NULL) {
    (void)dlclose(p_mp4dmx->pExtDLLModule);
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
gst_mp4dmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GstTcMp4Demux *		p_mp4dmx = GST_MP4DMX(pstDemuxBase);
	mp4_dmx_handle_t	h_dmx;
	mp4_dmx_result_t	res;
	mp4_dmx_init_t		init_param;
	mp4_dmx_info_t *	p_init_info = &p_mp4dmx->stInitInfo;
	mp4_dmx_output_t *	p_output = &p_mp4dmx->stOutput;
	GstFlowReturn		ret;

	mp4_dmx_callback_func_t callback_func = {
		g_malloc,
		g_malloc,
		g_free,
		g_free,
		memcpy,
		memset,
		g_realloc,
		memmove,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		tcdmx_io_open,
		tcdmx_io_read,
		tcdmx_io_seek,
		tcdmx_io_tell,
		NULL,
		tcdmx_io_close,
		tcdmx_io_unlink,
		tcdmx_io_eof,
		tcdmx_io_flush,
		tcdmx_io_seek64,
		tcdmx_io_tell64
		};

	if (gst_mp4dmx_load_library(p_mp4dmx) == 0) {

		(void)memset ( &init_param, 0, sizeof(mp4_dmx_init_t) );
		(void)memset ( p_output, 0, sizeof(mp4_dmx_output_t) );

		init_param.m_sCallbackFunc = callback_func;
		init_param.m_pOpenFileName = (char*)(void*)pstDemuxBase->pstSinkPad;

		init_param.m_uExtra.m_sCache.m_iAudioFileCacheSize = 16*1024;
		init_param.m_uExtra.m_sCache.m_iVideoFileCacheSize = 16*1024;
		init_param.m_uExtra.m_sCache.m_iSubtitleFileCacheSize = 1024;
		init_param.m_uExtra.m_sCache.m_iSTCOCacheSize = 256;
		init_param.m_uExtra.m_sCache.m_iSTSCCacheSize = 256;
		init_param.m_uExtra.m_sCache.m_iSTSSCacheSize = 256;
		init_param.m_uExtra.m_sCache.m_iSTSZCacheSize = 256;
		init_param.m_uExtra.m_sCache.m_iSTTSCacheSize = 256;
		init_param.m_uExtra.m_sCache.m_iDefaultCacheSize = 64*256;

		//init_param.m_iLastkeySearchEnable = 0;
		init_param.m_iSaveInternalIndex = 1; // allocate memory for stbl atoms
		//init_param.m_iPreventFseekToEof = 0;
		init_param.m_iSeekKeyframeEnable = 1;
		if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING))) {
			//init_param.m_iSequentialGetStream = 0;
			init_param.m_iRealDurationCheckEnable = 1;
		}
		else { // (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_SEQUENTIAL_DEMUXING))) {
			(void)g_printf ("demuxer <Sequential mode>\n");
			init_param.m_iSequentialGetStream = 1;
			//init_param.m_iRealDurationCheckEnable = 0;
		}

		res = p_mp4dmx->gExtDmxFunc(DMXOP_OPEN, &h_dmx, &init_param, p_init_info);
		if ( res != ERR_NONE ) {
			GST_ERROR_OBJECT (p_mp4dmx, "mp4 open failed - (error: %d)", res);
			ret = GST_FLOW_NOT_SUPPORTED;
		} else {

			mp4_dmx_options_t options;
			//--------------------------------------------------------
			// [*] MP4DMXOP_GET_VERSION operation
			//--------------------------------------------------------
			char* psz_version = NULL;
			char* psz_build_date = NULL;
			p_mp4dmx->gExtDmxFunc (MP4DMXOP_GET_VERSION,
				NULL,
				(void*)&psz_version,
				(void*)&psz_build_date );
			(void)g_printf ("\x1b[1;34mMP4 Version Info.: %s (%s) Sequential %d \x1b[0m \n", psz_version, psz_build_date, init_param.m_iSequentialGetStream );

			//seek fail handling scenario
			if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL))) {
				options.m_ulOptSeekOverLastKey = MP4DOPT_LAST_KEYFRAME;
			}
			else if (CHECK_FLAG((ulFlags), (TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL))) {
				options.m_ulOptSeekOverLastKey = MP4DOPT_TO_END_OF_FILE;
			}
			else {
				options.m_ulOptSeekOverLastKey = MP4DOPT_DO_NOTHING;
			}

			p_mp4dmx->gExtDmxFunc (MP4DMXOP_SET_OPTIONS,
				&h_dmx,
				&options,
				NULL);

			p_mp4dmx->stInput.m_pPacketBuff = g_malloc(GST_MP4DMX_BUFFER_SIZE);
			if( p_mp4dmx->stInput.m_pPacketBuff == NULL ) {
				GST_ERROR_OBJECT (p_mp4dmx, "mp4 buffer allocation failed");
				ret = GST_FLOW_NOT_SUPPORTED;
			} else {
				p_mp4dmx->stInput.m_iPacketBuffSize = GST_MP4DMX_BUFFER_SIZE;

				p_mp4dmx->hMp4Dmx = h_dmx;
				p_mp4dmx->lLastPTS = 0;
				p_mp4dmx->lErrorCount = 0;

				if (p_init_info->m_sAudioInfo.m_iAudioTrackCount > 0) {
					gst_tcdmx_set_total_audiostream(pstDemuxBase, 1);	//temp code
				}
				ret = GST_FLOW_OK;
			}
		}
	}
	else {
		ret = GST_FLOW_NOT_SUPPORTED;
	}

	return ret;
}

static
gboolean
gst_mp4dmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcMp4Demux *		p_mp4dmx = GST_MP4DMX(pstDemuxBase);
	mp4_dmx_handle_t	h_dmx = p_mp4dmx->hMp4Dmx;

	mp4_dmx_info_t *	p_init_info = &p_mp4dmx->stInitInfo;

	if (p_mp4dmx->stInitInfo.m_sFileInfo.m_iSeekable == 1){
		gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_TRUE);
	} else {
		gst_tcdmx_set_seekable(pstDemuxBase, TCDMX_FALSE);
	}

	if (p_init_info->m_sVideoInfo.m_iVideoTrackCount > 0) {
		tc_video_info_t info;
		info.ulFourCC       = (guint)p_init_info->m_sVideoInfo.m_iFourCC;
		info.ulWidth        = (guint)p_init_info->m_sVideoInfo.m_iWidth;
		info.ulHeight       = (guint)p_init_info->m_sVideoInfo.m_iHeight;
		info.ulBitPerPixel  = 0;
		info.ulFrameRate    = (guint)p_init_info->m_sVideoInfo.m_iFrameRate;
		info.ulDuration     = (guint)p_init_info->m_sFileInfo.m_iRunningtime;

		GST_DEBUG_OBJECT (p_mp4dmx, "m_iVideoTrackCount=%d m_iDefaultVideoTrackIndex=%d",p_init_info->m_sVideoInfo.m_iVideoTrackCount,p_init_info->m_sVideoInfo.m_iDefaultVideoTrackIndex);
		(void)gst_tcdmx_register_stream_info (pstDemuxBase,
										(guint)p_init_info->m_sVideoInfo.m_iDefaultVideoTrackIndex,	//TCDMX_TYPE_VIDEO,
										(gboolean)TCDMX_TYPE_VIDEO,
										&info,
										p_init_info->m_sVideoInfo.m_pExtraData,
										p_init_info->m_sVideoInfo.m_iExtraDataLen);
	}

	if (p_init_info->m_sAudioInfo.m_iAudioTrackCount > 0) {
		tc_audio_info_t info;
		memset(&info, 0, sizeof(tc_audio_info_t));
		info.ulFormatTag     = (guint)p_init_info->m_sAudioInfo.m_iFormatId;
		info.ulChannels      = (guint)p_init_info->m_sAudioInfo.m_iChannels;
		info.ulBlockAlign    = 0;
		info.ulBitPerSample  = (guint)p_init_info->m_sAudioInfo.m_iBitsPerSample;
		info.ulSampleRate    = (guint)p_init_info->m_sAudioInfo.m_iSamplePerSec;
		info.ulSize          = 0;
		info.ulDuration      = (guint)p_init_info->m_sFileInfo.m_iRunningtime;

		if (info.ulFormatTag == (guint)WAVE_FORMAT_PCM) {
			info.ulEndian		= 0;		//LittleEndian or Don't Care
		}

		if (((gshort)info.ulFormatTag == WAVE_FORMAT_VORBIS) ||  /* ogg/vorbis mode */
			((gshort)info.ulFormatTag == WAVE_FORMAT_VORBIS1) || /* ogg/vorbis mode 1 */
			((gshort)info.ulFormatTag == WAVE_FORMAT_VORBIS2) || /* ogg/vorbis mode 2 */
			((gshort)info.ulFormatTag == WAVE_FORMAT_VORBIS3) || /* ogg/vorbis mode 3 */
			((gshort)info.ulFormatTag == WAVE_FORMAT_VORBIS1PLUS) || /* ogg/vorbis mode 1+ */
			((gshort)info.ulFormatTag == WAVE_FORMAT_VORBIS2PLUS) || /* ogg/vorbis mode 2+ */
			((gshort)info.ulFormatTag == WAVE_FORMAT_VORBIS3PLUS)    /* ogg/vorbis mode 3+ */
		 )
		{
			p_mp4dmx->lFlagVorbisInit = 3;
			gst_tcdmx_search_vorbis_header(p_init_info->m_sAudioInfo.m_pExtraData, (guint32)p_init_info->m_sAudioInfo.m_iExtraDataLen, &p_mp4dmx->stVorbisHeaderInfo);
		}
		GST_DEBUG_OBJECT (p_mp4dmx, "m_iAudioTrackCount=%d m_iDefaultAudioTrackIndex=%d",p_init_info->m_sAudioInfo.m_iAudioTrackCount,p_init_info->m_sAudioInfo.m_iDefaultAudioTrackIndex);

		(void)gst_tcdmx_register_stream_info (pstDemuxBase,
										(guint32)p_init_info->m_sAudioInfo.m_iDefaultAudioTrackIndex,
										(gboolean)TCDMX_TYPE_AUDIO,
										&info,
										p_init_info->m_sAudioInfo.m_pExtraData,
										p_init_info->m_sAudioInfo.m_iExtraDataLen);
	}

	if (p_init_info->m_stSubtitleInfo.m_iSubtitleTrackCount > 0) {
		tc_subtitle_info_t info;
		memset(&info, 0, sizeof(tc_subtitle_info_t));
		switch (p_init_info->m_stSubtitleInfo.m_ulSubtitleType) {
		case SUBTITLETYPE_SRT:
			info.ulFormatId = (guint32)SUBTITLE_FORMAT_ID_SRT;
			break;
		case SUBTITLETYPE_SSA:
			info.ulFormatId = (guint32)SUBTITLE_FORMAT_ID_SSA;
			break;
		case SUBTITLETYPE_USF:
			info.ulFormatId = (guint32)SUBTITLE_FORMAT_ID_USF;
			break;
		case SUBTITLETYPE_BMP:
			info.ulFormatId = (guint32)SUBTITLE_FORMAT_ID_BMP;
			break;
		case SUBTITLETYPE_VOBSUB:
			info.ulFormatId = (guint32)SUBTITLE_FORMAT_ID_VOBSUB;
			break;
		case SUBTITLETYPE_KATE:
			info.ulFormatId = (guint32)SUBTITLE_FORMAT_ID_KATE;
			break;
		default:
			info.ulFormatId = (guint32)SUBTITLE_FORMAT_ID_UNKNOWN;
			break;
		}
		info.pszlanguage = p_init_info->m_stSubtitleInfo.m_pszLanguage;

		(void)gst_tcdmx_register_stream_info (pstDemuxBase,
										(guint32)p_init_info->m_stSubtitleInfo.m_iCurrentSubtitleTrackIndex,
										(gboolean)TCDMX_TYPE_SUBTITLE,
										&info,
										p_init_info->m_stSubtitleInfo.m_pSubtitleInfo,
										(gint32)p_init_info->m_stSubtitleInfo.m_ulSubtitleInfoSize);
	}

	return TCDMX_TRUE;
}

static
gboolean
gst_mp4dmx_vm_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestType,
	tcdmx_result_t  * pstResult
	)
{
	GstTcMp4Demux *		p_mp4dmx = GST_MP4DMX(pstDemuxBase);
	mp4_dmx_handle_t	h_dmx = p_mp4dmx->hMp4Dmx;
	mp4_dmx_result_t	res;
	mp4_dmx_input_t * 	get_param = &p_mp4dmx->stInput;
	mp4_dmx_output_t *	output = &p_mp4dmx->stOutput;
	gboolean ret;

	if( ulRequestType == (guint)TCDMX_TYPE_VIDEO ) {
		get_param->m_iPacketType = DMXTYPE_VIDEO;
	} else if (ulRequestType == (guint)TCDMX_TYPE_AUDIO ) {
		get_param->m_iPacketType = DMXTYPE_AUDIO;
	} else if (ulRequestType == (guint)TCDMX_TYPE_SUBTITLE ) {
		get_param->m_iPacketType = DMXTYPE_SUBTITLE;
	} else {
		get_param->m_iPacketType = DMXTYPE_NONE;
	}

	if (((get_param->m_iPacketType == DMXTYPE_AUDIO) || (get_param->m_iPacketType == DMXTYPE_NONE) )
		&& (p_mp4dmx->lFlagVorbisInit != 0))
	{
		switch(p_mp4dmx->lFlagVorbisInit)
		{
			case 3:
				p_mp4dmx->lFlagVorbisInit = 2;
				pstResult->pBuffer      = p_mp4dmx->stVorbisHeaderInfo.pData1;
				pstResult->lLength      = p_mp4dmx->stVorbisHeaderInfo.size1;
				break;
			case 2:
				p_mp4dmx->lFlagVorbisInit = 1;
				pstResult->pBuffer      = p_mp4dmx->stVorbisHeaderInfo.pData2;
				pstResult->lLength      = p_mp4dmx->stVorbisHeaderInfo.size2;
				break;
			case 1:
			default:
				p_mp4dmx->lFlagVorbisInit = 0;
				pstResult->pBuffer      = p_mp4dmx->stVorbisHeaderInfo.pData3;
				pstResult->lLength      = p_mp4dmx->stVorbisHeaderInfo.size3;
				break;
		}
		pstResult->ulStreamID = (guint32)p_mp4dmx->stInitInfo.m_sAudioInfo.m_iDefaultAudioTrackIndex;
		pstResult->ulStreamType   = (guint32)TCDMX_TYPE_AUDIO;
		pstResult->lTimestampMs = 0;

		ret = TCDMX_TRUE;
	}
	else 
	{

READ_AGAIN:
		res = p_mp4dmx->gExtDmxFunc(DMXOP_GET_STREAM, &h_dmx, get_param, output);

		if ( res != ERR_NONE ) {
			GST_ERROR_OBJECT (p_mp4dmx, "mp4 demux failed - (error: %d)", res);
			ret = TCDMX_FALSE;
		}
		else {
			switch( output->m_iPacketType ) {
				case DMXTYPE_VIDEO:
					pstResult->ulStreamType = (guint)TCDMX_TYPE_VIDEO;
					p_mp4dmx->lLastVideoPTS = output->m_iTimeStamp;
					pstResult->ulStreamID = (guint)p_mp4dmx->stInitInfo.m_sVideoInfo.m_iDefaultVideoTrackIndex;
					break;
				case DMXTYPE_AUDIO:
					pstResult->ulStreamType = (guint)TCDMX_TYPE_AUDIO;
					p_mp4dmx->lLastAudioPTS = output->m_iTimeStamp;
					pstResult->ulStreamID = (guint)p_mp4dmx->stInitInfo.m_sAudioInfo.m_iDefaultAudioTrackIndex;
					break;
				case DMXTYPE_SUBTITLE:
				default:
					pstResult->ulStreamType = (guint)TCDMX_TYPE_SUBTITLE;
					pstResult->ulStreamID = (guint)p_mp4dmx->stInitInfo.m_stSubtitleInfo.m_iCurrentSubtitleTrackIndex;
					break;
			}
		
			pstResult->pBuffer = output->m_pPacketData;
			pstResult->lLength = output->m_iPacketSize;
			pstResult->lTimestampMs = output->m_iTimeStamp;
			p_mp4dmx->lErrorCount = 0;
		
			GST_DEBUG_OBJECT (p_mp4dmx, "demux[ID:Type] = [%d:%d] timestamp=%d",pstResult->ulStreamID,pstResult->ulStreamType,pstResult->lTimestampMs);
			ret = TCDMX_TRUE;
		}
	}
	return ret;
}


static
gboolean
gst_mp4dmx_vm_seek (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags,
	gint32            lTargetPts,
	gint32          * plResultPts
	)
{
	GstTcMp4Demux *p_mp4dmx = GST_MP4DMX(pstDemuxBase);
	mp4_dmx_handle_t	h_dmx = p_mp4dmx->hMp4Dmx;
	mp4_dmx_result_t	res;
	mp4_dmx_seek_t		param;
	mp4_dmx_output_t *	result = &p_mp4dmx->stOutput;
	gboolean ret;

	if ((lTargetPts != 0) && (p_mp4dmx->stInitInfo.m_sFileInfo.m_iSeekable == 0)) {
		GST_ERROR_OBJECT (p_mp4dmx, "mp4 seek failed - non seekable");
		ret = TCDMX_FALSE;
	} else {

		param.m_lSeekTime  = lTargetPts;
		param.m_ulSeekMode = 0;

		res = p_mp4dmx->gExtDmxFunc (DMXOP_SEEK, &h_dmx, &param, result);
		if ( res != ERR_NONE ) {
			GST_ERROR_OBJECT (p_mp4dmx, "mp4 seek failed - (error: %d)", res);
			ret = TCDMX_FALSE;
		} else {
			*plResultPts = result->m_iTimeStamp;
			ret = TCDMX_TRUE;
		}
	}
	return ret;
}


static
gboolean
gst_mp4dmx_vm_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcMp4Demux *p_mpgdmx = GST_MP4DMX(pstDemuxBase);
	gint32 result_pts;

	(void)gst_mp4dmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);	//seek to 0ms

	return TCDMX_TRUE;
}

static
gboolean
gst_mp4dmx_vm_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcMp4Demux *		p_mp4dmx = GST_MP4DMX(pstDemuxBase);
	mp4_dmx_handle_t	h_dmx = p_mp4dmx->hMp4Dmx;
	mp4_dmx_input_t * 	get_param = &p_mp4dmx->stInput;
	mp4_dmx_result_t	res;
	gboolean ret;

	if(get_param->m_pPacketBuff != NULL)
	{
		g_free(get_param->m_pPacketBuff);
		get_param->m_pPacketBuff = NULL;
	}

	if ( h_dmx != 0U ) {
		res = p_mp4dmx->gExtDmxFunc (DMXOP_CLOSE, &h_dmx, NULL, NULL);
		if( res != ERR_NONE ) {
			GST_ERROR_OBJECT (p_mp4dmx, "mp4 close failed - (error: %d)", res);
		}

		p_mp4dmx->hMp4Dmx = 0;
		(void)memset ( &p_mp4dmx->stInitInfo, 0, sizeof(mp4_dmx_info_t) );

		gst_mp4dmx_close_library(p_mp4dmx);
		ret = TCDMX_TRUE;
	}
	else {
		GST_ERROR_OBJECT (p_mp4dmx, "mp4 already closed");
		ret = TCDMX_FALSE;
	}
	return ret;
}

static
gboolean
gst_mp4dmx_vm_getparam (
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
		*str = g_strdup("MP4");
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
//	GstTcMp4Demux private implementations
//
