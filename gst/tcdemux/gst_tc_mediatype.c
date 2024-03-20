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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gst_tc_mediatype.h"
#include "gst_tc_demux_base.h"

GST_DEBUG_CATEGORY_EXTERN (gst_tc_demux_debug);
#define GST_CAT_DEFAULT (gst_tc_demux_debug)


GstCaps *
gst_tc_create_audio_caps(
	guint32           ulFormatTag,
	tc_audio_info_t  *pstAudioInfo,
    GstBuffer        *pstExtraData, 
	gchar           **ppszCodecName
	)
{
  gboolean block_align = TCDMX_FALSE;
  gboolean rate_chan = TCDMX_TRUE;
  GstCaps *caps = NULL;
  gint rate_min = 1000, rate_max = 96000;
  gint channels_max = 2;

  GST_DEBUG ("audio format tag 0x%04X", ulFormatTag);

  switch (ulFormatTag & (guint32)0xFFFF) {
    case WAVE_FORMAT_PCM:     /* PCM */
    case WAVE_FORMAT_MS_SWAP: /* Big-endian LPCM */
      rate_max = 192000;
      channels_max = 8;

      if (pstAudioInfo != NULL) {
        gint ba = (gint)pstAudioInfo->ulBlockAlign;
        gint ch = (gint)pstAudioInfo->ulChannels;
        gint wd, ws,endianness;

        /* If we have an empty blockalign, we take the width contained in 
         * pstAudioInfo->ulSize */
        if (ba != 0){
        	wd = ba * 8 / ch;
        }
        else
        {
        	if (pstAudioInfo->ulSize != 0U) {
	          wd = (gint)pstAudioInfo->ulSize;
		} else {
		  wd = (gint)pstAudioInfo->ulBitPerSample;
		}
        }

        if (pstAudioInfo->ulSize > 32U) {
          GST_WARNING ("invalid depth (%d) of pcm audio, overwriting.",
              pstAudioInfo->ulSize);
          pstAudioInfo->ulSize = (guint32)(8u * (((guint)wd + 7u) / 8u));
        }

        /* in riff, the depth is stored in the size field but it just means that
         * the _least_ significant bits are cleared. We can therefore just play
         * the sample as if it had a depth == width */
        /* For reference, the actual depth is in pstAudioInfo->ulSize */
        ws = wd;

        if (ulFormatTag == (guint)WAVE_FORMAT_MS_SWAP){
            pstAudioInfo->ulEndian = 1;
        }

        endianness = (pstAudioInfo->ulEndian == 1U) ? G_BIG_ENDIAN : G_LITTLE_ENDIAN;
		
        caps = gst_caps_new_simple ("audio/x-lpcm",
									"endianness", G_TYPE_INT, endianness,
									"channels", G_TYPE_INT, ch,
									"width", G_TYPE_INT, wd,
									"depth", G_TYPE_INT, ws, "signed", G_TYPE_BOOLEAN, wd != 8, NULL);

        /* Add default channel layout. In theory this should be done
         * for 1 and 2 channels too but apparently breaks too many
         * things currently. Also we know no default layout for more than
         * 8 channels. */
		#if 0
        if (ch > 2) {
          if (ch > 8)
            GST_WARNING ("don't know default layout for %d channels", ch);
          else if (gst_riff_wave_add_default_channel_layout (caps))
            GST_DEBUG ("using default channel layout for %d channels", ch);
          else
            GST_WARNING ("failed to add channel layout");
        }
		#endif
      } else {
        /* FIXME: this is pretty useless - we need fixed caps */
        caps = gst_caps_from_string ("audio/x-lpcm, "
									 "endianness = (int) LITTLE_ENDIAN, "
									 "signed = (boolean) { true, false }, "
									 "width = (int) { 8, 16, 24, 32 }, " "depth = (int) [ 1, 32 ]");
      }
      if ((ppszCodecName != NULL) && (pstAudioInfo != NULL)) {
        *ppszCodecName = g_strdup_printf ("Uncompressed %d-bit PCM audio",
            pstAudioInfo->ulSize);
      }
      break;

	case WAVE_FORMAT_DVDPCM:
	case WAVE_FORMAT_BDPCM:
      rate_max = 192000;
      channels_max = 8;

      if (pstAudioInfo != NULL) {
        gint ba = (gint)pstAudioInfo->ulBlockAlign;
        gint ch = (gint)pstAudioInfo->ulChannels;
        gint wd, ws;

        /* If we have an empty blockalign, we take the width contained in 
         * pstAudioInfo->ulSize */
        if (ba != 0) {
          wd = ba * 8 / ch;
        } else {
          if (pstAudioInfo->ulSize != 0U){
            wd = (gint)pstAudioInfo->ulSize;
          } else {
            wd = (gint)pstAudioInfo->ulBitPerSample;
          }
        }

        if (pstAudioInfo->ulSize > 32U) {
          GST_WARNING ("invalid depth (%d) of pcm audio, overwriting.",
              pstAudioInfo->ulSize);
          pstAudioInfo->ulSize = (guint)(8u * (((guint)wd + 7u) / 8u));
        }

        /* in riff, the depth is stored in the size field but it just means that
         * the _least_ significant bits are cleared. We can therefore just play
         * the sample as if it had a depth == width */
        /* For reference, the actual depth is in pstAudioInfo->ulSize */
        ws = wd;

        if ((ulFormatTag&(guint32)0xFFFF) == (guint32)WAVE_FORMAT_DVDPCM) {
          caps = gst_caps_new_simple ("audio/x-private1-lpcm",
									"endianness", G_TYPE_INT, G_BIG_ENDIAN,
									"channels", G_TYPE_INT, ch,
									"width", G_TYPE_INT, wd,
									"depth", G_TYPE_INT, ws, "signed", G_TYPE_BOOLEAN, wd != 8, NULL);
        } else {
          caps = gst_caps_new_simple ("audio/x-private-ts-lpcm",
									"endianness", G_TYPE_INT, G_BIG_ENDIAN,
									"channels", G_TYPE_INT, ch,
									"width", G_TYPE_INT, wd,
									"depth", G_TYPE_INT, ws, "signed", G_TYPE_BOOLEAN, wd != 8, NULL);
        }

      } else {
        /* FIXME: this is pretty useless - we need fixed caps */
        if ((ulFormatTag&(guint32)0xFFFF) == (guint32)WAVE_FORMAT_DVDPCM) {
          caps = gst_caps_from_string ("audio/x-private1-lpcm, "
									 "endianness = (int) LITTLE_ENDIAN, "
									 "signed = (boolean) { true, false }, "
									 "width = (int) { 8, 16, 24, 32 }, " "depth = (int) [ 1, 32 ]");
        } else {
          caps = gst_caps_from_string ("audio/x-private-ts-lpcm, "
									 "endianness = (int) LITTLE_ENDIAN, "
									 "signed = (boolean) { true, false }, "
									 "width = (int) { 8, 16, 24, 32 }, " "depth = (int) [ 1, 32 ]");
        }
      }
      if ((ppszCodecName != NULL) && (pstAudioInfo != NULL)) {
        *ppszCodecName = g_strdup_printf ("Uncompressed %d-bit PCM audio",
            pstAudioInfo->ulSize);
      }
      break;
	  
    case WAVE_FORMAT_ADPCM:
      caps = gst_caps_new_simple ("audio/x-adpcm",
          "layout", G_TYPE_STRING, "microsoft", NULL);
      if (ppszCodecName != NULL){
        *ppszCodecName = g_strdup ("ADPCM audio");
      }
      block_align = TCDMX_TRUE;
      break;

    case WAVE_FORMAT_IEEE_FLOAT:
      rate_max = 192000;
      channels_max = 8;

      if (pstAudioInfo != NULL) {
        gint ba = (gint)pstAudioInfo->ulBlockAlign;
        gint ch = (gint)pstAudioInfo->ulChannels;
        gint wd = ba * 8 / ch;

        caps = gst_caps_new_simple ("audio/x-raw",//1.0 conversion
            "endianness", G_TYPE_INT, G_LITTLE_ENDIAN,
            "channels", G_TYPE_INT, ch, "width", G_TYPE_INT, wd, NULL);

        /* Add default channel layout. In theory this should be done
         * for 1 and 2 channels too but apparently breaks too many
         * things currently. Also we know no default layout for more than
         * 8 channels. */
		#if 0
        if (ch > 2) {
          if (ch > 8)
            GST_WARNING ("don't know default layout for %d channels", ch);
          else if (gst_riff_wave_add_default_channel_layout (caps))
            GST_DEBUG ("using default channel layout for %d channels", ch);
          else
            GST_WARNING ("failed to add channel layout");
        }
		#endif
      } else {
        /* FIXME: this is pretty useless - we need fixed caps */
        caps = gst_caps_from_string ("audio/x-raw, "//1.0 conversion
            "endianness = (int) LITTLE_ENDIAN, " "width = (int) { 32, 64 }");
      }
      if ((ppszCodecName != NULL) && (pstAudioInfo != NULL)) {
        *ppszCodecName = g_strdup_printf ("Uncompressed %d-bit IEEE float audio",
            pstAudioInfo->ulSize);
      }
      break;

    case WAVE_FORMAT_ALAW:
      if (pstAudioInfo != NULL) {
        if (pstAudioInfo->ulBitPerSample != 8U) {
          GST_WARNING ("invalid depth (%d) of alaw audio, overwriting.",
              pstAudioInfo->ulBitPerSample);
          pstAudioInfo->ulBitPerSample = 8;
          pstAudioInfo->ulBlockAlign = (guint)((pstAudioInfo->ulBitPerSample * pstAudioInfo->ulChannels) / 8u);
          pstAudioInfo->ulSize = pstAudioInfo->ulBlockAlign * pstAudioInfo->ulSampleRate;
        }
        if (pstAudioInfo->ulBlockAlign == 0U) {
          GST_WARNING ("fixing blockalign (%d) of alaw audio", pstAudioInfo->ulSize, pstAudioInfo->ulBlockAlign);
          pstAudioInfo->ulBlockAlign = (guint)((pstAudioInfo->ulBitPerSample * pstAudioInfo->ulChannels) / 8u);
        }
        pstAudioInfo->ulSize = pstAudioInfo->ulBlockAlign * pstAudioInfo->ulSampleRate;
      }
      rate_max = 48000;
      caps = gst_caps_new_simple ("audio/x-alaw", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("A-law audio");
      }
      break;

    case WAVE_FORMAT_WMS:
      caps = gst_caps_new_simple ("audio/x-wms", NULL, NULL);
      if (pstAudioInfo != NULL) {
        gst_caps_set_simple (caps,
            "bitrate", G_TYPE_INT, pstAudioInfo->ulSize * 8,
            "depth", G_TYPE_INT, pstAudioInfo->ulBitPerSample, NULL);
      } else {
        gst_caps_set_simple (caps,
            "bitrate", GST_TYPE_INT_RANGE, 0, G_MAXINT, NULL);
      }
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Windows Media Audio Speech");
      }
      block_align = TCDMX_TRUE;
      break;

    case WAVE_FORMAT_MULAW:
      if (pstAudioInfo != NULL) {
        if (pstAudioInfo->ulBitPerSample != 8u) {
          GST_WARNING ("invalid depth (%d) of mulaw audio, overwriting.",
              pstAudioInfo->ulBitPerSample);
          pstAudioInfo->ulBitPerSample = 8;
          pstAudioInfo->ulBlockAlign = (pstAudioInfo->ulBitPerSample * pstAudioInfo->ulChannels) / 8u;
        }
        if (pstAudioInfo->ulBlockAlign == 0u) {
          GST_WARNING ("fixing blockalign (%d) of mulaw audio", pstAudioInfo->ulSize, pstAudioInfo->ulBlockAlign);
          pstAudioInfo->ulBlockAlign = (pstAudioInfo->ulBitPerSample * pstAudioInfo->ulChannels) / 8u;
        }
        pstAudioInfo->ulSize = pstAudioInfo->ulBlockAlign * pstAudioInfo->ulSampleRate;
      }
      rate_max = 48000;
      caps = gst_caps_new_simple ("audio/x-mulaw", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Mu-law audio");
      }
      break;

    case WAVE_FORMAT_DVI_ADPCM:
      rate_max = 48000;
      caps = gst_caps_new_simple ("audio/x-adpcm",
          "layout", G_TYPE_STRING, "dvi", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("DVI ADPCM audio");
      }
      block_align = TCDMX_TRUE;
      break;

    case WAVE_FORMAT_DSP_TRUESPEECH:
      rate_min = 8000;
      rate_max = 8000;
      caps = gst_caps_new_simple ("audio/x-truespeech", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("DSP Group TrueSpeech");
      }
      break;

    case WAVE_FORMAT_GSM610:
    case WAVE_FORMAT_MSN:
      rate_min = 1;
      caps = gst_caps_new_simple ("audio/ms-gsm", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("MS GSM audio");
      }
      break;

    case WAVE_FORMAT_MPEGL1: /* mp1 */
    case WAVE_FORMAT_MPEGL12: /* mp1 or mp2 */
      rate_min = 16000;
      rate_max = 48000;
      caps = gst_caps_new_simple ("audio/mpeg",
          "mpegversion", G_TYPE_INT, 1, "layer", G_TYPE_INT, 2, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("MPEG-1 layer 2");
      }
      break;

    case WAVE_FORMAT_MPEGL3:  /* mp3 */
      rate_min = 8000;
      rate_max = 48000;
      caps = gst_caps_new_simple ("audio/mpeg",
          "mpegversion", G_TYPE_INT, 1, "layer", G_TYPE_INT, 3, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("MPEG-1 layer 3");
      }
      break;

    case WAVE_FORMAT_AMR_NB:  /* amr-nb */
      rate_min = 8000;
      rate_max = 8000;
      channels_max = 1;
      caps = gst_caps_new_simple ("audio/AMR", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("AMR Narrow Band (NB)");
      }
      break;

    case WAVE_FORMAT_AMR_WB:  /* amr-wb */
      rate_min = 16000;
      rate_max = 16000;
      channels_max = 1;
      caps = gst_caps_new_simple ("audio/AMR-WB", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("AMR Wide Band (WB)");
      }
      break;

    case WAVE_FORMAT_VORBIS: /* ogg/vorbis mode */
    case WAVE_FORMAT_VORBIS1: /* ogg/vorbis mode 1 */
    case WAVE_FORMAT_VORBIS2: /* ogg/vorbis mode 2 */
    case WAVE_FORMAT_VORBIS3: /* ogg/vorbis mode 3 */
    case WAVE_FORMAT_VORBIS1PLUS:     /* ogg/vorbis mode 1+ */
    case WAVE_FORMAT_VORBIS2PLUS:     /* ogg/vorbis mode 2+ */
    case WAVE_FORMAT_VORBIS3PLUS:     /* ogg/vorbis mode 3+ */
	  channels_max = 6;
      rate_max = 192000;
      caps = gst_caps_new_simple ("audio/x-vorbis", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Vorbis");
      }
      break;

    case WAVE_FORMAT_TRUEHD_AC3:
      channels_max = 8;
      caps = gst_caps_new_simple ("audio/x-true-hd", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("TrueHD/AC-3 audio");
      }
      break;
    case WAVE_FORMAT_A52:
      channels_max = 8;
      caps = gst_caps_new_simple ("audio/x-ac3", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("AC-3 audio");
      }
      break;
    case WAVE_FORMAT_DTS:
      channels_max = 8;
      caps = gst_caps_new_simple ("audio/x-dts", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("DTS audio");
      }
      /* wavparse is not always able to specify rate/channels for DTS-in-wav */
      rate_chan = TCDMX_FALSE;
      break;
    case WAVE_FORMAT_EAC3:
      channels_max = 8;
      caps = gst_caps_new_simple ("audio/x-eac3", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("EAC-3 audio");
      }
      break;

    case WAVE_FORMAT_TC_AAC:
    case WAVE_FORMAT_AAC:
    case WAVE_FORMAT_AAC_AC:
    case WAVE_FORMAT_AAC_pm:
    {
      channels_max = 8;
      caps = gst_caps_new_simple ("audio/mpeg",
          "mpegversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("MPEG-4 AAC audio");
      }
      break;
    }
    case WAVE_FORMAT_WMAV1:
    case WAVE_FORMAT_WMAV2:
    case WAVE_FORMAT_WMAV3:
    case WAVE_FORMAT_WMAV3_L:
    {
      gint version = ((gint)ulFormatTag - WAVE_FORMAT_WMAV1) + 1;

      channels_max = 8;
      block_align = TCDMX_TRUE;

      caps = gst_caps_new_simple ("audio/x-wma",
          "wmaversion", G_TYPE_INT, version, NULL);

      if (ppszCodecName != NULL) {
        if (ulFormatTag == (guint)WAVE_FORMAT_WMAV3_L) {
          *ppszCodecName = g_strdup ("WMA Lossless");
        } else {
          *ppszCodecName = g_strdup_printf ("WMA Version %d", version + 6);
        }
      }

      if (pstAudioInfo != NULL) {
        gst_caps_set_simple (caps,
            "bitrate", G_TYPE_INT, pstAudioInfo->ulSize * 8,
            "depth", G_TYPE_INT, pstAudioInfo->ulBitPerSample, NULL);
      } else {
        gst_caps_set_simple (caps,
            "bitrate", GST_TYPE_INT_RANGE, 0, G_MAXINT, NULL);
      }
      break;
    }
    case WAVE_FORMAT_SONY_ATRAC3:
      caps = gst_caps_new_simple ("audio/x-vnd.sony.atrac3", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Sony ATRAC3");
      }
      break;

    case WAVE_FORMAT_SIREN:
      caps = gst_caps_new_simple ("audio/x-siren", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Siren7");
      }
      rate_chan = TCDMX_FALSE;
      break;

    case WAVE_FORMAT_ADPCM_IMA_DK4:
      rate_min = 8000;
      rate_max = 96000;
      channels_max = 2;
      caps =
          gst_caps_new_simple ("audio/x-adpcm", "layout", G_TYPE_STRING, "dk4",
          NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("IMA/DK4 ADPCM");
      }
      break;
    case WAVE_FORMAT_ADPCM_IMA_DK3:
      rate_min = 8000;
      rate_max = 96000;
      channels_max = 2;
      caps =
          gst_caps_new_simple ("audio/x-adpcm", "layout", G_TYPE_STRING, "dk3",
          NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("IMA/DK3 ADPCM");
      }
      break;

    case WAVE_FORMAT_ADPCM_IMA_WAV:
      rate_min = 8000;
      rate_max = 96000;
      channels_max = 2;
      caps =
          gst_caps_new_simple ("audio/x-adpcm", "layout", G_TYPE_STRING, "dvi",
          NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("IMA/WAV ADPCM");
      }
      break;

      /* can anything decode these? pitfdll? */
    case WAVE_FORMAT_VOXWARE_AC8:
    case WAVE_FORMAT_VOXWARE_AC10:
    case WAVE_FORMAT_VOXWARE_AC16:
    case WAVE_FORMAT_VOXWARE_AC20:
    case WAVE_FORMAT_VOXWARE_METAVOICE:
    case WAVE_FORMAT_VOXWARE_METASOUND:
    case WAVE_FORMAT_VOXWARE_RT29HW:
    case WAVE_FORMAT_VOXWARE_VR12:
    case WAVE_FORMAT_VOXWARE_VR18:
    case WAVE_FORMAT_VOXWARE_TQ40:
    case WAVE_FORMAT_VOXWARE_TQ60:
    {
      caps = gst_caps_new_simple ("audio/x-voxware",
          "voxwaretype", G_TYPE_INT, (gint) ulFormatTag, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Voxware");
      }
      break;
    }
    case WAVE_FORMAT_APE:
    {
      rate_min = 8000;
      rate_max = 96000;
      channels_max = 2;

      if (pstAudioInfo != NULL)
      {
        caps = gst_caps_new_simple ("audio/x-ape",
               "depth", G_TYPE_INT, (gint) pstAudioInfo->ulBitPerSample, NULL);
      }
      else
      {
        caps = gst_caps_new_simple ("audio/x-ape", NULL, NULL);
      }

      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("APE Audio");
      }
      break;
    }
    case WAVE_FORMAT_FLAC:
      channels_max = 8;
      caps = gst_caps_new_simple ("audio/x-flac", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("FLAC Audio");
      }
      break;

    default:
      if (ulFormatTag == (guint)AUDIO_FOURCC_OPUS)
      {
        channels_max = 6;
        caps = gst_caps_new_simple ("audio/x-opus", NULL, NULL);
        if (ppszCodecName != NULL) {
          *ppszCodecName = g_strdup ("Opus");
        }
        break;
      }
      else {
        GST_WARNING ("Unknown audio tag 0x%04x", ulFormatTag);
        return NULL;
      }
  }

  if (pstAudioInfo != NULL) {
    if (rate_chan != 0) {
      if (pstAudioInfo->ulChannels > (guint)channels_max) {
        goto too_many_channels;
      }
      if ((pstAudioInfo->ulSampleRate < (guint)rate_min) ||
          (pstAudioInfo->ulSampleRate >(guint)rate_max)) {
        goto invalid_rate;
      }

      gst_caps_set_simple (caps,
          "rate", G_TYPE_INT, pstAudioInfo->ulSampleRate,
          "channels", G_TYPE_INT, pstAudioInfo->ulChannels, NULL);
    }
    if (block_align != 0) {
      gst_caps_set_simple (caps,
          "block_align", G_TYPE_INT, pstAudioInfo->ulBlockAlign, NULL);
    }
  } else {
    if (rate_chan != 0) {
      if (rate_min == rate_max) {
        gst_caps_set_simple (caps, "rate", G_TYPE_INT, rate_min, NULL);
      } else {
        gst_caps_set_simple (caps,
            "rate", GST_TYPE_INT_RANGE, rate_min, rate_max, NULL);
      }
      if (channels_max == 1) {
        gst_caps_set_simple (caps, "channels", G_TYPE_INT, 1, NULL);
      } else {
        gst_caps_set_simple (caps,
            "channels", GST_TYPE_INT_RANGE, 1, channels_max, NULL);
      }
    }
    if (block_align != 0) {
      gst_caps_set_simple (caps,
          "block_align", GST_TYPE_INT_RANGE, 1, G_MAXINT, NULL);
    }
  }

  /* extradata */
  if (pstExtraData != NULL) {
    gst_caps_set_simple (caps, "codec_data", GST_TYPE_BUFFER, pstExtraData, NULL);
  }

  return caps;

  /* ERROR */
too_many_channels:
  GST_WARNING
      ("Stream claims to contain %u channels, but format only supports %d",
      pstAudioInfo->ulChannels, channels_max);
  gst_caps_unref (caps);
  return NULL;
invalid_rate:
  GST_WARNING
      ("Stream with sample_rate %u, but format only supports %d .. %d",
      pstAudioInfo->ulSampleRate, rate_min, rate_max);
  gst_caps_unref (caps);
  return NULL;
}


#define gst_tc_toupper(ch) ( ((ch) >= (unsigned int)'a') && ((ch) <= (unsigned int)'z')) ? ((ch) - 32u) : (ch)

GstCaps *
gst_tc_create_video_caps (
	guint32          ulFourCC,
	tc_video_info_t *pstVideoInfo,
    GstBuffer       *pstExtraData, 
    gboolean         bEnableRingMode,
	gchar          **ppszCodecName
	)
{
  GstCaps *caps;
  guint32 fourcc;

  fourcc = ((gst_tc_toupper((ulFourCC >> 0u)& 255u))<<  0u)|
	       ((gst_tc_toupper((ulFourCC >> 8u)& 255u))<<  8u)|
	       ((gst_tc_toupper((ulFourCC >>16u)& 255u))<< 16u)|
	       ((gst_tc_toupper((ulFourCC >>24u)& 255u))<< 24u);

  GST_DEBUG ("video fourcc %" GST_FOURCC_FORMAT, GST_FOURCC_ARGS (ulFourCC));

  switch (fourcc) {
    case FOURCC_MJPG: /* YUY2 MJPEG */
    case FOURCC_AVRn:
    case FOURCC_IJPG:
    case FOURCC_ijpg:
    case FOURCC_dmb1:
    case FOURCC_ACDV:
    case FOURCC_QIVG:
      caps = gst_caps_new_simple ("video/x-jpeg", NULL, NULL); //[20201013 Helena] use JPU
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Motion JPEG");
      }
      break;

    case FOURCC_JPEG: /* generic (mostly RGB) MJPEG */
    case FOURCC_jpeg: /* generic (mostly RGB) MJPEG */
      caps = gst_caps_new_simple ("video/x-jpeg", NULL, NULL); //[20201013 Helena] use JPU
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("JPEG Still Image");
      }
      break;

    case FOURCC_PIXL: /* Miro/Pinnacle fourccs */
    case FOURCC_VIXL: /* Miro/Pinnacle fourccs */
      caps = gst_caps_new_simple ("image/jpeg", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Miro/Pinnacle Motion JPEG");
      }
      break;

    case FOURCC_CJPG:
      caps = gst_caps_new_simple ("image/jpeg", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Creative Webcam JPEG");
      }
      break;

    case FOURCC_SLMJ:
      caps = gst_caps_new_simple ("image/jpeg", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("SL Motion JPEG");
      }
      break;

    case FOURCC_JPGL:
      caps = gst_caps_new_simple ("image/jpeg", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Pegasus Lossless JPEG");
      }
      break;

    case FOURCC_LOCO:
      caps = gst_caps_new_simple ("video/x-loco", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("LOCO Lossless");
      }
      break;

    case FOURCC_SP53:
    case FOURCC_SP54:
    case FOURCC_SP55:
    case FOURCC_SP56:
    case FOURCC_SP57:
    case FOURCC_SP58:
      caps = gst_caps_new_simple ("video/sp5x", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Sp5x-like JPEG");
      }
      break;

    case FOURCC_ZMBV:
      caps = gst_caps_new_simple ("video/x-zmbv", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Zip Motion Block video");
      }
      break;

    case FOURCC_HFYU:
      caps = gst_caps_new_simple ("video/x-huffyuv", NULL, NULL);
      if (pstVideoInfo != NULL) {
        gst_caps_set_simple (caps, "bpp",
            G_TYPE_INT, (int) pstVideoInfo->ulBitPerPixel, NULL);
      }
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Huffman Lossless Codec");
      }
      break;

    case FOURCC_MPEG:
    case FOURCC_mpeg:
    case FOURCC_MPGI:
    case FOURCC_mpg1:
    case FOURCC_MPG1:
    case FOURCC_PIM1:
    case FOURCC_1000:
      caps = gst_caps_new_simple ("video/mpeg",
								  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
								  "mpegversion", G_TYPE_INT, 1, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("MPEG-1 video");
      }
      break;

    case FOURCC_MPG2:
    case FOURCC_mpg2:
    case FOURCC_PIM2:
    case FOURCC_DVR :
    case FOURCC_2000:
      caps = gst_caps_new_simple ("video/mpeg",
								  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
								  "mpegversion", G_TYPE_INT, 2, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("MPEG-2 video");
      }
      break;

    case FOURCC_LMP2:
      caps = gst_caps_new_simple ("video/mpeg",
								  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
								  "mpegversion", G_TYPE_INT, 2, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Lead MPEG-2 video");
      }
      break;

    case FOURCC_H263:
    case FOURCC_h263:
    case FOURCC_i263:
    case FOURCC_U263:
    case FOURCC_viv1:
    case FOURCC_T263:
    case FOURCC_s263:
    case FOURCC_S263:
      caps = gst_caps_new_simple ("video/x-h263",
								  "variant", G_TYPE_STRING, "itu", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("ITU H.26n");
      }
      break;

    case FOURCC_L263:
      // http://www.leadcodecs.com/Codecs/LEAD-H263.htm
      caps = gst_caps_new_simple ("video/x-h263",
								  "variant", G_TYPE_STRING, "lead", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Lead H.263");
      }
      break;

    case FOURCC_M263:
    case FOURCC_m263:
      caps = gst_caps_new_simple ("video/x-h263",
								  "variant", G_TYPE_STRING, "microsoft", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft H.263");
      }
      break;

    case FOURCC_VDOW:
      caps = gst_caps_new_simple ("video/x-h263",
								  "variant", G_TYPE_STRING, "vdolive", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("VDOLive");
      }
      break;

    case FOURCC_VIVO:
      caps = gst_caps_new_simple ("video/x-h263",
								  "variant", G_TYPE_STRING, "vivo", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Vivo H.263");
      }
      break;

    case FOURCC_x263:
      caps = gst_caps_new_simple ("video/x-h263",
								  "variant", G_TYPE_STRING, "xirlink", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Xirlink H.263");
      }
      break;

      /* apparently not standard H.263...? */
    case FOURCC_I263:
      caps = gst_caps_new_simple ("video/x-intel-h263",
								  "variant", G_TYPE_STRING, "intel", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Intel H.263");
      }
      break;

    case FOURCC_VX1K:
      caps = gst_caps_new_simple ("video/x-h263",
								  "variant", G_TYPE_STRING, "lucent", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Lucent VX1000S H.263");
      }
      break;

    case FOURCC_X264:
    case FOURCC_x264:
    case FOURCC_H264:
    case FOURCC_h264:
    case FOURCC_avc1:
    case FOURCC_AVC1:
      caps = gst_caps_new_simple ("video/x-h264",
								  "variant", G_TYPE_STRING, "itu", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("ITU H.264");
      }
      break;

    case FOURCC_MVC:
    case FOURCC_mvc:
      caps = gst_caps_new_simple ("video/x-h264-mvc",
                "variant", G_TYPE_STRING, "itu", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("ITU H.264 Multi-View");
      }
      break;
    
    case FOURCC_h265:
    case FOURCC_H265:
    case FOURCC_hevc:
    case FOURCC_HEVC:
    case FOURCC_hev1:
    case FOURCC_HEV1:
	case FOURCC_hvc1:
	case FOURCC_HVC1:
      caps = gst_caps_new_simple ("video/x-h265", 
                                  "variant", G_TYPE_STRING, "itu", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("ITU H.265");
      }
      break;

    case FOURCC_VSSH:
      caps = gst_caps_new_simple ("video/x-h264",
								  "variant", G_TYPE_STRING, "videosoft", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("VideoSoft H.264");
      }
      break;

    case FOURCC_L264:
      // http://www.leadcodecs.com/Codecs/LEAD-H264.htm
      caps = gst_caps_new_simple ("video/x-h264",
								  "variant", G_TYPE_STRING, "lead", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Lead H.264");
      }
      break;

    case FOURCC_SEDG:
      caps = gst_caps_new_simple ("video/mpeg",
                                  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
                                  "mpegversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Samsung MPEG-4");
      }
      break;

    case FOURCC_M4CC:
      caps = gst_caps_new_simple ("video/mpeg",
                                  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
                                  "mpegversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Divio MPEG-4");
      }
      break;

    case FOURCC_DIV3:
    case FOURCC_div3:
    case FOURCC_DVX3:
    case FOURCC_dvx3:
    case FOURCC_DIV4:
    case FOURCC_div4:
    case FOURCC_DIV5:
    case FOURCC_div5:
    case FOURCC_DIV6:
    case FOURCC_div6:
    case FOURCC_MPG3:
    case FOURCC_mpg3:
    case FOURCC_col0:
    case FOURCC_COL0:
    case FOURCC_col1:
    case FOURCC_COL1:
    case FOURCC_AP41:
      caps = gst_caps_new_simple ("video/x-divx",
								  "divxversion", G_TYPE_INT, 3, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("DivX MS-MPEG-4 Version 3");
      }
      break;

    case FOURCC_divx:
    case FOURCC_DIVX:
      caps = gst_caps_new_simple ("video/x-divx",
								  "divxversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("DivX MPEG-4 Version 4");
      }
      break;

    case FOURCC_BLZ0:
      caps = gst_caps_new_simple ("video/x-divx",
								  "divxversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Blizzard DivX");
      }
      break;

    case FOURCC_DX50:
      caps = gst_caps_new_simple ("video/x-divx",
								  "divxversion", G_TYPE_INT, 5, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("DivX MPEG-4 Version 5");
      }
      break;

    case FOURCC_XVID:
    case FOURCC_xvid:
      caps = gst_caps_new_simple ("video/x-xvid", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("XVID MPEG-4");
      }
      break;

    case FOURCC_RMP4:
      caps = gst_caps_new_simple ("video/x-xvid", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Sigma-Designs MPEG-4");
      }
      break;

    case FOURCC_MPG4:
    case FOURCC_MP41:
    case FOURCC_mp41:
      caps = gst_caps_new_simple ("video/x-msmpeg",
								  "msmpegversion", G_TYPE_INT, 41, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft MPEG-4 4.1");
      } 
      break;

    case FOURCC_mp42:
    case FOURCC_MP42:
      caps = gst_caps_new_simple ("video/x-msmpeg",
								  "msmpegversion", G_TYPE_INT, 42, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft MPEG-4 4.2");
      }
      break;

    case FOURCC_mp43:
    case FOURCC_MP43:
      caps = gst_caps_new_simple ("video/x-msmpeg",
								  "msmpegversion", G_TYPE_INT, 43, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft MPEG-4 4.3");
      }
      break;

    case FOURCC_MP4S:
    case FOURCC_M4S2:
      caps = gst_caps_new_simple ("video/mpeg",
                                  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
                                  "mpegversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft ISO MPEG-4 1.1");
      }
      break;

    case FOURCC_FMP4:
    case FOURCC_UMP4:
    case FOURCC_FFDS:
      caps = gst_caps_new_simple ("video/mpeg",
                                  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
                                  "mpegversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("FFmpeg MPEG-4");
      }
      break;

    case FOURCC_EM4A:
    case FOURCC_EPVH:
    case FOURCC_FVFW:
    case FOURCC_INMC:
    case FOURCC_DIGI:
    case FOURCC_DM2K:
    case FOURCC_DCOD:
    case FOURCC_MVXM:
    case FOURCC_PM4V:
    case FOURCC_SMP4:
    case FOURCC_DXGM:
    case FOURCC_VIDM:
    case FOURCC_M4T3:
    case FOURCC_GEOX:
    case FOURCC_MP4V:
    case FOURCC_mp4v:
      caps = gst_caps_new_simple ("video/mpeg",
                                  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
                                  "mpegversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("MPEG-4");
      }
      break;

    case FOURCC_3ivd:
    case FOURCC_3IVD:
      caps = gst_caps_new_simple ("video/x-msmpeg",
								  "msmpegversion", G_TYPE_INT, 43, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft MPEG-4 4.3");        /* FIXME? */
      }
      break;

    case FOURCC_3IV1:
    case FOURCC_3IV2:
      caps = gst_caps_new_simple ("video/x-3ivx", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("3ivx");
      }
      break;

    case FOURCC_DVSD:
    case FOURCC_dvsd:
    case FOURCC_dvc :
    case FOURCC_dv25:
      caps = gst_caps_new_simple ("video/x-dv",
								  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
								  "dvversion", G_TYPE_INT, 25, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Generic DV");
      }
      break;

    case FOURCC_CDVC:
    case FOURCC_cdvc:
      caps = gst_caps_new_simple ("video/x-dv",
								  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
								  "dvversion", G_TYPE_INT, 25, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Canopus DV");
      }
      break;

    case FOURCC_DV50:
    case FOURCC_dv50:
      caps = gst_caps_new_simple ("video/x-dv",
								  "systemstream", G_TYPE_BOOLEAN, TCDMX_FALSE,
								  "dvversion", G_TYPE_INT, 50, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("DVCPro50 Video");
     }
      break;

    case FOURCC_WMV1:
      caps = gst_caps_new_simple ("video/x-wmv",
								  "wmvversion", G_TYPE_INT, 1, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft Windows Media 7");
      }
      break;

    case FOURCC_WMV2:
      caps = gst_caps_new_simple ("video/x-wmv",
								  "wmvversion", G_TYPE_INT, 2, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft Windows Media 8");
      }
      break;

    case FOURCC_WMV3:
      caps = gst_caps_new_simple ("video/x-wmv",
								  "wmvversion", G_TYPE_INT, 3, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft Windows Media 9");
      }
      break;

    case FOURCC_WMVA:
      caps = gst_caps_new_simple ("video/x-wmv",
								  "wmvversion", G_TYPE_INT, 3, 
								  "format", G_TYPE_STRING, "WMVA", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft Windows Media Advanced Profile");
      }
      break;

    case FOURCC_WVC1:
    case FOURCC_wvc1:
      caps = gst_caps_new_simple ("video/x-wmv",
					  "wmvversion", G_TYPE_INT, 3, 
					  "format", G_TYPE_STRING, "WVC1", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Microsoft Windows Media VC-1");
      }
      break;

	case FOURCC_AVS:
	case FOURCC_avs:
	case FOURCC_CAVS:
	case FOURCC_cavs:
      caps = gst_caps_new_simple ("video/x-avs", 
					"format", G_TYPE_STRING, "AVS ", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Audio Video Standard");
      }
      break;

    case FOURCC_cvid:
      caps = gst_caps_new_simple ("video/x-cinepak", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Cinepak video");
      }
      break;

    case FOURCC_AASC:
      caps = gst_caps_new_simple ("video/x-aasc", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Autodesk Animator");
      }
      break;

    case FOURCC_Xxan:
      caps = gst_caps_new_simple ("video/x-xan",
          "wcversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Xan Wing Commander 4");
      }
      break;

    case FOURCC_RT21:
      caps = gst_caps_new_simple ("video/x-indeo",
				  "indeoversion", G_TYPE_INT, 2, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Intel Video 2");
      }
      break;

    case FOURCC_IV31:
    case FOURCC_IV32:
    case FOURCC_iv31:
    case FOURCC_iv32:
      caps = gst_caps_new_simple ("video/x-indeo",
				  "indeoversion", G_TYPE_INT, 3, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Intel Video 3");
      }
      break;

    case FOURCC_IV41:
    case FOURCC_iv41:
      caps = gst_caps_new_simple ("video/x-indeo",
				  "indeoversion", G_TYPE_INT, 4, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Intel Video 4");
      }
      break;

    case FOURCC_IV50:
      caps = gst_caps_new_simple ("video/x-indeo",
          "indeoversion", G_TYPE_INT, 5, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Intel Video 5");
      }
      break;

    case FOURCC_MSZH:
      caps = gst_caps_new_simple ("video/x-mszh", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Lossless MSZH Video");
      }
      break;

    case FOURCC_ZLIB:
      caps = gst_caps_new_simple ("video/x-zlib", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Lossless zlib video");
      }
      break;

    case FOURCC_CLJR:
    case FOURCC_cljr:
      caps = gst_caps_new_simple ("video/x-cirrus-logic-accupak", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Cirrus Logipak AccuPak");
      }
      break;

    case FOURCC_CYUV:
    case FOURCC_cyuv:
      caps = gst_caps_new_simple ("video/x-compressed-yuv", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("CYUV Lossless");
      }
      break;

    case FOURCC_DUCK:
    case FOURCC_PVEZ:
      caps = gst_caps_new_simple ("video/x-truemotion",
          "trueversion", G_TYPE_INT, 1, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Duck Truemotion1");
      }
      break;

    case FOURCC_TM20:
      caps = gst_caps_new_simple ("video/x-truemotion",
          "trueversion", G_TYPE_INT, 2, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("TrueMotion 2.0");
      }
      break;

    case FOURCC_VP30:
    case FOURCC_vp30:
    case FOURCC_VP31:
    case FOURCC_vp31:
    case FOURCC_VP3 :
      caps = gst_caps_new_simple ("video/x-vp3", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("VP3");
      }
      break;

    case FOURCC_ULTI:
      caps = gst_caps_new_simple ("video/x-ultimotion", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("IBM UltiMotion");
      }
      break;

    case FOURCC_TSCC:
    case FOURCC_tscc:
    {
      if (pstVideoInfo != NULL) {
        gint depth = (pstVideoInfo->ulBitPerPixel != 0) ? (gint) pstVideoInfo->ulBitPerPixel : 24;

        caps = gst_caps_new_simple ("video/x-camtasia", "depth", G_TYPE_INT,
            depth, NULL);
      } else {
        /* template caps */
        caps = gst_caps_new_simple ("video/x-camtasia", NULL, NULL);
      }
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("TechSmith Camtasia");
      }
      break;
    }

    case FOURCC_CSCD:
    {
      if (pstVideoInfo != NULL) {
        gint depth = (pstVideoInfo->ulBitPerPixel != 0U) ? (gint) pstVideoInfo->ulBitPerPixel : 24;

        caps = gst_caps_new_simple ("video/x-camstudio", "depth", G_TYPE_INT,
            depth, NULL);
      } else {
        /* template caps */
        caps = gst_caps_new_simple ("video/x-camstudio", NULL, NULL);
      }
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Camstudio");
      }
      break;
    }

    case FOURCC_VCR1:
      caps = gst_caps_new_simple ("video/x-ati-vcr",
          "vcrversion", G_TYPE_INT, 1, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("ATI VCR 1");
      }
      break;

    case FOURCC_VCR2:
      caps = gst_caps_new_simple ("video/x-ati-vcr",
          "vcrversion", G_TYPE_INT, 2, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("ATI VCR 2");
      }
      break;

    case FOURCC_ASV1:
      caps = gst_caps_new_simple ("video/x-asus",
          "asusversion", G_TYPE_INT, 1, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Asus Video 1");
      }
      break;

    case FOURCC_ASV2:
      caps = gst_caps_new_simple ("video/x-asus",
          "asusversion", G_TYPE_INT, 2, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Asus Video 2");
      }
      break;

    case FOURCC_MPNG:
    case FOURCC_mpng:
    case FOURCC_PNG :
      caps = gst_caps_new_simple ("image/png", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("PNG image");
      }
      break;

    case FOURCC_FLV1:
      caps = gst_caps_new_simple ("video/x-flash-video",
          "flvversion", G_TYPE_INT, 1, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Flash Video 1");
      }
      break;

    case FOURCC_VMnc:
      caps = gst_caps_new_simple ("video/x-vmnc",
          "version", G_TYPE_INT, 1, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("VMWare NC Video");
      }
      break;

    case FOURCC_drac:
      caps = gst_caps_new_simple ("video/x-dirac", NULL, NULL);
      if (ppszCodecName != NULL){
        *ppszCodecName = g_strdup ("Dirac");
      }
      break;

    case FOURCC_rpza:
    case FOURCC_azpr:
    case FOURCC_RPZA:
      caps = gst_caps_new_simple ("video/x-apple-video", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Apple Video (RPZA)");
      }
      break;


    case FOURCC_FFV1:
      caps = gst_caps_new_simple ("video/x-ffv",
          "ffvversion", G_TYPE_INT, 1, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("FFmpeg lossless video codec");
      }
      break;

    case FOURCC_KMVC:
      caps = gst_caps_new_simple ("video/x-kmvc", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Karl Morton's video codec");
      }
      break;

    case FOURCC_vp50:
    case FOURCC_VP50:
      caps = gst_caps_new_simple ("video/x-vp5", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("On2 VP5");
      }
      break;

    case FOURCC_vp60:
    case FOURCC_VP60:
    case FOURCC_vp61:
    case FOURCC_VP61:
    case FOURCC_Vp62:
    case FOURCC_VP62:
      caps = gst_caps_new_simple ("video/x-vp6", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("On2 VP6");
      }
      break;

    case FOURCC_VP6F:
    case FOURCC_vp6f:
    case FOURCC_FLV4:
      caps = gst_caps_new_simple ("video/x-vp6-flash", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("On2 VP6");
      }
      break;

    case FOURCC_vp70:
    case FOURCC_VP70:
      caps = gst_caps_new_simple ("video/x-vp7", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("On2 VP7");
      }
      break;

    case FOURCC_VP80:
      caps = gst_caps_new_simple ("video/x-vp8", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("On2 VP8");
      }
      break;

    case FOURCC_vp90:
    case FOURCC_VP90:
      caps = gst_caps_new_simple ("video/x-vp9", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("On2 VP9");
      }
      break;
    case FOURCC_LM20:
      caps = gst_caps_new_simple ("video/x-mimic", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Mimic webcam");
      }
      break;

    case FOURCC_THEO:
    case FOURCC_theo:
      caps = gst_caps_new_simple ("video/x-theora", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Theora video codec");
      }
      break;

    case FOURCC_FPS1:
      caps = gst_caps_new_simple ("video/x-fraps", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Fraps video");
      }
      break;

    default:
      GST_WARNING ("Unknown video fourcc %" GST_FOURCC_FORMAT,
          GST_FOURCC_ARGS (ulFourCC));
      return NULL;
  }

  if (pstVideoInfo != NULL) {
    gst_caps_set_simple (caps,
        "width", G_TYPE_INT, pstVideoInfo->ulWidth,
        "height", G_TYPE_INT, pstVideoInfo->ulHeight,
		"framerate", GST_TYPE_FRACTION, pstVideoInfo->ulFrameRate*1000, 1000000,
		NULL);

  } else {
    gst_caps_set_simple (caps,
        "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
        "height", GST_TYPE_INT_RANGE, 1, G_MAXINT,
		"framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1,
		NULL);
  }

  /* extradata */
  if (pstExtraData != NULL) {
    gst_caps_set_simple (caps, "codec_data", GST_TYPE_BUFFER, pstExtraData, NULL);
  }

  if (bEnableRingMode == TCDMX_TRUE) {
    gst_caps_set_simple (caps, "ring-mode", G_TYPE_BOOLEAN, TCDMX_TRUE, NULL);
  }
  return caps;
}

GstCaps *
gst_tc_create_subtitle_caps (
	guint32          ulFormatId,
	gchar           *szMimeType,
	GstBuffer       *pstExtraData, 
	gchar          **ppszCodecName
	)
{
  GstCaps *caps;

  switch ((subtitle_format_t)ulFormatId) {
    case SUBTITLE_FORMAT_ID_NONE:
      caps = gst_caps_new_simple (szMimeType, NULL, NULL);
      break;
    case SUBTITLE_FORMAT_ID_ASCII:
    case SUBTITLE_FORMAT_ID_UTF8:
    case SUBTITLE_FORMAT_ID_SRT:
      caps = gst_caps_new_simple ("text/x-raw", "format", G_TYPE_STRING, "pango-markup", NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Pango Markup");
      }
      break;
    case SUBTITLE_FORMAT_ID_SSA:
      caps = gst_caps_new_simple ("application/x-ssa", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("SubStation Alpha");
      }
      break;
    case SUBTITLE_FORMAT_ID_ASS:
      caps = gst_caps_new_simple ("application/x-ass", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Advanced SubStation Alpha");
      }
      break;
    case SUBTITLE_FORMAT_ID_USF:
      caps = gst_caps_new_simple ("application/x-usf", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Universal Subtitle Format");
      }
      break;
    case SUBTITLE_FORMAT_ID_KATE:
      caps = gst_caps_new_simple ("subtitle/x-kate", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Kate");
      }
      break;
    case SUBTITLE_FORMAT_ID_VOBSUB:
      caps = gst_caps_new_simple ("subpicture/x-dvd", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("VOB Subtitle");
      }
      break;
    case SUBTITLE_FORMAT_ID_HDMVPGS:
      caps = gst_caps_new_simple ("subpicture/x-pgs", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("HDMV PGS");
      }
      break;
    case SUBTITLE_FORMAT_ID_BMP:
    default:
      caps = gst_caps_new_simple ("application/x-subtitle-unknown", NULL, NULL);
      if (ppszCodecName != NULL) {
        *ppszCodecName = g_strdup ("Unknown");
      }
      break;
  }

  /* extradata */
  if (pstExtraData != NULL) {
    gst_caps_set_simple (caps, "codec_data", GST_TYPE_BUFFER, pstExtraData, NULL);
  }

  return caps;
}

GstCaps *
gst_tc_create_audio_template_caps (
	const guint16         *pusFormatTagArray
	)
{
	guint32 tag;
	GstCaps *caps, *one;
	const guint16 *local_FormatTagArray = pusFormatTagArray;

	caps = gst_caps_new_empty ();
	tag = *local_FormatTagArray;
	local_FormatTagArray++;
	while( tag != 0U ) {
		one = gst_tc_create_audio_caps (tag, NULL, TCDMX_FALSE, NULL);
		if (one != NULL){
			gst_caps_append (caps, one);
		}
		tag = *local_FormatTagArray;
		local_FormatTagArray++;
	}

	return caps;
}


GstCaps *
gst_tc_create_video_template_caps (
	const guint32         *pulFourCCArray
	)
{
	guint32 fcc;
	GstCaps *caps, *one;
	const guint32 *local_FourCCArray = pulFourCCArray;

	caps = gst_caps_new_empty ();
	fcc = *local_FourCCArray;
	local_FourCCArray++;
	while( fcc != 0U ) {
		one = gst_tc_create_video_caps (fcc, NULL, NULL, TCDMX_FALSE, NULL);
		if (one != NULL){
			gst_caps_append (caps, one);
		}
		fcc = *local_FourCCArray;
		local_FourCCArray++;
	}

	return caps;
}
