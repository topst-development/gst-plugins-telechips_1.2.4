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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <memory.h>
#include <string.h>

#include "gst_tc_demux_base.h"


#define PROF_START(__string_tag__)\
{\
	gchar *prof_tag = __string_tag__;\
	gint64 prof_start = g_get_real_time();

#define PROF_END()\
	gint64 prof_end = g_get_real_time();\
	GST_INFO("[PROFILE][%s][LATENCY: %lld]", prof_tag, prof_end-prof_start);\
}

GST_DEBUG_CATEGORY_EXTERN (gst_tc_demux_debug);
#define GST_CAT_DEFAULT (gst_tc_demux_debug)

GST_DEBUG_CATEGORY_EXTERN (gst_tc_demux_out_debug);
#define GST_CAT_OUTLOG (gst_tc_demux_out_debug)

/* Filter signals and args */
enum
{
  /* FILL me */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_DISABLE_VIDEO,
  PROP_DISABLE_AUDIO,
  PROP_DISABLE_SUBTITLE,
  PROP_DISABLE_PRIVATE,
  PROP_START_OFFSET,
  PROP_MULTIPLE_STREAM,
};

#define gst_tcdmx_demux_parent_class (parent_class)
G_DEFINE_TYPE (GstTcDemuxBase, gst_tcdmx, GST_TYPE_ELEMENT);

static GstElementClass *parent_class = NULL;

/* helper etc */
static gchar *gs_apszStreamTypeString[TCDMX_TYPE_TYPE_MAX] = {
	"",			   //NOTHING
	"video",       //TCDMX_TYPE_VIDEO
	"audio",       //TCDMX_TYPE_AUDIO
	"subtitle",    //TCDMX_TYPE_SUBTITLE
	"private"      //TCDMX_TYPE_PRIVATE
};

#define TYPE_STRING(type)	(gs_apszStreamTypeString[type])


/* GObject virtual method implementations */
static void gst_tcdmx_class_init (GstTcDemuxBaseClass * pstDemuxBase);
static void gst_tcdmx_init (GstTcDemuxBase * pstDemux);
static void gst_tcdmx_finalize (GObject * pstObject);
static void gst_tcdmx_set_property (GObject * pstObject, guint uiPropID, const GValue * pstValue, GParamSpec * pstParamSpec);
static void gst_tcdmx_get_property (GObject * pstObject, guint uiPropID, GValue * pstValue, GParamSpec * pstParamSpec);

/* GstElement virtual method implementations */
GstStateChangeReturn gst_tcdmx_change_state (GstElement * pstElement, GstStateChange enTransition);
#if 0
static void gst_tcdmx_set_index (GstElement * pstElement, GstIndex * pstIndex);
static GstIndex * gst_tcdmx_get_index (GstElement * pstElement);
#endif

/* GstPad virtual method implementations for sink pad */
static gboolean gst_tcdmx_sink_activate (GstPad * pstSinkPad, GstObject * pstParent);
static gboolean gst_tcdmx_sink_activate_mode (GstPad    *	pstSinkPad,	GstObject *	parent,	GstPadMode	mode, gboolean	active);
static GstFlowReturn gst_tcdmx_chain (GstPad * pstSinkPad, GstObject *parent, GstBuffer * pstBuffer);
static gboolean gst_tcdmx_handle_sink_event (GstPad * pstSinkPad, GstObject * pstParent, GstEvent * pstEvent);
static void gst_tcdmx_loop (GstPad * pstSinkPad);

/* GstPad virtual method implementations for source pad */
static gboolean gst_tcdmx_handle_src_event (GstPad    * pstSrcPad, GstObject * pstParent, GstEvent  * pstEvent);
static gboolean gst_tcdmx_handle_src_query (GstPad * pstSrcPad, GstObject * pstParent, GstQuery * pstQuery);

/* GstTcDemuxBase virtual method declaration */
static GstFlowReturn gst_tcdmx_vm_demux_open (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags);
static gboolean gst_tcdmx_vm_demux_setinfo (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_tcdmx_vm_demux_demux (GstTcDemuxBase * pstDemuxBase, guint32 ulRequestType, tcdmx_result_t * pstResult);
static gboolean gst_tcdmx_vm_demux_seek (GstTcDemuxBase * pstDemuxBase, guint32 ulFlags, gint32 lTargetPts, gint32 * plResultPts);
static gboolean gst_tcdmx_vm_demux_reset (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_tcdmx_vm_demux_close (GstTcDemuxBase * pstDemuxBase);
static gboolean gst_tcdmx_vm_demux_getparam (GstTcDemuxBase * pstDemuxBase, gint param, void * ret);

/* GstTcDemuxBase private implementations */
static GstFlowReturn gst_tcdmx_demux_init (GstTcDemuxBase * pstDemux);
static GstFlowReturn gst_tcdmx_demux_run (GstTcDemuxBase * pstDemux, gint32 *plEosIndex);
static GstFlowReturn gst_tcdmx_demux_do_skimming (GstTcDemuxBase * pstDemux, gint32 * plEosIndex);
static GstFlowReturn gst_tcdmx_demux_push (GstTcDemuxBase  * pstDemux);
static GstFlowReturn gst_tcdmx_demux_push_internal (GstTcDemuxBase  * pstDemux, tcdmx_stream_t *p_stream, GstBuffer *p_buffer, GstPad *p_srcpad, tcdmx_result_t *p_result);

static gboolean gst_tcdmx_demux_seek (GstTcDemuxBase * pstDemux, GstSegment * pstSeekSegment);
static gboolean gst_tcdmx_demux_reset (GstTcDemuxBase * pstDemux);
static gboolean gst_tcdmx_demux_deinit (GstTcDemuxBase * pstDemux);

static gboolean gst_tcdmx_handle_seek_pull (GstTcDemuxBase * pstDemux, GstEvent * pstEvent);
static gboolean gst_tcdmx_handle_seek_push (GstTcDemuxBase * pstDemux, GstEvent * pstEvent);
static gboolean gst_tcdmx_add_srcpads (GstTcDemuxBase * pstDemux);
static gboolean gst_tcdmx_remove_srcpads (GstTcDemuxBase * pstDemux);
static gboolean gst_tcdmx_push_event (GstTcDemuxBase * pstDemux, GstEvent * pstEvent, gint32 lIndex);
static void gst_tcdmx_push_taglist (GstTcDemuxBase * pstDemux);
static void gst_tcdmx_update_taglist (GstTcDemuxBase * pstDemux);

static const gchar *gst_tcdmx_get_format_string(guint32 ulFormat);
static const gchar *gst_tcdmx_get_alignment_string(guint32 ulAlignment);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	helper etc
//
static 
GstEvent*
gst_tcdmx_new_new_segment (
	GstTcDemuxBase  * pstDemux
	)
{
	GstSegment segment;
	guint64 stop_pos;

	if (pstDemux->stSegment.stop == GST_CLOCK_TIME_NONE) {
		stop_pos = pstDemux->stSegment.duration;
	}
	else {
		stop_pos = pstDemux->stSegment.stop;
	}

	segment.flags           = GST_SEGMENT_FLAG_NONE;

	segment.rate            = pstDemux->stSegment.rate;
	segment.applied_rate    = pstDemux->stSegment.applied_rate;
	segment.format          = pstDemux->stSegment.format;
	segment.base            = pstDemux->stSegment.base;
	segment.offset          = pstDemux->stSegment.offset;
	segment.time            = pstDemux->stSegment.time;
	segment.position        = pstDemux->stSegment.position;
	segment.duration        = pstDemux->stSegment.duration;

	if (pstDemux->stSegment.rate > 0.0) {
		/* forwards goes from last_stop to stop */
		segment.start       = pstDemux->stSegment.position;
		segment.stop        = stop_pos;
	}
	else {
		/* reverse goes from start to last_stop */
		segment.start       = pstDemux->stSegment.start;
		segment.stop        = pstDemux->stSegment.position;
	}
	return gst_event_new_segment (&segment);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GObject virtual method implementations
//

/* initialize the tcdemuxbase's class */
static void gst_tcdmx_class_init (GstTcDemuxBaseClass  * pstDemuxBase)
{
	GObjectClass *p_object_class = (GObjectClass *) pstDemuxBase;
	GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxBase);

	GST_TRACE_OBJECT (pstDemuxBase, "class init");

	// object class overiding
	p_object_class->set_property = gst_tcdmx_set_property;
	p_object_class->get_property = gst_tcdmx_get_property;
	p_object_class->finalize = gst_tcdmx_finalize;

	parent_class = g_type_class_peek_parent (pstDemuxBase);

	// element class overiding
	p_element_class->change_state = GST_DEBUG_FUNCPTR (gst_tcdmx_change_state);
#if 0
	p_element_class->set_index = GST_DEBUG_FUNCPTR (gst_tcdmx_set_index);
	p_element_class->get_index = GST_DEBUG_FUNCPTR (gst_tcdmx_get_index);
#endif
	// base class virtual function setting
	pstDemuxBase->vmOpen      = gst_tcdmx_vm_demux_open;
	pstDemuxBase->vmSetInfo   = gst_tcdmx_vm_demux_setinfo;
	pstDemuxBase->vmDemux     = gst_tcdmx_vm_demux_demux;
	pstDemuxBase->vmSeek      = gst_tcdmx_vm_demux_seek;
	pstDemuxBase->vmReset     = gst_tcdmx_vm_demux_reset;
	pstDemuxBase->vmClose     = gst_tcdmx_vm_demux_close;
	pstDemuxBase->vmGetParam  = gst_tcdmx_vm_demux_getparam;

	// property installation
	g_object_class_install_property (p_object_class
		, PROP_DISABLE_VIDEO
		, g_param_spec_boolean ("video-off", "Disable video stream", "The demux element won't create video stream pad", TCDMX_FALSE, G_PARAM_READWRITE)
	);
	g_object_class_install_property (p_object_class
		, PROP_DISABLE_AUDIO
		, g_param_spec_boolean ("audio-off", "Disable audio stream", "The demux element won't create audio stream pad", TCDMX_FALSE, G_PARAM_READWRITE)
	);
	g_object_class_install_property (p_object_class
		, PROP_DISABLE_SUBTITLE
		, g_param_spec_boolean ("subtitle-off", "Disable subtitle stream", "The demux element won't create subtitle stream pad", TCDMX_FALSE, G_PARAM_READWRITE)
	);
	g_object_class_install_property (p_object_class
		, PROP_DISABLE_PRIVATE
		, g_param_spec_boolean ("private-off", "Disable private stream", "The demux element won't create private stream pad", TCDMX_FALSE, G_PARAM_READWRITE)
	);
	g_object_class_install_property (p_object_class
		, PROP_START_OFFSET
		, g_param_spec_int64 ("start-offset", "Start time offset", "Start time offset as nanosecond", -1, 0x7FFFFFFFFFFFFFFF, -1, G_PARAM_READWRITE)
	);
	g_object_class_install_property (p_object_class
		, PROP_MULTIPLE_STREAM
		, g_param_spec_boolean ("multiple-stream", "Demux multiple stream", "The demux element demuxes all the strean available", TCDMX_TRUE, G_PARAM_READWRITE)
	);
}


/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static 
void
gst_tcdmx_init (
	GstTcDemuxBase       * pstDemux
	)
{
	GST_TRACE_OBJECT (pstDemux, "instance init");

	/* element init */ 
	gst_tcdmx_set_default_demux_mode (pstDemux, TCDMX_MODE_SELECTIVE);
	(void)gst_tcdmx_demux_reset (pstDemux);

	pstDemux->llStartOffset = -1;
	pstDemux->request_id = 0;
	pstDemux->ulTotalAudioNum = 0;
	pstDemux->fSkimmingThreshold = 32.0;
	pstDemux->bNeed_to_attach_codec_data = TCDMX_FALSE;
	SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_MULTIPLE_STEAM);
	SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_GOTO_LSYNC_IF_SEEK_FAIL);
	CLEAR_FLAG (pstDemux->ulFlags, (TCDMX_BASE_FLAG_RINGMODE_ENABLE));

	pstDemux->pstDemuxLock = g_mutex_new();
	pstDemux->bInitDone = TCDMX_TRUE;
}


static 
void
gst_tcdmx_finalize (
	GObject  * pstObject
	)
{
	gint32 i;
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstObject);
	GST_TRACE_OBJECT (pstObject, "finialize");

	if(p_dmx->bPushMode != 0)
	{
		GST_TRACE_OBJECT (pstObject, "Pipe Lock Unref");
		g_mutex_free(p_dmx->pstPipe.pstLock);
		GST_TRACE_OBJECT (pstObject, "Pipe Cond Unref");
		g_cond_free(p_dmx->pstPipe.pstCond);

		for(i = 0;i < TCDMX_MAX_SRC_HANDLER;i++)
		{
			GST_TRACE_OBJECT (pstObject, "Pipe Adapter Unref [%d]", i);
			gst_object_unref (p_dmx->pstPipe.pstAdapter[i]);
		}

		(void)g_mutex_trylock (p_dmx->pstTaskLock);
		g_mutex_unlock (p_dmx->pstTaskLock);

		GST_TRACE_OBJECT (pstObject, "Task Unref");
		gst_object_unref (p_dmx->pstTask);
		GST_TRACE_OBJECT (pstObject, "Task Lock Clean");
		g_mutex_free (p_dmx->pstTaskLock);
	}

	g_mutex_free(p_dmx->pstDemuxLock);

	G_OBJECT_CLASS (parent_class)->finalize (pstObject);
}


//TODO: set property
static 
void
gst_tcdmx_set_property (
	GObject       * pstObject, 
	guint           uiPropID, 
	const GValue  * pstValue, 
	GParamSpec    * pstParamSpec
	)
{
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstObject);

	GST_TRACE_OBJECT (pstObject, "set property (ID: %d)", uiPropID);

	switch (uiPropID) {
	case PROP_DISABLE_VIDEO:
		if( g_value_get_boolean (pstValue) != 0 ){
			SET_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_DISABLE_VIDEO);
		} else {
			CLEAR_FLAG (p_dmx->ulFlags, (TCDMX_BASE_FLAG_DISABLE_VIDEO));
		}
		break;

	case PROP_DISABLE_AUDIO:
		if( (g_value_get_boolean (pstValue)) != 0 ) {
			SET_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_DISABLE_AUDIO);
		} else {
			CLEAR_FLAG (p_dmx->ulFlags, (TCDMX_BASE_FLAG_DISABLE_AUDIO));
		}
		break;

	case PROP_DISABLE_SUBTITLE:
		if( g_value_get_boolean (pstValue) != 0 ) {
			SET_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_DISABLE_SUBTITLE);
		} else { 
			CLEAR_FLAG (p_dmx->ulFlags, (TCDMX_BASE_FLAG_DISABLE_SUBTITLE));
		}
		break;

	case PROP_DISABLE_PRIVATE:
		if( g_value_get_boolean (pstValue) != 0 ) {
			SET_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_DISABLE_PRIVATE);
		} else {
			CLEAR_FLAG (p_dmx->ulFlags, (TCDMX_BASE_FLAG_DISABLE_PRIVATE));
		}
		break;

	case PROP_START_OFFSET:
		p_dmx->llStartOffset = g_value_get_int64 (pstValue);
		break;

	case PROP_MULTIPLE_STREAM:
		if( g_value_get_boolean (pstValue) != 0 ) {
			SET_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_MULTIPLE_STEAM);
		} else {
			CLEAR_FLAG (p_dmx->ulFlags, (TCDMX_BASE_FLAG_MULTIPLE_STEAM));
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (pstObject, uiPropID, pstParamSpec);
		break;
	}
}


//TODO: get property
static 
void
gst_tcdmx_get_property (
	GObject     * pstObject, 
	guint         uiPropID, 
	GValue      * pstValue, 
	GParamSpec  * pstParamSpec
	)
{
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstObject);

	GST_TRACE_OBJECT (pstObject, "get property (ID: %d)", uiPropID);

	switch (uiPropID) {

	case PROP_DISABLE_VIDEO:
		if (CHECK_FLAG((p_dmx->ulFlags), (TCDMX_BASE_FLAG_DISABLE_VIDEO))){
			g_value_set_boolean (pstValue, TCDMX_TRUE);
		} else {
			g_value_set_boolean (pstValue, TCDMX_FALSE);
		}
		break;

	case PROP_DISABLE_AUDIO:
		if (CHECK_FLAG(p_dmx->ulFlags, TCDMX_BASE_FLAG_DISABLE_AUDIO)){
			g_value_set_boolean (pstValue, TCDMX_TRUE);
		} else {
			g_value_set_boolean (pstValue, TCDMX_FALSE);
		}
		break;

	case PROP_DISABLE_SUBTITLE:
		if (CHECK_FLAG(p_dmx->ulFlags, TCDMX_BASE_FLAG_DISABLE_SUBTITLE)){
			g_value_set_boolean (pstValue, TCDMX_TRUE);
		} else {
			g_value_set_boolean (pstValue, FALSE);
		}
		break;

	case PROP_DISABLE_PRIVATE:
		if (CHECK_FLAG(p_dmx->ulFlags, TCDMX_BASE_FLAG_DISABLE_PRIVATE)){
			g_value_set_boolean (pstValue, TCDMX_TRUE);
		} else {
			g_value_set_boolean (pstValue, FALSE);
		}
		break;
	case PROP_START_OFFSET:
		g_value_set_int64 (pstValue, p_dmx->llStartOffset);
		break;

	case PROP_MULTIPLE_STREAM:
		if (CHECK_FLAG(p_dmx->ulFlags, TCDMX_BASE_FLAG_MULTIPLE_STEAM)){
			g_value_set_boolean (pstValue, TCDMX_TRUE);
		} else {
			g_value_set_boolean (pstValue, FALSE);
		}
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (pstObject, uiPropID, pstParamSpec);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstElement virtual method implementations
//
GstStateChangeReturn
gst_tcdmx_change_state (
	GstElement      * pstElement, 
	GstStateChange    enTransition
	)
{
	GstStateChangeReturn res;
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstElement);
	
	GST_TRACE_OBJECT (pstElement, "change state (transition: %d)", enTransition);
	
	switch (enTransition) {
	case GST_STATE_CHANGE_NULL_TO_READY:
		GST_DEBUG_OBJECT(p_dmx, "change state (pre): NULL --> READY \r\n");
		break;
	case GST_STATE_CHANGE_READY_TO_PAUSED:
		GST_DEBUG_OBJECT(p_dmx, "change state (pre): READY --> PAUSED \r\n");
		break;
	case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
		GST_DEBUG_OBJECT(p_dmx, "change state (pre): PAUSED --> PLAYING \r\n");
		break;
	case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
		GST_DEBUG_OBJECT(p_dmx, "change state (pre): PLAYING --> PAUSED \r\n");
		break;
	case GST_STATE_CHANGE_PAUSED_TO_READY:
		GST_DEBUG_OBJECT(p_dmx, "change state (pre): PAUSED --> READY \r\n");
		break;
	case GST_STATE_CHANGE_READY_TO_NULL:
		GST_DEBUG_OBJECT(p_dmx, "change state (pre): READY --> NULL \r\n");
		break;
	default:
		break;
	}
	
	res = GST_ELEMENT_CLASS (parent_class)->change_state (pstElement, enTransition);
	if (res == GST_STATE_CHANGE_FAILURE){
		goto done;
	}

	switch (enTransition) {
	case GST_STATE_CHANGE_NULL_TO_READY:
		GST_DEBUG_OBJECT(p_dmx, "change state (post): NULL --> READY \r\n");
		break;
	case GST_STATE_CHANGE_READY_TO_PAUSED:
		GST_DEBUG_OBJECT(p_dmx, "change state (post): READY --> PAUSED \r\n");
		break;
	case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
		GST_DEBUG_OBJECT(p_dmx, "change state (post): PAUSED --> PLAYING \r\n");
		break;
	case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
		GST_DEBUG_OBJECT(p_dmx, "change state (post): PLAYING --> PAUSED \r\n");
		break;
	case GST_STATE_CHANGE_PAUSED_TO_READY:
		GST_DEBUG_OBJECT(p_dmx, "change state (post): PAUSED --> READY \r\n");
		break;
	case GST_STATE_CHANGE_READY_TO_NULL:
		GST_DEBUG_OBJECT(p_dmx, "change state (post): READY --> NULL \r\n");
		if(p_dmx->bInitDone != 0){		
			(void)gst_tcdmx_demux_deinit (p_dmx);
		}
		break;
	default:
		break;
	}

done:
	return res;
}

#if 0
static 
void
gst_tcdmx_set_index (
	GstElement  * pstElement, 
	GstIndex    * pstIndex
	)
{
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstElement);

	GST_TRACE_OBJECT (pstElement, "set index");

	//TODO: set index
}


static 
GstIndex *
gst_tcdmx_get_index (
	GstElement  * pstElement
	)
{
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstElement);

	GST_TRACE_OBJECT (pstElement, "get index");

	//TODO: get index

	return NULL;
}
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstPad virtual method implementations for sink pad
//
#if 0//V1.0

static 
gboolean
gst_tcdmx_sink_activate (
	GstPad * pstSinkPad,
	GstObject * pstParent
	)
{
	gboolean res;
	GstTcDemuxBase * pstDemux = GST_DMXBASE  (pstParent);
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS  (pstParent);

	if (gst_pad_check_pull_range (pstSinkPad)) {
		GST_DEBUG ("Going to pull mode");
		return gst_pad_activate_mode (pstSinkPad, GST_PAD_MODE_PULL, TCDMX_TRUE);
	} 
	else {
#if SUPPORT_PUSH_MODE
		g_mutex_lock(pstDemux->pstDemuxLock);
		p_dmx_class->vmGetParam(pstDemux, TCDEMUX_PARAM_PUSHMODE_AVAILABLE, (void*)&res);
		g_mutex_unlock(pstDemux->pstDemuxLock);
		if( res == TCDMX_TRUE ) {
			GST_DEBUG ("going to push (streaming) mode");
			return gst_pad_activate_mode (pstSinkPad, GST_PAD_MODE_PUSH, TCDMX_TRUE);
		}
		else
		{
			GST_ERROR ("Push mode is not supported.");
			return TCDMX_FALSE;
		}
#else
		GST_ERROR ("Push mode is not supported.");
		return TCDMX_FALSE;
#endif
	}
}

#else

static 
gboolean
gst_tcdmx_sink_activate (
	GstPad * pstSinkPad,
	GstObject * pstParent
	)
{
	gboolean res;
	GstQuery *query;
	gboolean pull_mode;
	GstTcDemuxBase * pstDemux = GST_DMXBASE  (pstParent);
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS  (pstParent);

	GST_TRACE ("pstSinkPad 0x%p, pstParent 0x%p", pstSinkPad, pstParent);

	query = gst_query_new_scheduling ();

	if (gst_pad_peer_query (pstSinkPad, query) == 0) {
		gst_query_unref (query);
		goto activate_push;
	}

	pull_mode = gst_query_has_scheduling_mode_with_flags (query,
	GST_PAD_MODE_PULL, GST_SCHEDULING_FLAG_SEEKABLE);
	gst_query_unref (query);

	if (pull_mode == TCDMX_FALSE) {
		goto activate_push;
	}

	GST_DEBUG ("Going to pull mode");
	return gst_pad_activate_mode (pstSinkPad, GST_PAD_MODE_PULL, TCDMX_TRUE);

activate_push:
#if SUPPORT_PUSH_MODE
	g_mutex_lock(pstDemux->pstDemuxLock);
	(void)p_dmx_class->vmGetParam(pstDemux, TCDEMUX_PARAM_PUSHMODE_AVAILABLE, (void*)&res);
	g_mutex_unlock(pstDemux->pstDemuxLock);
	if( res == TCDMX_TRUE ) {
		GST_DEBUG ("going to push (streaming) mode");
		return gst_pad_activate_mode (pstSinkPad, GST_PAD_MODE_PUSH, TCDMX_TRUE);
	}
	else
	{
		GST_ERROR ("Push mode is not supported.");
		return TCDMX_FALSE;
	}
#else
	GST_ERROR ("Push mode is not supported.");
	return TCDMX_FALSE;
#endif
}

#endif


static 
gboolean
gst_tcdmx_sink_activate_pull (
	GstPad    * pstSinkPad, 
	gboolean    bActive
	)
{
	GstTcDemuxBase *p_dmx = GST_DMXBASE  (GST_OBJECT_PARENT (pstSinkPad));
	gboolean ret;

	GST_TRACE ("");

	if (bActive == TCDMX_TRUE) {
		p_dmx->bPushMode = TCDMX_FALSE;
		p_dmx->bSegmentRunning = TCDMX_TRUE;
		CLEAR_FLAG (p_dmx->ulFlags, (TCDMX_BASE_FLAG_STREAMING_SOURCE));
		ret = gst_pad_start_task (pstSinkPad, (GstTaskFunction) gst_tcdmx_loop, pstSinkPad, NULL);
	} 
	else {
		p_dmx->bSegmentRunning = TCDMX_FALSE;
		ret = gst_pad_stop_task (pstSinkPad);
	}
	return ret;
}


static 
gboolean
gst_tcdmx_sink_activate_push (
	GstPad    * pstSinkPad, 
	gboolean    bActive
	)
{
	gint32 i;
	gboolean res = TCDMX_TRUE;
	GstTcDemuxBase *p_dmx = GST_DMXBASE (GST_OBJECT_PARENT (pstSinkPad));
	
	GST_TRACE ("");

#if SUPPORT_PUSH_MODE

	if (bActive == TCDMX_TRUE) {
		GST_DEBUG ("ts: activating push/chain function");
		SET_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_STREAMING_SOURCE);
		if (!CHECK_FLAG ((p_dmx->ulFlags), (TCDMX_BASE_FLAG_FIXED_DEMUXING_MODE))) {
			SET_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_SEQUENTIAL_DEMUXING);
			CLEAR_FLAG (p_dmx->ulFlags, (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING));
		}

		p_dmx->bPushMode = TCDMX_TRUE;
		//Initialize demuxer thread
		p_dmx->pstTask = gst_task_new ((GstTaskFunction)gst_tcdmx_loop, pstSinkPad, NULL);
		p_dmx->pstTaskLock = g_mutex_new();
		p_dmx->pstPipe.pstLock = g_mutex_new();
		p_dmx->pstPipe.pstCond = g_cond_new();
		p_dmx->pstPipe.lCurrentIOIdx = 0;
		gst_task_set_lock (p_dmx->pstTask, p_dmx->pstTaskLock);//We do not need to initialize p_dmx->pstLock, since it is GRectMutex

		GST_DEBUG ("Creating Adapters");
		for(i = 0; i < TCDMX_MAX_SRC_HANDLER; i++)
		{
			p_dmx->pstPipe.pstAdapter[i] = gst_adapter_new ();
			p_dmx->pstPipe.llStartOffset[i] = -1;	//This should be changed by IO part
			p_dmx->pstPipe.llEndOffset[i] = -1;		//This should be changed by Chain part
			p_dmx->pstPipe.llCurrentOffset[i] = -1;	//This should be changed by IO part
			p_dmx->pstPipe.llInUse[i] = 0;
		}
		//First IO handler should wait for first packet anyway.
		//and its offset should be 0.
		p_dmx->pstPipe.llStartOffset[0] = 0;
		p_dmx->pstPipe.llEndOffset[0] = 0;
		p_dmx->pstPipe.llCurrentOffset[0] = 0;

		GST_DEBUG ("Activating loop function");
		if(gst_task_set_state (p_dmx->pstTask, GST_TASK_STARTED) == TCDMX_FALSE)
		{
			GST_DEBUG_OBJECT (p_dmx, "Demuxing Thread occurs an Error on creation");
			res = TCDMX_FALSE;
		}
	} 
	else
	{
		GstTcDemuxPipe	 * pstPipe = &p_dmx->pstPipe;
		/* release chain and loop */
		GST_TCDMX_PIPE_MUTEX_LOCK (pstPipe);
		pstPipe->enSrcResult = GST_FLOW_FLUSHING;
		/* end streaming by making ffmpeg believe eos */
		pstPipe->bEos = TCDMX_TRUE;
		GST_TCDMX_PIPE_SIGNAL (pstPipe);
		GST_TCDMX_PIPE_MUTEX_UNLOCK (pstPipe);

		/* make sure streaming ends */
		(void)gst_task_stop (p_dmx->pstTask);
		res = gst_task_join (p_dmx->pstTask);
	}
	
	return res;

#else

	GST_ERROR ("Push mode is not supported.");
	return TCDMX_FALSE;

#endif
}

static 
gboolean
gst_tcdmx_sink_activate_mode (
	GstPad    *	pstSinkPad,
	GstObject *	parent,
	GstPadMode	mode,
	gboolean	active
	)
{
	GstTcDemuxBase *p_dmx;
	gboolean ret;

	GST_TRACE ("");

	p_dmx = GST_DMXBASE (parent);

	if (mode == GST_PAD_MODE_PUSH) {
		gchar * ppContainerName;
		GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(p_dmx);

		ret = gst_tcdmx_sink_activate_push(pstSinkPad, active);

		g_mutex_lock(p_dmx->pstDemuxLock);
		(void)p_dmx_class->vmGetParam(p_dmx, TCDEMUX_PARAM_QUERY_CONTAINER_NAME, &ppContainerName);
		g_mutex_unlock(p_dmx->pstDemuxLock);

		if (strstr(ppContainerName, "MP4")!= 0) {
			GST_WARNING("[TCDEMUX] SEQUENTIAL MODE FORCED !! <%s> ", ppContainerName);
			gst_tcdmx_set_default_demux_mode(p_dmx, TCDMX_MODE_SEQUENTIAL);
		}
		g_free(ppContainerName);
	}
	else {
		ret = gst_tcdmx_sink_activate_pull(pstSinkPad, active);
	}

	return ret;
}


static 
GstFlowReturn
gst_tcdmx_chain (
	GstPad		* pstSinkPad, 
	GstObject	* parent,
	GstBuffer	* pstBuffer
	)
{
	gint32 tcdmx_chain_index;
	GstTcDemuxBase *p_dmx = NULL;
	GstTcDemuxPipe *pPipe;

	GST_TRACE ("pstSinkPad 0x%p", pstSinkPad);

#if SUPPORT_PUSH_MODE
	p_dmx = GST_DMXBASE (parent);
	pPipe = &p_dmx->pstPipe;
	GST_TCDMX_PIPE_MUTEX_LOCK (pPipe);

	tcdmx_chain_index = pPipe->lCurrentIOIdx;
	if(tcdmx_chain_index < 0)
	{
		GST_LOG_OBJECT (p_dmx, "Discarding data since io is not activated");
		goto ignore;
	}
	GST_LOG_OBJECT (p_dmx, "IDX : %d, received buffer of %d bytes at offset %"
					G_GUINT64_FORMAT, 
					tcdmx_chain_index,
					gst_buffer_get_size (pstBuffer),
					GST_BUFFER_OFFSET (pstBuffer));

	if (G_UNLIKELY (pPipe->bEos)){
		goto eos;
	}

	if (G_UNLIKELY (pPipe->enSrcResult != GST_FLOW_OK)){
		goto ignore;
	}

	GST_TRACE ("[IDX:%d]Giving a buffer of %d bytes", tcdmx_chain_index, gst_buffer_get_size (pstBuffer));

	gst_adapter_push (pPipe->pstAdapter[tcdmx_chain_index], pstBuffer);
	//Adjust offset variables
	if(pPipe->llStartOffset[tcdmx_chain_index] == -1)
	{
		//If this is first buffer for the IO structure, reset all offset variables
		pPipe->llStartOffset[tcdmx_chain_index] = (gint64)GST_BUFFER_OFFSET (pstBuffer);
		pPipe->llEndOffset[tcdmx_chain_index] = ((gint64)GST_BUFFER_OFFSET (pstBuffer) + (gint64)gst_buffer_get_size (pstBuffer));
	}
	else
	{
		pPipe->llEndOffset[tcdmx_chain_index] = (gint64)pPipe->llEndOffset[tcdmx_chain_index] + (gint64)gst_buffer_get_size (pstBuffer);
	}
	GST_TRACE ("[IDX:%d][Start : %lld][End : %lld][Cur : %lld",
					   tcdmx_chain_index, pPipe->llStartOffset[tcdmx_chain_index], pPipe->llEndOffset[tcdmx_chain_index], pPipe->llCurrentOffset[tcdmx_chain_index]);

	pstBuffer = NULL;
	//(end_offset - current_offset) is data that is available.
	if(pPipe->ullRequred != 0)
	{
		while ( (pPipe->llEndOffset[tcdmx_chain_index] - pPipe->llCurrentOffset[tcdmx_chain_index]) >= pPipe->ullRequred )
		{
			GST_TRACE ("[IDX:%d]Adapter has more that requested (p_dmx->pstPipe.ullRequred:%" G_GINT64_FORMAT")",
					   tcdmx_chain_index, pPipe->ullRequred);
			GST_TCDMX_PIPE_SIGNAL (pPipe);
			GST_TCDMX_PIPE_WAIT (pPipe);
			/* may have become flushing */
			if (G_UNLIKELY (pPipe->enSrcResult != GST_FLOW_OK)){
				goto ignore;
			}
		}
	}

	GST_TCDMX_PIPE_MUTEX_UNLOCK (pPipe);

    return GST_FLOW_OK;

/* special cases */
eos:
	{
		GST_DEBUG_OBJECT (p_dmx, "ignoring buffer at end-of-stream");
		GST_TCDMX_PIPE_MUTEX_UNLOCK (pPipe);

		gst_buffer_unref (pstBuffer);
		return GST_FLOW_EOS;
	}
ignore:
	{
		GST_DEBUG_OBJECT (p_dmx, "ignoring buffer because src task encountered %s",
		gst_flow_get_name (pPipe->enSrcResult));
		GST_TCDMX_PIPE_MUTEX_UNLOCK (pPipe);

		if (pstBuffer != 0){
			gst_buffer_unref (pstBuffer);
		}
		return GST_FLOW_FLUSHING;
	}

#else

	GST_ERROR ("Push mode is not supported.");
	return GST_FLOW_ERROR;

#endif
}


static 
gboolean
gst_tcdmx_handle_sink_event (
	GstPad    * pstSinkPad, 
	GstObject * pstParent, 
	GstEvent  * pstEvent
	)
{
#if SUPPORT_PUSH_MODE
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstParent);
	GstTcDemuxPipe *pPipe = &p_dmx->pstPipe;
	gboolean res = TCDMX_TRUE;

	GST_TRACE ("0x%p, %d, %s",pstSinkPad, pPipe->lCurrentIOIdx, GST_EVENT_TYPE_NAME(pstEvent));

	switch (GST_EVENT_TYPE (pstEvent)) 
	{
	case GST_EVENT_SEGMENT:
		GST_LOG_OBJECT (p_dmx, "GST_EVENT_NEWSEGMENT");
		gst_event_unref (pstEvent);
		break;
	case GST_EVENT_EOS:
		/* inform the src task that it can stop now */
		GST_LOG_OBJECT (p_dmx, "GST_EVENT_EOS");
		GST_TCDMX_PIPE_MUTEX_LOCK (pPipe);
		pPipe->bEos = TCDMX_TRUE;
		GST_TCDMX_PIPE_SIGNAL (pPipe);
		GST_TCDMX_PIPE_MUTEX_UNLOCK (pPipe);

		/* eat this event for now, task will send eos when finished */
		gst_event_unref (pstEvent);
		break;

	case GST_EVENT_FLUSH_START:
		//Pass it through
		GST_LOG_OBJECT (p_dmx, "GST_EVENT_FLUSH_START");
		//Unlock Chain Fnc.
		GST_TCDMX_PIPE_MUTEX_LOCK (pPipe);
		pPipe->enSrcResult = GST_FLOW_FLUSHING;
		GST_TCDMX_PIPE_SIGNAL (pPipe);
		GST_TCDMX_PIPE_MUTEX_UNLOCK (pPipe);
		gst_event_unref (pstEvent);
		break;

	case GST_EVENT_FLUSH_STOP:
		//Pass it through
		GST_LOG_OBJECT (p_dmx, "GST_EVENT_FLUSH_STOP");
		//Make them work normally
		GST_TCDMX_PIPE_MUTEX_LOCK (pPipe);

		pPipe->enSrcResult = GST_FLOW_OK;
		pPipe->bEos = TCDMX_FALSE;
		(void)gst_task_start(p_dmx->pstTask);
		GST_LOG_OBJECT (p_dmx, "Work Again");
		GST_TCDMX_PIPE_MUTEX_UNLOCK (pPipe);
		gst_event_unref (pstEvent);
		break;

	default:
		GST_LOG_OBJECT (p_dmx, "Unknown Event");
		gst_event_unref (pstEvent);
		GST_LOG_OBJECT (p_dmx, "Unknown Event Done");
		break;
	}
	return res;

#else

	GST_ERROR ("Push mode is not supported.");
	return TCDMX_FALSE;

#endif
}


static 
void
gst_tcdmx_loop (
	GstPad  * pstSinkPad
	)
{
	GstFlowReturn res = GST_FLOW_OK;
	GstTcDemuxBase *p_dmx = GST_DMXBASE (GST_PAD_PARENT (pstSinkPad));
	gboolean push_eos = TCDMX_FALSE;
	gint32 eos_index = 0;

	GST_TRACE ("");

	switch (p_dmx->enState) 
	{
	case TCDMX_STATE_CLOSED:
		res = gst_tcdmx_demux_init (p_dmx);
		if ( res != GST_FLOW_OK ) {
			goto loop_pause;
		}
		p_dmx->enState = TCDMX_STATE_OPENED;
		break;

	case TCDMX_STATE_OPENED:
		if ( gst_tcdmx_add_srcpads (p_dmx) == TCDMX_FALSE ) {
			res = GST_FLOW_NOT_LINKED;
			goto loop_pause;
		}
		gst_element_no_more_pads (GST_ELEMENT_CAST (p_dmx));
		p_dmx->enState = TCDMX_STATE_DEMUXING;
		break;

	case TCDMX_STATE_DEMUXING:
		if (CHECK_FLAG ((p_dmx->ulFlags), (TCDMX_BASE_FLAG_SEEKING))) {
			goto loop_pause;
		}
		#if 0
		if (G_UNLIKELY (p_dmx->pstTagList)) {
			(void)gst_tcdmx_push_taglist (p_dmx);
		}
		#endif
		if (G_UNLIKELY (p_dmx->pstCloseSegEvent)) {
			(void)gst_tcdmx_push_event (p_dmx, p_dmx->pstCloseSegEvent, -1);
			p_dmx->pstCloseSegEvent = NULL;
		}
		if (G_UNLIKELY (p_dmx->pstNewSegEvent)) {
			(void)gst_tcdmx_push_event (p_dmx, p_dmx->pstNewSegEvent, -1);
			p_dmx->pstNewSegEvent = NULL;
		}
		if ( (res = gst_tcdmx_demux_run (p_dmx, &eos_index)) != GST_FLOW_OK ) {
			if (res == GST_FLOW_EOS) {
				push_eos = TCDMX_TRUE;
				if (p_dmx->lEosCount >= p_dmx->lStreamCount) {
					goto loop_pause;
				}
				else
				{
					goto loop_eos;
				}
			}
			else {
				push_eos = TCDMX_TRUE;
				goto loop_pause;
			}
		}
		(void)gst_tcdmx_demux_push (p_dmx);
		break;

	case TCDMX_STATE_ERROR:
		(void)gst_tcdmx_demux_deinit (p_dmx);
		p_dmx->enState = TCDMX_STATE_CLOSED;
		goto loop_pause;

	default:
		res = GST_FLOW_ERROR;
		goto loop_pause;
	}

	return;

  /* ERRORS */
loop_pause:

	p_dmx->bSegmentRunning = TCDMX_FALSE;
	(void)gst_pad_pause_task (p_dmx->pstSinkPad);

	GST_LOG_OBJECT (p_dmx, "pausing task, reason %s", gst_flow_get_name (res));

	if (res == GST_FLOW_EOS) {
		/* handle end-of-stream/segment */
		if (((guint32)p_dmx->stSegment.flags & (guint32)GST_SEEK_FLAG_SEGMENT) > 0u) {
			gint64 stop_pos;
			if ((stop_pos = (gint64)p_dmx->stSegment.stop) == -1){
				stop_pos = (gint64)p_dmx->stSegment.duration;
			}
			GST_INFO_OBJECT (p_dmx, "sending segment_done");
			(void)gst_element_post_message (GST_ELEMENT_CAST (p_dmx), 
									  gst_message_new_segment_done (GST_OBJECT_CAST (p_dmx), GST_FORMAT_TIME, stop_pos));
		}
	}
	else if (res == GST_FLOW_ERROR_NOT_SUPPORT_DIVX)
	{
		GST_ELEMENT_ERROR (p_dmx, STREAM, DIVX_NOT_SUPPORT, (NULL), ("streaming stopped, reason %s", gst_flow_get_name (res)));
		p_dmx->enState = TCDMX_STATE_ERROR;
	}
	else if ((res == GST_FLOW_NOT_LINKED) || (res < GST_FLOW_EOS)) {
		/* for fatal errors we post an error message, wrong-state is
		* not fatal because it happens due to flushes and only means
		* that we should stop now. */
		GST_ELEMENT_ERROR (p_dmx, STREAM, FAILED, ("Internal data stream error."), ("streaming stopped, reason %s", gst_flow_get_name (res)));
		p_dmx->enState = TCDMX_STATE_ERROR;
	} else {
		GST_LOG_OBJECT (p_dmx, "do nothing");
	}

loop_eos:

	if (push_eos == TCDMX_TRUE) {
		GST_INFO_OBJECT (p_dmx, "sending eos, %d", eos_index);
		(void)gst_tcdmx_push_event (p_dmx, gst_event_new_eos (), eos_index);
		if(p_dmx->bPushMode == TCDMX_TRUE){
			(void)gst_task_pause(p_dmx->pstTask);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstPad virtual method implementations for source pad
//
static 
gboolean
gst_tcdmx_handle_src_event (
	GstPad    * pstSrcPad, 
	GstObject * pstParent,
	GstEvent  * pstEvent
	)
{
	gboolean res;
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstParent);

	GST_TRACE ("pstSrcPad 0x%p", pstSrcPad);

	GST_TRACE_OBJECT (p_dmx, "have event type %s: 0x%p on src pad", GST_EVENT_TYPE_NAME (pstEvent), pstEvent);

	switch (GST_EVENT_TYPE (pstEvent)) {
	case GST_EVENT_SEEK:
		if (p_dmx->bPushMode == TCDMX_TRUE){
			res = gst_tcdmx_handle_seek_push (p_dmx, pstEvent);
		}else{
			res = gst_tcdmx_handle_seek_pull (p_dmx, pstEvent);
		}
		gst_event_unref (pstEvent);
		break;

	case GST_EVENT_QOS:
		{
			/* 
			<Skimming Mode Change Scenario>
			   1) Normal Playing	: Do nothing for QOS event.
			   2) Slow Playing		: Do nothing for QOS event.
			   3) Fast Playing		: Parse diff value from QOS event then if it's over time threshold,
									  change mode to 4)
			   4) Skimming Playing	: Parse diff value from QOS event then if it's over time threshold,
									  increase seek interval.
			*/

			if((p_dmx->stSegment.rate > 1.0f) || (p_dmx->stSegment.rate < -1.0f))
			{
				p_dmx->ulAccumQOS++;
				if(p_dmx->ulAccumQOS > (guint)TCDMX_SKIMMING_QOS_THRESHOLD)
				{
					GST_OBJECT_LOCK (p_dmx);

					if(CHECK_FLAG((p_dmx->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_SKIMMING_BACKWARD))
					   || CHECK_FLAG((p_dmx->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_SKIMMING_FORWARD)))
					{
						//Increase seek interval since QOS event occurs too much
						p_dmx->lSkimmingInterval += (TCDMX_SKIMMING_DEFAULT_SEEKINTERVAL * GST_MSECOND);
					}
					else//This means it's under fastplay mode. so change mode to skimming
					{
						gint32 i;
						GstQOSType type;
						gdouble proportion;
						GstClockTimeDiff diff;
						GstClockTime timestamp;

						gst_event_parse_qos (pstEvent, &type, &proportion, &diff, &timestamp);
						GST_DEBUG_OBJECT (p_dmx, "qos (type: %d, propertion: %"G_GINT64_FORMAT", timestamp: %"G_GINT64_FORMAT
							, type, proportion, diff);

						//Seeking to proper position will be added later.
						GST_DEBUG_OBJECT (p_dmx, "Going into skimming mode[Rate : %f]", p_dmx->stSegment.rate);
						if(p_dmx->stSegment.rate > 0.0){
							SET_FLAG(p_dmx->ulSkimmingFlags, TCDMX_SKIMMING_MODE_SKIMMING_FORWARD);
						} else {
							SET_FLAG(p_dmx->ulSkimmingFlags, TCDMX_SKIMMING_MODE_SKIMMING_BACKWARD);
						}
						p_dmx->lSkimmingInterval = TCDMX_SKIMMING_DEFAULT_SEEKINTERVAL * GST_MSECOND;

						//skimming frame pts is necessary 
						//since last time pts could be decreasing due to the ordering of video frame
						for(i = 0;i < p_dmx->lStreamCount;i++){
							p_dmx->astStream[i].llSkimmingFramePTS = p_dmx->astStream[i].llLastTimestamp;
						}
					}
					p_dmx->ulAccumQOS = 0;
					GST_OBJECT_UNLOCK (p_dmx);
				}
			}
			res = TCDMX_TRUE;
		}
		gst_event_unref (pstEvent);
		break;

	case GST_EVENT_NAVIGATION:
		res = TCDMX_FALSE;
		gst_event_unref (pstEvent);
		break;

	default:
		res = gst_pad_push_event (p_dmx->pstSinkPad, pstEvent);
		break;
	}
	
	return res;
}

static 
gboolean
gst_tcdmx_handle_src_query (
	GstPad    * pstSrcPad, 
	GstObject * pstParent, 
	GstQuery  * pstQuery
	)
{
	gboolean res = TCDMX_TRUE;
	GstTcDemuxBase *p_dmx = GST_DMXBASE (pstParent);
	tcdmx_stream_t *p_stream = (tcdmx_stream_t*)gst_pad_get_element_private (pstSrcPad);

	GST_TRACE ("");

	if (p_stream == NULL){
		return gst_pad_query_default (pstSrcPad, pstParent, pstQuery);
	}

	switch (GST_QUERY_TYPE (pstQuery)) 
	{
		case GST_QUERY_POSITION:
		{
			GstFormat format;

			gst_query_parse_position (pstQuery, &format, NULL);

			if (format == GST_FORMAT_TIME) {
				gint64 position_ns = (gint64)p_stream->llLastTimestamp;
				GST_DEBUG_OBJECT (p_dmx, "position query : %" GST_TIME_FORMAT, GST_TIME_ARGS (position_ns));
				gst_query_set_position (pstQuery, GST_FORMAT_TIME, position_ns);
			}
			else if (format == GST_FORMAT_BYTES) {
				GST_DEBUG_OBJECT (p_dmx, "position query : %" G_GINT64_FORMAT " byte (%" G_GINT64_FORMAT" MB)", p_dmx->llLastReadPos, p_dmx->llLastReadPos/1048576);
				gst_query_set_position (pstQuery, GST_FORMAT_BYTES, p_dmx->llLastReadPos);
			}
			else {
				GST_WARNING ("duration query failed (type %d is not supported)", format);
				res = TCDMX_FALSE;
			}
			break;
		}
	
		case GST_QUERY_DURATION:
		{
			GstFormat format;

			gst_query_parse_duration (pstQuery, &format, NULL);

			if (format == GST_FORMAT_TIME) {
				if (!(CHECK_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_TIMESEEK_ENABLE) ||
				      CHECK_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_BYTESEEK_AVAILABLE))) {
					return gst_pad_peer_query (p_dmx->pstSinkPad, pstQuery);
				}
				else {
					gint64 duration_ns;
					if ( p_stream->ulStreamType == (guint32)TCDMX_TYPE_VIDEO ){
						duration_ns = (gint64)p_stream->unInfo.stVideo.ulDuration * GST_MSECOND;
					}else if ( p_stream->ulStreamType == (guint32)TCDMX_TYPE_AUDIO ){
						duration_ns = (gint64)p_stream->unInfo.stAudio.ulDuration * GST_MSECOND;
					}else {
						GST_DEBUG ("duration query failed (audio/video only support)");
						res = TCDMX_FALSE;
						break;
					}
					GST_DEBUG_OBJECT (p_dmx, "duration query : %" GST_TIME_FORMAT, GST_TIME_ARGS (duration_ns));
					gst_query_set_duration (pstQuery, GST_FORMAT_TIME, duration_ns);
				}
			}
			else if (format == GST_FORMAT_BYTES) {
				GST_DEBUG_OBJECT (p_dmx, "duration query : %" G_GINT64_FORMAT " byte (%" G_GINT64_FORMAT" MB)", p_dmx->llFileSize, p_dmx->llFileSize/1048576);
				gst_query_set_duration (pstQuery, GST_FORMAT_BYTES, p_dmx->llFileSize);
			}
			else {
				GST_WARNING ("duration query failed (type %d is not supported)", format);
				res = TCDMX_FALSE;
			}

			break;
		}

		case GST_QUERY_LATENCY:
		{
			res = gst_pad_peer_query (p_dmx->pstSinkPad, pstQuery);
			if (res == TCDMX_TRUE) {
				GstClockTime min_lat, max_lat;
				gboolean live;

				/* According to H.222.0
				   Annex D.0.3 (System Time Clock recovery in the decoder)
				   and D.0.2 (Audio and video presentation synchronization)

				   We can end up with an interval of up to 700ms between valid
				   PCR/SCR. We therefore allow a latency of 700ms for that.
				 */
				gst_query_parse_latency (pstQuery, &live, &min_lat, &max_lat);
				if (min_lat != (GstClockTime)-1){
				  min_lat += 700 * GST_MSECOND;
				}
				if (max_lat != (GstClockTime)-1){
				  max_lat += 700 * GST_MSECOND;
				}
				gst_query_set_latency (pstQuery, live, min_lat, max_lat);
			}
			break;
		}
	
		case GST_QUERY_SEEKING:
		{
			GstFormat format;
			gint64 duration;
			gboolean seekable;

			gst_query_parse_seeking (pstQuery, &format, NULL, NULL, NULL);
			if (format == GST_FORMAT_TIME)
			{
				if (CHECK_FLAG((p_dmx->ulFlags), (TCDMX_BASE_FLAG_TIMESEEK_ENABLE))){
					return gst_pad_peer_query (p_dmx->pstSinkPad, pstQuery);
				}
				if ( p_stream->ulStreamType == (guint32)TCDMX_TYPE_VIDEO ){
					duration = (gint64)p_stream->unInfo.stVideo.ulDuration * GST_MSECOND;
				}else if ( p_stream->ulStreamType == (guint32)TCDMX_TYPE_AUDIO ){
					duration = (gint64)p_stream->unInfo.stAudio.ulDuration * GST_MSECOND;
				}else {
					GST_WARNING ("duration query failed (audio/video only support)");
					res = TCDMX_FALSE;
					break;
				}
					
				seekable = (gint)CHECK_FLAG ((p_dmx->ulFlags), (TCDMX_BASE_FLAG_SEEK_ENABLE));
				GST_DEBUG_OBJECT (p_dmx, "seeking query : seekable %d, duration %" GST_TIME_FORMAT, CHECK_FLAG (p_dmx->ulFlags, TCDMX_BASE_FLAG_SEEK_ENABLE), GST_TIME_ARGS (duration));
				gst_query_set_seeking (pstQuery, GST_FORMAT_TIME, seekable, 0, duration);
				res = TCDMX_TRUE;
			}
			else
			{
				res = TCDMX_FALSE;
			}
			break;
		}
	
		//TODO
#if 0
		case GST_QUERY_CONVERT:
		{
			GstFormat src_fmt, dest_fmt;
			gint64 src_val, dest_val;

			GST_DEBUG_OBJECT (p_dmx, "query (type: GST_QUERY_CONVERT)");
			
			gst_query_parse_convert (pstQuery, &src_fmt, &src_val, &dest_fmt, &dest_val);
			if ((res = gst_tcdmx_src_convert (pstSrcPad, src_fmt, src_val, &dest_fmt, &dest_val)))
				gst_query_set_convert (pstQuery, src_fmt, src_val, dest_fmt, dest_val);
			else
				res = gst_pad_query_default (pstSrcPad, pstQuery);
			break;
		}
#endif

		default:
			GST_DEBUG_OBJECT (p_dmx, "Unsupported query (type: %s)", GST_QUERY_TYPE_NAME (pstQuery)); 
			res = gst_pad_query_default (pstSrcPad, pstParent, pstQuery);
			break;
	}

	return res;
}




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstTcDemuxBase virtual method declaration
//
static
GstFlowReturn
gst_tcdmx_vm_demux_open (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulFlags
	)
{
	GST_ERROR_OBJECT (pstDemuxBase, "not implemented ulFlags %d", ulFlags); 
	return GST_FLOW_ERROR;
}

static
gboolean
gst_tcdmx_vm_demux_setinfo (
	GstTcDemuxBase * pstDemuxBase
	)
{
	GST_ERROR_OBJECT (pstDemuxBase, "not implemented"); 
	return TCDMX_FALSE;
}


static
gboolean
gst_tcdmx_vm_demux_demux (
	GstTcDemuxBase  * pstDemuxBase,
	guint32           ulRequestType,
	tcdmx_result_t  * pstResult
	)
{
	GST_ERROR_OBJECT (pstDemuxBase, "not implemented, ulRequestType %d, pstResult 0x%p", ulRequestType ,pstResult); 
	return TCDMX_FALSE;
}


static
gboolean
gst_tcdmx_vm_demux_seek (
	GstTcDemuxBase  * pstDemuxBase, 
	guint32           ulFlags, 
	gint32            lTargetPts, 
	gint32          * plResultPts
	)
{
	GST_ERROR_OBJECT (pstDemuxBase, "not implemented ulFlags %d, lTargetPts %d, plResultPts 0x%p", ulFlags, lTargetPts, plResultPts); 
	return TCDMX_FALSE;
}

static
gboolean
gst_tcdmx_vm_demux_reset (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GST_ERROR_OBJECT (pstDemuxBase, "not implemented"); 
	return TCDMX_FALSE;
}


static
gboolean
gst_tcdmx_vm_demux_close (
	GstTcDemuxBase  * pstDemuxBase
	)
{
	GST_ERROR_OBJECT (pstDemuxBase, "not implemented"); 
	return TCDMX_FALSE;
}

static
gboolean
gst_tcdmx_vm_demux_getparam (
	GstTcDemuxBase * pstDemuxBase, 
	gint param,
	void * ret
	)
{
	GST_ERROR_OBJECT (pstDemuxBase, "not implemented, param %d, ret 0x%p", param, ret); 
	return TCDMX_FALSE;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	GstTcDemuxBase private implementations
//


static
gint32
gst_tcdmx_extract_normal_seqheader(
	const guint8 *	pbyData,
	gint32			lDataSize,
	guint8 **		ppbySeqHead,
	gint32 *		plHeadLength,
	guint32			ulSyncword
	)
{
	guint32 syncword = 0xFFFFFFFFU;
	gint32	start_pos = -1;
	gint32 end_pos = -1;
	gint32 i;

	syncword <<= 8;
	syncword |= pbyData[0];
	syncword <<= 8;
	syncword |= pbyData[1];
	syncword <<= 8;
	syncword |= pbyData[2];

	for(i = 3; i < lDataSize; i++) {
		syncword <<= 8;
		syncword |= pbyData[i];

		if( (syncword >> 8) == 1U ) {	// 0x 000001??
			if( syncword == ulSyncword ){
				start_pos = i-3;
			}else if( (start_pos >= 0) || (*plHeadLength > 0) ) {
				if (syncword == MPEG12_EXTENSION_STARTCODE) {
					GST_INFO("Found the extension code");
					continue;
				} else {
					end_pos = i-3;
					break;
				}
			}else {
				GST_LOG("do nothing");
			}
		}
	}

	if( start_pos >= 0 ) {
		if( end_pos >= 0 ) {
			*plHeadLength = end_pos-start_pos;
			(void)memcpy(*ppbySeqHead, pbyData + start_pos, (guint32)*plHeadLength);
			return 1;
		}
		else {
			*plHeadLength = lDataSize - start_pos;
			(void)memcpy(*ppbySeqHead, pbyData + start_pos, (guint32)*plHeadLength);
			return 0;
		}
	}
	else if( (*plHeadLength) > 0 )
	{
		// If we already found the start point of the sequence header in previous frame data, attach new DATA behind it.
		if( end_pos < 0 ){
			end_pos = lDataSize;
		}
		if ( (*plHeadLength + end_pos) > MAX_SEQ_HEADER_ALLOC_SIZE ) {// check the maximum threshold
			return 0;
		}
		(void)memcpy(*ppbySeqHead + *plHeadLength, pbyData, (guint32)end_pos);
		*plHeadLength += end_pos;
		return 1;
	}
	else {
		GST_LOG("do nothing");
	}

	return 0;
}

static
gint32 
gst_tcdmx_extract_mpeg4_seqheader(
	const guint8 *	pbyData, 
	gint32			lDataSize,
	guint8 **		ppbySeqHead,
	guint32 *		plHeadLength
	)
{
	guint32 syncword = 0xFFFFFFFFU;
	gint32	start_pos = -1;
	gint32 end_pos = -1;
	gint32 i;

	syncword <<= 8; 
	syncword |= pbyData[0];
	syncword <<= 8; 
	syncword |= pbyData[1];
	syncword <<= 8; 
	syncword |= pbyData[2];

	for(i = 3; i < lDataSize; i++) {
		syncword <<= 8; 
		syncword |= pbyData[i];

		if( (syncword >> 8) == 1U ) {	// 0x 000001??
			if( (syncword >= (guint32)MPEG4_VOL_STARTCODE_MIN) &&
				(syncword <= (guint32)MPEG4_VOL_STARTCODE_MAX) ){
				start_pos = i-3;
			} else {
				if( (start_pos >= 0) || (*plHeadLength > 0U) ) {
					if ( syncword == (guint32)MPEG4_VOP_STARTCODE )
					{
						end_pos = i-3;
						break;
					}
				}
			}
		}
	}
	if( start_pos >= 0 ) {
		if( end_pos >= 0 ) {
			*plHeadLength = (guint32)(end_pos-start_pos);
			(void)memcpy(*ppbySeqHead, pbyData + start_pos, *plHeadLength);
			return 1;
		}
		else {
			*plHeadLength = (guint32)(lDataSize - start_pos);
			(void)memcpy(*ppbySeqHead, pbyData + start_pos, *plHeadLength);
			return 0;
		}
	}
	else {
		if( *plHeadLength > 0U ) {
			if( end_pos < 0 ){
				end_pos = lDataSize;
			}
			if ( ((gint32)(*plHeadLength) + end_pos) > MAX_SEQ_HEADER_ALLOC_SIZE ) {// check the maximum threshold
				return 0;
			}
			(void)memcpy(*ppbySeqHead + *plHeadLength, pbyData, (guint32)end_pos);
			*plHeadLength += (guint32)end_pos;
			return 1;
		}
	}

	return 0;
}

#if 0
static
gint32
gst_tcdmx_extract_h264_seqheader(
	const guint8 *	pbyStreamData, 
	gint32			lStreamDataSize,
	guint8 **		ppbySeqHeaderData,
	gint32 *		plSeqHeaderSize
	)
{
	gint32 i;
	gint32 l_seq_start_pos = 0, l_seq_end_pos = 0, l_seq_length = 0; // Start Position, End Position, Length of the sequence header
	gint32 l_sps_found = 0;
	gint32 l_pps_found = 0;

	guint32 ul_read_word_buff;	   	    	            //4 byte temporary buffer 
	guint32 ul_masking_word_seq          = 0x0FFFFFFF;    //Masking Value for finding H.264 sequence header
	guint32 ul_masking_word_sync         = 0x00FFFFFF;    //Masking Value for finding sync word of H.264
	guint32 ul_h264_result_word_seq_SPS  = 0x07010000;    //Masking result should be this value in case of SPS. SPS Sequence header of H.264 must start by "00 00 01 x7"
	guint32 ul_h264_result_word_seq_PPS  = 0x08010000;    //Masking result should be this value in case of PPS. PPS Sequence header of H.264 must start by "00 00 01 x8"
	guint32 ul_h264_result_word_sync     = 0x00010000;    //Masking result should be this value. Sequence header of H.264 must start by "00 00 01 x7"

	if ( lStreamDataSize < 4 ){
		return 0; // there's no Seq. header in this frame. we need the next frame.
	}
	if ( *plSeqHeaderSize > 0 )
	{
		// we already find the sps, pps in previous frame
		l_sps_found = 1;
		l_pps_found = 1;
		l_seq_start_pos = 0;
	}
	else
	{
		// find the SPS of H.264 
		ul_read_word_buff = 0;
		ul_read_word_buff |= (guint)(pbyStreamData[0] << 8);
		ul_read_word_buff |= (guint)(pbyStreamData[1] << 16);
		ul_read_word_buff |= (guint)(pbyStreamData[2] << 24);

		for ( i = 0; i < lStreamDataSize-4; i++ )      
		{
			ul_read_word_buff = ul_read_word_buff >> 8;
			ul_read_word_buff &= (guint)0x00FFFFFF; 
			ul_read_word_buff |= (guint)(pbyStreamData[i+3] << 24);

			if ( (ul_read_word_buff & ul_masking_word_seq) == ul_h264_result_word_seq_SPS ) 
			{
				// SPS Sequence Header has been detected
				l_sps_found = 1;              
				l_seq_start_pos = i;          // save the start position of the sequence header 

				break;                        
			}

			// Continue to find the sps in next loop
		}

		if ( l_sps_found == 1 )
		{
			// Now, let's start to find the PPS of the Seq. header.

			i = i + 4; 
			ul_read_word_buff = 0;
			ul_read_word_buff |= (guint)(pbyStreamData[i] << 8);
			ul_read_word_buff |= (guint)(pbyStreamData[i+1] << 16);
			ul_read_word_buff |= (guint)(pbyStreamData[i+2] << 24);

			for (  ; i < lStreamDataSize - 4; i++ )    
			{
				ul_read_word_buff = ul_read_word_buff >> 8;
				ul_read_word_buff &= (guint)0x00FFFFFF; 
				ul_read_word_buff |= (guint)(pbyStreamData[i+3] << 24);

				if ( (ul_read_word_buff & ul_masking_word_seq) == ul_h264_result_word_seq_PPS ) 
				{
					// PPS has been detected. 
					l_pps_found = 1;

					break;
				}
				// Continue to find the pps in next loop
			}
		}
	}

	if ( l_pps_found == 1 )
	{
		// Now, let's start to find the next sync word to find the end position of Seq. Header
		if ( *plSeqHeaderSize > 0 ){
			i = 0;     // we already find the sps, pps in previous frame
		}else{
			i = i + 4;
		}
		ul_read_word_buff = 0;
		ul_read_word_buff |= (guint)(pbyStreamData[i] << 8);
		ul_read_word_buff |= (guint)(pbyStreamData[i+1] << 16);
		ul_read_word_buff |= (guint)(pbyStreamData[i+2] << 24);

		for ( ; i < lStreamDataSize - 4; i++ )    
		{
			ul_read_word_buff = ul_read_word_buff >> 8;
			ul_read_word_buff &= (guint)0x00FFFFFF; 
			ul_read_word_buff |= (guint)(pbyStreamData[i+3] << 24);

			if ( (ul_read_word_buff & ul_masking_word_sync) == ul_h264_result_word_sync ) 
			{
				gint32 l_cnt_zeros = 0;       // to count extra zeros ahead of "00 00 01"

				// next sync-word has been found.
				l_seq_end_pos = i - 1;      // save the end position of the sequence header (00 00 01 case)

				// any zeros can be added ahead of "00 00 01" sync word by H.264 specification. Count the number of these leading zeros.
				while (1)
				{
					l_cnt_zeros++;
					if ( pbyStreamData[i-l_cnt_zeros] == 0 )    
					{
						l_seq_end_pos = l_seq_end_pos -1;    // decrease the end position of Seq. Header by 1.
					}
					else{
						break;
					}
				}
				
				if ( *plSeqHeaderSize > 0 )
				{
					// we already find the sps, pps in previous frame
					l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;       

					if ( l_seq_length > 0 )
					{
						if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE ) {// check the maximum threshold
							return 0;
						}
						(void)memcpy( (guint8*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
					}
					
					return 1;
					
				}
				else
				{
					// calculate the length of the sequence header
					l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;       

					if ( l_seq_length > 0 )
					{
						(void)memcpy( (guint8*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = l_seq_length;

						return 1;  // We've found the sequence header successfully
					}
				}
			}
			// Continue to find the sync-word in next loop
		}
	}

	if ( (l_sps_found == 1) && (l_pps_found == 1))
	{
		// we found sps and pps, but we couldn't find the next sync word yet
		l_seq_end_pos = lStreamDataSize - 1;
		l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;        // calculate the length of the sequence header

		if ( *plSeqHeaderSize > 0 )
		{
			// we already saved the sps, pps in previous frame
			if ( l_seq_length > 0 )
			{
				if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE ){     // check the maximum threshold
					return 0;
				}
				(void)memcpy( (guint8*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
				*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
			}

		}
		else
		{
			(void)memcpy( (guint8*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
			*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
		}
	}

	return 0; // We couldn't find the complete sequence header yet. We need to search the next frame data.
}
#endif

static
gint32
gst_tcdmx_extract_vc1AP_seqheader(
	const guint8 *	pbyStreamData, 
	gint32			lStreamDataSize,
	guint8 **		ppbySeqHeaderData,
	gint32 *		plSeqHeaderSize
	)
{
	gint32 i;
	gint32 l_seq_start_pos, l_seq_end_pos, l_seq_length;	// Start Position, End Position, Length of the sequence header
	gint32 l_ssc_found = 0;
	gint32 l_epc_found = 0;

	guint32 ul_read_word_buff;							//4 byte temporary buffer 
	guint32 ul_masking_word_seq           = 0x0FFFFFFF;	//Masking Value for finding VC1 AP sequence header
	guint32 ul_masking_word_sync          = 0x0FFFFFFF;	//Masking Value for finding sync word of VC1 AP
	guint32 ul_vc1AP_result_word_seq_SC   = 0x0f010000;	//Masking result should be this value in case of SC. SC Sequence header of VC1 AP must start by "00 00 01 xF"
	guint32 ul_vc1AP_result_word_seq_EPC  = 0x0e010000;	//Masking result should be this value in case of EPC. EPC Sequence header of VC1 AP must start by "00 00 01 xE"
	guint32 ul_vc1AP_result_word_sync     = 0x0d010000;	//Masking result should be this value. Sequence header of VC1 AP must start by "00 00 01 0D"

	if ( lStreamDataSize < 4 ){
		return 0; // there's no Seq. header in this frame. we need the next frame.
	}
	if ( *plSeqHeaderSize > 0 )
	{
		// we already find the sc, epc in previous frame
		l_ssc_found = 1;
		l_epc_found = 1;
		l_seq_start_pos = 0;
	}
	else
	{
		// find the SC of VC1 AP 
		ul_read_word_buff = ((guint)pbyStreamData[0] << 8);
		ul_read_word_buff |= ((guint)pbyStreamData[1] << 16);
		ul_read_word_buff |= ((guint)pbyStreamData[2] << 24);

		for ( i = 0; i < (lStreamDataSize-4); i++ )      
		{
			ul_read_word_buff = ul_read_word_buff >> 8;
			ul_read_word_buff |= ((guint)pbyStreamData[i+3] << 24);

			if ( (ul_read_word_buff & ul_masking_word_seq) == ul_vc1AP_result_word_seq_SC ) 
			{
				// SC Sequence Header has been detected
				l_ssc_found = 1;              
				l_seq_start_pos = i;          // save the start position of the sequence header 
				break;                        
			}
			// Continue to find the sc in next loop
		}

		if ( l_ssc_found == 1 )
		{
			// Now, let's start to find the EPC of the Seq. header.

			i = i + 4; 
			ul_read_word_buff = ((guint)pbyStreamData[i] << 8);
			ul_read_word_buff |= ((guint)pbyStreamData[i+1] << 16);
			ul_read_word_buff |= ((guint)pbyStreamData[i+2] << 24);

			for (  ; i < (lStreamDataSize - 4); i++ )    
			{
				ul_read_word_buff = ul_read_word_buff >> 8;
				ul_read_word_buff |= ((guint)pbyStreamData[i+3] << 24);

				if ( (ul_read_word_buff & ul_masking_word_seq) == ul_vc1AP_result_word_seq_EPC ) 
				{
					// EPC has been detected. 
					l_epc_found = 1;
					break;
				}
				// Continue to find the epc in next loop
			}
		}
	}

	if ( l_epc_found == 1 )
	{
		// Now, let's start to find the next sync word to find the end position of Seq. Header

		if ( *plSeqHeaderSize > 0 ){
			i = 0;     // we already find the sc, epc in previous frame
		}else{
			i = i + 4;
		}
		ul_read_word_buff = ((guint)pbyStreamData[i] << 8);
		ul_read_word_buff |= ((guint)pbyStreamData[i+1] << 16);
		ul_read_word_buff |= ((guint)pbyStreamData[i+2] << 24);

		for ( ; i < (lStreamDataSize - 4); i++ )    
		{
			ul_read_word_buff = ul_read_word_buff >> 8;
			ul_read_word_buff |= ((guint)pbyStreamData[i+3] << 24);

			if ( (ul_read_word_buff & ul_masking_word_sync) == ul_vc1AP_result_word_sync ) 
			{
				gint32 l_cnt_zeros = 0;       // to count extra zeros ahead of "00 00 01"

				// next sync-word has been found.
				l_seq_end_pos = i - 1;      // save the end position of the sequence header (00 00 01 case)

				// any zeros can be added ahead of "00 00 01" sync word by VC1 AP specification. Count the number of these leading zeros.
				while (1)
				{
					l_cnt_zeros++;
					if ( pbyStreamData[i-l_cnt_zeros] == 0u )    
					{
						l_seq_end_pos = l_seq_end_pos -1;    // decrease the end position of Seq. Header by 1.
					}
					else{
						break;
					}
				}
				
				if ( *plSeqHeaderSize > 0 )
				{
					// we already find the sc, epc in previous frame
					l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;       

					if ( l_seq_length > 0 )
					{
						if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE ){ // check the maximum threshold
							return 0;
						}
						(void)memcpy( (guint8*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], (guint32)l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
					}
					return 1;
				}
				else
				{
					// calculate the length of the sequence header
					l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;       

					if ( l_seq_length > 0 )
					{
						(void)memcpy( (guint8*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], (guint32)l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = l_seq_length;
						return 1;  // We've found the sequence header successfully
					}
				}
			}
			// Continue to find the sync-word in next loop
		}
	}

	if ( (l_ssc_found == 1) && (l_epc_found == 1))
	{
		// we found sc and epc, but we couldn't find the next sync word yet
		l_seq_end_pos = lStreamDataSize - 1;
		l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;        // calculate the length of the sequence header

		if ( *plSeqHeaderSize > 0 )
		{
			// we already saved the sc, epc in previous frame
			if ( l_seq_length > 0 )
			{
				if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE ){     // check the maximum threshold
					return 0;
				}
				(void)memcpy( (guint8*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], (guint32)l_seq_length);   // save the seq. header to array
				*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
			}

		}
		else
		{
			(void)memcpy( (guint8*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], (guint32)l_seq_length);   // save the seq. header to array
			*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
		}
	}

	return 0; // We couldn't find the complete sequence header yet. We need to search the next frame data.
}

static
gint32 // -1: not implemented, 0: none, 1: extracted / implemented
gst_tcdmx_extract_seqhead(
	const guint8 *	pbyData, 
	gint32			lDataSize,
	guint8 **		ppbySeqHead,
	gint32 *		plHeadLength,
	guint32			ulFourCC
	)
{
	gint32 ret = -1;
	switch( ulFourCC ) 
	{
		/* MPEG-4 AVC / H.264 */
	case FOURCC_avc1: case FOURCC_AVC1:
	case FOURCC_h264: case FOURCC_H264:
	case FOURCC_x264: case FOURCC_X264:
	case FOURCC_vssh: case FOURCC_VSSH:
	case FOURCC_davc: case FOURCC_DAVC:
	case FOURCC_z264:
	case FOURCC_MVC:
#if 1
		//Since h264prase element denies annexb as extra codec data, do not make it.
		break;
#else
		if( pbyData == NULL )
			return 1;
		else
			return gst_tcdmx_extract_h264_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength);
#endif
		/* H.263 (not implemented) */
	case FOURCC_s263: case FOURCC_h263:
	case FOURCC_S263: case FOURCC_H263:
		break;

		/* Sorenson H.264 (not implemented) */
	case FOURCC_flv1: case FOURCC_FLV1:
		break;

		/* MPEG-4 */
	case FOURCC_mp4v: case FOURCC_MP4V:
	case FOURCC_SEDG:
	case FOURCC_RMP4:
	case FOURCC_xvid: case FOURCC_XVID: case FOURCC_Xvid:
	case FOURCC_divx: case FOURCC_DIVX:	
	case FOURCC_dx50: case FOURCC_DX50:
	case FOURCC_fmd4: case FOURCC_FMD4:
	case FOURCC_3iv2: case FOURCC_3IV2:
	case FOURCC_fvfw: case FOURCC_FVFW:
	case FOURCC_FMP4:
	case FOURCC_MP4S:
	case FOURCC_M4S2:
		if( pbyData == NULL ) {
			ret = 1;
		}else{
			ret = gst_tcdmx_extract_mpeg4_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength);
		}
		break;
		/* MPEG-1/2 */
	case FOURCC_mpeg: case FOURCC_MPEG:
	case FOURCC_MPG2: case FOURCC_mpg2: 
	case FOURCC_mpg1:
		if( pbyData == NULL ){
			ret = 1;
		}else{
			ret = gst_tcdmx_extract_normal_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength, MPEG12_SEQHEAD_STARTCODE);
		}
		break;
		/* Theora */
	case FOURCC_THEORA: case FOURCC_theora:
		break;

		/* VC-1 Simple/Main Profile (not implemented) */
	case FOURCC_WMV3: case FOURCC_wmv3:
		break;

		/* VC-1 Advanced Profile */
	case FOURCC_WVC1: case FOURCC_wvc1:
	case FOURCC_WMVA:
	case FOURCC_VC1:  case FOURCC_vc1:
		if( pbyData == NULL ){
			ret = 1;
		}else{
			ret = gst_tcdmx_extract_vc1AP_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength);
		}
		break;
		/* WMV-8 (not implemented) */
	case FOURCC_WMV2: case FOURCC_wmv2:
		break;

		/* WMV-7 (not implemented) */
	case FOURCC_WMV1: case FOURCC_wmv1:
		break;

		/* DIVX-3 (not implemented) */
	case FOURCC_div3: case FOURCC_DIV3:
	case FOURCC_div4: case FOURCC_DIV4:
	case FOURCC_mp43: case FOURCC_MP43:
		break;

		/* RV (not implemented) */
	case FOURCC_rv30: case FOURCC_RV30: 
	case FOURCC_rv40: case FOURCC_RV40: case FOURCC_RV89COMBO: 
		break;

		/* Moiton JPEG (not implemented) */
	case FOURCC_MJPG:
	case FOURCC_IJPG:
	case FOURCC_AVRn:
	case FOURCC_jpeg:
		break;

		/* AVS */
	case FOURCC_AVS: 
	case FOURCC_avs:
	case FOURCC_cavs:
		if( pbyData == NULL ){
			ret = 1;
		}else{
			ret = gst_tcdmx_extract_normal_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength, AVS_SEQHEAD_STARTCODE);
		}
		break;
		/* unknown standard */
	case FOURCC_MPG4:
	case FOURCC_col1: case FOURCC_COL1:
	case FOURCC_mp42: case FOURCC_MP42:
	default:
		// not supported
		break;
	}

	// not implemented
	return ret;
}

static
void
vol_putbits_init (
	guint8 *	pbyBitsOutbuf,
	tc_mpeg4_vol *	pstVolPutData
	) 
{
	pstVolPutData->m_pbyPtr		= (guchar *)pbyBitsOutbuf;
	pstVolPutData->m_ulPos		= 0;
	pstVolPutData->m_ulData		= 0;
	pstVolPutData->m_ulUsedBytes= 0;
}


static
guint32 
vol_putbits (
	guint8  ucNumBits,
	guint32 ulValue,
	guint32 ulOption,	//0 : not return
								//1 : return used bytes after bit stuffing for byte align
	tc_mpeg4_vol *	pstVolPutData
	)
{
	guint32 ret = 0;
	guint32 i, bit_pos;
	guint32 write_bits, used_bytes, bit_data;
	guint8* pby_bitptr;
	static const guint32 ul_stuff_bits[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};
	static const guint32 bit_mask[32] = 
	{ 	
		0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
		0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
		0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
		0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF 
	};
	if( (ucNumBits == 0U) && (ulOption == 0U) ){
		ret = 0;
	}
	else {

	bit_pos		= pstVolPutData->m_ulPos;	
	pby_bitptr	= pstVolPutData->m_pbyPtr;	
	bit_data	= pstVolPutData->m_ulData;
	used_bytes	= pstVolPutData->m_ulUsedBytes;

	write_bits = bit_pos + ucNumBits;
	if( write_bits < 32U )
	{
		bit_data = (bit_data<<ucNumBits) | ulValue;
		bit_pos +=  ucNumBits;
	}
	else
	{
		i = 32U-bit_pos;
		bit_data = (bit_data<<i)|((ulValue<<(32u-ucNumBits))>>bit_pos);

		*pby_bitptr = (guint8)((bit_data>>24)&0x0FFU);
		pby_bitptr++;
		*pby_bitptr = (guint8)((bit_data>>16)&0x0FFU);
		pby_bitptr++;
		*pby_bitptr = (guint8)((bit_data>> 8)&0x0FFU);
		pby_bitptr++;
		*pby_bitptr = (guint8)((bit_data    )&0x0FFU);
		pby_bitptr++;
		bit_pos		= ucNumBits - i;
		bit_data	= ulValue & bit_mask[bit_pos];
		used_bytes += 4U;

	}

	if( ulOption != 0U ) 
	{
		if( bit_pos > 24U )
		{
			bit_pos -= 24U;
			i = bit_data>>bit_pos;
			*pby_bitptr = (guint8)((i>>16)&0x0FFU);
			pby_bitptr++;
			*pby_bitptr = (guint8)((i>> 8)&0x0FFU);
			pby_bitptr++;
			*pby_bitptr = (guint8)((i    )&0x0FFU);
			pby_bitptr++;
			used_bytes += 3U;
		}
		else if ( bit_pos > 16U )
		{
			bit_pos -= 16U;
			i = bit_data>>bit_pos;
			*pby_bitptr = (guint8)((i>> 8)&0x0FFU);
			pby_bitptr++;
			*pby_bitptr = (guint8)((i    )&0x0FFU);
			pby_bitptr++;
			used_bytes += 2U;
		}
		else if ( bit_pos > 8U )
		{
			bit_pos -= 8U;
			i = bit_data>>bit_pos;
			*pby_bitptr = (guint8)((i    )&0x0FFU);
			pby_bitptr++;
			used_bytes += 1U;
		}
		else
		{
			GST_LOG("bit_pos : %d",bit_pos);
		}

		if( bit_pos > 0U )
		{
			i = 8 - (bit_pos & 0x7u);
			if(i != 8U)
			{
				*pby_bitptr = (guint8)( ((bit_data&bit_mask[bit_pos])<<i)|ul_stuff_bits[i - 1] );
				pby_bitptr++;
				used_bytes += 1U;
			}
		}
		ret = used_bytes;
	}

	pstVolPutData->m_pbyPtr		= pby_bitptr;
	pstVolPutData->m_ulPos		= bit_pos;
	pstVolPutData->m_ulData		= bit_data;
	pstVolPutData->m_ulUsedBytes= used_bytes;

	}
	return ret;
}

static
gint32
gst_tcdmx_make_vol( 
	GstBuffer *			pstOutSeqHeader,
	guint8 *			pbyData,
	guint32				ulDataLen,
	tc_video_info_t* 	pstDmxVideoInfo
	)
{
	// should be made VOL header.
	guint8* pby_vop = pbyData;
	guint32 idx;
	guint32 find = 0;
	tc_mpeg4_vol stVolPutData;
	gint32 ret = -1;

	for( idx = 0; idx < ulDataLen; ++idx )
	{
		if( (pby_vop[idx+2] ==  0x01u) && (pby_vop[idx+3] == 0xb6u) )
		{
			if( (pby_vop[idx+4] & 0xc0u) == 0u ) // VOP type I
			{
				find = 1;
				break;
			}
		}
	}

	if( find != 0U )
	{
		gint32 time_incr = 0;
		gint32 modulo = 0;
		guint32 bits;
		gint32 cnt;

		guint8* pby_outbuf;
		guint32 width, height;
		GstMapInfo mapInfo;

		(void)gst_buffer_map(pstOutSeqHeader, &mapInfo, GST_MAP_WRITE);

		pby_outbuf	= mapInfo.data;
		width		= pstDmxVideoInfo->ulWidth;
		height		= pstDmxVideoInfo->ulHeight;

		cnt = 4;
		bits = 2u;

		while ( ((pby_vop[cnt]<<bits)&0x80u) != 0u )	/* time_base */
		{
			bits++;
			if( bits == 8u )
			{
				bits = 0;
				cnt++;
			}
			modulo++;
		}
		bits++; // to the next bit
		if( bits == 8u )
		{
			bits = 0;
			cnt++;
		}
		bits++; // skip for marker bit
		if( bits == 8u )
		{
			bits = 0;
			cnt++;
		}

		while ( ((pby_vop[cnt]<<bits)&0x80u) == 0u )
		{
			bits++;
			if( bits == 8u )
			{
				bits = 0;
				cnt++;
			}
			time_incr++;
		}

		vol_putbits_init(pby_outbuf, &stVolPutData);

		//vol start code
		(void)vol_putbits(32, 0x120, 0, &stVolPutData);
		//random accessible vol
		(void)vol_putbits(1, 0, 0, &stVolPutData);

		// vo_type
		(void)vol_putbits(8, 1, 0, &stVolPutData);

		(void)vol_putbits(1, 0, 0, &stVolPutData);
		//aspect ratio
		(void)vol_putbits(4, 1, 0, &stVolPutData);

		//vol control
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//chroma format
		(void)vol_putbits(2, 1, 0, &stVolPutData); // 4:2:0 format

		//low delay
		(void)vol_putbits(1, 1, 0, &stVolPutData);

		//vbv
		(void)vol_putbits(1, 0, 0, &stVolPutData);
		//vo layer shape
		(void)vol_putbits(2, 0, 0, &stVolPutData); //rect

		//marker
		(void)vol_putbits(1, 1, 0, &stVolPutData);

//////////////////////////////////////////////////////////////////////////
		//vop time increment resolution
		(void)vol_putbits(16, (1<<time_incr)-1, 0, &stVolPutData);
		//marker
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//vop rate
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//vop time increment
		(void)vol_putbits((guint8)time_incr, 1, 0, &stVolPutData);
//////////////////////////////////////////////////////////////////////////

		//marker
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//width
		(void)vol_putbits(13, width, 0, &stVolPutData);
		//marker
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//height
		(void)vol_putbits(13, height, 0, &stVolPutData);
		//marker
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//interlaced
		(void)vol_putbits(1, 0, 0, &stVolPutData);
		//OBMC
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//splite Enable
		(void)vol_putbits(1, 0, 0, &stVolPutData);
		//not 8_bit
		(void)vol_putbits(1, 0, 0, &stVolPutData);
		//quant type
		(void)vol_putbits(1, 0, 0, &stVolPutData);
		//complexity estimation disable
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//resync_marker disable
		(void)vol_putbits(1, 1, 0, &stVolPutData);
		//data_partitioned
		(void)vol_putbits(1, 0, 0, &stVolPutData);
		//scalability
		(void)vol_putbits(1, 0, 0, &stVolPutData);
		//bit stuffing for byte align
		//scalability and bit stuffing for byte align
		mapInfo.size = vol_putbits(1, 0, 1, &stVolPutData);
		gst_buffer_set_size(pstOutSeqHeader, (gssize)mapInfo.size);
		gst_buffer_unmap(pstOutSeqHeader, &mapInfo);

        	ret = 0;
	}
	return ret;
}

static gboolean
gst_tcdmx_find_seq_header (
	GstTcDemuxBase  * pstDemux
	)
{
	gint32 i, count = 0;
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(pstDemux);
	gint32 request_type = (gint32)TCDMX_TYPE_ANY;
	gboolean found_seq_header = 0;
	gint32 result_time_ms;
	gboolean ret;

	memset(&pstDemux->stDmxResult, 0, sizeof(tcdmx_result_t));

	for(i = 0;i < pstDemux->lStreamCount;i++)
	{
		if(pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_VIDEO)
		{
			//In case we don't have private codec data.
			if(pstDemux->astStream[i].pstCodecData == NULL)
			{
				if(gst_tcdmx_extract_seqhead(NULL, 0, NULL, NULL, pstDemux->astStream[i].unInfo.stVideo.ulFourCC) > 0)
				{
					GstMapInfo mapInfo;
					pstDemux->astStream[i].pstCodecData = gst_buffer_new_and_alloc(MAX_SEQ_HEADER_ALLOC_SIZE);

					if(pstDemux->astStream[i].pstCodecData == NULL)
					{
						GST_ERROR_OBJECT (pstDemux, "CodecData Buffer Allocation Failed");
						continue;
					}
					(void)gst_buffer_map(pstDemux->astStream[i].pstCodecData, &mapInfo, GST_MAP_WRITE);

					request_type = (gint32)pstDemux->astStream[i].ulStreamType;
					pstDemux->request_id = pstDemux->astStream[i].ulStreamID;

					while(count < TCDMX_MAX_SEQ_SEARCH_TIME)
					{
						g_mutex_lock(pstDemux->pstDemuxLock);
						gboolean local_ret = p_dmx_class->vmDemux (pstDemux, (guint32)request_type, &pstDemux->stDmxResult);
						g_mutex_unlock(pstDemux->pstDemuxLock);
						if (local_ret != TCDMX_TRUE) 
						{
							GST_ERROR_OBJECT (pstDemux, "Get Stream Failed");
							break;
						}
						GST_LOG_OBJECT(pstDemux, "GetStream Result : %p, %d", pstDemux->stDmxResult.pBuffer, pstDemux->stDmxResult.lLength);
						if(pstDemux->stDmxResult.ulStreamType == (guint)TCDMX_TYPE_VIDEO)
						{
							mapInfo.size = 0;
							// extract sequence header
							if( gst_tcdmx_extract_seqhead(
								pstDemux->stDmxResult.pBuffer, 
								pstDemux->stDmxResult.lLength, 
								&mapInfo.data, 
								&mapInfo.size,
								pstDemux->astStream[i].unInfo.stVideo.ulFourCC) > 0 ) 
							{
								found_seq_header = 1;
								g_mutex_lock(pstDemux->pstDemuxLock);
								local_ret = p_dmx_class->vmSeek (pstDemux, 0, 0, &result_time_ms);
								g_mutex_unlock(pstDemux->pstDemuxLock);
								if (local_ret == TCDMX_FALSE)
								{
									GST_ERROR_OBJECT(pstDemux, "Seek failed during initialization");
									return TCDMX_FALSE;
								}
								break;
							}
						}
						count++;
					}
					gst_buffer_set_size(pstDemux->astStream[i].pstCodecData, mapInfo.size);
					gst_buffer_unmap(pstDemux->astStream[i].pstCodecData, &mapInfo);

					if((found_seq_header == 0)
					   && ((pstDemux->astStream[i].unInfo.stVideo.ulFourCC == FOURCC_mp4v) || (pstDemux->astStream[i].unInfo.stVideo.ulFourCC == FOURCC_MP4V)))
					{
						count = 0;
						//If possible, make VOL header for MPEG4 video
						g_mutex_lock(pstDemux->pstDemuxLock);
						ret = p_dmx_class->vmSeek (pstDemux, 0, 0, &result_time_ms);
						g_mutex_unlock(pstDemux->pstDemuxLock);
						if (ret == TCDMX_FALSE)
						{
							GST_ERROR_OBJECT(pstDemux, "Seek failed during initialization");
							return TCDMX_FALSE;
						}
						while(count < TCDMX_MAX_SEQ_SEARCH_TIME)
						{
							g_mutex_lock(pstDemux->pstDemuxLock);
							ret = p_dmx_class->vmDemux (pstDemux, (guint32)request_type, &pstDemux->stDmxResult);
							g_mutex_unlock(pstDemux->pstDemuxLock);
							if (ret != TCDMX_TRUE)
							{
								GST_ERROR_OBJECT (pstDemux, "Get Stream Failed");
								break;
							}
							if((pstDemux->stDmxResult.ulStreamType == (guint)TCDMX_TYPE_VIDEO) && (pstDemux->stDmxResult.lLength > 0))
							{
								// extract sequence header
								if(gst_tcdmx_make_vol(
									pstDemux->astStream[i].pstCodecData,
									pstDemux->stDmxResult.pBuffer,
									(guint32)pstDemux->stDmxResult.lLength,
									&pstDemux->astStream[i].unInfo.stVideo) == 0)
								{
									GST_LOG_OBJECT(pstDemux, "Successful VOP generation");
									break;
								}
							}
							count++;
						}

						g_mutex_lock(pstDemux->pstDemuxLock);
						ret = p_dmx_class->vmSeek (pstDemux, 0, 0, &result_time_ms);
						g_mutex_unlock(pstDemux->pstDemuxLock);
						if (ret == TCDMX_FALSE)
						{
							GST_ERROR_OBJECT(pstDemux, "Seek failed during initialization");
							return TCDMX_FALSE;
						}
					}
				}
			}
		}
	}
	return TCDMX_TRUE;
}

static
GstFlowReturn
gst_tcdmx_demux_init (
	GstTcDemuxBase  * pstDemux
	)
{
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(pstDemux);
	gint32 i, count;
	guint32 duration_max = 0;
	gchar *mode;
	GstFlowReturn return_value;
	gboolean ret;
	GstQuery *query;

	GST_INFO_OBJECT (pstDemux, "[TCDEMUX] Start Working");

	/* set open flags */
	if( mode = g_getenv ("GST_TCDMX_MODE") )
	{
		GST_INFO("[TCDEMUX] mode: %s", mode);
		if (strncmp(mode, "selective", 9) == 0) {
			GST_INFO("[TCDEMUX] SELECTIVE MODE FORCED !!");
			gst_tcdmx_set_default_demux_mode(pstDemux, TCDMX_MODE_SELECTIVE);
		}
		else if (strncmp(mode, "sequential", 10) == 0) {
			GST_INFO("[TCDEMUX] SEQUENTIAL MODE FORCED !!");
			gst_tcdmx_set_default_demux_mode(pstDemux, TCDMX_MODE_SEQUENTIAL);
		}
		else {
			GST_LOG_OBJECT (pstDemux, "do nothing");
		}
	}

	if( CHECK_FLAG((pstDemux->ulFlags), (TCDMX_BASE_FLAG_STREAMING_SOURCE)) )
	{
		query = gst_query_new_seeking (GST_FORMAT_TIME);
		if (gst_pad_peer_query(pstDemux->pstSinkPad, query) != 0) {
			gboolean seekable = TCDMX_FALSE;
			gint64 start = -1, stop = -1;
			gst_query_parse_seeking (query, NULL, &seekable, &start, &stop);
			if (seekable == TCDMX_TRUE) {
				GST_INFO_OBJECT (pstDemux, "time-seek available (%"GST_TIME_FORMAT" ~ %"GST_TIME_FORMAT")", GST_TIME_ARGS(start), GST_TIME_ARGS(stop));
				SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_TIMESEEK_AVAILABLE);
				pstDemux->stSegment.duration = (guint64)stop;
			}
			else {
				GST_INFO_OBJECT (pstDemux, "time-seek NOT available");
			}
		}
		else {
			GST_INFO_OBJECT (pstDemux, "time-seek querying failed");
		}
		gst_query_unref (query);
	}

	query = gst_query_new_seeking (GST_FORMAT_BYTES);
	if (gst_pad_peer_query(pstDemux->pstSinkPad, query) != 0) {
		gboolean seekable = TCDMX_FALSE;
		gint64 start = -1, stop = -1;
		gst_query_parse_seeking (query, NULL, &seekable, &start, &stop);
		if (seekable == TCDMX_TRUE) {
			GST_INFO_OBJECT (pstDemux, "byte-seek available (%lld ~ %lld)", start, stop);
			SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_BYTESEEK_AVAILABLE);
		}
		else {
			GST_INFO_OBJECT (pstDemux, "byte-seek NOT available");
		}
	}
	else {
		GST_INFO_OBJECT (pstDemux, "byte-seek querying failed");
	}
	gst_query_unref (query);

	/* demuxer open */
	PROF_START("DEMUXER-INIT");
	g_mutex_lock(pstDemux->pstDemuxLock);
	return_value = p_dmx_class->vmOpen (pstDemux, pstDemux->ulFlags);
	g_mutex_unlock(pstDemux->pstDemuxLock);
	if (return_value != GST_FLOW_OK){
		return return_value;
	}
	PROF_END();

	SET_FLAG(pstDemux->ulFlags, TCDMX_BASE_FLAG_DEMUX_OPENED);

	/* set configuration and stream info. */
	g_mutex_lock(pstDemux->pstDemuxLock);
	ret = p_dmx_class->vmSetInfo (pstDemux);
	g_mutex_unlock(pstDemux->pstDemuxLock);
	if (ret == TCDMX_FALSE){
		return GST_FLOW_NOT_SUPPORTED;	//
	}
	GST_LOG_OBJECT (pstDemux, "find sequence header");
	/* Find Seq_header and attach if it's necessary*/
	(void)gst_tcdmx_find_seq_header(pstDemux);
	GST_LOG_OBJECT (pstDemux, "find sequence header done");

	/* Get Duration */
		count = pstDemux->lStreamCount;
		for(i = 0; i < count; i++) {
			if( pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_VIDEO ) {
				if( duration_max < (guint32)pstDemux->astStream[i].unInfo.stVideo.ulDuration ){
					duration_max = pstDemux->astStream[i].unInfo.stVideo.ulDuration;
				}
			}
			else if( pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_AUDIO ) {
				if( duration_max < (guint32)pstDemux->astStream[i].unInfo.stAudio.ulDuration ){
					duration_max = pstDemux->astStream[i].unInfo.stAudio.ulDuration;
				}
			}
			else {
				GST_TRACE_OBJECT (pstDemux,"This stream isn't Audio or Video");
			}
		}

	if(duration_max != 0U){
		pstDemux->ulDuration_ms = duration_max ;
	}else{
		pstDemux->ulDuration_ms = 0;
	}
	if (!CHECK_FLAG ((pstDemux->ulFlags), (TCDMX_BASE_FLAG_TIMESEEK_ENABLE))) {
		//TODO: stream selection
		pstDemux->stSegment.format = GST_FORMAT_TIME;
		if(duration_max != 0U){
			pstDemux->stSegment.duration = duration_max * (guint64)GST_MSECOND;
		}else {//In case of streaming, duration is uncertain. so let it be max value.
			pstDemux->stSegment.duration = UINT_MAX * (guint64)GST_MSECOND;
		}
	}

	if (pstDemux->pstNewSegEvent != 0){
		gst_event_unref (pstDemux->pstNewSegEvent);
	}
	pstDemux->pstNewSegEvent = gst_tcdmx_new_new_segment (pstDemux);

	/* seek to start offset */
	if (pstDemux->llStartOffset >= 0) {
		GstEvent *p_seek_event = gst_event_new_seek(1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_KEY_UNIT, GST_SEEK_TYPE_SET, pstDemux->llStartOffset, GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE);

		if (pstDemux->bPushMode != 0){
			(void)gst_tcdmx_handle_seek_push (pstDemux, p_seek_event);
		}else{
			(void)gst_tcdmx_handle_seek_pull (pstDemux, p_seek_event);
		}
		gst_event_unref (p_seek_event);
	}

	GST_LOG_OBJECT (pstDemux, "init done");


	return GST_FLOW_OK;
}

static
void
gst_tcdmx_delay_subtitle(
	GstTcDemuxBase  * pstDemux
	)
{
	gint32 i;

	for (i = 0; i < pstDemux->lStreamCount; i++) 
	{
		if((pstDemux->astStream[i].ulStreamType != (guint)TCDMX_TYPE_VIDEO) && 
		   (pstDemux->astStream[i].ulStreamType != (guint)TCDMX_TYPE_AUDIO))
		{
			if (GST_CLOCK_TIME_IS_VALID (pstDemux->astStream[i].llLastTimestamp) &&
				GST_CLOCK_TIME_IS_VALID (pstDemux->stSegment.position) &&
				pstDemux->stSegment.position > pstDemux->stSegment.start &&
				(pstDemux->astStream[i].llLastTimestamp + (GST_SECOND / 2)) < (gint64)pstDemux->stSegment.position)
			{

				GstEvent *event;
				gint64 start = pstDemux->astStream[i].llLastTimestamp;
				gint64 stop = (gint64)pstDemux->stSegment.position - (GST_SECOND / 2);

				GST_DEBUG_OBJECT (pstDemux,
					"Synchronizing stream %d with other by advancing time from %"
					GST_TIME_FORMAT " to %" GST_TIME_FORMAT, i,
					GST_TIME_ARGS (start), GST_TIME_ARGS (stop));

				pstDemux->astStream[i].llLastTimestamp = stop;

				event = gst_event_new_gap (start, stop - start);
				(void)gst_pad_push_event (pstDemux->astStream[i].pstPad, event);
			}
		}
	}
}

static
GstFlowReturn
gst_tcdmx_demux_do_skimming (
	GstTcDemuxBase  * pstDemux,
	gint32          * plEosIndex
	)
{
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(pstDemux);
	gint32 i;

	guint32 doSkimmingSeek = 0;

	gint32 target_time_ms;
	gint32 result_time_ms = G_MAXINT32;

	gboolean ret;

	//In case any of abnormal playback mode is required except forward fastplay.
	if( CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_FASTPLAY_BACKWARD)) )
	{
		if( CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_FASTPLAY_SEGMENT_CONSUMED)) )
		{
			//Find next target segment
			target_time_ms = -1;
			for (i = 0; i < pstDemux->lStreamCount; i++) {
				if ( pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_VIDEO )
				{
					target_time_ms = (gint32)(pstDemux->astStream[i].llPreviousAnchorPTS / GST_MSECOND);
					break;
				}
			}
			if(target_time_ms != -1)
			{
				while( target_time_ms <= result_time_ms )
				{
					//Target PTS should be right before the llCurrentAnchorPTS
					target_time_ms -= 100;
					//If we are in the beginning of file enough, assume that it's done.
					if( target_time_ms < 0 )
					{
						if(CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_FASTPLAY_REACHED_BEGINNING)))
						{
							GST_INFO_OBJECT (pstDemux, "[BacwardFastPlay]Reached the beginning of the file");
							goto FORCED_EOS;
						}
						else
						{
							target_time_ms = 0;
							SET_FLAG (pstDemux->ulSkimmingFlags, TCDMX_SKIMMING_FASTPLAY_REACHED_BEGINNING);
						}
					}
					GST_INFO_OBJECT (pstDemux, "[BacwardFastPlay]Current Target Time[%"GST_TIME_FORMAT"]",
									GST_TIME_ARGS(target_time_ms * GST_MSECOND));

					g_mutex_lock(pstDemux->pstDemuxLock);
					ret = p_dmx_class->vmSeek (pstDemux, 0, target_time_ms, &result_time_ms);
					g_mutex_unlock(pstDemux->pstDemuxLock);
					if (ret == TCDMX_FALSE) {
						goto FORCED_EOS;
					}
					GST_INFO_OBJECT (pstDemux, "Done backward seek to %"GST_TIME_FORMAT"(intended : %"GST_TIME_FORMAT")",
									GST_TIME_ARGS(result_time_ms * GST_MSECOND),
									GST_TIME_ARGS(target_time_ms * GST_MSECOND));
				}
				//Seeking to previous anchor done.
				//So apply previous anchor on current anchor
				//and then clear previous anchor to get new previous anchor.
				for (i = 0; i < pstDemux->lStreamCount; i++)
				{
					if((pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_VIDEO) 
					   || (pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_AUDIO))
					{
						pstDemux->astStream[i].llCurrentAnchorPTS = pstDemux->astStream[i].llPreviousAnchorPTS;
						pstDemux->astStream[i].llPreviousAnchorPTS = -1;
					}
					//since new segment will be started.
					CLEAR_FLAG(pstDemux->astStream[i].ulFlags, (STREAM_FLAG_SEGMENT_CONSUMED));
				}
				GST_LOG_OBJECT (pstDemux, "[BacwardFastPlay]Done lowering New Anchor");
				CLEAR_FLAG (pstDemux->ulSkimmingFlags, (TCDMX_SKIMMING_FASTPLAY_SEGMENT_CONSUMED));
			}
			else
			{
				GST_LOG_OBJECT (pstDemux, "[BacwardFastPlay]Failed to get Target PTS");
				goto FORCED_EOS;
			}
		}
	}
	else if(CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_SKIMMING_FORWARD))
			|| CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_SKIMMING_BACKWARD)))
	{
		//Check if all the stream is already appeared.
		for(i = 0;i < pstDemux->lStreamCount;i++){
			doSkimmingSeek = doSkimmingSeek | (pstDemux->astStream[i].ulFlags & (guint)STREAM_FLAG_WAIT_SKIMMING_FRAME);
		}

		//It's under skimming mode and all the stream is ready for new position.
		if(doSkimmingSeek == 0U)
		{
			GST_LOG_OBJECT (pstDemux, "Skimming");

			//Get current video'PTS
			target_time_ms = -1;
			for (i = 0; i < pstDemux->lStreamCount; i++) {
				if ( pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_VIDEO )
				{
					target_time_ms = (gint32)(pstDemux->astStream[i].llSkimmingFramePTS / GST_MSECOND);
					break;
				}
			}
			if(CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_SKIMMING_FORWARD)))
			{
				result_time_ms = -1;
				while( (target_time_ms != -1) && (target_time_ms > result_time_ms) )
				{
					GST_INFO_OBJECT (pstDemux, "Skimming interval[%"GST_TIME_FORMAT"], Current Target Time[%"GST_TIME_FORMAT"]",
									GST_TIME_ARGS(pstDemux->lSkimmingInterval),
									GST_TIME_ARGS(target_time_ms * GST_MSECOND));
					target_time_ms += (gint32)((pstDemux->lSkimmingInterval / GST_MSECOND));

					g_mutex_lock(pstDemux->pstDemuxLock);
					ret = p_dmx_class->vmSeek (pstDemux, 0, target_time_ms, &result_time_ms);
					g_mutex_unlock(pstDemux->pstDemuxLock);
					if (ret == TCDMX_FALSE){
						goto FORCED_EOS;
					}
					GST_INFO_OBJECT (pstDemux, "Done forward seek to %"GST_TIME_FORMAT"(intended : %"GST_TIME_FORMAT")",
									GST_TIME_ARGS(result_time_ms * GST_MSECOND),
									GST_TIME_ARGS(target_time_ms * GST_MSECOND));
				}
			}
			else
			{
				result_time_ms = G_MAXINT32;
				while( (target_time_ms != -1) && (target_time_ms < result_time_ms) )
				{
					target_time_ms -= (gint32)((pstDemux->lSkimmingInterval / GST_MSECOND));
					if(target_time_ms < 0){
						target_time_ms = 0;
					}

					g_mutex_lock(pstDemux->pstDemuxLock);
					ret = p_dmx_class->vmSeek (pstDemux, 0, target_time_ms, &result_time_ms);
					g_mutex_unlock(pstDemux->pstDemuxLock);
					if ((ret == TCDMX_FALSE) || (result_time_ms < 500)){
						goto FORCED_EOS;
					}
					GST_INFO_OBJECT (pstDemux, "Done backward seek to %"GST_TIME_FORMAT"(intended : %"GST_TIME_FORMAT")",
									GST_TIME_ARGS(result_time_ms * GST_MSECOND),
									GST_TIME_ARGS(target_time_ms * GST_MSECOND));
				}
			}

			for (i = 0; i < pstDemux->lStreamCount; i++)
			{
				if((pstDemux->astStream[i].ulStreamType == (guint32)TCDMX_TYPE_VIDEO) 
				   || (pstDemux->astStream[i].ulStreamType == (guint32)TCDMX_TYPE_AUDIO))
				{
					pstDemux->astStream[i].ulFlags |= (guint)STREAM_FLAG_WAIT_SKIMMING_FRAME;
				}
			}
		}
	}
	else {
		GST_LOG_OBJECT (pstDemux, "do nothing");
	}
	return GST_FLOW_OK;

FORCED_EOS:
	//If seek failed, make all the stream EOS.
	for(i = 0; i < pstDemux->lStreamCount; i++) {
		pstDemux->astStream[i].ulFlags |= (guint)STREAM_FLAG_END_OF_STREAM;
	}
	pstDemux->lEosCount = pstDemux->lStreamCount;

	if( plEosIndex != NULL){
		*plEosIndex = -1;
	}

	GST_ERROR_OBJECT (pstDemux, "Failed seek to %"GST_TIME_FORMAT") for Skimming",
				GST_TIME_ARGS(target_time_ms * GST_MSECOND));

	return GST_FLOW_EOS;
}

static
GstFlowReturn
gst_tcdmx_demux_run (
	GstTcDemuxBase  * pstDemux,
	gint32          * plEosIndex
	)
{
	gboolean ret;
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(pstDemux);
	GstFlowReturn res = GST_FLOW_OK;
	tcdmx_stream_t *p_stream = NULL;
	gint32 tcdmx_stream_idx = -1;
	gint32 request_id = -1;
	gint32 request_type = (gint32)TCDMX_TYPE_ANY;
	memset(&pstDemux->stDmxResult, 0, sizeof(tcdmx_result_t));

	GST_LOG_OBJECT (pstDemux, "demux, skimming flag is 0x%08x", pstDemux->ulSkimmingFlags);

	if( plEosIndex != NULL) {
		*plEosIndex = 0;
	}

	if(gst_tcdmx_demux_do_skimming(pstDemux, plEosIndex) != GST_FLOW_OK){
		return GST_FLOW_EOS;
	}

	if (pstDemux->lEosCount >= pstDemux->lStreamCount) {
		//This could be happenning if seeking is failed after we already got EOS.
		return GST_FLOW_EOS;
	}

	/* decide next stream type */
	if( CHECK_FLAG ((pstDemux->ulFlags), (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING)) ) 
	{
		gint32 local_i;
		gint64 max_pts = 0;
		gint64 min_pts = G_MAXINT64;
		gint64 current_pts;
		gfloat rate = (gfloat)pstDemux->stSegment.rate;

		for (local_i = 0; local_i < pstDemux->lStreamCount; local_i++) {
			GST_LOG_OBJECT (pstDemux, "Flow : %d, PTS : %" G_GINT64_FORMAT"", 
							pstDemux->astStream[local_i].enLastFlow,
							pstDemux->astStream[local_i].llLastTimestamp);
			if ( pstDemux->astStream[local_i].enLastFlow != GST_FLOW_OK ){
				continue;
			}

			current_pts = pstDemux->astStream[local_i].llLastTimestamp;

			if ((rate > 0.0) && (current_pts < min_pts)) {
				min_pts = current_pts;
				tcdmx_stream_idx = local_i;
			} else if ((rate < 0.0) && (current_pts >= max_pts)) {
				max_pts = current_pts;
				tcdmx_stream_idx = local_i;
			} else {
				GST_LOG_OBJECT (pstDemux,"do nothing");
			}
		}

		g_return_val_if_fail (tcdmx_stream_idx >= 0, GST_FLOW_EOS);

		request_id = (gint32)pstDemux->astStream[tcdmx_stream_idx].ulStreamID;
		pstDemux->request_id = pstDemux->astStream[tcdmx_stream_idx].ulStreamID;
		request_type = (gint32)pstDemux->astStream[tcdmx_stream_idx].ulStreamType;
	}

	gst_tcdmx_delay_subtitle(pstDemux);

	/* demux run */
	g_mutex_lock(pstDemux->pstDemuxLock);
	ret = p_dmx_class->vmDemux (pstDemux, (guint32)request_type, &pstDemux->stDmxResult);
	g_mutex_unlock(pstDemux->pstDemuxLock);
	if (ret != TCDMX_TRUE) 
	{
		res = GST_FLOW_EOS;

		if( plEosIndex != NULL ){
			*plEosIndex = tcdmx_stream_idx;
		}

		if( tcdmx_stream_idx >= 0 ) {
			p_stream = pstDemux->astStream + tcdmx_stream_idx;
			pstDemux->astStream[tcdmx_stream_idx].enLastFlow = res;
		}

		if (CHECK_FLAG ((pstDemux->ulFlags), (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING))) {
			if ( (p_stream != NULL) && ((p_stream->ulFlags & (guint32)STREAM_FLAG_END_OF_STREAM)) == 0u ) {
				SET_FLAG(p_stream->ulFlags, STREAM_FLAG_END_OF_STREAM);
				pstDemux->lEosCount++;
			}
		}
		else {
			/* set eos to all streams */
			gint32 ii, count = pstDemux->lStreamCount;
			for(ii = 0; ii < count; ii++){
				pstDemux->astStream[ii].ulFlags |= (guint)STREAM_FLAG_END_OF_STREAM;
			}
			pstDemux->lEosCount = pstDemux->lStreamCount;
		}

		GST_ERROR_OBJECT (pstDemux, "eos (request_id: %d / selective: %d)", request_id, CHECK_FLAG ((pstDemux->ulFlags), (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING)) ? 1 : 0);
	}

	//Bias irregular PTS
	if(pstDemux->stDmxResult.lTimestampMs < 0){
		pstDemux->stDmxResult.lTimestampMs = 0;
	}

	return res;
}

static
GstFlowReturn 
gst_tcdmx_demux_push_internal (
	GstTcDemuxBase  * pstDemux,
	tcdmx_stream_t *p_stream,
	GstBuffer *p_buffer,
	GstPad *p_srcpad,
	tcdmx_result_t *p_result
	)
{
	GST_TRACE ("p_result 0x%p", p_result);

	GstFlowReturn res = GST_FLOW_OK;

	if(CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_SKIMMING_FORWARD))
	   || CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_SKIMMING_BACKWARD)))
	{
		//Make seek again for skimming if it's skimming mode
		if(( p_stream->ulStreamType == (guint)TCDMX_TYPE_VIDEO ) || ( p_stream->ulStreamType ==(guint) TCDMX_TYPE_AUDIO ))
		{
			if( CHECK_FLAG ((p_stream->ulFlags), (STREAM_FLAG_WAIT_SKIMMING_FRAME)) )
			{
				CLEAR_FLAG (p_stream->ulFlags, (STREAM_FLAG_WAIT_SKIMMING_FRAME));
				GST_INFO_OBJECT (pstDemux, "%s frame for Skimming [%"GST_TIME_FORMAT"]", TYPE_STRING(p_stream->ulStreamType), GST_TIME_ARGS(GST_BUFFER_PTS (p_buffer)));
				//Backup the first frame PTS for next skimming seek.
				p_stream->llSkimmingFramePTS = p_stream->llLastTimestamp;
				GST_BUFFER_FLAG_SET (p_buffer, (guint32)(GST_BUFFER_FLAG_DISCONT));

				res = gst_pad_push (p_srcpad, p_buffer);
			}
			else
			{
				//In case we don't need any more video chunk since we're waiting for audio.
				//just skip video chunk.
				gst_buffer_unref(p_buffer);
			}
		}
		else
		{
			//Let's skip except VIDEO/AUDIO
			gst_buffer_unref(p_buffer);
		}
	}
	else if(CHECK_FLAG ((pstDemux->ulSkimmingFlags), (TCDMX_SKIMMING_MODE_FASTPLAY_BACKWARD)))
	{
		gint32 i;
		guint32 temp_streamFlag;

		//If previous anchor PTS is not set for new segment, set it.
		if(p_stream->llPreviousAnchorPTS == -1)
		{
			p_stream->llPreviousAnchorPTS = p_stream->llLastTimestamp;
			//Since this buffer must be the first after seeking
			GST_BUFFER_FLAG_SET (p_buffer, (guint32)(GST_BUFFER_FLAG_DISCONT));
		}

		//If this is first anchoring,
		//do not push frame data.(we're just setting the first anchor.)
		if(p_stream->llCurrentAnchorPTS == -1)
		{
			SET_FLAG (p_stream->ulFlags, STREAM_FLAG_SEGMENT_CONSUMED);
			gst_buffer_unref(p_buffer);
		}
		else
		{
			//If this frame is within the segment, push it.
			if(p_stream->llLastTimestamp < p_stream->llCurrentAnchorPTS){
				res = gst_pad_push (p_srcpad, p_buffer);
			} else {
				SET_FLAG (p_stream->ulFlags, STREAM_FLAG_SEGMENT_CONSUMED);
				gst_buffer_unref(p_buffer);
			}
		}
		//All the processing for this frame is done.
		//Now check conditions for next segment.
		temp_streamFlag = 0;
		for (i = 0; i < pstDemux->lStreamCount; i++)
		{
			if((pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_VIDEO) 
			   || (pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_AUDIO))
			{
				temp_streamFlag |= (!CHECK_FLAG (p_stream->ulFlags, STREAM_FLAG_SEGMENT_CONSUMED));
			}

			//If all the stream consumed current segment,
			//get ready to move next segment.
			if(temp_streamFlag == 0U){
				SET_FLAG (pstDemux->ulSkimmingFlags, TCDMX_SKIMMING_FASTPLAY_SEGMENT_CONSUMED);
			}
		}
	}
	else
	{
		res = gst_pad_push (p_srcpad, p_buffer);
	}
	return res;
}

static 
GstFlowReturn 
gst_tcdmx_demux_push (
	GstTcDemuxBase  * pstDemux
	)
{
	gint32 i;
	GstFlowReturn res = GST_FLOW_OK;
	tcdmx_stream_t *p_stream;
	GstBuffer *p_buffer;
	GstPad *p_srcpad;
	gint32 tcdmx_stream_idx = -1;

	GST_TRACE ("");

	tcdmx_result_t   *p_result = &pstDemux->stDmxResult;

	for(i = 0;i < pstDemux->lStreamCount;i++)
	{
		if(pstDemux->acIndexMap[i] == (gint32)p_result->ulStreamID)
		{
			tcdmx_stream_idx = i;
			break;
		}
	}
	if( tcdmx_stream_idx >= 0 ) {
		p_stream = pstDemux->astStream + tcdmx_stream_idx;
		p_stream->llLastTimestamp = (gint64)p_result->lTimestampMs * GST_MSECOND;
		p_stream->llFrameDuration = (p_result->lEndTimestampMs > p_result->lTimestampMs) ?
			(gint64)(p_result->lEndTimestampMs - p_result->lTimestampMs) * GST_MSECOND : GST_CLOCK_TIME_NONE;
		p_stream->enLastFlow = GST_FLOW_OK;
		p_srcpad = p_stream->pstPad;
	}
	else
	{
		GST_CAT_ERROR_OBJECT(GST_CAT_OUTLOG, pstDemux,
			"Invalid stream [PTS: %d][ID : %d][LEN: %8d]"
		, p_result->lTimestampMs
		, p_result->ulStreamID
		, p_result->lLength);
		return GST_FLOW_ERROR;
	}
	
	if (p_srcpad != NULL) {
		GST_CAT_INFO_OBJECT (GST_CAT_OUTLOG, pstDemux
		, "[%c|%c|%c|%c][PTS: %"GST_TIME_FORMAT"][ID : %d][LEN: %8d]%s"
		, p_result->ulStreamType == TCDMX_TYPE_VIDEO ? 'V' : ' '
		, p_result->ulStreamType == TCDMX_TYPE_AUDIO ? 'A' : ' '
		, p_result->ulStreamType == TCDMX_TYPE_SUBTITLE ? 'S' : ' '
		, p_result->ulStreamType == TCDMX_TYPE_PRIVATE ? 'P' : ' '
		, GST_TIME_ARGS (p_stream->llLastTimestamp)
		, p_result->ulStreamID
		, p_result->lLength
		, CHECK_FLAG((p_stream->ulFlags), (STREAM_FLAG_DISCONTINUE)) ? " - new-segment" : ""
		);

#if 0//V1.0
		p_buffer = gst_buffer_new_and_alloc(p_result->lLength);
		memcpy( GST_BUFFER_DATA(p_buffer), p_result->pBuffer, p_result->lLength);
#else
		GstMapInfo map = GST_MAP_INFO_INIT;
		gsize p_buffer_sz = 0;
		if( (pstDemux->bNeed_to_attach_codec_data== TCDMX_TRUE) && (p_result->ulStreamType == (guint)TCDMX_TYPE_VIDEO) && (p_stream->pstCodecData != NULL)) {
			GST_CAT_INFO_OBJECT (GST_CAT_OUTLOG, pstDemux, "Attached sequence header!");
			GstMapInfo cmap = GST_MAP_INFO_INIT;
			(void)gst_buffer_map (p_stream->pstCodecData, &cmap, GST_MAP_READ);
			p_buffer_sz =  ((gsize)p_result->lLength + (gsize)cmap.size);
			p_buffer = gst_buffer_new_allocate(NULL, p_buffer_sz, NULL);
			(void)gst_buffer_map (p_buffer, &map, GST_MAP_WRITE);
			(void)memcpy (map.data, cmap.data, cmap.size );
			(void)memcpy (map.data + cmap.size, (gconstpointer)p_result->pBuffer, (guint32)p_result->lLength);
			gst_buffer_unmap(p_stream->pstCodecData, &cmap);
			gst_buffer_unref(p_stream->pstCodecData);
			p_stream->pstCodecData = NULL;
			pstDemux->bNeed_to_attach_codec_data = TCDMX_FALSE;
		} 
		else {
			p_buffer_sz = (gsize)p_result->lLength;
			p_buffer = gst_buffer_new_allocate(NULL, p_buffer_sz, NULL);

			(void)gst_buffer_map (p_buffer, &map, GST_MAP_WRITE);
			(void)memcpy (map.data, (gconstpointer)p_result->pBuffer, (guint32)p_result->lLength);
		}
		gst_buffer_unmap (p_buffer, &map);
#endif
		GST_BUFFER_FLAG_UNSET (p_buffer, (guint32)GST_BUFFER_FLAG_DELTA_UNIT);	//no-keyframe
		GST_BUFFER_PTS (p_buffer) = (GstClockTime)p_stream->llLastTimestamp;//V1.0
		GST_BUFFER_DURATION (p_buffer) = p_stream->llFrameDuration;
		GST_BUFFER_OFFSET (p_buffer) = GST_BUFFER_OFFSET_NONE;
		GST_BUFFER_OFFSET_END (p_buffer) = GST_BUFFER_OFFSET_NONE;
		gst_buffer_set_size (p_buffer, p_buffer_sz);//V1.0

		if ((p_stream->ulFlags & (guint32)STREAM_FLAG_DISCONTINUE) != 0u) {
			GST_BUFFER_FLAG_SET (p_buffer, (guint32)GST_BUFFER_FLAG_DISCONT);
			CLEAR_FLAG(p_stream->ulFlags, (STREAM_FLAG_DISCONTINUE));
		}

		/* update current position in the segment */
#if 0
		gst_segment_set_last_stop (&pstDemux->stSegment, GST_FORMAT_TIME, p_stream->llLastTimestamp);
#else
		pstDemux->stSegment.position = (guint64)p_stream->llLastTimestamp;
#endif

		(void)gst_tcdmx_demux_push_internal(pstDemux, p_stream, p_buffer, p_srcpad, p_result);

	}

	return res;
}

#define SEEK_INTREVAL (1000)
static 
gboolean 
gst_tcdmx_demux_seek (
	GstTcDemuxBase  * pstDemux, 
	GstSegment      * pstSeekSegment
	)
{
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(pstDemux);
	GstClockTime target_time_us;
	GstClockTime result_time_us;
	gint32 target_time_ms;
	gint32 seek_time_ms;
	gint32 result_time_ms = -1;
	gboolean keyframe, ret;

	GST_LOG_OBJECT (pstDemux, "seek");

	target_time_us = pstSeekSegment->position;//V1.0
	keyframe = !!(pstSeekSegment->flags & GST_SEEK_FLAG_KEY_UNIT);

	GST_DEBUG_OBJECT (pstDemux, 
					  "seek to: %" GST_TIME_FORMAT
					  " keyframe seeking: %d (ignored)", 
					  GST_TIME_ARGS (target_time_us), 
					  keyframe);
	
	//TODO: prev | key seeking
	target_time_ms = (gint32)(target_time_us / GST_MSECOND);




	PROF_START("DEMUXER-SEEK");
	//[Seek Scenario 1] : Just do seeking
	g_mutex_lock(pstDemux->pstDemuxLock);
	ret = p_dmx_class->vmSeek (pstDemux, (guint32)pstSeekSegment->flags, target_time_ms, &result_time_ms);
	g_mutex_unlock(pstDemux->pstDemuxLock);

	seek_time_ms = target_time_ms;
	if( ((guint32)pstSeekSegment->flags & (guint32)GST_SEEK_FLAG_SNAP_BEFORE) != 0u )
	{
		gint32 search_zero_count = 3;
		while( (target_time_ms < result_time_ms) && (search_zero_count > 0) )
		{
			seek_time_ms -= SEEK_INTREVAL;
			if( seek_time_ms < 0 )
			{
				seek_time_ms = 0;
				search_zero_count--;
			}
			ret = p_dmx_class->vmSeek (pstDemux, (guint32)pstSeekSegment->flags, seek_time_ms, &result_time_ms);
			if (ret == TCDMX_FALSE)
			{
			//[Seek Scenario 2] : If failed, return to the closest position before now
				GST_DEBUG_OBJECT (pstDemux, "@@[Fail] \n");
				break;
			}
		}
	}
	else if( ((guint32)pstSeekSegment->flags & (guint32)GST_SEEK_FLAG_SNAP_AFTER) != 0u )
	{
		while( target_time_ms > result_time_ms) 
		{
			seek_time_ms += SEEK_INTREVAL;
			if( seek_time_ms > (gint32)pstDemux->ulDuration_ms){
				break;
			}
			ret = p_dmx_class->vmSeek (pstDemux, (guint32)pstSeekSegment->flags, seek_time_ms, &result_time_ms);
			if (ret == TCDMX_FALSE)
			{
				break;
			}
		}
	}
	else {
		GST_LOG_OBJECT (pstDemux, "do nothing");
	}

	if (ret == TCDMX_FALSE)
	{
		return TCDMX_FALSE;
	}
	else
	{
		pstDemux->ulSeeked_ms = (guint32)result_time_ms;
	}
	PROF_END();

	result_time_us = (guint64)((guint64)result_time_ms * GST_MSECOND);

	/* the seek time is also the last_stop and stream time when going
	* forwards */
	pstSeekSegment->position = result_time_us;
	if (pstSeekSegment->rate > 0.0){
		pstSeekSegment->time = result_time_us;
	}

	GST_INFO_OBJECT (pstDemux,
					  "done seek to: %" GST_TIME_FORMAT,
					  GST_TIME_ARGS (result_time_us));

	return TCDMX_TRUE;
}


static
gboolean
gst_tcdmx_demux_reset (
	GstTcDemuxBase  * pstDemux
	)
{
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(pstDemux);

	GST_LOG_OBJECT (pstDemux, "reset");

	if (CHECK_FLAG((pstDemux->ulFlags), (TCDMX_BASE_FLAG_DEMUX_OPENED))) {
		g_mutex_lock(pstDemux->pstDemuxLock);
		p_dmx_class->vmReset(pstDemux);
		g_mutex_unlock(pstDemux->pstDemuxLock);
	}

	gst_segment_init (&pstDemux->stSegment, GST_FORMAT_TIME);
	return TCDMX_TRUE;
}


static
gboolean
gst_tcdmx_demux_deinit (
	GstTcDemuxBase  * pstDemux
	)
{
	GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(pstDemux);

	GST_LOG_OBJECT (pstDemux, "deinit");

	(void)gst_tcdmx_remove_srcpads (pstDemux);

	if (CHECK_FLAG((pstDemux->ulFlags), (TCDMX_BASE_FLAG_DEMUX_OPENED))) {
		g_mutex_lock(pstDemux->pstDemuxLock);
		p_dmx_class->vmClose(pstDemux);
		g_mutex_unlock(pstDemux->pstDemuxLock);
		CLEAR_FLAG(pstDemux->ulFlags, (TCDMX_BASE_FLAG_DEMUX_OPENED));
	}

	pstDemux->enState = TCDMX_STATE_CLOSED;

	/* remove sink pad to prevent memory leak */
	if (pstDemux->pstSinkPad != NULL)
	{
		(void)gst_pad_set_active (pstDemux->pstSinkPad, TCDMX_FALSE);
		(void)gst_element_remove_pad (GST_ELEMENT_CAST (pstDemux), pstDemux->pstSinkPad);
		while (GST_OBJECT_REFCOUNT(pstDemux->pstSinkPad) > 1u) {
			gst_object_unref(pstDemux->pstSinkPad);
		}
		pstDemux->pstSinkPad = NULL;
	}

	if(pstDemux->pstTagList != NULL) {
		gst_tag_list_unref(pstDemux->pstTagList);
		pstDemux->pstTagList = NULL;
	}

	gst_segment_init (&pstDemux->stSegment, GST_FORMAT_TIME);
	pstDemux->bInitDone = TCDMX_FALSE;
	return TCDMX_TRUE;
}


static 
gboolean
gst_tcdmx_handle_seek_pull (
	GstTcDemuxBase  * pstDemux, 
	GstEvent        * pstEvent
	)
{
	gdouble rate;
	GstFormat format;
	GstSeekFlags flags;
	GstSeekType curr_type = GST_SEEK_TYPE_NONE, stop_type;
	gint64 curr_pos, stop_pos;
	gboolean update;
	GstSegment seeksegment = { 0, }, currentsegment = {0, };
	gint32 i;

	GST_LOG_OBJECT (pstDemux, "seek");

	gst_event_parse_seek (pstEvent, 
						  &rate, 
						  &format, 
						  &flags, 
						  &curr_type, &curr_pos, 
						  &stop_type, &stop_pos);
		
	if (format != GST_FORMAT_TIME) {
		GST_DEBUG_OBJECT (pstDemux, "unsupported format given, seek aborted.");
		return TCDMX_FALSE;
	}

	GST_DEBUG_OBJECT (pstDemux
					  , "seek requested: rate %g cur %" GST_TIME_FORMAT " stop %" GST_TIME_FORMAT
					  , rate, GST_TIME_ARGS (curr_pos)
					  , GST_TIME_ARGS (stop_pos));

	GST_OBJECT_LOCK (pstDemux);
	SET_FLAG(pstDemux->ulFlags, TCDMX_BASE_FLAG_SEEKING);
	GST_OBJECT_UNLOCK (pstDemux);

	/* FIXME: can we do anything with rate!=1.0 */


	/* copy segment, we need this because we still need the old
	 * segment when we close the current segment. */
	(void)memcpy (&seeksegment, &pstDemux->stSegment, sizeof (GstSegment));
	(void)memcpy (&currentsegment, &pstDemux->stSegment, sizeof (GstSegment));

	if (pstEvent != 0) {
		GST_DEBUG_OBJECT (pstDemux, "configuring seek");
#if 0//V1.0
		gst_segment_set_seek (&seeksegment, rate, format, flags, curr_type, curr_pos, stop_type, stop_pos, &update);
#else
		(void)gst_segment_do_seek (&seeksegment, rate, format, flags, curr_type, curr_pos, stop_type, stop_pos, &update);
#endif
	}

	if (((guint32)flags & (guint32)GST_SEEK_FLAG_FLUSH) != 0u){
		gst_tcdmx_push_event (pstDemux, gst_event_new_flush_start(), -1);
	}
	
	/* do the seek, seeksegment.last_stop contains the new position, this
	 * actually never fails. */
	GST_OBJECT_LOCK (pstDemux);
	seeksegment.flags = (GstSegmentFlags)flags;
	(void)gst_tcdmx_demux_seek (pstDemux, &seeksegment);

	GST_OBJECT_UNLOCK (pstDemux);

	if (((guint32)flags & (guint32)GST_SEEK_FLAG_FLUSH) != 0u){
		(void)gst_tcdmx_push_event (pstDemux, gst_event_new_flush_stop (TCDMX_TRUE), -1);
	}

	/* now update the real segment info */
	(void)memcpy (&pstDemux->stSegment, &seeksegment, sizeof (GstSegment));
	
	/* post the SEGMENT_START message when we do segmented playback */
	if (((guint32)pstDemux->stSegment.flags & (guint32)GST_SEEK_FLAG_SEGMENT) != 0u) {
		(void)gst_element_post_message (GST_ELEMENT_CAST (pstDemux),
					  gst_message_new_segment_start (GST_OBJECT_CAST (pstDemux),
								 pstDemux->stSegment.format, 
								 (gint64)pstDemux->stSegment.position)
								  );
	}

	/* queue the segment event for the streaming thread. */
	if (pstDemux->pstNewSegEvent != 0){
		gst_event_unref (pstDemux->pstNewSegEvent);
	}
	pstDemux->pstNewSegEvent = gst_tcdmx_new_new_segment (pstDemux);

	/* reset the last flow and mark discont, seek is always DISCONT */
	//reset skimming threshold for max playrate
	for (i = 0; i < pstDemux->lStreamCount; i++) {
		pstDemux->astStream[i].enLastFlow = GST_FLOW_OK;
		pstDemux->astStream[i].ulFlags |= (guint)STREAM_FLAG_DISCONTINUE;
		pstDemux->astStream[i].ulFlags &= ~STREAM_FLAG_END_OF_STREAM;
		pstDemux->astStream[i].llLastTimestamp = (gint64)GST_CLOCK_TIME_NONE;
	} 
	pstDemux->lEosCount = 0;

	/* start task */
	GST_OBJECT_LOCK (pstDemux);
	CLEAR_FLAG(pstDemux->ulFlags, (TCDMX_BASE_FLAG_SEEKING));
	//Reset the number of QOS for mode change
	pstDemux->ulAccumQOS = 0;
	pstDemux->ulSkimmingFlags = 0;
	pstDemux->lSkimmingInterval = TCDMX_SKIMMING_DEFAULT_SEEKINTERVAL * GST_MSECOND;

	//Set new operation mode based on segment rate
	//At here, we don't care about forward fastplay.
	//since demuxer doesn't do anything to do fastplay.
	//At the spot of mode change, current stage will be noticed by pstDemux->stSegment.rate
	GST_DEBUG_OBJECT (pstDemux, "Input segment rate is %f, and the threshold is %f", pstDemux->stSegment.rate, pstDemux->fSkimmingThreshold);
	if( pstDemux->stSegment.rate > pstDemux->fSkimmingThreshold )
	{
		GST_DEBUG_OBJECT (pstDemux, "Going to forward skimming mode [%f]", pstDemux->stSegment.rate);
		SET_FLAG(pstDemux->ulSkimmingFlags, TCDMX_SKIMMING_MODE_SKIMMING_FORWARD);

		//Since seek is done once already, for the first output should be out without seek for skimming
		for (i = 0; i < pstDemux->lStreamCount; i++)
		{
			if((pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_VIDEO) 
			   || (pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_AUDIO))
			{
				pstDemux->astStream[i].ulFlags |= (guint)STREAM_FLAG_WAIT_SKIMMING_FRAME;
			}
		}
	}
#if SUPPORT_BACKWARD_FASTPLAY
	else if( pstDemux->stSegment.rate < (pstDemux->fSkimmingThreshold * -1) )
#else
	else if( pstDemux->stSegment.rate < 0.0 )
#endif
	{
		GST_DEBUG_OBJECT (pstDemux, "Going to backward skimming mode [%f]", pstDemux->stSegment.rate);
		SET_FLAG(pstDemux->ulSkimmingFlags, TCDMX_SKIMMING_MODE_SKIMMING_BACKWARD);

		//Since seek is done once already, for the first output should be out without seek for skimming
		for (i = 0; i < pstDemux->lStreamCount; i++)
		{
			if((pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_VIDEO) 
			   || (pstDemux->astStream[i].ulStreamType == (guint)TCDMX_TYPE_AUDIO))
			{
				pstDemux->astStream[i].ulFlags |= (guint)STREAM_FLAG_WAIT_SKIMMING_FRAME;
			}

			//skimming frame pts is necessary 
			//since last time pts could be decreasing due to the ordering of video frame
			pstDemux->astStream[i].llSkimmingFramePTS = pstDemux->astStream[i].llLastTimestamp;
		}
	}
#if SUPPORT_BACKWARD_FASTPLAY
	else if( pstDemux->stSegment.rate < 0.0 )
	{
		/* 
			In this mode, the first PTS of each stream will be the llCurrentAnchorPTS of each stream.
			and then, seeking before the llCurrentAnchorPTS will return 
		    new PTS which will be assigned for llPreviousAnchorPTS.
		 
		    Demuxer operates from llPreviousAnchorPTS to right before llCurrentAnchorPTS.
		    (demuxer will dispose frame data which has PTS bigger than llCurrentAnchorPTS)
		 
		    After all the stream reaches llCurrentAnchorPTS,
		    demuxer seeks right before llPreviousAnchorPTS, and assigns the first PTS to llPreviousAnchorPTS.
		    Previous llPreviousAnchorPTS value will be assigned for llCurrentAnchorPTS.
		*/
		GST_DEBUG_OBJECT (pstDemux, "Going to backward fastplay mode [%f]", pstDemux->stSegment.rate);
		SET_FLAG(pstDemux->ulSkimmingFlags, TCDMX_SKIMMING_MODE_FASTPLAY_BACKWARD);
		CLEAR_FLAG(pstDemux->ulSkimmingFlags, (TCDMX_SKIMMING_FASTPLAY_SEGMENT_CONSUMED));
		CLEAR_FLAG(pstDemux->ulSkimmingFlags, (TCDMX_SKIMMING_FASTPLAY_REACHED_BEGINNING));
		
		for (i = 0; i < pstDemux->lStreamCount; i++)
		{
			//Wait forever until anchor is set.
			pstDemux->astStream[i].llCurrentAnchorPTS = GST_CLOCK_TIME_NONE;
			pstDemux->astStream[i].llPreviousAnchorPTS = GST_CLOCK_TIME_NONE;
			CLEAR_FLAG(pstDemux->astStream[i].ulFlags, (STREAM_FLAG_SEGMENT_CONSUMED));
		}
	}
#endif
	else {
		GST_LOG_OBJECT (pstDemux, "do nothing");
	}

	GST_OBJECT_UNLOCK (pstDemux);

	pstDemux->bSegmentRunning = TCDMX_TRUE;
	(void)gst_pad_start_task (pstDemux->pstSinkPad, (GstTaskFunction) gst_tcdmx_loop, pstDemux->pstSinkPad, NULL);
	
	return TCDMX_TRUE;
}


static 
gboolean
gst_tcdmx_handle_seek_push (
	GstTcDemuxBase  * pstDemux, 
	GstEvent        * pstEvent
	)
{
	gdouble rate;
	GstFormat format;
	GstSeekFlags flags;
	GstSeekType curr_type = GST_SEEK_TYPE_NONE, stop_type;
	gint64 curr_pos, stop_pos;
	gboolean update;
	gboolean res;
	gint i;
	GstSegment seeksegment = { 0, };

	GST_DEBUG_OBJECT (pstDemux, "seek");

	gst_event_parse_seek (pstEvent, 
						  &rate, 
						  &format, 
						  &flags, 
						  &curr_type, &curr_pos, 
						  &stop_type, &stop_pos);
		
	if (format != GST_FORMAT_TIME) {
		GST_DEBUG_OBJECT (pstDemux, "unsupported format given, seek aborted.");
		gst_event_unref (pstEvent);
		return TCDMX_FALSE;
	}

	GST_OBJECT_LOCK (pstDemux);
	SET_FLAG(pstDemux->ulFlags, TCDMX_BASE_FLAG_SEEKING);
	if(gst_task_set_state (pstDemux->pstTask, GST_TASK_PAUSED)==TCDMX_FALSE)
	{
		GST_DEBUG_OBJECT (pstDemux, "Demuxing Thread occurs an Error on creation");
		GST_OBJECT_UNLOCK (pstDemux);
		return TCDMX_FALSE;
	}
	GST_OBJECT_UNLOCK (pstDemux);
	(void)memcpy (&seeksegment, &pstDemux->stSegment, sizeof (GstSegment));

	if (pstEvent != NULL) {
		GST_DEBUG_OBJECT (pstDemux, "configuring seek");
		(void)gst_segment_do_seek (&seeksegment, rate, format, flags, curr_type, (guint64)curr_pos, stop_type, (guint64)stop_pos, &update);
	}

	if (CHECK_FLAG ((pstDemux->ulFlags), (TCDMX_BASE_FLAG_TIMESEEK_ENABLE))) 
	{
		(void)gst_event_ref (pstEvent);
		res = gst_pad_push_event (pstDemux->pstSinkPad, pstEvent);
		if (res == TCDMX_FALSE) {
			GST_ERROR_OBJECT (pstDemux, "TimeSeek failed");
			goto CONTINUE;
		}

		// set segment
		if (pstDemux->stSegment.rate > 0.0){
			pstDemux->stSegment.time = pstDemux->stSegment.position;
		}

		GST_INFO_OBJECT (pstDemux, "TimeSeek complete");
	}

	if (((guint64)flags & (guint64)GST_SEEK_FLAG_FLUSH) != 0u){
		(void)gst_tcdmx_push_event(pstDemux, gst_event_new_flush_start(), -1);
	}

	GST_OBJECT_LOCK (pstDemux);
	res = gst_tcdmx_demux_seek (pstDemux, &seeksegment);
	if(res == TCDMX_FALSE)
	{
		if (((guint64)flags & (guint64)GST_SEEK_FLAG_FLUSH) != 0u){
			(void)gst_tcdmx_push_event(pstDemux, gst_event_new_flush_stop(TCDMX_TRUE), -1);
		}

		GST_ERROR_OBJECT (pstDemux, "seek failed");
		CLEAR_FLAG(pstDemux->ulFlags, (TCDMX_BASE_FLAG_SEEKING));
		pstDemux->bSegmentRunning = TCDMX_TRUE;
		GST_OBJECT_UNLOCK (pstDemux);
		goto CONTINUE;
	}
	GST_OBJECT_UNLOCK (pstDemux);

	if (((guint64)flags & (guint64)GST_SEEK_FLAG_FLUSH) != 0u){
		(void)gst_tcdmx_push_event(pstDemux, gst_event_new_flush_stop(TCDMX_TRUE), -1);
	}

	/* now update the real segment info */
	if (!CHECK_FLAG ((pstDemux->ulFlags), (TCDMX_BASE_FLAG_TIMESEEK_ENABLE)))
	{
		GST_INFO_OBJECT (pstDemux, "Updating segment info");
		(void)memcpy (&pstDemux->stSegment, &seeksegment, sizeof (GstSegment));
	}

	/* queue the segment event for the streaming thread. */
	if (pstDemux->pstNewSegEvent !=  NULL){
		gst_event_unref (pstDemux->pstNewSegEvent);
	}
	pstDemux->pstNewSegEvent = gst_tcdmx_new_new_segment (pstDemux);

	/* reset the last flow and mark discont, seek is always DISCONT */
	for (i = 0; i < pstDemux->lStreamCount; i++) {
		pstDemux->astStream[i].enLastFlow = GST_FLOW_OK;
		pstDemux->astStream[i].ulFlags |= (guint)STREAM_FLAG_DISCONTINUE;
		pstDemux->astStream[i].ulFlags &= ~STREAM_FLAG_END_OF_STREAM;
		pstDemux->astStream[i].llLastTimestamp = 0;
	} 
	pstDemux->lEosCount = 0;

CONTINUE:
	/* start task */
	GST_OBJECT_LOCK (pstDemux);
	CLEAR_FLAG(pstDemux->ulFlags, (TCDMX_BASE_FLAG_SEEKING));
	if(gst_task_set_state (pstDemux->pstTask, GST_TASK_STARTED) == 0)
	{
		GST_DEBUG_OBJECT (pstDemux, "Demuxing Thread occurs an Error on creation");
		GST_OBJECT_UNLOCK (pstDemux);
		res = TCDMX_FALSE;
	}
	else {
		pstDemux->bSegmentRunning = TCDMX_TRUE;
		GST_OBJECT_UNLOCK (pstDemux);
	}

	return res;
}


static
gboolean
gst_tcdmx_add_srcpads (
	GstTcDemuxBase  * pstDemux
	)
{
	gint32 i, count = pstDemux->lStreamCount;
	GstElementClass *p_element_class = GST_ELEMENT_GET_CLASS (pstDemux);
	GstElement *p_element = GST_ELEMENT_CAST (pstDemux);
	GstPadTemplate *p_templ;
	GstCaps *p_caps = NULL;
	GstPad * p_newpad = NULL;
	tcdmx_stream_t *p_stream;
	gchar *p_padname = NULL;
	gchar *p_codecname;
	GstEvent *event;
	gchar *src_stream_id;
	gdouble tcdmx_streamPlayrate;
	gchar *disable;
	gint32 nAvailablePad = 0;
	gboolean ret;

	GST_LOG_OBJECT (pstDemux, "add srcpads");
	if (disable = g_getenv("GST_TCDMX_DISABLE_STREAM")) 
	{
		if (strstr(disable, "video") != NULL) {
			g_print("[TCDEMUX] VIDEO DISABLED !!\n");
			SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_DISABLE_VIDEO);
		}
		if (strstr(disable, "audio") != NULL) {
			g_print("[TCDEMUX] AUDIO DISABLED !!\n");
			SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_DISABLE_AUDIO);
		}
		if (strstr(disable, "subtitle") != NULL) {
			g_print("[TCDEMUX] SUBTITLE DISABLED !!\n");
			SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_DISABLE_SUBTITLE);
		}
	}

	//Create taglist
	//pstDemux->pstTagList = gst_tag_list_new_empty();
	//gst_tag_list_set_scope (pstDemux->pstTagList, GST_TAG_SCOPE_GLOBAL);
	GST_DEBUG_OBJECT(pstDemux,"total stream count = %d, total audio count = %d",pstDemux->lStreamCount, pstDemux->ulTotalAudioNum );

	for(i = 0; i < count; i++) {
		pstDemux->pstTagList = gst_tag_list_new_empty();
		gst_tcdmx_update_taglist(pstDemux);
		//gst_tag_list_set_scope (pstDemux->pstTagList, GST_TAG_SCOPE_STREAM);
		p_stream = pstDemux->astStream + i;

		if( p_stream->pstPad != NULL ){
			gst_object_unref (p_stream->pstPad);
		}

		CLEAR_FLAG(p_stream->ulFlags, (STREAM_FLAG_EXPOSED));
		p_stream->pstPad = NULL;
		p_codecname = NULL;

		switch( (tcdmx_stream_type_t)p_stream->ulStreamType ) {
			case TCDMX_TYPE_VIDEO:
				if ((pstDemux->ulFlags & (guint32)TCDMX_BASE_FLAG_DISABLE_VIDEO) != 0u) {
					continue;
				}
				if(p_padname != NULL)
					free(p_padname);
				p_padname = g_strdup_printf (TEMPLATE_NAME_VIDEO_PAD, pstDemux->acTypeCount[TCDMX_TYPE_VIDEO]);
				p_templ = gst_element_class_get_pad_template (p_element_class, TEMPLATE_NAME_VIDEO_PAD);
				p_caps = gst_tc_create_video_caps (
								p_stream->unInfo.stVideo.ulFourCC, 
								&p_stream->unInfo.stVideo, 
								pstDemux->bNeed_to_attach_codec_data ? NULL : p_stream->pstCodecData, 
								(gboolean)CHECK_FLAG((pstDemux->ulFlags), (TCDMX_BASE_FLAG_RINGMODE_ENABLE)),
								&p_codecname);

				gst_tag_list_add (pstDemux->pstTagList, GST_TAG_MERGE_APPEND,
								  GST_TAG_VIDEO_CODEC, p_codecname, NULL);

				//Set stream skimming threshold
				if((p_stream->unInfo.stVideo.ulWidth != 0U) && (p_stream->unInfo.stVideo.ulHeight != 0U))
				{
					guint32 resolution = p_stream->unInfo.stVideo.ulWidth * p_stream->unInfo.stVideo.ulHeight;
					if(resolution >= (1920U * 1080U)){
						tcdmx_streamPlayrate = 1.5;
					}else if(resolution > ((1280U * 720U * 3U) / 2U)){
						tcdmx_streamPlayrate = 2.5;
					}else if(resolution > (1280U * 720U)){
						tcdmx_streamPlayrate = 3.0;
					}else if(resolution > (720U * 480U)){
						tcdmx_streamPlayrate = 4.0;
					}else if(resolution > (360U * 240U)){
						tcdmx_streamPlayrate = 5.0;
					}else{
						tcdmx_streamPlayrate = 6.0;
					}

					GST_INFO_OBJECT (pstDemux, "Stream skimming threshold is set to %f", tcdmx_streamPlayrate);
				}
				else
				{
					tcdmx_streamPlayrate = TCDMX_SKIMMING_DEFAULT_THRESHOLD;
					GST_INFO_OBJECT (pstDemux, "There's no resolution info. Stream skimming threshold is set to %f", tcdmx_streamPlayrate);
				}

				//Set global skimming threshold
				if( pstDemux->fSkimmingThreshold > tcdmx_streamPlayrate )
				{
					pstDemux->fSkimmingThreshold = tcdmx_streamPlayrate;
					GST_INFO_OBJECT (pstDemux, "Skimming threshold is set to %f", pstDemux->fSkimmingThreshold);
				}
				break;

			case TCDMX_TYPE_AUDIO:
				if (pstDemux->ulFlags & TCDMX_BASE_FLAG_DISABLE_AUDIO){
					continue;
				}
				if(p_padname != NULL)
					free(p_padname);
				p_padname = g_strdup_printf (TEMPLATE_NAME_AUDIO_PAD, pstDemux->acTypeCount[TCDMX_TYPE_AUDIO]);
				p_templ = gst_element_class_get_pad_template (p_element_class, TEMPLATE_NAME_AUDIO_PAD);
				p_caps = gst_tc_create_audio_caps (
								p_stream->unInfo.stAudio.ulFormatTag, 
								&p_stream->unInfo.stAudio, 
								p_stream->pstCodecData, 
								&p_codecname);

				gst_tag_list_add (pstDemux->pstTagList, GST_TAG_MERGE_APPEND,
								  GST_TAG_AUDIO_CODEC, p_codecname, NULL);

				if (p_stream->unInfo.stAudio.pszlanguage != NULL)
				{
					gst_tag_list_add (pstDemux->pstTagList, GST_TAG_MERGE_REPLACE,
								  GST_TAG_LANGUAGE_CODE, p_stream->unInfo.stAudio.pszlanguage, NULL);
					GST_INFO_OBJECT (pstDemux, "demux audio: %s",p_stream->unInfo.stAudio.pszlanguage);
				}

				break;

			case TCDMX_TYPE_SUBTITLE:
				if (pstDemux->ulFlags & TCDMX_BASE_FLAG_DISABLE_SUBTITLE){
					continue;
				}
				if(p_padname != NULL)
					free(p_padname);
				p_padname = g_strdup_printf (TEMPLATE_NAME_SUBTITLE_PAD, pstDemux->acTypeCount[TCDMX_TYPE_SUBTITLE]);
				p_templ = gst_element_class_get_pad_template (p_element_class, TEMPLATE_NAME_SUBTITLE_PAD);
				p_caps = gst_tc_create_subtitle_caps (
								p_stream->unInfo.stSubtitle.ulFormatId,
								p_stream->unInfo.stSubtitle.szMimeType,
								p_stream->pstCodecData, 
								&p_codecname);

				gst_tag_list_add (pstDemux->pstTagList, GST_TAG_MERGE_APPEND,
								  GST_TAG_SUBTITLE_CODEC, p_codecname, NULL);

				if (p_stream->unInfo.stSubtitle.pszlanguage != NULL)
				{
					gst_tag_list_add (pstDemux->pstTagList, GST_TAG_MERGE_REPLACE,
								  GST_TAG_LANGUAGE_CODE, p_stream->unInfo.stSubtitle.pszlanguage, NULL);
					GST_INFO_OBJECT (pstDemux, "demux subtitle: %s",p_stream->unInfo.stSubtitle.pszlanguage);
				}

				if (p_stream->pstCodecData != NULL) {
					gst_caps_set_simple (p_caps, "codec_data", GST_TYPE_BUFFER, p_stream->pstCodecData, NULL);
				}
				break;

			case TCDMX_TYPE_PRIVATE:
			default:
				if (((guint32)pstDemux->ulFlags & (guint32)TCDMX_BASE_FLAG_DISABLE_PRIVATE) != 0u){
					continue;
				}
				if(p_padname != NULL)
					free(p_padname);
				p_padname = g_strdup_printf (TEMPLATE_NAME_PRIVATE_PAD, pstDemux->acTypeCount[TCDMX_TYPE_PRIVATE]);
				p_templ = gst_element_class_get_pad_template (p_element_class, TEMPLATE_NAME_PRIVATE_PAD);
				if (p_stream->unInfo.stPrivate.szMimeType[0] != 0) {
					p_caps = gst_caps_new_simple (p_stream->unInfo.stPrivate.szMimeType, NULL);
				} else {
					p_caps = gst_caps_new_simple ("application/x-private", NULL);
				}
				break;
		}

		if (p_caps != NULL)
		{
			if (p_stream->ulFormat != 0u) {
				gst_caps_set_simple (p_caps,
									 "stream-format", G_TYPE_STRING, gst_tcdmx_get_format_string(p_stream->ulFormat),
									 NULL);
			}
			if (p_stream->ulAlignment != 0u) {
				gst_caps_set_simple (p_caps,
									 "alignment", G_TYPE_STRING, gst_tcdmx_get_alignment_string(p_stream->ulAlignment),
									 NULL);
			}
		}

		if (p_templ == NULL) {
			GST_ERROR_OBJECT (pstDemux, "pad templete %s is not exist (type: %s)", TYPE_STRING (p_stream->ulStreamType));
			continue;
		}
		if (p_caps == NULL) {
			GST_ERROR_OBJECT (pstDemux, "caps creation is failed (type: %s)", TYPE_STRING (p_stream->ulStreamType));
			continue;
		}
		if ((p_newpad = gst_pad_new_from_template (p_templ, p_padname)) == NULL) {
			GST_ERROR_OBJECT (pstDemux, "pad creation is failed");
			continue;
		}

		if (p_codecname != NULL){
			GST_INFO_OBJECT (pstDemux, "srcpad created (type: %s, codec: %s)", TYPE_STRING (p_stream->ulStreamType), p_codecname);
		} else {
			GST_INFO_OBJECT (pstDemux, "srcpad created (type: %s)", TYPE_STRING (p_stream->ulStreamType));
		}

		p_stream->pstPad = p_newpad;
		SET_FLAG(p_stream->ulFlags, STREAM_FLAG_EXPOSED);

		// pad init
		gst_pad_set_event_function (p_newpad, GST_DEBUG_FUNCPTR (gst_tcdmx_handle_src_event));
#if 0//V1.0
		gst_pad_set_query_type_function (p_newpad, GST_DEBUG_FUNCPTR (gst_tcdmx_get_src_query_types));
#else
		//i don't know what to do, there's no info about replacement of this function.(even in official reference page)
#endif
		gst_pad_set_query_function (p_newpad, GST_DEBUG_FUNCPTR (gst_tcdmx_handle_src_query));

		// pad activate and add
		if( p_stream->ulStreamType == (guint)TCDMX_TYPE_SUBTITLE )
		{
			GstEvent * e_stream_start = gst_event_new_stream_start (p_padname);
			GstStreamFlags f_stream_flags = GST_STREAM_FLAG_NONE;
			f_stream_flags |= (GstStreamFlags)GST_STREAM_FLAG_SPARSE;

			gst_event_set_stream_flags (e_stream_start, f_stream_flags);
			(void)gst_pad_push_event (p_newpad, e_stream_start);
			GST_INFO_OBJECT (pstDemux, "srcpad configured as GST_STREAM_FLAG_SPARSE");
		}
		gst_pad_use_fixed_caps(p_newpad);
		(void)gst_pad_set_active (p_newpad, TCDMX_TRUE);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		src_stream_id = gst_pad_create_stream_id_printf (p_newpad, p_element, "%03u", nAvailablePad);

		event = gst_pad_get_sticky_event (pstDemux->pstSinkPad, GST_EVENT_STREAM_START, 0);
		if (event != 0) {
			if (gst_event_parse_group_id (event, &pstDemux->ulGroupId) != 0){
				pstDemux->bHaveGroupId = TCDMX_TRUE;
			} else {
				pstDemux->bHaveGroupId = TCDMX_FALSE;
			}
			gst_event_unref (event);
		}
		else if (pstDemux->bHaveGroupId == 0) {
				pstDemux->bHaveGroupId = TCDMX_TRUE;
				pstDemux->ulGroupId = gst_util_group_id_next ();
		}
		else {
			GST_LOG_OBJECT (pstDemux, "do nothing");
		}

		event = gst_event_new_stream_start (src_stream_id);
		if (pstDemux->bHaveGroupId == TCDMX_TRUE){
			gst_event_set_group_id (event, pstDemux->ulGroupId);
		}

		(void)gst_pad_push_event (p_newpad, event);

		// [Add] Fix 20.04.01
		if (pstDemux->pstTagList != NULL){
			GstEvent *tag_event;

			tag_event = gst_event_new_tag (gst_tag_list_copy(pstDemux->pstTagList));
			if(tag_event != NULL)
				(void)gst_pad_push_event (p_newpad, tag_event);

			gst_tag_list_unref(pstDemux->pstTagList);
			pstDemux->pstTagList = NULL;
		}

		g_free (src_stream_id);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		(void)gst_pad_set_caps(p_newpad, p_caps);
		gst_pad_set_element_private (p_newpad, p_stream);

		(void)gst_element_add_pad (p_element, p_newpad);
		gst_caps_unref (p_caps);
		if(p_codecname) {
			g_free (p_codecname);
		}
		nAvailablePad++;
		pstDemux->acTypeCount[p_stream->ulStreamType]++;
	}
	if(p_padname != NULL)
		g_free (p_padname);
	if(nAvailablePad != 0){
		ret = TCDMX_TRUE;
	} else {
		ret = TCDMX_FALSE;
	}
	return ret;
}


static
gboolean
gst_tcdmx_remove_srcpads (
	GstTcDemuxBase  * pstDemux
	)
{
	gboolean res = TCDMX_FALSE;
	gint32 i, count = pstDemux->lStreamCount;
	GstPad *p_pad;

	GST_LOG_OBJECT (pstDemux, "remove srcpads");

	for(i = 0; i < count; i++) {
		p_pad = pstDemux->astStream[i].pstPad;
		if (p_pad != NULL) {
			(void)gst_pad_set_active (p_pad, TCDMX_FALSE);
			(void)gst_element_remove_pad (GST_ELEMENT_CAST (pstDemux), p_pad);
		}

		if (pstDemux->astStream[i].pstCodecData != NULL){
			gst_buffer_unref(pstDemux->astStream[i].pstCodecData);
		}
		(void)memset (pstDemux->astStream+i, 0, sizeof(tcdmx_stream_t) );

		res = TCDMX_TRUE;
	}

	(void)memset (pstDemux->acTypeCount, 0, sizeof(pstDemux->acTypeCount));
	(void)memset (pstDemux->acIndexMap, 0, sizeof(pstDemux->acIndexMap));
	pstDemux->lStreamCount = 0;

	return res;
}


static 
gboolean
gst_tcdmx_push_event (
	GstTcDemuxBase  * pstDemux, 
	GstEvent        * pstEvent,
	gint32            lIndex
	)
{
	gboolean res = TCDMX_TRUE;
	gboolean push_result;
	gint32 i, count = pstDemux->lStreamCount;

	g_return_val_if_fail(lIndex < count, TCDMX_FALSE);

	if( lIndex >= 0 )
	{
		if( (push_result = gst_pad_push_event (pstDemux->astStream[lIndex].pstPad, gst_event_ref(pstEvent))) == TCDMX_FALSE ) {
			res = TCDMX_FALSE;
		}
		GST_DEBUG_OBJECT (pstDemux, "sending %s event to stream (index: %d)%s", GST_EVENT_TYPE_NAME (pstEvent), lIndex, push_result ? "" : " - FAILED");
	}
	else
	{
		for(i = 0; i < count; i++)
		{
			if( (push_result = gst_pad_push_event (pstDemux->astStream[i].pstPad, gst_event_ref(pstEvent))) == TCDMX_FALSE ){
				res = TCDMX_FALSE;
			}
			GST_DEBUG_OBJECT (pstDemux, "sending %s event to stream (index: %d)%s", GST_EVENT_TYPE_NAME (pstEvent), i, push_result ? "" : " - FAILED");
		}
	}

	gst_event_unref (pstEvent);

	return res;
}

static 
void
gst_tcdmx_update_taglist(
	GstTcDemuxBase * pstDemux
	)
{
	if(pstDemux != 0)
	{
		gchar * ppContainerName;
		GstTcDemuxBaseClass *p_dmx_class = GST_DMXBASE_GET_CLASS(pstDemux);

		g_mutex_lock(pstDemux->pstDemuxLock);
		p_dmx_class->vmGetParam(pstDemux, TCDEMUX_PARAM_QUERY_CONTAINER_NAME, &ppContainerName);
		g_mutex_unlock(pstDemux->pstDemuxLock);

		gst_tag_list_add (pstDemux->pstTagList, GST_TAG_MERGE_REPLACE,
						  GST_TAG_DURATION, pstDemux->stSegment.duration, NULL);
		if(ppContainerName != NULL)
		{
			gst_tag_list_add (pstDemux->pstTagList, GST_TAG_MERGE_REPLACE,
							  GST_TAG_CONTAINER_FORMAT, ppContainerName, NULL);
			g_free(ppContainerName);
		}
	}
}

static
void
gst_tcdmx_push_taglist (
	GstTcDemuxBase * pstDemux
	)
{
	gst_tcdmx_update_taglist(pstDemux);

	(void)gst_tcdmx_push_event (pstDemux, gst_event_new_tag(pstDemux->pstTagList), -1);
	pstDemux->pstTagList = NULL;
}

static
const gchar *
gst_tcdmx_get_format_string(guint32 ulFormat)
{
#if 1
	const gchar *str_format;
	switch (ulFormat) {
	case TCDMX_STREAM_FORMAT_BYTE_STREAM:
		str_format = "byte-stream";
		break;
	case TCDMX_STREAM_FORMAT_ADTS:
		str_format = "adts";
		break;
	case TCDMX_STREAM_FORMAT_LATM:
		str_format = "latm";
		break;
	case TCDMX_STREAM_FORMAT_LOAS:
		str_format = "loas";
		break;
	case TCDMX_STREAM_FORMAT_ADIF:
		str_format = "adif";
		break;
	case TCDMX_STREAM_FORMAT_AVC:
		str_format = "avc";
		break;
	default:
		str_format = NULL;
		break;
	}
	return str_format;
#else
	switch (ulFormat) {
	case TCDMX_STREAM_FORMAT_BYTE_STREAM:
		return "byte-stream";
	case TCDMX_STREAM_FORMAT_ADTS:
		return "adts";
	case TCDMX_STREAM_FORMAT_LATM:
		return "latm";
	case TCDMX_STREAM_FORMAT_LOAS:
		return "loas";
	case TCDMX_STREAM_FORMAT_ADIF:
		return "adif";
	case TCDMX_STREAM_FORMAT_AVC:
		return "avc";
	default:
		return NULL;
	}
#endif
}

static
const
gchar *
gst_tcdmx_get_alignment_string(guint32 ulAlignment)
{
#if 1
	const gchar *str_alignment = NULL;
	switch (ulAlignment) {
	case TCDMX_STREAM_ALIGNMENT_NONE:
		str_alignment = "none";
		break;
	case TCDMX_STREAM_ALIGNMENT_FRAME:
		str_alignment = "frame";
		break;
	case TCDMX_STREAM_ALIGNMENT_AU:
		str_alignment = "au";
		break;
	case TCDMX_STREAM_ALIGNMENT_NAL:
		str_alignment = "nal";
		break;
	default:
		str_alignment = NULL;
		break;
	}
	return str_alignment;
#else
	switch (ulAlignment) {
	case TCDMX_STREAM_ALIGNMENT_NONE:
		return "none";
	case TCDMX_STREAM_ALIGNMENT_FRAME:
		return "frame";
	case TCDMX_STREAM_ALIGNMENT_AU:
		return "au";
	case TCDMX_STREAM_ALIGNMENT_NAL:
		return "nal";
	default:
		return NULL;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	child class
//
void
gst_tcdmx_set_class_details (
	GstTcDemuxBaseClass  * pstDemuxClass,
	const gchar                * pszDemuxLongName,
	const gchar                * pszDemuxDesc
	)
{
	GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);

	GST_TRACE ("");

#if 0
	gst_element_class_set_details_simple
#else
	gst_element_class_set_metadata
#endif
		(p_element_class
		, pszDemuxLongName														//longname
		, "Codec/Demuxer"														//classification
		, pszDemuxDesc															//description
		, "Telechips Audio/Video Algorithm Group <AValgorithm@telechips.com>"	//author
		);
}


gboolean
gst_tcdmx_create_sinkpad_templates (
	GstTcDemuxBaseClass         * pstDemuxClass,
	const GstStaticPadTemplate  * pstPadTemplate
	)
{
	GstElementClass *p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);

	GST_TRACE ("");

	// set sink pad template
#if 0//V1.0
	// sink pad
	pstDemuxClass->stSinkFactory = *pstPadTemplate;
	p_templ = gst_static_pad_template_get (pstPadTemplate);
	g_return_val_if_fail (p_templ, TCDMX_FALSE);
	gst_element_class_add_pad_template (p_element_class, p_templ);
#else
	gst_element_class_add_pad_template (p_element_class, gst_static_pad_template_get (pstPadTemplate));
#endif

	return TCDMX_TRUE;
}


gboolean
gst_tcdmx_create_srcpad_templates (
	GstTcDemuxBaseClass  * pstDemuxClass,
	GstCaps              * pstVideoCaps,
	GstCaps              * pstAudioCaps,
	GstCaps              * pstSubtitleCaps,
	GstCaps              * pstPrivateCaps
	)
{
	GstElementClass * p_element_class = GST_ELEMENT_CLASS (pstDemuxClass);
	GstPadTemplate * p_templ;

	GST_TRACE ("");

	// src pad: video caps and pad template
	if( pstVideoCaps != NULL ) {
		p_templ = gst_pad_template_new (TEMPLATE_NAME_VIDEO_PAD, GST_PAD_SRC, GST_PAD_SOMETIMES, pstVideoCaps);
		g_return_val_if_fail (p_templ, TCDMX_FALSE);
		GST_TRACE("%p, %p", p_element_class, p_templ);
		gst_element_class_add_pad_template (p_element_class, p_templ);
	}

	// src pad: audio caps and pad template
	if( pstAudioCaps != NULL ) {
		p_templ = gst_pad_template_new (TEMPLATE_NAME_AUDIO_PAD, GST_PAD_SRC, GST_PAD_SOMETIMES, pstAudioCaps);
		g_return_val_if_fail (p_templ, TCDMX_FALSE);
		gst_element_class_add_pad_template (p_element_class, p_templ);
	}

	// src pad: subtitle caps and pad template
	if( pstSubtitleCaps != NULL ) {
		p_templ = gst_pad_template_new (TEMPLATE_NAME_SUBTITLE_PAD, GST_PAD_SRC, GST_PAD_SOMETIMES, pstSubtitleCaps);
		g_return_val_if_fail (p_templ, TCDMX_FALSE);
		gst_element_class_add_pad_template (p_element_class, p_templ);
	}

	// src pad: private caps and pad template
	if( pstPrivateCaps != NULL) {
		p_templ = gst_pad_template_new (TEMPLATE_NAME_PRIVATE_PAD, GST_PAD_SRC, GST_PAD_SOMETIMES, pstPrivateCaps);
		g_return_val_if_fail (p_templ, TCDMX_FALSE);
		gst_element_class_add_pad_template (p_element_class, p_templ);
	}

	return TCDMX_TRUE;
}

gboolean
gst_tcdmx_add_sinkpad (
	GstTcDemuxBase              * pstDemux,
	const GstStaticPadTemplate  * pstPadTemplate
	)
{
	pstDemux->pstSinkPad = gst_pad_new_from_static_template (pstPadTemplate, "sink");

	gst_pad_set_activate_function (pstDemux->pstSinkPad, GST_DEBUG_FUNCPTR (gst_tcdmx_sink_activate));
	gst_pad_set_activatemode_function (pstDemux->pstSinkPad, GST_DEBUG_FUNCPTR (gst_tcdmx_sink_activate_mode));
	gst_pad_set_event_function (pstDemux->pstSinkPad, GST_DEBUG_FUNCPTR (gst_tcdmx_handle_sink_event));
#if SUPPORT_PUSH_MODE
	gst_pad_set_chain_function (pstDemux->pstSinkPad, GST_DEBUG_FUNCPTR (gst_tcdmx_chain));
#endif
	return gst_element_add_pad (GST_ELEMENT_CAST (pstDemux), pstDemux->pstSinkPad);
}


void
gst_tcdmx_set_default_demux_mode (
	GstTcDemuxBase  * pstDemux,
	guint32           ulMode
	)
{
	GST_TRACE ("");

	if ((ulMode & TCDMX_MODE_SEQUENTIAL) > 0u) {
		CLEAR_FLAG (pstDemux->ulFlags, (TCDMX_BASE_FLAG_SELECTIVE_DEMUXING));
		SET_FLAG(pstDemux->ulFlags, TCDMX_BASE_FLAG_SEQUENTIAL_DEMUXING);
	}
	else if ((ulMode & TCDMX_MODE_SELECTIVE) > 0u) {
		CLEAR_FLAG(pstDemux->ulFlags, (TCDMX_BASE_FLAG_SEQUENTIAL_DEMUXING));
		SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_SELECTIVE_DEMUXING);
	}
	else {
		GST_LOG("do nothing");
	}

	if ((ulMode & TCDMX_MODE_FIXED) > 0u) {
		SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_FIXED_DEMUXING_MODE);
	}
}

void
gst_tcdmx_set_ringmode (
	GstTcDemuxBase  * pstDemux,
	gboolean 		  bEnable
	)
{
	GST_TRACE ("");
	if( bEnable == TCDMX_TRUE ) {
		SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_RINGMODE_ENABLE);
	}
	else {
		CLEAR_FLAG (pstDemux->ulFlags, (TCDMX_BASE_FLAG_RINGMODE_ENABLE));
	}
}

void
gst_tcdmx_set_seekable (
	GstTcDemuxBase  * pstDemux,
	gboolean          bSeekable
	)
{
	GST_TRACE ("");

	if (bSeekable == TCDMX_TRUE){
		SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_SEEK_ENABLE);
	} else {
		CLEAR_FLAG (pstDemux->ulFlags, (TCDMX_BASE_FLAG_SEEK_ENABLE));
	}
}


void
gst_tcdmx_set_timeseek (
	GstTcDemuxBase  * pstDemux,
	gboolean          bTimeSeek
	)
{
	if (bTimeSeek == TCDMX_TRUE){
		SET_FLAG (pstDemux->ulFlags, TCDMX_BASE_FLAG_TIMESEEK_ENABLE);
	} else {
		CLEAR_FLAG (pstDemux->ulFlags, (TCDMX_BASE_FLAG_TIMESEEK_ENABLE));
	}
}


gboolean
gst_tcdmx_register_stream_info (
	GstTcDemuxBase  * pstDemux,
	guint32           ulStreamID,
	guint32           ulStreamType,
	gpointer          pStreamInfo,
	gpointer          pCodecData,
	gint32            lCodecDataLen
	)
{

	if( pStreamInfo != NULL ) {
		if( ulStreamType == (guint32)TCDMX_TYPE_VIDEO ) {
			tc_video_info_t *p_info = (tc_video_info_t *)pStreamInfo;
			switch ((gint32)(p_info->ulFourCC)) {
			case FOURCC_avc1: 
			case FOURCC_AVC1:
			case FOURCC_h264: 
			case FOURCC_H264:
			case FOURCC_x264: 
			case FOURCC_X264:
			case FOURCC_vssh: 
			case FOURCC_VSSH:
			case FOURCC_z264: 
			case FOURCC_Z264:
			case FOURCC_mvc:  
			case FOURCC_MVC:
			case FOURCC_h265: 
			case FOURCC_H265: 
			case FOURCC_hvc1:
			case FOURCC_HVC1: 
			case FOURCC_hevc:
			case FOURCC_HEVC:
			case FOURCC_HM10:
				if ((pCodecData == NULL) || (lCodecDataLen < 4) || (GST_READ_UINT32_BE(pCodecData) == 1u)) {
					pstDemux->bNeed_to_attach_codec_data = TCDMX_TRUE;
					return gst_tcdmx_register_stream_info2(pstDemux, ulStreamID, ulStreamType, pStreamInfo, 
														   pCodecData, lCodecDataLen, 
														   TCDMX_STREAM_FORMAT_BYTE_STREAM, TCDMX_STREAM_ALIGNMENT_AU);
				}
			}
		}
	}

	return gst_tcdmx_register_stream_info2(pstDemux, ulStreamID, ulStreamType, pStreamInfo, pCodecData, lCodecDataLen, 0, 0);
}

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
	)
{
	gint32 tcdmx_stream_idx = pstDemux->lStreamCount;
	tcdmx_stream_t *p_stream = pstDemux->astStream+tcdmx_stream_idx;
	GstBuffer *p_codec_data;
	gboolean ret;

	GST_TRACE ("");

	if ( tcdmx_stream_idx >= (gint32)STREAM_MAX ) {
		GST_ERROR_OBJECT (pstDemux, "stream count have exceeded max");
		ret = TCDMX_FALSE;
	} else {

	// set pad to element
	p_stream->ulStreamID      = ulStreamID;
	p_stream->ulStreamType    = ulStreamType;
	p_stream->ulFlags         = STREAM_FLAG_DISCONTINUE;
	p_stream->pstPad          = NULL;
	p_stream->pPrivate        = NULL;
	p_stream->enLastFlow      = GST_FLOW_OK;
	p_stream->llLastTimestamp = 0;
	p_stream->ulFormat        = ulFormat;
	p_stream->ulAlignment     = ulAlignment;

	if( pStreamInfo != NULL ) {
		switch ((tcdmx_stream_type_t)ulStreamType) {
		case TCDMX_TYPE_VIDEO:
			(void)memcpy (&p_stream->unInfo.stVideo, pStreamInfo, sizeof(tc_video_info_t));
			break;
		case TCDMX_TYPE_AUDIO:
			(void)memcpy (&p_stream->unInfo.stAudio, pStreamInfo, sizeof(tc_audio_info_t));
			break;
		case TCDMX_TYPE_SUBTITLE:
			(void)memcpy (&p_stream->unInfo.stSubtitle, pStreamInfo, sizeof(tc_subtitle_info_t));
			break;
		case TCDMX_TYPE_PRIVATE:
		default:
			(void)memcpy (&p_stream->unInfo.stPrivate, pStreamInfo, sizeof(tc_private_info_t));
			break;
		}
	}

	if( (pCodecData != NULL) && (lCodecDataLen > 0) ) {
#if 0//V1.0
		p_codec_data = gst_buffer_new_and_alloc(lCodecDataLen);
		if( p_codec_data ) {
			memcpy( GST_BUFFER_DATA(p_codec_data), pCodecData, lCodecDataLen);
			GST_BUFFER_SIZE (p_codec_data) = lCodecDataLen;
		}
#else
		p_codec_data = gst_buffer_new_allocate(NULL, (gsize)lCodecDataLen, NULL);
		if( p_codec_data != NULL) {
			(void)gst_buffer_fill ( p_codec_data, 0, (gconstpointer)pCodecData, (gsize)lCodecDataLen);
			gst_buffer_set_size( p_codec_data, lCodecDataLen);
		}
#endif
		p_stream->pstCodecData = p_codec_data;	//TODO
	}

	pstDemux->acIndexMap[pstDemux->lStreamCount] = (gint)ulStreamID;
	pstDemux->lStreamCount++;

	ret = TCDMX_TRUE;
	}
	return ret;
}

void gst_tcdmx_search_vorbis_header(guchar *pData, guint32 size, vorbis_header_info_t *pHeader)
{
	gint exlen, run;
	guchar *p;
	exlen = (gint)size;
	run = 0;
	p = pData;

	while (run < exlen) {
		if (((p[0]==(guchar)1) || (p[0]==(guchar)3) || (p[0]==(guchar)5)) &&
			(p[1] == (guchar)'v') && (p[2] == (guchar)'o') && (p[3] == (guchar)'r') && (p[4] == (guchar)'b') && (p[5] == (guchar)'i') && (p[6] == (guchar)'s')) {
			break;
		}
		run++;	p++;
	}
	pHeader->pData1 = p;

	run = 7;
	p += run;
	exlen -= run;
	while (run < exlen) {
		if (((p[0]==(guchar)1) || (p[0]==(guchar)3) || (p[0]==(guchar)5)) &&
			(p[1] == (guchar)'v') && (p[2] == (guchar)'o') && (p[3] == (guchar)'r') && (p[4] == (guchar)'b') && (p[5] == (guchar)'i') && (p[6] == (guchar)'s')) {
			break;
		}
		run++;	p++;
	}
	pHeader->size1 = run;
	pHeader->pData2 = p;

	run = 7;
	p += run;
	exlen -= run;
	while (run < exlen) {
		if (((p[0]==(guchar)1) || (p[0]==(guchar)3) || (p[0]==(guchar)5)) &&
			(p[1] == (guchar)'v') && (p[2] == (guchar)'o') && (p[3] == (guchar)'r') && (p[4] == (guchar)'b') && (p[5] == (guchar)'i') && (p[6] == (guchar)'s')) {
			break;
		}
		run++;	p++;
	}
	pHeader->size2 = (gint)run;
	pHeader->pData3 = p;

	exlen -= run;
	pHeader->size3 = (gint)exlen;
}

void
gst_tcdmx_set_total_audiostream (
	GstTcDemuxBase  * pstDemux,
	guint32           ulStreamNum
)
{
	pstDemux->ulTotalAudioNum = ulStreamNum;
}
