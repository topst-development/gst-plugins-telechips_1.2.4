/*
 *   FileName : av_common_dmx.h
 *   Description :
 *
 *   TCC Version 2.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 *
 * This source code contains confidential information of Telechips.
 * Any unauthorized use without a written permission of Telechips including
 * not limited to re-distribution in source or binary form is strictly prohibited.
 * This source code is provided "AS IS" and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind,
 * including without limitation, any warranty of merchantability,
 * fitness for a particular purpose or non-infringement of any patent,
 * copyright or other third party intellectual property right.
 * No warranty is made, express or implied, regarding the informations accuracy,
 * completeness, or performance.
 * In no event shall Telechips be liable for any claim, damages or other liability
 * arising from, out of or in connection with this source code or the use
 * in the source code.
 * This source code is provided subject to the terms of a Mutual Non-Disclosure
 * Agreement between Telechips and Company.
 *
 */


#ifndef AV_COMMON_DMX_H_
#define AV_COMMON_DMX_H_

#include "av_common_fcc_tag.h"
#include "av_common_types.h"

typedef av_handle_t av_dmx_handle_t;    //!< The type of a demuxer's handle
typedef av_result_t av_dmx_result_t;    //!< The type of a demuxer's result

/*
   ============================================================
 *
 *  Constant Definition
 *
 ============================================================
 */

// Demuxer OP Code
#define DMXOP_OPEN                    (0) //!< demuxer open (initialize)
#define DMXOP_CLOSE                   (1) //!< demuxer close
#define DMXOP_GET_STREAM              (2) //!< get stream
#define DMXOP_SEEK                    (3) //!< seek stream
#define DMXOP_SELECT_STREAM           (4) //!< select stream(for multistream)
#define DMXOP_GET_LASTERROR           (5) //!< get last error
#define DMXOP_GET_METADATA            (6) //!< get meta data
#define DMXOP_OPEN_FOR_MEDIASCAN      (7) //!< get media info quickly
#define DMXOP_PRESEEK                 (9) //!< TCC8010 only

// Demuxer Return Value
// 0 ~ 999

#define DMXRET_SUCCESS               (0) //!< success(0 or plus)
#define DMXRET_FAILED               (-1) //!< general failed code
#define DMXRET_END_OF_STREAM        (-2) //!< the end of stream
#define DMXRET_INVALID_HANDLE       (-3) //!< invalid handle
#define DMXRET_FILE_OPEN_FAILED     (-4) //!< open file failure

#define DMXRET_ERR_INFO             (-100)
#define DMXRET_ERR_NOT_SUPPORTED_DivXMediaFormat (DMXRET_ERR_INFO - 1)

// Stream Type

#define DMXTYPE_NONE                 (0) //!< get auto selected stream
#define DMXTYPE_VIDEO                (1) //!< get video stream
#define DMXTYPE_AUDIO                (2) //!< get audio stream
#define DMXTYPE_SUBTITLE             (4) //!< get subtitle stream
#define DMXTYPE_GRAPHICS             (8) //!< get graphics stream
#define DMXTYPE_META				 (10) //!< get meta-data stream

// Metadata Structure
#define META_ID_DEFAULT              (0) //!< user defined ID
#define META_ID_COPYRIGHT            (1) //!< CopyRight [string]
#define META_ID_CREATION_TIME        (2) //!< Creation Time [string]
#define META_ID_PROFILE_LEVEL        (3) //!< Profile Level [string]
#define META_ID_CODEC_NAME           (4) //!< Codec Name [string]
#define META_ID_MANUFACTURER         (5) //!< Manufacturer [string]
#define META_ID_LANGUAGE             (6) //!< Language [string]
#define META_ID_ARTIST               (7) //!< Artist [string]
#define META_ID_ALBUM_TITLE          (8) //!< Album Title [string]
#define META_ID_VENDOR_NAME          (9) //!< Vendor Name [string]
#define META_ID_VKEY_TOTAL          (10) //!< Total Key frames
#define META_ID_VKEY_LAST_TIMESTAMP (11) //!< Last Key frame timestamp
#define META_ID_DISPLAY_WIDTH       (12) //!< Display width
#define META_ID_DISPLAY_HEIGHT      (13) //!< Display height

#define META_TYPE_SIGNED_LONG        (1)
#define META_TYPE_UNSIGNED_LONG      (2)
#define META_TYPE_FLOAT              (3)
#define META_TYPE_STRING             (4)
#define META_TYPE_DATA               (5)

typedef struct av_metadata_t
{
    av_uint32_t    u32DataID;
    av_uint32_t    u32DataType;
    av_uint32_t    u32DataLength;     //!< [out] for META_TYPE_STRING or META_TYPE_DATA

    union {
        av_sint32_t    s32Value;         //!< [out] META_TYPE_SIGNED
        av_uint32_t    u32Value;        //!< [out] META_TYPE_UNSIGNED
        av_float_t     fValue;         //!< [out] META_TYPE_FLOAT
        av_string_t    *pszString;      //!< [out] META_TYPE_STRING
        av_byte_t      *pbyData;        //!< [out] META_TYPE_DATA
    } unData;
} av_metadata_t;


/*
   ============================================================
 *
 *  Demuxer Operation Usage and Structure Definitions
 *
 ============================================================
 */

/*
   ============================================================
 *  Operation: Demuxer Open
 ============================================================
 *  PARAMS:
 *      - opCode:   DMXOP_OPEN
 *      - handle:   (av_dmx_handle_t*)pHandle
 *      - param1:   <demuxer specific init. structure>in (xxx_dmx_init_t*)
 *      - param2:   <demuxer specific info. structure>out (xxx_dmx_info_t*)
 *  RETURNS:
 *      - DMXRET_SUCCESS / DMXRET_FAILED
 ============================================================
 */

/*
   ============================================================
 *  Operation: Demuxer Close
 ============================================================
 *  PARAMS:
 *      - opCode:   DMXOP_CLOSE
 *      - handle:   (av_dmx_handle_t*)pHandle
 *      - param1:   NULL
 *      - param2:   NULL
 *  RETURNS:
 *      - DMXRET_SUCCESS / DMXRET_FAILED
 ============================================================
 */


/*============================================================
 *  Operation: Get Stream
 =============================================================
 *  PARAMS:
 *      - opCode:   DMXOP_GET_STREAM
 *      - handle:   (av_dmx_handle_t*)pHandle
 *      - param1:   (av_dmx_getstream_t*)in
 *      - param2:   (av_dmx_outstream_t*)out
 *  RETURNS:
 *      - DMXRET_SUCCESS / DMXRET_FAILED / DMXERR_END_OF_STREAM
 ============================================================*/

//! Get stream structure

typedef struct av_dmx_getstream_t
{
    av_uint8_t   *pbyStreamBuff;      //!< [in] the pointer to getstream buffer
    av_sint32_t   s32StreamBuffSize;    //!< [in] the size of getstream buffer
    av_sint32_t   s32StreamType;        //!< [in] the type of getstream
    void         *pvSpecificData;      //!< [in] the pointer to specific input of demuxer(if demuxer has a specific data)
} av_dmx_getstream_t;

typedef struct av_dmx_outstream_t
{
    av_byte_t     *pbyStreamData;      //!< [out] the pointer to outstream data
    av_sint32_t    s32StreamDataSize;    //!< [out] the size to outstream data
    av_sint32_t    s32StreamType;        //!< [out] the type of outstream
    av_sint32_t    s32TimeStamp;         //!< [out] the timestamp of outstream
    av_sint32_t    s32EndTimeStamp;      //!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
    void          *pvSpecificData;      //!< [out] the pointer to specific output of demuxer(if demuxer has a specific data)
} av_dmx_outstream_t;



/*
   ============================================================
 *  Operation: Select Stream
 ============================================================
 *  PARAMS:
 *      - opCode:   DMXOP_SELECT_CHANNEL
 *      - handle:   (av_dmx_handle_t*)pHandle
 *      - param1:   <demuxer specific select stream structure>in (xxx_dmx_sel_stream_t*)
 *      - param2:   <demuxer specific select info. structure>out (xxx_dmx_sel_info_t*)
 *  RETURNS:
 *      - DMXRET_SUCCESS / DMXRET_FAILED / DMXERR_END_OF_STREAM
 ============================================================
 */



/*
   ============================================================
 *  Operation: Frame Seek
 ============================================================
 *  PARAMS:
 *      - opCode:   DMXOP_SEEK
 *      - handle:   (DMXHANDLE_t*)pHandle
 *      - param1:   (av_dmx_seek_t*)in
 *      - param2:   (av_dmx_seek_t*)out
 *  RETURNS:
 *      - DMXRET_SUCCESS / DMXRET_FAILED / DMXERR_END_OF_STREAM
 ============================================================
 */

/*
 *-----------------------------------------------------------
 *  Seek Mode Flags (av_dmx_seek_t.u32SeekMode)
 *-----------------------------------------------------------
 */
#define DMXSEEK_DEFAULT             (0x00000000)  //!< default seek mode : DMXSEEK_SYNC|DMXSEEK_TIME_AUTO
//! seek options
#define DMXSEEK_SYNC                (0x00000001)  //!< find sync-frame
#define DMXSEEK_RELATIVE_TIME       (0x00000010)
#define DMXSEEK_TIME_BWD            (0x00000100)  //!< find stream that has PTS less than or equal to target time
#define DMXSEEK_TIME_FWD            (0x00000200)  //!< find stream that has PTS bigger than or equal to target time
#define DMXSEEK_TIME_AUTO           (0x00000300)  //!< find stream that has less/bigger or equal PTS which will be decided by considering current/target time.
#define DMXSEEK_GO_EOS_IF_EOS       (0x00010000)  //!< Set demuxing position to EOS when reached EOS as a result of seek.
#define DMXSEEK_GO_LSYNC_IF_EOS     (0x00020000)  //!< Set demuxing position to last sync-frame when reached EOS as a result of seek.

typedef struct av_dmx_seek_t
{
    av_sint32_t    s32SeekTime;    //!< millisecond unit
    av_uint32_t    u32SeekMode;    //!< mode flags
} av_dmx_seek_t;


/*!
  ============================================================
 *  Operation: Get Last Error
 ============================================================
 *  PARAMS:
 *      - opCode:   DMXOP_GET_LASTERROR
 *      - handle:   (av_dmx_handle_t*)pHandle
 *      - param1:   NULL
 *      - param2:   NULL
 *  RETURNS:
 *      - <Demuxer Specific Last Error Code>
 ============================================================
 */


#endif//AV_COMMON_DMX_H_
