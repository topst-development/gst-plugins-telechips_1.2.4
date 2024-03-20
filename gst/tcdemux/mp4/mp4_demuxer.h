/****************************************************************************
 *   FileName    : mp4_demuxer.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.1
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information��?s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef MP4_DEMUXER_H_
#define MP4_DEMUXER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "av_common_dmx.h"


#define MP4_DMX_VERSION ("3.10")

//! Op Code 
#define MP4DMXOP_GET_VERSION            (0x1000)
#define MP4DMXOP_SET_OPTIONS            (0x1001)

	

#define MP4_GET_AVC_INFO (100) //!< opcode : get sps/pps info
#define MP4_GET_HVC_INFO (1) //!< opcode : get sps/pps info
#define MP4DMX_CALC_REAL_DURATION 	(1)
#define MP4DMX_SEARCH_LASTKEY_FRAME	(0x00000002) //(1<<1)
#define MP4DMX_SAVE_STBL_INTERNAL	(0x00000004) //(1<<2)
#define MP4DMX_PREVENT_EOF_FSEEK	(0x00000008) //(1<<3)
#define MP4DMX_MEDIA_SCAN_MODE_ON	(0x00000010) //(1<<4)
#define MP4DMX_SEQUENTIAL_GETSTREAM	(0x00000020) //(1<<5)
#define MP4DMX_SEEK_KEY_FRAME_ENABLE	(0x00000040) //(1<<6)


//! Return value
#ifndef ERR_NONE
#define ERR_NONE				 (0)
#endif
#ifndef ERR_END_OF_FILE
#define ERR_END_OF_FILE			(-1)
#endif
#ifndef ERR_END_OF_VIDEO_FILE
#define ERR_END_OF_VIDEO_FILE	(-2)
#endif
#ifndef ERR_END_OF_AUDIO_FILE
#define ERR_END_OF_AUDIO_FILE	(-3)
#endif
#ifndef ERR_END_OF_SUBTITLE_FILE
#define ERR_END_OF_SUBTITLE_FILE (-4)
#endif

#define MP4DMX_ERROR_BASE (-27000)
#define MP4DMX_ERROR_INVALID_AUDIOID		(MP4DMX_ERROR_BASE-21)
#define MP4DMX_ERROR_INVALID_VIDEOID		(MP4DMX_ERROR_BASE-22)
#define MP4DMX_ERROR_INVALID_SUBTITLEID		(MP4DMX_ERROR_BASE-23)

#ifdef MP4_GET_AVC_INFO

//! sps/pps parameter set (demuxer) : 8 bytes
typedef struct mp4_avc_parameter_set_t
{
	void* m_pData;		//!< pointer of sps or pps 
	av_sint32_t m_iDataLength;	//!< size of sps or pps 
} mp4_avc_parameter_set_t;

//! avcC info(demuxer) : AVCDecoderConfigurationRecord in avc file format(ISO/IEC 14496-15) : 28 bytes
typedef struct mp4_avcC_t 
{
	av_sint32_t m_iSpsNum;						//!< numbers of sps
	mp4_avc_parameter_set_t* m_pSpsArray;	//!< array of sps data
	av_sint32_t m_iPpsNum;						//!< numbers of pps
	mp4_avc_parameter_set_t* m_pPpsArray;	//!< array of pps data
	av_sint32_t m_iNalLenSize;					//!< size of nal length
} mp4_avcC_t;

//! avc info interface : 36 bytes
typedef struct st_mp4_dmx_avc_info_t
{
	void* m_pExtraData;			//!< [IN] pointer of extra data
	av_sint32_t m_iExtraDataLength;		//!< [IN] length of extra data
	mp4_avcC_t* m_pAvcC;			//!< [OUT] pointer of avcC info
} mp4_dmx_avc_info_t;

#endif//MP4_GET_AVC_INFO

//! common callback function : 128 bytes
typedef struct mp4_dmx_callback_func_t
{
    void* (*m_pMalloc        ) (av_size_t size);                                    //!< malloc
    void* (*m_pNonCacheMalloc) (av_size_t size);                                    //!< non-cacheable malloc 
    void  (*m_pFree          ) (void* ptr);                                        //!< free
    void  (*m_pNonCacheFree  ) (void* ptr);                                        //!< non-cacheable free
    void* (*m_pMemcpy        ) (void* dst, const void* src, av_size_t num);             //!< memcpy
    void* (*m_pMemset        ) (void* ptr, av_sint32_t size, av_size_t num);                     //!< memset

    void* (*m_pRealloc       ) (void* ptr, av_size_t size);                          //!< realloc
    void* (*m_pMemmove       ) (void* dst, const void* src, av_size_t num);             //!< memmove
    av_sint32_t m_Reserved1[16-8];

    void*        (*m_pFopen  ) (const av_sint8_t* filename, const av_sint8_t* mode);                     //!< fopen
    av_size_t    (*m_pFread  ) (void* ptr, av_size_t size, av_size_t count, void* stream);     //!< fread
    av_sint32_t  (*m_pFseek  ) (void* stream, av_long_t offset, av_sint32_t origin);                             //!< fseek 32bit io
    av_long_t    (*m_pFtell  ) (void* stream);                                        //!< ftell 32bit io
    av_size_t    (*m_pFwrite ) (const void* ptr, av_size_t size, av_size_t count, void* stream);//!< fwrite (Muxer only)
    av_sint32_t  (*m_pFclose ) (void* stream);                                        //!< fclose
    av_sint32_t  (*m_pUnlink ) (const av_sint8_t* pathname);                                  //!< unlink (Muxer only)
    av_size_t    (*m_pFeof   ) (void* stream);                                        //!< feof
    av_size_t    (*m_pFflush ) (void* stream);                                        //!< fflush
    av_sint32_t  (*m_pFseek64) (void* stream, av_sint64_t offset, av_sint32_t origin);                      //!< fseek 64bit io
    av_sint64_t  (*m_pFtell64) (void* stream);                                        //!< ftell 64bit io
    av_sint32_t m_Reserved2[16-11];
} mp4_dmx_callback_func_t;

typedef av_dmx_handle_t mp4_dmx_handle_t;	//!< handle of libmp4
typedef av_dmx_result_t mp4_dmx_result_t;	//!< return value of libmp4

//! init parameter of TCC_MP4_DMX() : 128+32+96 bytes
typedef struct mp4_dmx_init_t 
{
	mp4_dmx_callback_func_t m_sCallbackFunc;//!< [in] callback functions of libmp4
	void* 	m_pOpenFileName;	//!< [in] open file name( when Op is INIT )
							//!< [in] handle of opened file( when Op is INIT_FROM_FILE_HANDLE )
	av_sint32_t		m_iRealDurationCheckEnable;
	av_sint32_t		m_iLastkeySearchEnable;
	av_sint32_t		m_iSaveInternalIndex;
	av_sint32_t		m_iPreventFseekToEof;
	av_sint32_t		m_iSequentialGetStream;
	av_sint32_t		m_iSeekKeyframeEnable;
	av_sint32_t     m_Reserved[8-7];

	//! 96 Bytes
	union 
	{
		av_sint32_t m_ReservedExtra[24];
		struct 
		{
			av_sint32_t m_iVideoFileCacheSize;	//!< [in] video file cache(buffer) size
			av_sint32_t m_iAudioFileCacheSize;	//!< [in] audio file cache(buffer) size
			av_sint32_t m_iSubtitleFileCacheSize;	//!< [in] audio file cache(buffer) size			av_sint32_t m_iSTSCCacheSize;		//!< [in] cache(buffer) size of STSC chunk
			av_sint32_t m_iSTSCCacheSize;		//!< [in] cache(buffer) size of STSC chunk
			av_sint32_t m_iSTSSCacheSize;		//!< [in] cache(buffer) size of STSS chunk
			av_sint32_t m_iSTSZCacheSize;		//!< [in] cache(buffer) size of STSZ chunk
			av_sint32_t m_iSTCOCacheSize;		//!< [in] cache(buffer) size of STCO chunk
			av_sint32_t m_iSTTSCacheSize;		//!< [in] cache(buffer) size of STTS chunk
			av_sint32_t m_iDefaultCacheSize;	//!< [in] cache(buffer) base file cache. On sequential mode, only this cache can be used.
			av_sint32_t m_ReservedExtra[24-9];
		} m_sCache;
	} m_uExtra;

} mp4_dmx_init_t;

typedef struct mp4_dmx_info_t
{
	//! file information : 128 Bytes
	struct 
	{
		/* common */
		av_sint8_t* 	m_pszOpenFileName;	//!< open file name
		av_sint8_t* 	m_pszCopyright;		//!< copyright
		av_sint8_t* 	m_pszCreationTime;	//!< creation time
		av_sint32_t 	m_iRunningtime;			//!< runing time * 1000
		av_sint64_t 	m_lFileSize;			//!< total file size

		av_sint64_t 	m_lUserDataPos;		//!< user data position
		av_sint32_t 	m_iTimeScale;			//!< timescale of file
		av_sint32_t 	m_iUserDataLen;			//!< user data length  40 bytes
		
		av_sint32_t					m_iNumTotalTracks;
		av_sint32_t					m_iSeekable;			// if(==1) seek is possible or ..
		av_sint8_t*				m_pcTitle;				
		av_sint8_t*				m_pcArtist;
		av_sint8_t*				m_pcAlbum;				//!<80 bytes
		av_sint8_t*				m_pcAlbumArtist;
		av_sint8_t*				m_pcYear;
		av_sint8_t*				m_pcWriter;
		av_sint8_t*				m_pcAlbumArt;
		av_sint32_t					m_iAlbumArtSize;
		av_sint8_t*				m_pcGenre;				//!< 100 bytes
		av_sint8_t				m_pcCompilation[4];		//!< 104 bytes
		av_sint8_t				m_pcCDTrackNumber[16];	//!< 120 bytes
		av_sint8_t				m_pcDiscNumber[16];		//!< 136 bytes
		av_sint8_t*				m_pcLocation;			//!< 140 bytes
		av_sint32_t					m_bStandardGenre;
		av_sint32_t					m_iFragmentedMp4;
		av_sint32_t 				m_Reserved[48-33];
	} m_sFileInfo;

	//! video information : 128 Bytes
	struct 
	{
		av_sint32_t					m_iVideoTrackCount;			// total track count
		av_sint32_t					m_iDefaultVideoTrackIndex; // if -1 no video track available
		av_sint32_t					m_iVideoTrackIndexCache;
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

		av_sint32_t m_iTotalNumber;			//!< total frame number
		av_sint32_t m_iKeyFrameNumber;		//!< key frame number
		av_sint32_t m_bAvcC;				//!< avcC flag for H264
		av_sint32_t m_iTrackTimeScale;		//!< timescale of video
		av_sint32_t m_iLastKeyTime;			//!< time of last key frame

		av_sint32_t	m_iMaxBitrate;			//!< maximum bitrate
		av_sint32_t m_iAvgBitrate;			//!< average bitrate
		av_sint32_t m_iRotateDegree;		//!< rotation degree
		av_uint32_t		m_uiAspectRatioX;
		av_uint32_t		m_uiAspectRatioY;		
		av_uint32_t        m_uiFirstSampleOffset;
		av_uint32_t        m_bHasBFrames;
		av_sint32_t m_Reserved[32-23];			
	} m_sVideoInfo;

	//! audio information : 128 Bytes
	struct 
	{
		av_sint32_t					m_iAudioTrackCount;			// total track count
		av_sint32_t					m_iDefaultAudioTrackIndex; // if -1 no audio track available
		av_sint32_t					m_iAudioTrackIndexCache;
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

		/* mp4 info */
		av_sint32_t m_iFramesPerSample;		//!< fps
		av_sint32_t m_iTrackTimeScale;		//!< timescale of audio
		av_sint32_t m_iSamplesPerPacket;	//!< samples / packet

		av_sint32_t	m_iMaxBitrate;			//!< maximum bitrate
		av_sint32_t m_iAvgBitrate;			//!< average bitrate
		av_uint32_t        m_uiFirstSampleOffset;

		av_sint32_t m_Reserved[32-18];			
	} m_sAudioInfo;

	struct
	{
		av_sint32_t				m_iSubtitleTrackCount;
		av_sint32_t				m_iCurrentSubtitleTrackIndex;
		av_sint32_t				m_iSubtitleTrackIndexCache;
		av_sint8_t			*m_pszLanguage;

//http://www.matroska.org/technical/specs/subtitles/index.html
#define SUBTITLETYPE_SRT		(0) // SRT subtitle
#define SUBTITLETYPE_SSA		(1) // SSA subtitle
#define SUBTITLETYPE_USF		(2) // Universal Subtitle Format
#define SUBTITLETYPE_BMP		(3) // BMP
#define SUBTITLETYPE_VOBSUB	(4) // VOBSUB
#define SUBTITLETYPE_KATE		(5) // KATE
		av_uint32_t	 m_ulSubtitleType;
		void			*m_pSubtitleInfo;		// if exist
		av_uint32_t	 m_ulSubtitleInfoSize;
	} m_stSubtitleInfo;

} mp4_dmx_info_t;


//! options
typedef struct mp4_dmx_options_t
{
	// seek process between the last keyframe and the end of file.
#define MP4DOPT_DO_NOTHING                  (0)	// default: keep current timestamp
#define MP4DOPT_TO_END_OF_FILE              (1)	// go to the end of file.
#define MP4DOPT_LAST_KEYFRAME               (2)	// find the last keyframe
	av_sint32_t m_ulOptSeekOverLastKey;
} mp4_dmx_options_t;




typedef struct mp4_dmx_input_t
{
	av_uint8_t* m_pPacketBuff;		//!< [in] allocated packet(video or audio) buffer pointer
	av_sint32_t m_iPacketBuffSize;				//!< [in] allocated packet(video or audio) buffer size
	av_sint32_t m_iPacketType;					//!< [in] PACKET_NONE or PACKET_VIDEO or PACKET_AUDIO
	av_sint32_t m_iUsedBytes;					//!< used bytes
	av_sint32_t	m_Reserved[8-4];
} mp4_dmx_input_t;

typedef struct mp4_dmx_output_t
{
	av_uint8_t* m_pPacketData;	//!< pointer of output data
	av_sint32_t m_iPacketSize;				//!< length of output data
	av_sint32_t m_iPacketType;				//!< packet type of output data
	av_sint32_t m_iTimeStamp;				//!< timestamp(msec) of output data
	av_sint32_t m_bKeyFrame;				//!< key flag of output data
	av_sint32_t m_iKeyCount;				//!< current key count
	av_sint64_t m_llOffset;					//!< current key count
	av_sint32_t m_iEndTimeStamp;			//!< end timestamp(msec) of output data
	av_sint32_t m_Reserved[32-9];			//!< reserved...
} mp4_dmx_output_t;

typedef struct mp4_dmx_seek_t
{
	av_sint32_t			m_lSeekTime;	//!< millisecond unit
	av_uint32_t	m_ulSeekMode;	//!< mode flags
} mp4_dmx_seek_t;

//! Select stream - input
typedef struct mp4_dmx_sel_stream_t
{
	av_uint32_t	 m_ulSelType;		//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	av_uint32_t	 m_ulVideoID;		//!< [in] Video ID (if m_ulSelType has DMXTYPE_VIDEO)
	av_uint32_t	 m_ulAudioID;		//!< [in] Audio ID (if m_ulSelType has DMXTYPE_AUDIO)
	av_uint32_t	 m_ulSubtitleID;	//!< [in] Subtitle ID (if m_ulSelType has DMXTYPE_SUBTITLE)
} mp4_dmx_sel_stream_t;

/*!
 ***********************************************************************
 * \brief
 *		TCC_MP4_DMX		: main api function of libmp4
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: libmp4's handle
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_MP4_DMX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
mp4_dmx_result_t 
TCC_MP4_DMX(av_sint32_t Op, mp4_dmx_handle_t* pHandle, void* pParam1, void* pParam2);

#ifdef __cplusplus
}
#endif
#endif//MP4_DEMUXER_H_
