/*
 *   FileName : av_common_types.h
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
#ifndef AV_COMMON_TYPES_H_
#define AV_COMMON_TYPES_H_

#include <stddef.h>
/*
 ============================================================
 *
 *  Type Definition
 *
 ============================================================
*/
#if defined(_WIN32_WCE) || defined(_WIN32)
    typedef signed __int64      S64;
    typedef unsigned __int64    U64;
#else
    typedef signed long long    S64;
    typedef unsigned long long  U64;
#endif

typedef char                  av_sint8_t;  //a signed 8-bit type
typedef unsigned char         av_uint8_t;  //an unsigned 8-bit type
typedef short                 av_sint16_t; //a signed 16-bit type
typedef unsigned short        av_uint16_t; //an unsigned 16-bit type
typedef int                   av_sint32_t; //a signed 32-bit type
typedef unsigned int          av_uint32_t; //an unsigned 32-bit type
typedef long                  av_long_t;   //a signed long type
typedef unsigned long         av_ulong_t;  //an unsigned long type
typedef long long             av_sint64_t; //a signed 64-bit type
typedef unsigned long long    av_uint64_t; //an unsigned 64-bit type
typedef unsigned long         av_size_t;  //size_t type
typedef float                 av_float_t; //float type

typedef void*          av_handle_t;
typedef void*          av_param_t;
typedef av_sint32_t    av_result_t;
typedef av_sint8_t     av_string_t;
typedef av_uint8_t     av_byte_t;
/*
============================================================
*
*   Common Structures Definition
*
============================================================
*/

//! Memory Management Functions
typedef struct av_memory_func_t
{
    void* (*pfnMalloc        ) (av_size_t size);                                    //!< malloc
    void  (*pfnFree          ) (void* ptr);                                        //!< free
    av_sint32_t (*pfnMemcmp  ) (const void* ptr1, const void* ptr2, av_size_t num);       //!< memcmp
    void* (*pfnMemcpy        ) (void* dst, const void* src, av_size_t num);             //!< memcpy
    void* (*pfnMemmove       ) (void* dst, const void* src, av_size_t num);             //!< memmove
    void* (*pfnMemset        ) (void* ptr, av_sint32_t size, av_size_t num);                     //!< memset
    void* (*pfnRealloc       ) (void* ptr, av_size_t size);                          //!< realloc
    av_sint32_t reserved[16-7];                                                             //!< Reserved for future
} av_memory_func_t;

//! File Management Functions
typedef struct av_file_func_t
{
    void*        (*pfnFopen  ) (const av_sint8_t* filename, const av_sint8_t* mode);                     //!< fopen
    av_size_t    (*pfnFread  ) (void* ptr, av_size_t size, av_size_t count, void* stream);     //!< fread
    av_sint32_t  (*pfnFseek  ) (void* stream, av_long_t offset, av_sint32_t origin);                         //!< fseek 32bit io
    av_long_t    (*pfnFtell  ) (void* stream);                                        //!< ftell 32bit io
    av_sint32_t  (*pfnFclose ) (void* stream);                                        //!< fclose
    av_size_t    (*pfnFeof   ) (void* stream);                                        //!< feof
    av_size_t    (*pfnFflush ) (void* stream);                                        //!< fflush
    av_size_t    (*pfnFwrite ) (const void* ptr, av_size_t size, av_size_t count, void* stream);//!< fwrite (Muxer only)
    av_sint32_t  (*pfnUnlink ) (const av_sint8_t* pathname);                                  //!< unlink (Muxer only)

    // 64bit io
    av_sint32_t  (*pfnFseek64) (void* stream, av_sint64_t offset, av_sint32_t origin);                              //!< fseek 64bit io
    av_sint64_t  (*pfnFtell64) (void* stream);                                        //!< ftell 64bit io
    av_sint32_t reserved[16-11];                                                            //!< Reserved for future
} av_file_func_t;

#endif//AV_COMMON_TYPES_H_
