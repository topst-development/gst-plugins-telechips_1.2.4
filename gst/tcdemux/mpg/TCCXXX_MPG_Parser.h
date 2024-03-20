/*
 * ________  _____           _____   _____           ____  ____   ____		
 *    /     /       /       /       /       /     /   /    /   \ /			
 *   /     /___    /       /___    /       /____ /   /    /____/ \___			
 *  /     /       /       /       /       /     /   /    /           \		
 * /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
 *  																				
 * Copyright (c) 2008 Telechips Inc.
 * Korad Bldg, 1000-12 Daechi-dong, Kangnam-Ku, Seoul, Korea					
 *
 *
 *
 * TCCXXX_MPG_Parser.h
 *
 * This file is api of libmpg.
 */
#ifndef MPG_H_
#define MPG_H_

#include "av_common_dmx.h"

//Format Discribtion
#define FOURCC(ch0, ch1, ch2, ch3)                              \
	((av_uint32_t)(av_uint8_t)(ch0) | ((av_uint32_t)(av_uint8_t)(ch1) << 8) |   \
	((av_uint32_t)(av_uint8_t)(ch2) << 16) | ((av_uint32_t)(av_uint8_t)(ch3) << 24 ))

//Video Format
#define FOURCC_mpeg			(FOURCC('m','p','e','g'))
#define FOURCC_mp4v			(FOURCC('m','p','4','v'))
#define FOURCC_avc1			(FOURCC('a','v','c','1'))

//Audio Format

//format tag of mpeg layer 1 is same with mpeg layer 2.
//In our solution, however, mpeg layer1 is not supported.
//so, we reset mpeg layer1 tag id as 0xFFFFFFFF
#define AUDIOFMT_MP1			(0xCC01)
#define AUDIOFMT_MP2			(0x50)
#define AUDIOFMT_MP3			(0x55)

#define AUDIOFMT_PCM			(0x01)
#define AUDIOFMT_A52			(0x2000)//AC3
#define AUDIOFMT_DTS			(0x2001)//DTS
#define AUDIOFMT_AAC			(0xAAC0)

//Operation Type
#define MPG_OP_INIT			(0)
#define MPG_OP_CLOSE			(1)
#define MPG_OP_SUBTITLE		(2)	//Decode RLE bitmap for subtitle
#define MPG_OP_SEEK			(3)
#define MPG_OP_GETSTREAM		(4)	//this means decoding
#define MPG_OP_SELSTREAM		(5)
#define MPG_OP_SCAN			(7)

//Operation Mode
	//* In case of Sequential Mode, parser doesn't consider the type of current packet(Video or Audio).
	//Parser just decode packets in order.

	//* Interleaved mode could be used in case that multiplexing between video and audio is very bad.
	//In that case, Video FIFO or Audio FIFO could be overflowed to keep precise PTS.
	//If interleaved mode is used, user can choose the type of data(Audio or Video) in every decoding point.
	//In other word, if user wants video data, parser would decode next video data, and vice versa.
#define MPG_MODE_SEQUENTIAL		(0x0)
#define MPG_MODE_INTERLEAVED		(0x1)

//Stream Selection related
#define MPG_MAX_STREAM_NUMBER	(32)

#define MPG_STREAM_INDEX_(n)	((guint32)1 << (n))

#define MPGOPT_SEEK_PREV_POS_AFTER_SEEK_ERROR	(0x0000) //Default
#define MPGOPT_SEEK_SEND_EOF_AFTER_SEEK_ERROR	(0x0001)
#define MPGOPT_SEEK_LAST_KEY_AFTER_SEEK_ERROR	(0x0002)

typedef av_dmx_handle_t	mpg_handle_t;
typedef av_dmx_result_t mpg_result_t;

typedef struct mpg_video_info_t
{
	av_uint32_t m_Index;
	av_uint32_t m_FourCC;
	av_uint32_t m_Width;
	av_uint32_t m_Height;
	av_uint32_t m_AspectRatio;//(height / width) * 10000
	av_uint32_t m_FrameRate; //fremarate * 1000;
	av_uint32_t m_BitRate;

	av_sint32_t m_StartPTS;
	av_sint32_t m_EndPTS;
} mpg_video_info_t;

typedef struct mpg_audio_info_t
{
	av_uint32_t m_Index;
	av_uint32_t m_FormatId;
	av_uint32_t m_SamplePerSec;
	av_uint32_t m_BitsPerSample;
	av_uint32_t m_BitsRate;
	av_uint32_t m_Channels;
	av_uint32_t m_SyncWord;

	av_uint32_t m_ChunkSize;

	av_sint32_t m_StartPTS;
	av_sint32_t m_EndPTS;
} mpg_audio_info_t;

typedef struct mpg_dec_info_t
{
	/* file information */
	U64 m_FileSize;
	av_uint32_t m_Runningtime;// in milli-second

	av_uint32_t		m_uiTotalVideoNum;
	av_sint32_t		m_uiDefaultVideo;
	mpg_video_info_t *	m_pszVideoInfo;

	av_uint32_t		m_uiTotalAudioNum;
	av_sint32_t		m_uiDefaultAudio;
	mpg_audio_info_t *	m_pszAudioInfo;
} mpg_dec_info_t;

typedef struct mpg_callback_func_t
{
	void* (*m_pMalloc		 ) ( av_ulong_t size);					//!< malloc
	void* (*m_pNonCacheMalloc) ( av_ulong_t size);					//!< non-cacheable malloc 
	void  (*m_pFree			 ) ( void* ptr);							//!< free
	void  (*m_pNonCacheFree	 ) ( void* ptr);							//!< non-cacheable free
	void* (*m_pMemcpy		 ) ( void* dst, const void* src, av_ulong_t num);//!< memcpy
	void  (*m_pMemset		 ) ( void* ptr, av_sint32_t value, av_ulong_t num);		//!< memset
	void* (*m_pRealloc		 ) ( void* ptr, av_ulong_t size);				//!< realloc
	void* (*m_pMemmove		 ) ( void* dst, const void* src, av_ulong_t num);//!< memmove

	void*		 (*m_pFopen	) (const char *filename, const char *mode);					//!< fopen
	av_ulong_t (*m_pFread		) (void* ptr, av_ulong_t size, av_ulong_t count, void* stream);		//!< fread
	av_ulong_t (*m_pCRCFread	) (void* ptr, av_ulong_t size, av_ulong_t count, void* stream);		//!< fread
	av_sint32_t	 (*m_pFseek	) (void* stream, av_long_t offset, av_sint32_t origin);						//!< fseek
	av_long_t	 (*m_pFtell	) (void* stream);										//!< ftell
	av_sint32_t	 (*m_pFseek64	) (void* stream, U64 offset, av_sint32_t origin);						//!< fseek
	U64		 (*m_pFtell64	) (void* stream);
	av_ulong_t (*m_pFwrite		) (const void* ptr, av_ulong_t size, av_ulong_t count, void* stream);	//!< fwrite
	av_sint32_t	 (*m_pFclose	) (void* stream);										//!< fclose
	av_sint32_t	 (*m_pUnlink	) (const char *pathname);									//!< _unlink
	av_ulong_t (*m_pFeof  		) (void* stream);											//!< feof
	av_ulong_t (*m_pFflush		) (void* stream);											//!< fflush

} mpg_callback_func_t;

//Structure for Initialization
typedef struct mpg_dec_init_t
{
	void *				m_pOpenFileName;//This field should be specified in cases of both Initialization stage and Refresh Stage
	mpg_callback_func_t		m_CallbackFunc;//Functions for Memory or File IO
	av_sint32_t				m_GetStreamMode;//Sequential Mode or Interleaved Mode
	av_sint32_t				m_FileCacheSize;//The limitation size of file cache(n case of interleave mode, parser consumes double of this size)
	av_sint32_t				m_ReturnDuration;//If this field is set, parser returns duration(In this case, initialization stage takes more time)
	av_uint32_t	m_ulFlags;//Sometimes FPS info in the sequence header is not correct. So calculate real fps by checking PTS between two sequence header
} mpg_dec_init_t;

typedef struct mpg_dec_input_t
{
	av_sint32_t				m_iSeekTimeMSec;// Seek Time(second) from Current Time
	av_uint32_t				m_iSeekMode;	// Seek mode(Reset, Forward, Backward)
	av_sint32_t				m_iSeekComponent; //Video only, Audio only, Both of them

	av_sint32_t				m_iGetStreamComponent;//This field is used in Interleaved mode.(Select Video or Audio)
	av_sint32_t				m_iInterestingCaptionIdx;//Through this field, user can choose caption.(Alternation is available)
} mpg_dec_input_t;

typedef struct mpg_caption_input_t
{
	av_uint8_t *			m_pData;
	av_uint32_t			m_iDataLength;
	av_uint32_t			m_bCropEnable;
} mpg_caption_input_t;

// FIXME : output
typedef struct mpg_caption_output_t
{	
	av_uint16_t *			m_pCaption;//pointer to Caption image buffer
	av_uint32_t			m_iCaptionPTS;

	av_uint32_t			m_iWidth;//width of Caption image buffer
	av_uint32_t			m_iHeight;//height of Caption image buffer

	av_uint32_t			m_iOffsetX;
	av_uint32_t			m_iOffsetY;

	av_uint32_t			m_iDisplayStart;//like PTS
	av_uint32_t			m_iDisplayEnd;//like PTS
} mpg_caption_output_t;

// FIXME : output
typedef struct mpg_dec_output_t
{
	av_uint8_t *			m_pData;//pointer to data
	av_sint32_t			m_iDataLength;

	av_sint32_t			m_iType;//Data Type(Video or Audio)
	av_uint32_t			m_iRefTime;//PTS.(sometimes this field could be '0' when there's no PTS info in packet)
	av_uint32_t			m_iLastSCR;
	av_uint32_t			m_iStreamIdx;
} mpg_dec_output_t;

typedef struct mpg_dec_sel_stream_t
{
	av_uint32_t	 m_ulMultipleDemuxing;	//!< [in] If it's set, demux multiple stream at the same time.
	av_uint32_t	 m_ulSelType;			//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	av_uint32_t	 m_ulVideoID;			//!< [in] Video ID (if m_ulSelType has DMXTYPE_VIDEO)
	av_uint32_t	 m_ulAudioID;			//!< [in] Audio ID (if m_ulSelType has DMXTYPE_AUDIO)
	av_uint32_t	 m_ulSubtitleID;		//!< [in] Subtitle ID (if m_ulSelType has DMXTYPE_SUBTITLE)
} mpg_dec_sel_stream_t;

//! Select stream - output
typedef struct mpg_dec_sel_info_t
{
	av_uint32_t		m_ulNumVideoStream;
	av_uint32_t		m_ulNumAudioStream;
	mpg_video_info_t *	m_pstVideoInfo[MPG_MAX_STREAM_NUMBER];	//!< [out] pointer to video information
	mpg_audio_info_t *	m_pstAudioInfo[MPG_MAX_STREAM_NUMBER];	//!< [out] pointer to audio information
} mpg_dec_sel_info_t;

/************************************************************************

	Common defines

************************************************************************/

/* Seek Mode */
#define SEEK_RESET				(0x00000000)
#define SEEK_DIR_FWD			(0x00000001)
#define SEEK_DIR_BWD			(0x00000002)

#define SEEK_BASE_BOTH	              	(0x00000000)
#define SEEK_BASE_VIDEO              	(0x00000010)
#define SEEK_BASE_AUDIO		(0x00000020)

#define SEEK_CHNK_DIR_FWD		(0x00000000)
#define SEEK_CHNK_DIR_BWD		(0x00000100)

#define SEEK_MODE_RELATIVE		(0x00000000)
#define SEEK_MODE_ABSOLUTE		(0x00001000)

#define SEEK_MODE_TIMESEEK		(0x00010000)

//Component Set
#define PACKET_NONE			(0)
#define PACKET_VIDEO			(1)
#define PACKET_AUDIO			(2)
#define PACKET_SUBTITLE			(4)
#define PACKET_PCI			(5)
#define PACKET_DSI			(6)
#define PACKET_NAVI			(7)

//Error Code
#define ERR_NONE			(0)
#define ERR_END_OF_FILE		(-1)
#define ERR_OUT_OF_MEMORY		(-2)
#define ERR_INVALID_DATA		(-3)
#define ERR_INVALID_PARAM		(-4)
#define ERR_NOT_SUPPORTED		(-5)
#define ERR_NEED_MORE_DATA		(-6)
#define ERR_BUFFER_FULL		(-7)

#define ERR_FILE_NOT_FOUND		(-8)
#define ERR_DEVICE_ERROR		(-10)
#define ERR_SYNCED			(-11)
#define ERR_DATA_NOT_FOUND		(-12)
#define ERR_MIME_NOT_FOUND		(-13)
#define ERR_NOT_DIRECTORY		(-14)
#define ERR_NOT_COMPATIBLE		(-15)
#define ERR_CONNECT_FAILED		(-16)
#define ERR_DROPPING			(-17)
#define ERR_STOPPED			(-18)
#define ERR_UNAUTHORIZED		(-19)
#define ERR_LOADING_HEADER		(-20)

#define ERR_STREAM_DOES_NOT_EXIST	(-21)

#define ERR_FILE_OPEN			(-30)
#define ERR_FILE_WRITE			(-31)
#define ERR_NEED_CHCAE_MEM		(-32)
#define ERR_FILE_SEEK			(-33)
#define ERR_FILE_READ			(-34)

/**
 * MPG MAIN API FUNCTION
 * [0]If successful, TCC_MKV_DEC returns 0 or plus. Otherwise, it returns a minus value.
 * [1]Op: decoder operation 
 * [2]pHandle: MPG's handle
 * [3]pParam1: input parameter(Refer to Test Code)
 * [4]pParam2: extension parameter(Refer to Test Code)
 */
mpg_result_t 
TCC_MPG_DEC(av_sint32_t Op, mpg_handle_t* pHandle, void* pParam1, void* pParam2);

#endif//MPG_H_*/
