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
 * SECTION:element-tc-mkvdemux
 *
 * FIXME:Describe tc-mkvdemux here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tc-mkvdemux ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>

#include "gst_tc_mkvdemux.h"
#include "gst_tc_demuxio.h"


/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

#define CONTAINER_MIMETYPE          ("video/x-matroska; video/webm")
#define STRING_DEMUX_LONGNAME       ("Telechips MKV Demuxer - "MKV_HEADER_VER)
#define STRING_DEMUX_DESCRIPTION    ("Demultiplex MKV into element streams")

#define LOGV(...)	{g_printf("\x1b[1;33m"__VA_ARGS__);g_printf("\x1b[0m\n");}
#define LOGD(...)	{g_printf("\x1b[1;36m"__VA_ARGS__);g_printf("\x1b[0m\n");}
#define LOGI(...)	{g_printf("\x1b[1;32m"__VA_ARGS__);g_printf("\x1b[0m\n");}
#define LOGW(...)	{g_printf("\x1b[1;35m"__VA_ARGS__);g_printf("\x1b[0m\n");}
#define LOGE(...)	{g_printf("\x1b[1;31m"__VA_ARGS__);g_printf("\x1b[0m\n");}

#define STEP(...)  	{GST_WARNING(__VA_ARGS__);}

#define INFO(...)   {LOGI("[INFO] "__VA_ARGS__)}
#define ERROR(...)  {LOGE("[ERROR] "__VA_ARGS__)}


#define MAX_DMX_INPUT_BUFF_SIZE (2*1014*1024) // 2M

static GstStaticPadTemplate gs_stSinkFactory = GST_STATIC_PAD_TEMPLATE (
	"sink",									//template-name
	GST_PAD_SINK,							//direction
	GST_PAD_ALWAYS,							//presence
	GST_STATIC_CAPS (CONTAINER_MIMETYPE)	//caps
    );

G_DEFINE_TYPE (GstTcMkvDemux, gst_mkvdmx, GST_TYPE_DMXBASE);

static GstElementClass *parent_class = NULL;

/* GObject virtual method implementations */
static void gst_mkvdmx_finalize (GObject * pstObject);

/* GstTcDemuxBase virtual method implementations */
static GstFlowReturn gst_mkvdmx_vm_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_mkvdmx_vm_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mkvdmx_vm_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestID, tcdmx_result_t * pstResult);
static gboolean gst_mkvdmx_vm_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_mkvdmx_vm_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mkvdmx_vm_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_mkvdmx_vm_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//
/* initialize the tc-mkvdemux's class */
static 
void
gst_mkvdmx_class_init (
	GstTcMkvDemuxClass  * pstDemuxClass
	)
{
	GObjectClass        *p_object_class  = (GObjectClass *) pstDemuxClass;
	//GstElementClass     *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);
	GstTcDemuxBaseClass *p_dmxbase_class = GST_DMXBASE_CLASS(pstDemuxClass);

	GST_TRACE ("");

	parent_class = g_type_class_peek_parent (pstDemuxClass);

	// object class overriding
	p_object_class->finalize = gst_mkvdmx_finalize;

	// base class overriding
	p_dmxbase_class->vmOpen    = gst_mkvdmx_vm_open;
	p_dmxbase_class->vmSetInfo = gst_mkvdmx_vm_setinfo;
	p_dmxbase_class->vmDemux   = gst_mkvdmx_vm_demux;
	p_dmxbase_class->vmSeek    = gst_mkvdmx_vm_seek;
	p_dmxbase_class->vmReset   = gst_mkvdmx_vm_reset;
	p_dmxbase_class->vmClose   = gst_mkvdmx_vm_close;
	p_dmxbase_class->vmGetParam= gst_mkvdmx_vm_getparam;

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
 * set pad callback functions
 * initialize instance structure
 */
static 
void
gst_mkvdmx_init (
	GstTcMkvDemux       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	(void)gst_tcdmx_add_sinkpad(GST_DMXBASE(pstDemux), &gs_stSinkFactory);
	gst_tcdmx_set_default_demux_mode (GST_DMXBASE(pstDemux), TCDMX_MODE_SEQUENTIAL);
}


static 
void
gst_mkvdmx_finalize (
	GObject  * pstObject
	)
{
  //GstTcMkvDemux *p_dmx = GST_MKVDMX (pstObject);
  GST_TRACE ("");
  
  //TODO: finalize

  G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


#define MKV_EXT_LIB_NAME ("libtccmkvdmx.so")

gint gst_mkvdmx_load_library(GstTcMkvDemux *p_mkvdmx);
void gst_mkvdmx_close_library(GstTcMkvDemux *p_mkvdmx);

gint gst_mkvdmx_load_library(GstTcMkvDemux *p_mkvdmx)
{
  gint result = 0;
  p_mkvdmx->pExtDLLModule = dlopen(MKV_EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
  if( p_mkvdmx->pExtDLLModule == NULL ) {
    GST_ERROR_OBJECT(p_mkvdmx,"[MKVDMX] Load library '%s' failed: %s", MKV_EXT_LIB_NAME, dlerror());
    result = -1;
  } else {
    GST_DEBUG_OBJECT(p_mkvdmx,"[MKVDMX] Library '%s' Loaded", MKV_EXT_LIB_NAME);

    p_mkvdmx->gExtDmxFunc = dlsym(p_mkvdmx->pExtDLLModule, "TCC_MKV_DMX");
    if( p_mkvdmx->gExtDmxFunc == NULL ) {
      GST_ERROR_OBJECT(p_mkvdmx,"[MKVDMX] p_mkvdmx->gExtDmxFunc Error");
      result = -1;
    }
  }

  return result;
}

void gst_mkvdmx_close_library(GstTcMkvDemux *p_mkvdmx)
{
  if( p_mkvdmx->pExtDLLModule != NULL){
    (void)dlclose(p_mkvdmx->pExtDLLModule);
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
gst_mkvdmx_vm_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GstTcMkvDemux *p_mkvdmx = GST_MKVDMX(pstDemuxBase);
	mkv_dmx_result_t res;
	GstFlowReturn ret;
	mkv_dmx_inst_t *pst_inst = &p_mkvdmx->stInst;
	gulong ul_opcode;

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

	if (gst_mkvdmx_load_library(p_mkvdmx) == 0) {

		(void)memset ( pst_inst, 0, sizeof(mkv_dmx_inst_t) );

		pst_inst->pfnDmxLib   = p_mkvdmx->gExtDmxFunc;
		pst_inst->pvLibHandle = p_mkvdmx->pExtDLLModule;

		pst_inst->stDmxInit.stMemoryFuncs   = mem_func;
		pst_inst->stDmxInit.stFileFuncs     = file_func;
		pst_inst->stDmxInit.pszOpenFileName = (char*)(void*)pstDemuxBase->pstSinkPad;

		pst_inst->stDmxInit.u32VideoFileCacheSize = 4*1024;
		pst_inst->stDmxInit.u32AudioFileCacheSize = 4*1024;
		pst_inst->stDmxInit.u32SeekFileCacheSize  = 1024;

		{
		//--------------------------------------------------------
		// [*] MKVDOP_GET_VERSION operation
		// --------------------------------------------------------
		char *psz_version = NULL;
		char *psz_build_date = NULL;
		pst_inst->pfnDmxLib( MKVDOP_GET_VERSION, NULL, &psz_version, &psz_build_date );
		INFO(" - %s, %s\n", psz_version, psz_build_date);
		}

		if(CHECK_FLAG((pstDemuxBase->ulFlags), (TCDMX_BASE_FLAG_MULTIPLE_STEAM)))
		{
	        // parse all streams
			pst_inst->stDmxInit.u32InitFlags |= MKVDF_PARSE_ALL_STREAMS;
			INFO("Multiple stream demuxing enabled" );
		}
		{
			STEP( "\tDMXOP_OPEN 01\n" );
			ul_opcode = DMXOP_OPEN;
		}

		if ( CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_STREAMING_SOURCE) )
		{
			STEP ("MKVDOP_OPEN_FROM_FILE_HANDLE");
			ul_opcode = MKVDOP_OPEN_FROM_FILE_HANDLE;
			pst_inst->stDmxInit.pszOpenFileName = tcdmx_io_open((char*)(void*)pstDemuxBase->pstSinkPad, NULL);
			p_mkvdmx->pstSourceHandler = pst_inst->stDmxInit.pszOpenFileName;
		}

		res = pst_inst->pfnDmxLib( ul_opcode, &pst_inst->hDmxHandle, &pst_inst->stDmxInit, &pst_inst->stDmxInfo );
		if( res < 0 )
		{
			pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
			ERROR( "[%ld] opcode:%d Error:%s, %lld", ul_opcode, pst_inst->stDmxError.s32ErrCode, pst_inst->stDmxError.pszErrStatus, pst_inst->stDmxError.s64LastStatus );
			if (pst_inst->stDmxError.s32ErrCode == -101) {
				ret = GST_FLOW_ERROR_NOT_SUPPORT_DIVX;
			} else {
				ret = GST_FLOW_NOT_SUPPORTED;
			}
		} else {

			//seek fail handling scenario
			mkv_dmx_options_t options;
			options.u32OptGetStream = 0;
			options.u32OptSeekOverLastKey = 0;
			if (CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL)) {
				options.u32OptSeekOverLastKey = MKVDOPT_LAST_KEYFRAME;
			} else if (CHECK_FLAG(ulFlags, TCDMX_BASE_FLAG_GOTO_EOS_IF_SEEK_FAIL)) {
				options.u32OptSeekOverLastKey = MKVDOPT_TO_END_OF_FILE;
			} else {
				GST_LOG("do nothing");
			}

			INFO("SEEK OPTIONS: seek over last key (%d)", options.u32OptSeekOverLastKey);
			pst_inst->pfnDmxLib (MKVDOP_SET_OPTIONS,
				&pst_inst->hDmxHandle,
				&options, 
				NULL);

			// finalize of init
			pst_inst->stCdmxBuff.ulStreamBuffSize = MAX_DMX_INPUT_BUFF_SIZE;
			pst_inst->stCdmxBuff.pbyStreamBuff = (guchar *)pst_inst->stDmxInit.stMemoryFuncs.pfnMalloc(pst_inst->stCdmxBuff.ulStreamBuffSize);
			if( pst_inst->stCdmxBuff.pbyStreamBuff == NULL )
			{
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", -1, __FILE__, __LINE__ );
				ret = GST_FLOW_NOT_SUPPORTED;
			} else {
				if (pst_inst->stDmxInfo.u32AudioStreamTotal > 0U) {
					gst_tcdmx_set_total_audiostream(pstDemuxBase, 1);	//temp code
				}
				ret = GST_FLOW_OK;
			}
		}
	} else {
		ret = GST_FLOW_NOT_SUPPORTED;
	}
	return ret;
}


static gint
gst_matroska_parse_xiph_stream_headers (gpointer codec_data,
    guint codec_data_size, vorbis_header_t *pstHeader)
{
  guint8 *p = codec_data;
  guint i, offset, num_packets;
  guint *length, last;

  GST_INFO ("xiph codec data", codec_data, codec_data_size);

  if ((codec_data == NULL) || (codec_data_size == 0u)){
    goto error;
  }
  /* start of the stream and vorbis audio or theora video, need to
   * send the codec_priv data as first three packets */
  num_packets = (guint)p[0] + 1u;
  GST_INFO ("%u stream headers, total length=%" G_GSIZE_FORMAT " bytes",
      (guint) num_packets, codec_data_size);

  length = g_alloca (num_packets * sizeof (guint));
  last = 0;
  offset = 1;

  /* first packets, read length values */
  for (i = 0; i < num_packets - 1; i++) {
    length[i] = 0;
    while (offset < codec_data_size) {
      length[i] += p[offset];
      if (p[offset++] != 0xffu)
        break;
    }
    last += length[i];
  }
  if ((offset + last) > codec_data_size)
    goto error;

  /* last packet is the remaining size */
  length[i] = codec_data_size - offset - last;

  for (i = 0; i < num_packets; i++) {
    GST_INFO ("buffer %d: %u bytes (offset:%d)", i, (guint) length[i], offset);

    if ((offset + length[i]) > codec_data_size)
      goto error;

    pstHeader->pbyData[i] = (guint8 *)(p + offset);
    pstHeader->u32DataSize[i] = length[i];

    offset += length[i];
  }

  return 1;

/* ERRORS */
error:
  {
    return 0;
  }
}

static
gboolean
gst_mkvdmx_vm_setinfo (
	GstTcDemuxBase  * pstDemuxBase
	)
{
    GstTcMkvDemux *p_mkvdmx = GST_MKVDMX(pstDemuxBase);
    mkv_dmx_inst_t *pst_inst = &p_mkvdmx->stInst;

    gst_tcdmx_set_seekable(pstDemuxBase,
                       ((pst_inst->stDmxInfo.pstFileInfo->u32InfoFlags & MKVDINFO_SEEKABLE) == 0u) ? TCDMX_FALSE : TCDMX_TRUE);

    if (pst_inst->stDmxInfo.u32VideoStreamTotal> 0u) {
        guint i, mkv_total_number = 1;
        mkv_dmx_video_info_t *p_info = NULL;

        #if 0 //FIXME: not support multiple video
        if (pst_inst->stDmxInit.u32InitFlags & MKVDF_PARSE_ALL_STREAMS) {
            mkv_total_number = pst_inst->stDmxInfo.u32VideoStreamTotal;
        }
        #endif

        INFO("===== Video Total:%d ====", mkv_total_number);
        for (i = 0; i < mkv_total_number; i++)
        {
            tc_video_info_t info;
            p_info = &pst_inst->stDmxInfo.pstVideoInfoList[i];
            info.ulFourCC       = p_info->u32FourCC;
            info.ulWidth        = p_info->u32Width;
            info.ulHeight       = p_info->u32Height;
            info.ulBitPerPixel  = 0;
            info.ulFrameRate    = p_info->u32FrameRate;
            info.ulDuration     = pst_inst->stDmxInfo.pstFileInfo->u32Duration;

            INFO("[Video:%d][FourCC:%c%c%c%c(0x%8x)][W:%d][H:%d][FPS:%d][Duration:%d]"
                     , p_info->u32StreamID
                     , ((info.ulFourCC >> 0) & 255)
                     , ((info.ulFourCC >> 8) & 255)
                     , ((info.ulFourCC >>16) & 255)
                     , ((info.ulFourCC >>24) & 255)
                     , info.ulFourCC
                     , info.ulWidth
                     , info.ulHeight
                     , info.ulFrameRate
                     , info.ulDuration);


            (void)gst_tcdmx_register_stream_info (pstDemuxBase,
                                            p_info->u32StreamID, //TCDMX_TYPE_VIDEO,
                                            (guint32)TCDMX_TYPE_VIDEO,
                                            &info,
                                            p_info->pbyCodecPrivate,
                                            (gint32)p_info->u32CodecPrivateSize);
        }
    }

    if (pst_inst->stDmxInfo.u32AudioStreamTotal > 0u) {
        guint i, mkv_total_number = 1;
        mkv_dmx_audio_info_t *p_info = NULL;

        if ((pst_inst->stDmxInit.u32InitFlags & MKVDF_PARSE_ALL_STREAMS) > 0u) {
            mkv_total_number = pst_inst->stDmxInfo.u32AudioStreamTotal;
        }

        INFO("===== Audio Total:%d ====", mkv_total_number);

        pst_inst->u32VorbisTotalCount = 0;
        for (i = 0; i < mkv_total_number; i++)
        {
            p_info = &pst_inst->stDmxInfo.pstAudioInfoList[i];
            if (((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS     ) || /* ogg/vorbis mode    */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS1    ) || /* ogg/vorbis mode 1  */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS2    ) || /* ogg/vorbis mode 2  */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS3    ) || /* ogg/vorbis mode 3  */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS1PLUS) || /* ogg/vorbis mode 1+ */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS2PLUS) || /* ogg/vorbis mode 2+ */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS3PLUS))   /* ogg/vorbis mode 3+ */
            {
                pst_inst->u32VorbisTotalCount++;
            }
        }

        if (pst_inst->u32VorbisTotalCount > 0u) {
            pst_inst->u32VorbisIndex = 0;
            pst_inst->pstVorbisHeaders = (vorbis_header_t *) g_malloc(pst_inst->u32VorbisTotalCount * sizeof(vorbis_header_t));

            GST_INFO("===== Vorbis Total:%d ====", pst_inst->u32VorbisTotalCount);
        }

        for (i = 0; i < mkv_total_number; i++)
        {
            tc_audio_info_t info;
            memset(&info, 0, sizeof(tc_audio_info_t));
            p_info = &pst_inst->stDmxInfo.pstAudioInfoList[i];
            info.ulFormatTag     = p_info->u32FormatTag;
            info.ulChannels      = p_info->u32Channels;
            info.ulBlockAlign    = p_info->u32BlockAlign;
            info.ulBitPerSample  = p_info->u32BitsPerSample;
            info.ulSampleRate    = p_info->u32SamplePerSec;
            info.ulSize          = p_info->u32AvgBytesPerSec;
            info.ulDuration      = pst_inst->stDmxInfo.pstFileInfo->u32Duration;

            if (((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS     ) || /* ogg/vorbis mode    */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS1    ) || /* ogg/vorbis mode 1  */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS2    ) || /* ogg/vorbis mode 2  */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS3    ) || /* ogg/vorbis mode 3  */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS1PLUS) || /* ogg/vorbis mode 1+ */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS2PLUS) || /* ogg/vorbis mode 2+ */
                ((short)(p_info->u32FormatTag) == WAVE_FORMAT_VORBIS3PLUS))   /* ogg/vorbis mode 3+ */
            {
                (void)memset(&pst_inst->pstVorbisHeaders[pst_inst->u32VorbisIndex], 0, sizeof(vorbis_header_t));
                (void)gst_matroska_parse_xiph_stream_headers(p_info->pbyCodecPrivate, p_info->u32CodecPrivateSize, &pst_inst->pstVorbisHeaders[pst_inst->u32VorbisIndex]);
                pst_inst->pstVorbisHeaders[pst_inst->u32VorbisIndex].u32StreamID = p_info->u32StreamID;
                pst_inst->u32VorbisIndex++;
            }

            if (info.ulFormatTag == WAVE_FORMAT_PCM) {
                info.ulEndian = 0;        //LittleEndian or Don't Care
            }

            info.pszlanguage = p_info->pszLanguage;

            INFO("[Audio:%d][FormatTag:%8x][Ch:%d][BlockAlign:%d][BPS:%3d][SampleRate:%5d][AvgBytes:%d][Duration:%8d][Lang:%s][Priv:%4d]"
                     , p_info->u32StreamID
                     , info.ulFormatTag
                     , info.ulChannels
                     , info.ulBlockAlign
                     , info.ulBitPerSample
                     , info.ulSampleRate
                     , info.ulSize
                     , info.ulDuration
                     , info.pszlanguage
                     , p_info->u32CodecPrivateSize);


            (void)gst_tcdmx_register_stream_info (pstDemuxBase,
                                            p_info->u32StreamID, //TCDMX_TYPE_AUDIO,
                                            (guint32)TCDMX_TYPE_AUDIO,
                                            &info,
                                            p_info->pbyCodecPrivate,
                                            (gint32)p_info->u32CodecPrivateSize);

        }
        // reset
        pst_inst->u32VorbisIndex = 0;
    }

    if (pst_inst->stDmxInfo.u32SubtitleStreamTotal > 0u) {
        guint i, mkv_total_number = 1;
        mkv_dmx_subtitle_info_t *p_info = NULL;

        #if 1
        if ((pst_inst->stDmxInit.u32InitFlags & MKVDF_PARSE_ALL_STREAMS) > 0u) {
            mkv_total_number = pst_inst->stDmxInfo.u32SubtitleStreamTotal;
        }
        #endif

        INFO("===== Subtitle Total:%d ====", mkv_total_number);
        for (i = 0; i < mkv_total_number; i++)
        {
            tc_subtitle_info_t info;
            memset(&info, 0, sizeof(tc_subtitle_info_t));
            p_info = &pst_inst->stDmxInfo.pstSubtitleInfoList[i];

            switch (p_info->u32SubtitleType) {
            case SUBTITLETYPE_SRT:
            	info.ulFormatId = (guint)SUBTITLE_FORMAT_ID_SRT;
            	break;
            case SUBTITLETYPE_SSA:
            	info.ulFormatId = (guint)SUBTITLE_FORMAT_ID_SSA;
            	break;
            case SUBTITLETYPE_USF:
            	info.ulFormatId = (guint)SUBTITLE_FORMAT_ID_USF;
            	break;
            case SUBTITLETYPE_BMP:
            	info.ulFormatId = (guint)SUBTITLE_FORMAT_ID_BMP;
            	break;
            case SUBTITLETYPE_VOBSUB:
            	info.ulFormatId = (guint)SUBTITLE_FORMAT_ID_VOBSUB;
            	break;
            case SUBTITLETYPE_KATE:
              info.ulFormatId = (guint)SUBTITLE_FORMAT_ID_KATE;
              break;
            default:
              info.ulFormatId = (guint)SUBTITLE_FORMAT_ID_UNKNOWN;
              break;
            }

            info.pszlanguage = p_info->pszLanguage;

            INFO("[Subtitle:%d][Type:%d][Lang:%s]"
                     , p_info->u32StreamID
                     , info.ulFormatId
                     , p_info->pszLanguage);

            (void)gst_tcdmx_register_stream_info (pstDemuxBase,
                                            p_info->u32StreamID, //TCDMX_TYPE_SUBTITLE,
                                            (guint32)TCDMX_TYPE_SUBTITLE,
                                            &info,
                                            p_info->pvSubtitleInfo,
                                            (gint32)p_info->u32SubtitleInfoSize);
        }
    }


    return TCDMX_TRUE;
}

static
gboolean
gst_mkvdmx_vm_demux (
    GstTcDemuxBase  * pstDemuxBase,
    guint32           ulRequestID,
    tcdmx_result_t  * pstResult
    )
{
    GstTcMkvDemux *p_mkvdmx = GST_MKVDMX(pstDemuxBase);
    mkv_dmx_result_t res;
    mkv_dmx_inst_t *pst_inst = &p_mkvdmx->stInst;
    u_int32_t mkv_stream_id = 0;
    u_int32_t key_flag = 0;

    pst_inst->stDmxGetStream.pbyStreamBuff     = pst_inst->stCdmxBuff.pbyStreamBuff;
    pst_inst->stDmxGetStream.u32StreamBuffSize = pst_inst->stCdmxBuff.ulStreamBuffSize;

    //INFO("gst_mkvdmx_vm_demux");
    if (ulRequestID != (guint32)TCDMX_TYPE_ANY) {
        switch (ulRequestID) {
        case (guint32)TCDMX_TYPE_VIDEO:
            pst_inst->stDmxGetStream.u32StreamType = DMXTYPE_VIDEO;
            break;
        case (guint32)TCDMX_TYPE_AUDIO:
            pst_inst->stDmxGetStream.u32StreamType = DMXTYPE_AUDIO;
            //TODO: selective mode vorbis: from pstDemuxBase->request_id;
            break;
        case (guint32)TCDMX_TYPE_SUBTITLE:
            pst_inst->stDmxGetStream.u32StreamType = DMXTYPE_SUBTITLE;
            break;
        default:
            break;
        }
    } else {
        pst_inst->stDmxGetStream.u32StreamType = DMXTYPE_NONE;

        if (pst_inst->u32VorbisTotalCount > 0u) {
            vorbis_header_t *pst_vorbis = &pst_inst->pstVorbisHeaders[pst_inst->u32VorbisIndex];
            GST_INFO("[%d] VorbisTotalCount: %d", pst_inst->u32VorbisIndex, pst_inst->u32VorbisTotalCount);
            GST_INFO("[%d] NumPackets: %d", pst_inst->u32VorbisIndex, pst_vorbis->u32NumPackets);

            if (pst_vorbis != NULL) {
                unsigned char *pby_data = pst_vorbis->pbyData[pst_vorbis->u32NumPackets];
                unsigned int   u32_data_size = pst_vorbis->u32DataSize[pst_vorbis->u32NumPackets];

                GST_INFO("Packet[%d] DataSize: %d", pst_vorbis->u32NumPackets, u32_data_size);
                if ((u32_data_size != 0u) && (pby_data != NULL)) {
                    pst_vorbis->u32NumPackets++;

                    pstResult->pBuffer = pby_data;
                    pstResult->lLength = (int)u32_data_size;

                    pstResult->ulStreamID   = pst_vorbis->u32StreamID;
                    pstResult->lTimestampMs = 0;

                    guint8 *pp = pstResult->pBuffer;
                    GST_INFO("CodecPrivateSize: %d", pstResult->lLength);
                    GST_INFO("%02x %02x %02x %02x %02x %02x %02x %02x %02x", *pp,*(pp+1),*(pp+2),*(pp+3),*(pp+4),*(pp+5),*(pp+6),*(pp+7),*(pp+8));

                    if (pst_vorbis->u32NumPackets == 3u) {
                        pst_vorbis->u32InitDone = 1;
                        pst_vorbis->u32NumPackets = 0;
                        pst_inst->u32VorbisTotalCount--;
                        pst_inst->u32VorbisIndex++;
                    }
                    return TCDMX_TRUE;
                }
            }
        }
    }

    res = pst_inst->pfnDmxLib( DMXOP_GET_STREAM, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
    if( res < 0 )
    {
        if( res == DMXRET_END_OF_STREAM )
        {
            STEP( "\tDMXRET_END_OF_STREAM\n" );
        }

        pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
        GST_ERROR( "[%d] DMXOP_GET_STREAM Error:%s, %lld\n", pst_inst->stDmxError.s32ErrCode, pst_inst->stDmxError.pszErrStatus, pst_inst->stDmxError.s64LastStatus );
        return TCDMX_FALSE;
    }

    switch (pst_inst->stDmxOutStream.u32StreamType) {
        case DMXTYPE_VIDEO:
        	pstResult->ulStreamType = (guint)TCDMX_TYPE_VIDEO;
        	break;
        case DMXTYPE_AUDIO:
        	pstResult->ulStreamType = (guint)TCDMX_TYPE_AUDIO;
        	break;
        case DMXTYPE_SUBTITLE:
        	pstResult->ulStreamType = (guint)TCDMX_TYPE_SUBTITLE;
        	break;
    }

    if ((pst_inst->stDmxInit.u32InitFlags & MKVDF_PARSE_ALL_STREAMS) > 0u) {
        if (pst_inst->stDmxOutStream.pvSpecificData != 0) {
            mkv_dmx_out_specific_data_t *pst_specific_data = (mkv_dmx_out_specific_data_t *)pst_inst->stDmxOutStream.pvSpecificData;
            mkv_stream_id = pst_specific_data->u32StreamId;
            key_flag = pst_specific_data->u32IsSyncSample;
        } else {
            switch (pst_inst->stDmxOutStream.u32StreamType) {
                case DMXTYPE_VIDEO:
                	mkv_stream_id = pst_inst->stDmxInfo.pstDefaultVideoInfo->u32StreamID;
                	break;
                case DMXTYPE_AUDIO:
                	mkv_stream_id = pst_inst->stDmxInfo.pstDefaultAudioInfo->u32StreamID;
                	break;
                case DMXTYPE_SUBTITLE:
                	mkv_stream_id = pst_inst->stDmxInfo.pstDefaultSubtitleInfo->u32StreamID;
                	break;
            }
        }
    } else {
        mkv_stream_id = pstResult->ulStreamType;
    }

    pstResult->ulStreamID   = mkv_stream_id;
    pstResult->pBuffer      = pst_inst->stDmxOutStream.pbyStreamData;
    pstResult->lLength      = (gint32)pst_inst->stDmxOutStream.u32StreamDataSize;
    pstResult->lTimestampMs = (gint32)pst_inst->stDmxOutStream.u32TimeStamp;
    pstResult->lEndTimestampMs = (gint32)pst_inst->stDmxOutStream.u32EndTimeStamp;

    GST_LOG_OBJECT(p_mkvdmx, "[%s:%s][Time:%8d][Len:%6d]"
            , (pst_inst->stDmxOutStream.u32StreamType == DMXTYPE_VIDEO) ? "V" : (pst_inst->stDmxOutStream.u32StreamType == DMXTYPE_AUDIO) ? "A" : (pst_inst->stDmxOutStream.u32StreamType == DMXTYPE_SUBTITLE) ? "S" : "U"
            , (key_flag == 0) ? "X" : "K"
            , pstResult->lTimestampMs
            , pstResult->lLength);

    if (pst_inst->stDmxOutStream.u32StreamType == DMXTYPE_SUBTITLE)
      GST_LOG_OBJECT(p_mkvdmx, "[Time:%8d][EndTime:%8d][Duration:%8d]",pstResult->lTimestampMs,pstResult->lEndTimestampMs, (pstResult->lEndTimestampMs-pstResult->lTimestampMs));
    return TCDMX_TRUE;
}


static
gboolean
gst_mkvdmx_vm_seek (
    GstTcDemuxBase  * pstDemuxBase,
    guint32           ulFlags,
    gint32            lTargetPts,
    gint32          * plResultPts
    )
{
    GstTcMkvDemux *p_mkvdmx = GST_MKVDMX(pstDemuxBase);
    mkv_dmx_result_t res;
    gboolean ret;
    mkv_dmx_inst_t *pst_inst = &p_mkvdmx->stInst;

    pst_inst->stDmxSeek.s32SeekTime  = lTargetPts;
    pst_inst->stDmxSeek.u32SeekMode = DMXSEEK_DEFAULT;

    res = pst_inst->pfnDmxLib( DMXOP_SEEK, &pst_inst->hDmxHandle, &pst_inst->stDmxSeek, &pst_inst->stDmxOutStream );
    if( res < 0 )
    {
        pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
        GST_ERROR( "[%d] DMXOP_SEEK Error:%s, %lld, %d\n", pst_inst->stDmxError.s32ErrCode, pst_inst->stDmxError.pszErrStatus, pst_inst->stDmxError.s64LastStatus, ulFlags);
        ret = TCDMX_FALSE;
    } else {

        *plResultPts = (gint32)pst_inst->stDmxOutStream.u32TimeStamp;
        ret = TCDMX_TRUE;
    }
    return ret;
}


static
gboolean
gst_mkvdmx_vm_reset (
    GstTcDemuxBase  * pstDemuxBase
    )
{
    //GstTcMkvDemux *p_mkvdmx = GST_MKVDMX(pstDemuxBase);
    gint32 result_pts;

   (void) gst_mkvdmx_vm_seek (pstDemuxBase, 0, 0, &result_pts);   //seek to 0ms

    return TCDMX_TRUE;
}

static
gboolean
gst_mkvdmx_vm_close (
    GstTcDemuxBase  * pstDemuxBase
    )
{
    GstTcMkvDemux *p_mkvdmx = GST_MKVDMX(pstDemuxBase);
    gboolean ret;
    mkv_dmx_inst_t *pst_inst = &p_mkvdmx->stInst;

    if ( pst_inst->hDmxHandle != NULL) {
        mkv_dmx_result_t res = pst_inst->pfnDmxLib( DMXOP_CLOSE, &pst_inst->hDmxHandle, NULL, NULL );
        if( res < 0 )
        {
            pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
            ERROR( "[%d] DMXOP_CLOSE Error:%s, %lld\n", pst_inst->stDmxError.s32ErrCode, pst_inst->stDmxError.pszErrStatus, pst_inst->stDmxError.s64LastStatus );
        }

        //release buffers
        if( pst_inst->stCdmxBuff.pbyStreamBuff != NULL )
        {
            pst_inst->stDmxInit.stMemoryFuncs.pfnFree(pst_inst->stCdmxBuff.pbyStreamBuff);
            pst_inst->stCdmxBuff.pbyStreamBuff = NULL;
            pst_inst->stCdmxBuff.ulStreamBuffSize = 0;
        }

        if (pst_inst->pstVorbisHeaders != NULL) {
            pst_inst->stDmxInit.stMemoryFuncs.pfnFree(pst_inst->pstVorbisHeaders);
            pst_inst->pstVorbisHeaders = NULL;
        }

        pst_inst->hDmxHandle = 0;
        (void)memset ( &pst_inst->stDmxInfo, 0, sizeof(mkv_dmx_info_t) );

        gst_mkvdmx_close_library(p_mkvdmx);
        ret = TCDMX_TRUE;
    }
    else {
        GST_ERROR_OBJECT (p_mkvdmx, "mkv already closed");
        ret = TCDMX_FALSE;
    }
    return ret;
}

static
gboolean
gst_mkvdmx_vm_getparam (
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
		*str = g_strdup("MKV");
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
//  GstTcMkvDemux private implementations
//
