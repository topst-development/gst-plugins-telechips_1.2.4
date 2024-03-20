/****************************************************************************
 *   FileName    : audio_demuxer.h
 *   Description : 
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

#ifndef AUDIO_DEMUXER_H__
#define AUDIO_DEMUXER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../av_common_dmx.h"
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Definitions
//
typedef av_handle_t audio_handle_t;

// opCode
#define AUDIO_DMX_OPEN			(0)
#define AUDIO_DMX_GETINFO		(1)
#define AUDIO_DMX_GETFRAME	(3)
#define AUDIO_DMX_CLOSE			(4)
#define AUDIO_DMX_SEEK			(5)

// Return value
#define ERR_AUDIO_DMX_NONE				 (0)
#define ERR_AUDIO_DMX_END_OF_FILE		(-1)

#define CONTAINER_TYPE_AUDIO_AAC	(0)
#define CONTAINER_TYPE_AUDIO_MP3	(1)
#define CONTAINER_TYPE_AUDIO_MP2	(2)
#define CONTAINER_TYPE_AUDIO_VORBIS	(3)
#define CONTAINER_TYPE_AUDIO_FLAC	(4)
#define CONTAINER_TYPE_AUDIO_APE	(5)
#define CONTAINER_TYPE_AUDIO_WAV	(6)
#define CONTAINER_TYPE_AUDIO_MP3HD	(7)

#define SEEK_RELATIVE	(0)
#define SEEK_ABSOLUTE	(1)

#ifndef NULL
#define NULL (0)
#endif

/*
============================================
  Demuxer Initialize (AVIOP_OPEN) : Input
============================================
*/

#define	APE_TAGINFO_BYTE_SIZE			(256)

typedef struct ape_tag_info_t
{
	av_sint8_t      m_cArtist[APE_TAGINFO_BYTE_SIZE+1];
	av_sint8_t      m_cAlbum[APE_TAGINFO_BYTE_SIZE+1];
	av_sint8_t      m_cTitle[APE_TAGINFO_BYTE_SIZE+1];
	av_sint8_t      m_cTrack[APE_TAGINFO_BYTE_SIZE+1];
	av_sint8_t      m_cGenre[APE_TAGINFO_BYTE_SIZE+1];
	av_sint8_t      m_cYear[APE_TAGINFO_BYTE_SIZE+1];
	av_sint8_t      m_cComment[APE_TAGINFO_BYTE_SIZE+1];
} ape_tag_info_t;

//! common callback function
typedef struct audio_callback_func_t
{
	void* (*m_pMalloc		 ) ( size_t size);					//!< malloc
	void* (*m_pNonCacheMalloc) ( size_t size);					//!< non-cacheable malloc 
	void  (*m_pFree			 ) ( void* ptr);							//!< free
	void  (*m_pNonCacheFree	 ) ( void* ptr);							//!< non-cacheable free
	void* (*m_pMemcpy		 ) ( void* dst, const void* src, size_t num);//!< memcpy
	void* (*m_pMemset		 ) ( void* ptr, av_sint32_t value, size_t num);		//!< memset
	void* (*m_pRealloc		 ) ( void* ptr, size_t size);				//!< realloc
	void* (*m_pMemmove		 ) ( void* dst, const void* src, size_t num);//!< memmove
	av_sint32_t (*m_pStrncmp ) ( av_sint8_t* str1, const av_sint8_t* str2, av_sint32_t num); //!< strncmp

	void*		 (*m_pFopen	) (const av_sint8_t *filename, const av_sint8_t *mode);						//!< fopen
	av_size_t (*m_pFread	) (void* ptr, av_size_t size, av_size_t count, void* stream);		//!< fread
	av_sint32_t	 (*m_pFseek	) (void* stream, av_long_t offset, av_sint32_t origin);						//!< fseek 32bit io
	av_long_t	 (*m_pFtell	) (void* stream);											//!< ftell 32 bit io
	size_t (*m_pFwrite) (const void* ptr, size_t size, size_t count, void* stream);	//!< fwrite
	av_sint32_t		 (*m_pFclose) (void *stream);											//!< fclose
	av_sint32_t		 (*m_pUnlink) ( const av_sint8_t * pathname);									//!< _unlink
	size_t (*m_pFeof  ) (void *stream);											//!< feof
	size_t (*m_pFflush) (void *stream);											//!< fflush

	av_sint32_t	 (*m_pFseek64) ( void* stream, S64 offset, av_sint32_t origin);							//!< fseek 64bit io
	S64		 (*m_pFtell64) ( void* stream);										//!< ftell 64bit io

} audio_callback_func_t;

typedef struct audio_dmx_init_t 
{
	void*						m_pOpenFileName;	//!< Open file name
	audio_callback_func_t		m_sCallbackFunc;	//!< System callback function
	av_sint32_t				m_iIoBlockSize;		//!< File read cache size
	av_uint32_t				m_uiOption;			//!< Demuxing options
	av_uint32_t				m_uiDmxType;		//!< Demuxer Type
	audio_handle_t				m_iDmxHandle;		//!< Demuxer Handle
	av_uint32_t				m_uiId3TagOffset;
	void						*m_pvExtraInfo;
} audio_dmx_init_t;

//! common demuxer information
typedef struct audio_dmx_info_t
{
	//! file information : 128 Bytes
	struct 
	{
		/* common */
		av_sint8_t* m_pszOpenFileName;	//!< open file name
		av_sint32_t m_iFileSize;			//!< total file size
		av_sint32_t m_iRunningtime;			//!< runing time * 1000

		av_sint32_t m_Reserved[32-3];
	} m_sFileInfo;

	//! video information : 128 Bytes
	struct 
	{
		/* common */
		av_sint32_t m_iTotalNumber;			//!< total audio number
		av_sint32_t m_iSamplePerSec;		//!< samples/sec
		av_sint32_t m_iBitsPerSample;		//!< bits/sample
		av_sint32_t m_iChannels;			//!< channels
		av_sint32_t m_iFormatId;			//!< format id

		/* extra info (common) */
		av_uint8_t* m_pExtraData;//!< extra data
		av_sint32_t m_iExtraDataLen;		//!< extra data length
		
		av_sint32_t m_iMinDataSize;			//!< minimum data size

		/* aac dec */
		av_sint32_t m_iAacHeaderType;		//!< aac header type ( 0 : RAW-AAC, 1: ADTS, 2: ADIF)
		av_sint32_t m_iAacBSAC;				//!< m_iAacBSAC: ( 0 : aac, 1 : bsac )
		
		av_sint32_t m_iBitRate;				//!< bitrate
		av_sint32_t m_iBlockAlign;			//!< block align
		
		av_sint32_t m_Reserved[32-12];			
	} m_sAudioInfo;
	
	ape_tag_info_t *pApeTagInfo;	

} audio_dmx_info_t;

//! input parameter
typedef struct audio_dmx_input_t
{
	av_uint8_t* m_pPacketBuff;		//!< [in] allocated packet(video or audio) buffer pointer
	av_sint32_t m_iPacketBuffSize;				//!< [in] allocated packet(video or audio) buffer size
	av_sint32_t m_iPacketType;					//!< [in] PACKET_NONE or PACKET_VIDEO or PACKET_AUDIO
										// if m_pPacketBuffer is zero, mp4 parser's internal memory will be used.
	av_sint32_t m_iUsedBytes;
	av_sint32_t m_Reserved[8-4];
} audio_dmx_input_t;

//! output parameter
typedef struct audio_dmx_output_t
{
	av_uint8_t* m_pPacketData;	//!< pointer of output data
	av_sint32_t m_iPacketSize;				//!< length of output data
	av_sint32_t m_iPacketType;				//!< packet type of output data
	av_sint32_t m_iTimeStamp;				//!< timestamp(msec) of output data
	av_sint32_t m_iEndOfFile;				//!< end of file
	av_sint32_t m_iDecodedSamples;			//!< number of frame in the m_iPacketSize
	av_uint32_t m_uiUseCodecSpecific; //!< variable use in several codec
	av_sint32_t m_Reserved[32-7];			//!< reserved...
} audio_dmx_output_t;

//! seek parameter
typedef struct audio_dmx_seek_t
{
	av_sint32_t m_iSeekTimeMSec;	//!< seek time(unit : second) from current time
	av_sint32_t m_iSeekMode;		//!< Seek mode : 0(default video) 1(video) 2(audio)
} audio_dmx_seek_t;

av_sint32_t TCC_APE_DMX( av_sint32_t iOpCode, audio_handle_t* pHandle, void* pParam1, void* pParam2 );

#ifdef __cplusplus
}
#endif
#endif //AUDIO_DEMUXER_H__
