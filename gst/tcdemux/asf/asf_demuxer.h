/*!
 ***********************************************************************
 \par Copyright
 \verbatim
  ________  _____           _____   _____           ____  ____   ____		
     /     /       /       /       /       /     /   /    /   \ /			
    /     /___    /       /___    /       /____ /   /    /____/ \___			
   /     /       /       /       /       /     /   /    /           \		
  /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
   																				
  Copyright (c) 2009 Telechips Inc.
  Korad Bldg, 1000-12 Daechi-dong, Kangnam-Ku, Seoul, Korea					
 \endverbatim
 ***********************************************************************
 */
/*!
 ***********************************************************************
 *
 * \file
 *		asf_demuxer.h
 * \date
 *		2017/06/22
 *		2011/04/01
 *		2011/03/22
 *		2010/12/22
 *		2009/09/14
 *		2009/07/08
 * \author
 *		Seock-Hoon Woo(AValgorithm@telechips.com) 
 *      Yun-Hee Kim(AValgorithm@telechips.com) 
 * \brief
 *		asf parser project main api
 * \version
 *		03.000 : 2017/06/22
 *		02.003 : 2011/04/04
 *		02.002 : 2011/03/22
 *		02.001 : 2010/12/22
 *		1.3 : 2009/09/14
 *		1.2 : 2009/07/08
 *
 ***********************************************************************
 */
#ifndef ASF_DEMUXER_H_
#define ASF_DEMUXER_H_

#include "../av_common_dmx.h"

#define THIS_DRM_NO				(0x00000000)
#define THIS_DRM_PD				(0x00000001)
#define THIS_DRM_JANUS_V1		(0x00000002)
#define THIS_DRM_JANUS_V2		(0x00000004)

#define ASF_SELECTIVE_MODE		(1)
#define ASF_SEQUENTIAL_MODE		(2)

//Stream Selection related
#define ASF_MAX_STREAM_NUMBER	(128)
#define ASF_STREAM_INDEX_(n)	(1 << (n))


/*!
 ============================================================
 *
 *	File/Video/Audio/Subtitle Information
 *		- information about video, audio or subtitle into one file.
 *
 ============================================================
*/

typedef struct asf_dmx_drm_info_t
{
#define	MAX_URL_SIZE	(0x100)
	av_uint32_t			m_ulTypeOfDRM;				//!< type of DRM "THIS_DRM_NO/THIS_DRM_PD/THIS_DRM_JANUS_V1/THIS_DRM_JANUS_V2"	// parser ХыЧе
	av_uint8_t			*m_pbyEncryptSecretData;	//!< JADRM V1
	av_uint32_t			m_ulEncryptSecretDataSize;	//!< JADRM V1
	av_uint8_t			*m_pbyEncryptKeyID;			//!< JADRM V1
	av_uint32_t			m_ulEncryptKeyIDSize;		//!< JADRM V1
	av_uint8_t			m_abyURL[MAX_URL_SIZE];		//!< JADRM V1
	av_uint32_t			m_ulURLSize;				//!< JADRM V1
	av_uint8_t			*m_pbyExEncryptData;		//!< JADRM V2
	av_uint32_t			m_ulExEncryptDataSize;		//!< JADRM V2
	av_uint32_t			m_Reserved[23];
} asf_dmx_drm_info_t;

typedef struct asf_dmx_meta_info_t
{
	av_uint32_t			m_ulTitleLength;			//!< title length
	av_uint8_t			*m_pbyTitle;				//!< title
	av_uint32_t			m_ulAuthorLength;			//!< author length
	av_uint8_t			*m_pbyAuthor;				//!< author
	av_uint32_t			m_ulAlbumLength;			//!< album length
	av_uint8_t			*m_pbyAlbum;				//!< album
	av_uint32_t			m_ulGenreLength;			//!< genre length
	av_uint8_t			*m_pbyGenre;				//!< genre
	av_uint32_t			m_Reserved[24];
} asf_dmx_meta_info_t;

//! file information
typedef struct asf_dmx_file_info_t
{
	// common info
	av_sint8_t				*m_pszFileName;
	av_uint32_t				m_lDuration;
	S64						m_llFileSize;

	// asf specific info.
	av_sint32_t				m_lTypeOfDRM;				//!< type of DRM "THIS_DRM_NO/THIS_DRM_PD/THIS_DRM_JANUS_V1/THIS_DRM_JANUS_V2"
	av_sint32_t				m_lSeekable;				//!< flag of file seekable
	av_sint32_t				m_lLastKeyFramePTS;			//!< last key frame PTS in index list.
	av_sint32_t				m_lHasRealLastPTS;			//!< real last frame PTS.
	av_sint32_t				m_lHasPayloadAspectRatio;	//!< payload extension pixel aspect ratio
	av_sint32_t				m_lAspectRatioX;			//!< video pixel aspect ratio X
	av_sint32_t				m_lAspectRatioY;			//!< video pixel aspect ratio Y
	av_sint32_t				m_lHasIndex;				//!< index info.
	av_sint32_t				m_lMaxBitrate;				//!< stream bitrate.
	av_sint32_t				m_lAudioSampleIDLength;		//!< Audio sampleID length for PlayReady
	av_sint32_t				m_lVideoSampleIDLength;		//!< Video sampleID length for PlayReady
	asf_dmx_drm_info_t		m_stAsfDrmInfo;			//!< asf DRM Info.
	asf_dmx_meta_info_t		m_stAsfMetaInfo;		//!< asf Meta Info.
	av_uint32_t				m_Reserved[15];
} asf_dmx_file_info_t;


//! video stream information
typedef struct asf_dmx_video_info_t
{
	// common info
	av_sint32_t			m_lStreamID;
	av_sint32_t			m_lWidth;
	av_sint32_t			m_lHeight;
	av_sint32_t			m_lFrameRate;
	av_uint32_t			m_ulFourCC;
	void				*m_pExtraData;
	av_sint32_t			m_lExtraLength;

	// asf specific info.
	av_sint32_t			m_lDuration;
	av_sint32_t			m_lLastKeyFrameTimeStamp;
	av_sint32_t			m_lAvgBitRate;
	av_uint32_t			m_Reserved[22];
} asf_dmx_video_info_t;


//! audio stream information
typedef struct asf_dmx_audio_info_t
{	
	// common info
	av_sint32_t				m_lStreamID;
	av_sint32_t				m_lSamplePerSec;
	av_sint32_t				m_lBitsPerSample;
	av_sint32_t				m_lChannels;
	av_sint32_t				m_lFormatTag;
	void					*m_pExtraData;
	av_sint32_t				m_lExtraLength;
	av_sint8_t				*m_pszLanguage;

	// asf specific info.
	av_sint32_t				m_lAvgBytesPerSec;	//!< the averate number of bytes per second of the Audio stream
	av_sint32_t				m_lBlockAlign;		//!< block size in bytes of the Audio stream
	av_sint32_t				m_lDuration;
	av_sint32_t				m_lisVBR;
	av_uint32_t				m_Reserved[20];
} asf_dmx_audio_info_t;


typedef av_dmx_handle_t asf_dmx_handle_t;
typedef av_dmx_result_t asf_dmx_result_t;


/*!
 ============================================================
 *
 *	Demuxer Initialize
 *
 ============================================================
*/

//! Demuxer Initialize - input
typedef struct asf_dmx_init_t
{
	// common
	av_memory_func_t	m_stMemoryFuncs;		//!< [in] callback functions for memory
	av_file_func_t		m_stFileFuncs;			//!< [in] callback functions for file
	av_sint8_t			*m_pszOpenFileName;		/*!< [in] open file name( when Op is DMXOP_OPEN ) */

	//! asf specific info.
	av_sint32_t			m_lFileCacheSize;			//!< [in] file cache(buffer) size( default:4096 bytes )
	av_sint32_t			(* m_pfnTCC_ASF_DRM_Decrypt) (void *m_pDRMState, U64 PlayTime, void *m_pPayload, av_uint32_t PayloadSize);
#ifdef ANDROID_SLEEP
	void				(* m_pfnTCC_ASF_ANDROID_SLEEP) (av_ulong_t sleeptime);
	av_uint32_t			m_ulSleepPayloads;
#endif
	av_sint32_t			m_lAndroidProgressive;
	av_sint32_t			m_lGetRealDuration;
	av_sint32_t			m_lPlayOption;				//!< [in] ASF_SELECTIVE_MODE or ASF_SEQUENTIAL_MODE
	av_sint32_t			m_lSeekCacheSize;			//!< [in] seek cache(buffer) size
	av_sint32_t			m_lSeekOption;				//!< [in] ASF seek option
	av_uint32_t			m_Reserved[23];
} asf_dmx_init_t;


//! Demuxer Initialize - output
typedef struct asf_dmx_info_t
{
	// file info
	asf_dmx_file_info_t		*m_pstFileInfo;				//!< pointer to file information

	// video stream info
	av_sint32_t				m_lVideoStreamTotal;		//!< the number of video stream
	asf_dmx_video_info_t	*m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	asf_dmx_video_info_t	*m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	av_sint32_t				m_lAudioStreamTotal;		//!< the number of audio stream
	asf_dmx_audio_info_t	*m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	asf_dmx_audio_info_t	*m_pstDefaultAudioInfo;		//!< pointer to default audio information
	av_uint32_t				m_Reserved[25];
} asf_dmx_info_t;

typedef struct asf_dmx_outstream_t
{
	U64					m_ulCurrentPacketOffset;//!< [out] for PRESEEK OP.
	av_uint8_t			*m_pbyStreamData;		//!< [out] the pointer to outstream data
	av_sint32_t			m_lStreamDataSize;		//!< [out] the size to outstream data
	av_sint32_t			m_lStreamType;			//!< [out] the type of outstream
	av_sint32_t			m_lTimeStamp;			//!< [out] the timestamp of outstream
	av_sint32_t			m_lAspecRatioX;			//!< [out] the Pixel Aspect Ratio X
	av_sint32_t			m_lAspecRatioY;			//!< [out] the Pixel Aspect Ratio Y
	av_uint32_t			m_iStreamIdx;			//!< [out] stream ID
	av_uint32_t			m_Reserved[23];
} asf_dmx_outstream_t;

/*!
 ============================================================
 *
 *	Demuxer Running..
 *		- mkv has no a specific data
 *
 ============================================================
*/
typedef av_dmx_getstream_t	asf_dmx_getstream_t;
typedef av_dmx_seek_t		asf_dmx_seek_t;

//! Select stream - input
typedef struct asf_dmx_sel_stream_t
{
	av_sint32_t			m_lMultipleDemuxing;	//!< [in] If it's set, demux multiple stream at the same time.
	av_sint32_t			m_lSelType;		//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	av_sint32_t			m_lVideoID;		//!< [in] Video ID
	av_sint32_t			m_lAudioID;		//!< [in] Audio ID
	av_uint32_t			m_Reserved[28];
} asf_dmx_sel_stream_t;

//! Select stream - output
typedef struct asf_dmx_sel_info_t
{
	av_uint32_t				m_ulNumVideoStream;
	av_uint32_t				m_ulNumAudioStream;
	asf_dmx_video_info_t	m_pstVideoInfo[ASF_MAX_STREAM_NUMBER];	//!< pointer to video information
	asf_dmx_audio_info_t	m_pstAudioInfo[ASF_MAX_STREAM_NUMBER];	//!< pointer to audio information
	av_uint32_t				m_Reserved[30];
} asf_dmx_sel_info_t;


/*!
 ***********************************************************************
 * \brief
 *		TCC_ASF_GetJanusDRMV1Info	: copy the Janus DRM V1 header infomation to destination.
 * \param
 *		[in]pHandle			: handle of asf demuxer library
 * \param
 *		[in,out]pDest			: pointer to the destination.
 * \return
 *		None.
 ***********************************************************************
 */
void TCC_ASF_GetJanusDRMV1Info(asf_dmx_handle_t* pHandle, av_uint8_t* pDest );

/*!
 ***********************************************************************
 * \brief
 *		TCC_ASF_GetJanusDRMV2Info	: copy the Janus DRM V2 header infomation to destination.
 * \param
 *		[in]pHandle			: handle of asf demuxer library
 * \param
 *		[in,out]pDest			: pointer to the destination.
 * \return
 *		TCC_ASF_GetJanusDRMV2Info returns size of Janus DRM V2 header.
 ***********************************************************************
 */
int TCC_ASF_GetJanusDRMV2Info(asf_dmx_handle_t* pHandle, av_uint8_t* pDest );

/*!
 ***********************************************************************
 * \brief
 *		TCC_ASF_DMX		: main api function of asf demuxer library
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of asf demuxer library
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_ASF_DMX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
asf_dmx_result_t 
TCC_ASF_DMX( int Op, asf_dmx_handle_t* pHandle, void* pParam1, void* pParam2 );


/*!
 ============================================================
 *
 *	Return Values
 *
 ============================================================
*/
#define ASFDMXERR_ERROR_NONE						(0)
#define ASFDMXERR_END_OF_FILE						(-1)
#define ASFDMXERR_END_OF_VIDEO_FILE			(-2)
#define ASFDMXERR_END_OF_AUDIO_FILE			(-3)
#define ASFDMXERR_UNSUPPORTED_DRM				(-4)
#define ASFDMXERR_FILE_NOT_FOUND				(-5)
#define ASFDMXERR_REALLOC								(-6)
#define ASFDMXERR_MALLOC								(-7)
#define ASFDMXERR_AUDIO_FAIL						(-8)
#define ASFDMXERR_VIDEO_FAIL						(-9)
#define ASFDMXERR_BINARY_FAIL						(-10)
#define ASFDMXERR_INVALID_OP_CODE				(-11)
#define ASFDMXERR_BROKEN_PACKET					(-12)
#define ASFDMXERR_BAD_PACKET_HEADER			(-13)
#define ASFDMXERR_BAD_PAYLOAD_HEADER		(-14)
#define ASFDMXERR_UNSUPPORTED_VIDEO_OUTPUT_FORMAT	(-15)
#define ASFDMXERR_SELECTED_VIDEO_STREAM_NOT_EXIST	(-16)
#define ASFDMXERR_SELECTED_AUDIO_STREAM_NOT_EXIST	(-17)
#define ASFDMXERR_BAD_ASF_HEADER						(-18)
#define ASFDMXERR_INVALID_ARGUMENTS					(-19)
#define ASFDMXERR_BAD_MEMORY								(-20)
#define ASFDMXERR_BROKEN_FRAME							(-21)
#define ASFDMXERR_NOOUTPUT									(-22)
#define ASFDMXERR_BUFFER_TOO_SMALL					(-23)
#define ASFDMXERR_INVALID_STREAMTYPE				(-24)
#define ASFDMXERR_INVALID_STREAMID					(-25)
#define ASFDMXERR_INVALID_INDEX							(-26)
#define ASFDMXERR_INVALID_INDEXENTRY				(-27)
#define ASFDMXERR_CODEC_ENTRY_NOT_EXIST			(-28)
#define ASFDMXERR_DECODE_COMPLETE						(-29)
#define ASFDMXERR_UNSUPPORTED_VIDEO_FORMAT	(-30)
#define ASFDMXERR_UNSUPPORTED_AUDIO_FORMAT	(-31)
#define ASFDMXERR_SEEKTIME_OUTOFRANGE				(-32)
#define ASFDMXERR_UNAVAILABLE_OP_CODE				(-33)
#define ASFDMXERR_PROTECTION_ERROR					(-400)

#endif//ASF_DEMUXER_H_