/*!
 ***********************************************************************
 \par Copyright
 \verbatim
  ________  _____           _____   _____           ____  ____   ____		
     /     /       /       /       /       /     /   /    /   \ /			
    /     /___    /       /___    /       /____ /   /    /____/ \___			
   /     /       /       /       /       /     /   /    /           \		
  /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
   																				
  Copyright (c) 2008-2009 Telechips Inc.
  Korad Bldg, 1000-12 Daechi-dong, Kangnam-Ku, Seoul, Korea					
 \endverbatim
 ***********************************************************************
 */
/*!
 ***********************************************************************
 *
 * \file
 *		flv_demuxer.h
 * \date
 *		2008/03/03
 * \author
 *		AV algorithm group (AValgorithm@telechips.com) 
 * \brief
 *		FLV demuxer's header
 * \version
 *		1.1.1 : 2009/10/15 
 *
 ***********************************************************************
 */
#ifndef FLV_DEMUXER_H_
#define FLV_DEMUXER_H_

#include "../av_common_dmx.h"

/*!
 ============================================================
 *
 *	File/Video/Audio/Subtitle Information
 *		- information about video, audio or subtitle into one file.
 *
 ============================================================
*/
//! file information
typedef struct flv_dmx_file_info_t
{
    // common info
    av_sint8_t      *m_pszFileName;
    av_uint32_t    m_ulDuration;
    av_sint64_t     m_llFileSize;

    // flv specific
#define SEEK_NOT_AVAILABLE  (0)
#define SEEK_BY_META_INDEX  (1)
#define SEEK_BY_TAG_SCAN    (2)
    av_uint32_t     m_ulSeekMode;
} flv_dmx_file_info_t;

//! video stream information
typedef struct flv_dmx_video_info_t
{
    // common info
    av_uint32_t     m_ulStreamID;
    av_uint32_t     m_ulWidth;
    av_uint32_t     m_ulHeight;
    av_uint32_t     m_ulFrameRate;
    av_uint32_t     m_ulFourCC;
    av_uint8_t     *m_pbyCodecPrivate;
    av_uint32_t     m_ulCodecPrivateSize;

    // flv specific
    av_uint32_t     m_ulLastKeyTimestamp;
} flv_dmx_video_info_t;

//! audio stream information
typedef struct flv_dmx_audio_info_t
{
    // common info
    av_uint32_t     m_ulStreamID;
    av_uint32_t     m_ulSamplePerSec;
    av_uint32_t     m_ulBitsPerSample;
    av_uint32_t     m_ulChannels;
    av_uint32_t     m_ulFormatTag;
    av_uint8_t     *m_pbyCodecPrivate;
    av_uint32_t     m_ulCodecPrivateSize;
    av_sint8_t     *m_pszLanguage;

    // flv specific
    av_uint32_t     m_ulBitRate;
} flv_dmx_audio_info_t;

/*!
 ============================================================
 *
 *	Demuxer Initialize
 *
 ============================================================
*/
//! FLV Demuxer Specific OP Code
#define FLVDOP_OPEN_MEDIAINFO			 (10) //!< demuxer open (initialize)

//! flv_dmx_init_t.m_ulOption flags
#define FLVOPT_DEFAULT						(0x0000)	//!< Sequential Read / Use Internal Buffer
#define FLVOPT_SELECTIVE					(0x0001)	//!< Video/Audio Selective Read
#define FLVOPT_USERBUFF						(0x0002)	//!< Use User buffer
#define FLVOPT_PROGRESSIVE_FILE		(0x0004)	//!< progressive file access
#define FLVOPT_MEMORY_INDEXTABLE	(0x0008)	//!< read index table (meta data) to heap memory

//! Demuxer Initialize - input
typedef struct flv_dmx_init_t
{
    // common
    av_memory_func_t m_stMemoryFuncs;    //!< [in] callback functions for memory
    av_file_func_t   m_stFileFuncs;      //!< [in] callback functions for file
    av_sint8_t      *m_pszOpenFileName;  /*!< [in] open file name( when Op is DMXOP_OPEN ) */

    // flv specific
    av_uint32_t    m_ulOption;
    av_uint32_t    m_ulFIOBlockSizeNormal;
    av_uint32_t    m_ulFIOBlockSizeSeek;
} flv_dmx_init_t;

//! Demuxer Initialize - output
typedef struct flv_dmx_info_t
{
    // file info
    flv_dmx_file_info_t    *m_pstFileInfo;             //!< pointer to file information

    // video stream info
    av_uint32_t                m_ulVideoStreamTotal;      //!< number of video stream
    flv_dmx_video_info_t   *m_pstVideoInfoList;        //!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
    flv_dmx_video_info_t   *m_pstDefaultVideoInfo;     //!< pointer to default video information

    // audio stream info
    av_uint32_t                m_ulAudioStreamTotal;      //!< number of audio stream
    flv_dmx_audio_info_t   *m_pstAudioInfoList;        //!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
    flv_dmx_audio_info_t   *m_pstDefaultAudioInfo;     //!< pointer to default audio information

} flv_dmx_info_t;

/*!
 ============================================================
 *
 *	Demuxer Running..
 *		- flv has no a specific data
 *
 ============================================================
*/
#if 0
typedef av_dmx_getstream_t	flv_dmx_getstream_t;
typedef av_dmx_outstream_t	flv_dmx_outstream_t;
#else
typedef struct flv_dmx_getstream_t
{
    av_uint8_t    *m_pbyStreamBuff;        //!< [in] the pointer to getstream buffer
    av_uint32_t    m_ulStreamBuffSize;     //!< [in] the size of getstream buffer
    av_uint32_t    m_ulStreamType;         //!< [in] the type of getstream
    void           *m_pSpecificData;    //!< [in] the pointer to specific input of demuxer(if demuxer has a specific data)
} flv_dmx_getstream_t;

typedef struct flv_dmx_outstream_t
{
    av_uint8_t    *m_pbyStreamData;        //!< [out] the pointer to outstream data
    av_uint32_t    m_ulStreamDataSize;     //!< [out] the size to outstream data
    av_uint32_t    m_ulStreamType;         //!< [out] the type of outstream
    av_uint32_t    m_ulTimeStamp;          //!< [out] the timestamp of outstream
    av_uint32_t    m_ulEndTimeStamp;       //!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
    void           *m_pSpecificData;    //!< [out] the pointer to specific output of demuxer(if demuxer has a specific data)
} flv_dmx_outstream_t;

#endif


/*!
 ============================================================
 *
 *	Get Errors
 *
 ============================================================
*/
#define FLVDERR_NONE								(0)

//! system error
#define FLVDERR_BASE_SYSTEM_ERROR					(0)
#define FLVDERR_SYSTEM_ERROR						(FLVDERR_BASE_SYSTEM_ERROR - 1 )
#define FLVDERR_FILE_SEEK_FAILED					(FLVDERR_BASE_SYSTEM_ERROR - 2 )
#define FLVDERR_PRIVATE_BUFF_ALLOCATION_FAILED		(FLVDERR_BASE_SYSTEM_ERROR - 3 )
#define FLVDERR_DEMUXER_OBJECT_ALLOCATION_FAILED	(FLVDERR_BASE_SYSTEM_ERROR - 4 )
#define FLVDERR_FILE_OPEN_FAILED0					(FLVDERR_BASE_SYSTEM_ERROR - 5 )
#define FLVDERR_FILE_OPEN_FAILED1					(FLVDERR_BASE_SYSTEM_ERROR - 6 )
#define FLVDERR_FRAME_BUFFER_ALLOCATION_FAILED		(FLVDERR_BASE_SYSTEM_ERROR - 7 )

//! broken file error
#define FLVDERR_BASE_BROKEN_FILE					(-1000)
#define FLVDERR_BROKEN_FILE							(FLVDERR_BASE_BROKEN_FILE - 0 )
#define FLVDERR_INVALID_BODYOFFSET					(FLVDERR_BASE_BROKEN_FILE - 1 )
#define FLVDERR_ELEMENT_STREAM_NOT_FOUND			(FLVDERR_BASE_BROKEN_FILE - 2 )
#define FLVDERR_METADATA_NOT_FOUND					(FLVDERR_BASE_BROKEN_FILE - 3 )
#define FLVDERR_MAIN_HEADER_NOT_FOUND				(FLVDERR_BASE_BROKEN_FILE - 4 )
#define FLVDERR_ITS_BROKEN_FILE						(FLVDERR_BASE_BROKEN_FILE - 5 )
#define FLVDERR_INVALID_PREVIOUS_TAG_SIZE			(FLVDERR_BASE_BROKEN_FILE - 6 )

//! seek failed error
#define FLVDERR_BASE_SEEK_FAILED					(-2000)
#define FLVDERR_SEEK_FAILED							(FLVDERR_BASE_SEEK_FAILED - 0)
#define FLVDERR_VIDEO_SEEK_FAILED					(FLVDERR_BASE_SEEK_FAILED - 1)
#define FLVDERR_AUDIO_SEEK_FAILED					(FLVDERR_BASE_SEEK_FAILED - 2)
#define FLVDERR_SEEK_FAILED_EOF						(FLVDERR_BASE_SEEK_FAILED - 3)
#define FLVDERR_VIDEO_SEEK_FAILED_EOF				(FLVDERR_BASE_SEEK_FAILED - 4)
#define FLVDERR_AUDIO_SEEK_FAILED_EOF				(FLVDERR_BASE_SEEK_FAILED - 5)
#define FLVDERR_INTERNAL_SEEK_ERROR					(FLVDERR_BASE_SEEK_FAILED - 6)
#define FLVDERR_TABLE_ERROR_ENCOUNTED				(FLVDERR_BASE_SEEK_FAILED - 7)

//! not supported format
#define FLVDERR_BASE_NOT_SUPPORTED_FORMAT			(-3000)
#define FLVDERR_NOT_SUPPORTED_FORMAT				(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 0)
#define FLVDERR_AVCC_PARSING_FAILED					(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 1)
#define FLVDERR_UNKNOWN_VIDEO_HEADER				(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 2)
#define FLVDERR_UNKNOWN_AUDIO_HEADER				(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 3)
#define FLVDERR_SEQUENCE_HEADER_NOT_FOUND			(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 4)

//! invalid parameter error
#define FLVDERR_BASE_INVALID_FUNC_PARAM				(-4000)
#define FLVDERR_INVALID_FUNC_PARAM					(FLVDERR_BASE_INVALID_FUNC_PARAM - 0 )
#define FLVDERR_INVALID_DEMUXER_HANDLE				(FLVDERR_BASE_INVALID_FUNC_PARAM - 1 )
#define FLVDERR_VIDEO_STREAM_NOT_EXIST				(FLVDERR_BASE_INVALID_FUNC_PARAM - 2 )
#define FLVDERR_AUDIO_STREAM_NOT_EXIST				(FLVDERR_BASE_INVALID_FUNC_PARAM - 3 )
#define FLVDERR_INVALID_SELECTION_INFO				(FLVDERR_BASE_INVALID_FUNC_PARAM - 4 )
#define FLVDERR_INVALID_SEEK_TIME					(FLVDERR_BASE_INVALID_FUNC_PARAM - 5 )
#define FLVDERR_OPERATION_NOT_SUPPORTED				(FLVDERR_BASE_INVALID_FUNC_PARAM - 6 )
#define FLVDERR_OPENED_FOR_MEDIAINFO				(FLVDERR_BASE_INVALID_FUNC_PARAM - 7 )

//! demuxer internal error
#define FLVDERR_BASE_DEMUXER_INTERNAL				(-10000)
#define FLVDERR_DEMUXER_INTERNAL					(FLVDERR_BASE_DEMUXER_INTERNAL - 0)
#define FLVDERR_LAST_TAG_NOT_FOUND					(FLVDERR_BASE_DEMUXER_INTERNAL - 1)
#define FLVDERR_END_OF_STREAM						(FLVDERR_BASE_DEMUXER_INTERNAL - 2)
#define FLVDERR_SEEK_UNAVAILABLE					(FLVDERR_BASE_DEMUXER_INTERNAL - 3)


#define IS_SYSERROR(code)						  ((    0 >  (code)) && ((code) > -1000))
#define IS_FILEERROR(code)						((-1000 >= (code)) && ((code) > -2000))
#define IS_SEEKERROR(code)						((-2000 >= (code)) && ((code) > -3000))
#define IS_FORMATERROR(code)					((-3000 >= (code)) && ((code) > -4000))
#define IS_PARAMERROR(code)						((-4000 >= (code)) && ((code) > -5000))
#define IS_INTERNALERROR(code)			(-10000 >= (code))


/*!
 ***********************************************************************
 * \brief
 *		TCC_FLV_DMX		: main api function of flv demuxer library
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of flv demuxer library
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in,out]pParam2	: output or information parameter
 * \return
 *		If successful, TCC_FLV_DMX returns 0 or positive value. Otherwise, it returns a negative value.
 ***********************************************************************
 */
av_dmx_result_t
TCC_FLV_DMX( av_uint32_t Op, av_dmx_handle_t* pHandle, void* pParam1, void* pParam2 );



#endif/*FLV_DEMUXER_H_*/
