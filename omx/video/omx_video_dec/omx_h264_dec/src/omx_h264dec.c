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

#include <omx_h264vd.h>
#include <omx_video_decoder_internal.h>
#define OMX_MAX_DEC_OP_BUFFERS 20

#include <utils/Log.h>

/* ==========================================================================*/
/**
 * @fn OMX_TI_H264VD_ComponentInit()
 *                     This method will initialize the H264 Decoder client
 *                     component. This method is called by the IL-Client to
 *                     initialize the component containing H264 Decoder.
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_H264VD_ComponentInit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE        eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE   *pHandle = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt  = NULL;

    eError = OMX_TI_VideoDecoder_ComponentInit(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    eError = OMX_H264VD_Init(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    OMX_TI_Init_Video_decoder_params(hComponent, pHandle->pComponentPrivate);

    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    strcpy((char *)pVideoDecoderComPvt->tComponentRole.cRole,
           "video_decoder.avc");

    pVideoDecoderComPvt->strComponentName = OMX_H264VD_COMPONENT_NAME;

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_H264VD_Init()
 *                     This method will perform the H264 Decoder specific
 *                     initialization. This is called during initialziation.
 *                     This function initializes the function pointers and
 *                     allocates memory required specifically for H264 decoder
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_H264VD_Init(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt  = NULL;
    OMX_H264VD_PVTTYPE         *pH264vdComPvt = NULL;
    BUFF_POINTERS              *decStructPtr = NULL;
    BASECLASS_BUFF_POINTERS    *decBaseClassPtr;
    IVIDDEC3_Params            *pDecParams;
    IH264VDEC_Params           *params;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt  = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    /*! Initialize the function parameters for OMX functions */
    pHandle->SetParameter = OMX_TI_H264VD_SetParameter;
    pHandle->GetParameter = OMX_TI_H264VD_GetParameter;

    /*! Initialize the function pointers */
    pVideoDecoderComPvt->fpSet_StaticParams = OMX_TI_H264VD_Set_StaticParams;
    pVideoDecoderComPvt->fpSet_DynamicParams = OMX_TI_H264VD_Set_DynamicParams;
    pVideoDecoderComPvt->fpSet_Status = OMX_TI_H264VD_Set_Status;
    pVideoDecoderComPvt->fpCalc_OubuffDetails
        = Calculate_TI_H264VD_outbuff_details;
    pVideoDecoderComPvt->fpDeinit_Codec = OMX_H264VD_DeInit;
    pVideoDecoderComPvt->strDecoderName = "ivahd_h264dec";
    pVideoDecoderComPvt->eCompressionFormat = OMX_VIDEO_CodingAVC;
    pVideoDecoderComPvt->fpHandle_ExtendedError = OMX_TI_H264VD_HandleError;

    TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                      "\n Codec selected : H264\n");

    pVideoDecoderComPvt->pCodecPrivate =
        (OMX_H264VD_PVTTYPE *) TIMM_OSAL_Malloc(sizeof (OMX_H264VD_PVTTYPE),
                                                TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT((pVideoDecoderComPvt->pCodecPrivate) != NULL, OMX_ErrorInsufficientResources);

    pH264vdComPvt =(OMX_H264VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    OMX_BASE_INIT_STRUCT_PTR(&(pH264vdComPvt->tH264VideoParam), OMX_VIDEO_PARAM_AVCTYPE);
    pH264vdComPvt->tH264VideoParam.nPortIndex = OMX_VIDEODECODER_INPUT_PORT;
    pH264vdComPvt->tH264VideoParam.eProfile = OMX_VIDEO_AVCProfileHigh;
    pH264vdComPvt->tH264VideoParam.eLevel = OMX_VIDEO_AVCLevel41;
    pH264vdComPvt->tH264VideoParam.nRefFrames = 0xFFFFFFFF;

    /*! Allocate Memory for variables to be passed to Codec */
    decStructPtr = &(pVideoDecoderComPvt->pDecStructures);
    decBaseClassPtr = &(pVideoDecoderComPvt->pBaseClassStructers);

    /*! Allocate Memory for Static Parameter */
    decStructPtr->pDecStaticParams
        = (IH264VDEC_Params *) TIMM_OSAL_Malloc(sizeof(IH264VDEC_Params),
                                                TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecStaticParams != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecStaticParams, 0x0,
                     sizeof(IH264VDEC_Params));

    params = (IH264VDEC_Params *) (decStructPtr->pDecStaticParams);
    pDecParams = &(params->viddec3Params);
    pDecParams->displayDelay  = IVIDDEC3_DISPLAY_DELAY_AUTO;
    params->presetLevelIdc =  IH264VDEC_LEVEL41;
    params->dpbSizeInFrames = IH264VDEC_DPB_NUMFRAMES_AUTO;

    /*! Allocate Memory for Dynamic Parameter */
    decStructPtr->pDecDynParams
        = (IH264VDEC_DynamicParams *) TIMM_OSAL_Malloc(sizeof(IH264VDEC_DynamicParams),
                                                       TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecDynParams != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecDynParams, 0x0,
                     sizeof(IH264VDEC_DynamicParams));

    /*! Allocate Memory for Status Structure */
    decStructPtr->pDecStatus
        = (IH264VDEC_Status *) TIMM_OSAL_Malloc(sizeof(IH264VDEC_Status),
                                                TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecStatus != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecStatus, 0x0, sizeof(IH264VDEC_Status));
    ((IH264VDEC_Status *)decStructPtr->pDecStatus)->spsMaxRefFrames = 1;

    /*! Allocate Memory for Input Arguments */
    decStructPtr->pDecInArgs
        = (IH264VDEC_InArgs *) TIMM_OSAL_Malloc(sizeof(IH264VDEC_InArgs),
                                                TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecInArgs != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecInArgs, 0x0, sizeof(IH264VDEC_InArgs));

    /*! Allocate Memory for Output Arguments */
    decStructPtr->pDecOutArgs
        = (IH264VDEC_OutArgs *) TIMM_OSAL_Malloc(sizeof(IH264VDEC_OutArgs),
                                                 TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecOutArgs != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecOutArgs, 0x0, sizeof(IH264VDEC_OutArgs));


    /*! Now initialize pointers of base class structures */
    decBaseClassPtr->pDecStaticParams
        = &((IH264VDEC_Params * )decStructPtr->pDecStaticParams)->viddec3Params;
    decBaseClassPtr->pDecDynParams
        = &((IH264VDEC_DynamicParams * )decStructPtr->pDecDynParams)->viddec3DynamicParams;
    decBaseClassPtr->pDecStatus
        = &((IH264VDEC_Status * )decStructPtr->pDecStatus)->viddec3Status;
    decBaseClassPtr->pDecInArgs
        = &((IH264VDEC_InArgs * )decStructPtr->pDecInArgs)->viddec3InArgs;
    decBaseClassPtr->pDecOutArgs
        = &((IH264VDEC_OutArgs * )decStructPtr->pDecOutArgs)->viddec3OutArgs;

    decBaseClassPtr->pDecInArgs->size = sizeof(IH264VDEC_InArgs);
    decBaseClassPtr->pDecOutArgs->size = sizeof(IH264VDEC_OutArgs);

    pVideoDecoderComPvt->tMetadata.bEnableMBInfo = OMX_FALSE;
    pVideoDecoderComPvt->tMetadata.bEnableSEIInfo = OMX_TRUE;
    pVideoDecoderComPvt->tMetadata.bEnableVUIInfo = OMX_TRUE;

    pVideoDecoderComPvt->nSEIInfoIndex = 0;
    pVideoDecoderComPvt->nVUIInfoIndex= 1;

    pDecParams->metadataType[pVideoDecoderComPvt->nSEIInfoIndex] = IH264VDEC_PARSED_SEI_DATA;
    pDecParams->metadataType[pVideoDecoderComPvt->nVUIInfoIndex] = IH264VDEC_PARSED_VUI_DATA;
    pDecParams->metadataType[2] = IVIDEO_METADATAPLANE_NONE;

    pDecParams->operatingMode = IVIDEO_DECODE_ONLY;
    pVideoDecoderComPvt->nMBInfoStructSize = 208;

    pDecParams->inputDataMode = IVIDEO_ENTIREFRAME;
    pDecParams->numInputDataUnits  = 0;

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_H264VD_Set_StaticParams()
 *                     This method will initialize the Static Parameters
 *                     required for the H264 Decoder codec
 *
 *  @param [in] pstaticParams : Pointer to the static params of H264 decoder
 *  @param [in] width         : Maximum width of frame
 *  @param [in] height        : Maximum height of frame
 *
 */
/* ==========================================================================*/

void OMX_TI_H264VD_Set_StaticParams(OMX_HANDLETYPE hComponent, void *staticparams)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    IVIDDEC3_Params            *pDecParams;
    IH264VDEC_Params           *params = (IH264VDEC_Params *)staticparams;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pDecParams = &(params->viddec3Params);

    pDecParams->size = sizeof(IH264VDEC_Params);

    pDecParams->maxWidth = 1920;
    pDecParams->maxHeight = 1088;

    pDecParams->maxFrameRate   = 30000;
    pDecParams->maxBitRate          = 10000000;
    pDecParams->dataEndianness      = XDM_BYTE;

    /*init outArgs IVIDDEC3_OutArgs*/
    pDecParams->forceChromaFormat  = XDM_YUV_420SP;

    pDecParams->displayBufsMode = IVIDDEC3_DISPLAYBUFS_EMBEDDED;

    pDecParams->outputDataMode = IVIDEO_ENTIREFRAME;
    pDecParams->numOutputDataUnits = 0;
    pDecParams->errorInfoMode = IVIDEO_ERRORINFO_OFF;
    params->errConcealmentMode =
        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.bFlagErrorConcealment;

    params->temporalDirModePred = IH264VDEC_ENABLE_TEMPORALDIRECT;
    params->debugTraceLevel= 0;
    params->lastNFramesToLog= 0;

    pVideoDecoderComPvt->bErrorConcealmentBuffer = OMX_FALSE;

    return;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_H264VD_Set_DynamicParams()
 *                     This method will initialize the Dynamic Parameters
 *                     required for the H264 xDM component
 *
 *  @param [in] pdynamicParams : Pointer to the Dynamic params of H264 decoder
 *
 */
/* ==========================================================================*/
void OMX_TI_H264VD_Set_DynamicParams(OMX_HANDLETYPE hComponent, void *dynParams)
{
    OMX_COMPONENTTYPE         *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE  *pVideoDecoderComPvt = NULL;
    IVIDDEC3_DynamicParams    *pDecDynParams;
    IH264VDEC_DynamicParams   *params = (IH264VDEC_DynamicParams *) dynParams;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pDecDynParams = &(params->viddec3DynamicParams);

    pDecDynParams->size = sizeof(IVIDDEC3_DynamicParams);

    /* init dynamic params IVIDDEC3_DynamicParams */
    pDecDynParams->decodeHeader  = XDM_DECODE_AU; /* Supported */

    pDecDynParams->displayWidth  = 0; /* Not Supported: Set default */
    /*Not Supported: Set default*/
    pDecDynParams->frameSkipMode = H264VD_DEFAULT_FRAME_SKIP;
    pDecDynParams->newFrameFlag = XDAS_TRUE; //Not Supported: Set default

    if( ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_ENTIREFRAME )
    {
        pDecDynParams->putBufferFxn = NULL;
        pDecDynParams->putBufferHandle = NULL;
        pDecDynParams->putDataFxn    = NULL;
        pDecDynParams->putDataHandle = NULL;
        pDecDynParams->getDataHandle = NULL;
        pDecDynParams->getDataFxn    = NULL;
    }
    else if( ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_SLICEMODE )
    {
        pDecDynParams->putBufferFxn = _OMX_VIDDEC_DataSync_PutBuffer;
        pDecDynParams->putBufferHandle = hComponent;
        pDecDynParams->putDataFxn    = NULL;
        pDecDynParams->putDataHandle = NULL;
        pDecDynParams->getDataHandle = hComponent;
        pDecDynParams->getDataFxn    = _OMX_VIDDEC_DataSync_GetInputData;
    }

    return;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_H264VD_Set_Status()
 *                     This method will initialize the parameters in the
 *                     Status xDM structure
 *
 *  @param [in] pStatus : Pointer to the status params of H264 decoder
 *
 */
/* ==========================================================================*/
void OMX_TI_H264VD_Set_Status(OMX_HANDLETYPE hComponent, void *decstatus)
{
    IH264VDEC_Status   *status;

    status = (IH264VDEC_Status *)decstatus;

    status->viddec3Status.size     = sizeof(IH264VDEC_Status);
    return;
}

/* ==========================================================================*/
/**
 * @fn Calculate_TI_H264VD_outbuff_details()
 *                     This method will calcullate the Buffer Details by
 *                     taking the width and height as parameters
 *
 *  @param [in] nWidth  : Width of the frame
 *  @param [in] nHeight : Height of the frame
 *
 */
/* ==========================================================================*/

BUFF_PARAMS Calculate_TI_H264VD_outbuff_details(OMX_HANDLETYPE hComponent, OMX_U32 width, OMX_U32 height)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt  = NULL;
    OMX_H264VD_PVTTYPE         *pH264vdComPvt = NULL;
    OMX_U32                     nRefBufferCount=16;
    IH264VDEC_Status           *pDecExtStatus = NULL;
    BUFF_POINTERS              *pDecExtStructure = NULL;
    IH264VDEC_Params           *staticparams;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt  = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    pH264vdComPvt =(OMX_H264VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;
    pDecExtStructure = &(pVideoDecoderComPvt->pDecStructures);

    pDecExtStatus = (IH264VDEC_Status *)
                    (pDecExtStructure->pDecStatus);

    BUFF_PARAMS    OutBuffDetails;
    OutBuffDetails.nBufferSize
        = ((((width + (2 * PADX) + 127) & 0xFFFFFF80) * (height + 4 * PADY)));
    /* Multiply buffer size by 1.5 to account for both luma and chroma */
    OutBuffDetails.nBufferSize = (OutBuffDetails.nBufferSize * 3) >> 1;
    OutBuffDetails.nBufferCountMin = OMX_TI_H264VD_Calculate_TotalRefFrames(width, height, pH264vdComPvt->tH264VideoParam.eLevel);
    staticparams = (IH264VDEC_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams);

    // Assume 0 ref frames initially only if IL client is using port reconfig for allocating padded buffers. In that case use the correct ref frames at the time of port reconfig to calculate nBufferCountMin/Actual.
    if( pDecExtStatus->spsMaxRefFrames != 0 || pVideoDecoderComPvt->bUsePortReconfigForPadding == OMX_TRUE ) {

        staticparams->viddec3Params.displayDelay
            = pDecExtStatus->spsMaxRefFrames;
        staticparams->dpbSizeInFrames
            = pDecExtStatus->spsMaxRefFrames;
        pH264vdComPvt->tH264VideoParam.nRefFrames
            = pDecExtStatus->spsMaxRefFrames;
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Codec dpb size set to %d", staticparams->dpbSizeInFrames);

        if( pH264vdComPvt->tH264VideoParam.eProfile == OMX_VIDEO_AVCProfileBaseline ) {
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\n Base Profile\n");

            OutBuffDetails.nBufferCountMin
                = pDecExtStatus->spsMaxRefFrames + 1;
        } else {
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\n High Profile\n");

            OutBuffDetails.nBufferCountMin =
                2 * pDecExtStatus->spsMaxRefFrames + 1;
        }
    } else if( pH264vdComPvt->tH264VideoParam.nRefFrames == 0xFFFFFFFF ) {
        if( pH264vdComPvt->tH264VideoParam.eProfile == OMX_VIDEO_AVCProfileBaseline ) {
            OutBuffDetails.nBufferCountMin = OMX_TI_H264VD_Calculate_TotalRefFrames
                                                 (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameWidth,
                                                 pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameHeight,
                                                 pH264vdComPvt->tH264VideoParam.eLevel);
        } else {
            nRefBufferCount = OMX_TI_H264VD_Calculate_TotalRefFrames
                                  (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameWidth,
                                  pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameHeight,
                                  pH264vdComPvt->tH264VideoParam.eLevel) - 1;
            if((2 * nRefBufferCount + 1) < OMX_MAX_DEC_OP_BUFFERS ) {
                OutBuffDetails.nBufferCountMin
                    = 2 * nRefBufferCount + 1;
            } else {
                OutBuffDetails.nBufferCountMin = OMX_MAX_DEC_OP_BUFFERS; //Cap max buffers to 20
            }
        }
    } else if( pH264vdComPvt->tH264VideoParam.nRefFrames <= 16 ) {
        if( pH264vdComPvt->tH264VideoParam.eProfile == OMX_VIDEO_AVCProfileBaseline ) {
            OutBuffDetails.nBufferCountMin
                = pH264vdComPvt->tH264VideoParam.nRefFrames + 1;
        } else {
            OutBuffDetails.nBufferCountMin =
                2 * pH264vdComPvt->tH264VideoParam.nRefFrames + 1;
        }
    }
    OutBuffDetails.nBufferCountActual = OutBuffDetails.nBufferCountMin + 2;
    OutBuffDetails.n1DBufferAlignment = 16;
    OutBuffDetails.nPaddedWidth = (width + (2 * PADX) + 127) & 0xFFFFFF80;
    OutBuffDetails.nPaddedHeight = height + 4 * PADY;
    OutBuffDetails.n2DBufferYAlignment = 1;
    OutBuffDetails.n2DBufferXAlignment = 16;

    return (OutBuffDetails);
}

/* ==========================================================================*/
/**
 * @fn OMX_H264VD_DeInit()
 *                     This method will de-initialize the H264 Decoder
 *                     component
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/
void OMX_H264VD_DeInit(OMX_HANDLETYPE hComponent)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt  = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt  = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    /*! Delete all the memory which was allocated during init of decoder */
    if( pVideoDecoderComPvt->pDecStructures.pDecStaticParams ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->pDecStructures.pDecStaticParams);
        pVideoDecoderComPvt->pDecStructures.pDecStaticParams = NULL;
    }
    if( pVideoDecoderComPvt->pDecStructures.pDecDynParams ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->pDecStructures.pDecDynParams);
        pVideoDecoderComPvt->pDecStructures.pDecDynParams = NULL;
    }
    if( pVideoDecoderComPvt->pDecStructures.pDecStatus ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->pDecStructures.pDecStatus);
        pVideoDecoderComPvt->pDecStructures.pDecStatus = NULL;
    }
    if( pVideoDecoderComPvt->pDecStructures.pDecInArgs ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->pDecStructures.pDecInArgs);
        pVideoDecoderComPvt->pDecStructures.pDecInArgs = NULL;
    }
    if( pVideoDecoderComPvt->pDecStructures.pDecOutArgs ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->pDecStructures.pDecOutArgs);
        pVideoDecoderComPvt->pDecStructures.pDecOutArgs = NULL;
    }
    if( pVideoDecoderComPvt->pCodecPrivate ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->pCodecPrivate);
        pVideoDecoderComPvt->pCodecPrivate = NULL;
    }
    pHandle->SetParameter = OMX_TI_VIDEODECODER_SetParameter;
    pHandle->GetParameter = OMX_TI_VIDEODECODER_GetParameter;
    pVideoDecoderComPvt->fpHandle_ExtendedError = NULL;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_H264VD_Calculate_TotalRefFrames()
 *                     This method is called by set parameter
 *                     in the Decoder component
 *
 *  @param [in] nWidth  : Actual width for which the decoder is created
 *  @param [in] nHeight : Actual height for which the decoder is created
 *
 */
/* ==========================================================================*/
OMX_U32 OMX_TI_H264VD_Calculate_TotalRefFrames(OMX_U32 nWidth, OMX_U32 nHeight, OMX_VIDEO_AVCLEVELTYPE eLevel)
{
    OMX_U32    ref_frames = 0;
    OMX_U32    MaxDpbMbs;
    OMX_U32    PicWidthInMbs;
    OMX_U32    FrameHeightInMbs;

    switch( eLevel ) {
        case OMX_VIDEO_AVCLevel1 :
        case OMX_VIDEO_AVCLevel1b :
        {
            MaxDpbMbs = 396;
            break;
        }
        case OMX_VIDEO_AVCLevel11 :
        {
            MaxDpbMbs = 900;
            break;
        }
        case OMX_VIDEO_AVCLevel12 :
        case OMX_VIDEO_AVCLevel13 :
        case OMX_VIDEO_AVCLevel2 :
        {
            MaxDpbMbs = 2376;
            break;
        }
        case OMX_VIDEO_AVCLevel21 :
        {
            MaxDpbMbs = 4752;
            break;
        }
        case OMX_VIDEO_AVCLevel22 :
        case OMX_VIDEO_AVCLevel3 :
        {
            MaxDpbMbs = 8100;
            break;
        }
        case OMX_VIDEO_AVCLevel31 :
        {
            MaxDpbMbs = 18000;
            break;
        }
        case OMX_VIDEO_AVCLevel32 :
        {
            MaxDpbMbs = 20480;
            break;
        }
        case OMX_VIDEO_AVCLevel5 :
        {
            MaxDpbMbs = 110400; //Maximum value for upto level 5
            break;
        }
        case OMX_VIDEO_AVCLevel51 :
        {
            MaxDpbMbs = 184320; //Maximum value for upto level 5.1
            break;
        }
        default :
        {
            MaxDpbMbs = 32768; //Maximum value for upto level 4.1
        }
    }

    PicWidthInMbs = nWidth / 16;
    FrameHeightInMbs = nHeight / 16;
    ref_frames =  (OMX_U32)(MaxDpbMbs / (PicWidthInMbs * FrameHeightInMbs));
    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                       "nWidth [%d] PicWidthInMbs [%d] nHeight [%d] FrameHeightInMbs [%d] ref_frames [%d]",
                       nWidth,
                       PicWidthInMbs,
                       nHeight,
                       FrameHeightInMbs,
                       ref_frames);

    ref_frames = (ref_frames > 16) ? 16 : ref_frames;

    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Final ref_frames [%d]",
                       ref_frames);

    /* Three is added to total reference frames because of the N+3 buffer issue
     * It was found that theoretically 2N+1 buffers are required but from a practical
     * point of view N+3 was sufficient */
    return (ref_frames + 1);
}

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

OMX_ERRORTYPE OMX_TI_H264VD_SetParameter(OMX_HANDLETYPE hComponent,
                                         OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt  = NULL;
    OMX_H264VD_PVTTYPE         *pH264vdComPvt = NULL;
    OMX_VIDEO_PARAM_AVCTYPE    *pH264VideoParam = NULL;
    OMX_U32                     nRefBufferCount = 0;
    OMX_U8                      i;
    IH264VDEC_Params           *staticparams;

    OMX_BASE_REQUIRE((hComponent != NULL) && (pParamStruct != NULL),
                     OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    pH264vdComPvt =
        (OMX_H264VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;
    staticparams = (IH264VDEC_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams);

    switch( nIndex ) {
        case OMX_IndexParamVideoAvc :
        {
            pH264VideoParam = (OMX_VIDEO_PARAM_AVCTYPE *) pParamStruct;
            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[pH264VideoParam->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_AVCTYPE, eError);
            OMX_BASE_ASSERT(pH264VideoParam->eProfile == OMX_VIDEO_AVCProfileBaseline
                            || pH264VideoParam->eProfile == OMX_VIDEO_AVCProfileMain
                            || pH264VideoParam->eProfile == OMX_VIDEO_AVCProfileExtended
                            || pH264VideoParam->eProfile == OMX_VIDEO_AVCProfileHigh,
                            OMX_ErrorUnsupportedSetting);
            OMX_BASE_ASSERT(pH264VideoParam->eLevel <= OMX_VIDEO_AVCLevel51,
                            OMX_ErrorUnsupportedSetting);
            if( pH264VideoParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                pH264vdComPvt->tH264VideoParam = *pH264VideoParam;
            } else if( pH264VideoParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoAvc supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
                goto EXIT;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Profile set = %x, Level Set = %x, num ref frames set = %d",
                               pH264VideoParam->eProfile, pH264VideoParam->eLevel, pH264VideoParam->nRefFrames);
            if( pH264VideoParam->eLevel == OMX_VIDEO_AVCLevel5 ) {
                staticparams->presetLevelIdc
                    =  IH264VDEC_LEVEL5;
            } else if( pH264VideoParam->eLevel == OMX_VIDEO_AVCLevel51 ) {
                staticparams->presetLevelIdc
                    =  IH264VDEC_LEVEL51;
            }
            if( pH264vdComPvt->tH264VideoParam.nRefFrames == 0xFFFFFFFF ) {
                if( pH264VideoParam->eProfile == OMX_VIDEO_AVCProfileBaseline ) {
                    staticparams->viddec3Params.displayDelay
                        = IVIDDEC3_DECODE_ORDER;

                    pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin = OMX_TI_H264VD_Calculate_TotalRefFrames
                                                                                                        (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameWidth,
                                                                                                        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameHeight,
                                                                                                        pH264VideoParam->eLevel);
                } else {
                    nRefBufferCount = OMX_TI_H264VD_Calculate_TotalRefFrames
                                          (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameWidth,
                                          pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameHeight,
                                          pH264VideoParam->eLevel) - 1;
                    staticparams->viddec3Params.displayDelay
                        = nRefBufferCount;
                    if((2 * nRefBufferCount + 1) < OMX_MAX_DEC_OP_BUFFERS ) {
                        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin
                            = 2 * nRefBufferCount + 1;
                    } else {
                        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin
                            = OMX_MAX_DEC_OP_BUFFERS;     //Cap max buffers to 20
                    }
                }
            } else if( pH264vdComPvt->tH264VideoParam.nRefFrames <= 16 ) {
#if 1
                staticparams->dpbSizeInFrames
                    = pH264vdComPvt->tH264VideoParam.nRefFrames;
#endif
                if( pH264VideoParam->eProfile == OMX_VIDEO_AVCProfileBaseline ) {
                    staticparams->viddec3Params.displayDelay
                        = IVIDDEC3_DECODE_ORDER;
                    pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin
                        = pH264vdComPvt->tH264VideoParam.nRefFrames + 1;
                } else {
                    staticparams->viddec3Params.displayDelay
                        = pH264vdComPvt->tH264VideoParam.nRefFrames;
                    pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin =
                        2 * pH264vdComPvt->tH264VideoParam.nRefFrames + 1;
                }
            } else {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "Invalid value of nRefFrames = %d of the structure OMX_VIDEO_PARAM_AVCTYPE provided ",
                                   pH264vdComPvt->tH264VideoParam.nRefFrames);
            }
        }
        break;
        case OMX_IndexParamVideoProfileLevelQuerySupported :
        {
            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);
            /* Check for the correct nSize & nVersion information */
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PROFILELEVELTYPE, eError);
            /*As of now do nothing. This index is required for StdVideoDecoderTest. Later on do code review and fill in proper code here.*/
            eError = OMX_ErrorNoMore;
        }
        break;
        case OMX_IndexParamVideoProfileLevelCurrent :
        {
            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);
            /* Check for the correct nSize & nVersion information */
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PROFILELEVELTYPE, eError);
            /*As of now do nothing. This index is required for StdVideoDecoderTest. Later on do code review and fill in proper code here.*/
            eError = OMX_ErrorNoMore;
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexParamVideoEnableMetadata :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_DECMETADATA, eError);

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);

            i=0;
            if(((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableMBInfo == OMX_TRUE ) {
                pVideoDecoderComPvt->nMBInfoIndex = i;
                pVideoDecoderComPvt->tMetadata.bEnableMBInfo = OMX_TRUE;
                ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->metadataType[i++] = IVIDEO_METADATAPLANE_MBINFO;
                if(((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableTranscodeMode == OMX_TRUE ) {
                    ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->operatingMode = IVIDEO_TRANSCODE_FRAMELEVEL;
                } else {
                    ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->operatingMode = IVIDEO_DECODE_ONLY;
                }

            } else {
                pVideoDecoderComPvt->tMetadata.bEnableMBInfo = OMX_FALSE;
            }
            if(((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableSEIInfo == OMX_TRUE ) {
                pVideoDecoderComPvt->nSEIInfoIndex = i;
                pVideoDecoderComPvt->tMetadata.bEnableSEIInfo = OMX_TRUE;
                ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->metadataType[i++] = IH264VDEC_PARSED_SEI_DATA;
            } else {
                pVideoDecoderComPvt->tMetadata.bEnableSEIInfo = OMX_FALSE;
            }
            if(((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableVUIInfo == OMX_TRUE ) {
                pVideoDecoderComPvt->nVUIInfoIndex = i;
                pVideoDecoderComPvt->tMetadata.bEnableVUIInfo = OMX_TRUE;
                ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->metadataType[i++] = IH264VDEC_PARSED_VUI_DATA;
            } else {
                pVideoDecoderComPvt->tMetadata.bEnableVUIInfo = OMX_FALSE;
            }

            while( i < 3 ) {
                ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->metadataType[i++] = IVIDEO_METADATAPLANE_NONE;
            }
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexParamVideoDataSyncMode :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_DATASYNCMODETYPE, eError);

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);
            if( ((OMX_VIDEO_PARAM_DATASYNCMODETYPE *) pParamStruct)->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                if(((OMX_VIDEO_PARAM_DATASYNCMODETYPE *) pParamStruct)->eDataMode == OMX_Video_EntireFrame ) {
                    ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode = IVIDEO_ENTIREFRAME;
                    ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->numInputDataUnits  = 0;
                } else if(((OMX_VIDEO_PARAM_DATASYNCMODETYPE *) pParamStruct)->eDataMode == OMX_Video_SliceMode ) {
                    ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode = IVIDEO_SLICEMODE;
                    ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->numInputDataUnits  = 1;
                } else {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Supports I/P data only in Frame mode or slice mode");
                }
            }
            else {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "OMX_TI_IndexParamVideoDataSyncMode supported only on i/p port");
            }
        }
        break;
        default :
            eError = OMX_TI_VIDEODECODER_SetParameter(hComponent, nIndex, pParamStruct);

    }

EXIT:
    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "exiting set parameter");
    return (eError);
}

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
OMX_ERRORTYPE OMX_TI_H264VD_GetParameter(OMX_HANDLETYPE hComponent,
                                         OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                       eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                  *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE           *pVideoDecoderComPvt = NULL;
    OMX_H264VD_PVTTYPE                 *pH264vdComPvt = NULL;
    OMX_VIDEO_PARAM_AVCTYPE            *pH264VideoParam = NULL;
    OMX_VIDEO_PARAM_PROFILELEVELTYPE   *pH264ProfileLevelParam = NULL;

    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                       "Get Parameter called");

    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pParamStruct != NULL), OMX_ErrorBadParameter);

    // Initialize the local variables
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pH264vdComPvt =
        (OMX_H264VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    /* GetParameter can't be invoked incase the comp is in Invalid State  */
    OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState != OMX_StateInvalid,
                    OMX_ErrorIncorrectStateOperation);

    switch( nIndex ) {
        case OMX_IndexParamVideoAvc :
        {

            pH264VideoParam = (OMX_VIDEO_PARAM_AVCTYPE *) pParamStruct;

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_AVCTYPE, eError);
            if( pH264VideoParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                *pH264VideoParam = pH264vdComPvt->tH264VideoParam;
            } else if( pH264VideoParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoAvc supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;
        case OMX_IndexParamVideoProfileLevelQuerySupported :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PROFILELEVELTYPE, eError);
            pH264ProfileLevelParam = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pParamStruct;
            if( pH264ProfileLevelParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                if( pH264ProfileLevelParam->nProfileIndex == 0 ) {
                    pH264ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_AVCProfileBaseline;
                    pH264ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_AVCLevel41;
                } else if( pH264ProfileLevelParam->nProfileIndex == 1 ) {
                    pH264ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_AVCProfileMain;
                    pH264ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_AVCLevel41;
                } else if( pH264ProfileLevelParam->nProfileIndex == 2 ) {
                    pH264ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_AVCProfileHigh;
                    pH264ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_AVCLevel41;
                } else {
                    eError = OMX_ErrorNoMore;
                }
            } else if( pH264ProfileLevelParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoProfileLevelQuerySupported supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;
        case OMX_IndexParamVideoProfileLevelCurrent :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PROFILELEVELTYPE, eError);
            eError = OMX_ErrorNoMore;
        }
        break;
        case OMX_IndexParamVideoMacroblocksPerFrame :
        {
            OMX_U32    MBwidth = 0, MBheight = 0;
            /* Check for the correct nSize & nVersion information */
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_MACROBLOCKSTYPE, eError);
            MBwidth = (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->format.video.nFrameWidth) / 16;
            MBwidth = MBwidth + ((pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->format.video.nFrameWidth) % 16);
            MBheight = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->format.video.nFrameHeight / 16;
            MBheight = MBheight + ((pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->format.video.nFrameHeight) % 16);
            ((OMX_PARAM_MACROBLOCKSTYPE *)(pParamStruct))->nMacroblocks = MBwidth * MBheight;
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexParamVideoEnableMetadata :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_DECMETADATA, eError);

            ((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableMBInfo = pVideoDecoderComPvt->tMetadata.bEnableMBInfo;
            if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->operatingMode == IVIDEO_TRANSCODE_FRAMELEVEL ) {
                ((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableTranscodeMode = OMX_TRUE;
            } else {
                ((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableTranscodeMode = OMX_FALSE;
            }
            ((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableSEIInfo = pVideoDecoderComPvt->tMetadata.bEnableSEIInfo;
            ((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableVUIInfo = pVideoDecoderComPvt->tMetadata.bEnableVUIInfo;
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexParamVideoDataSyncMode :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_DATASYNCMODETYPE, eError);

            /* GetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);
            if( ((OMX_VIDEO_PARAM_DATASYNCMODETYPE *) pParamStruct)->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                if( ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_ENTIREFRAME ) {
                   ((OMX_VIDEO_PARAM_DATASYNCMODETYPE *) pParamStruct)->eDataMode = OMX_Video_EntireFrame;
                   ((OMX_VIDEO_PARAM_DATASYNCMODETYPE *) pParamStruct)->nNumDataUnits = 0;
                } else if( ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_SLICEMODE ) {
                   ((OMX_VIDEO_PARAM_DATASYNCMODETYPE *) pParamStruct)->eDataMode = OMX_Video_SliceMode;
                   ((OMX_VIDEO_PARAM_DATASYNCMODETYPE *) pParamStruct)->nNumDataUnits = 1;
                }
            }
            else {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "OMX_TI_IndexParamVideoDataSyncMode supported only on i/p port");
            }
        }
        break;
        default :
            eError = OMX_TI_VIDEODECODER_GetParameter(hComponent, nIndex, pParamStruct);
    }

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_H264VD_HandleError()
 *                     This method is called by the common decoder to h264 specific error handling
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_H264VD_HandleError(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_U32                     nRefFramesOld, nRefFrames41, nRefFrames5;
    OMX_H264VD_PVTTYPE         *pH264vdComPvt = NULL;
    IH264VDEC_Params           *staticparams = NULL;
    IH264VDEC_Status           *pDecExtStatus = NULL;
    BUFF_POINTERS              *pDecExtStructure = NULL;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_U32                     nBufferCountMin_old = 0;

    /* Initialize pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    staticparams = (IH264VDEC_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams);
    pH264vdComPvt =(OMX_H264VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;
    pDecExtStructure = &(pVideoDecoderComPvt->pDecStructures);

    pDecExtStatus = (IH264VDEC_Status *)
                    (pDecExtStructure->pDecStatus);
    if( pH264vdComPvt->tH264VideoParam.nRefFrames == 0xFFFFFFFF ) {
        nRefFramesOld = OMX_TI_H264VD_Calculate_TotalRefFrames
                            (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameWidth,
                            pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameHeight,
                            pH264vdComPvt->tH264VideoParam.eLevel) - 1;
    } else {
        nRefFramesOld = pH264vdComPvt->tH264VideoParam.nRefFrames;
    }
    nRefFrames41 = OMX_TI_H264VD_Calculate_TotalRefFrames
                       (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameWidth,
                       pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameHeight,
                       OMX_VIDEO_AVCLevel41) - 1;
    nRefFrames5 = OMX_TI_H264VD_Calculate_TotalRefFrames
                      (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameWidth,
                      pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.nFrameHeight,
                      OMX_VIDEO_AVCLevel5) - 1;
    nBufferCountMin_old = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin;
    if( pDecExtStatus->spsMaxRefFrames > nRefFramesOld ) {
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "spsMaxRefFrames = %d, nRefFrames set initially = %d",
                           pDecExtStatus->spsMaxRefFrames, pH264vdComPvt->tH264VideoParam.nRefFrames);
        pH264vdComPvt->tH264VideoParam.nRefFrames
            = pDecExtStatus->spsMaxRefFrames;
        staticparams->dpbSizeInFrames
            = pDecExtStatus->spsMaxRefFrames;
        staticparams->viddec3Params.displayDelay
            = pDecExtStatus->spsMaxRefFrames;
        if( pH264vdComPvt->tH264VideoParam.eProfile == OMX_VIDEO_AVCProfileBaseline ) {
            staticparams->viddec3Params.displayDelay
                = IVIDDEC3_DECODE_ORDER;
            pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin
                = pH264vdComPvt->tH264VideoParam.nRefFrames + 1;
            pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountActual
                = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin + 2;
        } else {
            staticparams->viddec3Params.displayDelay
                = pDecExtStatus->spsMaxRefFrames;
            pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin
                = 2 * pH264vdComPvt->tH264VideoParam.nRefFrames + 1;
            pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountActual
                = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin + 2;
        }
        if( pDecExtStatus->spsMaxRefFrames > nRefFrames5 ) {
            pH264vdComPvt->tH264VideoParam.eLevel
                = OMX_VIDEO_AVCLevel51;
            staticparams->presetLevelIdc
                =  IH264VDEC_LEVEL51;
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Resetting level of the stream to Level 5.1");
            pVideoDecoderComPvt->nCodecRecreationRequired = 1;
            pVideoDecoderComPvt->nOutPortReconfigRequired = 1;
        } else if( pDecExtStatus->spsMaxRefFrames > nRefFrames41 ) {
            pH264vdComPvt->tH264VideoParam.eLevel
                = OMX_VIDEO_AVCLevel5;
            staticparams->presetLevelIdc
                =  IH264VDEC_LEVEL5;
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Resetting level of the stream to Level 5");
            pVideoDecoderComPvt->nCodecRecreationRequired = 1;
            pVideoDecoderComPvt->nOutPortReconfigRequired = 1;
        } else if( pDecExtStatus->spsMaxRefFrames > nRefFramesOld ) {
            pH264vdComPvt->tH264VideoParam.eLevel
                = OMX_VIDEO_AVCLevel41;
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Resetting level of the stream to Level 4.1");
        }
        if( pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin
            > nBufferCountMin_old ) {
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "nBufferCountMin_old = %d, nBufferCountMin_new = %d",
                               nBufferCountMin_old, pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin);
            pVideoDecoderComPvt->nOutPortReconfigRequired = 1;
            pVideoDecoderComPvt->nCodecRecreationRequired = 1;
        }
    }
    return (eError);
}

