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

#include <OMX_TI_Core.h>
#include <omx_video_decoder_internal.h>


#include <hardware/gralloc.h>
#include <hardware/hardware.h>
#include "hal_public.h"

#define HAL_NV12_PADDED_PIXEL_FORMAT (OMX_TI_COLOR_FormatYUV420PackedSemiPlanar - OMX_COLOR_FormatVendorStartUnused)

#if 1 //ndef BUILD_FOR_SMP
//#define USE_MEMORY_LEAK_TOOL
#endif //BUILD_FOR_SMP

#define MAX_REF_FRAMES 16

#define DUCATI_VIDEO_PERF_MEASUREMENT 0
#if (DUCATI_VIDEO_PERF_MEASUREMENT == 1)
//#include <WTSD_DucatiMMSW/framework/tools_library/inc/tools_time.h>
#endif

/* PSI_KPI profiler */
//#include "WTSD_DucatiMMSW/platform/utils/profile.h"

/* ==========================================================================*/
/**
 * @fn OMX_TI_VideoDecoder_ComponentInit()
 *                     This method will implement the base decoder init
 *                     functionality. This function is called by the decoder
 *                     specific ComponentInit function.
 *
 *  @param [in] hComponent : Pointer to the OMX Component
 *
 */
/* ==========================================================================*/

OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE              *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE       *pVideoDecoderComPvt = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE   *pOutputPortDef = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;


    /*! Call the Common Decoder Component Init */
    eError = OMX_TI_VideoDecoder_ComponentInit(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    /*! Call the MPEG4 Decoder Specific Init as default */
    eError = DecoderList[DEFAULT_COMPOENENT].fpDecoderComponentInit(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    /*! Initialize the Decoder Base Params */
    OMX_TI_Init_Video_decoder_params(hComponent, pHandle->pComponentPrivate);

    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pOutputPortDef
        = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];

    pVideoDecoderComPvt->tCropDimension.nWidth = pOutputPortDef->format.video.nFrameWidth;
    pVideoDecoderComPvt->tCropDimension.nHeight = pOutputPortDef->format.video.nFrameHeight;

    strcpy((char *)pVideoDecoderComPvt->tComponentRole.cRole,
           (char *)DecoderList[DEFAULT_COMPOENENT].cRole);

EXIT:
    return (eError);
}

OMX_ERRORTYPE OMX_TI_VideoDecoder_ComponentInit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    Engine_Error                errorCode;

    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pHandle->pComponentPrivate = (OMX_VIDEODECODER_PVTTYPE *)
                                 TIMM_OSAL_Malloc(sizeof(OMX_VIDEODECODER_PVTTYPE),
                                                  TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
    OMX_BASE_ASSERT(pHandle->pComponentPrivate != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(pHandle->pComponentPrivate, 0x0,
                     sizeof(OMX_VIDEODECODER_PVTTYPE));

    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    pVideoDecoderComPvt->mem_count_start = TIMM_OSAL_GetMemCounter();
    pVideoDecoderComPvt->mem_size_start = TIMM_OSAL_GetMemUsage();
#ifdef USE_MEMORY_LEAK_TOOL
    Memory_getStats(NULL, &pVideoDecoderComPvt->stats);
#endif //USE_MEMORY_LEAK_TOOL

    pVideoDecoderComPvt->strComponentName = OMX_VIDEODECODER_COMPONENT_NAME;

    /*! Initialize the DerToBase structure fields */
    eError =
        OMX_TI_VIDEODECODER_Init_DerToBase_Fields(pHandle->pComponentPrivate);
    if( eError != OMX_ErrorNone ) {
        goto EXIT;
    }
    /*Setting default properties.
     * PreCondition: NumOfPorts is filled and all pointers allocated*/
    OMX_BASE_SetDefaultProperties(hComponent);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, OMX_ErrorUnsupportedSetting);

    /*! Override Default values of DerToBase Structure fields */
    eError
        = OMX_TI_VIDEODECODER_Set_DerToBase_Fields(pHandle->pComponentPrivate);
    if( eError != OMX_ErrorNone ) {
        goto EXIT;
    }
    /*! Initialize function pointers for Command and Data Notify */
    pVideoDecoderComPvt->tDerToBase.fpCommandNotify
        = OMX_TI_VIDEODECODER_CommandNotify;
    pVideoDecoderComPvt->tDerToBase.fpDataNotify
        = OMX_TI_VIDEODECODER_DataNotify;

    /*! Call the Base component Init function */
    eError = OMX_BASE_ComponentInit(hComponent);
    OMX_BASE_REQUIRE(eError == OMX_ErrorNone, eError);

    /*! Initialize the function parameters for OMX functions */
    pHandle->SetParameter = OMX_TI_VIDEODECODER_SetParameter;
    pHandle->GetParameter = OMX_TI_VIDEODECODER_GetParameter;
    pHandle->GetConfig = OMX_TI_VIDEODECODER_GetConfig;
    pHandle->SetConfig = OMX_TI_VIDEODECODER_SetConfig;
    pHandle->ComponentDeInit = OMX_TI_VIDEODECODER_ComponentDeinit;
    pHandle->GetExtensionIndex = OMX_TI_VIDEODECODER_GetExtensionIndex;

    /*! Open instance of Codec engine */
    pVideoDecoderComPvt->ce = Engine_open(engineName, NULL, &errorCode);
    pVideoDecoderComPvt->pDecHandle = NULL;
    pVideoDecoderComPvt->pAttrDesc = NULL;
    pVideoDecoderComPvt->nFrameCounter = 0;
    pVideoDecoderComPvt->bSyncFrameReady = OMX_FALSE;
    pVideoDecoderComPvt->nOutbufInUseFlag = 0;
    pVideoDecoderComPvt->nCodecRecreationRequired = 0;
    pVideoDecoderComPvt->nOutPortReconfigRequired = 0;
    pVideoDecoderComPvt->nFatalErrorGiven = 0;
    pVideoDecoderComPvt->pMBinfo = NULL;
    pVideoDecoderComPvt->nMBinfoSize = 0;
    pVideoDecoderComPvt->bInputBufferCancelled = 0;
    /*! Initialize the Framerate divisor to 1000 as codec is supposed provide framerate after by multiplying with 1000 */
    pVideoDecoderComPvt->nFrameRateDivisor = 1000;


    //Disable Metadata as of now, since there is no support in ducati for allocating tiler buffers
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    nNumMetaDataFields = 14;
#if 1
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr = (OMX_EXTRADATATYPE *)
                             OMX_BASE_Malloc(14 * sizeof(OMX_EXTRADATATYPE),
                                             pVideoDecoderComPvt->tDerToBase.hDefaultHeap);
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[0] = (OMX_EXTRADATATYPE) OMX_TI_SEIinfo2004Frame1;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[1] = (OMX_EXTRADATATYPE) OMX_TI_SEIinfo2004Frame2;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[2] = (OMX_EXTRADATATYPE) OMX_TI_SEIinfo2010Frame1;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[3] = (OMX_EXTRADATATYPE) OMX_TI_SEIinfo2010Frame2;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[4] = (OMX_EXTRADATATYPE) OMX_TI_RangeMappingInfo;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[5] = (OMX_EXTRADATATYPE) OMX_TI_RescalingInfo;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[6] = (OMX_EXTRADATATYPE) OMX_TI_CodecExtenderErrorFrame1;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[7] = (OMX_EXTRADATATYPE) OMX_TI_CodecExtenderErrorFrame2;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[8] = (OMX_EXTRADATATYPE) OMX_TI_MBInfoFrame1;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[9] = (OMX_EXTRADATATYPE) OMX_TI_MBInfoFrame2;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[10] = (OMX_EXTRADATATYPE) OMX_TI_SEIInfoFrame1;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[11] = (OMX_EXTRADATATYPE) OMX_TI_SEIInfoFrame2;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[12] = (OMX_EXTRADATATYPE) OMX_TI_VUIInfoFrame1;
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr[13] = (OMX_EXTRADATATYPE) OMX_TI_VUIInfoFrame2;
#else
    pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->
    pMetaDataFieldTypesArr = NULL;
#endif

    pVideoDecoderComPvt->tCropDimension.nTop = 0;
    pVideoDecoderComPvt->tCropDimension.nLeft = 0;
    pVideoDecoderComPvt->tScaleParams.xWidth = 0x10000;
    pVideoDecoderComPvt->tScaleParams.xHeight = 0x10000;
    pVideoDecoderComPvt->tIlaceFmtTypes.bInterlaceFormat = OMX_FALSE;
    pVideoDecoderComPvt->tIlaceFmtTypes.nInterlaceFormats = OMX_InterlaceFrameProgressive;
    pVideoDecoderComPvt->bUsePortReconfigForCrop = OMX_FALSE;
    pVideoDecoderComPvt->bUsePortReconfigForPadding = OMX_FALSE;


#if (DUCATI_VIDEO_PERF_MEASUREMENT == 1)
    pVideoDecoderComPvt->decRecordSessionFrameCounter=0;
    pVideoDecoderComPvt->decRecordSessionProcessTimeCounter=0;
    pVideoDecoderComPvt->decRecordSessionAvgProcessTime=0;
    pVideoDecoderComPvt->decRecordSessionMinProcessTime=0xFF;
    pVideoDecoderComPvt->decRecordSessionMaxProcessTime=0;
#endif

    pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp = OMX_FALSE;
    pVideoDecoderComPvt->bSupportSkipGreyOutputFrames = OMX_TRUE;

    //Optimize this pipe to be created only if decode timestamps is requested.
    TIMM_OSAL_CreatePipe(&(pVideoDecoderComPvt->pTimeStampStoragePipe),
                         MAX_REF_FRAMES * sizeof(OMX_TICKS),
                         sizeof(OMX_TICKS), 1);
    //Can be added in base region to support all component
    //Creating mutex for suspend resume feature

EXIT:
    if( eError != OMX_ErrorNone ) {
        if( pHandle != NULL ) {
            OMX_TI_VIDEODECODER_ComponentDeinit(hComponent);
        }
    }
    return (eError);
}

/*
_LOOKUP_VUI_PARAMS(OMX_U32 aspect_ratio_idc, OMX_U32 * nScaleWidth, OMX_U32 * nScaleHeight) {
    switch( aspect_ratio_idc ) {
        case 1 :
            *nScaleWidth = 1;
            *nScaleHeight = 1;
            break;
        case 2 :
            *nScaleWidth = 12;
            *nScaleHeight = 11;
            break;
        case 3 :
            *nScaleWidth = 10;
            *nScaleHeight = 11;
            break;
        case 4 :
            *nScaleWidth = 16;
            *nScaleHeight = 11;
            break;
        case 5 :
            *nScaleWidth = 40;
            *nScaleHeight = 33;
            break;
        case 6 :
            *nScaleWidth = 24;
            *nScaleHeight = 11;
            break;
        case 7 :
            *nScaleWidth = 20;
            *nScaleHeight = 11;
            break;
        case 8 :
            *nScaleWidth = 32;
            *nScaleHeight = 11;
            break;
        case 9 :
            *nScaleWidth = 80;
            *nScaleHeight = 33;
            break;
        case 10 :
            *nScaleWidth = 18;
            *nScaleHeight = 11;
            break;
        case 11 :
            *nScaleWidth = 15;
            *nScaleHeight = 11;
            break;
        case 12 :
            *nScaleWidth = 64;
            *nScaleHeight = 33;
            break;
        case 13 :
            *nScaleWidth = 160;
            *nScaleHeight = 99;
            break;
        case 14 :
            *nScaleWidth = 4;
            *nScaleHeight = 3;
            break;
        case 15 :
            *nScaleWidth = 3;
            *nScaleHeight = 2;
            break;
        case 16 :
            *nScaleWidth = 2;
            *nScaleHeight = 1;
            break;
    }
}
*/

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_SetParameter()
 *                     This method is called by the IL Client to set parameter
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] nIndex      : Index of the parameter to be set
 *  @param [in] pParamStruct: pointer to the paramter structure to be set
 *
 */
/* ==========================================================================*/

OMX_ERRORTYPE OMX_TI_VIDEODECODER_SetParameter(OMX_HANDLETYPE hComponent,
                                               OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                        eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                   *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE            *pVideoDecoderComPvt = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE        *pPortDefs = NULL;
    OMX_VIDEO_PARAM_PORTFORMATTYPE      *pVideoParams = NULL;
    OMX_CONFIG_RECTTYPE                 *p2DBufferAllocParams = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE        *pInputPortDef = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE        *pOutputPortDef = NULL;
    OMX_VIDEO_PARAM_PORTFORMATTYPE      *pInPortFmtType = NULL;
    OMX_VIDEO_PARAM_PORTFORMATTYPE      *pOutPortFmtType = NULL;
    OMX_CONFIG_RECTTYPE                 *pOut2DBufAllocParam = NULL;
    OMX_CONFIG_RECTTYPE                 *pIn2DBufAllocParam = NULL;
    OMX_TI_PARAM_ENHANCEDPORTRECONFIG   *pEnhancedPortReconfig = NULL;
    OMX_TI_PARAM_USEBUFFERDESCRIPTOR    *ptBufDescParam = NULL;
    OMX_U32                              i=0;
    OMX_U32                              bFound = 0;
    OMX_U32                              nFrameHeight = 0;
    OMX_U32                              nFrameWidth  = 0;

    OMX_BASE_REQUIRE((hComponent != NULL) && (pParamStruct != NULL),
                     OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    switch( nIndex ) {
        case OMX_IndexParamPortDefinition :
        {
            /*! To set the Port Definition */
            OMX_BASE_CHK_VERSION(pParamStruct,
                                 OMX_PARAM_PORTDEFINITIONTYPE, eError);

            pPortDefs = (OMX_PARAM_PORTDEFINITIONTYPE *) pParamStruct;
            pInputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT];
            pOutputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];

            //Check if an invalid port index is not sent
            OMX_BASE_ASSERT(pPortDefs->nPortIndex >= pVideoDecoderComPvt->tDerToBase.nMinStartPortIndex
                            && pPortDefs->nPortIndex < (pVideoDecoderComPvt->tDerToBase.nNumPorts + pVideoDecoderComPvt->tDerToBase.nMinStartPortIndex),
                            OMX_ErrorBadPortIndex);

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[pPortDefs->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);
            // If Input Port Parameters have to be changed
            if( pPortDefs->nPortIndex == pInputPortDef->nPortIndex ) {
                /*! Check for correct resolution of the stream */
                nFrameWidth = pPortDefs->format.video.nFrameWidth;
                nFrameHeight = pPortDefs->format.video.nFrameHeight;

                OMX_BASE_ASSERT((nFrameWidth <= OMX_VIDDEC_MAX_WIDTH),
                                OMX_ErrorUnsupportedSetting);
                OMX_BASE_ASSERT((nFrameHeight <= OMX_VIDDEC_MAX_HEIGHT),
                                OMX_ErrorUnsupportedSetting);
                OMX_BASE_ASSERT((((nFrameWidth * nFrameHeight) >> 16) <= OMX_VIDDEC_MAX_MACROBLOCK),
                                OMX_ErrorUnsupportedSetting);

                /*! Call function to set input port definition */
                eError = OMX_TI_VIDDEC_SetInPortDef(hComponent, pPortDefs);
                if( eError != OMX_ErrorNone ) {
                    goto EXIT;
                }
                if( pVideoDecoderComPvt->tCurState != OMX_StateLoaded ) {
                    pVideoDecoderComPvt->nCodecRecreationRequired = 1;
                }
            }
            // If Output Port Parameters have to be changed
            else if( pPortDefs->nPortIndex == pOutputPortDef->nPortIndex ) {
                // Check the buffer cnt is greater than min required
                // buffer count
                OMX_BASE_ASSERT((pPortDefs->nBufferCountActual
                                 >= pOutputPortDef->nBufferCountMin),
                                OMX_ErrorUnsupportedSetting);
                // Check if Resolution being set at output port is same
                // as the input port
                OMX_BASE_ASSERT((pOutputPortDef->format.video.nFrameHeight
                                 == pPortDefs->format.video.nFrameHeight)
                                && (pOutputPortDef->format.video.nFrameWidth
                                    == pPortDefs->format.video.nFrameWidth),
                                OMX_ErrorUnsupportedSetting);

                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "Stride value set on o/p port is %d", pPortDefs->format.video.nStride);

                OMX_BASE_ASSERT(pPortDefs->format.video.nStride >=
                                pOutputPortDef->format.video.nFrameWidth,
                                OMX_ErrorUnsupportedSetting);
                if( pPortDefs->format.video.nStride >
                    pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth ) {
                    //Supported values of stride are only multiples of 128
                    OMX_BASE_ASSERT((pPortDefs->format.video.nStride & 0x7F) == 0,
                                    OMX_ErrorUnsupportedSetting);
                }
                //Check for the supported Color-fromat
                //and compression format.
                OMX_BASE_ASSERT((pOutputPortDef->format.video.eColorFormat ==
                                 OMX_TI_COLOR_FormatYUV420PackedSemiPlanar),
                                OMX_ErrorUnsupportedSetting);
                OMX_BASE_ASSERT((pOutputPortDef->format.video.eCompressionFormat ==
                                 OMX_VIDEO_CodingUnused),
                                OMX_ErrorUnsupportedSetting);

                /*! Call function to set output port definition */
                eError = OMX_TI_VIDDEC_SetOutPortDef(pVideoDecoderComPvt, pPortDefs);
                if( eError != OMX_ErrorNone ) {
                    goto EXIT;
                }
            }
        }
        break;

        case OMX_IndexParamVideoPortFormat :
        {
            // To set the Video Port Format
            // ToDo : We have to consider the possibility of format being
            //        changed here. In that case we might have to do what
            //        we did for Port Definition change
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PORTFORMATTYPE, eError);
            pVideoParams = (OMX_VIDEO_PARAM_PORTFORMATTYPE *) pParamStruct;
            pOutputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[pVideoParams->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            pInPortFmtType = &(pVideoDecoderComPvt->tVideoParams[OMX_VIDEODECODER_INPUT_PORT]);
            pOutPortFmtType = &(pVideoDecoderComPvt->tVideoParams[OMX_VIDEODECODER_OUTPUT_PORT]);
            // If Input Port Format type has to be changed
            if( pVideoParams->nPortIndex == pInPortFmtType->nPortIndex ) {
                // Change Compression type and frame-rate
                pInPortFmtType->eCompressionFormat
                    = pVideoParams->eCompressionFormat;
                pInPortFmtType->xFramerate = pVideoParams->xFramerate;
            }
            // In case Output Port Format type has to be changed
            else if( pVideoParams->nPortIndex == pOutPortFmtType->nPortIndex ) {
                //Check for the supported Color-fromat
                //and compression format.
                OMX_BASE_ASSERT((pVideoParams->eColorFormat ==
                                 OMX_TI_COLOR_FormatYUV420PackedSemiPlanar),
                                OMX_ErrorUnsupportedSetting);
                OMX_BASE_ASSERT(
                    (pVideoParams->eCompressionFormat ==
                     OMX_VIDEO_CodingUnused), OMX_ErrorUnsupportedSetting);

                pOutputPortDef->format.video.eColorFormat
                    = pVideoParams->eColorFormat;

                // Change Compression type, color format and frame-rate
                pOutPortFmtType->eCompressionFormat
                    = pVideoParams->eCompressionFormat;
                pOutPortFmtType->eColorFormat = pVideoParams->eColorFormat;
                pOutPortFmtType->xFramerate = pVideoParams->xFramerate;
                //pOutputPortDef->format.video.xFramerate = pOutPortFmtType->xFramerate << 16;
            }
        }
        break;

        case (OMX_INDEXTYPE)OMX_TI_IndexParam2DBufferAllocDimension :
        {
            // In case of 2D Buffer Alloc Dimension
            OMX_BASE_CHK_VERSION(pParamStruct,
                                 OMX_VIDEO_PARAM_PORTFORMATTYPE,
                                 eError);
            p2DBufferAllocParams = (OMX_CONFIG_RECTTYPE *) pParamStruct;

            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[p2DBufferAllocParams->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            pOut2DBufAllocParam
                = &(pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT]);
            pIn2DBufAllocParam
                = &(pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_INPUT_PORT]);
            if( p2DBufferAllocParams->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                // In case output buffer parameters have to be changed
                if((p2DBufferAllocParams->nWidth >= pOut2DBufAllocParam->nWidth) &&
                   (p2DBufferAllocParams->nHeight >= pOut2DBufAllocParam->nHeight) &&
                   (p2DBufferAllocParams->nLeft % pOut2DBufAllocParam->nLeft == 0) &&
                   (p2DBufferAllocParams->nTop % pOut2DBufAllocParam->nTop == 0)) {
                    *pOut2DBufAllocParam = *p2DBufferAllocParams;
                } else {
                    eError = OMX_ErrorBadParameter;
                }
            } else if( p2DBufferAllocParams->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                if((p2DBufferAllocParams->nWidth >= pIn2DBufAllocParam->nWidth) &&
                   (p2DBufferAllocParams->nHeight == 1) &&
                   (p2DBufferAllocParams->nLeft % pIn2DBufAllocParam->nLeft == 0)) {
                    *pIn2DBufAllocParam = *p2DBufferAllocParams;
                } else {
                    eError = OMX_ErrorBadParameter;
                }
            }
        }
        break;
        case OMX_IndexParamStandardComponentRole :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_COMPONENTROLETYPE, eError);

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);
            /*! In case there is change in Role */
            if( strcmp((char *)((OMX_PARAM_COMPONENTROLETYPE *)pParamStruct)->cRole,
                       (char *)pVideoDecoderComPvt->tComponentRole.cRole)) {
                /* De-initialize the current codec */
                pVideoDecoderComPvt->fpDeinit_Codec(hComponent);

                /* Call specific component init depending upon the
                   cRole set. */
                i=0;

                while( NULL != DecoderList[i].cRole[0] ) {
                    if( strcmp((char *)((OMX_PARAM_COMPONENTROLETYPE *)pParamStruct)->cRole,
                               (char *)DecoderList[i].cRole) == 0 ) {
                        /* Component found */
                        bFound = 1;
                        break;
                    }
                    i++;
                }

                if( bFound == 0 ) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Unsupported Role set");
                    eError = OMX_ErrorUnsupportedSetting;
                    goto EXIT;
                }
                /* Call the Specific Decoder Init function and Initialize Params */
                eError = DecoderList[i].fpDecoderComponentInit(hComponent);
                OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
                OMX_TI_Init_Video_decoder_params(hComponent, pHandle->pComponentPrivate);

                strcpy((char *)pVideoDecoderComPvt->tComponentRole.cRole,
                       (char *)((OMX_PARAM_COMPONENTROLETYPE *)pParamStruct)->cRole);
            }
        }
        break;
        case (OMX_INDEXTYPE)OMX_TI_IndexParamUseEnhancedPortReconfig :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_ENHANCEDPORTRECONFIG, eError);

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);
            pEnhancedPortReconfig = (OMX_TI_PARAM_ENHANCEDPORTRECONFIG *) pParamStruct;
            if( pEnhancedPortReconfig->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pVideoDecoderComPvt->bUsePortReconfigForCrop = pEnhancedPortReconfig->bUsePortReconfigForCrop;
                pVideoDecoderComPvt->bUsePortReconfigForPadding = pEnhancedPortReconfig->bUsePortReconfigForPadding;
            }
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexParamTimeStampInDecodeOrder :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_TIMESTAMP_IN_DECODE_ORDER, eError);

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);
            if(((OMX_TI_PARAM_TIMESTAMP_IN_DECODE_ORDER *) pParamStruct)->bEnabled == OMX_TRUE ) {
                pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp = OMX_TRUE;
            } else {
                pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp = OMX_FALSE;
            }
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexParamSkipGreyOutputFrames :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_SKIP_GREY_OUTPUT_FRAMES, eError);

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);
            if(((OMX_TI_PARAM_SKIP_GREY_OUTPUT_FRAMES *) pParamStruct)->bEnabled == OMX_TRUE ) {
                pVideoDecoderComPvt->bSupportSkipGreyOutputFrames = OMX_TRUE;
                pVideoDecoderComPvt->bSyncFrameReady = OMX_FALSE;
            } else {
                pVideoDecoderComPvt->bSupportSkipGreyOutputFrames = OMX_FALSE;
                pVideoDecoderComPvt->bSyncFrameReady = OMX_TRUE;
            }
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexParamVideoEnableMetadata :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_DECMETADATA, eError);

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState == OMX_StateLoaded,
                            OMX_ErrorIncorrectStateOperation);

            i=0;
            if(((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableSEIInfo == OMX_TRUE ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "Unsupported Metadata set");
                eError = OMX_ErrorUnsupportedSetting;
                goto EXIT;

            } else {
                pVideoDecoderComPvt->tMetadata.bEnableSEIInfo = OMX_FALSE;
            }
            if(((OMX_TI_PARAM_DECMETADATA *) pParamStruct)->bEnableVUIInfo == OMX_TRUE ) {
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "Unsupported Metadata set");
                eError = OMX_ErrorUnsupportedSetting;
                goto EXIT;

            } else {
                pVideoDecoderComPvt->tMetadata.bEnableVUIInfo = OMX_FALSE;
            }

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

            while( i < 3 ) {
                ((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->metadataType[i++] = IVIDEO_METADATAPLANE_NONE;
            }
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexUseBufferDescriptor :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_USEBUFFERDESCRIPTOR, eError);
            ptBufDescParam = (OMX_TI_PARAM_USEBUFFERDESCRIPTOR *) pParamStruct;
            /* SetParameter can be invoked in Loaded State or disabled port */
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[ptBufDescParam->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);
            if( ptBufDescParam->bEnabled == OMX_TRUE ) {
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "OMX_TI_IndexUseBufferDescriptor is enabled");
                pInputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT];
                pOutputPortDef = pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
                /* checking for I/P ports (1D) */
                if( ptBufDescParam->nPortIndex == pInputPortDef->nPortIndex ) {
                    OMX_BASE_ASSERT(ptBufDescParam->eBufferType == OMX_TI_BufferTypeDefault, OMX_ErrorUnsupportedSetting);
                } /* checking for O/P ports (2D) */
                else if( ptBufDescParam->nPortIndex == pOutputPortDef->nPortIndex ) {
                    OMX_BASE_ASSERT(ptBufDescParam->eBufferType == OMX_TI_BufferTypeDefault || ptBufDescParam->eBufferType == OMX_TI_BufferTypeVirtual2D, OMX_ErrorUnsupportedSetting);
                }
                //save settings
                pVideoDecoderComPvt->tUseBufDesc[ptBufDescParam->nPortIndex].bEnabled = ptBufDescParam->bEnabled;
                pVideoDecoderComPvt->tUseBufDesc[ptBufDescParam->nPortIndex].eBufferType = ptBufDescParam->eBufferType;
            } else {
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "OMX_TI_IndexUseBufferDescriptor is disabled");
            }
        }
        break;
        default :
            eError = OMX_BASE_SetParameter(hComponent, nIndex, pParamStruct);
    }

EXIT:
    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "exiting set parameter");
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_GetParameter()
 *                     This method is called by the IL Client to get parameter
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] nIndex      : Index of the parameter to get
 *  @param [in] pParamStruct: pointer to the paramter structure
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_GetParameter(OMX_HANDLETYPE hComponent,
                                               OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                        eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                   *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE            *pVideoDecoderComPvt = NULL;
    OMX_VIDEO_PARAM_PORTFORMATTYPE      *pVideoParams = NULL;
    OMX_CONFIG_RECTTYPE                 *p2DBufferAllocParams = NULL;
    OMX_VIDEO_PARAM_PORTFORMATTYPE       inPortParam;
    OMX_VIDEO_PARAM_PORTFORMATTYPE       outPortParam;
    OMX_TI_PARAM_ENHANCEDPORTRECONFIG   *pEnhancedPortReconfig = NULL;
    OMX_TI_PARAM_USEBUFFERDESCRIPTOR    *ptBufDescParam = NULL;
    OMX_TI_PARAMNATIVEBUFFERUSAGE *pUsage = NULL;

    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pParamStruct != NULL), OMX_ErrorBadParameter);

    // Initialize the local variables
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    /* GetParameter can't be invoked incase the comp is in Invalid State  */
    OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState != OMX_StateInvalid,
                    OMX_ErrorIncorrectStateOperation);

    switch( nIndex ) {
        case OMX_IndexParamVideoPortFormat :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PORTFORMATTYPE, eError);

            // Initialize Port Params
            pVideoParams = (OMX_VIDEO_PARAM_PORTFORMATTYPE *) pParamStruct;
            inPortParam =
                pVideoDecoderComPvt->tVideoParams[OMX_VIDEODECODER_INPUT_PORT];
            outPortParam =
                pVideoDecoderComPvt->tVideoParams[OMX_VIDEODECODER_OUTPUT_PORT];
            if( pVideoParams->nPortIndex == inPortParam.nPortIndex ) {
                if( pVideoParams->nIndex > inPortParam.nIndex ) {
                    return (OMX_ErrorNoMore);
                }
                pVideoParams->eCompressionFormat = inPortParam.eCompressionFormat;
                pVideoParams->eColorFormat = inPortParam.eColorFormat;
                pVideoParams->xFramerate = inPortParam.xFramerate;
                if(pVideoParams->eColorFormat == OMX_COLOR_FormatYUV420PackedSemiPlanar)
                    pVideoParams->eColorFormat = OMX_TI_COLOR_FormatYUV420PackedSemiPlanar;

            } else if( pVideoParams->nPortIndex == outPortParam.nPortIndex ) {
                if( pVideoParams->nIndex == 0 ) {
                    pVideoParams->eColorFormat = OMX_COLOR_FormatYUV420PackedSemiPlanar;
                } else if( pVideoParams->nIndex > outPortParam.nIndex ) {
                    return (OMX_ErrorNoMore);
                }
                pVideoParams->eCompressionFormat = outPortParam.eCompressionFormat;
                pVideoParams->xFramerate = outPortParam.xFramerate;
                if(pVideoParams->eColorFormat == OMX_COLOR_FormatYUV420PackedSemiPlanar) {
                    if (pVideoDecoderComPvt->tDerToBase.pPortProperties[pVideoParams->nPortIndex]->eBufMemoryType == OMX_BASE_BUFFER_MEMORY_GRALLOC)
                        pVideoParams->eColorFormat = HAL_NV12_PADDED_PIXEL_FORMAT;
                    else
                        pVideoParams->eColorFormat = OMX_TI_COLOR_FormatYUV420PackedSemiPlanar;
                }
            }
            break;

        // To be used by client in Use Buffer scenerio,
        // for querying padded height and width requiremnt of component,
        // primarily on ouput port.
        case (OMX_INDEXTYPE)OMX_TI_IndexParam2DBufferAllocDimension :

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_CONFIG_RECTTYPE, eError);
            p2DBufferAllocParams = (OMX_CONFIG_RECTTYPE *) pParamStruct;
            if( p2DBufferAllocParams->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                *p2DBufferAllocParams
                    = pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT];
            } else if( p2DBufferAllocParams->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                *p2DBufferAllocParams
                    = pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_INPUT_PORT];
            }
            break;

        case OMX_IndexParamStandardComponentRole :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_COMPONENTROLETYPE, eError);
            strcpy((char *)((OMX_PARAM_COMPONENTROLETYPE *)pParamStruct)->cRole,
                   (char *)pVideoDecoderComPvt->tComponentRole.cRole);
            break;

        case (OMX_INDEXTYPE) OMX_TI_IndexParamTimeStampInDecodeOrder :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_TIMESTAMP_IN_DECODE_ORDER, eError);

            if( pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp == OMX_TRUE ) {
                ((OMX_TI_PARAM_TIMESTAMP_IN_DECODE_ORDER *) pParamStruct)->bEnabled = OMX_TRUE;
            } else {
                ((OMX_TI_PARAM_TIMESTAMP_IN_DECODE_ORDER *) pParamStruct)->bEnabled = OMX_FALSE;
            }
            break;

        case (OMX_INDEXTYPE) OMX_TI_IndexParamSkipGreyOutputFrames :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_SKIP_GREY_OUTPUT_FRAMES, eError);

            if( pVideoDecoderComPvt->bSupportSkipGreyOutputFrames == OMX_TRUE ) {
                ((OMX_TI_PARAM_SKIP_GREY_OUTPUT_FRAMES *) pParamStruct)->bEnabled = OMX_TRUE;
            } else {
                ((OMX_TI_PARAM_SKIP_GREY_OUTPUT_FRAMES *) pParamStruct)->bEnabled = OMX_FALSE;
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
        case (OMX_INDEXTYPE)OMX_TI_IndexParamUseEnhancedPortReconfig :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_ENHANCEDPORTRECONFIG, eError);

            pEnhancedPortReconfig = (OMX_TI_PARAM_ENHANCEDPORTRECONFIG *) pParamStruct;
            if( pEnhancedPortReconfig->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pEnhancedPortReconfig->bUsePortReconfigForCrop = pVideoDecoderComPvt->bUsePortReconfigForCrop;
                pEnhancedPortReconfig->bUsePortReconfigForPadding = pVideoDecoderComPvt->bUsePortReconfigForPadding;
            }
        }
        break;
        case (OMX_INDEXTYPE) OMX_TI_IndexUseBufferDescriptor :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_USEBUFFERDESCRIPTOR, eError);
            ptBufDescParam = (OMX_TI_PARAM_USEBUFFERDESCRIPTOR *) pParamStruct;
            /* GetParameter can be invoked in Loaded State or disabled port */
            OMX_BASE_ASSERT((pVideoDecoderComPvt->tCurState == OMX_StateLoaded) ||
                            (pVideoDecoderComPvt->pPortdefs[ptBufDescParam->nPortIndex]->bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);
            if( ptBufDescParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT || ptBufDescParam->nPortIndex == OMX_VIDEODECODER_INPUT_PORT ) {
                ((OMX_TI_PARAM_USEBUFFERDESCRIPTOR *)pParamStruct)->bEnabled = pVideoDecoderComPvt->tUseBufDesc[ptBufDescParam->nPortIndex].bEnabled;
                ((OMX_TI_PARAM_USEBUFFERDESCRIPTOR *)pParamStruct)->eBufferType = pVideoDecoderComPvt->tUseBufDesc[ptBufDescParam->nPortIndex].eBufferType;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;

        case (OMX_INDEXTYPE) OMX_TI_IndexAndroidNativeBufferUsage:
            pUsage = (OMX_TI_PARAMNATIVEBUFFERUSAGE*)pParamStruct;
            pUsage->nUsage = GRALLOC_USAGE_HW_RENDER;
        break;
        
        default :
            eError = OMX_BASE_GetParameter(hComponent, nIndex, pParamStruct);
    }

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_GetConfig()
 *                     This method is called by the IL Client to get config
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] nIndex      : Index of the config to get
 *  @param [in] pComponentConfigStructure: pointer to the config structure
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_GetConfig(OMX_HANDLETYPE hComponent,
                              OMX_INDEXTYPE nIndex, OMX_PTR pComponentConfigStructure) {
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE              *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE       *pVideoDecoderComPvt = NULL;
    OMX_CONFIG_RECTTYPE            *pCropParams = NULL;
    OMX_CONFIG_SCALEFACTORTYPE     *pScaleParams = NULL;
    OMX_TI_STREAMINTERLACEFORMAT   *pIlaceFmtTypes = NULL;

    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pComponentConfigStructure != NULL), OMX_ErrorBadParameter);

    // Initialize the local variables
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    /* GetConfig can't be invoked incase the comp is in Invalid State  */
    OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState != OMX_StateInvalid,
                    OMX_ErrorIncorrectStateOperation);

    switch( nIndex ) {
        case OMX_IndexConfigCommonOutputCrop :
        {
            OMX_BASE_CHK_VERSION(pComponentConfigStructure, OMX_CONFIG_RECTTYPE, eError);
            pCropParams = (OMX_CONFIG_RECTTYPE *) pComponentConfigStructure;
            if( pCropParams->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pCropParams->nWidth = pVideoDecoderComPvt->tCropDimension.nWidth;
                pCropParams->nHeight = pVideoDecoderComPvt->tCropDimension.nHeight;
                pCropParams->nTop = pVideoDecoderComPvt->tCropDimension.nTop;
                pCropParams->nLeft = pVideoDecoderComPvt->tCropDimension.nLeft;
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexConfigCommonOutputCrop called on o/p port.");

            } else {
                eError = OMX_ErrorBadParameter;
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexConfigCommonOutputCrop called on i/p port. Not Supported.");
            }
        }
        break;
        case OMX_IndexConfigCommonScale :
        {
            OMX_BASE_CHK_VERSION(pComponentConfigStructure, OMX_CONFIG_SCALEFACTORTYPE, eError);
            pScaleParams = (OMX_CONFIG_SCALEFACTORTYPE *) pComponentConfigStructure;
            if( pScaleParams->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pScaleParams->xWidth = pVideoDecoderComPvt->tScaleParams.xWidth;
                pScaleParams->xHeight = pVideoDecoderComPvt->tScaleParams.xHeight;
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexConfigCommonScale called on o/p port.");

            } else {
                eError = OMX_ErrorBadParameter;
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_IndexConfigCommonOutputCrop called on i/p port. Not Supported.");
            }
        }
        break;
        case (OMX_INDEXTYPE)OMX_TI_IndexConfigStreamInterlaceFormats :
        {
            OMX_BASE_CHK_VERSION(pComponentConfigStructure, OMX_TI_STREAMINTERLACEFORMAT, eError);
            pIlaceFmtTypes = (OMX_TI_STREAMINTERLACEFORMAT *) pComponentConfigStructure;
            if( pIlaceFmtTypes->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {
                pIlaceFmtTypes->bInterlaceFormat = pVideoDecoderComPvt->tIlaceFmtTypes.bInterlaceFormat;
                pIlaceFmtTypes->nInterlaceFormats = pVideoDecoderComPvt->tIlaceFmtTypes.nInterlaceFormats;
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_TI_IndexConfigStreamInterlaceFormats called on o/p port.");

            } else {
                eError = OMX_ErrorBadParameter;
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_TI_IndexConfigStreamInterlaceFormats called on i/p port. Not Supported.");
            }
        }
        break;

        default :
            eError = OMX_BASE_GetConfig(hComponent, nIndex, pComponentConfigStructure);
    }

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_SetConfig()
 *                     This method is called by the IL Client to set config
 *                     in the Decoder component
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] nIndex      : Index of the config to set
 *  @param [in] pComponentConfigStructure: pointer to the config structure
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_SetConfig(OMX_HANDLETYPE hComponent,
                              OMX_INDEXTYPE nIndex, OMX_PTR pComponentConfigStructure) {
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE              *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE       *pVideoDecoderComPvt = NULL;
    OMX_TI_STREAMINTERLACEFORMAT   *pIlaceFmtTypes = NULL;


    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pComponentConfigStructure != NULL), OMX_ErrorBadParameter);

    // Initialize the local variables
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    /* SetConfig can't be invoked incase the comp is in Invalid State  */
    OMX_BASE_ASSERT(pVideoDecoderComPvt->tCurState != OMX_StateInvalid,
                    OMX_ErrorIncorrectStateOperation);

    switch( nIndex ) {
        case (OMX_INDEXTYPE)OMX_TI_IndexConfigStreamInterlaceFormats :
        {
            OMX_BASE_CHK_VERSION(pComponentConfigStructure, OMX_TI_STREAMINTERLACEFORMAT, eError);

            pIlaceFmtTypes = (OMX_TI_STREAMINTERLACEFORMAT *) pComponentConfigStructure;

            if( pIlaceFmtTypes->bInterlaceFormat == OMX_TRUE ) {
                OMX_BASE_ASSERT(pIlaceFmtTypes->nInterlaceFormats == OMX_InterlaceFrameTopFieldFirst,
                                OMX_ErrorUnsupportedSetting);
            }

            if( pIlaceFmtTypes->nPortIndex == OMX_VIDEODECODER_OUTPUT_PORT ) {

                pVideoDecoderComPvt->tIlaceFmtTypes.bInterlaceFormat = pIlaceFmtTypes->bInterlaceFormat;
                pVideoDecoderComPvt->tIlaceFmtTypes.nInterlaceFormats = pIlaceFmtTypes->nInterlaceFormats;

                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_TI_IndexConfigStreamInterlaceFormats called on o/p port.");

            } else {
                eError = OMX_ErrorBadParameter;
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "OMX_TI_IndexConfigStreamInterlaceFormats called on i/p port. Not Supported.");
            }
        }
        break;

        default :
            eError = OMX_BASE_SetConfig(hComponent, nIndex, pComponentConfigStructure);
    }

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_CommandNotify()
 *                     This method is called by the base component to notify
 *                     the derived decoder component about a command
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] Cmd         : Command type to be notified
 *  @param [in] nParam      : Command Parameter
 *  @param [in] pCmdData    : pointer to the Command Data
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_CommandNotify(OMX_HANDLETYPE hComponent,
                                                OMX_COMMANDTYPE Cmd, OMX_U32 nParam, OMX_PTR pCmdData)
{
    OMX_ERRORTYPE                          eError = OMX_ErrorNone, eRMError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                     *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE              *pVideoDecoderComPvt = NULL;
    XDAS_Int32                             status;
    OMX_U32                                ReturnCmdCompletionCallBack = 1, i;
    OMX_PTR                                pDecStaticParams;
    OMX_PTR                                pDecDynamicParams;
    OMX_PTR                                pDecStatus;
    OMX_PARAM_PORTDEFINITIONTYPE          *pOutputPortDef;
    OMX_U32                                nFrameWidth, nFrameHeight;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER    sCustomHeader;

    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = pHandle->pComponentPrivate;

    switch( Cmd ) {
        case OMX_CommandStateSet :
            if( pVideoDecoderComPvt->tCurState == OMX_StateLoaded &&
                pVideoDecoderComPvt->tNewState == OMX_StateIdle ) {

                TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                  "\nIN:DRV'd CmdNtfy: Handle = 0x%x  Start transition from L --> I \n", pHandle);

                pOutputPortDef =
                    pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
                if( pOutputPortDef->format.video.nStride <
                    pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth ) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Unsupported Stride set on o/p port defs");
                    eError = OMX_ErrorUnsupportedSetting;
                    goto EXIT;
                }
                // Check the buffer cnt is greater than min required
                // buffer count
                if( pOutputPortDef->nBufferCountActual < pOutputPortDef->nBufferCountMin ) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Number of actual buffers on o/p port less than the minmum required by componenet as set on o/p port defs");
                    eError = OMX_ErrorUnsupportedSetting;
                    goto EXIT;
                }

                sCustomHeader.nMetaDataUsageBitField =
                    OMX_VIDEODECODER_SEIINFO2004FRAME1BIT | OMX_VIDEODECODER_SEIINFO2010FRAME1BIT
                    | OMX_VIDEODECODER_SEIINFO2004FRAME2BIT | OMX_VIDEODECODER_SEIINFO2010FRAME2BIT
                    | OMX_VIDEODECODER_RANGEMAPPINGBIT | OMX_VIDEODECODER_RESCALINGBIT
                    | OMX_VIDEODECODER_CODECEXTERROR1BIT | OMX_VIDEODECODER_CODECEXTERROR2BIT;

                pVideoDecoderComPvt->nMBInfoMaxSize = 0;

                if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                    sCustomHeader.nMetaDataUsageBitField |=
                        OMX_VIDEODECODER_MBINFOFRAME1BIT | OMX_VIDEODECODER_MBINFOFRAME2BIT;

                    pVideoDecoderComPvt->nMBInfoMaxSize = ((pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth *
                                                            pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight) >> 8) + 4350;

                    if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->operatingMode == IVIDEO_TRANSCODE_FRAMELEVEL ) {
                        pVideoDecoderComPvt->nMBInfoMaxSize = pVideoDecoderComPvt->nMBInfoMaxSize + pVideoDecoderComPvt->nMBInfoStructSize *
                                                              ((pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth * pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight) >> 8);
                    }
                    sCustomHeader.tMbInfoFrame1.nSize = pVideoDecoderComPvt->nMBInfoMaxSize;
                    sCustomHeader.tMbInfoFrame2.nSize = pVideoDecoderComPvt->nMBInfoMaxSize;
                }

                if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                    sCustomHeader.nMetaDataUsageBitField |=
                        OMX_VIDEODECODER_SEIINFOFRAME1BIT | OMX_VIDEODECODER_SEIINFOFRAME2BIT;
                    sCustomHeader.tSeiInfoFrame1.nSize = sizeof(IH264VDEC_SeiMessages);
                    sCustomHeader.tSeiInfoFrame2.nSize = sizeof(IH264VDEC_SeiMessages);
                }

                if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {

                    sCustomHeader.nMetaDataUsageBitField |=
                        OMX_VIDEODECODER_VUIINFOFRAME1BIT | OMX_VIDEODECODER_VUIINFOFRAME2BIT;
                    sCustomHeader.tVuiInfoFrame1.nSize = sizeof(IH264VDEC_VuiParams);
                    sCustomHeader.tVuiInfoFrame2.nSize = sizeof(IH264VDEC_VuiParams);
                }
                if( pVideoDecoderComPvt->bErrorConcealmentBuffer ) {
                    pVideoDecoderComPvt->nMBInfoMaxSize = pVideoDecoderComPvt->nMBInfoMaxSize + (pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth *
                                                                                                 pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight);
                }

                pVideoDecoderComPvt->fpDioUtil(hComponent, OMX_VIDEODECODER_OUTPUT_PORT,
                                               OMX_BASE_DIO_UTIL_GetTotalBufferSize, &sCustomHeader);
                // Transitioning from Loaded to Idle state
                if(pVideoDecoderComPvt->pDecHandle == NULL ) {
                    pDecStaticParams
                        = pVideoDecoderComPvt->pDecStructures.pDecStaticParams;
                    // Call the Video Decoder Create Call
                    OMX_BASE_Menu(" 1.Codec Create for %s", pVideoDecoderComPvt->strDecoderName);
                    OMX_BASE_Trace("\n Before codec create \n");
                    pVideoDecoderComPvt->pDecHandle
                        = VIDDEC3_create(pVideoDecoderComPvt->ce,
                                         pVideoDecoderComPvt->strDecoderName,
                                         (VIDDEC3_Params *) pDecStaticParams);
                    OMX_BASE_Trace("\n After codec create \n");
                    if( pVideoDecoderComPvt->pDecHandle == NULL ) {
                        OMX_BASE_Error("VIDDEC3_create failed ....! \n");
                        eError = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                    pVideoDecoderComPvt->nCodecRecreationRequired = 0;
                    pVideoDecoderComPvt->nOutPortReconfigRequired = 0;

                    // Populate Dynamic Params and the Status structures
                    pVideoDecoderComPvt->fpSet_DynamicParams(hComponent,
                                                             pVideoDecoderComPvt->pDecStructures.pDecDynParams);
                    pVideoDecoderComPvt->fpSet_Status(hComponent,
                                                      pVideoDecoderComPvt->pDecStructures.pDecStatus);
                    //This is to handle Arbitrary stride requirement given by IL client.
                    if((pOutputPortDef->format.video.nStride != OMX_VIDDEC_TILER_STRIDE) &&
                       (pOutputPortDef->format.video.nStride >
                        pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth)) {
                        pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->displayWidth =
                            pOutputPortDef->format.video.nStride;
                    }
                    pDecDynamicParams
                        = pVideoDecoderComPvt->pDecStructures.pDecDynParams;
                    pDecStatus
                        = pVideoDecoderComPvt->pDecStructures.pDecStatus;

                    // Call the Decoder Control function
                    eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_SETPARAMS,
                                                          pDecDynamicParams, pDecStatus, hComponent, &status);
                    if( status != VIDDEC3_EOK ) {
                        OMX_BASE_Error("VIDDEC3_control XDM_SETPARAMS failed! \n");
                        eError = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

                    // Call the Decoder Control function
                    eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_GETBUFINFO,
                                                          pDecDynamicParams, pDecStatus, hComponent, &status);
                    if( status != VIDDEC3_EOK ) {
                        OMX_BASE_Error("VIDDEC3_control XDM_GETBUFINFO failed! \n");
                        eError = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

                    // Allocate input buffer header list in case input data mode is slice mode
                    if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_SLICEMODE ) {
                        pVideoDecoderComPvt->bIsFlushRequired = OMX_FALSE;

                        for( i=0; i < OMX_VIDDEC_DATASYNC_DEFAULT_IN_BUF_COUNT; i++ ) {
                            pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].pInBufHeader =
                                (OMX_BUFFERHEADERTYPE *) TIMM_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE), TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
                            pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].bEntryUsedFlag = OMX_FALSE;
                        }
                    }

                }
            } else if( pVideoDecoderComPvt->tCurState == OMX_StateIdle &&
                       pVideoDecoderComPvt->tNewState == OMX_StateLoaded ) {
                // Transitioning from Idle to Loaded
                TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                  "\nIN:DRV'd CmdNtfy: Handle = 0x%x Start transition from I --> L \n", pHandle);
                if( pVideoDecoderComPvt->pDecHandle != NULL ) {
                    // Delete the Decoder Component Private Handle
                    pVideoDecoderComPvt->nFrameCounter = 0;
                    if( pVideoDecoderComPvt->bSupportSkipGreyOutputFrames ) {
                        pVideoDecoderComPvt->bSyncFrameReady = OMX_FALSE;
                    }
                    if( pVideoDecoderComPvt->bErrorConcealmentBuffer && !pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                        RELEASE_MBINFO(pVideoDecoderComPvt)
                    }
                    pVideoDecoderComPvt->nOutbufInUseFlag = 0;
                    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "\n Before codec delete \n");
                    VIDDEC3_delete(pVideoDecoderComPvt->pDecHandle);
                    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "\n After codec delete \n");
                    pVideoDecoderComPvt->pDecHandle = NULL;
                }

                //Freeing the input buffer header list in case of input data mode is slice mode
                if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_SLICEMODE ) {
                    for( i=0; i < OMX_VIDDEC_DATASYNC_DEFAULT_IN_BUF_COUNT; i++ ) {
                        if( pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].pInBufHeader ) {
                            TIMM_OSAL_Free(pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].pInBufHeader);
                        }
                    }
                }

            } else if(((pVideoDecoderComPvt->tCurState == OMX_StateIdle) && (pVideoDecoderComPvt->tNewState == OMX_StateExecuting)) ||
                      ((pVideoDecoderComPvt->tCurState == OMX_StateIdle) && (pVideoDecoderComPvt->tNewState == OMX_StatePause))) {
                TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                  "\nIN:DRV'd CmdNtfy: Handle = 0x%x Start transition from I --> E \n", pHandle);
                if( pVideoDecoderComPvt->tNewState == OMX_StatePause ) {

                } else {

                }
            }
            /* State transition from pause to executing state */
            else if((pVideoDecoderComPvt->tCurState == OMX_StatePause) &&
                    (pVideoDecoderComPvt->tNewState == OMX_StateExecuting)) {

            } else if((pVideoDecoderComPvt->tCurState == OMX_StateExecuting) && (pVideoDecoderComPvt->tNewState == OMX_StatePause)) {

            } else if(((pVideoDecoderComPvt->tCurState == OMX_StateExecuting) &&
                       (pVideoDecoderComPvt->tNewState == OMX_StateIdle)) ||
                      ((pVideoDecoderComPvt->tCurState == OMX_StatePause) &&
                       (pVideoDecoderComPvt->tNewState == OMX_StateIdle))) {
                // Transitioning from Execute or Pause state to Idle state

                //Call codec flush and free allocated buffer headers
                TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                  "\nIN:DRV'd CmdNtfy: Handle = 0x%x Start transition from E -->P or I\n", pHandle);
                OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, NULL, NULL);


            } else if( pVideoDecoderComPvt->tNewState == OMX_StateInvalid ) {

            }
            break;

        case OMX_CommandPortDisable :
            // In case of Port Disable Command
            // Loaded state implies codec is deleted. so no need to delete again
            if( pVideoDecoderComPvt->tCurState != OMX_StateLoaded
                && pVideoDecoderComPvt->pDecHandle != NULL ) {
                // Call Decoder Flush function
                OMX_BASE_Info("{CompThread} : CmdNotify PortDisabling....");
                OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, NULL, NULL);
            }
            break;

        case OMX_CommandPortEnable :
            // In case of Port Enable Command
            //ToDo:Is this neede for both ports?
            if( nParam == OMX_VIDEODECODER_OUTPUT_PORT ) {
                // Check the buffer cnt is greater than min required
                // buffer count
                pOutputPortDef =
                    pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
                if( pOutputPortDef->nBufferCountActual < pOutputPortDef->nBufferCountMin ) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Number of actual buffers on o/p port less than the minmum required by componenet as set on o/p port defs");
                    eError = OMX_ErrorUnsupportedSetting;
                    goto EXIT;
                }
                if( pOutputPortDef->format.video.nStride <
                    pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth ) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Unsupported Stride set on o/p port defs");
                    eError = OMX_ErrorUnsupportedSetting;
                    goto EXIT;
                }
            }
            OMX_BASE_Info("{CompThread} : CmdNotify PortEnabling... on %d ", nParam);
            if( pVideoDecoderComPvt->tCurState != OMX_StateLoaded ) {
                if( pVideoDecoderComPvt->nCodecRecreationRequired == 1
                    && pVideoDecoderComPvt->pDecHandle != NULL ) {
                    pVideoDecoderComPvt->nCodecRecreationRequired =  0;
                    pVideoDecoderComPvt->nFrameCounter = 0;

                    pVideoDecoderComPvt->tScaleParams.xWidth = 0x10000;
                    pVideoDecoderComPvt->tScaleParams.xHeight = 0x10000;

                    if( pVideoDecoderComPvt->bSupportSkipGreyOutputFrames ) {
                        pVideoDecoderComPvt->bSyncFrameReady = OMX_FALSE;
                    }
                    if( pVideoDecoderComPvt->bErrorConcealmentBuffer && !pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                        RELEASE_MBINFO(pVideoDecoderComPvt)
                    }
                    pVideoDecoderComPvt->nOutbufInUseFlag = 0;

                    VIDDEC3_delete(pVideoDecoderComPvt->pDecHandle);
                    pVideoDecoderComPvt->pDecHandle = NULL;

                    pOutputPortDef =
                        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
                    // Set the Static Params
                    pVideoDecoderComPvt->fpSet_StaticParams(hComponent,
                                                            pVideoDecoderComPvt->pDecStructures.pDecStaticParams);
                    if( pVideoDecoderComPvt->bUsePortReconfigForPadding == OMX_TRUE
                        && pVideoDecoderComPvt->pBaseClassStructers.pDecStatus->outputWidth != 0
                        && pVideoDecoderComPvt->pBaseClassStructers.pDecStatus->outputHeight != 0 ) {
                        nFrameWidth =  pVideoDecoderComPvt->pBaseClassStructers.pDecStatus->outputWidth;
                        nFrameHeight =  pVideoDecoderComPvt->pBaseClassStructers.pDecStatus->outputHeight;
                    } else {
                        nFrameWidth = pOutputPortDef->format.video.nFrameWidth;
                        nFrameHeight = pOutputPortDef->format.video.nFrameHeight;
                    }
                    if( nFrameWidth & 0x0F ) {

                        nFrameWidth = nFrameWidth + 16 - (nFrameWidth & 0x0F);

                    }
                    if( nFrameHeight & 0x1F ) {
                        nFrameHeight = nFrameHeight + 32 - (nFrameHeight & 0x1F);
                    }
                    pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxHeight =
                        nFrameHeight;
                    pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->maxWidth =
                        nFrameWidth;

                    // And Call the Codec Create
                    OMX_BASE_Menu("2.Codec Create for %s", pVideoDecoderComPvt->strDecoderName);
                    pVideoDecoderComPvt->pDecHandle
                        = VIDDEC3_create(pVideoDecoderComPvt->ce,
                                         pVideoDecoderComPvt->strDecoderName,
                                         (VIDDEC3_Params *)
                                         pVideoDecoderComPvt->pDecStructures.pDecStaticParams);
                    if( pVideoDecoderComPvt->pDecHandle == NULL ) {
                        OMX_BASE_Error("VIDDEC3_create failed ....! \n");
                        eError = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                    // Populate Dynamic Params and the Status structures
                    pVideoDecoderComPvt->fpSet_DynamicParams(hComponent,
                                                             pVideoDecoderComPvt->pDecStructures.pDecDynParams);
                    pVideoDecoderComPvt->fpSet_Status(hComponent,
                                                      pVideoDecoderComPvt->pDecStructures.pDecStatus);
                    //This is to handle Arbitrary stride requirement given by IL client.
                    if((pOutputPortDef->format.video.nStride != OMX_VIDDEC_TILER_STRIDE) &&
                       (pOutputPortDef->format.video.nStride !=
                        pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth)) {
                        pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->displayWidth =
                            pOutputPortDef->format.video.nStride;
                    }
                    pDecDynamicParams
                        = pVideoDecoderComPvt->pDecStructures.pDecDynParams;
                    pDecStatus
                        = pVideoDecoderComPvt->pDecStructures.pDecStatus;

                    // Call the Decoder Control function
                    eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_SETPARAMS,
                                                          pDecDynamicParams, pDecStatus, hComponent, &status);
                    if( status != VIDDEC3_EOK ) {
                        OMX_BASE_Error("VIDDEC3_control XDM_SETPARAMS failed! \n");
                        eError = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
                }
                pVideoDecoderComPvt->nOutPortReconfigRequired = 0;

            }
            if( pVideoDecoderComPvt->bUsePortReconfigForCrop == OMX_TRUE ) {
                eError = pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                                  OMX_EventPortSettingsChanged, OMX_VIDEODECODER_OUTPUT_PORT, OMX_IndexConfigCommonOutputCrop, NULL);
                if( eError != OMX_ErrorNone ) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Port reconfig callback returned error, trying to continue");
                }
            }
            break;

        case OMX_CommandFlush :
        {
            OMX_BASE_Info("{compThread} CmdNotify:FLUSH: %d ", nParam);
            // In case of Flush Command
            // In case all ports have to be flushed or the output port
            // has to be flushed
            if( nParam == OMX_ALL || nParam == OMX_VIDEODECODER_OUTPUT_PORT ) {
                // Only then flush the port
                TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "\nIN:DRV'd CmdNtfy: Handle = 0x%x FLUSH called\n", pHandle);
                OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, NULL, NULL);
            }
        }
        break;

        case OMX_CommandMarkBuffer :
            pVideoDecoderComPvt->pMark = (OMX_MARKTYPE *)pCmdData;
            ReturnCmdCompletionCallBack = 0;
            break;
    }

    if( ReturnCmdCompletionCallBack == 1 ) {
        eError = pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                          OMX_EventCmdComplete, Cmd, nParam, NULL);

        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    }
EXIT:

    return (eError);

}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_DataNotify()
 *                     This method is called by the base component to notify
 *                     the derived decoder component about Data
 *
 *  @param [in] hComponent  : pointer to the OMX Component.
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_DataNotify(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE                          eError = OMX_ErrorNone, eRMError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                     *pHandle = NULL;
    OMX_BUFFERHEADERTYPE                   sInBufHeader;
    OMX_VIDEODECODER_PVTTYPE              *pVideoDecoderComPvt = NULL;
    OMX_BUFFERHEADERTYPE                  *pInBufHeader = &sInBufHeader;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pOutBufHeader = NULL;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pDupBufHeader = NULL;
    OMX_VIDEODECODER_CUSTOMBUFFERHEADER   *pFreeBufHeader = NULL;
    OMX_U32                                nInMsgCount = 0;
    OMX_U32                                nOutMsgCount = 0;
    OMX_U32                                Buffer_locked = 1;
    OMX_U32                                ii = 0, i;
    OMX_U32                                nStride = OMX_VIDDEC_TILER_STRIDE;
    OMX_U32                                nLumaFilledLen, nChromaFilledLen;
    OMX_PARAM_PORTDEFINITIONTYPE          *pOutputPortDef = NULL;
    OMX_BOOL                               nIsDioReady = OMX_FALSE;

#ifdef VIDEODECODER_TILERTEST
    OMX_PTR    pUVBuffer = NULL;
#endif

    XDAS_Int32             status;
    XDM2_BufDesc          *pInBufDescPtr = NULL;
    XDM2_BufDesc          *pOutBufDescPtr = NULL;
    OMX_CONFIG_RECTTYPE    out2DAllocParam;
    OMX_U32                nOutbufferSize = 0;
    BUFF_POINTERS         *pDecExtStructure = NULL;
    IVIDDEC3_OutArgs      *pDecOutArgs = NULL;
    IVIDDEC3_Status       *pDecStatus = NULL;
    XDM_Rect               activeFrameRegion[2];
    OMX_U32                nNewInBufferRequired = 0;
    OMX_PTR                pSEIinfo = NULL, pClientMetaData = NULL, pCodecMetaData = NULL;
    OMX_PTR                pVUIinfo = NULL;
    OMX_METADATA          *pOMXmeta = NULL, *pOMXmetaFrame1 = NULL, *pOMXmetaFrame2 = NULL;
    OMX_BOOL               bSendPortReconfigForScale = OMX_FALSE;
    OMX_U32                nScale, nScaleRem, nScaleQ16Low, nScaleWidth, nScaleHeight;
    OMX_U64                nScaleQ16 = 0;
#if (DUCATI_VIDEO_PERF_MEASUREMENT == 1)
    uint64    clkTickBef=0, clkTickAfter=0;
    uint32    clkTickDiff=0;
#endif
    TIMM_OSAL_U32                       nActualSize;

    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);

    //! Initialize pointers
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;


    OMX_BASE_REQUIRE(pVideoDecoderComPvt != NULL, OMX_ErrorBadParameter);

    pOutputPortDef =
        pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT];
    out2DAllocParam =
        pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT];

    /*! Ensure that the stride on output portdef structure is more than
        the padded width. This is needed in the case where application
        sets the Stride less than padded width */
    if( pOutputPortDef->format.video.nStride >=
        pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth ) {
        nStride = pOutputPortDef->format.video.nStride;
    } else {
        nStride = pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth;
    }
    if((pVideoDecoderComPvt->tCurState != OMX_StateExecuting) ||
       (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->bEnabled == OMX_FALSE) ||
       (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->bEnabled == OMX_FALSE)) {
        goto EXIT;
    }
    nIsDioReady = OMX_BASE_IsDioReady(hComponent, OMX_VIDEODECODER_OUTPUT_PORT);
    if( nIsDioReady == OMX_FALSE ) {
        goto EXIT;
    }
    //! Get the number of input and output buffers
    pVideoDecoderComPvt->fpDioGetCount(hComponent,
                                       OMX_VIDEODECODER_INPUT_PORT,
                                       (OMX_PTR)&nInMsgCount);
    pVideoDecoderComPvt->fpDioGetCount(hComponent,
                                       OMX_VIDEODECODER_OUTPUT_PORT,
                                       (OMX_PTR)&nOutMsgCount);
    TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                      "{CompThread} DataNotify: # of IN buff = %d # of OUT buff = %d", nInMsgCount, nOutMsgCount);

    // Loop until input or output buffers are exhausted
    while((nInMsgCount > 0) && (nOutMsgCount > 0)) {
        // Only if Cur-State is Execute proceed.
        if((pVideoDecoderComPvt->tCurState != OMX_StateExecuting) ||
           (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_OUTPUT_PORT]->bEnabled == OMX_FALSE) ||
           (pVideoDecoderComPvt->pPortdefs[OMX_VIDEODECODER_INPUT_PORT]->bEnabled == OMX_FALSE)) {
            break;
        }
        if( OMX_BASE_IsCmdPending(hComponent) && pVideoDecoderComPvt->nOutbufInUseFlag == 0 ) {
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Exiting dataNotify because command is pending");
            goto EXIT;
        }

        if( pVideoDecoderComPvt->nOutPortReconfigRequired == 1 ) {
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Port disable/reconfiguration needed");
            //Add error here if needed.
            goto EXIT;
        }
        if( pVideoDecoderComPvt->nFatalErrorGiven == 1 ) {
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Fatal error given to IL client");
            //Add error here if needed.
            goto EXIT;
        }
        // Buffer is locked by the codec by default.
        // It can be freed only if mentioned in freeBufId[] field.
        Buffer_locked = 1;
        ii=0;
        if( nNewInBufferRequired == 0 && pVideoDecoderComPvt->pAttrDesc == NULL ) {
            //! Get Input and Output Buffer header from Queue
            if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_SLICEMODE ) {
                pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[0].bEntryUsedFlag = OMX_TRUE;
                pInBufHeader = pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[0].pInBufHeader;
            }
            eError = pVideoDecoderComPvt->fpDioDequeue(hComponent, OMX_VIDEODECODER_INPUT_PORT,
                                                       (OMX_PTR)(pInBufHeader));
            TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                              "This BuffHdr =0x%x De-qued on INPORT for processing", pInBufHeader);
            if( eError == OMX_TI_WarningAttributePending ) {

                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Codec Config data Recieved");
                pVideoDecoderComPvt->pAttrDesc =
                    TIMM_OSAL_MallocExtn(sizeof(OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR),
                                         TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT, pVideoDecoderComPvt->tDerToBase.hDefaultHeap);
                OMX_BASE_ASSERT(pVideoDecoderComPvt->pAttrDesc != NULL, OMX_ErrorInsufficientResources);

                pVideoDecoderComPvt->pAttrDesc->pAttributeData =
                    TIMM_OSAL_MallocExtn(2, TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT,
                                         pVideoDecoderComPvt->tDerToBase.hDefaultHeap);
                OMX_BASE_ASSERT(pVideoDecoderComPvt->pAttrDesc->pAttributeData != NULL, OMX_ErrorInsufficientResources);
                pVideoDecoderComPvt->pAttrDesc->nSize = 2;

                eError = pVideoDecoderComPvt->fpDioControl(hComponent, OMX_VIDEODECODER_INPUT_PORT,
                                                           OMX_BASE_DIO_CTRLCMD_GetCtrlAttribute, (OMX_PTR)pVideoDecoderComPvt->pAttrDesc);
                if( eError == OMX_TI_WarningInsufficientAttributeSize ) {
                    /*Allocate the data pointer again with correct size*/
                    TIMM_OSAL_Free(pVideoDecoderComPvt->pAttrDesc->pAttributeData);

                    //Change this to tiler-1D allocation.

                    pVideoDecoderComPvt->pAttrDesc->pAttributeData =
                        TIMM_OSAL_MallocExtn(pVideoDecoderComPvt->pAttrDesc->nSize,
                                             TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT, pVideoDecoderComPvt->tDerToBase.hDefaultHeap);

                    OMX_BASE_ASSERT(pVideoDecoderComPvt->pAttrDesc->pAttributeData != NULL, OMX_ErrorInsufficientResources);
                    eError = pVideoDecoderComPvt->fpDioControl(hComponent, OMX_VIDEODECODER_INPUT_PORT,
                                                               OMX_BASE_DIO_CTRLCMD_GetCtrlAttribute, (OMX_PTR)pVideoDecoderComPvt->pAttrDesc);
                    if( eError != OMX_ErrorNone ) {
                        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                           "Codec config test failed because DIO Control returned buffer");
                        goto EXIT;
                    }
                }

                /*Do cache writeback*/
                //Cache_wb(pVideoDecoderComPvt->pAttrDesc->pAttributeData, pVideoDecoderComPvt->pAttrDesc->nSize, Cache_Type_ALL, TRUE);
                pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader  = pVideoDecoderComPvt->parseHeaderFlag;
                pInBufHeader = NULL;

    #ifndef CODEC_CONFIG_WORKAROUND
                // Call the Decoder Control function
                eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_SETPARAMS,
                                                      pVideoDecoderComPvt->pDecStructures.pDecDynParams, pVideoDecoderComPvt->pDecStructures.pDecStatus,
                                                      hComponent, &status);
                if( status != VIDDEC3_EOK ) {
                    OMX_BASE_Error("VIDDEC3_control XDM_SETPARAMS failed ....! \n");
                    eError = OMX_ErrorInsufficientResources;
                    goto EXIT;
                }
                OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    #endif

            } else if( pVideoDecoderComPvt->nOutbufInUseFlag == 0 ) {
                if( pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader != XDM_DECODE_AU ) {
                    pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader = XDM_DECODE_AU;
    #ifndef CODEC_CONFIG_WORKAROUND
                    eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_SETPARAMS,
                                                          pVideoDecoderComPvt->pDecStructures.pDecDynParams, pVideoDecoderComPvt->pDecStructures.pDecStatus,
                                                          hComponent, &status);
                    if( status != VIDDEC3_EOK ) {
                        OMX_BASE_Error("VIDDEC3_control XDM_SETPARAMS failed ....! \n");
                        eError = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    #else
                    if( pVideoDecoderComPvt->pAttrDesc ) {
                        if((pInBufHeader->nAllocLen) <
                           (pInBufHeader->nFilledLen + pVideoDecoderComPvt->pAttrDesc->nSize)) {
                            TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                               "Buffer not big enough to hold codec config and frame data");
                            eError = OMX_ErrorBadParameter;
                            goto EXIT;
                        }
                        memmove(pInBufHeader->pBuffer + pVideoDecoderComPvt->pAttrDesc->nSize,
                                pInBufHeader->pBuffer, pInBufHeader->nFilledLen);
                        TIMM_OSAL_Memcpy(pInBufHeader->pBuffer, pVideoDecoderComPvt->pAttrDesc->pAttributeData,
                                         pVideoDecoderComPvt->pAttrDesc->nSize);
                        pInBufHeader->nFilledLen = pInBufHeader->nFilledLen +
                                                   pVideoDecoderComPvt->pAttrDesc->nSize;
                        if( pVideoDecoderComPvt->pAttrDesc ) {
                            if( pVideoDecoderComPvt->pAttrDesc->pAttributeData ) {
                                TIMM_OSAL_Free(pVideoDecoderComPvt->pAttrDesc->pAttributeData);
                                pVideoDecoderComPvt->pAttrDesc->pAttributeData = NULL;
                            }
                            TIMM_OSAL_Free(pVideoDecoderComPvt->pAttrDesc);
                            pVideoDecoderComPvt->pAttrDesc = NULL;
                        }
                    }
    #endif
                }
                if( pInBufHeader ) {
                    if( !(pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS) && (pInBufHeader->nFilledLen - pInBufHeader->nOffset == 0)) {
                        // Send Input buffer back to base
                        pVideoDecoderComPvt->fpDioSend(hComponent,
                                                       OMX_VIDEODECODER_INPUT_PORT,
                                                       pInBufHeader);

                        //! Get the number of input and output buffers
                        pVideoDecoderComPvt->fpDioGetCount(hComponent,
                                                           OMX_VIDEODECODER_INPUT_PORT,
                                                           (OMX_PTR)&nInMsgCount);
                        if( nInMsgCount ) {
                            continue;
                        } else {
                            break;
                        }
                    }
                }
                //! Allocate Memory for output buffer header
                pOutBufHeader = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *) TIMM_OSAL_Malloc(sizeof(OMX_VIDEODECODER_CUSTOMBUFFERHEADER),
                                                                                         TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
                if( pOutBufHeader == TIMM_OSAL_NULL ) {
                    eError = OMX_ErrorInsufficientResources;
                    goto EXIT;
                }

                pOutBufHeader->nMetaDataUsageBitField =
                    OMX_VIDEODECODER_SEIINFO2004FRAME1BIT | OMX_VIDEODECODER_SEIINFO2010FRAME1BIT
                    | OMX_VIDEODECODER_SEIINFO2004FRAME2BIT | OMX_VIDEODECODER_SEIINFO2010FRAME2BIT
                    | OMX_VIDEODECODER_RANGEMAPPINGBIT | OMX_VIDEODECODER_RESCALINGBIT
                    | OMX_VIDEODECODER_CODECEXTERROR1BIT | OMX_VIDEODECODER_CODECEXTERROR2BIT;

                if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                    pOutBufHeader->nMetaDataUsageBitField |=
                        OMX_VIDEODECODER_MBINFOFRAME1BIT | OMX_VIDEODECODER_MBINFOFRAME2BIT;
                    pOutBufHeader->tMbInfoFrame1.nSize = pVideoDecoderComPvt->nMBInfoMaxSize;
                    pOutBufHeader->tMbInfoFrame2.nSize = pVideoDecoderComPvt->nMBInfoMaxSize;
                }

                if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                    pOutBufHeader->nMetaDataUsageBitField |=
                        OMX_VIDEODECODER_SEIINFOFRAME1BIT | OMX_VIDEODECODER_SEIINFOFRAME2BIT;
                    pOutBufHeader->tSeiInfoFrame1.nSize = sizeof(IH264VDEC_SeiMessages);
                    pOutBufHeader->tSeiInfoFrame2.nSize = sizeof(IH264VDEC_SeiMessages);
                }

                if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {

                    pOutBufHeader->nMetaDataUsageBitField |=
                        OMX_VIDEODECODER_VUIINFOFRAME1BIT | OMX_VIDEODECODER_VUIINFOFRAME2BIT;
                    pOutBufHeader->tVuiInfoFrame1.nSize = sizeof(IH264VDEC_VuiParams);
                    pOutBufHeader->tVuiInfoFrame2.nSize = sizeof(IH264VDEC_VuiParams);
                }

                eError = pVideoDecoderComPvt->fpDioDequeue(hComponent, OMX_VIDEODECODER_OUTPUT_PORT,
                                                           (OMX_PTR)(pOutBufHeader));
            }
        }
        if( pVideoDecoderComPvt->nFrameCounter == 0 ) {
            // Call the Decoder Control function
            eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_GETBUFINFO,
                                                  pVideoDecoderComPvt->pDecStructures.pDecDynParams, pVideoDecoderComPvt->pDecStructures.pDecStatus,
                                                  hComponent, &status);
            if( status != VIDDEC3_EOK ) {
                OMX_BASE_Error("VIDDEC3_control XDM_GETBUFINFO failed! \n");
                eError = OMX_ErrorInsufficientResources;
                goto EXIT;
            }
            OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
            pDecStatus =    (IVIDDEC3_Status *)
                         (pVideoDecoderComPvt->pDecStructures.pDecStatus);
            if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                if( pDecStatus->bufInfo.minOutBufSize[pVideoDecoderComPvt->nSEIInfoIndex + 2].bytes != sizeof(IH264VDEC_SeiMessages)) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Metadata buffer size not as expected %d vs %d", pDecStatus->bufInfo.minOutBufSize[pVideoDecoderComPvt->nSEIInfoIndex + 2].bytes, sizeof(IH264VDEC_SeiMessages));
                }
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
                if( pDecStatus->bufInfo.minOutBufSize[pVideoDecoderComPvt->nVUIInfoIndex + 2].bytes != sizeof(IH264VDEC_VuiParams)) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Metadata buffer size not as expected for VUI %d vs %d", pDecStatus->bufInfo.minOutBufSize[pVideoDecoderComPvt->nSEIInfoIndex + 2].bytes, sizeof(IH264VDEC_SeiMessages));
                }
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo || pVideoDecoderComPvt->bErrorConcealmentBuffer ) {
                pVideoDecoderComPvt->nMBinfoSize = (pDecStatus->bufInfo.minOutBufSize[pVideoDecoderComPvt->nMBInfoIndex + 2].bytes + 3) & 0xfffffffc;
                /* Align the mbinfo size to 4 bytes */
            }
            if( pVideoDecoderComPvt->bErrorConcealmentBuffer && !pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                RELEASE_MBINFO(pVideoDecoderComPvt)
                pVideoDecoderComPvt->pMBinfo = (XDAS_Int8 *) TIMM_OSAL_Malloc(pVideoDecoderComPvt->nMBinfoSize, TIMM_OSAL_TRUE, 128, TIMMOSAL_MEM_SEGMENT_EXT);
                if( pVideoDecoderComPvt->pMBinfo == TIMM_OSAL_NULL ) {
                    pVideoDecoderComPvt->nMBinfoSize = 0;
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Memory allocation of meta data buffer for MBInfo is failed");
                    eError = OMX_ErrorInsufficientResources;
                    goto EXIT;
                }
            }
        }

        if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {

            pVideoDecoderComPvt->pMBinfo = (XDAS_Int8 *) TIMM_OSAL_Malloc(pVideoDecoderComPvt->nMBinfoSize, TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);

            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "MBInfo Max: %d, MBinfo Requested: %d", pVideoDecoderComPvt->nMBInfoMaxSize, pVideoDecoderComPvt->nMBinfoSize);

            if( pVideoDecoderComPvt->pMBinfo == TIMM_OSAL_NULL ) {
                pVideoDecoderComPvt->nMBinfoSize = 0;
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Memory allocation of meta data buffer for MBInfo is failed %d", pVideoDecoderComPvt->nMBinfoSize);
                eError = OMX_ErrorInsufficientResources;
                goto EXIT;
            }
        }

        if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
            pSEIinfo = (IH264VDEC_SeiMessages *) TIMM_OSAL_Malloc(sizeof(IH264VDEC_SeiMessages),
                                                                  TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
        }
        if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
            pVUIinfo = (IH264VDEC_VuiParams *) TIMM_OSAL_Malloc(sizeof(IH264VDEC_VuiParams),
                                                                TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
            TIMM_OSAL_Memset(pVUIinfo, 0, sizeof(IH264VDEC_VuiParams));
        }
        pOutBufDescPtr = &(pVideoDecoderComPvt->tOutBufDesc);
        pInBufDescPtr = &(pVideoDecoderComPvt->tInBufDesc);
        if( pVideoDecoderComPvt->pAttrDesc != NULL &&
            pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader != pVideoDecoderComPvt->parseHeaderFlag ) {
            pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader = pVideoDecoderComPvt->parseHeaderFlag;

            // Call the Decoder Control function
            eError    = _OMX_VIDEODEC_VISACONTROL(pVideoDecoderComPvt->pDecHandle, XDM_SETPARAMS,
                                                  pVideoDecoderComPvt->pDecStructures.pDecDynParams, pVideoDecoderComPvt->pDecStructures.pDecStatus,
                                                  hComponent, &status);
            if( status != VIDDEC3_EOK ) {
                OMX_BASE_Error("VIDDEC3_control XDM_SETPARAMS failed ....! \n");
                eError = OMX_ErrorInsufficientResources;
                goto EXIT;
            }
            OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
        }
        pVideoDecoderComPvt->tInBufDesc.numBufs = 1;
        if( pInBufHeader != NULL && pVideoDecoderComPvt->nOutbufInUseFlag == 1 ) {
            pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->numBytes
                = pInBufHeader->nFilledLen - pInBufHeader->nOffset;
            //Sending the same input ID for second field
            pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID = (OMX_S32) pOutBufHeader;

            if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_ENTIREFRAME ) {
                pInBufDescPtr->descs[0].buf
                    =    (XDAS_Int8 *)((XDAS_Int32)pInBufHeader->pBuffer +
                                       pInBufHeader->nOffset);
                pInBufDescPtr->descs[0].memType = XDM_MEMTYPE_RAW;
                pInBufDescPtr->descs[0].bufSize.bytes = pInBufHeader->nFilledLen;
            } else {
                pVideoDecoderComPvt->tInBufDesc.descs[0].buf
                    =    (XDAS_Int8 *)((XDAS_Int32)pInBufHeader->pBuffer +
                                       pInBufHeader->nOffset);
                pVideoDecoderComPvt->tInBufDesc.descs[0].memType = XDM_MEMTYPE_RAW;
                pVideoDecoderComPvt->tInBufDesc.descs[0].bufSize.bytes = pInBufHeader->nFilledLen;
                pVideoDecoderComPvt->bIsFirstDataSyncCall = OMX_TRUE;
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Slice mode enqueue i/p buffer header=%x buffer=%x\n", pInBufHeader, pVideoDecoderComPvt->tInBufDesc.descs[0].buf);
            }

            pOutBufDescPtr->numBufs = 2;
            if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                pOutBufDescPtr->numBufs += 1;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nSEIInfoIndex + 2].bufSize.bytes = sizeof(IH264VDEC_SeiMessages);
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nSEIInfoIndex + 2].buf = pSEIinfo;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nSEIInfoIndex + 2].memType = XDM_MEMTYPE_RAW;
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo || pVideoDecoderComPvt->bErrorConcealmentBuffer ) {
                pOutBufDescPtr->numBufs += 1;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nMBInfoIndex + 2].bufSize.bytes = pVideoDecoderComPvt->nMBinfoSize;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nMBInfoIndex + 2].buf = pVideoDecoderComPvt->pMBinfo;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nMBInfoIndex + 2].memType = XDM_MEMTYPE_RAW;
                pOutBufDescPtr->descs[1].accessMask = XDM_ACCESSMODE_READ;
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
                pOutBufDescPtr->numBufs += 1;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nVUIInfoIndex + 2].bufSize.bytes = sizeof(IH264VDEC_VuiParams);
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nVUIInfoIndex + 2].buf = pVUIinfo;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nVUIInfoIndex + 2].memType = XDM_MEMTYPE_RAW;
            }
        } else if( pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader == pVideoDecoderComPvt->parseHeaderFlag ) {
#ifdef CODEC_CONFIG_WORKAROUND
            goto EXIT;
#else
            pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->numBytes =
                pVideoDecoderComPvt->pAttrDesc->nSize;
            pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID = 1;
            pVideoDecoderComPvt->tInBufDesc.descs[0].buf =
                pVideoDecoderComPvt->pAttrDesc->pAttributeData;
            pVideoDecoderComPvt->tInBufDesc.descs[0].memType = XDM_MEMTYPE_RAW;
            pVideoDecoderComPvt->tInBufDesc.descs[0].bufSize.bytes =
                pVideoDecoderComPvt->pAttrDesc->nSize;
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Codec config data size provided to codec is : %d",
                               pVideoDecoderComPvt->pAttrDesc->nSize);
            pOutBufDescPtr->numBufs = 2;
            if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
                pOutBufDescPtr->numBufs++;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nSEIInfoIndex + 2].bufSize.bytes = sizeof(IH264VDEC_SeiMessages);
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nSEIInfoIndex + 2].buf = pSEIinfo;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nSEIInfoIndex + 2].memType = XDM_MEMTYPE_RAW;
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo || pVideoDecoderComPvt->bErrorConcealmentBuffer ) {
                pOutBufDescPtr->numBufs++;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nMBInfoIndex + 2].bufSize.bytes = pVideoDecoderComPvt->nMBinfoSize;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nMBInfoIndex + 2].buf = pVideoDecoderComPvt->pMBinfo;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nMBInfoIndex + 2].memType = XDM_MEMTYPE_RAW;
                pOutBufDescPtr->descs[1].accessMask = XDM_ACCESSMODE_READ;
            }
            if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
                pOutBufDescPtr->numBufs++;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nVUIInfoIndex + 2].bufSize.bytes = sizeof(IH264VDEC_VuiParams);
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nVUIInfoIndex + 2].buf = pVUIinfo;
                pOutBufDescPtr->descs[pVideoDecoderComPvt->nVUIInfoIndex + 2].memType = XDM_MEMTYPE_RAW;
            }
#endif
        } else if( pInBufHeader != NULL && pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader
                   == XDM_DECODE_AU ) {
            // In case EOS and Number of Input bytes=0. Flush Decoder and exit
            if( pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS ) {
                if( pInBufHeader->nFilledLen == 0 ) {
                    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "\n EOS with 0 filled length\n");
                    pOutBufHeader->nFilledLen = 0;
                    pOutBufHeader->nMetaDataUsageBitField = 0;
                    eError = OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent,
                                                                      pOutBufHeader, pInBufHeader);
                    if( pSEIinfo ) {
                        TIMM_OSAL_Free(pSEIinfo);
                        pSEIinfo = NULL;
                    }
                    if( pVUIinfo ) {
                        TIMM_OSAL_Free(pVUIinfo);
                        pVUIinfo = NULL;
                    }
                    if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                        RELEASE_MBINFO(pVideoDecoderComPvt);
                    }
                    goto EXIT;
                } else if( pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->displayDelay
                           == IVIDDEC3_DECODE_ORDER ) {
                    pOutBufHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                }
            }

            if( pVideoDecoderComPvt->bUsePortReconfigForPadding == OMX_TRUE ) {
                if( pOutputPortDef->format.video.nFrameWidth < out2DAllocParam.nWidth
                    || pOutputPortDef->format.video.nFrameHeight < out2DAllocParam.nHeight ) {

                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "width = %d. expected width = %d, height =  %d, expected height = %d",
                                       pOutputPortDef->format.video.nFrameWidth, out2DAllocParam.nWidth,
                                       pOutputPortDef->format.video.nFrameHeight, out2DAllocParam.nHeight);
                    pOutputPortDef->format.video.nFrameWidth = out2DAllocParam.nWidth;
                    pOutputPortDef->format.video.nFrameHeight = out2DAllocParam.nHeight;

                    pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                     OMX_VIDEODECODER_INPUT_PORT, pInBufHeader);
                    pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                     OMX_VIDEODECODER_OUTPUT_PORT, pOutBufHeader);
                    TIMM_OSAL_Free(pOutBufHeader);
                    pOutBufHeader = TIMM_OSAL_NULL;

                    /*! Notify to Client change in output port settings */
                    eError = pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                                      OMX_EventPortSettingsChanged, OMX_VIDEODECODER_OUTPUT_PORT, 0, NULL);

                    pVideoDecoderComPvt->nOutPortReconfigRequired = 1;

                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "Dynamic port reconfiguration triggered since small o/p buffer size provided in first frame decode");
                    if( pSEIinfo ) {
                        TIMM_OSAL_Free(pSEIinfo);
                        pSEIinfo = NULL;
                    }
                    if( pVUIinfo ) {
                        TIMM_OSAL_Free(pVUIinfo);
                        pVUIinfo = NULL;
                    }
                    if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
                        RELEASE_MBINFO(pVideoDecoderComPvt);
                    }
                    goto EXIT;
                }
            }

            /* Handling a Mark Buffer  */
            if( pVideoDecoderComPvt->pMark != NULL &&
                pInBufHeader->hMarkTargetComponent == NULL ) {
                pInBufHeader->hMarkTargetComponent
                    = pVideoDecoderComPvt->pMark->hMarkTargetComponent;
                pInBufHeader->pMarkData
                    = pVideoDecoderComPvt->pMark->pMarkData;
                pVideoDecoderComPvt->pMark = NULL;
                pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                         OMX_EventCmdComplete,
                                                         OMX_CommandMarkBuffer,
                                                         0,
                                                         pInBufHeader->pMarkData);
            }
            /* Propogating the mark from an input to the output buffer */
            if( pInBufHeader->hMarkTargetComponent != pHandle ) {
                pOutBufHeader->hMarkTargetComponent
                    = pInBufHeader->hMarkTargetComponent;
                pOutBufHeader->pMarkData = pInBufHeader->pMarkData;
                pInBufHeader->pMarkData = NULL;
                pInBufHeader->hMarkTargetComponent = NULL;
            }
            /*! Populate xDM structure */
            /* Fill In Args */
            pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->numBytes
                = pInBufHeader->nFilledLen - pInBufHeader->nOffset;
            pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID
                =  (OMX_S32) pOutBufHeader;
            if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_ENTIREFRAME ) {
                /* Fill Input Buffer Descriptor */
                pInBufDescPtr->descs[0].buf
                    =    (XDAS_Int8 *)((XDAS_Int32)pInBufHeader->pBuffer +
                                       pInBufHeader->nOffset);
                pInBufDescPtr->descs[0].memType = XDM_MEMTYPE_RAW;
                pInBufDescPtr->descs[0].bufSize.bytes = pInBufHeader->nFilledLen - pInBufHeader->nOffset;
            } else {
                /* Fill Input Buffer Descriptor */
                pVideoDecoderComPvt->tInBufDesc.descs[0].buf
                    =    (XDAS_Int8 *)((XDAS_Int32)pInBufHeader->pBuffer +
                                       pInBufHeader->nOffset);
                pVideoDecoderComPvt->tInBufDesc.descs[0].memType = XDM_MEMTYPE_RAW;
                pVideoDecoderComPvt->tInBufDesc.descs[0].bufSize.bytes = pInBufHeader->nFilledLen - pInBufHeader->nOffset;
                pVideoDecoderComPvt->bIsFirstDataSyncCall = OMX_TRUE;
            }

            /* Initialize Number of Buffers for input and output */
            pVideoDecoderComPvt->tInBufDesc.numBufs = 1;
            pVideoDecoderComPvt->tOutBufDesc.numBufs = 2;

            /* Fill Output Buffer Descriptor */
            nOutbufferSize = pOutputPortDef->nBufferSize;

            pOutBufDescPtr->descs[0].bufSize.bytes = nOutbufferSize * 2 / 3;    // To be changed
            pOutBufDescPtr->descs[0].buf = (XDAS_Int8 *) pOutBufHeader->pBuffer;
            pOutBufDescPtr->descs[0].memType = XDM_MEMTYPE_RAW;

            pOutBufDescPtr->descs[1].bufSize.bytes = nOutbufferSize / 3;    // To be changed
            pOutBufDescPtr->descs[1].buf
                = (XDAS_Int8 *) pOutBufHeader->pBuffer +
                  pVideoDecoderComPvt->tOutBufDesc.descs[0].bufSize.bytes;
            pOutBufDescPtr->descs[1].memType = XDM_MEMTYPE_RAW;


            pOutBufHeader->nTimeStamp = pInBufHeader->nTimeStamp;

            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Time stamp provided at i/p buffer = %d",
                               pInBufHeader->nTimeStamp);
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Buffer provided to Codec with Padded Width = %d, Padded Height = %d",
                               pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nWidth,
                               pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight);

            if( pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp == OMX_TRUE && pVideoDecoderComPvt->bInputBufferCancelled == 0 ) {
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "Time stamp provided at i/p buffer = %d",
                                   pInBufHeader->nTimeStamp);
                TIMM_OSAL_WriteToPipe(pVideoDecoderComPvt->pTimeStampStoragePipe,
                                      &(pInBufHeader->nTimeStamp),
                                      sizeof(OMX_TICKS),
                                      TIMM_OSAL_NO_SUSPEND);
            }
        }
        //pVideoDecoderComPvt->bInputBufferCancelled = 0;
#ifdef H264VD_CODEC_BUG
        pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID
            = (OMX_U32)(pOutBufHeader) & 0x7FFFFFFF;
#endif
        /*Copy OMX_BUFFERFLAG_DECODEONLY from input buffer header to output buffer header*/
        pOutBufHeader->nFlags |= (pInBufHeader->nFlags & OMX_BUFFERFLAG_DECODEONLY);

        pDecExtStructure = &(pVideoDecoderComPvt->pDecStructures);

        /*! Call the Codec Process Function */
        //TIMM_OSAL_BenchmarkStart("Codec_Process %x", (TIMM_OSAL_PTR *)393962);
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "\n Before codec process \n");
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Input Buffer Size provided to codec is : %d",
                           pVideoDecoderComPvt->tInBufDesc.descs[0].bufSize.bytes);
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Frame count is : %d", pVideoDecoderComPvt->nFrameCounter + 1);
       // TOOLS_PROF_ADD_LOG_EX(TOOLS_PROF_LOG_ID_CODEC_PROC_START, (MSP_U32)hComponent, (MSP_U32)NULL);

#if (DUCATI_VIDEO_PERF_MEASUREMENT == 1)
        clkTickBef = Tools_Time_getCurrent_1ms();
#endif

        status =  VIDDEC3_process
                 (
            pVideoDecoderComPvt->pDecHandle,
            pInBufDescPtr, pOutBufDescPtr,
            (VIDDEC3_InArgs *)pDecExtStructure->pDecInArgs,
            (VIDDEC3_OutArgs *)pDecExtStructure->pDecOutArgs
                 );

#ifdef PSI_KPI
        kpi_after_codec();
#endif /*PSI_KPI*/

#if (DUCATI_VIDEO_PERF_MEASUREMENT == 1)
        clkTickAfter = Tools_Time_getCurrent_1ms();
        clkTickDiff  = (uint32)(clkTickAfter - clkTickBef);
        ++(pVideoDecoderComPvt->decRecordSessionFrameCounter);
        (pVideoDecoderComPvt->decRecordSessionProcessTimeCounter) +=clkTickDiff;
        if( clkTickDiff < pVideoDecoderComPvt->decRecordSessionMinProcessTime ) {
            pVideoDecoderComPvt->decRecordSessionMinProcessTime = clkTickDiff;
        }
        if( clkTickDiff > pVideoDecoderComPvt->decRecordSessionMaxProcessTime ) {
            pVideoDecoderComPvt->decRecordSessionMaxProcessTime = clkTickDiff;
        }
#endif

      //  TIMM_OSAL_BenchmarkStop("Codec_Process %x", (TIMM_OSAL_PTR *)393962);
       // TOOLS_PROF_ADD_LOG_EX(TOOLS_PROF_LOG_ID_CODEC_PROC_STOP, (MSP_U32)hComponent, (MSP_U32)NULL);
        //TOOLS_PROF_ADD_LOG_EX(TOOLS_PROF_LOG_ID_CODEC_BYTE_CONSUM,
          //                    (MSP_U32)pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->bytesConsumed, (MSP_U32)NULL);

        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "\n After codec process \n");
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                           "Bytes consumed - %d",
                           pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->bytesConsumed);

        pDecOutArgs
            = pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs;

        /*Do cache inalidate*/
//        Cache_inv(pSEIinfo, sizeof(IH264VDEC_SeiMessages), Cache_Type_ALL, TRUE);
  //      Cache_inv(pVUIinfo, sizeof(IH264VDEC_VuiParams), Cache_Type_ALL, TRUE);
    //    Cache_inv(pVideoDecoderComPvt->pMBinfo, pVideoDecoderComPvt->nMBinfoSize, Cache_Type_ALL, TRUE);



        if( pVUIinfo ) {
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "vui parsed flag = %d, aspect_ratio_idc = %d, \
               aspect_ratio_info_present_flag = %d, pVUIinfo->sar_width = %d,pVUIinfo->sar_height = %d",
                               ((IH264VDEC_VuiParams *)pVUIinfo)->parsed_flag, ((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_idc,
                               ((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_info_present_flag,
                               ((IH264VDEC_VuiParams *)pVUIinfo)->sar_width, ((IH264VDEC_VuiParams *)pVUIinfo)->sar_height);

            if(((IH264VDEC_VuiParams *)pVUIinfo)->parsed_flag == 1 && ((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_info_present_flag == 1 ) {
                if(((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_idc >= 1 && ((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_idc <= 16 ) {
        //            _LOOKUP_VUI_PARAMS(((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_idc, &nScaleWidth, &nScaleHeight);
                    nScale = nScaleWidth / nScaleHeight;
                } else if(((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_idc == 255 ) {
                    nScaleWidth = ((IH264VDEC_VuiParams *)pVUIinfo)->sar_width;
                    nScaleHeight = ((IH264VDEC_VuiParams *)pVUIinfo)->sar_height;
                    nScale = nScaleWidth / nScaleHeight;
                }
                if((((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_idc >= 1 && ((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_idc <= 16)
                   || ((IH264VDEC_VuiParams *)pVUIinfo)->aspect_ratio_idc == 255 ) {
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
                    } else if( nScale < 1 ) {
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
        }
        pOMXmeta = (OMX_METADATA *) TIMM_OSAL_Malloc(sizeof(OMX_METADATA), TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
        TIMM_OSAL_Memset(pOMXmeta, NULL, sizeof(OMX_METADATA));
        pOMXmeta->CodecExtenderError = pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError;
        if( pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
            pOMXmeta->pMBinfo=pVideoDecoderComPvt->pMBinfo;
        }
        if( pVideoDecoderComPvt->tMetadata.bEnableSEIInfo ) {
            pOMXmeta->pSEIinfo=pSEIinfo;
        }
        if( pVideoDecoderComPvt->tMetadata.bEnableVUIInfo ) {
            pOMXmeta->pVUIinfo=pVUIinfo;
        }

        if( !(pDecOutArgs->outBufsInUseFlag && pVideoDecoderComPvt->nOutbufInUseFlag == 1) && !(pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader
                                                                                                == pVideoDecoderComPvt->parseHeaderFlag)) {
            _OMX_VIDEODECODER_PUSH_METADATA_BUFFER(hComponent, pOMXmeta,
                                                   pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID);
        } else {
            _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pOMXmeta);
        }


        /*! In case Process returns error */
        if( status == XDM_EFAIL ) {

            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\n Process function returned an Error...  \n");
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Codec Extended - 0x%x",
                               pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError);
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Input Buffer Size provided to codec is : %d", pVideoDecoderComPvt->tInBufDesc.descs[0].bufSize.bytes);
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Frame count is : %d", pVideoDecoderComPvt->nFrameCounter + 1);
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "Bytes consumed - %d",
                               pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->bytesConsumed);
            OMX_BASE_Menu(" Codec Used is %s", pVideoDecoderComPvt->strDecoderName);

#if 1
            if( !(pVideoDecoderComPvt->bInputBufferCancelled == 1 && pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError & 0x8000)) {
                /*! Call function to handle Codec Error */
                eError = OMX_TI_VIDDEC_HandleCodecProcError(hComponent,
                                                            &(pInBufHeader), &(pOutBufHeader));
            }
            if( eError != OMX_ErrorNone ) {
                goto EXIT;
            }
            if( pVideoDecoderComPvt->nOutPortReconfigRequired == 1 ) {
                goto EXIT;
            }
#endif

        }  // End of check for status == XDM_EFAIL
        if( pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader
            == pVideoDecoderComPvt->parseHeaderFlag ) {
            eError = OMX_TI_VIDDEC_HandleFirstFrame(hComponent, NULL);
            goto EXIT;
        }

        // Increment the FrameCounter value
        pVideoDecoderComPvt->nFrameCounter++;
        if( pDecOutArgs->outBufsInUseFlag && pVideoDecoderComPvt->nOutbufInUseFlag == 1 ) {
            pVideoDecoderComPvt->nFrameCounter--;
        }
        if( pDecOutArgs->outBufsInUseFlag ) {
            pVideoDecoderComPvt->nOutbufInUseFlag = 1;

            /* Check for any output buffer which is freed by codec*/
            ii = 0;

            while( pDecOutArgs->freeBufID[ii] ) {
                if( pDecOutArgs->outputID[0] == 0 && pDecOutArgs->freeBufID[ii] == pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID ) {
                    pVideoDecoderComPvt->nOutbufInUseFlag = 0;
                    pVideoDecoderComPvt->nFrameCounter--;
                }
                _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, pDecOutArgs->freeBufID[ii]);
                pFreeBufHeader = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *) pDecOutArgs->freeBufID[ii];
                /* Send the Freed buffer back to base component */
                pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                 OMX_VIDEODECODER_OUTPUT_PORT,
                                                 pFreeBufHeader);
                /* Free the Memory for buffer header */
                if( pFreeBufHeader ) {
                    TIMM_OSAL_Free(pFreeBufHeader);
                    pFreeBufHeader = NULL;
                }
                ii++;

            }

            if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_ENTIREFRAME ) {
                ii = 0;
                if( pInBufHeader != NULL ) {
                    if((pInBufHeader->nFilledLen - pInBufHeader->nOffset) != pDecOutArgs->bytesConsumed ) {
                        nNewInBufferRequired = 1;
                        pInBufHeader->nOffset = pInBufHeader->nOffset + pDecOutArgs->bytesConsumed;
                        continue;
                    }
                    nNewInBufferRequired = 0;
                    //pInBufHeader->nFilledLen = pInBufHeader->nFilledLen - pDecOutArgs->bytesConsumed;
                    pInBufHeader->nOffset = pInBufHeader->nOffset + pDecOutArgs->bytesConsumed;
                    if( !(pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS)) {
                        // Send Input buffer back to base
                        pVideoDecoderComPvt->fpDioSend(hComponent,
                                                       OMX_VIDEODECODER_INPUT_PORT,
                                                       pInBufHeader);

                    } else if( pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS ) {
                        OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, NULL,
                                                                 pInBufHeader);
                        if( eError != OMX_ErrorNone ) {
                            goto EXIT;
                        }
                    }
                }
                pVideoDecoderComPvt->fpDioGetCount(hComponent,
                                                   OMX_VIDEODECODER_INPUT_PORT, (OMX_PTR)&nInMsgCount);
                if( nInMsgCount > 0 ) {
                    continue;
                } else {
                    break;
                }
            } else {
                if( pVideoDecoderComPvt->bIsFlushRequired ) {
                    OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, NULL,
                                                             NULL);
                    if( eError != OMX_ErrorNone ) {
                        goto EXIT;
                    }
                    pVideoDecoderComPvt->bIsFlushRequired = OMX_FALSE;
                }
                pVideoDecoderComPvt->fpDioGetCount(hComponent,
                                                   OMX_VIDEODECODER_INPUT_PORT, (OMX_PTR)&nInMsgCount);
                if( nInMsgCount > 0 ) {
                    continue;
                } else {
                    break;
                }
            }
        }
#if 1
        // Check for the width/height after first frame
        if((pVideoDecoderComPvt->nFrameCounter == 1) || (pVideoDecoderComPvt->nOutbufInUseFlag == 1 && pVideoDecoderComPvt->nFrameCounter == 2)) {
            eError = OMX_TI_VIDDEC_HandleFirstFrame(hComponent,
                                                    &(pInBufHeader));
            if( eError != OMX_ErrorNone ) {
                goto EXIT;
            }
            OMX_BASE_Warn("BEFORE content type %d color format 0x%x",
                          pDecOutArgs->decodedBufs.contentType,
                          pOutputPortDef->format.video.eColorFormat);

            if( pDecOutArgs->decodedBufs.contentType >= 1 && pDecOutArgs->decodedBufs.contentType <= 3 ) {
                pOutputPortDef->format.video.eColorFormat = OMX_COLOR_FormatYUV420PackedSemiPlanar;
                pVideoDecoderComPvt->bSupportSkipGreyOutputFrames = OMX_FALSE;
                pVideoDecoderComPvt->bSyncFrameReady = OMX_TRUE;
                /* update the interlaced format type */
                pVideoDecoderComPvt->tIlaceFmtTypes.bInterlaceFormat = OMX_TRUE;
                if( pDecOutArgs->decodedBufs.contentType == 3 ) {
                    pVideoDecoderComPvt->tIlaceFmtTypes.nInterlaceFormats = OMX_InterlaceFrameBottomFieldFirst;
                } else {
                    pVideoDecoderComPvt->tIlaceFmtTypes.nInterlaceFormats = OMX_InterlaceFrameTopFieldFirst;
                }
                OMX_BASE_Warn("AFTER content type %d color format 0x%x",
                              pDecOutArgs->decodedBufs.contentType,
                              pOutputPortDef->format.video.eColorFormat);

                OMX_BASE_Error("Port settings changes, Send Notification to Client(A9)");

                /*! Notify to Client change in output port settings */
                eError = pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                                  OMX_EventPortSettingsChanged,
                                                                  OMX_VIDEODECODER_OUTPUT_PORT,
                                                                  OMX_TI_IndexConfigStreamInterlaceFormats,
                                                                  NULL);

                OMX_BASE_Trace("Dynamic port reconfiguration triggered");
            }
            if( pVideoDecoderComPvt->nOutPortReconfigRequired == 1
                && pVideoDecoderComPvt->pBaseClassStructers.pDecStaticParams->displayDelay
                != IVIDDEC3_DECODE_ORDER ) {
                /*! In case Port reconfiguration is required
                 *  output buffer */

                //  _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pOMXmeta);
                goto EXIT;
            }
        }     // End of if condition from nFrameCounter = 1
#endif
        if( pVideoDecoderComPvt->nOutbufInUseFlag == 1 ) {
            pVideoDecoderComPvt->nOutbufInUseFlag = 0;
            nNewInBufferRequired = 0;
        }
#ifdef H264VD_CODEC_BUG
        if( pDecOutArgs->outputID[0] != 0x0 ) {
            pDecOutArgs->outputID[0]
                = pDecOutArgs->outputID[0] | 0x80000000;
        }
        ii=0;

        while( pDecOutArgs->freeBufID[ii] != 0x0 ) {
            pDecOutArgs->freeBufID[ii]
                = pDecOutArgs->freeBufID[ii] | 0x80000000;
            ii++;
        }

        ii=0;
#endif
        //may have to change to multiple outputID[ii] in a loop like freeBufID
        if( pDecOutArgs->outputID[0] ) {
            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\n OutputId is : %x", pDecOutArgs->outputID[0]);

            pOutBufHeader
                = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)pDecOutArgs->outputID[0];

            if( pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp == OMX_TRUE ) {

                TIMM_OSAL_ReadFromPipe(pVideoDecoderComPvt->pTimeStampStoragePipe,
                                       &(pOutBufHeader->nTimeStamp),
                                       sizeof(OMX_TICKS),
                                       &(nActualSize),
                                       TIMM_OSAL_NO_SUSPEND);
                TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "Time stamp recived at o/p buffer = %d",
                                   pOutBufHeader->nTimeStamp);
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "\n TimeStamp dequeued : %x", (OMX_U32)pOutBufHeader->nTimeStamp);
            }

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
            activeFrameRegion[0]
                = pDecOutArgs->displayBufs.bufDesc[0].activeFrameRegion;
            activeFrameRegion[1].bottomRight.y
                = (activeFrameRegion[0].bottomRight.y) / 2;
            activeFrameRegion[1].bottomRight.x
                = activeFrameRegion[0].bottomRight.x;
            // Offset = (rows*stride) + x-offset
            pOutBufHeader->nOffset
                = (activeFrameRegion[0].topLeft.y * nStride)
                  + activeFrameRegion[0].topLeft.x;

            nLumaFilledLen
                = (nStride * (pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight))
                  - (nStride * (activeFrameRegion[0].topLeft.y))
                  - activeFrameRegion[0].topLeft.x;

            nChromaFilledLen
                = (nStride * (activeFrameRegion[1].bottomRight.y))
                  - nStride + activeFrameRegion[1].bottomRight.x;

            pOutBufHeader->nFilledLen =
                nLumaFilledLen + nChromaFilledLen;


            /* Update the metadata size to base*/
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
            pSEIinfo = pVUIinfo = NULL;
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

            activeFrameRegion[0]
                = pDecOutArgs->displayBufs.bufDesc[0].activeFrameRegion;
            activeFrameRegion[1].bottomRight.y
                = (activeFrameRegion[0].bottomRight.y) / 2;
            activeFrameRegion[1].bottomRight.x
                = activeFrameRegion[0].bottomRight.x;


            TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                               "\n Codec x-offset provided = %d, Codec y-offset provided = %d,\
                    OMX nOffest provided = %d", activeFrameRegion[0].topLeft.x,
                               activeFrameRegion[0].topLeft.y, pOutBufHeader->nOffset);
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

            // FilledLen
            nLumaFilledLen
                = (nStride * (pVideoDecoderComPvt->t2DBufferAllocParams[OMX_VIDEODECODER_OUTPUT_PORT].nHeight))
                  - (nStride * (activeFrameRegion[0].topLeft.y))
                  - activeFrameRegion[0].topLeft.x;

            nChromaFilledLen
                = (nStride * (activeFrameRegion[1].bottomRight.y))
                  - nStride + activeFrameRegion[1].bottomRight.x;

            pOutBufHeader->nFilledLen =
                nLumaFilledLen + nChromaFilledLen;

            // Loop for all output buffers
            while( pDecOutArgs->freeBufID[ii] ) {
                TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                   "\n FreeBuffID is : %x", pDecOutArgs->freeBufID[ii]);
                _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, pDecOutArgs->freeBufID[ii]);
                if( pDecOutArgs->freeBufID[ii] == (OMX_S32) pOutBufHeader ) {
                    // If Free Buf Id matches output buffer id, then the
                    // output buffer is not locked by the codec
                    Buffer_locked = 0;
                    ii++;
                } else {
                    pDupBufHeader
                        = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)
                          pDecOutArgs->freeBufID[ii++];
                    pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                     OMX_VIDEODECODER_OUTPUT_PORT, pDupBufHeader);
                    if( pDupBufHeader ) {
                        TIMM_OSAL_Free(pDupBufHeader);
                        pDupBufHeader = TIMM_OSAL_NULL;
                    }
                }
            }

            if( pVideoDecoderComPvt->bSupportSkipGreyOutputFrames ) {
                if( pDecOutArgs->displayBufs.bufDesc[0].frameType == IVIDEO_I_FRAME || pDecOutArgs->displayBufs.bufDesc[0].frameType == IVIDEO_IDR_FRAME ) {
                    pVideoDecoderComPvt->bSyncFrameReady = OMX_TRUE;
                }
            }
            if( Buffer_locked == 1 ) {
                pDupBufHeader = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)TIMM_OSAL_Malloc(sizeof(OMX_VIDEODECODER_CUSTOMBUFFERHEADER), TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT);
                if( pDupBufHeader == TIMM_OSAL_NULL ) {
                    eError = OMX_ErrorInsufficientResources;
                    goto EXIT;
                }
                pVideoDecoderComPvt->fpDioDup(hComponent, OMX_VIDEODECODER_OUTPUT_PORT, pOutBufHeader, pDupBufHeader);
                pDupBufHeader->nMetaDataUsageBitField = pOutBufHeader->nMetaDataUsageBitField;

                pDupBufHeader->tSEIinfo2004Frame1 = pOutBufHeader->tSEIinfo2004Frame1;
                pDupBufHeader->tSEIinfo2004Frame2 = pOutBufHeader->tSEIinfo2004Frame2;
                pDupBufHeader->tSEIinfo2010Frame1 = pOutBufHeader->tSEIinfo2010Frame1;
                pDupBufHeader->tSEIinfo2010Frame2 = pOutBufHeader->tSEIinfo2010Frame2;
                pDupBufHeader->tRescalingMatrix = pOutBufHeader->tRescalingMatrix;
                pDupBufHeader->tRangeMapping = pOutBufHeader->tRangeMapping;
                pDupBufHeader->tMbInfoFrame1 = pOutBufHeader->tMbInfoFrame1;
                pDupBufHeader->tMbInfoFrame2 = pOutBufHeader->tMbInfoFrame2;
                pDupBufHeader->tSeiInfoFrame1 = pOutBufHeader->tSeiInfoFrame1;
                pDupBufHeader->tSeiInfoFrame2 = pOutBufHeader->tSeiInfoFrame2;
                pDupBufHeader->tVuiInfoFrame1 = pOutBufHeader->tVuiInfoFrame1;
                pDupBufHeader->tVuiInfoFrame2 = pOutBufHeader->tVuiInfoFrame2;

                if( pOutBufHeader->hMarkTargetComponent != NULL ) {
                    pOutBufHeader->pMarkData = NULL;
                    pOutBufHeader->hMarkTargetComponent = NULL;
                }

                if( pVideoDecoderComPvt->bSyncFrameReady == OMX_TRUE ) {
                    if( !(pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS) || !(pVideoDecoderComPvt->bIsFlushRequired)) {
                        pVideoDecoderComPvt->fpDioSend(hComponent, OMX_VIDEODECODER_OUTPUT_PORT, pDupBufHeader);
                    }
                } else {
                    pVideoDecoderComPvt->fpDioCancel(hComponent, OMX_VIDEODECODER_OUTPUT_PORT, pDupBufHeader);
                }
                if( pVideoDecoderComPvt->bSyncFrameReady == OMX_FALSE || (!(pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS) || !(pVideoDecoderComPvt->bIsFlushRequired))) {
                    if( pDupBufHeader ) {
                        TIMM_OSAL_Free(pDupBufHeader);
                        pDupBufHeader = TIMM_OSAL_NULL;
                    }
                }
            } else {
                // In case Buffer is not locked
                if( pVideoDecoderComPvt->bSyncFrameReady == OMX_TRUE ) {
                    if( !(pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS) || !(pVideoDecoderComPvt->bIsFlushRequired)) {
                        // Send the Output buffer to Base component
                        pVideoDecoderComPvt->fpDioSend(hComponent,
                                                       OMX_VIDEODECODER_OUTPUT_PORT, pOutBufHeader);
                    }
                } else {
                    pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                     OMX_VIDEODECODER_OUTPUT_PORT, pOutBufHeader);
                }
                if( pVideoDecoderComPvt->bSyncFrameReady == OMX_FALSE || (!(pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS) || !(pVideoDecoderComPvt->bIsFlushRequired))) {
                    if( pOutBufHeader ) {
                        TIMM_OSAL_Free(pOutBufHeader);
                        pOutBufHeader = TIMM_OSAL_NULL;
                    }
                }
            }
        } else if((pDecOutArgs->outputID[0] == 0)
                  && (pDecOutArgs->freeBufID[0] == pVideoDecoderComPvt->pBaseClassStructers.pDecInArgs->inputID)) {
            /* Special case where Codec only frees buffer but there
             * is no buffer for display */
            ii = 0;

            /* Loop for all the free buffers */
            while( pDecOutArgs->freeBufID[ii] ) {
                _OMX_VIDEODECODER_PULL_METADATA_BUFFER(hComponent, pDecOutArgs->freeBufID[ii], &pOMXmetaFrame1, &pOMXmetaFrame2);
                if( pOMXmetaFrame1 ) {
                    _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pOMXmetaFrame1);
                    pOMXmetaFrame1 = NULL;
                }
                if( pOMXmetaFrame2 ) {
                    _OMX_VIDEODECODER_FREE_METADATA_BUFFER(pOMXmetaFrame2);
                    pOMXmetaFrame2 = NULL;
                }

                _OMX_VIDEODECODER_DELETE_METADATA_BUFFER(hComponent, pDecOutArgs->freeBufID[ii]);
                pDupBufHeader
                    = (OMX_VIDEODECODER_CUSTOMBUFFERHEADER *)
                      pDecOutArgs->freeBufID[ii];
                pVideoDecoderComPvt->fpDioCancel(hComponent,
                                                 OMX_VIDEODECODER_OUTPUT_PORT, pDupBufHeader);
                if( pDupBufHeader ) {
                    TIMM_OSAL_Free(pDupBufHeader);
                    pDupBufHeader = TIMM_OSAL_NULL;
                }
                ii++;
            }
        }   /* End of else-if condition */
        if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_ENTIREFRAME ) {
            if( pInBufHeader && pDecOutArgs &&
                (pVideoDecoderComPvt->pBaseClassStructers.pDecDynParams->decodeHeader == XDM_DECODE_AU && pVideoDecoderComPvt->bSupportDecodeOrderTimeStamp == OMX_TRUE)) {
                if((pVideoDecoderComPvt->bInputBufferCancelled == 1) && (((pDecOutArgs->bytesConsumed == 0) || (((OMX_S32)pInBufHeader->nFilledLen) <= ((OMX_S32)pInBufHeader->nOffset + pDecOutArgs->bytesConsumed + 3))) || (pVideoDecoderComPvt->pBaseClassStructers.pDecOutArgs->extendedError & 0x8000))) {
                    pVideoDecoderComPvt->bInputBufferCancelled = 0;
                } else if((((OMX_S32)pInBufHeader->nFilledLen) > ((OMX_S32)pInBufHeader->nOffset + pDecOutArgs->bytesConsumed + 3)) && ((pDecOutArgs->bytesConsumed != 0) && !(pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS))) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "\n i/p bytes provided = %d, bytes left to be processed = %d \n",
                                       pInBufHeader->nFilledLen - pInBufHeader->nOffset,
                                       pInBufHeader->nFilledLen - pInBufHeader->nOffset - pDecOutArgs->bytesConsumed);
                    pInBufHeader->nOffset = pInBufHeader->nOffset + pDecOutArgs->bytesConsumed;
                    pVideoDecoderComPvt->bInputBufferCancelled = 1;
                    pVideoDecoderComPvt->fpDioCancel(hComponent, OMX_VIDEODECODER_INPUT_PORT, pInBufHeader);
                }
            }
            /* Currently the assumption is that the entire input buffer is consumed
             * going forward we might have to handle cases where partial buffer is
             * consumed */
            if( pVideoDecoderComPvt->bInputBufferCancelled == 0 ) {
                if( pInBufHeader != NULL &&
                    !(pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS) &&
                    pVideoDecoderComPvt->nOutPortReconfigRequired != 1 ) {
                    pInBufHeader->nFilledLen
                        = pInBufHeader->nFilledLen - pDecOutArgs->bytesConsumed - pInBufHeader->nOffset;
                    pInBufHeader->nOffset = 0;

                    // Send Input buffer back to base
                    pVideoDecoderComPvt->fpDioSend(hComponent,
                                                   OMX_VIDEODECODER_INPUT_PORT,
                                                   pInBufHeader);
                }
                if( pInBufHeader != NULL &&
                    (pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS) &&
                    pVideoDecoderComPvt->nOutPortReconfigRequired != 1 ) {
                    pInBufHeader->nFilledLen
                        = pInBufHeader->nFilledLen - pDecOutArgs->bytesConsumed - pInBufHeader->nOffset;
                    pInBufHeader->nOffset = 0;
                    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC,
                                       "\n EOS with non-zero filled length\n");
                    if( Buffer_locked == 1 ) {
                        OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, pDupBufHeader,
                                                                 pInBufHeader);
                    } else {
                        OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, pOutBufHeader,
                                                                 pInBufHeader);
                    }
                    if( eError != OMX_ErrorNone ) {
                        goto EXIT;
                    }
                }
            }
        } else {
            if( pVideoDecoderComPvt->bIsFlushRequired ) {
                if( Buffer_locked == 1 ) {
                    OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, pDupBufHeader,
                                                             NULL);
                } else {
                    OMX_TI_VIDEODECODER_Handle_FLUSH_and_EOS(hComponent, pOutBufHeader,
                                                             NULL);
                }
                pVideoDecoderComPvt->bIsFlushRequired = OMX_FALSE;
                if( eError != OMX_ErrorNone ) {
                    goto EXIT;
                }
            }
        }
        /* send an Mark Event incase if the target comp
         * is same as this comp */
        if( pInBufHeader != NULL && pInBufHeader->hMarkTargetComponent == pHandle ) {
            pVideoDecoderComPvt->fpReturnEventNotify(hComponent,
                                                     OMX_EventMark, 0, 0, pInBufHeader->pMarkData);
            pInBufHeader->pMarkData = NULL;
            pInBufHeader->hMarkTargetComponent = NULL;
        }
        nIsDioReady = OMX_BASE_IsDioReady(hComponent, OMX_VIDEODECODER_OUTPUT_PORT);
        if( nIsDioReady == OMX_FALSE ) {
            goto EXIT;
        }
        // Get the number of buffers in Input and Output port
        pVideoDecoderComPvt->fpDioGetCount(hComponent,
                                           OMX_VIDEODECODER_INPUT_PORT, (OMX_PTR)&nInMsgCount);
        pVideoDecoderComPvt->fpDioGetCount(hComponent,
                                           OMX_VIDEODECODER_OUTPUT_PORT, (OMX_PTR)&nOutMsgCount);

    } // End of while loop for input and output buffers

EXIT:

    if( pVideoDecoderComPvt->bResourceInUse == OMX_TRUE ) {
        pVideoDecoderComPvt->bResourceInUse = OMX_FALSE;
        TIMM_OSAL_SemaphoreRelease(pVideoDecoderComPvt->pSemaphoreSuspendResume);
    }

#ifndef CODEC_CONFIG_WORKAROUND
    if( pVideoDecoderComPvt ) {
        if( pVideoDecoderComPvt->pAttrDesc && pVideoDecoderComPvt->nOutPortReconfigRequired == 0 ) {
            if( pVideoDecoderComPvt->pAttrDesc->pAttributeData ) {
                TIMM_OSAL_Free(pVideoDecoderComPvt->pAttrDesc->pAttributeData);
                pVideoDecoderComPvt->pAttrDesc->pAttributeData = NULL;
            }
            TIMM_OSAL_Free(pVideoDecoderComPvt->pAttrDesc);
            pVideoDecoderComPvt->pAttrDesc = NULL;
        }
    }
#endif

    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_ComponentDeinit()
 *                     This method de-initializes the component and frees up
 *                     memory acquired by the decoder component
 *  @param [in] hComponent  : pointer to the OMX Component.
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_ComponentDeinit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pComp;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_U32                     i;
    OMX_BASE_PORT_PROPERTIES   *pOutPortProperties = NULL;

    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);
    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt = (OMX_VIDEODECODER_PVTTYPE *)pComp->pComponentPrivate;

    //Freeing the input buffer header list in case of input data mode is slice mode
    if(((IVIDDEC3_Params *)(pVideoDecoderComPvt->pDecStructures.pDecStaticParams))->inputDataMode == IVIDEO_SLICEMODE ) {
        for( i=0; i < OMX_VIDDEC_DATASYNC_DEFAULT_IN_BUF_COUNT; i++ ) {
            if( pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].pInBufHeader ) {
                TIMM_OSAL_Free(pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].pInBufHeader);
            }
        }
    }
    TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXBASE, "Inside comp deinit");

    TIMM_OSAL_DeletePipe(pVideoDecoderComPvt->pTimeStampStoragePipe);

    OMX_BASE_UtilCleanupIfError(hComponent);

    // Call to Specific Decoder De-init routine
    pVideoDecoderComPvt->fpDeinit_Codec(hComponent);

    //Delete the mutex created for suspend resume feature
    TIMM_OSAL_MutexDelete(pVideoDecoderComPvt->pMutexResourceSuspended);
    pVideoDecoderComPvt->pMutexResourceSuspended  = NULL;

    //Delete the Semaphore created for suspend resume feature
    TIMM_OSAL_SemaphoreDelete(pVideoDecoderComPvt->pSemaphoreSuspendResume);
    pVideoDecoderComPvt->pSemaphoreSuspendResume  = NULL;

    // Call to base Component De-init routine
    eError = OMX_BASE_ComponentDeinit(hComponent);
    // Close Codec-Engine
    if( pVideoDecoderComPvt->ce ) {
        Engine_close(pVideoDecoderComPvt->ce);
    }
#ifdef VIDEODECODER_TILERTEST
    if( pVideoDecoderComPvt->tDerToBase.pPortProperties != NULL ) {
        pOutPortProperties
            = pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT];
        if((pOutPortProperties != NULL)
           && (pOutPortProperties->pBufParams != NULL)) {
            TIMM_OSAL_Free(pOutPortProperties->pBufParams);
            pOutPortProperties->pBufParams = NULL;
        }
    } else {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
#endif
    if( pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->pMetaDataFieldTypesArr ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->pMetaDataFieldTypesArr);
        pVideoDecoderComPvt->tDerToBase.pPortProperties[OMX_VIDEODECODER_OUTPUT_PORT]->pMetaDataFieldTypesArr = NULL;
    }

    // Free up Memory for Port Properties
    for( i = 0; i < (pVideoDecoderComPvt->tDerToBase.nNumPorts); i++ ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->tDerToBase.pPortProperties[i]);
        pVideoDecoderComPvt->tDerToBase.pPortProperties[i] = NULL;
    }

    // Free up rest of memory taken by the component
    if( pVideoDecoderComPvt->tDerToBase.pPortProperties ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->tDerToBase.pPortProperties);
        pVideoDecoderComPvt->tDerToBase.pPortProperties = NULL;
    }
    if( pVideoDecoderComPvt->tDerToBase.cComponentName ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->tDerToBase.cComponentName);
        pVideoDecoderComPvt->tDerToBase.cComponentName = NULL;
    }
    if( pVideoDecoderComPvt->tDerToBase.pVideoPortParams ) {
        TIMM_OSAL_Free(pVideoDecoderComPvt->tDerToBase.pVideoPortParams);
        pVideoDecoderComPvt->tDerToBase.pVideoPortParams = NULL;
    }
    if( pVideoDecoderComPvt->bErrorConcealmentBuffer && !pVideoDecoderComPvt->tMetadata.bEnableMBInfo ) {
        RELEASE_MBINFO(pVideoDecoderComPvt);
    }
    pVideoDecoderComPvt->mem_count_end = TIMM_OSAL_GetMemCounter();
    pVideoDecoderComPvt->mem_size_end = TIMM_OSAL_GetMemUsage();
    if( pVideoDecoderComPvt->mem_count_start != pVideoDecoderComPvt->mem_count_end ) {
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Memory leak detected. Bytes lost = %d", (pVideoDecoderComPvt->mem_size_end - pVideoDecoderComPvt->mem_size_start));
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, " Value from GetMemCounter = %d", pVideoDecoderComPvt->mem_count_end);
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, " Value from GetMemUsage = %d", pVideoDecoderComPvt->mem_size_end);

#ifdef USE_MEMORY_LEAK_TOOL
        Memory_getStats(NULL, &pVideoDecoderComPvt->stats);
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Total size = %d", pVideoDecoderComPvt->stats.totalSize);
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Total free size = %d", pVideoDecoderComPvt->stats.totalFreeSize);
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "Largest Free size = %d", pVideoDecoderComPvt->stats.largestFreeSize);
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "");
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "-----------------------------------------------");
        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "");
#endif //USE_MEMORY_LEAK_TOOL
    }
    TIMM_OSAL_Free(pVideoDecoderComPvt);
    pVideoDecoderComPvt = NULL;

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn OMX_TI_VIDEODECODER_GetExtensionIndex()
 *                     This method gets the extension index
 *  @param [in] hComponent  : pointer to the OMX Component.
 *  @param [in] cParameterName : Name of Parameter
 *  @param [out] pIndexType : Index Type to be returned
 *
 */
/* ==========================================================================*/
OMX_ERRORTYPE OMX_TI_VIDEODECODER_GetExtensionIndex(OMX_HANDLETYPE hComponent,
                                                    OMX_STRING cParameterName, OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;

    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    // Check for NULL Parameters
    if((cParameterName == NULL) || (pIndexType == NULL)) {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    // Check for Valid State
    if( pVideoDecoderComPvt->tCurState == OMX_StateInvalid ) {
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }
    // Ensure that String length is not greater than Max allowed length
    if( strlen(cParameterName) > 127 ) {
        //strlen does not include \0 size, hence 127
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

	if(strcmp(cParameterName, "OMX.google.android.index.enableAndroidNativeBuffers") == 0)
	{
		// If Index type is 2D Buffer Allocated Dimension
		*pIndexType = (OMX_INDEXTYPE) OMX_TI_IndexUseNativeBuffers;
		goto EXIT;
	}
	else if (strcmp(cParameterName, "OMX.google.android.index.useAndroidNativeBuffer2") == 0)
	{
		//This is call just a dummy for android to support backward compatibility
		*pIndexType = (OMX_INDEXTYPE) NULL;
		goto EXIT;
	}
else if (strcmp(cParameterName, "OMX.google.android.index.getAndroidNativeBufferUsage") == 0)
	{
		*pIndexType = (OMX_INDEXTYPE) OMX_TI_IndexAndroidNativeBufferUsage;
	}

else if( strcmp(cParameterName, "OMX_TI_IndexParam2DBufferAllocDimension") == 0 ) {
        // If Index type is 2D Buffer Allocated Dimension
        *pIndexType = (OMX_INDEXTYPE) OMX_TI_IndexParam2DBufferAllocDimension;
    } else if( strcmp(cParameterName, "OMX_TI_IndexParamTimeStampInDecodeOrder") == 0 ) {
        // If Index type is Time Stamp In Decode Order
        *pIndexType = (OMX_INDEXTYPE) OMX_TI_IndexParamTimeStampInDecodeOrder;
    } else if( strcmp(cParameterName, "OMX_TI_IndexParamSkipGreyOutputFrames") == 0 ) {
        // If Index type is Skip Grey OutputFrames
        *pIndexType = (OMX_INDEXTYPE) OMX_TI_IndexParamSkipGreyOutputFrames;
    } else {
        //does not match any custom index
        eError = OMX_ErrorUnsupportedIndex;
    }
EXIT:
    return (eError);

}

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
XDAS_Int32 _OMX_VIDDEC_DataSync_GetInputData(XDM_DataSyncHandle dataSyncHandle, XDM_DataSyncDesc *dataSyncDesc)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_BUFFERHEADERTYPE       *pInBufHeader = NULL;
    OMX_U32                     i;

    /* Check the input parameters */
    OMX_BASE_REQUIRE((dataSyncHandle != NULL) && (dataSyncDesc != NULL), OMX_ErrorBadParameter);

    pHandle = (OMX_COMPONENTTYPE *)dataSyncHandle;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;
    if( pVideoDecoderComPvt->bIsFirstDataSyncCall ) {
        dataSyncDesc->size = sizeof(XDM_DataSyncDesc);
        dataSyncDesc->scatteredBlocksFlag = XDAS_FALSE;
        dataSyncDesc->baseAddr = (XDAS_Int32 *)pVideoDecoderComPvt->tInBufDesc.descs[0].buf;
        dataSyncDesc->numBlocks = 1;
        dataSyncDesc->varBlockSizesFlag = XDAS_TRUE;
        pVideoDecoderComPvt->nSliceSizes[0] = pVideoDecoderComPvt->tInBufDesc.descs[0].bufSize.bytes;
        dataSyncDesc->blockSizes = &(pVideoDecoderComPvt->nSliceSizes[0]);
        pVideoDecoderComPvt->bIsFirstDataSyncCall = OMX_FALSE;
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "DS Params=%d %d %x %d %d %x %d\n", dataSyncDesc->size, dataSyncDesc->scatteredBlocksFlag,
                           dataSyncDesc->baseAddr, dataSyncDesc->numBlocks, dataSyncDesc->varBlockSizesFlag, dataSyncDesc->blockSizes, dataSyncDesc->blockSizes[0]);
    } else {
        for( i=0; i < OMX_VIDDEC_DATASYNC_DEFAULT_IN_BUF_COUNT; i++ ) {
            if( pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].bEntryUsedFlag == OMX_FALSE ) {
                break;
            }
        }

        pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].bEntryUsedFlag = OMX_TRUE;
        pInBufHeader = pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].pInBufHeader;
        eError = pVideoDecoderComPvt->fpDioDequeue(dataSyncHandle, OMX_VIDEODECODER_INPUT_PORT,
                                                   (OMX_PTR)(pInBufHeader));

        dataSyncDesc->size = sizeof(XDM_DataSyncDesc);
        dataSyncDesc->scatteredBlocksFlag = XDAS_FALSE;
        dataSyncDesc->baseAddr = (XDAS_Int32 *)((XDAS_Int32)pInBufHeader->pBuffer + pInBufHeader->nOffset);
        dataSyncDesc->numBlocks = 1;
        dataSyncDesc->varBlockSizesFlag = XDAS_TRUE;
        pVideoDecoderComPvt->nSliceSizes[0] = pInBufHeader->nFilledLen - pInBufHeader->nOffset;
        dataSyncDesc->blockSizes = &(pVideoDecoderComPvt->nSliceSizes[0]);

        if( pInBufHeader->nFlags & OMX_BUFFERFLAG_EOS ) {
            pVideoDecoderComPvt->bIsFlushRequired = OMX_TRUE;
        }

        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "DS Params=%d %d %x %d %d %x %d\n", dataSyncDesc->size, dataSyncDesc->scatteredBlocksFlag, dataSyncDesc->baseAddr,
                           dataSyncDesc->numBlocks, dataSyncDesc->varBlockSizesFlag, dataSyncDesc->blockSizes, dataSyncDesc->blockSizes[0]);
    }

EXIT:
    return (eError);
}

/* ==========================================================================*/
/**
 * @fn _OMX_VIDDEC_DataSync_PutBuffer(XDM_DataSyncHandle dataSyncHandle, XDM_DataSyncDesc *dataSyncDesc)
 *                     This method is called by the codec in slice mode to release the
 *                     consumed, partial input data
 *
 *  @param [in] dataSyncHandle  : pointer to the OMX Component.
 *  @param [in] dataSyncDesc    : Data sync descriptor used by codec
 *
 */
/* ==========================================================================*/
XDAS_Int32 _OMX_VIDDEC_DataSync_PutBuffer(XDM_DataSyncHandle dataSyncHandle, XDM_DataSyncDesc *dataSyncDesc)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMX_VIDEODECODER_PVTTYPE   *pVideoDecoderComPvt = NULL;
    OMX_BUFFERHEADERTYPE       *pInBufHeader = NULL;
    OMX_U32                     i;

    /* Check the input parameters */
    OMX_BASE_REQUIRE((dataSyncHandle != NULL) && (dataSyncDesc != NULL), OMX_ErrorBadParameter);

    pHandle = (OMX_COMPONENTTYPE *)dataSyncHandle;
    pVideoDecoderComPvt
        = (OMX_VIDEODECODER_PVTTYPE *)pHandle->pComponentPrivate;

    for( i=0; i < OMX_VIDDEC_DATASYNC_DEFAULT_IN_BUF_COUNT; i++ ) {
        pInBufHeader = pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].pInBufHeader;
        TIMM_OSAL_TraceExt(TIMM_OSAL_TRACEGRP_OMXVIDEODEC, "DataSync_PutBuffer %x InBufHeader=%x pBuffer=%x\n", dataSyncDesc->baseAddr, pInBufHeader, pInBufHeader->pBuffer);
        if( pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].bEntryUsedFlag == OMX_TRUE && dataSyncDesc->baseAddr == (XDAS_Int32 *)pInBufHeader->pBuffer ) {
            break;
        }
    }

    pInBufHeader = pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].pInBufHeader;
    pVideoDecoderComPvt->fpDioSend(dataSyncHandle,
                                   OMX_VIDEODECODER_INPUT_PORT, pInBufHeader);
    pVideoDecoderComPvt->tDatasync_InBufHeader_table_entries[i].bEntryUsedFlag = OMX_FALSE;
EXIT:
    return (eError);
}

