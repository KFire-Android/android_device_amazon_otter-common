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
*   @file  omx_mpeg4dec.c
*   This file contains the implementation of the codec specific layer for
*   MPEG4 Decoder
*
*   @path \WTSD_DucatiMMSW\omx\omx_il_1_x\omx_video_dec\omx_mpeg4_dec\src
*
*  @rev 1.0
*/
#include <omx_mpeg4vd.h>


/* ==========================================================================*/
/**
 * @fn OMX_TI_MPEG4VD_ComponentInit()
 *                     This method will initialize the MPEG4 Decoder client
 *                     component. This method is called by the IL-Client to
 *                     initialize the component containing VC1 Decoder.
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_MPEG4VD_ComponentInit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE        eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE   *pHandle = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;

    /*! Call the Common Decoder Component Init */
    eError = OMX_TI_VideoDecoder_ComponentInit(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    /*! Call the MPEG4 Decoder Specific Init */
    eError = OMX_MPEG4VD_Init(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    /*! Initialize the Decoder Base Params */
    OMX_TI_Init_Video_decoder_params(hComponent, pHandle->pComponentPrivate);

    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    strcpy((char *)pVideoDecoderComPvt->tComponentRole.cRole,
           "video_decoder.mpeg4");

    pVideoDecoderComPvt->strComponentName = OMX_MPEG4VD_COMPONENT_NAME;

EXIT:
    return (eError);
}

OMX_ERRORTYPE OMX_MPEG4VD_Init(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_MPEG4VD_PVTTYPE        *pMPEG4vdComPvt = NULL;

    eError = OMX_MPEG4_H263VD_Init(hComponent);

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pMPEG4vdComPvt =(OMX_MPEG4VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;
    pVideoDecoderComPvt->eCompressionFormat = OMX_VIDEO_CodingMPEG4;
    pMPEG4vdComPvt->bIsSorensonSpark = OMX_FALSE;

    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                       "\n Default Codec selected : MPEG4\n");

    return (eError);
}

OMX_ERRORTYPE OMX_SORENSONSPKVD_Init(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_MPEG4VD_PVTTYPE        *pMPEG4vdComPvt = NULL;

    eError = OMX_MPEG4_H263VD_Init(hComponent);

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pMPEG4vdComPvt =(OMX_MPEG4VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;
    pVideoDecoderComPvt->eCompressionFormat = (OMX_VIDEO_CODINGTYPE) OMX_TI_VIDEO_CodingSORENSONSPK;
    pMPEG4vdComPvt->bIsSorensonSpark = OMX_TRUE;

    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                       "\n Codec selected : Sorenson spark\n");

    return (eError);
}

OMX_ERRORTYPE OMX_H263VD_Init(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_MPEG4VD_PVTTYPE        *pMPEG4vdComPvt = NULL;

    eError = OMX_MPEG4_H263VD_Init(hComponent);

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    pMPEG4vdComPvt =(OMX_MPEG4VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    pVideoDecoderComPvt->eCompressionFormat = OMX_VIDEO_CodingH263;
    pMPEG4vdComPvt->bIsSorensonSpark = OMX_FALSE;

    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                       "\n Codec selected : H263\n");

    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_MPEG4VD_Init()
 *                     This method will perform the MPEG4 Decoder specific
 *                     initialization. This is called during initialziation.
 *                     This function initializes the function pointers and
 *                     allocates memory required specifically for MPEG4 decoder
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_MPEG4_H263VD_Init(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_MPEG4VD_PVTTYPE        *pMPEG4vdComPvt = NULL;
    BUFF_POINTERS              *decStructPtr = NULL;
    BASECLASS_BUFF_POINTERS    *decBaseClassPtr;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    /*! Initialize the function pointers */
    pVideoDecoderComPvt->fpSet_StaticParams = OMX_MPEG4VD_Set_StaticParams;
    pVideoDecoderComPvt->fpSet_DynamicParams = OMX_MPEG4VD_Set_DynamicParams;
    pVideoDecoderComPvt->fpSet_Status = OMX_MPEG4VD_Set_Status;
    pVideoDecoderComPvt->fpCalc_OubuffDetails
        = Calculate_TI_MPEG4VD_outbuff_details;
    pVideoDecoderComPvt->fpDeinit_Codec = OMX_MPEG4VD_DeInit;
    pVideoDecoderComPvt->strDecoderName = "ivahd_mpeg4dec";
    pHandle->SetParameter = OMX_TI_MPEG4VD_SetParameter;
    pHandle->GetParameter = OMX_TI_MPEG4VD_GetParameter;
    pVideoDecoderComPvt->fpHandle_ExtendedError = OMX_TI_MPEG4VD_HandleError;

    /*! Initialize the framerate divisor to 10000 as WA because MPEG4 codec is providing framerate by multiplying with 10000 instead of 1000*/
    pVideoDecoderComPvt->nFrameRateDivisor   = 10000;

    pVideoDecoderComPvt->pCodecPrivate =
        (OMX_MPEG4VD_PVTTYPE *) TIMM_OSAL_Malloc(sizeof (OMX_MPEG4VD_PVTTYPE),
                                                 TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT((pVideoDecoderComPvt->pCodecPrivate) != NULL, OMX_ErrorInsufficientResources);

    pMPEG4vdComPvt =(OMX_MPEG4VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    OMX_BASE_INIT_STRUCT_PTR(&(pMPEG4vdComPvt->tDeblockingParam), OMX_PARAM_DEBLOCKINGTYPE);
    pMPEG4vdComPvt->tDeblockingParam.nPortIndex = OMX_VIDEODECODER_OUTPUT_PORT;
    pMPEG4vdComPvt->tDeblockingParam.bDeblocking = OMX_TRUE;

    OMX_BASE_INIT_STRUCT_PTR(&(pMPEG4vdComPvt->tDeblockingQpParam), OMX_TI_VIDEO_PARAM_DEBLOCKINGQP);
    pMPEG4vdComPvt->tDeblockingQpParam.nPortIndex = OMX_VIDEODECODER_OUTPUT_PORT;
    pMPEG4vdComPvt->tDeblockingQpParam.nDeblockingQP = 0;

    OMX_BASE_INIT_STRUCT_PTR(&(pMPEG4vdComPvt->tMPEG4VideoParam), OMX_VIDEO_PARAM_MPEG4TYPE);
    pMPEG4vdComPvt->tMPEG4VideoParam.nPortIndex = OMX_VIDEODECODER_INPUT_PORT;
    pMPEG4vdComPvt->tMPEG4VideoParam.eProfile = OMX_VIDEO_MPEG4ProfileAdvancedSimple;
    pMPEG4vdComPvt->tMPEG4VideoParam.eLevel = OMX_VIDEO_MPEG4Level5;

    /*! Allocate Memory for variables to be passed to Codec */
    decStructPtr = &(pVideoDecoderComPvt->pDecStructures);
    decBaseClassPtr = &(pVideoDecoderComPvt->pBaseClassStructers);

    /*! Allocate Memory for Static Parameter */
    decStructPtr->pDecStaticParams
        = (IMPEG4VDEC_Params *) TIMM_OSAL_Malloc(sizeof(IMPEG4VDEC_Params),
                                                 TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecStaticParams != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecStaticParams, 0x0,
                     sizeof(IMPEG4VDEC_Params));

    /*! Allocate Memory for Dynamic Parameter */
    decStructPtr->pDecDynParams = (IMPEG4VDEC_DynamicParams *)
                                  TIMM_OSAL_Malloc(sizeof(IMPEG4VDEC_DynamicParams), TIMM_OSAL_TRUE,
                                                   0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecDynParams != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecDynParams, 0x0,
                     sizeof(IMPEG4VDEC_DynamicParams));

    /*! Allocate Memory for Status Structure */
    decStructPtr->pDecStatus
        = (IMPEG4VDEC_Status *) TIMM_OSAL_Malloc(sizeof(IMPEG4VDEC_Status),
                                                 TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecStatus != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecStatus, 0x0, sizeof(IMPEG4VDEC_Status));

    /*! Allocate Memory for Input Arguments */
    decStructPtr->pDecInArgs
        = (IMPEG4VDEC_InArgs *) TIMM_OSAL_Malloc(sizeof(IMPEG4VDEC_InArgs),
                                                 TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecInArgs != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecInArgs, 0x0, sizeof(IMPEG4VDEC_InArgs));

    /*! Allocate Memory for Output Arguments */
    decStructPtr->pDecOutArgs
        = (IMPEG4VDEC_OutArgs *) TIMM_OSAL_Malloc(sizeof(IMPEG4VDEC_OutArgs),
                                                  TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(decStructPtr->pDecOutArgs != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(decStructPtr->pDecOutArgs, 0x0,
                     sizeof(IMPEG4VDEC_OutArgs));

    /*! Now initialize pointers of base class structures */
    decBaseClassPtr->pDecStaticParams
        = &((IMPEG4VDEC_Params * )decStructPtr->pDecStaticParams)->viddec3Params;
    decBaseClassPtr->pDecDynParams
        = &((IMPEG4VDEC_DynamicParams * )
            decStructPtr->pDecDynParams)->viddec3DynamicParams;
    decBaseClassPtr->pDecStatus
        = &((IMPEG4VDEC_Status * )decStructPtr->pDecStatus)->viddec3Status;
    decBaseClassPtr->pDecInArgs
        = &((IMPEG4VDEC_InArgs * )decStructPtr->pDecInArgs)->viddec3InArgs;
    decBaseClassPtr->pDecOutArgs
        = &((IMPEG4VDEC_OutArgs * )decStructPtr->pDecOutArgs)->viddec3OutArgs;

    decBaseClassPtr->pDecInArgs->size = sizeof(IMPEG4VDEC_InArgs);
    decBaseClassPtr->pDecOutArgs->size = sizeof(IMPEG4VDEC_OutArgs);


    pVideoDecoderComPvt->tMetadata.bEnableMBInfo = OMX_FALSE;
    pVideoDecoderComPvt->tMetadata.bEnableSEIInfo = OMX_FALSE;
    pVideoDecoderComPvt->tMetadata.bEnableVUIInfo = OMX_FALSE;

    decBaseClassPtr->pDecStaticParams->metadataType[0] = IVIDEO_METADATAPLANE_NONE;
    decBaseClassPtr->pDecStaticParams->metadataType[1] = IVIDEO_METADATAPLANE_NONE;
    decBaseClassPtr->pDecStaticParams->metadataType[2] = IVIDEO_METADATAPLANE_NONE;

    decBaseClassPtr->pDecStaticParams->operatingMode = IVIDEO_DECODE_ONLY;

    pVideoDecoderComPvt->bErrorConcealmentBuffer = OMX_FALSE;
    pVideoDecoderComPvt->nMBInfoStructSize = 112;

    decBaseClassPtr->pDecStaticParams->inputDataMode = IVIDEO_ENTIREFRAME;
    decBaseClassPtr->pDecStaticParams->numInputDataUnits  = 0;

EXIT:
    return (eError);

}

/* ==========================================================================*/
/**
 * @fn OMX_MPEG4VD_Set_StaticParams()
 *                     This method will initialize the Static Parameters
 *                     required for the MPEG4 Decoder codec
 *
 *  @param [in] pstaticParams : Pointer to the static params of VC1 decoder
 *  @param [in] width         : Maximum width of frame
 *  @param [in] height        : Maximum height of frame
 *
 */
/* ==========================================================================*/

void OMX_MPEG4VD_Set_StaticParams(OMX_HANDLETYPE hComponent, void *staticparams)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_MPEG4VD_PVTTYPE        *pMPEG4vdComPvt = NULL;
    IMPEG4VDEC_Params          *params;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pMPEG4vdComPvt =
        (OMX_MPEG4VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    params = (IMPEG4VDEC_Params *) staticparams;

    params->viddec3Params.size = sizeof(IMPEG4VDEC_Params);
    /* Max Frame Rate: Not currently used in the algorithm                  */
    params->viddec3Params.maxFrameRate        = 300000;
    /* Max Bit Rate: Not currently used in the algorithm                    */
    params->viddec3Params.maxBitRate          = 10000000;
    /* Data Endianness (1: Big) : only Big Endian supported                 */
    params->viddec3Params.dataEndianness      = XDM_BYTE;
    /* Max Image Height  Supported ----------------------------------------- */
    //params->viddec3Params.maxHeight = height;
    /* Max Image Width  Supported ------------------------------------------- */
    //params->viddec3Params.maxWidth = width;
    /*-----------------------------------------------------------------------*/
    /*  forceChromaFormat :  YUV 4:2:0 (Planner)*/
    /*-----------------------------------------------------------------------*/
    params->viddec3Params.forceChromaFormat      = XDM_YUV_420SP;
    /*-----------------------------------------------------------------------*/
    /*  displayDelay :  Video decoder output frame order                     */
    /*-----------------------------------------------------------------------*/
    params->viddec3Params.displayDelay      = IVIDDEC3_DISPLAY_DELAY_1;
    /*-----------------------------------------------------------------------*/
    /*  inputDataMode :  Video input data mode                               */
    /*-----------------------------------------------------------------------*/
    params->viddec3Params.inputDataMode      = IVIDEO_ENTIREFRAME;
    /*-----------------------------------------------------------------------*/
    /*  numInputDataUnits :  Granularity of the data if DATA_SYNC is enabled  */
    /*-----------------------------------------------------------------------*/
    params->viddec3Params.numInputDataUnits  = 0;
    /*-----------------------------------------------------------------------*/
    /*  outputDataMode :  Video output data mode                             */
    /*-----------------------------------------------------------------------*/
    params->viddec3Params.outputDataMode      = IVIDEO_ENTIREFRAME;
    /*-----------------------------------------------------------------------*/
    /* Indicates which mode the displayBufs are presented in:                */
    /* pointer Vs Structure                                                  */
    /*-----------------------------------------------------------------------*/
    params->viddec3Params.displayBufsMode = IVIDDEC3_DISPLAYBUFS_EMBEDDED;
    //params->viddec3Params.bFrameColocatedMBStore = 0;

    params->viddec3Params.errorInfoMode  = IVIDEO_ERRORINFO_OFF;

    if(pMPEG4vdComPvt->tDeblockingParam.bDeblocking == OMX_FALSE) {
        params->outloopDeBlocking = IMPEG4VDEC_DEBLOCK_DISABLE;
        params->enhancedDeBlockingQp = 31;
    }
    else if(pMPEG4vdComPvt->tDeblockingParam.bDeblocking == OMX_TRUE && pMPEG4vdComPvt->tDeblockingQpParam.nDeblockingQP == 0) {
        params->outloopDeBlocking = IMPEG4VDEC_DEBLOCK_ENABLE;
        params->enhancedDeBlockingQp = 31;
    }
    else {
        params->outloopDeBlocking = IMPEG4VDEC_ENHANCED_DEBLOCK_ENABLE;
        if(pMPEG4vdComPvt->tDeblockingQpParam.nDeblockingQP > 31)
            params->enhancedDeBlockingQp = 31;
        else
            params->enhancedDeBlockingQp = pMPEG4vdComPvt->tDeblockingQpParam.nDeblockingQP;
    }

    params->sorensonSparkStream = pMPEG4vdComPvt->bIsSorensonSpark;
    params->errorConcealmentEnable =
        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->format.video.bFlagErrorConcealment;
    params->debugTraceLevel= 0;
    params->lastNFramesToLog= 0;
    params->paddingMode = IMPEG4VDEC_MPEG4_MODE_PADDING;

    return;
}

/* ==========================================================================*/
/**
 * @fn OMX_MPEG4VD_Set_DynamicParams()
 *                     This method will initialize the Dynamic Parameters
 *                     required for the VC1 xDM component
 *
 *  @param [in] pdynamicParams : Pointer to the Dynamic params of VC1 decoder
 *
 */
/* ==========================================================================*/
void OMX_MPEG4VD_Set_DynamicParams(OMX_HANDLETYPE hComponent, void *dynParams)
{
    IMPEG4VDEC_DynamicParams   *dynamicParams;

    dynamicParams = (IMPEG4VDEC_DynamicParams *)dynParams;

    /*! Update the Individual fields in the Dyanmic Params of MPEG4 decoder */
    dynamicParams->viddec3DynamicParams.size = sizeof(IMPEG4VDEC_DynamicParams);
    dynamicParams->viddec3DynamicParams.decodeHeader  = XDM_DECODE_AU;
    dynamicParams->viddec3DynamicParams.displayWidth  = 0;
    dynamicParams->viddec3DynamicParams.frameSkipMode = IVIDEO_NO_SKIP;
    dynamicParams->viddec3DynamicParams.newFrameFlag  = XDAS_TRUE;
    dynamicParams->viddec3DynamicParams.putDataFxn = NULL;
    dynamicParams->viddec3DynamicParams.putDataHandle = NULL;
    dynamicParams->viddec3DynamicParams.getDataFxn = NULL;
    dynamicParams->viddec3DynamicParams.getDataHandle = NULL;
    dynamicParams->viddec3DynamicParams.putBufferFxn = NULL;
    dynamicParams->viddec3DynamicParams.putBufferHandle = NULL;
   // dynamicParams->viddec3DynamicParams.lateAcquireArg
     //   = IRES_HDVICP2_UNKNOWNLATEACQUIREARG;
    return;
}

/* ==========================================================================*/
/**
 * @fn OMX_MPEG4VD_Set_Status()
 *                     This method will initialize the parameters in the
 *                     Status xDM structure
 *
 *  @param [in] pStatus : Pointer to the status params of VC1 decoder
 *
 */
/* ==========================================================================*/

void OMX_MPEG4VD_Set_Status(OMX_HANDLETYPE hComponent, void *decstatus)
{
    IMPEG4VDEC_Status   *status;

    status = (IMPEG4VDEC_Status *)decstatus;

    status->viddec3Status.size     = sizeof(IMPEG4VDEC_Status);
    return;
}

/* ==========================================================================*/
/**
 * @fn Calculate_TI_MPEG4VD_outbuff_details()
 *                     This method will calcullate the Buffer Details by
 *                     taking the width and height as parameters
 *
 *  @param [in] nWidth  : Width of the frame
 *  @param [in] nHeight : Height of the frame
 *
 */
/* ==========================================================================*/
BUFF_PARAMS Calculate_TI_MPEG4VD_outbuff_details(OMX_HANDLETYPE hComponent,
                                                 OMX_U32 width, OMX_U32 height)
{
    BUFF_PARAMS    OutBuffDetails;

    OutBuffDetails.nBufferSize
        = ((((width + PADX + 127) & ~127) * (height + PADY)));
    /* Multiply buffer size by 1.5 to account for both luma and chroma */
    OutBuffDetails.nBufferSize = (OutBuffDetails.nBufferSize * 3) >> 1;

    OutBuffDetails.nBufferCountMin = 4;
    OutBuffDetails.nBufferCountActual = 8;
    OutBuffDetails.n1DBufferAlignment = 16;
    OutBuffDetails.nPaddedWidth = (width + PADX + 127) & ~127;
    OutBuffDetails.nPaddedHeight = height + PADY;
    OutBuffDetails.n2DBufferYAlignment = 1;
    OutBuffDetails.n2DBufferXAlignment = 16;

    return (OutBuffDetails);
}

/* ==========================================================================*/
/**
 * @fn OMX_MPEG4VD_DeInit()
 *                     This method will de-initialize the VC1 Decoder
 *                     component
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/

void OMX_MPEG4VD_DeInit(OMX_HANDLETYPE hComponent)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
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
    pVideoDecoderComPvt->nFrameRateDivisor      = 1000;
}

OMX_ERRORTYPE OMX_TI_MPEG4VD_SetParameter(OMX_HANDLETYPE hComponent,
                                          OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE           *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE    *pVideoDecoderComPvt  = NULL;
    OMX_MPEG4VD_PVTTYPE         *pMPEG4vdComPvt = NULL;
    OMX_PARAM_DEBLOCKINGTYPE    *pDeblockingParam = NULL;
    OMX_VIDEO_PARAM_MPEG4TYPE   *pMPEG4VideoParam = NULL;
    OMX_TI_VIDEO_PARAM_DEBLOCKINGQP  *pDeblockingQpParam = NULL;

    OMX_BASE_REQUIRE((hComponent != NULL) && (pParamStruct != NULL),
                     OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pMPEG4vdComPvt =(OMX_MPEG4VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    switch( nIndex ) {
        case OMX_IndexParamVideoMpeg4 :

            pMPEG4VideoParam = (OMX_VIDEO_PARAM_MPEG4TYPE *) pParamStruct;
            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[pMPEG4VideoParam->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_MPEG4TYPE, eError);
            OMX_BASE_ASSERT(pMPEG4VideoParam->eProfile == OMX_VIDEO_MPEG4ProfileSimple
                            || pMPEG4VideoParam->eProfile == OMX_VIDEO_MPEG4ProfileMain
                            || pMPEG4VideoParam->eProfile == OMX_VIDEO_MPEG4ProfileAdvancedSimple,
                            OMX_ErrorUnsupportedSetting);
            if( pMPEG4VideoParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                pMPEG4vdComPvt->tMPEG4VideoParam = *pMPEG4VideoParam;
            } else if( pMPEG4VideoParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoMpeg4 supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        case OMX_IndexParamCommonDeblocking :

            pDeblockingParam = (OMX_PARAM_DEBLOCKINGTYPE *) pParamStruct;

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[pDeblockingParam->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_DEBLOCKINGTYPE, eError);
            if( pDeblockingParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pMPEG4vdComPvt->tDeblockingParam.bDeblocking = pDeblockingParam->bDeblocking;
            } else if( pDeblockingParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "\n Deblocking supported only on o/p port...  \n");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        case OMX_TI_IndexParamVideoDeblockingQP :

            pDeblockingQpParam = (OMX_TI_VIDEO_PARAM_DEBLOCKINGQP *) pParamStruct;

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[pDeblockingParam->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_VIDEO_PARAM_DEBLOCKINGQP, eError);
            if( pDeblockingQpParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pMPEG4vdComPvt->tDeblockingQpParam.nDeblockingQP = pDeblockingQpParam->nDeblockingQP;
            } else if( pDeblockingQpParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "\n SetParam on Deblocking QP supported only on o/p port...  \n");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;

        default :
            eError = OMX_TI_VIDEODECODER_SetParameter(hComponent, nIndex, pParamStruct);
    }

EXIT:
    return (eError);
}

OMX_ERRORTYPE OMX_TI_MPEG4VD_GetParameter(OMX_HANDLETYPE hComponent,
                                          OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                       eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                  *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE           *pVideoDecoderComPvt  = NULL;
    OMX_MPEG4VD_PVTTYPE                *pMPEG4vdComPvt = NULL;
    OMX_PARAM_DEBLOCKINGTYPE           *pDeblockingParam = NULL;
    OMX_VIDEO_PARAM_MPEG4TYPE          *pMPEG4VideoParam = NULL;
    OMX_VIDEO_PARAM_PROFILELEVELTYPE   *pMPEG4ProfileLevelParam = NULL;
    OMX_TI_VIDEO_PARAM_DEBLOCKINGQP    *pDeblockingQpParam = NULL;

    OMX_BASE_REQUIRE((hComponent != NULL) && (pParamStruct != NULL),
                     OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    /* GetParameter can't be invoked incase the comp is in Invalid State  */
    OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState != OMX_StateInvalid,
                    OMX_ErrorIncorrectStateOperation);

    pMPEG4vdComPvt =(OMX_MPEG4VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    switch( nIndex ) {
        case OMX_IndexParamVideoMpeg4 :

            pMPEG4VideoParam = (OMX_VIDEO_PARAM_MPEG4TYPE *) pParamStruct;

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_MPEG4TYPE, eError);
            if( pMPEG4VideoParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                *pMPEG4VideoParam = pMPEG4vdComPvt->tMPEG4VideoParam;
            } else if( pMPEG4VideoParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoMpeg4 supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;

        case OMX_IndexParamCommonDeblocking :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_DEBLOCKINGTYPE, eError);
            pDeblockingParam = (OMX_PARAM_DEBLOCKINGTYPE *) pParamStruct;
            if( pDeblockingParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pDeblockingParam->bDeblocking = pMPEG4vdComPvt->tDeblockingParam.bDeblocking;
            } else if( pDeblockingParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "\n Deblocking supported only on o/p port...  \n");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        case OMX_IndexParamVideoProfileLevelQuerySupported :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PROFILELEVELTYPE, eError);
            pMPEG4ProfileLevelParam = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pParamStruct;
            if( pMPEG4ProfileLevelParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                if( pMPEG4ProfileLevelParam->nProfileIndex == 0 ) {
                    if( pVideoDecoderComPvt->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ) {
                        pMPEG4ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_MPEG4ProfileSimple;
                        pMPEG4ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_MPEG4LevelMax;
                    } else if( pVideoDecoderComPvt->eCompressionFormat == OMX_VIDEO_CodingH263 ) {
                        pMPEG4ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_H263ProfileBaseline;
                        pMPEG4ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_H263Level70;
                    }
                } else if( pMPEG4ProfileLevelParam->nProfileIndex == 1 ) {
                    if( pVideoDecoderComPvt->eCompressionFormat == OMX_VIDEO_CodingMPEG4 ) {
                        pMPEG4ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_MPEG4ProfileAdvancedSimple;
                        pMPEG4ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_MPEG4LevelMax;
                    } else if( pVideoDecoderComPvt->eCompressionFormat == OMX_VIDEO_CodingH263 ) {
                        pMPEG4ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_H263ProfileISWV2;
                        pMPEG4ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_H263Level70;
                    }
                } else {
                    eError = OMX_ErrorNoMore;
                }
            } else if( pMPEG4ProfileLevelParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoProfileLevelQuerySupported supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;
        case OMX_TI_IndexParamVideoDeblockingQP :

            pDeblockingQpParam = (OMX_TI_VIDEO_PARAM_DEBLOCKINGQP *) pParamStruct;

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[pDeblockingParam->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_VIDEO_PARAM_DEBLOCKINGQP, eError);
            if( pDeblockingQpParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pDeblockingQpParam->nDeblockingQP = pMPEG4vdComPvt->tDeblockingQpParam.nDeblockingQP;
            } else if( pDeblockingQpParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "\n GetParam on Deblocking QP supported only on o/p port...  \n");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
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
 * @fn OMX_TI_MPEG4VD_HandleError()
 *                     This method is called by the common decoder to MPEG4 specific error handling
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_MPEG4VD_HandleError(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;

    /* Initialize pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    /* To send the output buffer to display in case of no valid VOL/VOP header */

    if((!(pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError & 0x8000)
        && ((pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError & 0x100000)
            || (pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError & 0x80000)))
       && (pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID == pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->freeBufID[0]
           && pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->outputID[0] == 0)) {
        pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->outputID[0] = pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID;
        pVideoDecoderComPvt->bSyncFrameReady = OMX_TRUE;
    }

    return (eError);
}

