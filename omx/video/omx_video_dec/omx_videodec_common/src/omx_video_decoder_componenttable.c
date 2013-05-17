/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <omx_video_decoder_componenttable.h>

/****************************************************************
 * EXTERNAL REFERENCES
 ****************************************************************/
/*--------function prototypes ---------------------------------*/

extern OMX_ERRORTYPE OMX_H264VD_Init(OMX_HANDLETYPE hComponent);
extern OMX_ERRORTYPE OMX_MPEG4VD_Init(OMX_HANDLETYPE hComponent);
/*
extern OMX_ERRORTYPE OMX_H263VD_Init(OMX_HANDLETYPE hComponent);
extern OMX_ERRORTYPE OMX_SORENSONSPKVD_Init(OMX_HANDLETYPE hComponent);
extern OMX_ERRORTYPE OMX_H264VD_Init(OMX_HANDLETYPE hComponent);
extern OMX_ERRORTYPE OMX_VC1VD_Init(OMX_HANDLETYPE hComponent);
extern OMX_ERRORTYPE OMX_MPEG2VD_Init(OMX_HANDLETYPE hComponent);
extern OMX_ERRORTYPE OMX_H264SVCVD_Init(OMX_HANDLETYPE hComponent);
extern OMX_ERRORTYPE OMX_MJPEGVD_Init(OMX_HANDLETYPE hComponent);
*/
/****************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ****************************************************************/
/*--------data declarations -----------------------------------*/

/* ==========================================================================*/
/*
 * OMXDecoderComponentList Structure maintain the table for decoder names and
 * function pointer for the respective decoder init functions.
 *
 */
/* ==========================================================================*/

OMXDecoderComponentList    DecoderList[] =
{
    { "video_decoder.mpeg4", OMX_VIDEO_CodingMPEG4, OMX_MPEG4VD_Init },           /*! MPEG4 Decoder */
   // { "video_decoder.h263", OMX_VIDEO_CodingH263, OMX_H263VD_Init },           /*! H263 Decoder */
   // { "video_decoder.sorspk", (OMX_VIDEO_CODINGTYPE)OMX_TI_VIDEO_CodingSORENSONSPK, OMX_SORENSONSPKVD_Init },           /*! Sorenson spark Decoder */
    { "video_decoder.avc", OMX_VIDEO_CodingAVC, OMX_H264VD_Init },              /*! H264  Decoder */
   // { "video_decoder.wmv", OMX_VIDEO_CodingWMV, OMX_VC1VD_Init },                /*! VC1 Decoder */
   // { "video_decoder.mpeg2", OMX_VIDEO_CodingMPEG2, OMX_MPEG2VD_Init },           /*! MPEG2 Decoder */
   // { "video_decoder.svc", (OMX_VIDEO_CODINGTYPE)OMX_VIDEO_CodingSVC, OMX_H264SVCVD_Init },               /*! H264  Decoder */
   // { "video_decoder.mjpeg", OMX_VIDEO_CodingMJPEG, OMX_MJPEGVD_Init },              /*! MJPEG  Decoder */
    { NULL, 0, NULL }
};

