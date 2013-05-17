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

#ifndef _OMX_VIDEO_DECODER_INTERNAL_H_
#define _OMX_VIDEO_DECODER_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <timm_osal_trace.h>
#include <OMX_Core.h>
#include <OMX_TI_Index.h>
#include <OMX_TI_IVCommon.h>
#include <OMX_Component.h>
#include <omx_base.h>
#include <OMX_Types.h>
#include <omx_base_utils.h>
#include <timm_osal_interfaces.h>
#include <omx_video_decoder.h>
#include <omx_video_decoder_componenttable.h>


#ifdef OMX_TILERTEST
    #define VIDEODECODER_TILERTEST
#endif

/** Default context Type as HOST CONTEXT(0) */
#define OMX_VIDDEC_DEFAULT_CONTEXT_TYPE OMX_BASE_CONTEXT_ACTIVE

/** Priority of the OMX_VIDEODECODER Comp  */
#define OMX_VIDDEC_TASK_PRIORITY               (10)

/** Stack Size of the OMX_VIDEODECODER Comp*/
#define OMX_VIDDEC_TASK_STACKSIZE              (50 * 1024)

/** Group Id of the OMX_VIDEODECODER Comp */
#define OMX_VIDDEC_GROUPID                      (0)

/** Priority of the OMX VIDEODECODER Comp  */
#define OMX_VIDDEC_GROUPPRIORITY                (2)

#define OMX_VIDDEC_COMP_VERSION_MAJOR 1
#define OMX_VIDDEC_COMP_VERSION_MINOR 1
#define OMX_VIDDEC_COMP_VERSION_REVISION 0
#define OMX_VIDDEC_COMP_VERSION_STEP 0

#define OMX_VIDEODECODER_DEFAULT_FRAMERATE 30

/*! Minimum Input Buffer Count */
#define OMX_VIDDEC_MIN_IN_BUF_COUNT 1
/*! Default Input Buffer Count */
#define OMX_VIDDEC_DEFAULT_IN_BUF_COUNT 2
/*! Minimum Input Buffer Count in Data Sync mode
    codec releases input buffer of Nth Data Sync call during (N+2)th Data Sync call
    So minimum number of input buffers required is 3 */
#define OMX_VIDDEC_DATASYNC_MIN_IN_BUF_COUNT 3
/*! Default Input Buffer Count in Data Sync mode*/
#define OMX_VIDDEC_DATASYNC_DEFAULT_IN_BUF_COUNT 4
/*! Default Frame Width */
#define OMX_VIDDEC_DEFAULT_FRAME_WIDTH 176
/*! Default Frame Height */
#define OMX_VIDDEC_DEFAULT_FRAME_HEIGHT 144
/*! Default 1D-Buffer Alignment */
#define OMX_VIDDEC_DEFAULT_1D_INPUT_BUFFER_ALIGNMENT 1
/*! Default Stride value for 2D buffer */
#define OMX_VIDDEC_TILER_STRIDE  (4096)
/*! Max Image Width supported */
#define OMX_VIDDEC_MAX_WIDTH  (2048)
/*! Max Image Height supported */
#define OMX_VIDDEC_MAX_HEIGHT  (2048)
/*! Max Number of MBs supported */
#define OMX_VIDDEC_MAX_MACROBLOCK (8160)

#define OMX_VIDEODECODER_COMPONENT_NAME "OMX.TI.DUCATI1.VIDEO.DECODER"

/*For setting the bit field*/
#define OMX_VIDEODECODER_SEIINFO2004FRAME1BIT   (0x00000001)
#define OMX_VIDEODECODER_SEIINFO2004FRAME2BIT   (0x00000002)
#define OMX_VIDEODECODER_SEIINFO2010FRAME1BIT   (0x00000004)
#define OMX_VIDEODECODER_SEIINFO2010FRAME2BIT   (0x00000008)
#define OMX_VIDEODECODER_RANGEMAPPINGBIT        (0x00000010)
#define OMX_VIDEODECODER_RESCALINGBIT           (0x00000020)
#define OMX_VIDEODECODER_CODECEXTERROR1BIT      (0X00000040)
#define OMX_VIDEODECODER_CODECEXTERROR2BIT      (0X00000080)
#define OMX_VIDEODECODER_MBINFOFRAME1BIT        (0X00000100)
#define OMX_VIDEODECODER_MBINFOFRAME2BIT        (0X00000200)
#define OMX_VIDEODECODER_SEIINFOFRAME1BIT       (0X00000400)
#define OMX_VIDEODECODER_SEIINFOFRAME2BIT       (0X00000800)
#define OMX_VIDEODECODER_VUIINFOFRAME1BIT       (0X00001000)
#define OMX_VIDEODECODER_VUIINFOFRAME2BIT       (0X00002000)


/*! \const H264VD_CODEC_BUG
 *  To enable H264 Bug Workarounds */
//#define H264VD_CODEC_BUG

/*! \const CODEC_CONFIG_WORKAROUND
 *  To enable Codec config Workarounds */
//#define CODEC_CONFIG_WORKAROUND

#define Codec_GetBuffinfoBug

#define RELEASE_MBINFO(a)  { \
        if( a->pMBinfo ) { \
            TIMM_OSAL_Free(a->pMBinfo); \
            a->pMBinfo = NULL; \
            a->nMBinfoSize = 0; \
        } \
}

/*----------          function prototypes      ------------------- */

/*===============================================================*/
/** @fn OMX_TI_VIDEODECODER_SetParameter:
 *      To implement the SetParameter functionality
 *
 * @param hComponent  : handle of the component
 * @param nIndex      : Index of the parameter to be set
 * @param pParamStruct: Pointer to the Parameter to be set
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_SetParameter(OMX_HANDLETYPE hComponent,
                                               OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);

/*===============================================================*/
/** @fn OMX_TI_VIDEODECODER_GetParameter:
 *      To implement the GetParameter functionality
 *
 * @param hComponent  : handle of the component
 * @param nIndex      : Index of the parameter to be read
 * @param pParamStruct: Pointer to the Parameter to be returned
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_GetParameter(OMX_HANDLETYPE hComponent,
                                               OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);

/*===============================================================*/
/** @fn OMX_TI_VIDEODECODER_GetConfig:
 *      To implement the GetConfig functionality
 *
 * @param hComponent  : handle of the component
 * @param nIndex      : Index of the config to be read
 * @param pComponentConfigStructuret: Pointer to the config to be returned
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_GetConfig(OMX_HANDLETYPE hComponent,
                                            OMX_INDEXTYPE nIndex, OMX_PTR pComponentConfigStructure);

/*===============================================================*/
/** @fn OMX_TI_VIDEODECODER_SetConfig:
 *      To implement the SetConfig functionality
 *
 * @param hComponent  : handle of the component
 * @param nIndex      : Index of the config to be read
 * @param pComponentConfigStructuret: Pointer to the config to be returned
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_SetConfig(OMX_HANDLETYPE hComponent,
                                            OMX_INDEXTYPE nIndex, OMX_PTR pComponentConfigStructure);

/*===============================================================*/
/** @fn OMX_TI_VIDEODECODER_CommandNotify:
 *      This function is used by base to notify the decoder of any
 *      command events
 *
 * @param hComponent  : handle of the component
 * @param Cmd         : Command to be notified to the decoder
 * @param nParam      : Parameter of the Command sent by base
 * @param pCmdData    : Pointer to the data associated with the command
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_TI_VIDEODECODER_CommandNotify(OMX_HANDLETYPE hComponent, OMX_COMMANDTYPE Cmd,
                                                       OMX_U32 nParam, OMX_PTR pCmdData);

/*===============================================================*/
/** @fn OMX_TI_VIDEODECODER_DataNotify:
 *      This function is used by base OMX component to notify the
 *      decoder of any Data events
 *
 * @param hComponent  : handle of the component
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_TI_VIDEODECODER_DataNotify(OMX_HANDLETYPE hComponent);

/*===============================================================*/
/** @fn OMX_TI_VIDEODECODER_ComponentDeinit:
 *      This function is used by to de-initialize the component
 *
 * @param hComponent  : handle of the component
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_TI_VIDEODECODER_ComponentDeinit(OMX_HANDLETYPE hComponent);

/*===============================================================*/
/** @fn OMX_TI_VIDEODECODER_GetExtensionIndex:
 *      This function is used by base to get the extension index
 *
 * @param hComponent  : handle of the component
 * @param cParameterName: Index of the parameter to be read
 * @param pIndexType  : Pointer to the return value
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_TI_VIDEODECODER_GetExtensionIndex(OMX_HANDLETYPE hComponent, OMX_STRING cParameterName,
                                                           OMX_INDEXTYPE *pIndexType);

/* ==========================================================================*/
/**
 * @fn _OMX_VIDDEC_DataSync_GetInputData(XDM_DataSyncHandle dataSyncHandle, XDM_DataSyncDesc *dataSyncDesc)
 *                     This method is called by the codec in slice mode to get the
 *                     partial input data
 *
 *  @param [in] dataSyncHandle  : pointer to the OMX Component.
 *  @param [in] dataSyncDesc    : Data sync descriptor used by codec
 *
 */
/* ==========================================================================*/
XDAS_Int32 _OMX_VIDDEC_DataSync_GetInputData(XDM_DataSyncHandle dataSyncHandle, XDM_DataSyncDesc *dataSyncDesc);

/* ==========================================================================*/
/**
 * @fn _OMX_VIDDEC_DataSync_PutBuffer(XDM_DataSyncHandle dataSyncHandle, XDM_DataSyncDesc *dataSyncDesc)
 *                     This method is called by the codec in slice mode to get the
 *                     partial input data
 *
 *  @param [in] dataSyncHandle  : pointer to the OMX Component.
 *  @param [in] dataSyncDesc    : Data sync descriptor used by codec
 *
 */
/* ==========================================================================*/
XDAS_Int32 _OMX_VIDDEC_DataSync_PutBuffer(XDM_DataSyncHandle dataSyncHandle, XDM_DataSyncDesc *dataSyncDesc);

static OMX_STRING    engineName   = "ivahd_vidsvr";

/*----------  external function declarations      ------------------- */
extern OMX_ERRORTYPE OMX_TI_VIDEODECODER_Init_DerToBase_Fields(OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt);
extern OMX_ERRORTYPE OMX_TI_VIDEODECODER_Set_DerToBase_Fields(OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt);
extern void OMX_TI_VIDEODECODER_Init_PortDefs(OMX_HANDLETYPE hComponent, OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt);
extern void OMX_TI_VIDEODECODER_Init_PortParams(OMX_HANDLETYPE hComponent, OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt);
extern void OMX_TI_VIDEODECODER_Set_2D_BuffParams(OMX_HANDLETYPE hComponent, OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt);
extern OMX_U32 Calc_InbufSize(OMX_U32 width, OMX_U32 height);
extern OMX_ERRORTYPE OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(OMX_HANDLETYPE hComponent, OMX_VIDEODECODER_CUSTOMBUFFERHEADER *pOutBufHeader,
                                                              OMX_BUFFERHEADERTYPE *pInBufHeader);
extern OMX_ERRORTYPE OMX_TI_VIDDEC_SetInPortDef(OMX_HANDLETYPE hComponent,
                                                OMX_PARAM_PORTDEFINITIONTYPE *pPortDefs);
extern OMX_ERRORTYPE OMX_TI_VIDDEC_SetOutPortDef(OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt,
                                                 OMX_PARAM_PORTDEFINITIONTYPE *pPortDefs);
extern OMX_ERRORTYPE OMX_TI_VIDDEC_HandleFirstFrame(OMX_HANDLETYPE hComponent,
                                                    OMX_BUFFERHEADERTYPE * *ppInBufHeader);
extern OMX_ERRORTYPE OMX_TI_VIDDEC_HandleCodecProcError(OMX_HANDLETYPE hComponent,
                                                        OMX_BUFFERHEADERTYPE * *ppInBufHeader,
                                                        OMX_VIDEODECODER_CUSTOMBUFFERHEADER * *ppOutBufHeader);
extern OMX_ERRORTYPE  OMX_TI_VIDDEC_HandleLockedBuffer(OMX_HANDLETYPE hComponent,
                                                       OMX_VIDEODECODER_CUSTOMBUFFERHEADER *pOutBufHeader);

/* ==========================================================================*/
/**
 * @fn _OMX_VIDEODECODER_PUSH_METADATA_BUFFER()
 *                     This method adds o/p buffer and corrresponding metadata buffers
 *                     to an internal table
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *              pSEIinfo : pointer to metadata buffer
 *              nInputID : pointer to o/p buffer
 */
/* ==========================================================================*/
extern void _OMX_VIDEODECODER_PUSH_METADATA_BUFFER(OMX_HANDLETYPE hComponent, OMX_PTR pMetadata, OMX_U32 nInputID);

/* ==========================================================================*/
/**
 * @fn _OMX_VIDEODECODER_PULL_METADATA_BUFFER()
 *                     This method gives the poinetr to metadata buffers corrresponding
 *                     to an o/p buffer
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *              ppSEIinfoFrame1 : pointer to metadata buffer corresponding to field 1
  *              ppSEIinfoFrame2 : pointer to metadata buffer corresponding to field 2
 *              nOutputID : pointer to o/p buffer
 */
/* ==========================================================================*/
extern void _OMX_VIDEODECODER_PULL_METADATA_BUFFER(OMX_HANDLETYPE hComponent, OMX_U32 nOutputID, OMX_METADATA * *ppMetadataFrame1, OMX_METADATA * *ppMetadataFrame2);

/* ==========================================================================*/
/**
 * @fn _OMX_VIDEODECODER_DELETE_METADATA_BUFFER()
 *                     This method removes the o/p buffer header and corresponding
 *                     buffers from the internal table.
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *              nOutputID : pointer to o/p buffer
 */
/* ==========================================================================*/
extern void _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(OMX_HANDLETYPE hComponent, OMX_U32 nOutputID);

/* ==========================================================================*/
/**
 * @fn _OMX_VIDEODECODER_FREE_METADATA_BUFFER()
 *                     This method frees tha metadata memory
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *              pMetadata : pointer to metadata buffer
 */
/* ==========================================================================*/
extern void _OMX_VIDEODECODER_FREE_METADATA_BUFFER(OMX_METADATA *pMetadata);

/* ==========================================================================*/
/**
 * @fn _OMX_VIDEODEC_VISACONTROL()
 *                A wrapper for the VISA Control call to include the RM calls
 */
/* ==========================================================================*/
OMX_ERRORTYPE _OMX_VIDEODEC_VISACONTROL(VIDDEC3_Handle handle, VIDDEC3_Cmd id,
                                      VIDDEC3_DynamicParams *dynParams, VIDDEC3_Status *status, OMX_HANDLETYPE hComponent, XDAS_Int32 *retval);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_VIDEO_DECODER_H_ */

