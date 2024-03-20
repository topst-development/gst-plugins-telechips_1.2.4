/****************************************************************************
 *   FileName    : ogg_demuxer.h
 *   Description : ogg parser project main api
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
/*!
 ***********************************************************************
 \par Copyright
 \verbatim
  ________  _____           _____   _____           ____  ____   ____		
     /     /       /       /       /       /     /   /    /   \ /			
    /     /___    /       /___    /       /____ /   /    /____/ \___			
   /     /       /       /       /       /     /   /    /           \		
  /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
   																				
  Copyright (c) 2008 Telechips Inc.
  Korad Bldg, 1000-12 Daechi-dong, Kangnam-Ku, Seoul, Korea					
 \endverbatim
 ***********************************************************************
 */
/*!
 ***********************************************************************
 *
 * \file
 *		ogg_demuxer.h
 * \date
 *		2009/07/21
 * \author
 *		Brian Kim
 * \brief
 *		ogg parser project main api
 * \version
  *		2.0.0 : 2010/03/25
 *		1.1.0 : 2010/03/01
 *		1.0.0 : 2009/07/21
 *
 ***********************************************************************
 */
#ifndef OGG_DEMUXER_H_
#define OGG_DEMUXER_H_

#include "../av_common_dmx.h"
#define OGG_EXTRA_SIZE	(1024*360)
#define OGG_PACKET_SIZE	(1024*1024*1)

typedef av_memory_func_t		ogg_dmx_memory_func_t;		//!< callback functions of TCC_OGG_DMX()
typedef av_file_func_t			ogg_dmx_file_func_t;		//!< callback functions of TCC_OGG_DMX()
typedef av_handle_t				ogg_dmx_handle_t;			//!< handle of TCC_OGG_DMX()
typedef av_result_t				ogg_dmx_result_t;			//!< return value of TCC_OGG_DMX()

typedef struct ogg_dmx_file_info_t
{
	/* common */
	av_sint8_t* m_pszOpenFileName;	//!< open file name
	av_sint8_t* m_pszCopyright;		//!< copyright
	av_sint8_t* m_pszCreationTime;	//!< creation time
	av_sint32_t m_iRunningtime;			//!< runing time * 1000
	U64 m_lFileSize;			//!< total file size
		
	av_uint32_t m_uiTotalBitrate;
	av_uint32_t m_uiSeekalble;
	av_sint32_t m_Reserved[32-8];
} ogg_dmx_file_info_t;

typedef struct ogg_dmx_video_info_t
{
	/* common */
	av_sint32_t m_iWidth;				//!< width
	av_sint32_t m_iHeight;				//!< height
	av_sint32_t m_iFrameRate;			//!< framerate * 1000;
	av_sint32_t m_iFourCC;				//!< fourcc

	/* extra info (common) */
	av_sint8_t* m_pszCodecName;		//!< codec name
	av_sint8_t* m_pszCodecVendorName;	//!< codec vendor
	av_uint8_t* m_pExtraData;//!< extra data
	av_sint32_t m_iExtraDataLen;		//!< extra data length
	
	av_uint32_t m_uiBitrate;
	av_sint32_t m_iStreamID;			//!< stream id
	av_sint32_t m_Reserved[32-10];			
} ogg_dmx_video_info_t;

typedef struct ogg_dmx_audio_info_t
{
	/* common */
	av_sint32_t m_iTotalNumber;			//!< total audio number
	av_sint32_t m_iSamplePerSec;		//!< samples/sec
	av_sint32_t m_iBitsPerSample;		//!< bits/sample
	av_sint32_t m_iChannels;			//!< channels
	av_sint32_t m_iFormatId;			//!< format id

	/* extra info (common) */
	av_sint8_t* m_pszCodecName;		//!< codec name
	av_sint8_t* m_pszCodecVendorName;	//!< codec vendor
	av_uint8_t* m_pExtraData;//!< extra data
	av_sint32_t m_iExtraDataLen;		//!< extra data length

	av_uint32_t m_uiBitrate;		
	av_uint32_t m_uiMaxBitrate;		
	av_uint32_t m_uiMinBitrate;		
	av_uint32_t m_iBitrateMode;		

	av_sint32_t m_iStreamID;			//!< stream id
	
	av_sint32_t m_Reserved[32-14];			
} ogg_dmx_audio_info_t;

//! Structure for initialization parameter of TCC_OGG_DMX()
typedef struct ogg_dmx_init_t 
{
	ogg_dmx_memory_func_t m_sCallbackFuncMem;	//!< [in] callback functions of ogg dmx
	ogg_dmx_file_func_t	  m_sCallbackFuncFile;	//!< [in] callback functions of ogg dmx
	void* m_pOpenFileName;						//!< [in] open file name( when Op is INIT ) or handle of opened file( when Op is INIT_FROM_FILE_HANDLE )
	av_uint32_t	m_uiOpenOption;
	av_uint32_t	m_uiFileCacheSize;
	av_uint32_t	m_uiSeekEndOption;			//!< 0: play current frame, 1: End of Stream, 2: last key frame
	av_sint32_t	m_Reserved[8-5];

	//! extra info: 96 Bytes
	union 
	{
		av_sint32_t m_ReservedExtra[24];		
	} m_uExtra;

} ogg_dmx_init_t;

typedef struct ogg_dmx_info_t
{
	//! file information : 128 Bytes
	ogg_dmx_file_info_t m_sFileInfo;

	//! default video information : 128 Bytes
	ogg_dmx_video_info_t m_sVideoInfo;

	//! default audio information : 128 Bytes
 	ogg_dmx_audio_info_t m_sAudioInfo;
	// video stream info
	av_uint32_t			 m_ulVideoStreamTotal;		//!< number of video stream
	ogg_dmx_video_info_t    *m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )

	// audio stream info
	av_uint32_t			 m_ulAudioStreamTotal;		//!< number of audio stream
	ogg_dmx_audio_info_t    *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
} ogg_dmx_info_t;

//! Select stream - input
typedef struct ogg_dmx_sel_stream_t
{
	av_sint32_t			 m_lSelType;		//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	av_sint32_t			 m_lVideoID;		//!< [in] Video ID
	av_sint32_t			 m_lAudioID;		//!< [in] Audio ID
} ogg_dmx_sel_stream_t;

//! Select stream - output
typedef struct ogg_dmx_sel_info_t
{
	ogg_dmx_video_info_t	*m_pstVideoInfo;	//!< pointer to video information
	ogg_dmx_audio_info_t	*m_pstAudioInfo;	//!< pointer to audio information
} ogg_dmx_sel_info_t;

//typedef common_dmx_input_t ogg_dmx_input_t;		//! input parameter of TCC_OGG_DMX()
//typedef common_dmx_output_t ogg_dmx_output_t;		//! output parameter of TCC_OGG_DMX()
//typedef common_dmx_seek_t ogg_dmx_seek_t;			//! seek parameter of TCC_OGG_DMX()

typedef struct ogg_dmx_input_t
{
	av_uint8_t  *m_pbyStreamBuff;		//!< [in] the pointer to getstream buffer
	av_uint32_t	m_ulStreamBuffSize;		//!< [in] the size of getstream buffer
	av_uint32_t	m_ulStreamType;			//!< [in] the type of getstream
	av_uint32_t	m_iStreamIdx;			//!< [in] the index of getstream
} ogg_dmx_input_t;

typedef struct ogg_dmx_output_t
{
	av_uint8_t  *m_pbyStreamData;		//!< [out] the pointer to outstream data
	av_uint32_t	m_ulStreamDataSize;		//!< [out] the size to outstream data
	av_uint32_t	m_ulStreamType;			//!< [out] the type of outstream
	av_uint32_t	m_ulTimeStamp;			//!< [out] the timestamp of outstream
	av_uint32_t	m_ulEndTimeStamp;		//!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
	av_uint32_t	m_iStreamIdx;			//!< [out] the index of output stream
} ogg_dmx_output_t;

typedef av_dmx_seek_t			ogg_dmx_seek_t;		//! seek parameter of TCC_OGG_DMX()

#define OGG_SEEKABLE_NO				(0x0000)		// Sequential Read (default)
#define OGG_SEEKABLE_YES			(0x0001)		// Video/Audio Selective Read

#define OGG_OPENOPT_SEQUENTIAL	(0x0001)		// Sequential Read (Not Use)
#define OGG_OPENOPT_SELECTIVE		(0x0002)		// Video/Audio Selective Read
#define OGG_OPENOPT_MULTIDEMUX	(0x0004)

#define OGG_SEEKENDMODE_PLAY_CURRENTFRAME	(0x0)
#define OGG_SEEKENDMODE_ENDOFSTREAM				(0x1)
#define OGG_SEEKENDMODE_LASTKEYFRAME			(0x2)

#define OGG_SEEKMODE_RELATIVE_TIME	(0x0)
#define OGG_SEEKMODE_ABSOLUTE_TIME	(0x1)
#define OGG_SEEKMODE_DIR_FORWARD		(0x0)
#define OGG_SEEKMODE_DIR_BACKWARD		(0x2)

#define OGG_PACKETTYPE_ANY			(0x0)
#define OGG_PACKETTYPE_VIDEO		(0x1)
#define OGG_PACKETTYPE_AUDIO		(0x2)

#define OGG_DMX_SUCCESS						(0)
#define OGG_DMX_NOMORE_PAGE				(-1)
#define OGG_DMX_STREAM_READ_ERROR	(-2)
#define OGG_DMX_ERROR							(-4)
#define OGG_DMX_FILE_OPEN_ERROR		(-5)
#define OGG_DMX_NULL_INSTANCE			(-6)
#define OGG_DMX_NOT_FOUND_SYNCH		(-7)
#define OGG_DMX_SMALL_PACKET_SIZE	(-8)
#define OGG_DMX_INVALID_PACKET_TYPE (-9)
#define OGG_DMX_NOT_SUPPORT				(-11)
#define OGG_DMX_SEEK_OUTOFRANGE		(-12)

#define OGG_DMX_USE_EXTRADATA_DECODETIME

/*!
 ***********************************************************************
 * \brief
 *		TCC_OGG_DMX		: main api function of OGG demuxer
 * \param
 *		[in]Op			: demuxer operation 
 * \param
 *		[in,out]pHandle	: handle of OGG demuxer
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_OGG_DMX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
ogg_dmx_result_t 
TCC_OGG_DMX( av_uint32_t Op, ogg_dmx_handle_t* pHandle, void* pParam1, void* pParam2 );

#endif//OGG_DEMUXER_H_
