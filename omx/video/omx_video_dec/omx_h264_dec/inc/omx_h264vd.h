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

#ifndef _OMX_H264VD_NEW_H_
#define _OMX_H264VD_NEW_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/******************************************************************
  *   INCLUDE FILES
 ******************************************************************/
#include <omx_video_decoder.h>
#include <ti/sdo/codecs/h264dec/ih264vdec.h>

/******************************************************************
 *  CONSTANTS
 ******************************************************************/
/*! Padding for width as per Codec Requirement */
 #define PADX  (32)
/*! Padding for height as per Codec requirement */
 #define PADY  (24)

/** OMX H264 Video Decoder component's name */
 #define OMX_H264VD_COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.H264D"

/** Default Frame skip H264 Decoder */
#define H264VD_DEFAULT_FRAME_SKIP              IVIDEO_SKIP_DEFAULT

/*----------          function prototypes      ------------------- */
/* ==========================================================================*/
/**
 * @fn OMX_TI_H264VD_GetParameter()
 *                     This method is called by the IL Client to get parameter
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] nIndex      : Index of the parameter to get
 *  @param [in] pParamStruct: pointer to the paramter structure
 *
 */
/* ==========================================================================*/
static OMX_ERRORTYPE OMX_TI_H264VD_GetParameter(OMX_HANDLETYPE hComponent,
                                                OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);

/* ==========================================================================*/
/**
 * @fn OMX_TI_H264VD_SetParameter()
 *                     This method is called by the IL Client to set parameter
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] nIndex      : Index of the parameter to be set
 *  @param [in] pParamStruct: pointer to the paramter structure to be set
 *
 */
/* ==========================================================================*/

static OMX_ERRORTYPE OMX_TI_H264VD_SetParameter(OMX_HANDLETYPE hComponent,
                                                OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);

/*===============================================================*/
/** @fn OMX_TI_H264VD_Set_StaticParams:
 *      To initialize the static parameters of H264 Decoder
 *
 * @param pstaticParams : Static parameters of H264 Decoder
 * @param width         : Max width of frame
 * @param height        : Max height of frame
 */
/*===============================================================*/
void OMX_TI_H264VD_Set_StaticParams(OMX_HANDLETYPE hComponent, void *staticparams);

/*===============================================================*/
/** @fn OMX_TI_H264VD_Set_DynamicParams:
 *      To initialize Dynamic parameters of the H264 Decoder
 *
 * @param pdynamicParams : Dynamic parameters of H264 Decoder
 */
/*===============================================================*/
void OMX_TI_H264VD_Set_DynamicParams(OMX_HANDLETYPE hComponent, void *dynamicParams);

/*===============================================================*/
/** @fn OMX_TI_H264VD_Set_Status:
 *      To initialize the Status structure of the H264 Decoder
 *
 * @param pStatus : Pointer to the Status parameters of H264 dec
 */
/*===============================================================*/
void OMX_TI_H264VD_Set_Status(OMX_HANDLETYPE hComponent, void *status);

/*===============================================================*/
/** @fn Calculate_TI_H264VD_outbuff_details:
 *      To initialize the output buffer details for H264 Decoder
 *      component output port
 *  @param nWidth  : Width of buffer
 *  @param nHeight : height of Buffer
 *
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_TI_H264VD_HandleError(OMX_HANDLETYPE hComponent);
BUFF_PARAMS Calculate_TI_H264VD_outbuff_details(OMX_HANDLETYPE hComponent,
                                                OMX_U32 width, OMX_U32 height);

/*----------          function prototypes      ------------------- */
extern OMX_ERRORTYPE OMX_TI_VideoDecoder_ComponentInit(OMX_HANDLETYPE hComponent);
extern OMX_ERRORTYPE OMX_H264VD_Init(OMX_HANDLETYPE hComponent);
extern void OMX_H264VD_DeInit(OMX_HANDLETYPE hComponent);
extern OMX_U32 OMX_TI_H264VD_Calculate_TotalRefFrames(OMX_U32 nWidth, OMX_U32 nHeight, OMX_VIDEO_AVCLEVELTYPE eLevel);

typedef struct OMX_H264VD_PVTTYPE {
    OMX_VIDEO_PARAM_AVCTYPE tH264VideoParam;
} OMX_H264VD_PVTTYPE;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_H2644VD_NEW_H_ */

