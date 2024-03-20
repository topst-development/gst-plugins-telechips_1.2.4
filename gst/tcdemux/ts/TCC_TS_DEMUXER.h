// SPDX-License-Identifier: LGPL-2.1-or later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef TCC_TS_DEMUXER_H__
#define TCC_TS_DEMUXER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "av_common_dmx.h"

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Definitions
//
// for tsd_selinfo_t.m_ulBuffChangeStatus / tsd_outstream_t.m_ulBuffChangeStatus
#define VIDEO_BUFF_CHANGED		(0x01)	//! video buffer changed with "tsd_selprog_t.m_pbyVideoBuffer" / "tsd_getstream_t.m_pbyNextVideoBuff"
#define AUDIO_BUFF_CHANGED		(0x02)	//! audio buffer changed with "tsd_selprog_t.m_pbyAudioBuffer" / "tsd_getstream_t.m_pbyNextAudioBuff"
#define GRAPHICS_BUFF_CHANGED	(0x04)	//! graphics buffer changed with "tsd_selprog_t.m_pbyGraphicsBuffer" / "tsd_getstream_t.m_pbyNextGraphicsBuff"
#define SUBTITLE_BUFF_CHANGED	(0x08)	//! subtitle buffer changed with "tsd_selprog_t.m_pbySubtitleBuffer" / "tsd_getstream_t.m_pbyNextSubtitleBuff"
#define PRIVATE_BUFF_CHANGED	(0x10)	//! private buffer changed with "tsd_selprog_t.m_pbySubtitleBuffer" / "tsd_getstream_t.m_pbyNextSubtitleBuff"



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Exported Types
//
//! API return typs
typedef	long	tsresult_t;
typedef	void*	tshandle_t;

//! TS Source type
#define SOURCE_TYPE_RTP		(0x0001)	//!< communication link
#define SOURCE_TYPE_FILE	(0x0002)	//!< file io

//! TS type
#define TS_FORMAT_UNKNOWN	(0x00000000)	
#define TS_FORMAT_NORMAL	(0x00000001)	//!< 188 byte packet
#define TS_FORMAT_M2TS		(0x00000002)	//!< 192 byte packet
#define TS_FORMAT_DVHS		(0x00000003)	//!< not used
#define TS_FORMAT_FEC204	(0x00000004)	//!< 204 byte packet
#define TS_FORMAT_FEC208	(0x00000005)	//!< 208 byte packet
#define TS_FORMAT_ONESEG	(0x00000100)	//!< no PAT
#define TS_FORMAT_SSIF		(0x00010000)	//!< SSIF

//! Stream Type
#define ES_TYPE_UNKNOWN		(0x0000)
#define ES_TYPE_VIDEO			(0x0001)
#define ES_TYPE_AUDIO			(0x0002)
#define ES_TYPE_GRAPHICS	(0x0010)
#define ES_TYPE_SUBTITLE	(0x0020)
#define ES_TYPE_PRIVATE		(0x0100)
#define ES_TYPE_BASE			(0x1000)
#define ES_TYPE_DEPENDENT	(0x2000)

//! graphic stream type
#define GRAPHICS_TYPE_UNKNOWN				(0x0000)
#define GRAPHICS_TYPE_PRESENTATION	(0x0001)
#define GRAPHICS_TYPE_INTERACTIVE		(0x0002)

//! chararcter encoding
#define CHARENC_UNKNOWN			(0x0000)
#define CHARENC_UNICODE20		(0x0001)			//!< Unicode 2.0 (UTF-8 and UTF-16BE)
#define CHARENC_SJIS				(0x0002)			//!< Shift-JIS
#define CHARENC_KSC5601			(0x0003)			//!< KSC 5601-1987 (including KSC 5653)
#define CHARENC_GB18030			(0x0004)			//!< GB18030-2000
#define CHARENC_GB2312			(0x0005)			//!< GB2312
#define CHARENC_BIG5				(0x0006)			//!< BIG5

//! telechips private type
#define TCCP_TYPE_GPSDATA		(0xE1)

//! secure type values
#define SECURE_TYPE_HDCP			(1)


//! memory management functions
typedef struct tsd_mem_funcs_t
{
	void* (*m_pfnMalloc			) ( av_uint32_t size);									//!< malloc
	void  (*m_pfnFree			) ( void* ptr);										//!< free
	av_sint32_t   (*m_pfnMemcmp	) ( const void* ptr1, const void* ptr2, av_uint32_t num);		//!< memcmp
	void* (*m_pfnMemcpy			) ( void* dst, const void* src, av_uint32_t num);				//!< memcpy
	void* (*m_pfnMemmove		) ( void* dst, const void* src, av_uint32_t num);				//!< memmove
	void* (*m_pfnMemset			) ( void* ptr, av_sint32_t size, av_uint32_t num);						//!< memset
	void* (*m_pfnRealloc		) ( void* ptr, av_uint32_t size);							//!< realloc
	av_sint32_t reserved[16-7];
} tsd_mem_funcs_t;

//! file management functions
typedef struct tsd_file_funcs_t
{
	void*		 (*m_pfnFopen	) ( const char* filename, const char* mode);						//!< fopen
	av_uint32_t (*m_pfnFread	) ( void* ptr, av_uint32_t size, av_uint32_t count, void* stream);		//!< fread
	av_sint32_t (*m_pfnFseek	) ( void* stream, av_long_t offset, av_sint32_t origin);								//!< fseek
	av_long_t	(*m_pfnFtell	) ( void* stream);										//!< ftell
	av_sint32_t (*m_pfnFclose	) ( void* stream);										//!< fclose
	av_uint32_t	 (*m_pfnFeof	) ( void* stream);										//!< feof
	av_uint32_t (*m_pfnFflush	) ( void* stream);										//!< fflush
	av_uint32_t (*m_pfnFwrite	) ( const void* ptr, av_uint32_t size, av_uint32_t count, void* stream);//!< fwrite (Muxer only)
	av_sint32_t (*m_pfnUnlink	) ( const char* pathname);									//!< _unlink (Muxer only)

	av_sint32_t (*m_pfnFseek64	) (void* stream, S64 offset, av_sint32_t origin);						//!< fseeko
	S64	    (*m_pfnFtell64	) (void* stream);										//!< ftello
	av_sint32_t reserved[16-11];
} tsd_file_funcs_t;

//! security functions
typedef struct tsd_secure_funcs_t
{
	av_sint32_t			(*m_pfnSecureProc)(av_ulong_t ulOpCode, void *param1, void *param2);
} tsd_secure_funcs_t;


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Exported Constants
//

//! BOOL type constant values
#ifndef TRUE
    #define TRUE (1 == 1)
#endif
#ifndef FALSE
    #define FALSE (!TRUE)
#endif

#ifndef NULL
	#define NULL (0)
#endif


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//
//	TS DeMuxer API
//
//! for all audio codec
#define AUDIO_SUBTYPE_NONE					(0x0000)

//! AAC Sub-type
#define AUDIO_SUBTYPE_AAC_ADTS				(1)
#define AUDIO_SUBTYPE_AAC_LATM				(2)

//! AC-3 Sub-type
#define AUDIO_SUBTYPE_AC3_LOSSLESS		(3)
#define AUDIO_SUBTYPE_AC3_PLUS				(4)
#define AUDIO_SUBTYPE_AC3_PLUS_S			(5)	//HDMV secondary audio

//! DTS Sub-type
#define AUDIO_SUBTYPE_DTS_HD_EXCEPT_XLL		(6)
#define AUDIO_SUBTYPE_DTS_HD_XLL			(7)
#define AUDIO_SUBTYPE_DTS_HD_LBR			(8)

//! DTS Sub-type
#define AUDIO_SUBTYPE_LPCM_HDMV				(9)
#define AUDIO_SUBTYPE_LPCM_DVD				(10)



/*!
============================================================
 *
 *	Structure definition about program information
 *
============================================================
*/
typedef struct tsd_videoinfo_t
{
	av_ulong_t			m_ulEsPID;				//!< video stream PID
	av_long_t			m_lWidth;				//!< frame width
	av_long_t			m_lHeight;				//!< frame height
	av_long_t			m_lFrameRate;			//!< frame rate
	av_long_t			m_lFourCC;				//!< video codec info. (four character code)
	av_ulong_t			m_bIsStereo;			//!< 0 or 1
	/* Dolby Vision bitstreams within the MPEG?2 transport stream format v1.1 */
	av_long_t   			m_lDolbyProfile;
	av_long_t   			m_lDolbyLevel;
	av_ulong_t		m_bDolbyRpuPresent;		//!< 1 when the Dolby Vision program carries the reference picture unit
	av_ulong_t	 	m_bDolbyBlPresent;		//!< 1 when the Dolby Vision program carries the base layer.
	av_ulong_t	 	m_bDolbyElPresent;		//!< 1 when the Dolby Vision program carries the enhancement layer
	av_ulong_t	    m_ulDolbyDependency_pid; //!< primary Dolby Vision program that carries the base layer. only valid when m_bDolbyElPresent value is 1
} tsd_videoinfo_t;

typedef struct tsd_audioinfo_t
{
	av_ulong_t	m_ulEsPID;				//!< audio stream PID
	av_ulong_t	m_ulFormatTag;			//!< audio code info. (format tag)
	av_ulong_t	m_ulSubType;			//!< audio code info. (sub-type information)
	av_long_t			m_lSamplePerSec;		//!< samples per sec
	av_long_t			m_lBitPerSample;		//!< bits per sample
	av_long_t			m_lBitRate;				//!< Kbps
	av_long_t			m_lChannels;			//!< the numbers of channel
	av_sint8_t			m_szLanguageCode[4];	//!< ISO 639 Language Code
} tsd_audioinfo_t;

typedef struct tsd_graphicsinfo_t
{
	av_ulong_t	m_ulEsPID;				//!< graphics stream PID
	av_ulong_t	m_ulGraphicsType;		//!< graphics type (PRESENTATION / INTERACTIVE)
	av_sint8_t	m_szLanguageCode[4];	//!< ISO 639 Language Code
} tsd_graphicsinfo_t;

typedef struct tsd_subtitleinfo_t
{
	av_ulong_t	m_ulEsPID;				//!< subtitle stream PID
	av_ulong_t	m_ulCharEnc;			//!< character encoding
	av_sint8_t	m_szLanguageCode[4];	//!< ISO 639 Language Code
} tsd_subtitleinfo_t;

typedef struct tsd_privateinfo_t
{
	av_ulong_t	m_ulEsPID;				//!< private stream PID
	av_ulong_t	m_ulPrivateType;
	av_long_t			m_lExtraInfoLength;
	av_uint8_t  *m_pbyExtraInfo;
} tsd_privateinfo_t;

typedef struct tsd_esinfo_t
{
	av_ulong_t			m_ulEsType;		//!< element stream type
	av_ulong_t			m_ulStreamType;	//!< stream type (ISO/IEC 13818-1)
	av_ulong_t			m_ulElementPID;	//!< element packet id
	union {
		tsd_videoinfo_t		stVideo;		//!< information of current video stream
		tsd_audioinfo_t		stAudio;		//!< information of current audio stream
		tsd_graphicsinfo_t	stGraphics;		//!< information of current graphic stream
		tsd_subtitleinfo_t	stSubtitle;		//!< information of current text subtitle stream
		tsd_privateinfo_t	stPrivate;		//!< information of current private stream
	} m_unInfo;
} tsd_esinfo_t;

typedef struct tsd_program_t
{
	av_ulong_t	m_ulProgramNum;			//!< program number (track ID)
	av_ulong_t	m_ulProgramMapPID;		//!< program map PID

	av_ulong_t	m_ulNbES;				//!< number of ES
	tsd_esinfo_t   *m_pstEsInfoList;		//!< ES information list

	av_long_t			m_lDuration;			//!< duration (milisecond)
} tsd_program_t;

typedef struct tsd_selinfo_t
{
	//! program information
	av_ulong_t		m_ulProgramNum;		//!< program number
	av_ulong_t		m_ulProgramMapPID;	//!< PID of PMT (internal info.)

	//! selected ES information
	tsd_videoinfo_t		m_stVideoInfo;		//!< Video ES Information
	tsd_audioinfo_t		m_stAudioInfo;		//!< Audio ES Information
	tsd_graphicsinfo_t	m_stGraphicsInfo;	//!< Graphics ES Information
	tsd_subtitleinfo_t	m_stSubtitleInfo;	//!< Subtitle ES Information
	tsd_privateinfo_t	m_stPrivateInfo;	//!< Private ES Information

	av_long_t				m_lDuration;		//!< duration (milisecond)
	av_ulong_t		m_ulSecureType;		//!< secure type

	//! used with "stream buffer" & "user buffer" mode
	av_ulong_t		m_ulBuffChangeStatus;	//! buffer changing status (VIDEO_BUFF_CHANGED | AUDIO_BUFF_CHANGED | .. )
} tsd_selinfo_t;


/*!
============================================================
 *
 *	Structure definition about secure information
 *
============================================================
*/
//! secure type values
#define SECURE_TYPE_HDCP			(1)

/* HDCP (for SECURE_TYPE_HDCP type) */
typedef struct hdcp_secure_info_t
{
	av_ulong_t	ulStreamCounter;
	U64	ullInputCounter;
} hdcp_secure_info_t;


/*!
============================================================
	ROUTINE: 
		TCC_TS_DMX

	DESCRIPTION:
		Main function of the TS DeMuxer API

	PARAMS:
		- opCode: operation code (defined below)
		- handle: TS DeMuxer handle
		- param1: operation parameter 1 (generally used as input)
		- param2: operation parameter 2 (generally used as output)

	RETURNS:
		- TRUE: Operation success
		- FALSE: Operation failed
		- ERROR_CODE: defined below
============================================================
*/
tsresult_t TCC_TS_DMX(av_ulong_t opCode, tshandle_t handle, void *param1, void *param2);

/* opCode constants */
// common
#define TSDOP_OPEN						(1)	
#define TSDOP_OPEN_MEDIAINFO	(2)
#define TSDOP_CLOSE						(3)	
#define TSDOP_SEL_PROG				(4)
#define TSDOP_GET_LAST_ERROR	(5)
#define TSDOP_SETUP_BUFFER		(100)	// for stream buffer mode only
#define TSDOP_DEMUX_CONFIG		(101)	// config after demuxer open
// for file source
#define TSDOP_GET_FRAME			(10)	// get frame
#define TSDOP_GET_STREAM		(11)	// get stream (If you set TSDOPT_STREAMBUFF option, then use this opcode.)
#define TSDOP_FRAME_SEEK		(12)	// frame seek
// for communication link source (currently not supported)
#define TSDOP_SCAN_PROG_LIST	(20)
#define TSDOP_GET_PROG_LIST		(21)	
#define TSDOP_SET_PROG_LIST		(22)	
#define TSDOP_PUSH_TS					(23)


/*!
============================================================
	DESC: DeMuxer Instance Open
============================================================
	PARAMS:
		- opCode:		TSDOP_OPEN
		- handle:		NULL
		- param1(in):	(tsd_init_t*)in
		- param2(out):	(tsd_info_t*)out

		<< file io source >>
		param1:	in->m_szPath = "example.ts"
				in->m_emSrcType = SRC_FILE;
				in->m_emTsType = TS_UNKNOWN / TS_NORMAL / TS_M2TS / TS_FEC;
				in->m_stMemFuncs = <memory management functions>;
				in->m_stFileFuncs = <file management functions>;
				in->m_stCallbacks = NULL;

		<< communication link source >>
		param1:	in->m_szPath = <not used>
				in->m_emSrcType = SRC_RTP;
				in->m_emTsType = TS_NORMAL / TS_M2TS / TS_FEC;
				in->m_stMemFuncs = <memory management functions>;
				in->m_stFileFuncs = NULL;
				in->m_stCallbacks = <Callback functions>
	
	RETURNS:
		- TRUE / FALSE
==========================================================
*/
//! ulOption
#define TSDOPT_DEFAULT								(0x00000000)	//!< default
#define TSDOPT_SELECTIVE							(0x00000001)	//!< Video/Audio Selective Read
#define TSDOPT_USERBUFF								(0x00000002)	//!< Use User buffer.
#define TSDOPT_MERGE_PES							(0x00000004)	//!< PESs which belongs to same PTS are merged
#define TSDOPT_PROGRESSIVE_FILE				(0x00000008)	//!< progressive file access
#define TSDOPT_STREAMBUFF							(0x00000010)	//!< stream buffer mode (only fill given buffer. no frame merging)
#define TSDOPT_FIND_ENCINFO						(0x00000020)	//!< packet decryption (for telechips encrypted stream)
#define TSDOPT_DO_NOT_CHECK_CRC				(0x00000040)	//!< Check section CRC
#define TSDOPT_DO_NOT_PARSE_HEADER		(0x00000080)	//!< Codec type will only be included in stream information.
#define TSDOPT_FAST_OPEN							(0x00000100)	//!< Some error detection processes are disabled.
#define TSDOPT_LEGACY_SEEK_PARAM_USED	(0x00000200)	//!<
#define TSDOPT_HDCP_SESSION_FORCED		(0x00000400)	//!< HDCP session enable without PMT parsing
#define TSDOPT_SSIF_REGION_SCAN				(0x00000800)	//!< pre-scan SSIF base/dependent region
#define TSDOPT_SKIP_PACKET_LOSS_PES		(0x00001000)	//!< skip PES have packet loss
#define TSDOPT_DISABLE_INPUT_CACHE		(0x00002000)
#define TSDOPT_START_FROM_VIDEO_PES		(0x00004000)
#define TSDOPT_USE_FIRST_PMT_ONLY			(0x01000000)
#define TSDOPT_DETECT_PATPMT_CHANGE		(0x02000000)
#define TSDOPT_CORE_STREAM_ONLY				(0x10000000)
#define TSDOPT_SUB_STREAM_ONLY				(0x20000000)

#define TSDOPT_DONT_CHECKCRC	(TSDOPT_DO_NOT_CHECK_CRC)

typedef struct tsd_init_t
{
	//! source information
	av_sint8_t     *m_pszOpenFileName;		//!< source path
	av_ulong_t	m_ulSrcType;			//!< TS source type (file source supported only)
	av_ulong_t	m_ulTsType;				//!< TS type (If this is TS_UNKNOWN, then find TS type.)
	av_ulong_t	m_ulOption;				//!< Demuxer options

	//! callback functions
	tsd_mem_funcs_t		m_stMemFuncs;			//!< Memory management functions
	tsd_file_funcs_t	m_stFileFuncs;			//!< File management functions
	tsd_secure_funcs_t	m_stSecureFuncs;		//!< Security functions for encrypted stream

	//! IO configuration
	av_ulong_t		m_ulFIOBlockSizeNormal;	//!< File read/write block size (0: default 6114)
	av_ulong_t		m_ulFIOBlockSizeSeek;	//!< File read/write block size (0: default 6114)

	//! used with TSDOPT_MERGE_PES flag (0: unlimited merge for same PTS)
	av_long_t				m_lMergeLimitBytes;

	//! used with TSDOPT_STREAMBUFF flag
	av_long_t				m_lVideoBuffLength;		//!< the length of video stream buffer		(512~1048576)
	av_long_t				m_lAudioBuffLength;		//!< the length of audio stream buffer		(512~65536)
	av_long_t				m_lGraphicsBuffLength;	//!< the length of graphics stream buffer	(512~65536)
	av_long_t				m_lSubtitleBuffLength;	//!< the length of subtitle stream buffer	(512~65536)
	av_long_t				m_lPrivateBuffLength;	//!< the length of private stream buffer	(512~65536)

	//! used with TSDOPT_STREAMBUFF | TSDOPT_USERBUFF flag
	av_uint8_t	   *m_pbyVideoBuffer;		//!< video stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t	   *m_pbyAudioBuffer;		//!< audio stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t	   *m_pbyGraphicsBuffer;	//!< graphics stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t	   *m_pbySubtitleBuffer;	//!< subtitle stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t	   *m_pbyPrivateBuffer;		//!< private stream buffer (set to NULL for not "user buffer mode")

	//! for extension
	void			   *m_pExtension;			//!< must be NULL (0)
} tsd_init_t;

typedef struct tsd_info_t
{
	tshandle_t		m_hTsd;						//!< TS DeMuxer Handle
	av_ulong_t	m_ulTsType;					//!< TS type (result)
	av_ulong_t	m_bIsSeekEnable;			//!< 0 or 1

	//! for SRC_FILE
	av_long_t			m_lProgramTotal;			//!< number of programs
	tsd_program_t  *m_pstProgramInfoList;		//!< program list

	//! selection information
	tsd_selinfo_t	m_stDefProgInfo;			//!< information about default program

	av_ulong_t 	m_bIsDolbyStream;			//!< 0 or 1
	av_long_t 	m_lDolbyProfile;			//!< Dolby Vision profile.
	av_long_t 	m_lDolbyLevel;				//!< Dolby Vision level
	av_ulong_t 	m_ulDolbyBlPID;
	av_ulong_t 	m_ulDolbyElPID;
	av_ulong_t 	m_ulDolbyRpuPID;
} tsd_info_t;


/*!
============================================================
	DESC: Select Program
============================================================
	PARAMS:
		- opCode:	TSDOP_SEL_PROG
		- handle:	<TS DeMuxer Handle>
		- param1:	(tsd_selprog_t*)in
		- param2:	(tsd_selinfo_t*)out
	RETURNS:
		- TRUE / FALSE
===========================================================
*/
typedef struct tsd_selprog_t
{
#define TSDSEL_PROGRAM			(0x10000000)
#define TSDSEL_VIDEO				(0x00000001)
#define TSDSEL_AUDIO				(0x00000002)
#define TSDSEL_GRAPHICS			(0x00000004)
#define TSDSEL_SUBTITLE			(0x00000008)
#define TSDSEL_PRIVATE			(0x00000010)
#define TSDSEL_NO_VIDEO			(0x00000100)
#define TSDSEL_NO_AUDIO			(0x00000200)
#define TSDSEL_NO_GRAPHICS	(0x00000400)
#define TSDSEL_NO_SUBTITLE	(0x00000800)
#define TSDSEL_NO_PRIVATE		(0x00001000)

	av_ulong_t	m_ulSelectType;			//!< TSDSEL_PROGRAM/TSDSEL_VIDEO/TSDSEL_AUDIO/...

	av_ulong_t	m_ulProgramNum;			//!< program number

	av_ulong_t	m_ulVideoPID;			//!< PID of video ES (0: select default stream)
	av_ulong_t	m_ulAudioPID;			//!< PID of audio ES (0: select default stream)
	av_ulong_t	m_ulGrapicsPID;			//!< PID of graphics ES (0: select default stream)
	av_ulong_t	m_ulSubtitlePID;		//!< PID of subtitle ES (0: select default stream)
	av_ulong_t	m_ulPrivatePID;			//!< PID of subtitle ES (0: select default stream)

	//! used with TSDOPT_STREAMBUFF | TSDOPT_USERBUFF flag
	av_uint8_t  *m_pbyVideoBuffer;		//!< video stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t  *m_pbyAudioBuffer;		//!< audio stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t  *m_pbyGraphicsBuffer;	//!< graphics stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t  *m_pbySubtitleBuffer;	//!< subtitle stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t  *m_pbyPrivateBuffer;		//!< private stream buffer (set to NULL for not "user buffer mode")
    av_ulong_t	m_ulMultipleDemuxing;	//!< [in] If it's set, demux multiple stream at the same time.
} tsd_selprog_t;

/*!
============================================================
	DESC: Get Frame (for File Source)
============================================================
	PARAMS:
		- opCode:	TSDOP_GET_FRAME
		- handle:	<TS DeMuxer Handle>
		- param1:	(tsd_getframe_t*)in
		- param2:	(tsd_outframe_t*)out
	RETURNS:
		- TRUE / FALSE
============================================================
*/
typedef struct tsd_getframe_t
{
	av_ulong_t	m_ulStreamType;		//!< stream type (used with "selective" mode)
	av_uint8_t  *m_pbyDataBuff;		//!< stream buffer
	av_long_t			m_lBuffLength;		//!< stream length
} tsd_getframe_t;

typedef struct tsd_outframe_t
{
	av_ulong_t	m_ulElementPID;     //!< stream PID 
	av_ulong_t	m_ulStreamType;		//!< stream type (video/audio/text/..)
	av_long_t			m_lDataLength;		//!< frame data length
	av_uint8_t  *m_pbyDataBuff;		//!< data buffer pointer
	av_long_t			m_lTimeStamp;		//!< presentation time stamp (milisecond)
	av_long_t			m_lPriority;		//!< 0: sub stream / 1: core stream
	av_long_t			m_lNewLength;		//!< new buffer length
	av_long_t			m_bPacketDiscont;	//!< packet loss detected

	av_ulong_t	m_ulSecureType;
	void           *m_pSecureInfo;
} tsd_outframe_t;



/*============================================================
	DESC: Get Stream (for File Source)
==============================================================
	PARAMS:
		- opCode:	TSDOP_GET_STREAM
		- handle:	<TS DeMuxer Handle>
		- param1:	(tsd_getstream_t*)in
		- param2:	(tsd_outstream_t*)out
	RETURNS:
		- TRUE / FALSE
============================================================*/
typedef struct tsd_getstream_t
{
	//! used with TSDOPT_SELECTIVE flag
	av_ulong_t	m_ulStreamType;			//! stream type (used with "selective" mode)

	//! used with TSDOPT_USERBUFF flag
	av_uint8_t	*m_pbyNextVideoBuff;	//! next video buffer (NULL: not change) 
	av_uint8_t	*m_pbyNextAudioBuff;	//! next audio buffer (NULL: not change) 
	av_uint8_t	*m_pbyNextGraphicsBuff;	//! next graphics buffer (NULL: not change)
	av_uint8_t	*m_pbyNextSubtitleBuff;	//! next subtitle buffer (NULL: not change)
	av_uint8_t	*m_pbyNextPrivateBuff;	//! next private buffer (NULL: not change)
} tsd_getstream_t;

typedef struct tsd_outstream_t
{
	av_ulong_t	m_ulStreamType;			//! stream type (used with "selective" mode)
	av_long_t			m_lDataLength;			//! frame data length
	av_uint8_t  *m_pbyDataBuff;			//! data buffer pointer
	av_long_t			m_lTimeStamp;			//! presentation time stamp (milisecond)
	av_long_t			m_lPriority;			//!< 0: sub stream / 1: core stream
	av_long_t			m_bPacketDiscont;		//!< packet loss detected

	//! used with TSDOPT_USERBUFF flag
	av_ulong_t	m_ulBuffChangeStatus;	//! buffer changing status (VIDEO_BUFF_CHANGED | AUDIO_BUFF_CHANGED | .. )
} tsd_outstream_t;


/*!
============================================================
	DESC: Frame Seek (for File Source)
============================================================
	PARAMS:
		- opCode:	TSDOP_FRAME_SEEK
		- handle:	<TS DeMuxer Handle>
		- param1:	(av_ulong_t* or tsd_seek_t)in	// seek time (milisecond)
		- param2:	(av_ulong_t* or tsd_seek_t)out	// result time (milisecond)
	RETURNS:
		- TRUE / FALSE
============================================================
*/
#define TSDSEEK_DEFAULT				(TSDSEEK_SYNC|TSDSEEK_TIME_AUTO)
#define TSDSEEK_SYNC						(0x00000001u)	// find sync-frame
#define TSDSEEK_TIME_BWD				(0x00000010u)	// find stream that has time less than or equal to target time
#define TSDSEEK_TIME_FWD				(0x00000020u)	// find stream that has time bigger than or equal to target time
#define TSDSEEK_TIME_AUTO				(0x00000030u)	// find stream that has less/bigger or equal time which will be decided by considering current/target time.
#define TSDSEEK_FAST_METHOD			(0x00000100u)	// less accurate seek, TSDSEEK_TIME_BWD/TSDSEEK_TIME_FWD flags are ignoring.
#define TSDSEEK_GO_EOS_IF_EOS		(0x00010000)	// Set demuxing position to EOS when reached EOS as a result of seek.
#define TSDSEEK_GO_LSYNC_IF_EOS	(0x00020000)	// Set demuxing position to last sync-frame when reached EOS as a result of seek.
#define TSDSEEK_EXTERNAL				(0x10000000u)	// for progressive file condition

typedef struct tsd_seek_t {
	av_long_t			m_lSeekTime;	// target seek time (milisecond)
	av_ulong_t	m_ulFlags;		//
} tsd_seek_t;


/*!
============================================================
	DESC: Scan Program List
============================================================
	PARAMS:
		- opCode:	TSDOP_SCAN_PROG_LIST
		- handle:	<TS DeMuxer Handle>
		- param1:	NULL
		- param2:	(tsd_proglist_t*)out
	RETURNS:
		- TRUE / FALSE
============================================================
*/
/*!
============================================================
	DESC: Set Program List
============================================================
	PARAMS:
		- opCode:	TSDOP_SET_PROG_LIST
		- handle:	<TS DeMuxer Handle>
		- param1:	(tsd_proglist_t*)in
		- param2:	NULL
	RETURNS:
		- TRUE / FALSE
============================================================
*/
/*!
============================================================
	DESC: Get Program List
============================================================
	PARAMS:
		- opCode:	TSDOP_GET_PROG_LIST
		- handle:	<TS DeMuxer Handle>
		- param1:	NULL
		- param2:	(tsd_proglist_t*)out
	RETURNS:
		- TRUE / FALSE
============================================================
*/
typedef struct tsd_proglist_t
{
	av_long_t			m_lProgramTotal;			//!< number of programs
	tsd_program_t  *m_pstProgramInfoList;		//!< program list
} tsd_proglist_t;


/*!
============================================================
	DESC: Push TS Data (for Communication Link Source)
============================================================
	PARAMS:
		- opCode:	TSDOP_PUSH_TS
		- handle:	<TS DeMuxer Handle>
		- param1:	(tsd_pushts_t*)in
		- param2:	NULL
	RETURNS:
		- TRUE / FALSE
============================================================
*/
typedef struct tsd_pushts_t
{
	av_long_t			m_lDataLength;		//!< data length
	av_uint8_t	*m_pbyDataBuff;		//!< data buffer pointer
} tsd_pushts_t;


/*!
============================================================
	DESC: Set first stream buffer (before first get_frame op)
============================================================
	PARAMS:
		- opCode:	TSDOP_SETUP_BUFFER
		- handle:	<TS DeMuxer Handle>
		- param1:	(tsd_set_buffer_t*)in
		- param2:	NULL
	RETURNS:
		- TRUE / FALSE
============================================================
*/
typedef struct tsd_setup_buffer_t {
	//! used with TSDOPT_STREAMBUFF flag
	av_long_t				m_lVideoBuffLength;		//!< the length of video stream buffer		(512~1048576)
	av_long_t				m_lAudioBuffLength;		//!< the length of audio stream buffer		(512~65536)
	av_long_t				m_lGraphicsBuffLength;	//!< the length of graphics stream buffer	(512~65536)
	av_long_t				m_lSubtitleBuffLength;	//!< the length of subtitle stream buffer	(512~65536)
	av_long_t				m_lPrivateBuffLength;	//!< the length of private stream buffer	(512~65536)

	//! used with TSDOPT_STREAMBUFF | TSDOPT_USERBUFF flag
	av_uint8_t	   *m_pbyVideoBuffer;		//!< video stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t	   *m_pbyAudioBuffer;		//!< audio stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t	   *m_pbyGraphicsBuffer;	//!< graphics stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t	   *m_pbySubtitleBuffer;	//!< subtitle stream buffer (set to NULL for not "user buffer mode")
	av_uint8_t	   *m_pbyPrivateBuffer;		//!< private stream buffer (set to NULL for not "user buffer mode")
} tsd_setup_buffer_t;


/*!
============================================================
	DESC: Set first stream buffer ()
============================================================
	PARAMS:
		- opCode:	TSDOP_DEMUX_CONFIG
		- handle:	<TS DeMuxer Handle>
		- param1:	(tsd_demux_config_t*)in
		- param2:	NULL
	RETURNS:
		- TRUE / FALSE
============================================================
*/
typedef struct tsd_demux_config_t {
	av_ulong_t	m_ulOption;	// TSDOPT_CORE_STREAM_ONLY / TSDOPT_SUB_STREAM_ONLY
} tsd_demux_config_t;


/*!
============================================================
	Usage - Get Last Error
============================================================
	PARAMS:
		- opCode:	TSDOP_GET_LAST_ERROR
		- handle:	<TS DeMuxer Handle>
		- param1:	NULL
		- param2:	NULL
	RETURNS:
		- ERROR_CODE:	defined below
============================================================
*/
/*!
============================================================
	ERROR_CODE constants
============================================================
*/
#define TSDERR_NONE								(0)

//! system error
#define TSDERR_BASE_SYSTEM_ERROR				(0)
#define TSDERR_SYSTEM_ERROR						(TSDERR_BASE_SYSTEM_ERROR - 1 )
#define TSDERR_PROGRAM_ALLOCATION_FAILED		(TSDERR_BASE_SYSTEM_ERROR - 2 )
#define TSDERR_PES_BUFFER_ALLOCATION_FAILED		(TSDERR_BASE_SYSTEM_ERROR - 3 )
#define TSDERR_PES_BUFFER_REALLOCATION_FAILED	(TSDERR_BASE_SYSTEM_ERROR - 4 )
#define TSDERR_FRAME_BUFFER_REALLOCATION_FAILED	(TSDERR_BASE_SYSTEM_ERROR - 5 )
#define TSDERR_SECTION_FILTER_ALLOCATION_FAILED	(TSDERR_BASE_SYSTEM_ERROR - 6 )
#define TSDERR_PES_FILTER_ALLOCATION_FAILED		(TSDERR_BASE_SYSTEM_ERROR - 7 )
#define TSDERR_DEMUXER_ALLOCATION_FAILED		(TSDERR_BASE_SYSTEM_ERROR - 8 )
#define TSDERR_PACKETBUFF_ALLOCATION_FAILED		(TSDERR_BASE_SYSTEM_ERROR - 9 )
#define TSDERR_INFOLIST_ALLOCATION_FAILED		(TSDERR_BASE_SYSTEM_ERROR - 10)
#define TSDERR_PAT_ALLOCATION_FAILED			(TSDERR_BASE_SYSTEM_ERROR - 11)
#define TSDERR_PMT_ALLOCATION_FAILED			(TSDERR_BASE_SYSTEM_ERROR - 12)
#define TSDERR_PRIVATE_INFO_ALLOCATION_FAILED	(TSDERR_BASE_SYSTEM_ERROR - 13)
#define TSDERR_REGION_MAP_ALLOCATION_FAILED		(TSDERR_BASE_SYSTEM_ERROR - 14)
#define TSDERR_FILE0_OPEN_FAILED				(TSDERR_BASE_SYSTEM_ERROR - 15)
#define TSDERR_FILE12_OPEN_FAILED				(TSDERR_BASE_SYSTEM_ERROR - 16)
#define TSDERR_FILE_SEEK_FAILED					(TSDERR_BASE_SYSTEM_ERROR - 17)
#define TSDERR_INVALID_SYSTEM					(TSDERR_BASE_SYSTEM_ERROR - 18)

//! broken file error
#define TSDERR_BASE_BROKEN_FILE					(-1000)
#define TSDERR_BROKEN_FILE						(TSDERR_BASE_BROKEN_FILE - 0 )
#define TSDERR_INVALID_SECTION_LENGTH			(TSDERR_BASE_BROKEN_FILE - 1 )
#define TSDERR_INVALID_PAT_SECTION_LENGTH		(TSDERR_BASE_BROKEN_FILE - 2 )
#define TSDERR_INVALID_PMT_SECTION_LENGTH		(TSDERR_BASE_BROKEN_FILE - 3 )
#define TSDERR_SECTION_NOT_STARTED				(TSDERR_BASE_BROKEN_FILE - 4 )
#define TSDERR_INVALID_PROGRAM_INFO_LENGTH		(TSDERR_BASE_BROKEN_FILE - 5 )
#define TSDERR_INVALID_ES_INFO_LENGTH			(TSDERR_BASE_BROKEN_FILE - 6 )
#define TSDERR_TS_TYPE_ANALIZATION_FAILED		(TSDERR_BASE_BROKEN_FILE - 7 )
#define TSDERR_PROGRAM_LIST_NOT_FOUND			(TSDERR_BASE_BROKEN_FILE - 8 )
#define TSDERR_SECTION_CRC_FAILED				(TSDERR_BASE_BROKEN_FILE - 9 )
#define TSDERR_PROGRAM_NOT_EXIST				(TSDERR_BASE_BROKEN_FILE - 10)
#define TSDERR_SYNCWORD_NOT_FOUND				(TSDERR_BASE_BROKEN_FILE - 11)
#define TSDERR_SUPPORTED_ES_NOT_EXIST			(TSDERR_BASE_BROKEN_FILE - 12)
#define TSDERR_ES_INFO_NOT_FOUND				(TSDERR_BASE_BROKEN_FILE - 13)
#define TSDERR_INVALID_ADAPTATION_FIELD_LENGTH	(TSDERR_BASE_BROKEN_FILE - 14)
#define TSDERR_INVALID_TABLE_ID					(TSDERR_BASE_BROKEN_FILE - 15)
#define TSDERR_INVALID_ES_TYPE					(TSDERR_BASE_BROKEN_FILE - 16)
#define TSDERR_INVALID_ES_LENGTH				(TSDERR_BASE_BROKEN_FILE - 17)
#define TSDERR_PTS_IS_DAMAGED					(TSDERR_BASE_BROKEN_FILE - 18)

//! seek failed error
#define TSDERR_BASE_SEEK_FAILED					(-2000)
#define TSDERR_SEEK_FAILED						(TSDERR_BASE_SEEK_FAILED - 0)
#define TSDERR_SEEK_FAILED_EOF					(TSDERR_BASE_SEEK_FAILED - 1)
#define TSDERR_STREAM_NOT_EXIST					(TSDERR_BASE_SEEK_FAILED - 2)

//! unsupported format
#define TSDERR_BASE_UNSUPPORTED_FORMAT			(-3000)
#define TSDERR_UNSUPPORTED_FORMAT				(TSDERR_BASE_UNSUPPORTED_FORMAT - 0)
#define TSDERR_UNSUPPORTED_VIDEO_FORMAT			(TSDERR_BASE_UNSUPPORTED_FORMAT - 1)
#define TSDERR_UNSUPPORTED_AUDIO_FORMAT			(TSDERR_BASE_UNSUPPORTED_FORMAT - 2)
#define TSDERR_UNKNOWN_VIDEO_HEADER				(TSDERR_BASE_UNSUPPORTED_FORMAT - 3)
#define TSDERR_UNKNOWN_AUDIO_HEADER				(TSDERR_BASE_UNSUPPORTED_FORMAT - 4)
#define TSDERR_SCRAMBLING_CONTROL_NOT_SUPPORTED	(TSDERR_BASE_UNSUPPORTED_FORMAT - 5)

//! invalid parameter error
#define TSDERR_BASE_INVALID_FUNC_PARAM			(-4000)
#define TSDERR_INVALID_FUNC_PARAM				(TSDERR_BASE_INVALID_FUNC_PARAM - 0 )
#define TSDERR_DATA_NOT_ENOUGH					(TSDERR_BASE_INVALID_FUNC_PARAM - 1 )
#define TSDERR_FILE_SIZE_NOT_ENOUGH				(TSDERR_BASE_INVALID_FUNC_PARAM - 2 )
#define TSDERR_INVALID_PROGRAM_INDEX			(TSDERR_BASE_INVALID_FUNC_PARAM - 3 )
#define TSDERR_TS_TYPE_NOT_INDICATED			(TSDERR_BASE_INVALID_FUNC_PARAM - 4 )
#define TSDERR_THIS_OP_ON_RTP_NOT_SUPPORTED		(TSDERR_BASE_INVALID_FUNC_PARAM - 5 )
#define TSDERR_THIS_OP_ON_FILE_NOT_SUPPORTED		(TSDERR_BASE_INVALID_FUNC_PARAM - 6 )
#define TSDERR_VIDEO_ES_NOT_EXIST				(TSDERR_BASE_INVALID_FUNC_PARAM - 7 )
#define TSDERR_AUDIO_ES_NOT_EXIST				(TSDERR_BASE_INVALID_FUNC_PARAM - 8 )
#define TSDERR_GRAPHICS_ES_NOT_EXIST			(TSDERR_BASE_INVALID_FUNC_PARAM - 9 )
#define TSDERR_SUBTITLE_ES_NOT_EXIST			(TSDERR_BASE_INVALID_FUNC_PARAM - 10)
#define TSDERR_PRIVATE_ES_NOT_EXIST				(TSDERR_BASE_INVALID_FUNC_PARAM - 11)
#define TSDERR_INVALID_OP_CODE					(TSDERR_BASE_INVALID_FUNC_PARAM - 12)
#define TSDERR_INVALID_STREAM_TYPE				(TSDERR_BASE_INVALID_FUNC_PARAM - 13)
#define TSDERR_USER_BUFFER_NOT_ENOUGH			(TSDERR_BASE_INVALID_FUNC_PARAM - 14)
#define TSDERR_THIS_OP_NOT_SUPPORTED_YET		(TSDERR_BASE_INVALID_FUNC_PARAM - 15)
#define TSDERR_INVALID_PROGRAM_NUMBER			(TSDERR_BASE_INVALID_FUNC_PARAM - 16)
#define TSDERR_INVALID_VIDEO_ID					(TSDERR_BASE_INVALID_FUNC_PARAM - 20)
#define TSDERR_INVALID_AUDIO_ID					(TSDERR_BASE_INVALID_FUNC_PARAM - 21)
#define TSDERR_INVALID_GRAPHICS_ID				(TSDERR_BASE_INVALID_FUNC_PARAM - 22)
#define TSDERR_INVALID_SUBTITLE_ID				(TSDERR_BASE_INVALID_FUNC_PARAM - 23)
#define TSDERR_INVALID_PRIVATE_ID				(TSDERR_BASE_INVALID_FUNC_PARAM - 24)
#define TSDERR_PROGRESSIVE_FILE_ACCESS_MODE		(TSDERR_BASE_INVALID_FUNC_PARAM - 25)
#define TSDERR_INVALID_DEMUXER_HANDLE			(TSDERR_BASE_INVALID_FUNC_PARAM - 26)
#define TSDERR_INVALID_DECRYPTION_LENGTH		(TSDERR_BASE_INVALID_FUNC_PARAM - 27)
#define TSDERR_CURRENTLY_NOT_SUPPORTED			(TSDERR_BASE_INVALID_FUNC_PARAM - 30)
#define TSDERR_STREAM_BUFFER_FLAG_NOT_USED		(TSDERR_BASE_INVALID_FUNC_PARAM - 31)
#define TSDERR_OPENED_FOR_MEDIAINFO				(TSDERR_BASE_INVALID_FUNC_PARAM - 32)
#define TSDERR_SECURE_PROCESSOR_REQUIRED		(TSDERR_BASE_INVALID_FUNC_PARAM - 33)
#define TSDERR_INVALID_OPTION_FLAGS				(TSDERR_BASE_INVALID_FUNC_PARAM - 34)

//! demuxer internal error
#define TSDERR_BASE_DEMUXER_INTERNAL			(-10000)
#define TSDERR_DEMUXER_INTERNAL					(TSDERR_BASE_DEMUXER_INTERNAL - 0)
#define TSDERR_PROGRAM_NOT_ALLOCATED			(TSDERR_BASE_DEMUXER_INTERNAL - 1)
#define TSDERR_PROGRAM_ALLOC_NOT_ENOUGH			(TSDERR_BASE_DEMUXER_INTERNAL - 2)
#define TSDERR_FILTER_NOT_ALLOCATED				(TSDERR_BASE_DEMUXER_INTERNAL - 3)
#define TSDERR_INVALID_VIDEO_INDEX				(TSDERR_BASE_DEMUXER_INTERNAL - 4)
#define TSDERR_INVALID_AUDIO_INDEX				(TSDERR_BASE_DEMUXER_INTERNAL - 5)
#define TSDERR_INVALID_GRAPHICS_INDEX			(TSDERR_BASE_DEMUXER_INTERNAL - 6)
#define TSDERR_INVALID_SUBTITLE_INDEX			(TSDERR_BASE_DEMUXER_INTERNAL - 7)
#define TSDERR_INVALID_PRIVATE_INDEX			(TSDERR_BASE_DEMUXER_INTERNAL - 8)
#define TSDERR_END_OF_STREAM					(TSDERR_BASE_DEMUXER_INTERNAL - 9)
#define TSDERR_PATPMT_UPDATED					(TSDERR_BASE_DEMUXER_INTERNAL -10)


#define IS_SYSERROR(code)							(    0 >  (code) && (code) > -1000)
#define IS_FILEERROR(code)						(-1000 >= (code) && (code) > -2000)
#define IS_SEEKERROR(code)						(-2000 >= (code) && (code) > -3000)
#define IS_FORMATERROR(code)					(-3000 >= (code) && (code) > -4000)
#define IS_PARAMERROR(code)						(-4000 >= (code) && (code) > -5000)
#define IS_INTERNALERROR(code)				(-10000 >= (code))


#ifdef __cplusplus
}
#endif
#endif //TCC_TS_DEMUXER_H__

