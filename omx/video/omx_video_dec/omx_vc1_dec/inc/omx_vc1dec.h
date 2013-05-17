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
*   @file  omx_vc1dec.h
*   This file is the header file for codec specific layer of VC1 decoder
*
*   @path \WTSD_DucatiMMSW\omx\omx_il_1_x\omx_video_dec\omx_vc1_dec\inc
*
*  @rev 1.0
*/
#ifndef _OMX_VC1DEC_H_
#define _OMX_VC1DEC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <omx_video_decoder.h>
#include <ti/sdo/codecs/vc1vdec/ivc1vdec.h>

/******************************************************************
 *  CONSTANTS
 ******************************************************************/

#define VC1_DEC_MAX_FRAME_WIDTH  (1920)
#define VC1_DEC_MAX_FRAME_HEIGHT (1080)
#define VC1_DEC_MAX_FRAME_RATE   (30 * 1000)
#define VC1_DEC_MAX_BIT_RATE     (45000000)
#define VC1_DEC_PADX         (32)
#define VC1_DEC_PADY         (40)
#define VC1_DEC_MIN_BUF_COUNT    (5)
#define VC1_DEC_ACTUAL_BUF_COUNT (8)
#define VC1_DEC_1D_BUF_ALIGNMENT (16)
#define VC1_DEC_2D_Y_ALIGNMENT (1)
#define VC1_DEC_2D_X_ALIGNMENT  (16)

#define OMX_TI_VC1D_COMPONENT_NAME  "OMX.TI.DUCATI1.VIDEO.VC1D"
#define VC1_MEMALLOC(x) TIMM_OSAL_Malloc((x), TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT)
#define VC1_NULLPTRCHECK(x) OMX_BASE_ASSERT((x) != NULL, OMX_ErrorInsufficientResources)
#define VC1_MEMSET_0(x, y) TIMM_OSAL_Memset((x), 0x0, (y))


/*----------          function prototypes      ------------------- */

/*===============================================================*/
/** @fn OMX_TI_VC1VD_Set_StaticParams:
 *      To initialize the static parameters of VC1 Decoder
 *
 * @param pstaticParams : Static parameters of VC1 Decoder
 * @param width         : Max width of frame
 * @param height        : Max height of frame
 */
/*===============================================================*/
void OMX_TI_VC1VD_Set_StaticParams(OMX_HANDLETYPE hComponent, void *pstaticParams);

/*===============================================================*/
/** @fn OMX_TI_VC1VD_Set_DynamicParams:
 *      To initialize Dynamic parameters of the VC1 Decoder
 *
 * @param pdynamicParams : Dynamic parameters of VC1 Decoder
 */
/*===============================================================*/
void OMX_TI_VC1VD_Set_DynamicParams(OMX_HANDLETYPE hComponent, void *pdynamicParams);

/*===============================================================*/
/** @fn OMX_TI_VC1VD_Set_Status:
 *      To initialize the Status structure of the VC1 Decoder
 *
 * @param pStatus : Pointer to the Status parameters of VC1 dec
 */
/*===============================================================*/
void OMX_TI_VC1VD_Set_Status(OMX_HANDLETYPE hComponent, void *pStatus);

/*===============================================================*/
/** @fn OMX_TI_VC1VD_Calc_Outbuf_details:
 *      To initialize the output buffer details for VC1 Decoder
 *      component output port
 *  @param nWidth  : Width of buffer
 *  @param nHeight : height of Buffer
 *
 */
/*===============================================================*/
BUFF_PARAMS OMX_TI_VC1VD_Calc_Outbuf_details(OMX_HANDLETYPE hComponent,
                                             OMX_U32 nWidth, OMX_U32 nHeight);

/*===============================================================*/
/** @fn OMX_VC1VD_Init:
 *      To initialize the VC1 Decoder Component
 *
 *  @param hComponent : OMX Handle of the component
 *
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_VC1VD_Init(OMX_HANDLETYPE hComponent);

/*===============================================================*/
/** @fn OMX_TI_VC1D_DeInit:
 *      To de-initialize the VC1 Decoder Component
 *
 *  @param hComponent : OMX Handle of the component
 *
 */
/*===============================================================*/
void OMX_TI_VC1D_DeInit(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE OMX_TI_VC1VD_HandleError(OMX_HANDLETYPE hComponent);

/*----------          function prototypes      ------------------- */
extern
OMX_ERRORTYPE OMX_TI_VideoDecoder_ComponentInit(OMX_HANDLETYPE hComponent);

extern OMX_ERRORTYPE OMX_TI_VIDEODECODER_SetParameter(OMX_HANDLETYPE hComponent,
                                                      OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);

extern OMX_ERRORTYPE OMX_TI_VIDEODECODER_GetParameter(OMX_HANDLETYPE hComponent,
                                                      OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);

extern OMX_ERRORTYPE OMX_TI_VC1VD_GetParameter(OMX_HANDLETYPE hComponent,
                                               OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);
extern OMX_ERRORTYPE OMX_TI_VC1VD_SetParameter(OMX_HANDLETYPE hComponent,
                                               OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);
typedef struct OMX_VC1VD_PVTTYPE {
    OMX_VIDEO_PARAM_WMVTYPE tVC1VideoParam;
} OMX_VC1VD_PVTTYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_VC1DEC_H_ */

