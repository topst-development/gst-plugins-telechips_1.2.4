// SPDX-License-Identifier: LGPL-2.1-or later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef TCC_TS_DEMUXER_EXT_H__
#define TCC_TS_DEMUXER_EXT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WCE)
typedef	signed __int64		tsd_s64_t;
typedef	unsigned __int64	tsd_u64_t;
#else
typedef	signed long long	tsd_s64_t;
typedef	unsigned long long	tsd_u64_t;
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Extensions
//

/*
	Tag of the extension fields
*/
typedef enum TSDEXT_TAGS {
	TSDEXT_TAG_CONFIG_SCAN_ENDPTS,
	TSDEXT_TAG_CONFIG_SCAN_ES_INFO,
	TSDEXT_TAG_CONFIG_SCAN_ES_INFO2,
	TSDEXT_TAG_REGISTER_ES_INFO,
	TSDEXT_TAG_SET_FILE_INFO,
	TSDEXT_TAG_CONFIG_PTS_RESET_CRITERIA,
	TSDEXT_TAG_MAX
};

/*
	extension field header
*/
typedef struct tsdext_header_t
{
	unsigned long	ulExtTag;			// TSDEXT_ID_CONFIG_SCAN_ENDPTS
	long			lExtSize;			// sizeof(tsdext_header_t) + ??
	void		   *pNextField;			// pointer of next header
} tsdext_header_t;


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Extension Fields
//
typedef struct tsdext_config_scan_endpts_t
{
	unsigned long	ulExtTag;			// TSDEXT_TAG_CONFIG_SCAN_ENDPTS
	long			lExtSize;			// sizeof(tsdext_config_scan_endpts_t)
	void		   *pNextField;			// point to next header

	tsd_s64_t		llStartOffset;		// offset from end of file (positive value)
	tsd_s64_t		llMaxOffset;		// offset from end of file (positive value)
	tsd_s64_t		llStepSize;			// step size of backward file seek
} tsdext_config_scan_endpts_t;

typedef struct tsdext_config_scan_es_info_t
{
	unsigned long	ulExtTag;			// TSDEXT_TAG_CONFIG_SCAN_ES_INFO
	long			lExtSize;			// sizeof(tsdext_config_scan_es_info_t)
	void		   *pNextField;			// point to next header

	tsd_s64_t		llMaxOffset;		// max scan offset (0: default 30*1024*1024)
} tsdext_config_scan_es_info_t;

typedef struct tsdext_config_scan_es_info2_t
{
	unsigned long	ulExtTag;			// TSDEXT_TAG_CONFIG_SCAN_ES_INFO2
	long			lExtSize;			// sizeof(tsdext_config_scan_es_info2_t)
	void		   *pNextField;			// point to next header

	tsd_s64_t		llMaxOffset;		// max scan offset (0: default 30*1024*1024)
	tsd_s64_t		llPacketScanSize;	// packet scanning size after finding at least each one audio and video stream header. (0: not applied)
										// It is about specific condition that PMT has many A/V stream informations which is not exist in real.

	long			lProgramCount;		// criteria of init success (-1: find all)
	long			lVideoCount;		// criteria of init success (-1: find all)
	long			lAudioCount;		// criteria of init success (-1: find all)

	long			bSimpleVideoScan;	// TRUE / FALSE (default)
	long			bSimpleAudioScan;	// TRUE / FALSE (default)
} tsdext_config_scan_es_info2_t;

typedef struct tsdext_register_es_info_t
{
	unsigned long	ulExtTag;			// TSDEXT_TAG_REGISTER_ES_INFO
	long			lExtSize;			// sizeof(tsdext_register_es_info_t)
	void		   *pNextField;			// point to next header

	unsigned long	ulStandard;
	unsigned long	ulStreamType;		// stream type (ISO/IEC 13818-1)

	unsigned long	ulProgramNumber;	// program number
	unsigned long	ulElementPID;		// element packet id

	long			lDuration;			// duration of element stream

	union {
		tsd_videoinfo_t		stVideo;			// information of video stream (width, height, frame-rate are used)
		tsd_audioinfo_t		stAudio;			// information of audio stream (sampling-per-second, bit-per-sample, bit-rate, language-code are used)
		tsd_graphicsinfo_t	stGraphics;			// information of graphic stream
		tsd_subtitleinfo_t	stSubtitle;			// information of text subtitle stream
		tsd_privateinfo_t	stPrivate;			// information of private stream
	} unInfo;
} tsdext_register_es_info_t;

typedef struct tsdext_set_file_info_t
{
	unsigned long	ulExtTag;			// TSDEXT_TAG_SET_FILE_INFO
	long			lExtSize;			// sizeof(tsdext_set_file_info_t)
	void		   *pNextField;			// point to next header

	tsd_s64_t		llFileSize;
	long			lDuration;
} tsdext_set_file_info_t;

typedef struct tsdext_config_pts_reset_criteria_t
{
	unsigned long	ulExtTag;			// TSDEXT_TAG_CONFIG_PTS_RESET_CRITERIA
	long			lExtSize;			// sizeof(tsdext_set_file_info_t)
	void		   *pNextField;			// point to next header

	tsd_s64_t		llMaxDiff;			// 90kHz unit
	long			lMaxDiffMs;			// milisecond unit
} tsdext_config_pts_reset_criteria_t;

#ifdef __cplusplus
}
#endif
#endif //TCC_TS_DEMUXER_EXT_H__
