/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2013 Telechips Audio/Video Algorithm Group <AValgorithm@telechips.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GST_TC_MEDIATYPE_H__
#define GST_TC_MEDIATYPE_H__

#include <gst/gst.h>

G_BEGIN_DECLS

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Video Four Character Code
// 
//image : jpeg (Motion JPEG)
#define FOURCC_MJPG		(GST_MAKE_FOURCC ('M', 'J', 'P', 'G'))
#define FOURCC_AVRn		(GST_MAKE_FOURCC ('A', 'V', 'R', 'n'))
#define FOURCC_IJPG		(GST_MAKE_FOURCC ('I', 'J', 'P', 'G'))
#define FOURCC_ijpg		(GST_MAKE_FOURCC ('i', 'j', 'p', 'g'))
#define FOURCC_dmb1		(GST_MAKE_FOURCC ('d', 'm', 'b', '1'))
#define FOURCC_ACDV		(GST_MAKE_FOURCC ('A', 'C', 'D', 'V'))
#define FOURCC_QIVG		(GST_MAKE_FOURCC ('Q', 'I', 'V', 'G'))

//image : jpeg (JPEG Still Image)
#define FOURCC_JPEG		(GST_MAKE_FOURCC ('J', 'P', 'E', 'G')) /* generic (mostly RGB) MJPEG */
#define FOURCC_jpeg		(GST_MAKE_FOURCC ('j', 'p', 'e', 'g')) /* generic (mostly RGB) MJPEG */

//image/jpeg (Miro/Pinnacle Motion JPEG)
#define FOURCC_PIXL		(GST_MAKE_FOURCC ('P', 'I', 'X', 'L')) /* Miro/Pinnacle fourccs */
#define FOURCC_VIXL		(GST_MAKE_FOURCC ('V', 'I', 'X', 'L')) /* Miro/Pinnacle fourccs */

//image/jpeg (Creative Webcam JPEG)
#define FOURCC_CJPG		(GST_MAKE_FOURCC ('C', 'J', 'P', 'G'))

//image/jpeg (SL Motion JPEG)
#define FOURCC_SLMJ		(GST_MAKE_FOURCC ('S', 'L', 'M', 'J'))

//image/jpeg (Pegasus Lossless JPEG)
#define FOURCC_JPGL		(GST_MAKE_FOURCC ('J', 'P', 'G', 'L'))

//video/x-loco (LOCO Lossless)
#define FOURCC_LOCO		(GST_MAKE_FOURCC ('L', 'O', 'C', 'O'))

//video/sp5x (Sp5x-like JPEG)
#define FOURCC_SP53		(GST_MAKE_FOURCC ('S', 'P', '5', '3'))
#define FOURCC_SP54		(GST_MAKE_FOURCC ('S', 'P', '5', '4'))
#define FOURCC_SP55		(GST_MAKE_FOURCC ('S', 'P', '5', '5'))
#define FOURCC_SP56		(GST_MAKE_FOURCC ('S', 'P', '5', '6'))
#define FOURCC_SP57		(GST_MAKE_FOURCC ('S', 'P', '5', '7'))
#define FOURCC_SP58		(GST_MAKE_FOURCC ('S', 'P', '5', '8'))

//video/x-zmbv (Zip Motion Block video)
#define FOURCC_ZMBV		(GST_MAKE_FOURCC ('Z', 'M', 'B', 'V'))

//video/x-huffyuv (Huffman Lossless Codec)
#define FOURCC_HFYU		(GST_MAKE_FOURCC ('H', 'F', 'Y', 'U'))

//video/mpeg (MPEG-1 video)
#define FOURCC_MPEG		(GST_MAKE_FOURCC ('M', 'P', 'E', 'G'))
#define FOURCC_mpeg		(GST_MAKE_FOURCC ('m', 'p', 'e', 'g'))
#define FOURCC_MPGI		(GST_MAKE_FOURCC ('M', 'P', 'G', 'I'))
#define FOURCC_mpg1		(GST_MAKE_FOURCC ('m', 'p', 'g', '1'))
#define FOURCC_MPG1		(GST_MAKE_FOURCC ('M', 'P', 'G', '1'))
#define FOURCC_PIM1		(GST_MAKE_FOURCC ('P', 'I', 'M', '1'))
#define FOURCC_1000		(GST_MAKE_FOURCC (0x01, 0x00, 0x00, 0x10))

//video/mpeg (MPEG-2 video)
#define FOURCC_MPG2		(GST_MAKE_FOURCC ('M', 'P', 'G', '2'))
#define FOURCC_mpg2		(GST_MAKE_FOURCC ('m', 'p', 'g', '2'))
#define FOURCC_PIM2		(GST_MAKE_FOURCC ('P', 'I', 'M', '2'))
#define FOURCC_DVR 		(GST_MAKE_FOURCC ('D', 'V', 'R', ' '))
#define FOURCC_2000		(GST_MAKE_FOURCC (0x02, 0x00, 0x00, 0x10))

//video/mpeg (Lead MPEG-2 video)
#define FOURCC_LMP2		(GST_MAKE_FOURCC ('L', 'M', 'P', '2'))

//video/x-h263 (ITU H.26n)
#define FOURCC_H263		(GST_MAKE_FOURCC ('H', '2', '6', '3'))
#define FOURCC_h263		(GST_MAKE_FOURCC ('h', '2', '6', '3'))
#define FOURCC_i263		(GST_MAKE_FOURCC ('i', '2', '6', '3'))
#define FOURCC_U263		(GST_MAKE_FOURCC ('U', '2', '6', '3'))
#define FOURCC_viv1		(GST_MAKE_FOURCC ('v', 'i', 'v', '1'))
#define FOURCC_T263		(GST_MAKE_FOURCC ('T', '2', '6', '3'))

//video/x-h263 (Lead H.263)
#define FOURCC_L263		(GST_MAKE_FOURCC ('L', '2', '6', '3'))

//video/x-h263 (Microsoft H.263)
#define FOURCC_M263		(GST_MAKE_FOURCC ('M', '2', '6', '3'))
#define FOURCC_m263		(GST_MAKE_FOURCC ('m', '2', '6', '3'))

#define FOURCC_s263     (GST_MAKE_FOURCC ('s', '2', '6', '3')) //0x33363273 ITU H.263 video (3GPP format)
#define FOURCC_S263     (GST_MAKE_FOURCC ('S', '2', '6', '3')) //0x33363253

//video/x-h263 (VDOLive)
#define FOURCC_VDOW		(GST_MAKE_FOURCC ('V', 'D', 'O', 'W'))

//video/x-h263 (Vivo H.263)
#define FOURCC_VIVO		(GST_MAKE_FOURCC ('V', 'I', 'V', 'O'))

//video/x-h263 (Xirlink H.263)
#define FOURCC_x263		(GST_MAKE_FOURCC ('x', '2', '6', '3'))

//video/x-intel-h263 (Intel H.263)
#define FOURCC_I263		(GST_MAKE_FOURCC ('I', '2', '6', '3'))

//video/x-h263 (Lucent VX1000S H.263)
#define FOURCC_VX1K		(GST_MAKE_FOURCC ('V', 'X', '1', 'K'))

//video/x-h264 (ITU H.264)
#define FOURCC_avc1     (GST_MAKE_FOURCC ('a', 'v', 'c', '1')) //0x31637661
#define FOURCC_AVC1     (GST_MAKE_FOURCC ('A', 'V', 'C', '1')) //0x31435641
#define FOURCC_h264     (GST_MAKE_FOURCC ('h', '2', '6', '4'))
#define FOURCC_H264     (GST_MAKE_FOURCC ('H', '2', '6', '4')) //0x34363248
#define FOURCC_x264     (GST_MAKE_FOURCC ('x', '2', '6', '4'))
#define FOURCC_X264     (GST_MAKE_FOURCC ('X', '2', '6', '4'))
#define FOURCC_vssh     (GST_MAKE_FOURCC ('v', 's', 's', 'h'))
#define FOURCC_VSSH     (GST_MAKE_FOURCC ('V', 'S', 'S', 'H'))
#define FOURCC_z264     (GST_MAKE_FOURCC ('z', '2', '6', '4'))
#define FOURCC_Z264     (GST_MAKE_FOURCC ('Z', '2', '6', '4'))
#define FOURCC_davc     (GST_MAKE_FOURCC ('d', 'a', 'v', 'c')) //Dicas MPEGable H.264/MPEG-4 AVC base profile codec
#define FOURCC_DAVC     (GST_MAKE_FOURCC ('D', 'A', 'V', 'C')) //Dicas MPEGable H.264/MPEG-4 AVC base profile codec

//video/x-h264-mvc (ITU H.264 Multi-View)
#define FOURCC_MVC 		(GST_MAKE_FOURCC ('M', 'V', 'C', ' '))
#define FOURCC_mvc 		(GST_MAKE_FOURCC ('m', 'v', 'c', ' '))

//video/x-h264 (Lead H.264)
#define FOURCC_L264		(GST_MAKE_FOURCC ('L', '2', '6', '4'))

//video/x-h265 (ITU H.265/HEVC)
#define FOURCC_h265     (GST_MAKE_FOURCC ('h', '2', '6', '5'))
#define FOURCC_H265     (GST_MAKE_FOURCC ('H', '2', '6', '5')) 
#define FOURCC_hevc     (GST_MAKE_FOURCC ('h', 'e', 'v', 'c'))
#define FOURCC_HEVC     (GST_MAKE_FOURCC ('H', 'E', 'V', 'C'))
#define FOURCC_hev1     (GST_MAKE_FOURCC ('h', 'e', 'v', '1'))
#define FOURCC_HEV1     (GST_MAKE_FOURCC ('H', 'E', 'V', '1'))
#define FOURCC_hvc1     (GST_MAKE_FOURCC ('h', 'v', 'c', '1'))
#define FOURCC_HVC1     (GST_MAKE_FOURCC ('H', 'V', 'C', '1'))
#define FOURCC_HM10     (GST_MAKE_FOURCC ('H', 'M', '1', '0')) 

//video/mpeg (Samsung MPEG-4)
#define FOURCC_SEDG		(GST_MAKE_FOURCC ('S', 'E', 'D', 'G'))

//video/mpeg (Divio MPEG-4)
#define FOURCC_M4CC		(GST_MAKE_FOURCC ('M', '4', 'C', 'C'))

#define FOURCC_THEORA   (GST_MAKE_FOURCC ('T', 'H', 'E', 'O'))
#define FOURCC_theora   (GST_MAKE_FOURCC ('t', 'h', 'e', 'o'))

//video/x-divx (DivX MS-MPEG-4 Version 3)
#define FOURCC_DIV3		(GST_MAKE_FOURCC ('D', 'I', 'V', '3'))
#define FOURCC_div3		(GST_MAKE_FOURCC ('d', 'i', 'v', '3'))
#define FOURCC_DVX3		(GST_MAKE_FOURCC ('D', 'V', 'X', '3'))
#define FOURCC_dvx3		(GST_MAKE_FOURCC ('d', 'v', 'x', '3'))
#define FOURCC_DIV4		(GST_MAKE_FOURCC ('D', 'I', 'V', '4'))
#define FOURCC_div4		(GST_MAKE_FOURCC ('d', 'i', 'v', '4'))
#define FOURCC_DIV5		(GST_MAKE_FOURCC ('D', 'I', 'V', '5'))
#define FOURCC_div5		(GST_MAKE_FOURCC ('d', 'i', 'v', '5'))
#define FOURCC_DIV6		(GST_MAKE_FOURCC ('D', 'I', 'V', '6'))
#define FOURCC_div6		(GST_MAKE_FOURCC ('d', 'i', 'v', '6'))
#define FOURCC_MPG3		(GST_MAKE_FOURCC ('M', 'P', 'G', '3'))
#define FOURCC_mpg3		(GST_MAKE_FOURCC ('m', 'p', 'g', '3'))
#define FOURCC_col0		(GST_MAKE_FOURCC ('c', 'o', 'l', '0'))
#define FOURCC_COL0		(GST_MAKE_FOURCC ('C', 'O', 'L', '0'))
#define FOURCC_col1		(GST_MAKE_FOURCC ('c', 'o', 'l', '1'))
#define FOURCC_COL1		(GST_MAKE_FOURCC ('C', 'O', 'L', '1'))
#define FOURCC_AP41		(GST_MAKE_FOURCC ('A', 'P', '4', '1'))

//video/x-divx (DivX MPEG-4 Version 4)
#define FOURCC_divx		(GST_MAKE_FOURCC ('d', 'i', 'v', 'x'))
#define FOURCC_DIVX		(GST_MAKE_FOURCC ('D', 'I', 'V', 'X'))

//video/x-divx (Blizzard DivX)
#define FOURCC_BLZ0		(GST_MAKE_FOURCC ('B', 'L', 'Z', '0'))

//video/x-divx (DivX MPEG-4 Version 5)
#define FOURCC_DX50		(GST_MAKE_FOURCC ('D', 'X', '5', '0'))
#define FOURCC_dx50     (GST_MAKE_FOURCC ('d', 'x', '5', '0'))

#define FOURCC_FMD4     (GST_MAKE_FOURCC ('F', 'M', 'D', '4'))
#define FOURCC_fmd4     (GST_MAKE_FOURCC ('f', 'm', 'd', '4'))

//video/x-xvid (XVID MPEG-4)
#define FOURCC_XVID		(GST_MAKE_FOURCC ('X', 'V', 'I', 'D'))
#define FOURCC_xvid		(GST_MAKE_FOURCC ('x', 'v', 'i', 'd'))
#define FOURCC_Xvid     (GST_MAKE_FOURCC ('X', 'v', 'i', 'd'))

//video/x-xvid (Sigma-Designs MPEG-4)
#define FOURCC_RMP4		(GST_MAKE_FOURCC ('R', 'M', 'P', '4'))

//video/x-msmpeg (Microsoft MPEG-4 4.1)
#define FOURCC_MPG4		(GST_MAKE_FOURCC ('M', 'P', 'G', '4'))
#define FOURCC_MP41		(GST_MAKE_FOURCC ('M', 'P', '4', '1'))
#define FOURCC_mp41		(GST_MAKE_FOURCC ('m', 'p', '4', '1'))

//video/x-msmpeg (Microsoft MPEG-4 4.2)
#define FOURCC_mp42		(GST_MAKE_FOURCC ('m', 'p', '4', '2'))
#define FOURCC_MP42		(GST_MAKE_FOURCC ('M', 'P', '4', '2'))

//video/x-msmpeg (Microsoft MPEG-4 4.3)
#define FOURCC_mp43		(GST_MAKE_FOURCC ('m', 'p', '4', '3'))
#define FOURCC_MP43		(GST_MAKE_FOURCC ('M', 'P', '4', '3'))

//video/mpeg (Microsoft ISO MPEG-4 1.1)
#define FOURCC_MP4S		(GST_MAKE_FOURCC ('M', 'P', '4', 'S'))
#define FOURCC_M4S2		(GST_MAKE_FOURCC ('M', '4', 'S', '2'))

//video/mpeg (FFmpeg MPEG-4)
#define FOURCC_FMP4		(GST_MAKE_FOURCC ('F', 'M', 'P', '4'))
#define FOURCC_UMP4		(GST_MAKE_FOURCC ('U', 'M', 'P', '4'))
#define FOURCC_FFDS		(GST_MAKE_FOURCC ('F', 'F', 'D', 'S'))

//video/mpeg (MPEG-4)
#define FOURCC_EM4A		(GST_MAKE_FOURCC ('E', 'M', '4', 'A'))
#define FOURCC_EPVH		(GST_MAKE_FOURCC ('E', 'P', 'V', 'H'))
#define FOURCC_FVFW		(GST_MAKE_FOURCC ('F', 'V', 'F', 'W'))
#define FOURCC_fvfw   (GST_MAKE_FOURCC ('f', 'v', 'f', 'w'))
#define FOURCC_INMC		(GST_MAKE_FOURCC ('I', 'N', 'M', 'C'))
#define FOURCC_DIGI		(GST_MAKE_FOURCC ('D', 'I', 'G', 'I'))
#define FOURCC_DM2K		(GST_MAKE_FOURCC ('D', 'M', '2', 'K'))
#define FOURCC_DCOD		(GST_MAKE_FOURCC ('D', 'C', 'O', 'D'))
#define FOURCC_MVXM		(GST_MAKE_FOURCC ('M', 'V', 'X', 'M'))
#define FOURCC_PM4V		(GST_MAKE_FOURCC ('P', 'M', '4', 'V'))
#define FOURCC_SMP4		(GST_MAKE_FOURCC ('S', 'M', 'P', '4'))
#define FOURCC_DXGM		(GST_MAKE_FOURCC ('D', 'X', 'G', 'M'))
#define FOURCC_VIDM		(GST_MAKE_FOURCC ('V', 'I', 'D', 'M'))
#define FOURCC_M4T3		(GST_MAKE_FOURCC ('M', '4', 'T', '3'))
#define FOURCC_GEOX		(GST_MAKE_FOURCC ('G', 'E', 'O', 'X'))
#define FOURCC_MP4V		(GST_MAKE_FOURCC ('M', 'P', '4', 'V'))
#define FOURCC_mp4v		(GST_MAKE_FOURCC ('m', 'p', '4', 'v'))

//video/x-msmpeg (Microsoft MPEG-4 4.3)
#define FOURCC_3ivd		(GST_MAKE_FOURCC ('3', 'i', 'v', 'd'))
#define FOURCC_3IVD		(GST_MAKE_FOURCC ('3', 'I', 'V', 'D'))

//video/x-3ivx (3ivx)
#define FOURCC_3IV1		(GST_MAKE_FOURCC ('3', 'I', 'V', '1'))
#define FOURCC_3IV2		(GST_MAKE_FOURCC ('3', 'I', 'V', '2'))
#define FOURCC_3iv2   (GST_MAKE_FOURCC ('3', 'i', 'v', '2'))

//video/x-dv (Generic DV)
#define FOURCC_DVSD		(GST_MAKE_FOURCC ('D', 'V', 'S', 'D'))
#define FOURCC_dvsd		(GST_MAKE_FOURCC ('d', 'v', 's', 'd'))
#define FOURCC_dvc 		(GST_MAKE_FOURCC ('d', 'v', 'c', ' '))
#define FOURCC_dv25		(GST_MAKE_FOURCC ('d', 'v', '2', '5'))

//video/x-dv (Canopus DV)
#define FOURCC_CDVC		(GST_MAKE_FOURCC ('C', 'D', 'V', 'C'))
#define FOURCC_cdvc		(GST_MAKE_FOURCC ('c', 'd', 'v', 'c'))

//video/x-dv (DVCPro50 Video)
#define FOURCC_DV50		(GST_MAKE_FOURCC ('D', 'V', '5', '0'))
#define FOURCC_dv50		(GST_MAKE_FOURCC ('d', 'v', '5', '0'))

//video/x-wmv (Microsoft Windows Media 7)
#define FOURCC_WMV1		(GST_MAKE_FOURCC ('W', 'M', 'V', '1'))
#define FOURCC_wmv1   (GST_MAKE_FOURCC ('w', 'm', 'v', '1'))

//video/x-wmv (Microsoft Windows Media 8)
#define FOURCC_WMV2		(GST_MAKE_FOURCC ('W', 'M', 'V', '2'))
#define FOURCC_wmv2   (GST_MAKE_FOURCC ('w', 'm', 'v', '2'))

//video/x-wmv (Microsoft Windows Media 9)
#define FOURCC_VC1      (GST_MAKE_FOURCC ('V', 'C', '1', ' '))
#define FOURCC_vc1      (GST_MAKE_FOURCC ('v', 'c', '1', ' '))
#define FOURCC_WMV3     (GST_MAKE_FOURCC ('W', 'M', 'V', '3')) // Complex profile is not covered by VC-1 standard and may occur in old WMV3 files where it was called "advanced profile".
#define FOURCC_wmv3     (GST_MAKE_FOURCC ('w', 'm', 'v', '3'))

//video/x-wmv (Microsoft Windows Media Advanced Profile)
#define FOURCC_WMVA		(GST_MAKE_FOURCC ('W', 'M', 'V', 'A'))

//video/x-wmv (Microsoft Windows Media VC-1)
#define FOURCC_WVC1		(GST_MAKE_FOURCC ('W', 'V', 'C', '1'))
#define FOURCC_wvc1		(GST_MAKE_FOURCC ('w', 'v', 'c', '1'))

//video/x-avs (Audio Video Standard)
#define FOURCC_AVS 		(GST_MAKE_FOURCC ('A', 'V', 'S', ' '))
#define FOURCC_avs 		(GST_MAKE_FOURCC ('a', 'v', 's', ' '))
#define FOURCC_CAVS     (GST_MAKE_FOURCC ('C','A','V','S'))
#define FOURCC_cavs     (GST_MAKE_FOURCC ('c','a','v','s'))

//video/x-cinepak (Cinepak video)
#define FOURCC_cvid		(GST_MAKE_FOURCC ('c', 'v', 'i', 'd'))

//video/x-aasc (Autodesk Animator
#define FOURCC_AASC		(GST_MAKE_FOURCC ('A', 'A', 'S', 'C'))

//video/x-xan (Xan Wing Commander 4)
#define FOURCC_Xxan		(GST_MAKE_FOURCC ('X', 'x', 'a', 'n'))

//video/x-indeo (Intel Video 2)
#define FOURCC_RT21		(GST_MAKE_FOURCC ('R', 'T', '2', '1'))

//video/x-indeo (Intel Video 3)
#define FOURCC_IV31		(GST_MAKE_FOURCC ('I', 'V', '3', '1'))
#define FOURCC_IV32		(GST_MAKE_FOURCC ('I', 'V', '3', '2'))
#define FOURCC_iv31		(GST_MAKE_FOURCC ('i', 'v', '3', '1'))
#define FOURCC_iv32		(GST_MAKE_FOURCC ('i', 'v', '3', '2'))

//video/x-indeo (Intel Video 4)
#define FOURCC_IV41		(GST_MAKE_FOURCC ('I', 'V', '4', '1'))
#define FOURCC_iv41		(GST_MAKE_FOURCC ('i', 'v', '4', '1'))

//video/x-indeo (Intel Video 5)
#define FOURCC_IV50		(GST_MAKE_FOURCC ('I', 'V', '5', '0'))

//video/x-mszh (Lossless MSZH Video)
#define FOURCC_MSZH		(GST_MAKE_FOURCC ('M', 'S', 'Z', 'H'))

//video/x-zlib (Lossless zlib video)
#define FOURCC_ZLIB		(GST_MAKE_FOURCC ('Z', 'L', 'I', 'B'))

//video/x-cirrus-logic-accupak (Cirrus Logipak AccuPak)
#define FOURCC_CLJR		(GST_MAKE_FOURCC ('C', 'L', 'J', 'R'))
#define FOURCC_cljr		(GST_MAKE_FOURCC ('c', 'l', 'j', 'r'))

//video/x-compressed-yuv (CYUV Lossless)
#define FOURCC_CYUV		(GST_MAKE_FOURCC ('C', 'Y', 'U', 'V'))
#define FOURCC_cyuv		(GST_MAKE_FOURCC ('c', 'y', 'u', 'v'))

//video/x-truemotion (Duck Truemotion1)
#define FOURCC_DUCK		(GST_MAKE_FOURCC ('D', 'U', 'C', 'K'))
#define FOURCC_PVEZ		(GST_MAKE_FOURCC ('P', 'V', 'E', 'Z'))

//video/x-truemotion (TrueMotion 2.0)
#define FOURCC_TM20		(GST_MAKE_FOURCC ('T', 'M', '2', '0'))

//video/x-vp3 (VP3)
#define FOURCC_VP30		(GST_MAKE_FOURCC ('V', 'P', '3', '0'))
#define FOURCC_vp30		(GST_MAKE_FOURCC ('v', 'p', '3', '0'))
#define FOURCC_VP31		(GST_MAKE_FOURCC ('V', 'P', '3', '1'))
#define FOURCC_vp31		(GST_MAKE_FOURCC ('v', 'p', '3', '1'))
#define FOURCC_VP3 		(GST_MAKE_FOURCC ('V', 'P', '3', ' '))

// RealVideo
#define FOURCC_rv10     (GST_MAKE_FOURCC('r','v','1','0'))
#define FOURCC_RV10     (GST_MAKE_FOURCC('R','V','1','0'))
#define FOURCC_rv20     (GST_MAKE_FOURCC('r','v','2','0'))
#define FOURCC_RV20     (GST_MAKE_FOURCC('R','V','2','0'))
#define FOURCC_rv30     (GST_MAKE_FOURCC('r','v','3','0'))
#define FOURCC_RV30     (GST_MAKE_FOURCC('R','V','3','0'))
#define FOURCC_rv40     (GST_MAKE_FOURCC('r','v','4','0'))
#define FOURCC_RV40     (GST_MAKE_FOURCC('R','V','4','0'))
#define FOURCC_RV89COMBO (GST_MAKE_FOURCC('T','R','O','M'))

//video/x-ultimotion (IBM UltiMotion)
#define FOURCC_ULTI		(GST_MAKE_FOURCC ('U', 'L', 'T', 'I'))

//video/x-camtasia (TechSmith Camtasia)
#define FOURCC_TSCC		(GST_MAKE_FOURCC ('T', 'S', 'C', 'C'))
#define FOURCC_tscc		(GST_MAKE_FOURCC ('t', 's', 'c', 'c'))

//video/x-camstudio (Camstudio)
#define FOURCC_CSCD		(GST_MAKE_FOURCC ('C', 'S', 'C', 'D'))

//video/x-ati-vcr (ATI VCR 1)
#define FOURCC_VCR1		(GST_MAKE_FOURCC ('V', 'C', 'R', '1'))

//video/x-ati-vcr (ATI VCR 2)
#define FOURCC_VCR2		(GST_MAKE_FOURCC ('V', 'C', 'R', '2'))

//video/x-asus (Asus Video 1)
#define FOURCC_ASV1		(GST_MAKE_FOURCC ('A', 'S', 'V', '1'))

//video/x-asus (Asus Video 2)
#define FOURCC_ASV2		(GST_MAKE_FOURCC ('A', 'S', 'V', '2'))

//image/png (PNG image)
#define FOURCC_MPNG		(GST_MAKE_FOURCC ('M', 'P', 'N', 'G'))
#define FOURCC_mpng		(GST_MAKE_FOURCC ('m', 'p', 'n', 'g'))
#define FOURCC_PNG 		(GST_MAKE_FOURCC ('P', 'N', 'G', ' '))

//video/x-flash-video (Flash Video 1)
#define FOURCC_FLV1		(GST_MAKE_FOURCC ('F', 'L', 'V', '1'))
#define FOURCC_flv1   (GST_MAKE_FOURCC('f','l','v','1'))

//video/x-vmnc (VMWare NC Video)
#define FOURCC_VMnc		(GST_MAKE_FOURCC ('V', 'M', 'n', 'c'))

//video/x-dirac (Dirac)
#define FOURCC_drac		(GST_MAKE_FOURCC ('d', 'r', 'a', 'c'))

//video/x-apple-video (Apple Video (RPZA))
#define FOURCC_rpza		(GST_MAKE_FOURCC ('r', 'p', 'z', 'a'))
#define FOURCC_azpr		(GST_MAKE_FOURCC ('a', 'z', 'p', 'r'))
#define FOURCC_RPZA		(GST_MAKE_FOURCC ('R', 'P', 'Z', 'A'))

//video/x-ffv (FFmpeg lossless video codec)
#define FOURCC_FFV1		(GST_MAKE_FOURCC ('F', 'F', 'V', '1'))

//video/x-kmvc (Karl Morton's video codec)
#define FOURCC_KMVC		(GST_MAKE_FOURCC ('K', 'M', 'V', 'C'))

//video/x-vp5 (On2 VP5)
#define FOURCC_vp50		(GST_MAKE_FOURCC ('v', 'p', '5', '0'))
#define FOURCC_VP50		(GST_MAKE_FOURCC ('V', 'P', '5', '0'))

//video/x-vp6 (On2 VP6)
#define FOURCC_vp60		(GST_MAKE_FOURCC ('v', 'p', '6', '0'))
#define FOURCC_VP60		(GST_MAKE_FOURCC ('V', 'P', '6', '0'))
#define FOURCC_vp61		(GST_MAKE_FOURCC ('v', 'p', '6', '1'))
#define FOURCC_VP61		(GST_MAKE_FOURCC ('V', 'P', '6', '1'))
#define FOURCC_Vp62		(GST_MAKE_FOURCC ('V', 'p', '6', '2'))
#define FOURCC_VP62		(GST_MAKE_FOURCC ('V', 'P', '6', '2'))

//video/x-vp6-flash (On2 VP6)
#define FOURCC_VP6F		(GST_MAKE_FOURCC ('V', 'P', '6', 'F'))
#define FOURCC_vp6f		(GST_MAKE_FOURCC ('v', 'p', '6', 'f'))
#define FOURCC_FLV4		(GST_MAKE_FOURCC ('F', 'L', 'V', '4'))

//video/x-vp7 (On2 VP7)
#define FOURCC_vp70		(GST_MAKE_FOURCC ('v', 'p', '7', '0'))
#define FOURCC_VP70		(GST_MAKE_FOURCC ('V', 'P', '7', '0'))

//video/x-vp8 (On2 VP8)
#define FOURCC_VP80		(GST_MAKE_FOURCC ('V', 'P', '8', '0'))

//video/x-vp9 (On2 VP9)
#define FOURCC_VP90  (GST_MAKE_FOURCC('V','P','9','0'))
#define FOURCC_vp90  (GST_MAKE_FOURCC('v','p','9','0'))

//video/x-mimic (Mimic webcam)
#define FOURCC_LM20		(GST_MAKE_FOURCC ('L', 'M', '2', '0'))

//video/x-theora (Theora video codec)
#define FOURCC_THEO		(GST_MAKE_FOURCC ('T', 'H', 'E', 'O'))
#define FOURCC_theo		(GST_MAKE_FOURCC ('t', 'h', 'e', 'o'))

//video/x-fraps (Fraps video)
#define FOURCC_FPS1		(GST_MAKE_FOURCC ('F', 'P', 'S', '1'))

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Audio Format Tag
// 
//audio/x-raw-int (Uncompressed PCM audio)
#define WAVE_FORMAT_PCM				(0x0001u)

#define WAVE_FORMAT_DVDPCM			(0x1001u)	/* defined by telechips */
#define WAVE_FORMAT_BDPCM			(0x1002u)	/* defined by telechips */

//audio/x-adpcm (ADPCM audio)
#define WAVE_FORMAT_ADPCM			(0x0002)

//audio/x-raw-float (Uncompressed IEEE float audio)
#define WAVE_FORMAT_IEEE_FLOAT		(0x0003)

//audio/unknown
#define WAVE_FORMAT_IBM_CVSD		(0x0005)

//audio/x-alaw (A-law audio)
#define WAVE_FORMAT_ALAW			(0x0006)

//audio/x-mulaw (Mu-law audio)
#define WAVE_FORMAT_MULAW			(0x0007)

//audio/x-raw-int (Big endian LPCM)
#define WAVE_FORMAT_MS_SWAP			(0x0008u)

//audio/x-wms (Windows Media Audio Speech)
#define WAVE_FORMAT_WMS				(0x000a)

//audio/unknown
#define WAVE_FORMAT_OKI_ADPCM		(0x0010)

//audio/x-adpcm (DVI ADPCM audio)
#define WAVE_FORMAT_DVI_ADPCM		(0x0011)

//audio/x-truespeech (DSP Group TrueSpeech)
#define WAVE_FORMAT_DSP_TRUESPEECH	(0x0022)

//audio/ms-gsm (MS GSM audio)
#define WAVE_FORMAT_GSM610			(0x0031)
#define WAVE_FORMAT_MSN				(0x0032)

//audio/mpeg (MPEG-1)
#define WAVE_FORMAT_MPEGL1			(0xCC01)

//audio/mpeg (MPEG-1 layer 2)
#define WAVE_FORMAT_MPEGL12			(0x0050)

//audio/mpeg (MPEG-1 layer 3)
#define WAVE_FORMAT_MPEGL3			(0x0055)

//audio/AMR (AMR Narrow Band (NB))
#define WAVE_FORMAT_AMR_NB			(0x0057)

//audio/AMR-WB (AMR Wide Band (WB))
#define WAVE_FORMAT_AMR_WB			(0x0058)

//audio/x-vorbis (Vorbis)
#define WAVE_FORMAT_VORBIS			(0x566f) /* ogg/vorbis */
#define WAVE_FORMAT_VORBIS1			(0x674f) /* ogg/vorbis mode 1 */
#define WAVE_FORMAT_VORBIS2			(0x6750) /* ogg/vorbis mode 2 */
#define WAVE_FORMAT_VORBIS3			(0x6751) /* ogg/vorbis mode 3 */
#define WAVE_FORMAT_VORBIS1PLUS		(0x676f)	/* ogg/vorbis mode 1+ */
#define WAVE_FORMAT_VORBIS2PLUS		(0x6770)	/* ogg/vorbis mode 2+ */
#define WAVE_FORMAT_VORBIS3PLUS		(0x6771)	/* ogg/vorbis mode 3+ */

//audio/x-ac3 (AC-3 audio)
#define WAVE_FORMAT_A52				(0x2000)

//audio/x-dts (DTS audio)
#define WAVE_FORMAT_DTS				(0x2001)

//audio/x-eac3 (EAC3 Dolby Digital Plus)
#define WAVE_FORMAT_EAC3			(0x2008)

//audio/x-true-hd (TrudHD/AC-3)
#define WAVE_FORMAT_TRUEHD_AC3      (0xeb27cec4U)

//audio/mpeg (MPEG-4 AAC audio)
#define WAVE_FORMAT_TC_AAC			(0xAAC0)	/* defined by telechips */
#define WAVE_FORMAT_AAC				(0x00ff)
#define WAVE_FORMAT_AAC_AC			(0x4143)
#define WAVE_FORMAT_AAC_pm			(0x706d)

//audio/x-wma (Windows Media Audio)
#define WAVE_FORMAT_WMAV1			(0x0160)
#define WAVE_FORMAT_WMAV2			(0x0161)
#define WAVE_FORMAT_WMAV3			(0x0162)
#define WAVE_FORMAT_WMAV3_L			(0x0163)

//audio/x-vnd.sony.atrac3 (Sony ATRAC3)
#define WAVE_FORMAT_SONY_ATRAC3		(0x0270)

//audio/x-siren (Siren7)
#define WAVE_FORMAT_SIREN			(0x028E)

//audio/x-adpcm (IMA/DK4 ADPCM)
#define WAVE_FORMAT_ADPCM_IMA_DK4	(0x0061)  /* not official */

//audio/x-adpcm (IMA/DK3 ADPCM)
#define WAVE_FORMAT_ADPCM_IMA_DK3	(0x0062)  /* not official */

//audio/x-adpcm (IMA/WAV ADPCM)
#define WAVE_FORMAT_ADPCM_IMA_WAV	(0x0069)

//audio/x-voxware
#define WAVE_FORMAT_VOXWARE_AC8			(0x0070)
#define WAVE_FORMAT_VOXWARE_AC10		(0x0071)
#define WAVE_FORMAT_VOXWARE_AC16		(0x0072)
#define WAVE_FORMAT_VOXWARE_AC20		(0x0073)
#define WAVE_FORMAT_VOXWARE_METAVOICE	(0x0074)
#define WAVE_FORMAT_VOXWARE_METASOUND	(0x0075)
#define WAVE_FORMAT_VOXWARE_RT29HW		(0x0076)
#define WAVE_FORMAT_VOXWARE_VR12		(0x0077)
#define WAVE_FORMAT_VOXWARE_VR18		(0x0078)
#define WAVE_FORMAT_VOXWARE_TQ40		(0x0079)
#define WAVE_FORMAT_VOXWARE_TQ60		(0x0081)

//audio/x-ape
#define WAVE_FORMAT_APE		(0x0a9e)

//audio/x-flac
#define WAVE_FORMAT_FLAC	(0xf1ac)

//audio/x-opus
#define AUDIO_FOURCC_OPUS        (0x5355504f) //('O', 'P', 'U', 'S')
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Subtitle Format ID
// 

typedef enum {
	SUBTITLE_FORMAT_ID_UNKNOWN  = -1,
	SUBTITLE_FORMAT_ID_NONE = 0,//mime type
	SUBTITLE_FORMAT_ID_ASCII = 1,
	SUBTITLE_FORMAT_ID_UTF8 = 2,
	SUBTITLE_FORMAT_ID_SRT = 3,
	SUBTITLE_FORMAT_ID_SSA = 4,
	SUBTITLE_FORMAT_ID_ASS = 5,
	SUBTITLE_FORMAT_ID_USF = 6,
	SUBTITLE_FORMAT_ID_KATE = 7,
	SUBTITLE_FORMAT_ID_VOBSUB = 8,
	SUBTITLE_FORMAT_ID_HDMVPGS = 9,
	SUBTITLE_FORMAT_ID_BMP = 10,
	SUBTITLE_FORMAT_ID_MAX = 11
} subtitle_format_t;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	caps creators 
// 

typedef struct tc_mpeg4_vol
{
	guchar*         m_pbyPtr;
	guint32         m_ulPos;
	guint32         m_ulData;
	guint32         m_ulUsedBytes;
} tc_mpeg4_vol;

typedef struct tc_audio_info_t
{
	guint32 ulFormatTag;
	guint32 ulChannels;
	guint32 ulBlockAlign;
	guint32 ulBitPerSample;		
	guint32 ulSampleRate;
	guint32 ulSize;				
	guint32 ulDuration;
	guint32 ulEndian;
	gchar  *pszlanguage;
} tc_audio_info_t;

typedef struct tc_video_info_t
{
	guint32 ulFourCC;
	guint32	ulWidth;
	guint32	ulHeight;
	guint32 ulBitPerPixel; //bits per pixel
	guint32 ulFrameRate;
	guint32 ulDuration;
} tc_video_info_t;

typedef struct tc_subtitle_info_t
{
	guint32 ulFormatId;
	gchar  *pszlanguage;
	gint8   szMimeType[32];
} tc_subtitle_info_t;

typedef struct tc_private_info_t
{
	gint8   szMimeType[32];
} tc_private_info_t;

GstCaps *
gst_tc_create_audio_caps (
	guint32           ulFormatTag,
	tc_audio_info_t  *pstAudioInfo,
    GstBuffer        *pstExtraData, 
	gchar           **ppszCodecName
	);

GstCaps *
gst_tc_create_video_caps (
	guint32          ulFourCC,
	tc_video_info_t *pstVideoInfo,
    GstBuffer       *pstExtraData, 
    gboolean         bEnableRingMode,
	gchar          **ppszCodecName
	);

GstCaps *
gst_tc_create_subtitle_caps (
	guint32          ulFormatId,
	gchar           *szMimeType,
	GstBuffer       *pstExtraData, 
	gchar          **ppszCodecName
	);

GstCaps *
gst_tc_create_audio_template_caps (
	const guint16         *pusFormatTagArray
	);

GstCaps *
gst_tc_create_video_template_caps (
	const guint32         *pulFourCCArray
	);

G_END_DECLS

#endif /* GST_TC_MEDIATYPE_H__ */
