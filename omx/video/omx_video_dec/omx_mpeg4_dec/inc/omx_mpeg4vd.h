/* ====================================================================
 *   Copyright (C) 2010 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
 * ==================================================================== */
/*
*   @file  omx_mpeg4vd.h
*   This file is the header file for codec specific layer of MPEG4 decoder
*
*   @path \WTSD_DucatiMMSW\omx\omx_il_1_x\omx_video_dec\omx_mpeg4_dec\inc
*
*  @rev 1.0
*/
 #ifndef _OMX_MPEG4VD_NEW_H_
 #define _OMX_MPEG4VD_NEW_H_

 #ifdef __cplusplus
extern "C" {
 #endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
******************************************************************/

#include <omx_video_decoder.h>
#include <OMX_TI_Video.h>
#include <ti/sdo/codecs/mpeg4dec/impeg4vdec.h>

/******************************************************************
 *  CONSTANTS
 ******************************************************************/
/*! Padding for width as per Codec Requirement */
#define PADX (32)
/*! Padding for height as per Codec requirement */
#define PADY (32)

/*! OMX MPEG4 Video Decoder component's name */
#define OMX_MPEG4VD_COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.MPEG4D"



/*----------          function prototypes      ------------------- */

/*===============================================================*/
/** @fn OMX_MPEG4VD_Set_StaticParams:
 *      To initialize the static parameters of MPEG4 Decoder
 *
 * @param pstaticParams : Static parameters of VC1 Decoder
 * @param width         : Max width of frame
 * @param height        : Max height of frame
 */
/*===============================================================*/

void OMX_MPEG4VD_Set_StaticParams(OMX_HANDLETYPE hComponent, void *staticparams);

/*===============================================================*/
/** @fn OMX_MPEG4VD_Set_DynamicParams:
 *      To initialize Dynamic parameters of the MPEG4 Decoder
 *
 * @param pdynamicParams : Dynamic parameters of MPEG4 Decoder
 */
/*===============================================================*/
void OMX_MPEG4VD_Set_DynamicParams(OMX_HANDLETYPE hComponent, void *dynamicParams);

/*===============================================================*/
/** @fn OMX_MPEG4VD_Set_Status:
 *      To initialize the Status structure of the MPEG4 Decoder
 *
 * @param pStatus : Pointer to the Status parameters of VC1 dec
 */
/*===============================================================*/
void OMX_MPEG4VD_Set_Status(OMX_HANDLETYPE hComponent, void *status);

/*===============================================================*/
/** @fn Calculate_TI_MPEG4VD_outbuff_details:
 *      To initialize the output buffer details for MPEG4 Decoder
 *      component output port
 *  @param nWidth  : Width of buffer
 *  @param nHeight : height of Buffer
 *
 */
/*===============================================================*/
BUFF_PARAMS Calculate_TI_MPEG4VD_outbuff_details(OMX_HANDLETYPE hComponent,
                                                 OMX_U32 width, OMX_U32 height);

/*----------          function prototypes      ------------------- */
extern OMX_ERRORTYPE OMX_TI_VideoDecoder_ComponentInit(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE OMX_MPEG4VD_Init(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE OMX_H263VD_Init(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE OMX_SORENSONSPKVD_Init(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE OMX_MPEG4_H263VD_Init(OMX_HANDLETYPE hComponent);
void OMX_MPEG4VD_DeInit(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE OMX_TI_MPEG4VD_HandleError(OMX_HANDLETYPE hComponent);

extern OMX_ERRORTYPE OMX_TI_MPEG4VD_SetParameter(OMX_HANDLETYPE hComponent,
                                                 OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);
extern OMX_ERRORTYPE OMX_TI_MPEG4VD_GetParameter(OMX_HANDLETYPE hComponent,
                                                 OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);

extern OMX_ERRORTYPE OMX_TI_VIDEODECODER_SetParameter(OMX_HANDLETYPE hComponent,
                                                      OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);
extern OMX_ERRORTYPE OMX_TI_VIDEODECODER_GetParameter(OMX_HANDLETYPE hComponent,
                                                      OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);


typedef struct OMX_MPEG4VD_PVTTYPE {
    OMX_PARAM_DEBLOCKINGTYPE         tDeblockingParam;
    OMX_VIDEO_PARAM_MPEG4TYPE        tMPEG4VideoParam;
    OMX_BOOL                         bIsSorensonSpark;
    OMX_TI_VIDEO_PARAM_DEBLOCKINGQP  tDeblockingQpParam;
} OMX_MPEG4VD_PVTTYPE;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_MPEG4VD_NEW_H_ */

