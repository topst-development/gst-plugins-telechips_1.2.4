/****************************************************************************
 *   FileName    : mkv_demuxer.h
 *   Description :
 ****************************************************************************
 *
 *   TCC Version 1.3
 *   Copyright (c) Telechips Inc.
 *   All rights reserved

This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement	of	any	patent,	copyright	or	other	third	party	intellectual	property	right.	No	warranty	is	made,	express	or	implied,	regarding	the	information��s	accuracy,	completeness,	or	performance.
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code.
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef MKV_DEMUXER_H_
#define MKV_DEMUXER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "av_common_dmx.h"

#define MKV_HEADER_VER "V1.3"

//! MKV Demuxer OP Code
#define MKVDOP_OPEN_FROM_FILE_HANDLE    (3000) //!< opcode : this is special operation to open from file handle
#define MKVDOP_GET_AVC_INFO             (3001) //!< opcode : get sps/pps info
#define MKVDOP_CREATE_INDEX             (3002) //!< opcode : this means that when index table does not exist, we can create it.
                                               //             it doesn't work if MKVDOP_OPEN_FROM_FILE_HANDLE or MKVDF_DISABLE_SEEK_TO_END_OF_FILE is set.
#define MKVDOP_GET_HEVC_INFO            (3003) //!< opcode : get hevc seq.header info

#define MKVDOP_SET_OPTIONS              (4000)
#define MKVDOP_GET_VERSION              (4001) //!< opcode : if you want to get version info. of current demuxer, use this opcode.

/*!
 ============================================================
 *
 *  File/Video/Audio/Subtitle Information
 *      - information about video, audio or subtitle into one file.
 *
 ============================================================
*/
//! file information [out]
typedef struct mkv_dmx_file_info_t
{
    // common info
    av_string_t   *pszFileName;
    av_uint32_t    u32Duration;      // duration
    av_sint64_t    s64FileSize;      // filesize

    // mkv specific info
#define MKVDINFO_SEEKABLE   	(0x00000001u) //(1<<0)      // this means that demuxer is able to seek.
    av_uint32_t    u32InfoFlags; // Additional Output Information Flags
} mkv_dmx_file_info_t;

//! video stream information
typedef struct mkv_dmx_video_info_t
{
    // common info
    av_uint32_t    u32StreamID;
    av_uint32_t    u32Width;
    av_uint32_t    u32Height;
    av_uint32_t    u32FrameRate;
    av_uint32_t    u32FourCC;
    av_uint8_t    *pbyCodecPrivate;
    av_uint32_t    u32CodecPrivateSize;

    // mkv specific info
    av_uint32_t    u32LastKeyFrameTimeStamp;
    av_uint32_t    u32TimeStampType; // if it is set, this means that demuxer's timestamp is decoding order(DTS)
                                        // it is not used for MKV, mkv's default value is PTS(zero)(Presentation TimeStamp)
    av_uint32_t    u32StereoMode;
    // Stereo-3D video mode
    //  0: mono or unknown
    //  1: side by side (left eye is first)
    //  2: top-bottom (right eye is first)
    //  3: top-bottom (left eye is first)
    //  4: checkboard (right is first)
    //  5: checkboard (left is first)
    //  6: row interleaved (right is first)
    //  7: row interleaved (left is first)
    //  8: column interleaved (right is first)
    //  9: column interleaved (left is first)
    // 10: anaglyph (cyan/red)
    // 11: side by side (right eye is first)
    // 12: anaglyph (green/magenta)
    // 13: both eyes laced in one Block (left eye is first)
    // 14: both eyes laced in one Block (right eye is first))
} mkv_dmx_video_info_t;

//! audio stream information
typedef struct mkv_dmx_audio_info_t
{
    // common info
    av_uint32_t    u32StreamID;
    av_uint32_t    u32SamplePerSec;
    av_uint32_t    u32BitsPerSample;
    av_uint32_t    u32Channels;
    av_uint32_t    u32FormatTag;
    av_byte_t     *pbyCodecPrivate;
    av_uint32_t    u32CodecPrivateSize;
    av_string_t   *pszLanguage;

    // mkv specific info
    av_uint32_t    u32BlockAlign;        // for WMA 2011/01/20
    av_uint32_t    u32AvgBytesPerSec;    // for WMA 2011/01/20
} mkv_dmx_audio_info_t;

//! subtitle stream information
typedef struct mkv_dmx_subtitle_info_t
{
    // common info
    av_uint32_t    u32StreamID;
    av_string_t   *pszLanguage;

    // mkv specific info.
//http://www.matroska.org/technical/specs/subtitles/index.html
#define SUBTITLETYPE_SRT    (0u) // SRT subtitle
#define SUBTITLETYPE_SSA    (1u) // SSA subtitle
#define SUBTITLETYPE_ASS    (1u) // ASS (Advanced SSA)
#define SUBTITLETYPE_USF    (2u) // Universal Subtitle Format
#define SUBTITLETYPE_BMP    (3u) // BMP
#define SUBTITLETYPE_VOBSUB (4u) // VOBSUB
#define SUBTITLETYPE_KATE   (5u) // KATE
#define SUBTITLETYPE_PGS    (6u) // PGS
    av_uint32_t    u32SubtitleType;
    void          *pvSubtitleInfo;       // if exist
    av_uint32_t    u32SubtitleInfoSize;
} mkv_dmx_subtitle_info_t;

typedef av_dmx_handle_t mkv_dmx_handle_t;
typedef av_dmx_result_t mkv_dmx_result_t;
/*!
 ============================================================
 *
 *  Demuxer Initialize
 *
 ============================================================
*/
// these flags are used to set u32InitFlags.
#define MKVDF_DISABLE_SEEK_TO_END_OF_FILE	(0x00000001u) //(1<<0)	// It prevents the demuxer from seeking to end of file.
																//  therefore host cannot get some info. (duration, last key)
#define MKVDF_USE_FREAD_DIRECTLY			(0x00000002u) //(1<<1)	// If it is set, your callback fread function will be called directly without buffering.
#define MKVDF_DISABLE_AUDIO_STREAM		(0x00000004u) //(1<<2)	// If you don't need to get audio streams, set it.
#define MKVDF_DISABLE_VIDEO_STREAM		(0x00000008u) //(1<<3)	// If you don't need to get video streams, set it.
#define MKVDF_DISABLE_SUBTITLE_STREAM	(0x00000010u) //(1<<4)	// If you don't need to get subtitle streams, set it.

#define MKVDF_PARSE_ALL_STREAMS       (0x00000020u) //(1<<5)  // If you want to get all streams, set it.

//! options
typedef struct mkv_dmx_options_t
{
#define MKVDOPT_DISABLE_AUDIO_STREAM		(0x00000002u) //(1<<1)	// If you don't need to get audio streams, set it.
#define MKVDOPT_DISABLE_VIDEO_STREAM		(0x00000004u) //(1<<2)	// If you don't need to get video streams, set it.
#define MKVDOPT_DISABLE_SUBTITLE_STREAM	(0x00000008u) //(1<<3)	// If you don't need to get subtitle streams, set it.
    av_uint32_t u32OptGetStream;

// seek process between the last keyframe and the end of file.
#define MKVDOPT_DO_NOTHING                  (0) // default: keep current timestamp
#define MKVDOPT_TO_END_OF_FILE              (1) // go to the end of file.
#define MKVDOPT_LAST_KEYFRAME               (2) // find the last keyframe
    av_uint32_t u32OptSeekOverLastKey;
} mkv_dmx_options_t;

//! Demuxer Initialize - input
typedef struct mkv_dmx_init_t
{
    // common
    av_memory_func_t    stMemoryFuncs;           //!< [in] callback functions for memory
    av_file_func_t      stFileFuncs;             //!< [in] callback functions for file
    av_string_t        *pszOpenFileName;         /*!< [in] open file name( when Op is DMXOP_OPEN) or
                                                          handle of opened file( when Op is MKVDOP_OPEN_FROM_FILE_HANDLE) */
    // mkv specific info.
    av_uint32_t    u32InitFlags;             //!< [in] options or flags

    // cache settings
    av_uint32_t    u32VideoFileCacheSize;    //!< [in] video file cache(buffer) size( default:520833 bytes)
    av_uint32_t    u32AudioFileCacheSize;    //!< [in] audio file cache(buffer) size( default:520833 bytes)
    av_uint32_t    u32SeekFileCacheSize;     //!< [in] file cache for seek routine( default:1024 bytes)
} mkv_dmx_init_t;

//! Demuxer Initialize - output
typedef struct mkv_dmx_info_t
{
    // file info
    mkv_dmx_file_info_t     *pstFileInfo;             //!< pointer to file information

    // video stream info
    av_uint32_t              u32VideoStreamTotal;      //!< number of video stream
    mkv_dmx_video_info_t    *pstVideoInfoList;        //!< pointer to video information list ( m_lVideoStreamTotal x sizeof(pstVideoInfoList))
    mkv_dmx_video_info_t    *pstDefaultVideoInfo;     //!< pointer to default video information

    // audio stream info
    av_uint32_t              u32AudioStreamTotal;      //!< number of audio stream
    mkv_dmx_audio_info_t    *pstAudioInfoList;        //!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(pstAudioInfoList))
    mkv_dmx_audio_info_t    *pstDefaultAudioInfo;     //!< pointer to default audio information

    // subtitle stream info
    av_uint32_t              u32SubtitleStreamTotal;   //!< number of subtitle stream
    mkv_dmx_subtitle_info_t *pstSubtitleInfoList;     //!< pointer to subtitle information list ( m_lAudioStreamTotal x sizeof(pstSubtitleInfoList))
    mkv_dmx_subtitle_info_t *pstDefaultSubtitleInfo;  //!< pointer to default subtitle information
} mkv_dmx_info_t;

typedef struct mkv_dmx_metadata_t
{
    av_uint32_t       u32MetaTotal;
    av_metadata_t    *pstMetaList;
} mkv_dmx_metadata_t;

/*!
 ============================================================
 *
 *  Demuxer Running..
 *      - mkv has no a specific data
 *
 ============================================================
*/
//#define USE_AV_COMMON_DMX

//! Running - getstream
#ifdef USE_AV_COMMON_DMX
typedef av_dmx_getstream_t  mkv_dmx_getstream_t;
#else
typedef struct mkv_dmx_getstream_t
{
    av_byte_t     *pbyStreamBuff;        //!< [in] the pointer to getstream buffer
    av_uint32_t    u32StreamBuffSize;     //!< [in] the size of getstream buffer
    av_uint32_t    u32StreamType;         //!< [in] the type of getstream
    void          *pvSpecificData;        //!< [in] the pointer to specific input of demuxer(if demuxer has a specific data)
} mkv_dmx_getstream_t;
#endif

typedef struct mkv_dmx_out_specific_data_t
{
    av_uint32_t    u32StreamId;
    av_uint32_t    u32IsSyncSample;
} mkv_dmx_out_specific_data_t;

//! Running - outstream
#ifdef USE_AV_COMMON_DMX
typedef av_dmx_outstream_t  mkv_dmx_outstream_t;
#else
typedef struct mkv_dmx_outstream_t
{
    av_byte_t    *pbyStreamData;        //!< [out] the pointer to outstream data
    av_uint32_t   u32StreamDataSize;     //!< [out] the size to outstream data
    av_uint32_t   u32StreamType;         //!< [out] the type of outstream
    av_uint32_t   u32TimeStamp;          //!< [out] the timestamp of outstream
    av_uint32_t   u32EndTimeStamp;       //!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
    void         *pvSpecificData;        //!< [out] the pointer to specific output of demuxer(if demuxer has a specific data)
} mkv_dmx_outstream_t;
#endif

//! Running - seek
#define MKVDSEEK_TIME_BASED_GETFRAME (0x100u)      /* this means that demuxer seek to only desired time (does not find keyframe)
                                                    it would be a good selection for long keyframe intervals */
#ifdef USE_AV_COMMON_DMX
typedef av_dmx_seek_t       mkv_dmx_seek_t;
#else
typedef struct mkv_dmx_seek_t
{
    av_sint32_t    s32SeekTime;    //!< millisecond unit (Target Time)
    av_uint32_t    u32SeekMode;   //!< mode flags
} mkv_dmx_seek_t;
#endif

/*!
 ============================================================
 *
 *  Channel Selection
 *
 ============================================================
*/

//! Select stream - input
typedef struct mkv_dmx_sel_stream_t
{
    av_uint32_t    u32SelType;       //!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
    av_uint32_t    u32VideoID;       //!< [in] Video ID (if u32SelType has DMXTYPE_VIDEO)
    av_uint32_t    u32AudioID;       //!< [in] Audio ID (if u32SelType has DMXTYPE_AUDIO)
    av_uint32_t    u32SubtitleID;    //!< [in] Subtitle ID (if u32SelType has DMXTYPE_SUBTITLE)
} mkv_dmx_sel_stream_t;

//! Select stream - output
typedef struct mkv_dmx_sel_info_t
{
    mkv_dmx_video_info_t       *pstVideoInfo;    //!< [out] pointer to video information
    mkv_dmx_audio_info_t       *pstAudioInfo;    //!< [out] pointer to audio information
    mkv_dmx_subtitle_info_t    *pstSubtitleInfo; //!< [out] pointer to subtitle information
} mkv_dmx_sel_info_t;

/*!
 ============================================================
 *
 *  Get AvcC Information(for H.264)
 *
 ============================================================
*/
#ifdef MKVDOP_GET_AVC_INFO

//! sps/pps parameter set
typedef struct mkv_dmx_avc_parameter_set_t
{
    av_byte_t    *pbyData;
    av_uint32_t   u32DataLength;
} mkv_dmx_avc_parameter_set_t;

//! output interface, avcC info(demuxer) : AVCDecoderConfigurationRecord in avc file format(ISO/IEC 14496-15) : 28 bytes
typedef struct mkv_dmx_avcC_t
{
    av_uint32_t                     u32SpsNum;        //!< [out] numbers of sps
    mkv_dmx_avc_parameter_set_t    *pstSpsArray;     //!< [out] array of sps data
    av_uint32_t                     u32PpsNum;        //!< [out] numbers of pps
    mkv_dmx_avc_parameter_set_t    *pstPpsArray;     //!< [out] array of pps data
    av_uint32_t                     u32NalLenSize;    //!< [out] size of nal length
} mkv_dmx_avcC_t;

//! input interface
typedef struct mkv_dmx_getavc_t
{
    av_byte_t    *pbyCodecPrivate;
    av_uint32_t   u32CodecPrivateSize;
} mkv_dmx_getavc_t;

//! output interface
typedef struct mkv_dmx_outavc_t
{
    mkv_dmx_avcC_t    *pstAvcC;         //!< [out] pointer to avcC info
} mkv_dmx_outavc_t;

#endif//MKVDOP_GET_AVC_INFO

/*!
 ============================================================
 *
 *  Get HvcC Information(for HEVC/H.265)
 *
 ============================================================
*/
#ifdef MKVDOP_GET_HEVC_INFO

//! hevc parameter set
typedef struct mkv_dmx_hevc_data_t
{
    av_byte_t    *pbyData;
    av_uint32_t   u32DataLength;
} mkv_dmx_hevc_data_t;

typedef struct mkv_dmx_hevc_nal_unit_t
{
    av_uint32_t            u32NalUnitType;
    av_uint32_t            u32NalUnitNum;
    mkv_dmx_hevc_data_t   *pstNalUnits;
} mkv_dmx_hevc_nal_unit_t;

typedef struct mkv_dmx_hvcC_t
{
    av_uint32_t                 u32NalLengthSize;
    av_uint32_t                 u32NumOfArrays;
    mkv_dmx_hevc_nal_unit_t    *pstNalUnitArrays;
} mkv_dmx_hvcC_t;

//! input interface
typedef struct mkv_dmx_gethevc_t
{
    av_byte_t    *pbyCodecPrivate;
    av_uint32_t   u32CodecPrivateSize;
} mkv_dmx_gethevc_t;

//! output interface
typedef struct mkv_dmx_outhevc_t
{
    mkv_dmx_hvcC_t  *pstHvcC;         //!< [out] pointer to hvcC info
} mkv_dmx_outhevc_t;

#endif//MKVDOP_GET_AVC_INFO

/*!
 ============================================================
 *
 *  Get MPEG-4 VOL HEADER
 *
 ============================================================
*/
#if 1//def MKVDOP_GET_MPEG4_VOL_HEADER
typedef struct mkv_dmx_mpeg4_header_t
{
    av_uint32_t   u32Width;      // in
    av_uint32_t   u32Height;     // in
    av_byte_t    *pbyVop;       // in
    av_uint32_t   u32VopLength;  // in
    av_byte_t    *pbyVol;       // out
    av_uint32_t   u32VolLength;  // out
} mkv_dmx_mpeg4_header_t;
#endif

/*!
 ============================================================
 *
 *  Get Errors
 *
 ============================================================
*/
typedef struct mkv_dmx_error_t
{
    av_sint32_t     s32ErrCode;
    av_string_t    *pszErrStatus;
    av_sint64_t     s64LastStatus;
} mkv_dmx_error_t;

/*!
 ***********************************************************************
 * \brief
 *      TCC_MKV_DMX         : main api function of mkv demuxer library
 * \param
 *      [in]ulOpCode        : demuxer operation
 * \param
 *      [in,out]ptHandle    : handle of mkv demuxer library
 * \param
 *      [in]pParam1         : init or input parameter
 * \param
 *      [in,out]pParam2     : output or information parameter
 * \return
 *      If successful, TCC_MKV_DMX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
mkv_dmx_result_t
TCC_MKV_DMX(av_uint32_t ulOpCode, mkv_dmx_handle_t *ptHandle, av_param_t *pParam1, av_param_t *pParam2);


#ifdef __cplusplus
}
#endif
#endif/*MKV_DEMUXER_H_*/
