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
*   @file  omx_vc1dec.c
*   This file contains the implementation of the codec specific layer for
*   VC1 Decoder
*
*   @path \WTSD_DucatiMMSW\omx\omx_il_1_x\omx_video_dec\omx_vc1_dec\src
*
*  @rev 1.0
*/
#include <omx_vc1dec.h>


/* ==========================================================================*/
/**
 * @fn OMX_TI_VC1D_ComponentInit()
 *                     This method will initialize the VC1 Decoder client
 *                     component. This method is called by the IL-Client to
 *                     initialize the component containing VC1 Decoder.
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VC1D_ComponentInit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE        eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE   *pHandle = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;

    /*! Call the Common Decoder Component Init */
    eError = OMX_TI_VideoDecoder_ComponentInit(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    /*! Call the VC1 Decoder Specific Init */
    eError = OMX_VC1VD_Init(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    /*! Initialize the Decoder Base Params */
    OMX_TI_Init_Video_decoder_params(hComponent, pHandle->pComponentPrivate);

    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    strcpy((char *)pVideoDecoderComPvt->tComponentRole.cRole,
           "video_decoder.wmv");

    pVideoDecoderComPvt->strComponentName = OMX_TI_VC1D_COMPONENT_NAME;

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_VC1VD_Init()
 *                     This method will perform the VC1 Decoder specific
 *                     initialization. This is called during initialziation.
 *                     This function initializes the function pointers and
 *                     allocates memory required specifically for VC1 decoder
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_VC1VD_Init(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_VC1VD_PVTTYPE          *pVC1vdComPvt = NULL;
    BUFF_POINTERS              *decStructPtr = NULL;
    BASECLASS_BUFF_POINTERS    *decBaseClassPtr;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    /*! Initialize the function parameters for OMX functions */
    pHandle->SetParameter = OMX_TI_VC1VD_SetParameter;
    pHandle->GetParameter = OMX_TI_VC1VD_GetParameter;

    /*! Initialize the function pointers */
    pVideoDecoderComPvt->fpSet_StaticParams   = OMX_TI_VC1VD_Set_StaticParams;
    pVideoDecoderComPvt->fpSet_DynamicParams  = OMX_TI_VC1VD_Set_DynamicParams;
    pVideoDecoderComPvt->fpSet_Status         = OMX_TI_VC1VD_Set_Status;
    pVideoDecoderComPvt->fpCalc_OubuffDetails = OMX_TI_VC1VD_Calc_Outbuf_details;
    pVideoDecoderComPvt->fpDeinit_Codec       = OMX_TI_VC1D_DeInit;
    pVideoDecoderComPvt->strDecoderName       = "ivahd_vc1vdec";
    pVideoDecoderComPvt->eCompressionFormat   = OMX_VIDEO_CodingWMV;
    pVideoDecoderComPvt->fpHandle_ExtendedError = OMX_TI_VC1VD_HandleError;

    /*! Initialize the framerate divisor to 1 as WA because VC1 codec is providing framerate without multiplying with 1000*/
    pVideoDecoderComPvt->nFrameRateDivisor      = 1;

    pVideoDecoderComPvt->pCodecPrivate =
        (OMX_VC1VD_PVTTYPE *) TIMM_OSAL_Malloc(sizeof (OMX_VC1VD_PVTTYPE),
                                               TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT((pVideoDecoderComPvt->pCodecPrivate) != NULL, OMX_ErrorInsufficientResources);

    pVC1vdComPvt =(OMX_VC1VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    OMX_BASE_INIT_STRUCT_PTR(&(pVC1vdComPvt->tVC1VideoParam), OMX_VIDEO_PARAM_WMVTYPE);
    pVC1vdComPvt->tVC1VideoParam.nPortIndex = OMX_VIDEODECODER_INPUT_PORT;
    pVC1vdComPvt->tVC1VideoParam.eFormat = OMX_VIDEO_WMVFormat9;

    /*! Allocate Memory for variables to be passed to Codec */
    decStructPtr = &(pVideoDecoderComPvt->pDecStructures);
    decBaseClassPtr = &(pVideoDecoderComPvt->pBaseClassStructers);

    /*! Allocate Memory for Static Parameter */
    decStructPtr->pDecStaticParams
        = (IVC1VDEC_Params *)VC1_MEMALLOC(sizeof(IVC1VDEC_Params));
    VC1_NULLPTRCHECK(decStructPtr->pDecStaticParams);
    TIMM_OSAL_Memset(decStructPtr->pDecStaticParams, 0x0,
                     sizeof(IVC1VDEC_Params));

    /*! Allocate Memory for Dynamic Parameter */
    decStructPtr->pDecDynParams = (IVC1VDEC_DynamicParams *)
                                  VC1_MEMALLOC(sizeof(IVC1VDEC_DynamicParams));
    VC1_NULLPTRCHECK(decStructPtr->pDecDynParams);
    TIMM_OSAL_Memset(decStructPtr->pDecDynParams, 0x0, sizeof(IVC1VDEC_DynamicParams));

    /*! Allocate Memory for Status Structure */
    decStructPtr->pDecStatus
        = (IVC1VDEC_Status *) VC1_MEMALLOC(sizeof(IVC1VDEC_Status));
    VC1_NULLPTRCHECK(decStructPtr->pDecStatus);
    TIMM_OSAL_Memset(decStructPtr->pDecStatus, 0x0, sizeof(IVC1VDEC_Status));

    /*! Allocate Memory for Input Arguments */
    decStructPtr->pDecInArgs
        = (IVC1VDEC_InArgs *) VC1_MEMALLOC(sizeof(IVC1VDEC_InArgs));
    VC1_NULLPTRCHECK(decStructPtr->pDecInArgs);
    TIMM_OSAL_Memset(decStructPtr->pDecInArgs, 0x0, sizeof(IVC1VDEC_InArgs));

    /*! Allocate Memory for Output Arguments */
    decStructPtr->pDecOutArgs
        = (IVC1VDEC_OutArgs *) VC1_MEMALLOC(sizeof(IVC1VDEC_OutArgs));
    VC1_NULLPTRCHECK(decStructPtr->pDecOutArgs);
    TIMM_OSAL_Memset(decStructPtr->pDecOutArgs, 0x0, sizeof(IVC1VDEC_OutArgs));

    /*! Now initialize pointers of base class structures */
    decBaseClassPtr->pDecStaticParams
        = &(((IVC1VDEC_Params *)decStructPtr->pDecStaticParams)->viddecParams);
    decBaseClassPtr->pDecDynParams
        = &(((IVC1VDEC_DynamicParams *)
             decStructPtr->pDecDynParams)->viddecDynamicParams);
    decBaseClassPtr->pDecStatus
        = &((IVC1VDEC_Status  *)decStructPtr->pDecStatus)->viddecStatus;
    decBaseClassPtr->pDecInArgs
        = &((IVC1VDEC_InArgs *)decStructPtr->pDecInArgs)->viddecInArgs;
    decBaseClassPtr->pDecOutArgs
        = &((IVC1VDEC_OutArgs *)decStructPtr->pDecOutArgs)->viddecOutArgs;

    /*! Mark the Base Strucuture size as that of the extended structure */
    decBaseClassPtr->pDecInArgs->size = sizeof(IVC1VDEC_InArgs);
    decBaseClassPtr->pDecOutArgs->size = sizeof(IVC1VDEC_OutArgs);


    pVideoDecoderComPvt->tMetadata.bEnableMBInfo = OMX_FALSE;
    pVideoDecoderComPvt->tMetadata.bEnableSEIInfo = OMX_FALSE;
    pVideoDecoderComPvt->tMetadata.bEnableVUIInfo = OMX_FALSE;

    decBaseClassPtr->pDecStaticParams->metadataType[0] = IVIDEO_METADATAPLANE_NONE;
    decBaseClassPtr->pDecStaticParams->metadataType[1] = IVIDEO_METADATAPLANE_NONE;
    decBaseClassPtr->pDecStaticParams->metadataType[2] = IVIDEO_METADATAPLANE_NONE;

    decBaseClassPtr->pDecStaticParams->operatingMode = IVIDEO_DECODE_ONLY;
    pVideoDecoderComPvt->bErrorConcealmentBuffer = OMX_FALSE;
    pVideoDecoderComPvt->nMBInfoStructSize = 192;
    decBaseClassPtr->pDecStaticParams->inputDataMode = IVIDEO_ENTIREFRAME;
    decBaseClassPtr->pDecStaticParams->numInputDataUnits  = 0;

EXIT:
    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Exiting OMX_VC1D_Init");
    return (eError);

}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VC1VD_Set_StaticParams()
 *                     This method will initialize the Static Parameters
 *                     required for the VC1 Decoder Codec
 *
 *  @param [in] pstaticParams : Pointer to the static params of VC1 decoder
 *  @param [in] width         : Not used
 *  @param [in] height        : Not used
 *
 */
/* ==========================================================================*/
void OMX_TI_VC1VD_Set_StaticParams(OMX_HANDLETYPE hComponent, void *pstaticParams)
{
    IVC1VDEC_Params            *params = (IVC1VDEC_Params *)pstaticParams;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    params->viddecParams.size     = sizeof(IVC1VDEC_Params);
    params->viddecParams.maxHeight = VC1_DEC_MAX_FRAME_HEIGHT;
    params->viddecParams.maxWidth  = VC1_DEC_MAX_FRAME_WIDTH;
    params->viddecParams.maxFrameRate = VC1_DEC_MAX_FRAME_RATE;
    params->viddecParams.maxBitRate   = VC1_DEC_MAX_BIT_RATE;
    /* Data Endianness (1: Big) : only Big Endian supported        */
    params->viddecParams.dataEndianness = XDM_BYTE;
    /* Chroma format supported in VC-1 decoder on IVA-HD           */
    params->viddecParams.forceChromaFormat = XDM_YUV_420SP;
    /*  displayDelay :  Video decoder output frame order           */
    params->viddecParams.displayDelay = IVIDDEC3_DISPLAY_DELAY_1;
    /*  inputDataMode :  Video input data mode                     */
    params->viddecParams.inputDataMode = IVIDEO_ENTIREFRAME;
    /*  outputDataMode :  Video output data mode                   */
    params->viddecParams.outputDataMode = IVIDEO_ENTIREFRAME;
    /* Num of input data units supported in VC-1 decoder           */
    /* Set to zero since data sync is not currently supported      */
    params->viddecParams.numInputDataUnits  = 0;
    /* Number of output data units supported in VC-1 decoder       */
    /* Set to zero since data sync is not currently supported      */
    params->viddecParams.numOutputDataUnits = 0;
    /* display buffer mode provided to VC-1 decoder                */
    params->viddecParams.displayBufsMode = IVIDDEC3_DISPLAYBUFS_EMBEDDED;
    /* packet error information required for VC-1 decoder          */
    params->viddecParams.errorInfoMode = IVIDEO_ERRORINFO_OFF;
    /* set the extended parameters */
    params->errorConcealmentON = 1;       /* Set Error concealment on */
    params->frameLayerDataPresentFlag = 0;  /* Frame Layer data present */

    params->debugTraceLevel = 0;
    params->lastNFramesToLog = 0;

    pVideoDecoderComPvt->bErrorConcealmentBuffer = OMX_FALSE;

    return;

}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VC1VD_Set_DynamicParams()
 *                     This method will initialize the Dynamic Parameters
 *                     required for the VC1 xDM component
 *
 *  @param [in] pdynamicParams : Pointer to the Dynamic params of VC1 decoder
 *
 */
/* ==========================================================================*/
void OMX_TI_VC1VD_Set_DynamicParams(OMX_HANDLETYPE hComponent, void *pdynamicParams)
{
    IVC1VDEC_DynamicParams   *dynamicParams
        = (IVC1VDEC_DynamicParams *)pdynamicParams;

    /*! Update the Individual fields in the Dyanmic Params of VC1 decoder */
    dynamicParams->viddecDynamicParams.size = sizeof(IVC1VDEC_DynamicParams);
    dynamicParams->viddecDynamicParams.decodeHeader  = XDM_DECODE_AU;
    dynamicParams->viddecDynamicParams.displayWidth  = 0;
    dynamicParams->viddecDynamicParams.frameSkipMode = IVIDEO_NO_SKIP;
    dynamicParams->viddecDynamicParams.newFrameFlag  = XDAS_TRUE;

    /* Data sync related parameters supported in VC-1 decoder */
    /* Note: Data sync is currently not supported.            */
    dynamicParams->viddecDynamicParams.putDataFxn = NULL;
    dynamicParams->viddecDynamicParams.putDataHandle = NULL;
    dynamicParams->viddecDynamicParams.getDataFxn = NULL;
    dynamicParams->viddecDynamicParams.getDataHandle = NULL;
    dynamicParams->viddecDynamicParams.putBufferFxn = NULL;
    dynamicParams->viddecDynamicParams.putBufferHandle = NULL;

   // dynamicParams->viddecDynamicParams.lateAcquireArg = IRES_HDVICP2_ID_LATE_ACQUIRE;
    return;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VC1VD_Set_Status()
 *                     This method will initialize the parameters in the
 *                     Status xDM structure
 *
 *  @param [in] pStatus : Pointer to the status params of VC1 decoder
 *
 */
/* ==========================================================================*/
void OMX_TI_VC1VD_Set_Status(OMX_HANDLETYPE hComponent, void *pStatus)
{
    IVC1VDEC_Status   *status = (IVC1VDEC_Status *)pStatus;

    status->viddecStatus.size     = sizeof(IVC1VDEC_Status);
    return;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VC1VD_Calc_Outbuf_details()
 *                     This method will calcullate the Buffer Details by
 *                     taking the width and height as parameters
 *
 *  @param [in] nWidth  : Width of the frame
 *  @param [in] nHeight : Height of the frame
 *
 */
/* ==========================================================================*/
BUFF_PARAMS OMX_TI_VC1VD_Calc_Outbuf_details(OMX_HANDLETYPE hComponent,
                                             OMX_U32 nWidth, OMX_U32 nHeight)
{
    /*! ToDo: This needs to be changed after consulting PP and Sarthak */
    BUFF_PARAMS    outBuffDetails;
    OMX_U32        nWidthPad = 0;
    OMX_U32        nHeightPad = 0;

    //! Calcullate Width with Padding and make it aligned to 16-byte boundary
    // nWidthPad = (((nWidth + 15)>>4) <<4) + (2*VC1_DEC_PADX);
    nWidthPad = (((nWidth + (2 * VC1_DEC_PADX) + 0x7F) >> 7) << 7);

    //! Calcullate Height with Padding and make it aligned to 16-byte boundary
    nHeightPad = ((((nHeight / 2 + 15) >> 4) << 4) * 2) + (4 * VC1_DEC_PADY);
    // nHeightPad = (((nHeight + 15)>>4)<<4) + (2*VC1_DEC_PADY);


    outBuffDetails.nBufferSize         = (nWidthPad * nHeightPad * 3) >> 1;
    outBuffDetails.nBufferCountMin     = VC1_DEC_MIN_BUF_COUNT;
    outBuffDetails.nBufferCountActual  = VC1_DEC_ACTUAL_BUF_COUNT;
    outBuffDetails.n1DBufferAlignment  = VC1_DEC_1D_BUF_ALIGNMENT;
    outBuffDetails.nPaddedWidth        = nWidthPad;
    outBuffDetails.nPaddedHeight       = nHeightPad;
    outBuffDetails.n2DBufferYAlignment = VC1_DEC_2D_Y_ALIGNMENT;
    outBuffDetails.n2DBufferXAlignment = VC1_DEC_2D_X_ALIGNMENT;

    return (outBuffDetails);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VC1D_DeInit()
 *                     This method will de-initialize the VC1 Decoder
 *                     component
 *
 *  @param [in] hComponent : Component Handle
 *
 */
/* ==========================================================================*/
void OMX_TI_VC1D_DeInit(OMX_HANDLETYPE hComponent)
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

    return;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VC1VD_SetParameter()
 *                     This method is called by the IL Client to set parameter
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] nIndex      : Index of the parameter to be set
 *  @param [in] pParamStruct: pointer to the paramter structure to be set
 *
 */
/* ==========================================================================*/

OMX_ERRORTYPE OMX_TI_VC1VD_SetParameter(OMX_HANDLETYPE hComponent,
                                        OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt  = NULL;
    OMX_VC1VD_PVTTYPE          *pVC1vdComPvt = NULL;
    OMX_VIDEO_PARAM_WMVTYPE    *pVC1VideoParam = NULL;

    OMX_BASE_REQUIRE((hComponent != NULL) && (pParamStruct != NULL),
                     OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pVC1vdComPvt =(OMX_VC1VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    switch( nIndex ) {
        case OMX_IndexParamVideoWmv :

            pVC1VideoParam = (OMX_VIDEO_PARAM_WMVTYPE *) pParamStruct;
            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[pVC1VideoParam->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_WMVTYPE, eError);
            if( pVC1VideoParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                if( pVC1VideoParam->eFormat == OMX_VIDEO_WMVFormat9 ) {
                    pVC1vdComPvt->tVC1VideoParam = *pVC1VideoParam;
                } else {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Unsupported format");
                    eError = OMX_ErrorUnsupportedSetting;
                }
            } else if( pVC1VideoParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoVC1 supported only on i/p port");
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

/* ==========================================================================*/
/**
 * @fn OMX_TI_VC1VD_GetParameter()
 *                     This method is called by the IL Client to get parameter
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] nIndex      : Index of the parameter to get
 *  @param [in] pParamStruct: pointer to the paramter structure
 *
 */
/* ==========================================================================*/

OMX_ERRORTYPE OMX_TI_VC1VD_GetParameter(OMX_HANDLETYPE hComponent,
                                        OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                       eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                  *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE           *pVideoDecoderComPvt  = NULL;
    OMX_VC1VD_PVTTYPE                  *pVC1vdComPvt = NULL;
    OMX_VIDEO_PARAM_WMVTYPE            *pVC1VideoParam = NULL;
    OMX_VIDEO_PARAM_PROFILELEVELTYPE   *pVC1ProfileLevelParam = NULL;

    OMX_BASE_REQUIRE((hComponent != NULL) && (pParamStruct != NULL),
                     OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pVC1vdComPvt =(OMX_VC1VD_PVTTYPE *) pVideoDecoderComPvt->pCodecPrivate;

    switch( nIndex ) {
        case OMX_IndexParamVideoWmv :

            pVC1VideoParam = (OMX_VIDEO_PARAM_WMVTYPE *) pParamStruct;
            /* GetParameter can be invoked in all states except invalid state*/
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState != OMX_StateInvalid),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_WMVTYPE, eError);
            if( pVC1VideoParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                *pVC1VideoParam = pVC1vdComPvt->tVC1VideoParam;
            } else if( pVC1VideoParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoVC1 supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        case OMX_IndexParamVideoProfileLevelQuerySupported :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PROFILELEVELTYPE, eError);
            pVC1ProfileLevelParam = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pParamStruct;
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState != OMX_StateInvalid),
                            OMX_ErrorIncorrectStateOperation);
            if( pVC1ProfileLevelParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                if( pVC1ProfileLevelParam->nProfileIndex == 0 ) {
                    pVC1ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_WMVFormat9;
                } else {
                    eError = OMX_ErrorNoMore;
                }
            } else if( pVC1ProfileLevelParam->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexParamVideoProfileLevelQuerySupported supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
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
 * @fn OMX_TI_VC1VD_HandleError()
 *                     This method is called by the common decoder to VC1 specific error handling
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VC1VD_HandleError(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    XDAS_Int32                  status = 0;
    IVC1VDEC_Status            *pDecExtStatus = NULL;
    BUFF_POINTERS              *pDecExtStructure = NULL;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;

    /* Initialize pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    pDecExtStructure = &(pVideoDecoderComPvt->pDecStructures);

    /*! Call the Codec Status function to know cause of error */
    status = VIDDEC3_control(pVideoDecoderComPvt->pDecHandle,
                             XDM_GETSTATUS,
                             (VIDDEC3_DynamicParams *) pDecExtStructure->pDecDynParams,
                             (VIDDEC3_Status *) pDecExtStructure->pDecStatus);

    /* Check whether the Codec Status call was succesful */
    if( status != VIDDEC3_EOK ) {
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "VIDDEC3_control XDM_GETSTATUS failed");
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    pDecExtStatus = (IVC1VDEC_Status *)
                    (pDecExtStructure->pDecStatus);

    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                       "Extended error codes returned by codec : extendedErrorCode0 = %d, extendedErrorCode1 = %d, extendedErrorCode2 = %d, extendedErrorCode3 = %d", pDecExtStatus->extendedErrorCode0, pDecExtStatus->extendedErrorCode1, pDecExtStatus->extendedErrorCode2, pDecExtStatus->extendedErrorCode3);

EXIT:
    return (eError);
}

