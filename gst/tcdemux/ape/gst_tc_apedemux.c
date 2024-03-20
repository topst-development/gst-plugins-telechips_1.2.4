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
 * SECTION:element-tc-apedemux
 *
 * FIXME:Describe tc-apedemux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tc-apedemux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_apedemux.h"
#include "gst_tc_demuxio.h"


/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE		    ("application/x-ape")
#define STRING_DEMUX_LONGNAME       ("Telechips APE Demuxer")
#define STRING_DEMUX_DESCRIPTION    ("Demultiplex APE into element streams")


static GstStaticPadTemplate gs_stSinkFactory = GST_STATIC_PAD_TEMPLATE (
	"sink",											//template-name
	GST_PAD_SINK,							//direction
	GST_PAD_ALWAYS,					//presence
	GST_STATIC_CAPS (CONTAINER_MIMETYPE)	//caps
    );

static const guint16 gs_usSupportAudioTags[] = {
	(guint16)WAVE_FORMAT_APE,
	(guint16)0	// end of tags
};

static const guint32 gs_ulSupportVideoFcc[] = {
	(guint32)0	// end of fccs
};

G_DEFINE_TYPE (GstTcApeDemux, gst_apedmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

gint ape_strncmp( gint8* pSrc1, const gint8* pSrc2, gint32 Len );

/* GObject virtual method implementations */
static void gst_apedmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_apedmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_apedmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_apedmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestID, tcdmx_result_t * pstResult);
static gboolean gst_apedmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_apedmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_apedmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_apedmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);


gint
ape_strncmp( gint8* pSrc1, const gint8* pSrc2, gint32 Len )
{
  gint32 i;
  gint result;
  gint end=0;
  gchar* tSrc1 = (gchar*)pSrc1;
  const gchar* tSrc2 = pSrc2;

  for( i = 0; ((i < Len) && (*tSrc1 == *tSrc2)); i++ )
  {
    if( *tSrc1 == '\0' ) {
      end = 1;
      break;
    }
    else {
      tSrc1++;
      tSrc2++;
    }
  }

  if(( i == Len ) || (end == 1)){
    result = 0;
  }
  else {
    result = (gint)(*tSrc1) - (gint)*tSrc2;
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//


/* initialize the tc-apedemux's class */
static 
void
gst_apedmx_class_init (
	GstTcApeDemuxClass  * pstDemuxClass
	)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxClass;
	GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("");

	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overiding
	p_object_class->finalize = gst_apedmx_finalize;

	// base class overiding
	p_dmxbase_class->vmOpen    = gst_apedmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_apedmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_apedmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_apedmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_apedmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_apedmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_apedmx_vm_getparam;

	// set demux class details
	gst_tcdmx_set_class_details (GST_DMXBASE_CLASS(pstDemuxClass)
		, (gchar *)STRING_DEMUX_LONGNAME
		, (gchar *)STRING_DEMUX_DESCRIPTION
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
gst_apedmx_init (
	GstTcApeDemux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	(void)gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), TCDMX_MODE_SEQUENTIAL_ONLY);
}


static 
void
gst_apedmx_finalize (
	GObject  * pstObject
	)
{
  GstTcApeDemux *p_dmx = GST_APEDMX (pstObject);

  GST_TRACE ("");

  //TODO: finialize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define APE_EXT_LIB_NAME ("libtccapedmx.so")

static gint gst_apedmx_load_library(GstTcApeDemux *p_apedmx)
{
  gint result = 0;
  p_apedmx->pExtDLLModule = dlopen(APE_EXT_LIB_NAME, (int)((guint16)RTLD_LAZY | (guint16)RTLD_GLOBAL));
  if( p_apedmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_apedmx,"[APEDMX] Load library '%s' failed: %s", APE_EXT_LIB_NAME, dlerror());
    result = -1;
  } else {
    GST_DEBUG_OBJECT(p_apedmx,"[APEDMX] Library '%s' Loaded", APE_EXT_LIB_NAME);

    p_apedmx->gExtDmxFunc = dlsym(p_apedmx->pExtDLLModule, "TCC_APE_DMX");
    if( p_apedmx->gExtDmxFunc == NULL ) {
      GST_ERROR_OBJECT(p_apedmx,"[APEDMX] p_apedmx->gExtDmxFunc Error");
      result = -1;
    }
  }

  return result;
}

static void gst_apedmx_close_library(GstTcApeDemux *p_apedmx)
{
  if( p_apedmx->pExtDLLModule != NULL) {
    (void)dlclose(p_apedmx->pExtDLLModule);
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
gst_apedmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GstTcApeDemux *p_apedmx = GST_APEDMX(pstDemuxBase);
	void* fp_inp;
	guint8 buffer[16] = {0,};
	gint res;
	GstFlowReturn ret;
	audio_dmx_init_t *init_param = &p_apedmx->gsApeDmxInit;
	audio_dmx_info_t *p_init_info = &p_apedmx->gsApeDmxInfo;

	(void)memset(&p_apedmx->gsApeDmxInit, 0, sizeof(audio_dmx_init_t));
	(void)memset(&p_apedmx->gsApeDmxInfo, 0, sizeof(audio_dmx_info_t));
	(void)memset(&p_apedmx->gsApeDmxOutput, 0, sizeof(audio_dmx_output_t));

	init_param->m_sCallbackFunc.m_pMalloc			= g_malloc;
	init_param->m_sCallbackFunc.m_pFree			= g_free;
	init_param->m_sCallbackFunc.m_pMemcpy			= memcpy;
	init_param->m_sCallbackFunc.m_pMemset			= memset;
	init_param->m_sCallbackFunc.m_pRealloc			= g_realloc;
	init_param->m_sCallbackFunc.m_pMemmove			= memmove;
	init_param->m_sCallbackFunc.m_pFopen			= tcdmx_io_open;
	init_param->m_sCallbackFunc.m_pFread			= tcdmx_io_read;
	init_param->m_sCallbackFunc.m_pFseek			= tcdmx_io_seek;
	init_param->m_sCallbackFunc.m_pFtell			= tcdmx_io_tell;
	init_param->m_sCallbackFunc.m_pFclose			= tcdmx_io_close;
	init_param->m_sCallbackFunc.m_pStrncmp          = ape_strncmp;
	init_param->m_sCallbackFunc.m_pFseek64          = tcdmx_io_seek64;
	init_param->m_sCallbackFunc.m_pFtell64			= tcdmx_io_tell64;

	init_param->m_pOpenFileName       = (gint8*)(void*)pstDemuxBase->pstSinkPad;
	p_apedmx->gsApeDmxHandle = 0;

	if (gst_apedmx_load_library(p_apedmx) != 0){
		ret = GST_FLOW_NOT_SUPPORTED;
	} else {

		fp_inp = tcdmx_io_open(init_param->m_pOpenFileName, "rb");
		if( fp_inp == NULL )
		{
			ret = GST_FLOW_NOT_SUPPORTED;
		}
		else {

			(void)tcdmx_io_read( buffer, 10, 1, fp_inp );

			init_param->m_uiId3TagOffset = 0;

			if( ape_strncmp((gint8*)buffer, "ID3", 3) == 0 )
			{
				init_param->m_uiId3TagOffset = (guint)(((guint)buffer[6] << 21) | ((guint)buffer[7] << 14) | ((guint)buffer[8] <<  7) | ((guint)buffer[9] <<  0));
				init_param->m_uiId3TagOffset += 10U;
			}

			(void)tcdmx_io_seek(fp_inp, (long)init_param->m_uiId3TagOffset, 0);	// skip id3 tag
			(void)tcdmx_io_read( buffer, 16, 1, fp_inp );

			if( ape_strncmp((gint8*)buffer, "MAC ", 4) == 0 )
			{
				p_init_info->m_sAudioInfo.m_iFormatId = WAVE_FORMAT_APE;
				p_init_info->pApeTagInfo = NULL;
			}
		
			(void)tcdmx_io_close( fp_inp );

			if (p_init_info->m_sAudioInfo.m_iFormatId == 0){
				ret = GST_FLOW_NOT_SUPPORTED;
			} else {
				res = p_apedmx->gExtDmxFunc (AUDIO_DMX_OPEN, &p_apedmx->gsApeDmxHandle, init_param, p_init_info);
				if ( res < 0 ) {
					(void)p_apedmx->gExtDmxFunc (AUDIO_DMX_CLOSE, &p_apedmx->gsApeDmxHandle, NULL, NULL);
					GST_ERROR_OBJECT (p_apedmx, "apedmx open failed - (error: %d)", res);
					ret = GST_FLOW_NOT_SUPPORTED;
				} else {	
					gst_tcdmx_set_total_audiostream(pstDemuxBase, 1);
					ret = GST_FLOW_OK;
				}
			}
		}
	}

	return ret;
}

static
gboolean
gst_apedmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcApeDemux *p_apedmx = GST_APEDMX(pstDemuxBase);
	audio_dmx_info_t *p_info = &p_apedmx->gsApeDmxInfo;
	tc_audio_info_t info;
	gboolean ret;
	memset(&info, 0, sizeof(tc_audio_info_t));
	info.ulFormatTag     = (guint32)p_info->m_sAudioInfo.m_iFormatId;
	info.ulChannels      = (guint32)p_info->m_sAudioInfo.m_iChannels;
	info.ulBlockAlign    = (guint32)p_info->m_sAudioInfo.m_iBlockAlign;
	info.ulBitPerSample  = (guint32)p_info->m_sAudioInfo.m_iBitsPerSample;
	info.ulSampleRate    = (guint32)p_info->m_sAudioInfo.m_iSamplePerSec;
	//info.ulSize          = 0;
	info.ulDuration      = (guint32)p_info->m_sFileInfo.m_iRunningtime;
	//info.ulEndian	= 0;

	gst_tcdmx_set_seekable(pstDemuxBase, (gboolean)TCDMX_TRUE);

	ret = gst_tcdmx_register_stream_info (pstDemuxBase,
										(guint)TCDMX_TYPE_AUDIO,
										(guint)TCDMX_TYPE_AUDIO,
										&info,
										p_info->m_sAudioInfo.m_pExtraData,
										p_info->m_sAudioInfo.m_iExtraDataLen);

	return ret;

}

static
gboolean
gst_apedmx_vm_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestID,
	tcdmx_result_t  * pstResult
	)
{
	GstTcApeDemux *p_apedmx = GST_APEDMX(pstDemuxBase);
	audio_handle_t h_dmx = p_apedmx->gsApeDmxHandle;
	gint res;
	gboolean ret;
	audio_dmx_output_t *output = &p_apedmx->gsApeDmxOutput;

	res = p_apedmx->gExtDmxFunc (AUDIO_DMX_GETFRAME, &h_dmx, NULL, output);
	if ( res < 0 ) {
		GST_ERROR_OBJECT (p_apedmx, "ape demux failed - (error: %d)", res);
		ret = TCDMX_FALSE;
	} else {
		pstResult->ulStreamID = (guint)TCDMX_TYPE_AUDIO;
		pstResult->pBuffer = output->m_pPacketData;
		pstResult->lLength = output->m_iPacketSize;
		pstResult->lTimestampMs = output->m_iTimeStamp;
		GST_LOG_OBJECT(p_apedmx,"demux[ID:Type] = [%d:%d] timestamp=%d length=%d",pstResult->ulStreamID,pstResult->ulStreamType,pstResult->lTimestampMs,pstResult->lLength);
		ret = TCDMX_TRUE;
	}
	return ret;
}


static
gboolean
gst_apedmx_vm_seek (
	GstTcDemuxBase  * pstDemuxBase, 
	guint32           ulFlags,
	gint32            lTargetPts, 
	gint32          * plResultPts
	)
{
	GstTcApeDemux *p_apedmx = GST_APEDMX(pstDemuxBase);
	audio_handle_t h_dmx = p_apedmx->gsApeDmxHandle;
	gint res;
	gboolean ret;
	audio_dmx_seek_t param;
	audio_dmx_output_t *result = &p_apedmx->gsApeDmxOutput;

	param.m_iSeekTimeMSec = lTargetPts;
	param.m_iSeekMode = SEEK_ABSOLUTE;	//SEEK_RELATIVE

	res = p_apedmx->gExtDmxFunc (AUDIO_DMX_SEEK, &h_dmx, &param, result);
	if ( res < 0 ) {
		GST_ERROR_OBJECT (p_apedmx, "ape seek failed - (error: %d)", res);
		ret = TCDMX_FALSE;
	} else {
		if (plResultPts != 0) {
			*plResultPts = result->m_iTimeStamp;
		}
		ret = TCDMX_TRUE;
	}

	return ret;
}


static
gboolean
gst_apedmx_vm_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcApeDemux *p_apedmx = GST_APEDMX(pstDemuxBase);
	gint32 result_pts;

	return gst_apedmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);	//seek to 0ms
}

static
gboolean
gst_apedmx_vm_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GstTcApeDemux *p_apedmx = GST_APEDMX(pstDemuxBase);
	audio_handle_t h_dmx = p_apedmx->gsApeDmxHandle;
	gboolean ret;

	if ( h_dmx != 0U ) {
		gint res = p_apedmx->gExtDmxFunc (AUDIO_DMX_CLOSE, &h_dmx, NULL, NULL);

		(void)memset ( &p_apedmx->gsApeDmxInit, 0, sizeof(audio_dmx_init_t));
		(void)memset ( &p_apedmx->gsApeDmxInfo, 0, sizeof(audio_dmx_info_t));
		(void)memset ( &p_apedmx->gsApeDmxOutput, 0, sizeof(audio_dmx_output_t));

		p_apedmx->gsApeDmxHandle = 0;
		gst_apedmx_close_library(p_apedmx);

		ret = TCDMX_TRUE;
	}
	else {
		GST_ERROR_OBJECT (p_apedmx, "ape demux already closed");
		ret = TCDMX_FALSE;
	}
	return ret;
}

static 
gboolean 
gst_apedmx_vm_getparam (
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
		*str = g_strdup("APE");
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
//	GstTcApeDemux private implementations
//

