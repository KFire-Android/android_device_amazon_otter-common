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

#include <omx_video_decoder_internal.h>
#define codec_get_status_workaround
#define MAX_H264D_REF_FRAMES (16)

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_Init_DerToBase_Fields()
 *                     This method initializes the DerToBase Fields. This
 *                     method is called during init of common decoder
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_Init_DerToBase_Fields(OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt)
{
    OMX_ERRORTYPE    eError = OMX_ErrorNone;
    OMX_U32          i = 0;

    /* Allocate Memory for Component Name */
    pVideoDecoderComPvt->tDerToBase.cComponentName = (OMX_STRING )
                                                     TIMM_OSAL_Malloc(sizeof(OMX_U8) * OMX_MAX_STRINGNAME_SIZE,
                                                                      TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(pVideoDecoderComPvt->tDerToBase.cComponentName != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memcpy(pVideoDecoderComPvt->tDerToBase.cComponentName,
                     pVideoDecoderComPvt->strComponentName,
                     strlen(pVideoDecoderComPvt->strComponentName));


    pVideoDecoderComPvt->tDerToBase.hDefaultHeap = NULL;

    /* Initialize Video Port parameters */
    OMX_BASE_MALLOC_STRUCT(pVideoDecoderComPvt->tDerToBase.pVideoPortParams,
                           OMX_PORT_PARAM_TYPE,
                           pVideoDecoderComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_INIT_STRUCT_PTR(pVideoDecoderComPvt->tDerToBase.pVideoPortParams,
                             OMX_PORT_PARAM_TYPE);
    pVideoDecoderComPvt->tDerToBase.pVideoPortParams->nPorts
        = OMX_VIDEODECODER_NUM_OF_PORTS;
    pVideoDecoderComPvt->tDerToBase.pVideoPortParams->nStartPortNumber
        = OMX_VIDEODECODER_DEFAULT_START_PORT_NUM;


    /*Setting number of ports - this has to be set before
     * calling SetDefaultProperties*/
    pVideoDecoderComPvt->tDerToBase.nNumPorts = OMX_VIDEODECODER_NUM_OF_PORTS;
    pVideoDecoderComPvt->tDerToBase.nMinStartPortIndex
        =  OMX_VIDEODECODER_DEFAULT_START_PORT_NUM;

    /*Allocating memory for port properties before
     * calling SetDefaultProperties*/
    pVideoDecoderComPvt->tDerToBase.pPortProperties
        = (OMX_BASE_PORT_PROPERTIES * *)
          TIMM_OSAL_Malloc(sizeof(OMX_BASE_PORT_PROPERTIES *) *
                           pVideoDecoderComPvt->tDerToBase.nNumPorts,
                           TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(pVideoDecoderComPvt->tDerToBase.pPortProperties != NULL,
                    OMX_ErrorInsufficientResources);

    /* Allocate Memory for Port Properties */
    for( i = 0; i < (pVideoDecoderComPvt->tDerToBase.nNumPorts); i++ ) {
        pVideoDecoderComPvt->tDerToBase.pPortProperties[i] =
            (OMX_BASE_PORT_PROPERTIES *)
            TIMM_OSAL_Malloc(sizeof(OMX_BASE_PORT_PROPERTIES),
                             TIMM_OSAL_TRUE, 0,
                             TIMMOSAL_MEM_SEGMENT_EXT);

        OMX_BASE_ASSERT(pVideoDecoderComPvt->tDerToBase.pPortProperties[i] != NULL,
                        OMX_ErrorInsufficientResources);
    }

    /* Set the ContextType whether passive/Active depending on requirement */
    pVideoDecoderComPvt->tDerToBase.ctxType = OMX_VIDDEC_DEFAULT_CONTEXT_TYPE;
    /* Incase of Active Context, set the Stacksize and GroupPriority */
    if( pVideoDecoderComPvt->tDerToBase.ctxType == OMX_BASE_CONTEXT_ACTIVE ) {
        TIMM_OSAL_Memcpy(pVideoDecoderComPvt->tDerToBase.cTaskName,
                         "OMX_Video_Decoder", strlen(" OMX_Video_Decoder"));
        pVideoDecoderComPvt->tDerToBase.nStackSize
            = OMX_VIDDEC_TASK_STACKSIZE;
        pVideoDecoderComPvt->tDerToBase.nPrioirty
            = OMX_VIDDEC_TASK_PRIORITY;
    }
    pVideoDecoderComPvt->tDerToBase.nGroupID         = OMX_VIDDEC_GROUPID;
    pVideoDecoderComPvt->tDerToBase.nGroupPriority
        = OMX_VIDDEC_GROUPPRIORITY;

    pVideoDecoderComPvt->tDerToBase.nComponentVersion.s.nVersionMajor
        = OMX_VIDDEC_COMP_VERSION_MAJOR;
    pVideoDecoderComPvt->tDerToBase.nComponentVersion.s.nVersionMinor
        = OMX_VIDDEC_COMP_VERSION_MINOR;
    pVideoDecoderComPvt->tDerToBase.nComponentVersion.s.nRevision
        = OMX_VIDDEC_COMP_VERSION_REVISION;
    pVideoDecoderComPvt->tDerToBase.nComponentVersion.s.nStep
        = OMX_VIDDEC_COMP_VERSION_STEP;

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_Set_DerToBase_Fields()
 *                     This method overrides the default values provided to
 *                     the fields of DerToBase structure during initialization
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_Set_DerToBase_Fields(OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_BASE_PORT_PROPERTIES   *inputPortProperties = NULL;
    OMX_BASE_PORT_PROPERTIES   *outputPortProperties = NULL;
    OMX_U32                     inPort, outPort;


    pVideoDecoderComPvt->tDerToBase.bNotifyForAnyPort = OMX_FALSE;
    inPort  = (OMX_U32) OMX_VIDEODECODER_INPUT_PORT;
    outPort = (OMX_U32) OMX_VIDEODECODER_OUTPUT_PORT;

    inputPortProperties
        = pVideoDecoderComPvt->tDerToBase.pPortProperties[inPort];
    outputPortProperties
        = pVideoDecoderComPvt->tDerToBase.pPortProperties[outPort];

#ifdef VIDEODECODER_TILERTEST
    /* update for input port */
    inputPortProperties->eBufMemoryType = OMX_BASE_BUFFER_MEMORY_1D_CONTIGUOUS;
    inputPortProperties->nNumComponentBuffers = 1;
    inputPortProperties->pBufParams = NULL;

    /* Update for output port */
    outputPortProperties->eBufMemoryType = OMX_BASE_BUFFER_MEMORY_2D;
    outputPortProperties->nNumComponentBuffers = 2;

    // Allocate memory for OMX_BASE_2D_MEM_PARAMS
    outputPortProperties->pBufParams = (OMX_BASE_2D_MEM_PARAMS *) TIMM_OSAL_Malloc( \
        ((sizeof(OMX_BASE_2D_MEM_PARAMS) *
          outputPortProperties->nNumComponentBuffers)),
        TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
#endif

    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_Init_PortDefs()
 *                     This method initializes the input and output port
 *                     defintions
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
void OMX_TI_VIDEODECODER_Init_PortDefs(OMX_HANDLETYPE hComponent,
                                       OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt)
{
    OMX_PARAM_PORTDEFINITIONTYPE   *inPortDefs
        = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT];
    OMX_PARAM_PORTDEFINITIONTYPE   *outPortDefs
        = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];

    /* set the default/Actual values of an Input port */
    inPortDefs->nSize              = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    inPortDefs->nVersion = pVideoDecoderComPvt->tDerToBase.nComponentVersion;
    inPortDefs->bEnabled           = OMX_TRUE;
    inPortDefs->bPopulated         = OMX_FALSE;
    inPortDefs->eDir               = OMX_DirInput;
    inPortDefs->nPortIndex         = OMX_VIDEODECODER_INPUT_PORT;
    if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_ENTIREFRAME) {
        inPortDefs->nBufferCountMin    = OMX_VIDDEC_MIN_IN_BUF_COUNT;
        inPortDefs->nBufferCountActual = OMX_VIDDEC_DEFAULT_IN_BUF_COUNT;
        inPortDefs->nBufferSize
            = Calc_InbufSize(OMX_VIDDEC_DEFAULT_FRAME_WIDTH,
                             OMX_VIDDEC_DEFAULT_FRAME_HEIGHT);
    }
    else if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_SLICEMODE) {
        inPortDefs->nBufferCountMin    = OMX_VIDDEC_DATASYNC_MIN_IN_BUF_COUNT;
        inPortDefs->nBufferCountActual = OMX_VIDDEC_DATASYNC_DEFAULT_IN_BUF_COUNT;
        inPortDefs->nBufferSize
            = (OMX_VIDDEC_DEFAULT_FRAME_WIDTH * OMX_VIDDEC_DEFAULT_FRAME_HEIGHT) / OMX_VIDDEC_DATASYNC_DEFAULT_IN_BUF_COUNT;
    }
    inPortDefs->eDomain              = OMX_PortDomainVideo;
    inPortDefs->bBuffersContiguous = OMX_TRUE;
    inPortDefs->nBufferAlignment
        = OMX_VIDDEC_DEFAULT_1D_INPUT_BUFFER_ALIGNMENT;
    inPortDefs->format.video.cMIMEType= NULL;
    inPortDefs->format.video.pNativeRender = NULL;
    inPortDefs->format.video.nFrameWidth = OMX_VIDDEC_DEFAULT_FRAME_WIDTH;
    inPortDefs->format.video.nFrameHeight = OMX_VIDDEC_DEFAULT_FRAME_HEIGHT;
    inPortDefs->format.video.nStride = 0;
    inPortDefs->format.video.nSliceHeight = 0;
    inPortDefs->format.video.nBitrate = 0;
    inPortDefs->format.video.xFramerate = OMX_VIDEODECODER_DEFAULT_FRAMERATE << 16;
    inPortDefs->format.video.bFlagErrorConcealment = OMX_TRUE;
    inPortDefs->format.video.eCompressionFormat
        = pVideoDecoderComPvt->eCompressionFormat;
    inPortDefs->format.video.eColorFormat
        = OMX_COLOR_FormatUnused;


    /* set the default/Actual values of an output port */
    outPortDefs->nSize              = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    outPortDefs->nVersion = pVideoDecoderComPvt->tDerToBase.nComponentVersion;
    outPortDefs->bEnabled           = OMX_TRUE;
    outPortDefs->bPopulated         = OMX_FALSE;
    outPortDefs->eDir               = OMX_DirOutput;
    outPortDefs->nPortIndex         = OMX_VIDEODECODER_OUTPUT_PORT;
    outPortDefs->nBufferCountMin
        = pVideoDecoderComPvt->fpCalc_OubuffDetails(
        hComponent,
        OMX_VIDDEC_DEFAULT_FRAME_WIDTH,
        OMX_VIDDEC_DEFAULT_FRAME_HEIGHT).nBufferCountMin;
    outPortDefs->nBufferCountActual
        = pVideoDecoderComPvt->fpCalc_OubuffDetails(
        hComponent,
        OMX_VIDDEC_DEFAULT_FRAME_WIDTH,
        OMX_VIDDEC_DEFAULT_FRAME_HEIGHT).nBufferCountActual;
    outPortDefs->nBufferSize
        = pVideoDecoderComPvt->fpCalc_OubuffDetails(
        hComponent,
        OMX_VIDDEC_DEFAULT_FRAME_WIDTH,
        OMX_VIDDEC_DEFAULT_FRAME_HEIGHT).nBufferSize;
    outPortDefs->eDomain              = OMX_PortDomainVideo;
    outPortDefs->bBuffersContiguous = OMX_TRUE;
    outPortDefs->nBufferAlignment
        = pVideoDecoderComPvt->fpCalc_OubuffDetails(
        hComponent,
        OMX_VIDDEC_DEFAULT_FRAME_WIDTH,
        OMX_VIDDEC_DEFAULT_FRAME_HEIGHT).n1DBufferAlignment;
    outPortDefs->format.video.cMIMEType = NULL;
    outPortDefs->format.video.pNativeRender = NULL;
    outPortDefs->format.video.nFrameWidth = OMX_VIDDEC_DEFAULT_FRAME_WIDTH;
    outPortDefs->format.video.nFrameHeight
        = OMX_VIDDEC_DEFAULT_FRAME_HEIGHT;
    outPortDefs->format.video.nStride =  OMX_VIDDEC_TILER_STRIDE;
    outPortDefs->format.video.nSliceHeight
        = OMX_VIDDEC_DEFAULT_FRAME_HEIGHT;
    outPortDefs->format.video.nBitrate = 0;
    outPortDefs->format.video.xFramerate = OMX_VIDEODECODER_DEFAULT_FRAMERATE << 16;
    outPortDefs->format.video.bFlagErrorConcealment = OMX_TRUE;
    outPortDefs->format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
    outPortDefs->format.video.eColorFormat
        = OMX_COLOR_FormatYUV420PackedSemiPlanar;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_Init_PortParams()
 *                     This method initializes the Port Params
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
void OMX_TI_VIDEODECODER_Init_PortParams(OMX_HANDLETYPE hComponent,
                                         OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt)
{
    OMX_VIDEO_PARAM_PORTFORMATTYPE   *tInPortVideoParam
        = &(pVideoDecoderComPvt->tVideoParams[OMX_VIDEODECODER_INPUT_PORT]);
    OMX_VIDEO_PARAM_PORTFORMATTYPE   *tOutPortVideoParam
        = &(pVideoDecoderComPvt->tVideoParams[OMX_VIDEODECODER_OUTPUT_PORT]);

    // Initialize Input Video Port Param
    tInPortVideoParam->nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    tInPortVideoParam->nVersion = pVideoDecoderComPvt->tDerToBase.nComponentVersion;
    tInPortVideoParam->nPortIndex = OMX_VIDEODECODER_INPUT_PORT;
    tInPortVideoParam->nIndex = 0;
    tInPortVideoParam->eCompressionFormat = pVideoDecoderComPvt->eCompressionFormat;
    tInPortVideoParam->eColorFormat = OMX_COLOR_FormatUnused;
    tInPortVideoParam->xFramerate = OMX_VIDEODECODER_DEFAULT_FRAMERATE;

    // Initialize Output Video Port Param
    tOutPortVideoParam->nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    tOutPortVideoParam->nVersion = pVideoDecoderComPvt->tDerToBase.nComponentVersion;
    tOutPortVideoParam->nPortIndex = OMX_VIDEODECODER_OUTPUT_PORT;
    tOutPortVideoParam->nIndex = 1;
    tOutPortVideoParam->eCompressionFormat = OMX_VIDEO_CodingUnused;
    tOutPortVideoParam->eColorFormat = OMX_COLOR_FormatYUV420PackedSemiPlanar;
    tOutPortVideoParam->xFramerate = OMX_VIDEODECODER_DEFAULT_FRAMERATE;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_Set_2D_BuffParams()
 *                     This method initializes the Port Params
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
void OMX_TI_VIDEODECODER_Set_2D_BuffParams(OMX_HANDLETYPE hComponent,
                                           OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt)
{
    OMX_U32                outPort = (OMX_U32)OMX_VIDEODECODER_OUTPUT_PORT;
    OMX_U32                inPort = (OMX_U32)OMX_VIDEODECODER_INPUT_PORT;
    OMX_CONFIG_RECTTYPE   *p2DOutBufAllocParam
        = &(pVideoDecoderComPvt->t2DBufferAllocParams[outPort]);
    OMX_CONFIG_RECTTYPE   *p2DInBufAllocParam
        = &(pVideoDecoderComPvt->t2DBufferAllocParams[inPort]);
    BUFF_PARAMS                 outBuffParams;
    OMX_BASE_PORT_PROPERTIES   *pOutPortProperty
        = pVideoDecoderComPvt->tDerToBase.pPortProperties[outPort];
    OMX_U32                         nFrameWidth, nFrameHeight;
    OMX_PARAM_PORTDEFINITIONTYPE   *pOutputPortDef = NULL;

    /* To be used by the client in UseBuffer scenerio through Get Parameter.*/
    p2DOutBufAllocParam->nSize = sizeof(OMX_CONFIG_RECTTYPE);
    p2DOutBufAllocParam->nVersion
        = pVideoDecoderComPvt->tDerToBase.nComponentVersion;
    p2DOutBufAllocParam->nPortIndex = OMX_VIDEODECODER_OUTPUT_PORT;
    pOutputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];


    nFrameWidth
        = pVideoDecoderComPvt->pPortdefs[inPort]->format.video.nFrameWidth;
    nFrameHeight
        = pVideoDecoderComPvt->pPortdefs[inPort]->format.video.nFrameHeight;
    if( nFrameWidth & 0x0F ) {
        nFrameWidth = nFrameWidth + 16 - (nFrameWidth & 0x0F);
    }
    if( nFrameHeight & 0x1F ) {
        nFrameHeight = nFrameHeight + 32 - (nFrameHeight & 0x1F);
    }
    outBuffParams
        = pVideoDecoderComPvt->fpCalc_OubuffDetails(hComponent,
                                                    nFrameWidth, nFrameHeight);
    p2DOutBufAllocParam->nWidth = outBuffParams.nPaddedWidth;
    p2DOutBufAllocParam->nHeight = outBuffParams.nPaddedHeight;
    p2DOutBufAllocParam->nLeft = outBuffParams.n2DBufferXAlignment;
    p2DOutBufAllocParam->nTop = outBuffParams.n2DBufferYAlignment;


    p2DInBufAllocParam->nSize = sizeof(OMX_CONFIG_RECTTYPE);
    p2DInBufAllocParam->nVersion
        = pVideoDecoderComPvt->tDerToBase.nComponentVersion;
    p2DInBufAllocParam->nPortIndex = OMX_VIDEODECODER_INPUT_PORT;
    p2DInBufAllocParam->nWidth
        = pVideoDecoderComPvt->pPortdefs[inPort]->nBufferSize;
    p2DInBufAllocParam->nHeight = 1;     //On input port only 1D buffers supported.
    p2DInBufAllocParam->nLeft
        = pVideoDecoderComPvt->pPortdefs[inPort]->nBufferAlignment;
    p2DInBufAllocParam->nTop = 1;

#ifdef VIDEODECODER_TILERTEST
    if( pOutputPortDef->format.video.nStride == OMX_VIDDEC_TILER_STRIDE ) {
        if((OMX_BASE_2D_MEM_PARAMS *)pOutPortProperty->pBufParams != NULL ) {
            ((OMX_BASE_2D_MEM_PARAMS *)pOutPortProperty->pBufParams)[0].nHeight
                = p2DOutBufAllocParam->nHeight;
            ((OMX_BASE_2D_MEM_PARAMS *)pOutPortProperty->pBufParams)[0].nWidth
                = p2DOutBufAllocParam->nWidth;
            ((OMX_BASE_2D_MEM_PARAMS *)pOutPortProperty->pBufParams)[0].eAccessMode
                = OMX_BASE_MEMORY_ACCESS_8BIT;
            ((OMX_BASE_2D_MEM_PARAMS *)pOutPortProperty->pBufParams)[1].nHeight
                = (p2DOutBufAllocParam->nHeight) / 2;
            ((OMX_BASE_2D_MEM_PARAMS *)pOutPortProperty->pBufParams)[1].nWidth
                = (p2DOutBufAllocParam->nWidth) / 2;
            ((OMX_BASE_2D_MEM_PARAMS *)pOutPortProperty->pBufParams)[1].eAccessMode
                = OMX_BASE_MEMORY_ACCESS_16BIT;
        }
    }
#endif
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_Set_UseBufferDescParams()
 *                     This method initializes the UseBuffer Descriptor params
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
void OMX_TI_VIDEODECODER_Set_UseBufferDescParams(OMX_HANDLETYPE hComponent,
                                                 OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt)
{
    OMX_U32                             outPort = (OMX_U32)OMX_VIDEODECODER_OUTPUT_PORT;
    OMX_U32                             inPort = (OMX_U32)OMX_VIDEODECODER_INPUT_PORT;
    OMX_TI_PARAM_USEBUFFERDESCRIPTOR   *pOutUseBufDesc
        = &(pVideoDecoderComPvt->tUseBufDesc[outPort]);
    OMX_TI_PARAM_USEBUFFERDESCRIPTOR   *pInUseBufDesc
        = &(pVideoDecoderComPvt->tUseBufDesc[inPort]);


    /* To be used by the client in UseBuffer scenerio through Get Parameter.*/
    OMX_BASE_INIT_STRUCT_PTR(&(pVideoDecoderComPvt->tUseBufDesc[inPort]),
                             OMX_TI_PARAM_USEBUFFERDESCRIPTOR);
    OMX_BASE_INIT_STRUCT_PTR(&(pVideoDecoderComPvt->tUseBufDesc[outPort]),
                             OMX_TI_PARAM_USEBUFFERDESCRIPTOR);
    pInUseBufDesc->bEnabled = pOutUseBufDesc->bEnabled = OMX_FALSE;
    pInUseBufDesc->eBufferType = pOutUseBufDesc->eBufferType = OMX_TI_BufferTypeDefault;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS()
 *                     This method initializes the Port Params
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(OMX_HANDLETYPE hComponent,
                                                       OMX_VIDEODECODER_CUSTOMBUFFERHEADER *pLastOutBufHeader,
                                                       OMX_BUFFERHEADERTYPE *pInBufHeader)
{
    OMX_ERRORTYPE                          eError = OMX_ErrorNone, eRMError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                     *pHandle = NULL;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pOutBufHeader = NULL;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pDupBufHeader = NULL;
    OMX_VIDEODECODER_PVTTYPE              *pVideoDecoderComPvt = NULL;
    OMX_U32                                i = 0;
    OMX_U32                                nStride;
    IVIDDEC3_OutArgs                      *pDecOutArgs = NULL;
    OMX_U32                                outPort = (OMX_U32)OMX_VIDEODECODER_OUTPUT_PORT;
    OMX_U32                                nLumaFilledLen=0, nChromaFilledLen=0;
    OMX_CONFIG_RECTTYPE                   *p2DOutBufAllocParam = NULL;
    XDM_Rect                               activeFrameRegion[2];
    OMX_U32                                tLError_Status_RMInteractions = 0;
    OMX_PTR                                pClientMetaData = NULL, pCodecMetaData = NULL;
    OMX_METADATA                          *pOMXmetaFrame1 = NULL, *pOMXmetaFrame2 = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE          *pOutputPortDef = NULL;
    XDAS_Int32                             status;
    TIMM_OSAL_U32                          nActualSize;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    pDecOutArgs
        = pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs;
    p2DOutBufAllocParam
        = &(pVideoDecoderComPvt->t2DBufferAllocParams[outPort]);

    pOutputPortDef =
        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
    if( pVideoDecoderComPvt->nFrameCounter > 0 ) {
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "\n Before Control Call for flush.\n");
        /* Call codec flush and call process call until error */
        eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_FLUSH,
                                              pVideoDecoderComPvt->pDecStructures.pDecDynParams, pVideoDecoderComPvt->pDecStructures.pDecStatus,
                                              hComponent, &status);
        if( status != VIDDEC3_EOK ) {
            OMX_BASE_Error("VIDDEC3_control XDM_FLUSH failed ....! \n");
            eError = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "\n After Control Call for flush.\n");

        do {
            TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\n Before Process Call after FLUSH.\n");
            status =  VIDDEC3_process
                         (pVideoDecoderComPvt->pDecHandle,
                         (XDM2_BufDesc *)&pVideoDecoderComPvt->tInBufDesc,
                         (XDM2_BufDesc *)&pVideoDecoderComPvt->tOutBufDesc,
                         (VIDDEC3_InArgs *)pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs,
                         (VIDDEC3_OutArgs *)pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs
                         );
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\n After Process Call after FLUSH. Return status is : %d\n", status);

    #ifdef H264VD_CODEC_BUG
            if( pDecOutArgs->outputID[0] != 0x0 ) {
                pDecOutArgs->outputID[0]
                    = pDecOutArgs->outputID[0] | 0x80000000;
            }
            i=0;

            while( pDecOutArgs->freeBufID[i] != 0x0 ) {
                pDecOutArgs->freeBufID[i]
                    = pDecOutArgs->freeBufID[i] | 0x80000000;
                i++;
            }

            i=0;
    #endif
            if( pOutBufHeader != NULL ) {
                if( pLastOutBufHeader != NULL ) {
                    pVideoDecoderComPvt->fpDioSend(hComponent,
                                                   OMX_VIDEODECODER_OUTPUT_PORT,
                                                   pLastOutBufHeader);
                    TIMM_OSAL_Free(pLastOutBufHeader);
                    pLastOutBufHeader = TIMM_OSAL_NULL;
                }
                if( status == XDM_EFAIL && pLastOutBufHeader == NULL && (pInBufHeader != NULL || pVideoDecoderComPvt->bIsFlushRequired) ) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,"OutBuf EOS\n");
                    pOutBufHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                }
                if( pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp == OMX_TRUE ) {
                    TIMM_OSAL_ReadFromPipe(pVideoDecoderComPvt->pTimeStampStoragePipe,
                                           &(pOutBufHeader->nTimeStamp),
                                           sizeof(OMX_TICKS),
                                           &(nActualSize),
                                           TIMM_OSAL_NO_SUSPEND);
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Time stamp recived at o/p buffer = %d",
                                       pOutBufHeader->nTimeStamp);
                }
                pVideoDecoderComPvt->fpDioSend(hComponent,
                                               OMX_VIDEODECODER_OUTPUT_PORT,
                                               pOutBufHeader);
                TIMM_OSAL_Free(pOutBufHeader);
                pOutBufHeader = TIMM_OSAL_NULL;
            }
            if( status != XDM_EFAIL ) {
                /* Send the buffers out */
                if( pDecOutArgs->outputID[0] != 0 ) {
                    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "\n OutputId is : %x", pDecOutArgs->outputID[0]);
                    if( pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->format.video.nStride
                        >= p2DOutBufAllocParam->nWidth ) {
                        nStride = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->format.video.nStride;
                    } else {
                        nStride = p2DOutBufAllocParam->nWidth;
                    }
                    pOutBufHeader = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)pDecOutArgs->outputID[0];
                    pOutBufHeader->nMetaDataUsageBitField = 0;

                    _OMX_VIDEODECODER_PULL_METADATA_BUFFER(hComponent, pDecOutArgs->outputID[0], &pOMXmetaFrame1, &pOMXmetaFrame2);
                    if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                        if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pSEIinfo) != NULL ) {
                            if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->parsed_flag == 1 ) {
                                if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->stereo_video_info.parsed_flag == 1 ) {
                                    pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2004FRAME1BIT;
                                } else if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->frame_packing.parsed_flag == 1 ) {

                                    pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2010FRAME1BIT;
                                }
                            }
                        }
                        if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pSEIinfo) != NULL ) {
                            if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->parsed_flag == 1 ) {
                                if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->stereo_video_info.parsed_flag == 1 ) {

                                    pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2004FRAME2BIT;
                                } else if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->frame_packing.parsed_flag == 1 ) {
                                    pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2010FRAME2BIT;
                                }
                            }
                        }
                    }


                    if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                        if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pMBinfo) != NULL ) {
                            (pOutBufHeader->tMbInfoFrame1).nSize = pVideoDecoderComPvt->nMBinfoSize;
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_MBINFOFRAME1BIT;
                        }
                        if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pMBinfo) != NULL ) {
                            (pOutBufHeader->tMbInfoFrame2).nSize = pVideoDecoderComPvt->nMBinfoSize;
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_MBINFOFRAME2BIT;
                        }

                    }
                    if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                        if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pSEIinfo) != NULL ) {
                            (pOutBufHeader->tSeiInfoFrame1).nSize = sizeof(IH264VDEC_SeiMessages);
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFOFRAME1BIT;
                        }
                        if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pSEIinfo) != NULL ) {
                            (pOutBufHeader->tSeiInfoFrame2).nSize = sizeof(IH264VDEC_SeiMessages);
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFOFRAME2BIT;
                        }
                    }
                    if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
                        if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pVUIinfo) != NULL ) {
                            (pOutBufHeader->tVuiInfoFrame1).nSize = sizeof(IH264VDEC_VuiParams);
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_VUIINFOFRAME1BIT;
                        }
                        if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pVUIinfo) != NULL ) {
                            (pOutBufHeader->tVuiInfoFrame2).nSize = sizeof(IH264VDEC_VuiParams);
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_VUIINFOFRAME2BIT;
                        }
                    }

                    if( pVideoDecoderComPvt->eCompressionFormat == OMX_VIDEO_CodingWMV ) {
                        if( pDecOutArgs->displayBufs.bufDesc[0].rangeMappingLuma != 0
                            || pDecOutArgs->displayBufs.bufDesc[0].rangeMappingChroma != 0 ) {
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_RANGEMAPPINGBIT;
                        }
                        if( pDecOutArgs->displayBufs.bufDesc[0].scalingWidth != pOutputPortDef->format.video.nFrameWidth
                            || pDecOutArgs->displayBufs.bufDesc[0].scalingHeight != pOutputPortDef->format.video.nFrameHeight ) {
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_RESCALINGBIT;
                        }
                    }

                    if( pOMXmetaFrame1 != NULL ) {
                        pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_CODECEXTERROR1BIT;
                    }
                    if( pOMXmetaFrame2 != NULL ) {
                        pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_CODECEXTERROR2BIT;
                    }

                    if( (pInBufHeader != NULL || pVideoDecoderComPvt->bIsFlushRequired) || pVideoDecoderComPvt->nOutPortReconfigRequired == 1 ) {

                        activeFrameRegion[0]
                            = pDecOutArgs->displayBufs.bufDesc[0].activeFrameRegion;
                        activeFrameRegion[1].bottomRight.y
                            = activeFrameRegion[0].bottomRight.y / 2;
                        activeFrameRegion[1].bottomRight.x
                            = activeFrameRegion[0].bottomRight.x;

                        pOutBufHeader->nOffset
                            = (activeFrameRegion[0].topLeft.y * nStride) +
                              activeFrameRegion[0].topLeft.x;

                        nLumaFilledLen
                            = (nStride * pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight)
                              - (nStride * activeFrameRegion[0].topLeft.y)
                              - activeFrameRegion[0].topLeft.x;

                        nChromaFilledLen
                            = (nStride * activeFrameRegion[1].bottomRight.y)
                              - nStride + activeFrameRegion[1].bottomRight.x;

                        pOutBufHeader->nFilledLen =
                            nLumaFilledLen + nChromaFilledLen;
                    } else {
                        pOutBufHeader->nOffset = 0;
                        pOutBufHeader->nFilledLen = 0;

                    }

                    pVideoDecoderComPvt->fpDioUtil(hComponent, OMX_VIDEODECODER_OUTPUT_PORT, OMX_BASE_DIO_UTIL_UpdatePacketExtraDataPointer, pOutBufHeader);


                    if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                        if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pSEIinfo) != NULL ) {
                            if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->parsed_flag == 1 ) {
                                if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->stereo_video_info.parsed_flag == 1 ) {
                                    pClientMetaData = (OMX_PTR)(&(pOutBufHeader->tSEIinfo2004Frame1));
                                    pCodecMetaData = (OMX_PTR)(&(((IH264VDEC_SeiMessages *)(pOMXmetaFrame1->pSEIinfo))->stereo_video_info));
                                    pCodecMetaData = (OMX_PTR)((OMX_U32) pCodecMetaData + sizeof(XDAS_UInt32));

                                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(OMX_TI_STEREODECINFO));
                                    pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2004FRAME1BIT;
                                } else if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->frame_packing.parsed_flag == 1 ) {
                                    pClientMetaData = (OMX_PTR)(&(pOutBufHeader->tSEIinfo2010Frame1));
                                    pCodecMetaData = (OMX_PTR)(&(((IH264VDEC_SeiMessages *)(pOMXmetaFrame1->pSEIinfo))->frame_packing));
                                    pCodecMetaData = (OMX_PTR)((OMX_U32) pCodecMetaData + sizeof(XDAS_UInt32));

                                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(OMX_TI_FRAMEPACKINGDECINFO));
                                    pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2010FRAME1BIT;
                                }
                            }
                        }
                        if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pSEIinfo) != NULL ) {
                            if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->parsed_flag == 1 ) {
                                if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->stereo_video_info.parsed_flag == 1 ) {
                                    pClientMetaData = (OMX_PTR)(&(pOutBufHeader->tSEIinfo2004Frame2));
                                    pCodecMetaData = (OMX_PTR)(&(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->stereo_video_info));
                                    pCodecMetaData = (OMX_PTR)((OMX_U32) pCodecMetaData + sizeof(XDAS_UInt32));

                                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(OMX_TI_STEREODECINFO));
                                    pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2004FRAME2BIT;
                                } else if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->frame_packing.parsed_flag == 1 ) {
                                    pClientMetaData = (OMX_PTR)(&(pOutBufHeader->tSEIinfo2010Frame2));
                                    pCodecMetaData = (OMX_PTR)(&(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->frame_packing));
                                    pCodecMetaData = (OMX_PTR)((OMX_U32) pCodecMetaData + sizeof(XDAS_UInt32));

                                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(OMX_TI_FRAMEPACKINGDECINFO));
                                    pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2010FRAME2BIT;
                                }
                            }
                        }
                    }

                    if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                        if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pMBinfo) != NULL ) {
                            pClientMetaData = (pOutBufHeader->tMbInfoFrame1).pMBinfo;
                            pCodecMetaData = pOMXmetaFrame1->pMBinfo;
                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, pVideoDecoderComPvt->nMBinfoSize);
                            (pOutBufHeader->tMbInfoFrame1).nSize = pVideoDecoderComPvt->nMBinfoSize;
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_MBINFOFRAME1BIT;
                        }
                        if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pMBinfo) != NULL ) {
                            pClientMetaData = (pOutBufHeader->tMbInfoFrame2).pMBinfo;
                            pCodecMetaData = pOMXmetaFrame2->pMBinfo;

                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, pVideoDecoderComPvt->nMBinfoSize);
                            (pOutBufHeader->tMbInfoFrame2).nSize = pVideoDecoderComPvt->nMBinfoSize;
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_MBINFOFRAME2BIT;
                        }
                    }

                    if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                        if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pSEIinfo) != NULL ) {
                            pClientMetaData = (pOutBufHeader->tSeiInfoFrame1).pSEIinfo;
                            pCodecMetaData = pOMXmetaFrame1->pSEIinfo;
                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(IH264VDEC_SeiMessages));
                            (pOutBufHeader->tSeiInfoFrame1).nSize = sizeof(IH264VDEC_SeiMessages);
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFOFRAME1BIT;
                        }
                        if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pSEIinfo) != NULL ) {
                            pClientMetaData = (pOutBufHeader->tSeiInfoFrame2).pSEIinfo;
                            pCodecMetaData = pOMXmetaFrame2->pSEIinfo;
                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(IH264VDEC_SeiMessages));
                            (pOutBufHeader->tSeiInfoFrame2).nSize = sizeof(IH264VDEC_SeiMessages);
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFOFRAME2BIT;
                        }
                    }
                    if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
                        if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pVUIinfo) != NULL ) {
                            pClientMetaData = (pOutBufHeader->tVuiInfoFrame1).pVUIinfo;
                            pCodecMetaData = pOMXmetaFrame1->pVUIinfo;
                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(IH264VDEC_VuiParams));
                            (pOutBufHeader->tVuiInfoFrame1).nSize = sizeof(IH264VDEC_VuiParams);
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_VUIINFOFRAME1BIT;
                        }
                        if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pVUIinfo) != NULL ) {
                            pClientMetaData = (pOutBufHeader->tVuiInfoFrame2).pVUIinfo;
                            pCodecMetaData = pOMXmetaFrame2->pVUIinfo;
                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(IH264VDEC_VuiParams));
                            (pOutBufHeader->tVuiInfoFrame2).nSize = sizeof(IH264VDEC_VuiParams);
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_VUIINFOFRAME2BIT;
                        }
                    }


                    if( pOMXmetaFrame1 != NULL ) {

                        pOutBufHeader->nCodecExtendedErrorFrame1 = pOMXmetaFrame1->CodecExtenderError;
                        pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_CODECEXTERROR1BIT;
                    }
                    if( pOMXmetaFrame2 != NULL ) {

                        pOutBufHeader->nCodecExtendedErrorFrame2 = pOMXmetaFrame2->CodecExtenderError;
                        pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_CODECEXTERROR2BIT;
                    }


                    _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pOMXmetaFrame1);

                    _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pOMXmetaFrame2);

                    if( pVideoDecoderComPvt->eCompressionFormat == OMX_VIDEO_CodingWMV ) {
                        if( pDecOutArgs->displayBufs.bufDesc[0].rangeMappingLuma != 0
                            || pDecOutArgs->displayBufs.bufDesc[0].rangeMappingChroma != 0 ) {
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_RANGEMAPPINGBIT;
                            pOutBufHeader->tRangeMapping.nRangeMappingLuma = pDecOutArgs->displayBufs.bufDesc[0].rangeMappingLuma;
                            pOutBufHeader->tRangeMapping.nRangeMappingChroma = pDecOutArgs->displayBufs.bufDesc[0].rangeMappingChroma;
                        }
                        if( pDecOutArgs->displayBufs.bufDesc[0].scalingWidth != pOutputPortDef->format.video.nFrameWidth
                            || pDecOutArgs->displayBufs.bufDesc[0].scalingHeight != pOutputPortDef->format.video.nFrameHeight ) {
                            pOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_RESCALINGBIT;
                            pOutBufHeader->tRescalingMatrix.nScaledHeight = pDecOutArgs->displayBufs.bufDesc[0].scalingHeight;
                            pOutBufHeader->tRescalingMatrix.nScaledWidth = pDecOutArgs->displayBufs.bufDesc[0].scalingWidth;
                            ;
                        }
                    }
                    if( (pInBufHeader != NULL || pVideoDecoderComPvt->bIsFlushRequired) || pVideoDecoderComPvt->nOutPortReconfigRequired == 1 ) {

                        activeFrameRegion[0]
                            = pDecOutArgs->displayBufs.bufDesc[0].activeFrameRegion;
                        activeFrameRegion[1].bottomRight.y
                            = activeFrameRegion[0].bottomRight.y / 2;
                        activeFrameRegion[1].bottomRight.x
                            = activeFrameRegion[0].bottomRight.x;

                        pOutBufHeader->nOffset
                            = (activeFrameRegion[0].topLeft.y * nStride) +
                              activeFrameRegion[0].topLeft.x;

                        if((pVideoDecoderComPvt->tCropDimension.nTop != activeFrameRegion[0].topLeft.y
                            || pVideoDecoderComPvt->tCropDimension.nLeft != activeFrameRegion[0].topLeft.x)
                           || (pVideoDecoderComPvt->tCropDimension.nWidth != activeFrameRegion[0].bottomRight.x - activeFrameRegion[0].topLeft.x
                               || pVideoDecoderComPvt->tCropDimension.nHeight != activeFrameRegion[0].bottomRight.y - activeFrameRegion[0].topLeft.y)) {
                            pVideoDecoderComPvt->tCropDimension.nTop = activeFrameRegion[0].topLeft.y;
                            pVideoDecoderComPvt->tCropDimension.nLeft = activeFrameRegion[0].topLeft.x;
                            pVideoDecoderComPvt->tCropDimension.nWidth = activeFrameRegion[0].bottomRight.x - activeFrameRegion[0].topLeft.x;
                            pVideoDecoderComPvt->tCropDimension.nHeight = activeFrameRegion[0].bottomRight.y - activeFrameRegion[0].topLeft.y;
                            if( pVideoDecoderComPvt->bUsePortReconfigForCrop == OMX_TRUE ) {
                                eError = pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                                                  OMX_EventPortSettingsChanged, OMX_VIDEODECODER_OUTPUT_PORT, OMX_IndexConfigCommonOutputCrop, NULL);
                                if( eError != OMX_ErrorNone ) {
                                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                                       "Port reconfig callback returned error, trying to continue");
                                }
                            }
                        }
                        nLumaFilledLen
                            = (nStride * pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight)
                              - (nStride * activeFrameRegion[0].topLeft.y)
                              - activeFrameRegion[0].topLeft.x;

                        nChromaFilledLen
                            = (nStride * activeFrameRegion[1].bottomRight.y)
                              - nStride + activeFrameRegion[1].bottomRight.x;

                        pOutBufHeader->nFilledLen =
                            nLumaFilledLen + nChromaFilledLen;
                    } else {
                        pOutBufHeader->nOffset = 0;
                        pOutBufHeader->nFilledLen = 0;

                    }
                    _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, pDecOutArgs->outputID[0]);
                }
                i=0;

                while( pDecOutArgs->freeBufID[i] != 0 ) {
                    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "\n FreeBuffID is : %x", pDecOutArgs->freeBufID[i]);
                    if( pDecOutArgs->freeBufID[i] != (OMX_S32) pOutBufHeader ) {
                        _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, pDecOutArgs->freeBufID[i]);
                        pDupBufHeader
                            = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)pDecOutArgs->freeBufID[i];

                        if( pDupBufHeader ) {
                            pDupBufHeader->nOffset = 0;
                            pDupBufHeader->nFilledLen = 0;
                            pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                             OMX_VIDEODECODER_OUTPUT_PORT,
                                                             pDupBufHeader);
                            TIMM_OSAL_Free(pDupBufHeader);
                            pDupBufHeader = TIMM_OSAL_NULL;
                        }
                    }
                    i++;
                }
            }
        } while( status != XDM_EFAIL );
    }

    if( pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp == OMX_TRUE ) {
        TIMM_OSAL_ClearPipe(pVideoDecoderComPvt->pTimeStampStoragePipe);
    }
    if( pLastOutBufHeader != NULL ) {
        pLastOutBufHeader->nFlags |= OMX_BUFFERFLAG_EOS;
        //pLastOutBufHeader->nFilledLen = 0;
        pVideoDecoderComPvt->fpDioSend(hComponent,
                                       OMX_VIDEODECODER_OUTPUT_PORT,
                                       pLastOutBufHeader);
        TIMM_OSAL_Free(pLastOutBufHeader);
        pLastOutBufHeader = TIMM_OSAL_NULL;
    }
    if( pInBufHeader != NULL ) {
        pVideoDecoderComPvt->fpDioSend(hComponent,
                                       OMX_VIDEODECODER_INPUT_PORT,
                                       pInBufHeader);
        /* Send the EOS event to client */
        pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                 OMX_EventBufferFlag,
                                                 OMX_VIDEODECODER_OUTPUT_PORT,
                                                 OMX_BUFFERFLAG_EOS, NULL);
    }
    pVideoDecoderComPvt->nFrameCounter = 0;
    if( pVideoDecoderComPvt->bSupportSkipGreyOutputFrames ) {
        pVideoDecoderComPvt->bSyncFrameReady = OMX_FALSE;
    }
    if( pVideoDecoderComPvt->bErrorConcealmentBuffer && !pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
        RELEASE_MBINFO(pVideoDecoderComPvt)\
    }
    pVideoDecoderComPvt->nOutbufInUseFlag = 0;
    pVideoDecoderComPvt->nFatalErrorGiven = 0;

    //    if( pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->metadataType[0] == IH264VDEC_PARSED_SEI_DATA ) {
    for( i=0; i < (2 * MAX_H264D_REF_FRAMES); i++ ) {
        if( pVideoDecoderComPvt->tMetadata_table_entries[i].bEntryUsedFlag == 1 ) {
            pVideoDecoderComPvt->tMetadata_table_entries[i].bEntryUsedFlag = 0;
            if( pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1 ) {
                _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1);
                pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1 = NULL;
            }
            if( pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 ) {
                _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2);
                pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 = NULL;
            }
            pOutBufHeader
                = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *) pVideoDecoderComPvt->tMetadata_table_entries[i].inputID;

            pOutBufHeader->nOffset = 0;
            pOutBufHeader->nFilledLen = 0;

            TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\n Codec did not free buffer %x, Freeing at OMX!!! \n", pOutBufHeader);

            pVideoDecoderComPvt->fpDioCancel(hComponent,
                                             OMX_VIDEODECODER_OUTPUT_PORT,
                                             pOutBufHeader);
            TIMM_OSAL_Free(pOutBufHeader);
            pOutBufHeader = NULL;

        }
    }

    // }
EXIT:
    if( pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp == OMX_TRUE ) {
        //Clear the pipe, to discard the stale messages
        TIMM_OSAL_ERRORTYPE    err = TIMM_OSAL_ClearPipe(pVideoDecoderComPvt->pTimeStampStoragePipe);
        if( err != TIMM_OSAL_ERR_NONE ) {
            /* if pipe clear fails, nothing can be done, just put error trace */
            TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\npipe clear failed");
        }
    }

    return (eError);
}

/* ==========================================================================*/
/**
 * @fn Calc_InbufSize()
 *                     This method Calcullates Buffer size given width and
 *                     height of buffer
 *
 *  @param [in] width  : Width of the buffer
 *  @param [in] height : Height of the buffer
 *
 */
/* ==========================================================================*/
OMX_U32 Calc_InbufSize(OMX_U32 width, OMX_U32 height)
{
    return (width * height);      //To be changed.
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_Init_Video_decoder_params()
 *                     This method initializes the Port Definitions,
 *                     Port Params, Buffer Params and calls the specific
 *                     Decoder function to set Static Params
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
void OMX_TI_Init_Video_decoder_params(OMX_HANDLETYPE hComponent,
                                      OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt)
{
    OMX_TI_VIDEODECODER_Init_PortDefs(hComponent, pVideoDecoderComPvt);
    OMX_TI_VIDEODECODER_Init_PortParams(hComponent, pVideoDecoderComPvt);
    OMX_TI_VIDEODECODER_Set_2D_BuffParams(hComponent, pVideoDecoderComPvt);
    OMX_TI_VIDEODECODER_Set_UseBufferDescParams(hComponent, pVideoDecoderComPvt);
    /* Call Decoder Specific function to set Static Params */
    pVideoDecoderComPvt->fpSet_StaticParams(hComponent,
                                            pVideoDecoderComPvt->pDecStructures.pDecStaticParams);

    pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxHeight =
        OMX_VIDDEC_DEFAULT_FRAME_HEIGHT;
    pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxWidth =
        OMX_VIDDEC_DEFAULT_FRAME_WIDTH;
    return;
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDDEC_SetInPortDef()
 *                     This method initializes the Port Definitions,
 *                     Port Params, Buffer Params and calls the specific
 *                     Decoder function to set Static Params
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDDEC_SetInPortDef(OMX_HANDLETYPE hComponent,
                                         OMX_PARAM_PORTDEFINITIONTYPE *pPortDefs)
{
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    OMX_VIDEO_CODINGTYPE            currentCompressionType, desiredCompressionType;
    OMX_U32                         nFrameWidth, nFrameHeight;
    OMX_PARAM_PORTDEFINITIONTYPE   *pInputPortDef = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE   *pOutputPortDef = NULL;
    BUFF_PARAMS                     tOutBufParams;
    OMX_U32                         i=0;
    OMX_U32                         bFound = 0;
    OMX_COMPONENTTYPE              *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE       *pVideoDecoderComPvt = NULL;

    /*! Initialize pointers and variables */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    pInputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT];
    pOutputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
    nFrameWidth = pPortDefs->format.video.nFrameWidth;
    nFrameHeight = pPortDefs->format.video.nFrameHeight;
    if( nFrameWidth & 0x0F ) {
        nFrameWidth = nFrameWidth + 16 - (nFrameWidth & 0x0F);
    }
    if( nFrameHeight & 0x1F ) {
        nFrameHeight = nFrameHeight + 32 - (nFrameHeight & 0x1F);
    }
    currentCompressionType = pInputPortDef->format.video.eCompressionFormat;
    desiredCompressionType = pPortDefs->format.video.eCompressionFormat;
    /*! In case there is change in Compression type */
    if( currentCompressionType != desiredCompressionType ) {
        /* De-initialize the current codec */
        pVideoDecoderComPvt->fpDeinit_Codec(hComponent);

        /* Call specific component init depending upon the
           eCompressionFormat set. */
        i=0;

        while( NULL != DecoderList[i].eCompressionFormat ) {
            if( DecoderList[i].eCompressionFormat
                == desiredCompressionType ) {
                /* Component found */
                bFound = 1;
                break;
            }
            i++;
        }

        if( bFound == 0 ) {
            TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Unsupported Compression format given in port definition");
            eError = OMX_ErrorUnsupportedSetting;
            goto EXIT;
        }
        /* Call the Specific Decoder Init function and Initialize Params */
        eError = DecoderList[i].fpDecoderComponentInit(hComponent);
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
        OMX_TI_Init_Video_decoder_params(hComponent, pHandle->pComponentPrivate);

        strcpy((char *)pVideoDecoderComPvt->tComponentRole.cRole,
               (char *)DecoderList[i].cRole);

    }  /* End of if condition for change in codec type */

    /*! set the Actual values of an Input port */
    pInputPortDef->nBufferCountActual = pPortDefs->nBufferCountActual;
    pInputPortDef->format = pPortDefs->format;
    pInputPortDef->nBufferSize = Calc_InbufSize(nFrameWidth, nFrameHeight);

    pVideoDecoderComPvt->tCropDimension.nTop = 0;
    pVideoDecoderComPvt->tCropDimension.nLeft = 0;
    pVideoDecoderComPvt->tCropDimension.nWidth = pInputPortDef->format.video.nFrameWidth;
    pVideoDecoderComPvt->tCropDimension.nHeight = pInputPortDef->format.video.nFrameHeight;

    /*! Set o/p port details according to width/height set at i/p Port. */
    pOutputPortDef->format.video.nFrameWidth = pInputPortDef->format.video.nFrameWidth;
    pOutputPortDef->format.video.nFrameHeight = pInputPortDef->format.video.nFrameHeight;
    OMX_TI_VIDEODECODER_Set_2D_BuffParams(hComponent, pHandle->pComponentPrivate);
    pOutputPortDef->nBufferSize
        = pOutputPortDef->format.video.nStride *
          ((pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight * 3) >> 1);

    tOutBufParams = pVideoDecoderComPvt->fpCalc_OubuffDetails(hComponent,
                                                              nFrameWidth, nFrameHeight);
    pOutputPortDef->nBufferCountMin = tOutBufParams.nBufferCountMin;
    pOutputPortDef->nBufferCountActual = tOutBufParams.nBufferCountActual;

    /*! Set the Static Params (Decoder Specific) */
    pVideoDecoderComPvt->fpSet_StaticParams(hComponent,
                                            pVideoDecoderComPvt->pDecStructures.pDecStaticParams);

    pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxHeight = nFrameHeight;
    pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxWidth = nFrameWidth;
    if( pOutputPortDef->nBufferCountActual < pOutputPortDef->nBufferCountMin ) {
        pOutputPortDef->nBufferCountActual = pOutputPortDef->nBufferCountMin;
        //Add callback if needed.
    }
    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                       " Set Parameter on OMX_IndexParamPortDefinition called \
                         with width= %d, height = %d",
                       pPortDefs->format.video.nFrameWidth,
                       pPortDefs->format.video.nFrameHeight);

EXIT:
    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "exiting SetInPortDef");
    return (eError);


}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDDEC_SetOutPortDef()
 *                     This method Sets the output Port definition. This is
 *                     called from the SetParameter function of the common
 *                     video decoder
 *
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *  @param [in] pPortDefs : The Parameters (port definition) given by the
 *                          SetParameter call
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDDEC_SetOutPortDef(OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt,
                                          OMX_PARAM_PORTDEFINITIONTYPE *pPortDefs)
{
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE   *pOutputPortDef = NULL;
    OMX_PTR                        *pBufParams = NULL;
    OMX_U32                         nOutPort = OMX_VIDEODECODER_OUTPUT_PORT;
    OMX_U32                         nNumBuffers = 0;

    pOutputPortDef = pVideoDecoderComPvt->pPortdefs[nOutPort];

    /*! Set Values to output port based on input parameter */
    pOutputPortDef->nBufferCountActual = pPortDefs->nBufferCountActual;
    pOutputPortDef->format             = pPortDefs->format;
    pOutputPortDef->format.video.nSliceHeight
        = pOutputPortDef->format.video.nFrameHeight;
    pOutputPortDef->nBufferSize = pOutputPortDef->format.video.nStride *
                                  ((pVideoDecoderComPvt->t2DBufferAllocParams[nOutPort].nHeight * 3) >> 1);


#ifdef VIDEODECODER_TILERTEST
    if( pOutputPortDef->format.video.nStride == OMX_VIDDEC_TILER_STRIDE ) {
        /*! In case output buffer is 2D */
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Tiler 2D mode selected on o/p buffers");
        pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->eBufMemoryType
            = OMX_BASE_BUFFER_MEMORY_2D;
        pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->nNumComponentBuffers
            = 2;
        /*! Allocate memory for OMX_BASE_2D_MEM_PARAMS */
        if( pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->pBufParams
            == NULL ) {
            // Allocate memory for OMX_BASE_2D_MEM_PARAMS
            nNumBuffers
                =  pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->nNumComponentBuffers;
            pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->pBufParams
                = (OMX_BASE_2D_MEM_PARAMS *) TIMM_OSAL_Malloc( \
                ((sizeof(OMX_BASE_2D_MEM_PARAMS) * nNumBuffers)),
                TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
            if( pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->pBufParams
                == NULL ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "Could Not allocate Memory");
                eError = OMX_ErrorInsufficientResources;
                goto EXIT;
            }
        }
        pBufParams = pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->pBufParams;

        /*! Set 2D Memory Parameters for Luma */
        ((OMX_BASE_2D_MEM_PARAMS *)pBufParams)[0].nHeight
            = pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight;
        ((OMX_BASE_2D_MEM_PARAMS *)pBufParams)[0].nWidth
            = pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth;
        ((OMX_BASE_2D_MEM_PARAMS *)pBufParams)[0].eAccessMode
            = OMX_BASE_MEMORY_ACCESS_8BIT;

        /*! Set 2D Memory parameters for Chroma */
        ((OMX_BASE_2D_MEM_PARAMS *)pBufParams)[1].nHeight
            = (pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight) >> 1;
        ((OMX_BASE_2D_MEM_PARAMS *)pBufParams)[1].nWidth
            = (pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth) >> 1;
        ((OMX_BASE_2D_MEM_PARAMS *)pBufParams)[1].eAccessMode
            = OMX_BASE_MEMORY_ACCESS_16BIT;
    } else {
        /*! In case output buffer is 1D */
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Tiler 1D mode selected on o/p buffers");
        /*! In case 2D_PARAMS are present make the pointer to NULL */
        if( pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->pBufParams
            != NULL ) {
            TIMM_OSAL_Free(pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->pBufParams);
            pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->pBufParams
                = NULL;
        }
        pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->eBufMemoryType
            = OMX_BASE_BUFFER_MEMORY_1D_CONTIGUOUS;
        pVideoDecoderComPvt->tDerToBase.pPortProperties[nOutPort]->nNumComponentBuffers
            = 1;
    }
#endif

EXIT:
    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "exiting set OutPortDef");
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDDEC_HandleFirstFrame()
 *                     This method handles the first frame processing after
 *                     the call to the codec process function
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *  @param [in] pVideoDecoderComPvt : pointer to decoder private structure
 *  @param [out] pPortReconfigDone  : Flag for Port Reconfig Done
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDDEC_HandleFirstFrame(OMX_HANDLETYPE hComponent,
                                             OMX_BUFFERHEADERTYPE * *ppInBufHeader)
{
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE              *pHandle = NULL;
    OMX_U32                         nFrameWidth, nFrameHeight, nFrameWidthNew, nFrameHeightNew, nFrameRate, nFrameRateNew;
    OMX_VIDEODECODER_PVTTYPE       *pVideoDecoderComPvt = NULL;
    XDAS_Int32                      status = 0;
    BUFF_POINTERS                  *pDecExtStructure = NULL;
    IVIDDEC3_Status                *pDecStatus = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE   *pOutputPortDef = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE   *pInputPortDef = NULL;
    BUFF_PARAMS                     tOutBufParams;
    OMX_BUFFERHEADERTYPE           *pInBufHeader;
    OMX_BOOL                        bPortReconfigRequiredForPadding = OMX_FALSE;
    OMX_CONFIG_RECTTYPE            *p2DOutBufAllocParam = NULL;

    OMX_BOOL    bSendPortReconfigForScale = OMX_FALSE;
    OMX_U32     nScale, nScaleRem, nScaleQ16Low, nScaleWidth, nScaleHeight;
    OMX_U64     nScaleQ16 = 0;


    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    pDecExtStructure = &(pVideoDecoderComPvt->pDecStructures);
    pOutputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
    pInputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT];
    pVideoDecoderComPvt->nOutPortReconfigRequired = 0;
    p2DOutBufAllocParam
        = &(pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT]);

    /*! Call the Codec Control call to get Status from Codec */
    eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_GETSTATUS,
                                          pDecExtStructure->pDecDynParams, pDecExtStructure->pDecStatus, hComponent, &status);
    if( status != VIDDEC3_EOK ) {
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Error in Codec Control Call for GETSTATUS");
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    if( pVideoDecoderComPvt->fpHandle_CodecGetStatus != NULL ) {
        TIMM_OSAL_WarningExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                             "[ hande = 0x%x] Calling GetStatusHandling func", hComponent);
        eError = pVideoDecoderComPvt->fpHandle_CodecGetStatus(hComponent);
    }
    pDecStatus = (IVIDDEC3_Status *)
                 (pVideoDecoderComPvt->pBaseClassStructers.pDecStatus);

    nFrameWidth = pOutputPortDef->format.video.nFrameWidth;
    nFrameHeight = pOutputPortDef->format.video.nFrameHeight;
    nFrameWidthNew = (OMX_U32)pDecStatus->outputWidth;
    nFrameHeightNew = (OMX_U32)pDecStatus->outputHeight;
    nFrameRate = pOutputPortDef->format.video.xFramerate >> 16;

    OMX_BASE_ASSERT(pVideoDecoderComPvt->nFrameRateDivisor != 0, OMX_ErrorBadParameter);

    nFrameRateNew = (OMX_U32)(pDecStatus->frameRate / pVideoDecoderComPvt->nFrameRateDivisor);

    /*Set 200 as max cap, as if clip with incorrect setting is present in sdcard
     it breaks havoc on thumbnail generation */
    if((nFrameRateNew == 0) || (nFrameRateNew > 200)) {
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Codec Returned spurious FrameRate Value - %d Setting Back to - %d",
                           nFrameRateNew, nFrameRate);
        nFrameRateNew = nFrameRate;
    }

#ifdef codec_get_status_workaround
    if( nFrameWidth & 0x0F ) {
        nFrameWidth = nFrameWidth + 16 - (nFrameWidth & 0x0F);
    }
    if( nFrameHeight & 0x1F ) {
        nFrameHeight = nFrameHeight + 32 - (nFrameHeight & 0x1F);
    }
    if( nFrameWidthNew & 0x0F ) {
        nFrameWidthNew = nFrameWidthNew + 16 - (nFrameWidthNew & 0x0F);
    }
    if( nFrameHeightNew & 0x1F ) {
        nFrameHeightNew = nFrameHeightNew + 32 - (nFrameHeightNew & 0x1F);
    }
#endif
    if( pVideoDecoderComPvt->bUsePortReconfigForPadding == OMX_TRUE ) {
        if( pOutputPortDef->format.video.nFrameWidth != p2DOutBufAllocParam->nWidth
            || pOutputPortDef->format.video.nFrameHeight != p2DOutBufAllocParam->nHeight ) {
            bPortReconfigRequiredForPadding = OMX_TRUE;
        }
        nFrameWidth = pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxWidth;
        nFrameHeight = pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxHeight;
    }

    tOutBufParams =
        pVideoDecoderComPvt->fpCalc_OubuffDetails(hComponent,
                                                  (OMX_U32)pDecStatus->outputWidth, (OMX_U32)pDecStatus->outputHeight);

    /*! Check whether the height and width reported by codec matches
     *  that of output port */
    if( nFrameHeightNew != nFrameHeight || nFrameWidthNew != nFrameWidth
        || bPortReconfigRequiredForPadding == OMX_TRUE ||
        pOutputPortDef->nBufferCountMin < tOutBufParams.nBufferCountMin ||
        nFrameRate < nFrameRateNew ) {     /* Compare the min againt the older min buffer count
 since parameters like display delay also gets set according to ref frame. */
        /*! Since the dimensions does not match trigger port reconfig */
        pVideoDecoderComPvt->nOutPortReconfigRequired = 1;
        pVideoDecoderComPvt->nCodecRecreationRequired = 1;
        /* Return back the Input buffer headers Note that the output header
         * will be cancelled later so no need to cancel it here */
        if( ppInBufHeader != NULL ) {
            pInBufHeader = *(ppInBufHeader);
            pVideoDecoderComPvt->fpDioCancel(hComponent, OMX_VIDEODECODER_INPUT_PORT,
                                             pInBufHeader);
            pInBufHeader = NULL;
        }
        /*! Change port definition to match with what codec reports */
        pInputPortDef->format.video.nFrameHeight = (OMX_U32)pDecStatus->outputHeight;
        pInputPortDef->format.video.nFrameWidth = (OMX_U32)pDecStatus->outputWidth;
        pOutputPortDef->format.video.nFrameHeight = (OMX_U32)pDecStatus->outputHeight;
        pOutputPortDef->format.video.nFrameWidth = (OMX_U32)pDecStatus->outputWidth;
        pOutputPortDef->format.video.nSliceHeight = (OMX_U32)pDecStatus->outputHeight;
        if( nFrameRate < nFrameRateNew ) {
            pOutputPortDef->format.video.xFramerate = nFrameRateNew << 16;
            pVideoDecoderComPvt->tVideoParams[OMX_VIDEODECODER_OUTPUT_PORT].xFramerate = nFrameRateNew;
        }
        pVideoDecoderComPvt->tCropDimension.nWidth = (OMX_U32)pDecStatus->outputWidth;
        pVideoDecoderComPvt->tCropDimension.nHeight = (OMX_U32)pDecStatus->outputHeight;
#ifdef Codec_GetBuffinfoBug

        tOutBufParams
            = pVideoDecoderComPvt->fpCalc_OubuffDetails(hComponent,
                                                        (OMX_U32)pDecStatus->outputWidth,
                                                        (OMX_U32)pDecStatus->outputHeight);
        pOutputPortDef->nBufferCountMin = tOutBufParams.nBufferCountMin;
        pOutputPortDef->nBufferCountActual = tOutBufParams.nBufferCountActual;
        OMX_TI_VIDEODECODER_Set_2D_BuffParams(hComponent, pHandle->pComponentPrivate);

        if( pOutputPortDef->format.video.nStride != OMX_VIDDEC_TILER_STRIDE ) {
            pOutputPortDef->format.video.nStride = p2DOutBufAllocParam->nWidth;
        }
        pOutputPortDef->nBufferSize  = pOutputPortDef->format.video.nStride *
                                       ((pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight * 3) >> 1);
#endif
        if( pVideoDecoderComPvt->bUsePortReconfigForPadding == OMX_TRUE ) {
            pInputPortDef->format.video.nFrameHeight = p2DOutBufAllocParam->nHeight;
            pInputPortDef->format.video.nFrameWidth = p2DOutBufAllocParam->nWidth;
            pOutputPortDef->format.video.nFrameHeight = p2DOutBufAllocParam->nHeight;
            pOutputPortDef->format.video.nFrameWidth = p2DOutBufAllocParam->nWidth;
        }
    }
#ifndef Codec_GetBuffinfoBug

    pDecDynamicParams
        = pVideoDecoderComPvt->pDecStructures.pDecDynParams;
    pDecStatus
        = pVideoDecoderComPvt->pDecStructures.pDecStatus;

    // Call the Decoder Control function
    eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_GETBUFINFO,
                                          pDecDynamicParams, pDecStatus, hComponent, &status);
    if( status != VIDDEC3_EOK ) {
        OMX_BASE_Error("VIDDEC3_control XDM_SETPARAMS failed! \n");
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    if( pOutputPortDef->nBufferCountMin !=
        || p2DOutBufAllocParam->nWidth !=
        || p2DOutBufAllocParam->nHeight != ) {
        pVideoDecoderComPvt->nOutPortReconfigRequired = 1;

        pOutputPortDef->nBufferCountMin = (OMX_U32) pDecStatus->maxNumDisplayBufs;
        p2DOutBufAllocParam->nWidth = pDecStatus->bufInfo \ \ Check this in XDM
                                      p2DOutBufAllocParam->nHeight = DecStatus->bufInfo \ \ Check this in XDM
                                                                     pOutputPortDef->nBufferSize
                                                                         = pOutputPortDef->format.video.nStride *
                                                                           ((p2DOutBufAllocParam.nHeight * 3) >> 1);
    }
#endif
    if( pVideoDecoderComPvt->nOutPortReconfigRequired == 1 ) {

        /*! Notify to Client change in output port settings */
        eError = pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                          OMX_EventPortSettingsChanged,
                                                          OMX_VIDEODECODER_OUTPUT_PORT, 0, NULL);

        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Dynamic port reconfiguration triggered");
    } else if( pDecStatus->sampleAspectRatioHeight != 0 && pDecStatus->sampleAspectRatioWidth != 0 ) {

        nScaleWidth = (OMX_U32)pDecStatus->sampleAspectRatioWidth;
        nScaleHeight = (OMX_U32)pDecStatus->sampleAspectRatioHeight;
        nScale = nScaleWidth / nScaleHeight;

        if( nScale >= 1 ) {
            nScaleRem = nScaleWidth % nScaleHeight;
            nScaleQ16Low = 0xFFFF * nScaleRem / nScaleHeight;
            nScaleQ16 = nScale << 16;
            nScaleQ16 |= nScaleQ16Low;
            if( pVideoDecoderComPvt->tScaleParams.xWidth != nScaleQ16
                || pVideoDecoderComPvt->tScaleParams.xHeight != 0x10000 ) {
                pVideoDecoderComPvt->tScaleParams.xWidth = nScaleQ16;
                pVideoDecoderComPvt->tScaleParams.xHeight = 0x10000;
                bSendPortReconfigForScale = OMX_TRUE;
            }
        } else {
            nScale = nScaleHeight / nScaleWidth;
            nScaleRem =  nScaleHeight % nScaleWidth;
            nScaleQ16Low = 0xFFFF * nScaleRem / nScaleWidth;
            nScaleQ16 = nScale << 16;
            nScaleQ16 |= nScaleQ16Low;
            if( pVideoDecoderComPvt->tScaleParams.xWidth != 0x10000
                || pVideoDecoderComPvt->tScaleParams.xHeight != nScaleQ16 ) {
                pVideoDecoderComPvt->tScaleParams.xWidth = 0x10000;
                pVideoDecoderComPvt->tScaleParams.xHeight = nScaleQ16;
                bSendPortReconfigForScale = OMX_TRUE;
            }
        }

        if( bSendPortReconfigForScale == OMX_TRUE ) {

            TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "port reconfig for scaling given, scale width = %x, scale height = %x",
                               pVideoDecoderComPvt->tScaleParams.xWidth, pVideoDecoderComPvt->tScaleParams.xHeight);
            /*! Notify to Client change in output port settings */
            eError = pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                              OMX_EventPortSettingsChanged, OMX_VIDEODECODER_OUTPUT_PORT, OMX_IndexConfigCommonScale, NULL);
            bSendPortReconfigForScale = OMX_FALSE;
        }
    }
    /* Currently only callback for o/p port buffer is done. In future
     * need to determine whether to do callback for i/p buffer too */

EXIT:
    return (eError);

}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDDEC_HandleCodecProcError()
 *                     This method handles the first frame processing after
 *                     the call to the codec process function
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDDEC_HandleCodecProcError(OMX_HANDLETYPE hComponent,
                                                 OMX_BUFFERHEADERTYPE * *ppInBufHeader,
                                                 OMX_VIDEODECODER_CUSTOMBUFFERHEADER * *ppOutBufHeader)
{
    OMX_ERRORTYPE                          eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                     *pHandle = NULL;
    OMX_U32                                nFrameWidth, nFrameHeight, nFrameWidthNew, nFrameHeightNew;
    OMX_VIDEODECODER_PVTTYPE              *pVideoDecoderComPvt = NULL;
    XDAS_Int32                             status = 0;
    IVIDDEC3_Status                       *pDecStatus = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE          *pOutputPortDef = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE          *pInputPortDef = NULL;
    BUFF_POINTERS                         *pDecExtStructure = NULL;
    BUFF_PARAMS                            tOutBufParams;
    OMX_BUFFERHEADERTYPE                  *pInBufHeader = *(ppInBufHeader);
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pDupBufHeader;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pNewOutBufHeader = NULL;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pOutBufHeader = *(ppOutBufHeader);
    OMX_U32                                ii=0;
    XDM_Rect                               activeFrameRegion[2];
    OMX_U32                                nStride = OMX_VIDDEC_TILER_STRIDE;
    OMX_U32                                nLumaFilledLen, nChromaFilledLen;
    OMX_BOOL                               bPortReconfigRequiredForPadding = OMX_FALSE;
    OMX_CONFIG_RECTTYPE                    *p2DOutBufAllocParam = NULL;
    OMX_METADATA                           *pOMXmetaFrame1 = NULL, *pOMXmetaFrame2 = NULL;
    OMX_PTR                                pClientMetaData = NULL, pCodecMetaData = NULL;

    /* Initialize pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    pOutputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
    pInputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT];
    pDecExtStructure = &(pVideoDecoderComPvt->pDecStructures);
    p2DOutBufAllocParam
        = &(pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT]);

    /* Commenting this condition to handle errors(FATAL/Non FATAL) from codec those requests dynamic resolution change */
    #if 0
    if((pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError & 0x8000)
       || ((pVideoDecoderComPvt->eCompressionFormat == OMX_VIDEO_CodingAVC)
           && (pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError
               & IH264VDEC_ERR_PICSIZECHANGE != 0))) {
    #endif
    /*! Call the Codec Status function to know cause of error */
    eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_GETSTATUS,
                                          pDecExtStructure->pDecDynParams, pDecExtStructure->pDecStatus, hComponent, &status);
    /* Check whether the Codec Status call was succesful */
    if( status != VIDDEC3_EOK ) {
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "VIDDEC3_control XDM_GETSTATUS failed");
        if( pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader
            != pVideoDecoderComPvt->parseHeaderFlag ) {
            _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, (OMX_U32) pOutBufHeader);
            // Return the Input and Output buffer header
            pVideoDecoderComPvt->fpDioCancel(hComponent,
                                             OMX_VIDEODECODER_OUTPUT_PORT,
                                             pOutBufHeader);
            /*! Free up the Output buffer */
            if( pOutBufHeader ) {
                TIMM_OSAL_Free(pOutBufHeader);
                pOutBufHeader = TIMM_OSAL_NULL;
            }
            pVideoDecoderComPvt->fpDioCancel(hComponent,
                                             OMX_VIDEODECODER_INPUT_PORT,
                                             pInBufHeader);
            /*! Make Input buffer header pointer NULL */
            pInBufHeader = NULL;
        }
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    pDecStatus = (IVIDDEC3_Status *)
                 (pVideoDecoderComPvt->pBaseClassStructers.pDecStatus);

    nFrameWidth = pOutputPortDef->format.video.nFrameWidth;
    nFrameHeight = pOutputPortDef->format.video.nFrameHeight;
    nFrameWidthNew = pDecStatus->outputWidth;
    nFrameHeightNew = pDecStatus->outputHeight;

    #ifdef codec_get_status_workaround
    if( nFrameWidth & 0x0F ) {
        nFrameWidth = nFrameWidth + 16 - (nFrameWidth & 0x0F);
    }
    if( nFrameHeight & 0x1F ) {
        nFrameHeight = nFrameHeight + 32 - (nFrameHeight & 0x1F);
    }
    if( nFrameWidthNew & 0x0F ) {
        nFrameWidthNew = nFrameWidthNew + 16 - (nFrameWidthNew & 0x0F);
    }
    if( nFrameHeightNew & 0x1F ) {
        nFrameHeightNew = nFrameHeightNew + 32 - (nFrameHeightNew & 0x1F);
    }
    #endif
    if( pVideoDecoderComPvt->bUsePortReconfigForPadding == OMX_TRUE ) {
        if( pOutputPortDef->format.video.nFrameWidth != p2DOutBufAllocParam->nWidth
            || pOutputPortDef->format.video.nFrameHeight != p2DOutBufAllocParam->nHeight ) {
            bPortReconfigRequiredForPadding = OMX_TRUE;
        }
        nFrameWidth = pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxWidth;
        nFrameHeight = pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxHeight;
    }
    /*! Check whether the height and width reported by codec matches
     *  that of output port */
    if( nFrameHeightNew != nFrameHeight || nFrameWidthNew != nFrameWidth
        || bPortReconfigRequiredForPadding == OMX_TRUE ) {
        pVideoDecoderComPvt->nOutPortReconfigRequired = 1;
        pVideoDecoderComPvt->nCodecRecreationRequired = 1;
    }
    #if 0
}

    #endif
    if( pVideoDecoderComPvt->fpHandle_ExtendedError != NULL ) {
        eError = pVideoDecoderComPvt->fpHandle_ExtendedError(hComponent);
    }
    if( pVideoDecoderComPvt->nOutPortReconfigRequired == 1 ) {

        pNewOutBufHeader = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)
                           pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->outputID[0];
        if( pNewOutBufHeader != NULL ) {
            pVideoDecoderComPvt->tCropDimension.nWidth = pDecStatus->outputWidth;
            pVideoDecoderComPvt->tCropDimension.nHeight = pDecStatus->outputHeight;

            /* Extract the stored meta data corresponding to this output buffer
            from the meta data table and copy to buffer header */
            pNewOutBufHeader->nMetaDataUsageBitField = 0;
            _OMX_VIDEODECODER_PULL_METADATA_BUFFER(hComponent,
            pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->outputID[0], &pOMXmetaFrame1, &pOMXmetaFrame2);
            if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pSEIinfo) != NULL ) {
                    if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->parsed_flag == 1 ) {
                        if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->stereo_video_info.parsed_flag == 1 ) {
                            pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2004FRAME1BIT;
                        } else if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->frame_packing.parsed_flag == 1 ) {
                            pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2010FRAME1BIT;
                        }
                    }
                }
                if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pSEIinfo) != NULL ) {
                    if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->parsed_flag == 1 ) {
                        if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->stereo_video_info.parsed_flag == 1 ) {
                            pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2004FRAME2BIT;
                        } else if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->frame_packing.parsed_flag == 1 ) {
                            pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2010FRAME2BIT;
                        }
                    }
                }
            }


            if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pMBinfo) != NULL ) {
                    (pNewOutBufHeader->tMbInfoFrame1).nSize = pVideoDecoderComPvt->nMBinfoSize;
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_MBINFOFRAME1BIT;
                }
                if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pMBinfo) != NULL ) {
                    (pNewOutBufHeader->tMbInfoFrame2).nSize = pVideoDecoderComPvt->nMBinfoSize;
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_MBINFOFRAME2BIT;
                }
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pSEIinfo) != NULL ) {
                    (pNewOutBufHeader->tSeiInfoFrame1).nSize = sizeof(IH264VDEC_SeiMessages);
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFOFRAME1BIT;
                }
                if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pSEIinfo) != NULL ) {
                    (pNewOutBufHeader->tSeiInfoFrame2).nSize = sizeof(IH264VDEC_SeiMessages);
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFOFRAME2BIT;
                }
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
                if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pVUIinfo) != NULL ) {
                    (pNewOutBufHeader->tVuiInfoFrame1).nSize = sizeof(IH264VDEC_VuiParams);
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_VUIINFOFRAME1BIT;
                }
                if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pVUIinfo) != NULL ) {
                    (pNewOutBufHeader->tVuiInfoFrame2).nSize = sizeof(IH264VDEC_VuiParams);
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_VUIINFOFRAME2BIT;
                }
            }

            if( pOMXmetaFrame1 != NULL ) {
                pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_CODECEXTERROR1BIT;
            }
            if( pOMXmetaFrame2 != NULL ) {
                pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_CODECEXTERROR2BIT;
            }

            nStride =
                pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->format.video.nStride;
            activeFrameRegion[0]
                = pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->displayBufs.bufDesc[0].activeFrameRegion;
            activeFrameRegion[1].bottomRight.y
                = (activeFrameRegion[0].bottomRight.y) / 2;
            activeFrameRegion[1].bottomRight.x
                = activeFrameRegion[0].bottomRight.x;

            // Offset = (rows*stride) + x-offset
            pNewOutBufHeader->nOffset
                = (activeFrameRegion[0].topLeft.y * nStride)
                  + activeFrameRegion[0].topLeft.x;

            // FilledLen
            nLumaFilledLen
                = (nStride * (pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight))
                  - (nStride * (activeFrameRegion[0].topLeft.y))
                  - activeFrameRegion[0].topLeft.x;

            nChromaFilledLen
                = (nStride * (activeFrameRegion[1].bottomRight.y))
                  - nStride + activeFrameRegion[1].bottomRight.x;

            pNewOutBufHeader->nFilledLen =
                nLumaFilledLen + nChromaFilledLen;

            /* Update the metadata size to base*/
            pVideoDecoderComPvt->fpDioUtil(hComponent, OMX_VIDEODECODER_OUTPUT_PORT, OMX_BASE_DIO_UTIL_UpdatePacketExtraDataPointer, pNewOutBufHeader);


            if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pSEIinfo) != NULL ) {
                    if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->parsed_flag == 1 ) {
                        if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->stereo_video_info.parsed_flag == 1 ) {
                            pClientMetaData = (OMX_PTR)(&(pNewOutBufHeader->tSEIinfo2004Frame1));
                            pCodecMetaData = (OMX_PTR)(&(((IH264VDEC_SeiMessages *)(pOMXmetaFrame1->pSEIinfo))->stereo_video_info));
                            pCodecMetaData = (OMX_PTR)((OMX_U32) pCodecMetaData + sizeof(XDAS_UInt32));

                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(OMX_TI_STEREODECINFO));
                            pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2004FRAME1BIT;
                        } else if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame1->pSEIinfo))->frame_packing.parsed_flag == 1 ) {
                            pClientMetaData = (OMX_PTR)(&(pNewOutBufHeader->tSEIinfo2010Frame1));
                            pCodecMetaData = (OMX_PTR)(&(((IH264VDEC_SeiMessages *)(pOMXmetaFrame1->pSEIinfo))->frame_packing));
                            pCodecMetaData = (OMX_PTR)((OMX_U32) pCodecMetaData + sizeof(XDAS_UInt32));

                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(OMX_TI_FRAMEPACKINGDECINFO));
                            pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2010FRAME1BIT;
                        }
                    }
                }
                if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pSEIinfo) != NULL ) {
                    if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->parsed_flag == 1 ) {
                        if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->stereo_video_info.parsed_flag == 1 ) {
                            pClientMetaData = (OMX_PTR)(&(pNewOutBufHeader->tSEIinfo2004Frame2));
                            pCodecMetaData = (OMX_PTR)(&(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->stereo_video_info));
                            pCodecMetaData = (OMX_PTR)((OMX_U32) pCodecMetaData + sizeof(XDAS_UInt32));

                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(OMX_TI_STEREODECINFO));
                            pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2004FRAME2BIT;
                        } else if(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->frame_packing.parsed_flag == 1 ) {
                            pClientMetaData = (OMX_PTR)(&(pNewOutBufHeader->tSEIinfo2010Frame2));
                            pCodecMetaData = (OMX_PTR)(&(((IH264VDEC_SeiMessages *) (pOMXmetaFrame2->pSEIinfo))->frame_packing));
                            pCodecMetaData = (OMX_PTR)((OMX_U32) pCodecMetaData + sizeof(XDAS_UInt32));

                            TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(OMX_TI_FRAMEPACKINGDECINFO));
                            pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFO2010FRAME2BIT;
                        }
                    }
                }
            }

            if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pMBinfo) != NULL ) {
                    pClientMetaData = (pNewOutBufHeader->tMbInfoFrame1).pMBinfo;
                    pCodecMetaData = pOMXmetaFrame1->pMBinfo;
                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, pVideoDecoderComPvt->nMBinfoSize);
                    (pNewOutBufHeader->tMbInfoFrame1).nSize = pVideoDecoderComPvt->nMBinfoSize;
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_MBINFOFRAME1BIT;
                }
                if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pMBinfo) != NULL ) {
                    pClientMetaData = (pNewOutBufHeader->tMbInfoFrame2).pMBinfo;
                    pCodecMetaData = pOMXmetaFrame2->pMBinfo;

                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, pVideoDecoderComPvt->nMBinfoSize);
                    (pNewOutBufHeader->tMbInfoFrame2).nSize = pVideoDecoderComPvt->nMBinfoSize;
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_MBINFOFRAME2BIT;
                }
            }

            if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pSEIinfo) != NULL ) {
                    pClientMetaData = (pNewOutBufHeader->tSeiInfoFrame1).pSEIinfo;
                    pCodecMetaData = pOMXmetaFrame1->pSEIinfo;
                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(IH264VDEC_SeiMessages));
                    (pNewOutBufHeader->tSeiInfoFrame1).nSize = sizeof(IH264VDEC_SeiMessages);
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFOFRAME1BIT;
                }
                if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pSEIinfo) != NULL ) {
                    pClientMetaData = (pNewOutBufHeader->tSeiInfoFrame2).pSEIinfo;
                    pCodecMetaData = pOMXmetaFrame2->pSEIinfo;
                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(IH264VDEC_SeiMessages));
                    (pNewOutBufHeader->tSeiInfoFrame2).nSize = sizeof(IH264VDEC_SeiMessages);
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_SEIINFOFRAME2BIT;
                }
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
                if( pOMXmetaFrame1 != NULL && (pOMXmetaFrame1->pVUIinfo) != NULL ) {
                    pClientMetaData = (pNewOutBufHeader->tVuiInfoFrame1).pVUIinfo;
                    pCodecMetaData = pOMXmetaFrame1->pVUIinfo;
                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(IH264VDEC_VuiParams));
                    (pNewOutBufHeader->tVuiInfoFrame1).nSize = sizeof(IH264VDEC_VuiParams);
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_VUIINFOFRAME1BIT;
                }
                if( pOMXmetaFrame2 != NULL && (pOMXmetaFrame2->pVUIinfo) != NULL ) {
                    pClientMetaData = (pNewOutBufHeader->tVuiInfoFrame2).pVUIinfo;
                    pCodecMetaData = pOMXmetaFrame2->pVUIinfo;
                    TIMM_OSAL_Memcpy(pClientMetaData, pCodecMetaData, sizeof(IH264VDEC_VuiParams));
                    (pNewOutBufHeader->tVuiInfoFrame2).nSize = sizeof(IH264VDEC_VuiParams);
                    pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_VUIINFOFRAME2BIT;
                }
            }


            if( pOMXmetaFrame1 != NULL ) {

                pNewOutBufHeader->nCodecExtendedErrorFrame1 = pOMXmetaFrame1->CodecExtenderError;
                pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_CODECEXTERROR1BIT;
            }
            if( pOMXmetaFrame2 != NULL ) {

                pNewOutBufHeader->nCodecExtendedErrorFrame2 = pOMXmetaFrame2->CodecExtenderError;
                pNewOutBufHeader->nMetaDataUsageBitField |= OMX_VIDEODECODER_CODECEXTERROR2BIT;
            }

            _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pOMXmetaFrame1);

            _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pOMXmetaFrame2);

            pVideoDecoderComPvt->fpDioSend(hComponent, OMX_VIDEODECODER_OUTPUT_PORT,
                                           pNewOutBufHeader);
        }

        if( pVideoDecoderComPvt->nOutbufInUseFlag == 0
            && pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader != pVideoDecoderComPvt->parseHeaderFlag ) {
            _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, (OMX_U32) pOutBufHeader);
            pVideoDecoderComPvt->fpDioCancel(hComponent,
                                             OMX_VIDEODECODER_OUTPUT_PORT, pOutBufHeader); // This buffer header is freed afterwards.
        } else {
            pOutBufHeader = NULL;
        }
        if( pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader != pVideoDecoderComPvt->parseHeaderFlag ) {
            pVideoDecoderComPvt->fpDioCancel(hComponent,
                                             OMX_VIDEODECODER_INPUT_PORT, pInBufHeader);

            while( pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->freeBufID[ii] != 0 ) {
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "\n FreeBuffID is : %x", pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->freeBufID[ii]);
                pDupBufHeader
                    = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)
                      pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->freeBufID[ii++];
                _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, (OMX_U32) pDupBufHeader);
                if( pDupBufHeader != pOutBufHeader && pDupBufHeader != pNewOutBufHeader ) {
                    pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                     OMX_VIDEODECODER_OUTPUT_PORT, pDupBufHeader);
                    if( pDupBufHeader ) {
                        TIMM_OSAL_Free(pDupBufHeader);
                        pDupBufHeader = TIMM_OSAL_NULL;
                    }
                }
            }
        }

        if( pOutBufHeader ) {
            TIMM_OSAL_Free(pOutBufHeader);
            pOutBufHeader = TIMM_OSAL_NULL;
        }
        if( pNewOutBufHeader ) {
            TIMM_OSAL_Free(pNewOutBufHeader);
            pNewOutBufHeader = TIMM_OSAL_NULL;
        }
        /*! Notify to Client change in output port settings */
        eError = pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                          OMX_EventPortSettingsChanged,
                                                          OMX_VIDEODECODER_OUTPUT_PORT, 0, NULL);

        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Dynamic port reconfiguration triggered");

        // SEND CALLBACK FOR i/p buffer too?
    }
    if( pVideoDecoderComPvt->nOutPortReconfigRequired == 0 ) {
        if( pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError & 0x8000 ) {
            eError = OMX_ErrorFormatNotDetected;
            if( pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader != pVideoDecoderComPvt->parseHeaderFlag ) {
                if( pVideoDecoderComPvt->nOutbufInUseFlag == 0 ) {
                    _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, (OMX_U32) pOutBufHeader);
                    pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                     OMX_VIDEODECODER_OUTPUT_PORT, pOutBufHeader); // This buffer header is freed afterwards.
                }
                pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                 OMX_VIDEODECODER_INPUT_PORT, pInBufHeader);

                while( pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->freeBufID[ii] != 0 ) {
                    pDupBufHeader
                        = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)
                          pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->freeBufID[ii++];
                    if( pOutBufHeader != pDupBufHeader ) {
                        _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, (OMX_U32) pDupBufHeader);
                        pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                         OMX_VIDEODECODER_OUTPUT_PORT, pDupBufHeader);
                        if( pDupBufHeader ) {
                            TIMM_OSAL_Free(pDupBufHeader);
                            pDupBufHeader = TIMM_OSAL_NULL;
                        }
                    }
                }

                if( pOutBufHeader ) {
                    TIMM_OSAL_Free(pOutBufHeader);
                    pOutBufHeader = TIMM_OSAL_NULL;
                }
                if( pVideoDecoderComPvt->bErrorConcealmentBuffer && !pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                    RELEASE_MBINFO(pVideoDecoderComPvt);
                }
            }
            pVideoDecoderComPvt->nFatalErrorGiven = 1;
        }
    }

    if( nFrameHeightNew != nFrameHeight || nFrameWidthNew != nFrameWidth
        || bPortReconfigRequiredForPadding == OMX_TRUE ) {

        /*Return back the locked buffers before changing the port definition */
        OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, NULL, NULL);

        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "nFrameHeightNew = %d, nFrameHeight = %d, nFrameWidthNew = %d, nFrameWidth = %d",
                           nFrameHeightNew, nFrameHeight, nFrameWidthNew, nFrameWidth);

        /*! Change Port Definition */
        pInputPortDef->format.video.nFrameHeight = pDecStatus->outputHeight;
        pInputPortDef->format.video.nFrameWidth =  pDecStatus->outputWidth;

        pOutputPortDef->format.video.nFrameHeight = pDecStatus->outputHeight;
        pOutputPortDef->format.video.nFrameWidth =  pDecStatus->outputWidth;
        pOutputPortDef->format.video.nSliceHeight = pDecStatus->outputHeight;
        pVideoDecoderComPvt->tCropDimension.nWidth = pDecStatus->outputWidth;
        pVideoDecoderComPvt->tCropDimension.nHeight = pDecStatus->outputHeight;
        tOutBufParams
            = pVideoDecoderComPvt->fpCalc_OubuffDetails(hComponent,
                                                        pDecStatus->outputWidth,
                                                        pDecStatus->outputHeight);
        pOutputPortDef->nBufferCountMin = tOutBufParams.nBufferCountMin;
        pOutputPortDef->nBufferCountActual = tOutBufParams.nBufferCountActual;
        OMX_TI_VIDEODECODER_Set_2D_BuffParams(hComponent, pHandle->pComponentPrivate);

        if( pOutputPortDef->format.video.nStride != OMX_VIDDEC_TILER_STRIDE ) {
            pOutputPortDef->format.video.nStride = p2DOutBufAllocParam->nWidth;
        }
        pOutputPortDef->nBufferSize  = pOutputPortDef->format.video.nStride *
                                       ((pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight * 3) >> 1);
        if( pVideoDecoderComPvt->bUsePortReconfigForPadding == OMX_TRUE ) {
            pInputPortDef->format.video.nFrameHeight = p2DOutBufAllocParam->nHeight;
            pInputPortDef->format.video.nFrameWidth = p2DOutBufAllocParam->nWidth;
            pOutputPortDef->format.video.nFrameHeight = p2DOutBufAllocParam->nHeight;
            pOutputPortDef->format.video.nFrameWidth = p2DOutBufAllocParam->nWidth;
        }
    }

EXIT:
    return (eError);

}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDDEC_HandleCodecProcError()
 *                     This method handles the first frame processing after
 *                     the call to the codec process function
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE  OMX_TI_VIDDEC_HandleLockedBuffer(OMX_HANDLETYPE hComponent,
                                                OMX_VIDEODECODER_CUSTOMBUFFERHEADER *pOutBufHeader)
{
    OMX_ERRORTYPE                          eError = OMX_ErrorNone;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pDupBufHeader = NULL;
    OMX_COMPONENTTYPE                     *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE              *pVideoDecoderComPvt = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    /* Allocate Memory for Duplicate Buf header */
    pDupBufHeader = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)
                    TIMM_OSAL_Malloc(sizeof(OMX_VIDEODECODER_CUSTOMBUFFERHEADER),
                                     TIMM_OSAL_TRUE, 0,
                                     TIMMOSAL_MEM_SEGMENT_EXT);
    if( pDupBufHeader == TIMM_OSAL_NULL ) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    /* Duplicate the Output Buffer */
    pVideoDecoderComPvt->fpDioDup(hComponent,
                                  OMX_VIDEODECODER_OUTPUT_PORT,
                                  pOutBufHeader,
                                  pDupBufHeader);
    if( pOutBufHeader->hMarkTargetComponent != NULL ) {
        pOutBufHeader->pMarkData = NULL;
        pOutBufHeader->hMarkTargetComponent = NULL;
    }
    /* Send the Duplicate buffer back to base */
    pVideoDecoderComPvt->fpDioSend(hComponent,
                                   OMX_VIDEODECODER_OUTPUT_PORT,
                                   pDupBufHeader);
    if( pDupBufHeader ) {
        TIMM_OSAL_Free(pDupBufHeader);
        pDupBufHeader = TIMM_OSAL_NULL;
    }
EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDDEC_CalcFilledLen()
 *                     This method handles the first frame processing after
 *                     the call to the codec process function
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *
 */
/* ==========================================================================*/
void OMX_TI_VIDDEC_CalcFilledLen(OMX_HANDLETYPE hComponent,
                                 IVIDDEC3_OutArgs *pDecOutArgs,
                                 OMX_U32 nStride)
{
    XDM_Rect                               tActiveFrameRegion[2];
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pOutBufHeader;
    OMX_U32                                nLumaFilledLen = 0;
    OMX_U32                                nChromaFilledLen = 0;
    OMX_COMPONENTTYPE                     *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE              *pVideoDecoderComPvt = NULL;

    /* Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pOutBufHeader = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)pDecOutArgs->outputID[0];
    tActiveFrameRegion[0]
        = pDecOutArgs->displayBufs.bufDesc[0].activeFrameRegion;
    tActiveFrameRegion[1]
        = pDecOutArgs->displayBufs.bufDesc[1].activeFrameRegion;

    /*! Calcullate the offset to start of frame */
    pOutBufHeader->nOffset = (tActiveFrameRegion[0].topLeft.y * nStride)
                             + tActiveFrameRegion[0].topLeft.x;

    /*! Calcullate the Luma Filled Length */
    nLumaFilledLen
        = (nStride * (pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight))
          - (nStride * (tActiveFrameRegion[0].topLeft.y))
          - tActiveFrameRegion[0].topLeft.x;

    /*! Calcullate the Chroma Filled Length */
    nChromaFilledLen = (nStride * (tActiveFrameRegion[1].bottomRight.y))
                       - nStride + tActiveFrameRegion[1].bottomRight.x;

    /*! Calcullate the Total Filled length */
    pOutBufHeader->nFilledLen =
        nLumaFilledLen + nChromaFilledLen;

    return;

}

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
void _OMX_VIDEODECODER_PUSH_METADATA_BUFFER(OMX_HANDLETYPE hComponent, OMX_PTR pMetadata, OMX_U32 nInputID)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_U32                     i =0;
    BUFF_POINTERS              *pDecExtStructure = NULL;
    IH264VDEC_Status           *pDecExtStatus = NULL;

    /* Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    if( pVideoDecoderComPvt->nOutbufInUseFlag == 1 ) {
        for( i=0; i < (2 * MAX_H264D_REF_FRAMES); i++ ) {
            if( pVideoDecoderComPvt->tMetadata_table_entries[i].bEntryUsedFlag == 1 &&
                pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 == NULL ) {
                pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 = pMetadata;
                return;
            }
        }
    }
    /* For parse header mode where we dont have any o/p buffer header.
   Thus no need to store the corresponding buffer header */
    if( pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader == XDM_DECODE_AU ) {
        for( i=0; i < (2 * MAX_H264D_REF_FRAMES); i++ ) {
            if( pVideoDecoderComPvt->tMetadata_table_entries[i].bEntryUsedFlag == 0 ) {
                pVideoDecoderComPvt->tMetadata_table_entries[i].bEntryUsedFlag = 1;
                pVideoDecoderComPvt->tMetadata_table_entries[i].inputID = nInputID;
                if( pVideoDecoderComPvt->nOutbufInUseFlag == 0 ) {
                    pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1 = pMetadata;
                    pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 = NULL;
                } else if( pVideoDecoderComPvt->nOutbufInUseFlag == 1 ) {
                    pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 = pMetadata;
                }
                //The below logic is added to trigger port reconfig if we fall short of buffers during h264 playback.
                if( pVideoDecoderComPvt->eCompressionFormat == OMX_VIDEO_CodingAVC &&
                    pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->freeBufID[0] == 0 ) {
                    while( pVideoDecoderComPvt->tMetadata_table_entries[i].bEntryUsedFlag == 1 ) {
                        i++;
                        //H264 Codec can lock max twice the reference frames in side the codec
                        if( i == ((2 * MAX_H264D_REF_FRAMES) + 1)) {
                            TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                               "\nCODEC ERROR!!! H264 codec has locked all 32 buffers!!!!.\n");
                            return;
                        }
                    }

                    if( i == pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountActual ) {
                        pDecExtStructure = &(pVideoDecoderComPvt->pDecStructures);
                        pDecExtStatus = (IH264VDEC_Status *) (pDecExtStructure->pDecStatus);
                        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin
                            = 2 * pDecExtStatus->spsMaxRefFrames + 1;
                        pVideoDecoderComPvt->nOutPortReconfigRequired = 1;
                        /*! Notify to Client change in output port settings */
                        pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                                 OMX_EventPortSettingsChanged,
                                                                 OMX_VIDEODECODER_OUTPUT_PORT, 0, NULL);

                        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                           "Dynamic port reconfiguration triggered. \
                                Number of buffers allocated are less than required by codec for reference. \
                                    Current nBufferCountActual = %d, New nBufferCountMin = %d",
                                           pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountActual,
                                           pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->nBufferCountMin);
                    }
                }
                return;
            }
        }
    } else {
        return;
    }
    //If no empty entry found in Buffer table print error
    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                       "\n Buffer table not big enough to accomodate all buffers.\n");

    return;
}

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
void _OMX_VIDEODECODER_PULL_METADATA_BUFFER(OMX_HANDLETYPE hComponent, OMX_U32 nOutputID, OMX_METADATA * *ppMetadataFrame1, OMX_METADATA * *ppMetadataFrame2)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_U32                     i =0;

    /* Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    for( i=0; i < (2 * MAX_H264D_REF_FRAMES); i++ ) {
        if( pVideoDecoderComPvt->tMetadata_table_entries[i].inputID == nOutputID ) {
            //pVideoDecoderComPvt->tMetadata_table_entries[i].bEntryUsedFlag = 0;
            *ppMetadataFrame1 = pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1;
            *ppMetadataFrame2 = pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2;
            pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1 = NULL;
            pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 = NULL;
            break;
        }
    }

    return;
}

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
void _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(OMX_HANDLETYPE hComponent, OMX_U32 nOutputID)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_U32                     i =0;

    /* Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    for( i=0; i < (2 * MAX_H264D_REF_FRAMES); i++ ) {
        if( pVideoDecoderComPvt->tMetadata_table_entries[i].inputID == nOutputID ) {
            pVideoDecoderComPvt->tMetadata_table_entries[i].bEntryUsedFlag = 0;
            //*ppSEIinfoFrame1 = pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1;
            //*ppSEIinfoFrame2 = pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2;
            if( pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1 ) {
                _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1);
                pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1 = NULL;
            }
            if( pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 ) {
                _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2);
                pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 = NULL;
            }
            //pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame1 = NULL;
            //pVideoDecoderComPvt->tMetadata_table_entries[i].pMetadataBufferFrame2 = NULL;
            break;
        }
    }

    return;
}

/* ==========================================================================*/
/**
 * @fn _OMX_VIDEODECODER_FREE_METADATA_BUFFER()
 *                     This method frees tha metadata memory
 *
 *  @param [in] hComponent : pointer to the OMX Component
 *              pMetadata : pointer to metadata buffer
 */
/* ==========================================================================*/
void _OMX_VIDEODECODER_FREE_METADATA_BUFFER(OMX_METADATA *pMetadata)
{
    if( pMetadata ) {
        if( pMetadata->pMBinfo ) {
            TIMM_OSAL_Free(pMetadata->pMBinfo);
        }

        if( pMetadata->pSEIinfo ) {
            TIMM_OSAL_Free(pMetadata->pSEIinfo);
        }

        if( pMetadata->pVUIinfo ) {
            TIMM_OSAL_Free(pMetadata->pVUIinfo);
        }

        TIMM_OSAL_Free(pMetadata);
    }
    return;
}

OMX_ERRORTYPE _OMX_VIDEODEC_VISACONTROL(VIDDEC3_Handle handle, VIDDEC3_Cmd id,
                                        VIDDEC3_DynamicParams *dynParams, VIDDEC3_Status *status, OMX_HANDLETYPE hComponent, XDAS_Int32 *retval)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    /*Resource Specific params*/

    /*Check for the Params*/
    OMX_BASE_REQUIRE(((hComponent != NULL) && (dynParams != NULL) && (status != NULL) && (retval != NULL)), OMX_ErrorBadParameter);
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = pHandle->pComponentPrivate;

    (*retval) = VIDDEC3_control(pVideoDecoderComPvt->pDecHandle, id, dynParams, status);

EXIT:
    return (eError);

}

