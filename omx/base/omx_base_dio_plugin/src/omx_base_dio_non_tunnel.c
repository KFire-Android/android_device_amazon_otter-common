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

#include <omx_base_internal.h>
#include <timm_osal_interfaces.h>
#include <omx_base_utils.h>
#include <omx_base_dio_plugin.h>
#include <memplugin.h>
#include <OMX_TI_Common.h>

#define LINUX_PAGE_SIZE 4096


/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*---------------- function prototypes  -------------------------- */
extern OMX_BASE_EXTRA_DATA_TABLE    OMX_BASE_ExtraDataTable[];
extern OMX_ERRORTYPE _OMX_BASE_DIO_DefaultGetTotalBufferSize(OMX_HANDLETYPE hComponent,
                                                             OMX_U32 nPortIndex,
                                                             OMX_PTR pDioBufHeader);
extern OMX_ERRORTYPE _OMX_BASE_DIO_DefaultUpdatePacketExtraDataPointer(OMX_HANDLETYPE hComponent,
                                                                       OMX_U32 nPortIndex,
                                                                       OMX_PTR pDioBufHeader);

/*******************************************************************
 * PUBLIC DECLARATIONS: defined here, used elsewhere
 *******************************************************************/
/*---------------- data declarations    -------------------------- */
/*---------------- function prototypes  -------------------------- */

/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------------   data declarations  -------------------------- */

typedef struct DIO_NonTunnel_Attrs {
    OMX_BASE_DIO_CreateParams sCreateParams;
    OMX_U32                   nFlags;
    OMX_PTR                   pPipeHandle;
    OMX_PTR                   pBufPool;
    OMX_PTR                   pHdrPool;
    OMX_PTR                   pOutPrivatePool;
    OMX_PTR                   pPlatformPrivatePool;
}DIO_NonTunnel_Attrs;

/*---------------- function prototypes  ------------------------- */

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Open (OMX_HANDLETYPE handle,
                                             OMX_BASE_DIO_OpenParams *pParams);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Close (OMX_HANDLETYPE handle);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Queue (OMX_HANDLETYPE handle,
                                              OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Dequeue (OMX_HANDLETYPE handle,
                                                OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Send (OMX_HANDLETYPE handle,
                                             OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Cancel (OMX_HANDLETYPE handle,
                                               OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Dup (OMX_HANDLETYPE handle,
                                            OMX_PTR pBuffHeader,
                                            OMX_PTR pDupBuffHeader);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Control (OMX_HANDLETYPE handle,
                                                OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                                                OMX_PTR pParams);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Util (OMX_HANDLETYPE handle,
                                             OMX_BASE_DIO_UTIL_TYPE nUtilType,
                                             OMX_PTR pParams);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Getcount (OMX_HANDLETYPE handle,
                                                 OMX_U32 *pCount);

static OMX_ERRORTYPE OMX_DIO_NonTunnel_Deinit (OMX_HANDLETYPE handle);


/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Init :
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_DIO_NonTunnel_Init(OMX_HANDLETYPE handle,
                                     OMX_PTR pCreateParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs       *pContext = NULL;

    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)(((OMX_BASE_DIO_CreateParams *)(pCreateParams))->hComponent);
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Init");

    /* creating memory for DIO object private area */
    hDIO->pDIOPrivate = OMX_BASE_Malloc(sizeof(DIO_NonTunnel_Attrs), pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(NULL != hDIO->pDIOPrivate, OMX_ErrorInsufficientResources);

    TIMM_OSAL_Memset(hDIO->pDIOPrivate, 0x0, sizeof(DIO_NonTunnel_Attrs));

    hDIO->open        =  OMX_DIO_NonTunnel_Open;
    hDIO->close       =  OMX_DIO_NonTunnel_Close;
    hDIO->queue       =  OMX_DIO_NonTunnel_Queue;
    hDIO->dequeue     =  OMX_DIO_NonTunnel_Dequeue;
    hDIO->send        =  OMX_DIO_NonTunnel_Send;
    hDIO->cancel      =  OMX_DIO_NonTunnel_Cancel;
    hDIO->dup         =  OMX_DIO_NonTunnel_Dup;
    hDIO->control     =  OMX_DIO_NonTunnel_Control;
    hDIO->util        =  OMX_DIO_NonTunnel_Util;
    hDIO->getcount    =  OMX_DIO_NonTunnel_Getcount;
    hDIO->deinit      =  OMX_DIO_NonTunnel_Deinit;

    pContext = hDIO->pDIOPrivate;
    /* Initialize private data */
    pContext->sCreateParams = *(OMX_BASE_DIO_CreateParams *)pCreateParams;

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Init", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Deinit :
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Deinit (OMX_HANDLETYPE handle)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs       *pContext = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Deinit");
    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    if( NULL != pContext ) {
        TIMM_OSAL_Free(pContext);
        pContext = NULL;
    }
    /*EXIT:*/
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Deinit", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Open :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Open (OMX_HANDLETYPE handle,
                                             OMX_BASE_DIO_OpenParams *pParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE        tStatus = TIMM_OSAL_ERR_NONE;
    MEMPLUGIN_ERRORTYPE        eMemError = MEMPLUGIN_ERROR_NONE;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_U32                    i = 0;
    OMX_U32                    nPortIndex = 0;
    OMX_U32                    nStartPortNumber = 0;
    OMX_U8                    *pTmpBuffer = NULL;
    OMX_U32                    nLocalComBuffers = 0;
    MEMPLUGIN_BUFFER_PARAMS   *pBufParams = NULL;
    MEMPLUGIN_INIT_PARAMS     *pInitParams = NULL;
    OMX_BASE_2D_MEM_PARAMS    *pLocal2DParams = NULL;
    OMX_PTR                   *pBufArr = NULL;

    OMX_COMPONENTTYPE      *pComp = NULL;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Open");

    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;

    pComp = (OMX_COMPONENTTYPE *)(pContext->sCreateParams.hComponent);
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    nPortIndex = pPort->sPortDef.nPortIndex;

    /* supplier should allocate both the buffer and buffer headers
      non supplier should allocate only the buffer headers  */
    pPort->pBufferlist = (OMX_BUFFERHEADERTYPE * *)OMX_BASE_Malloc(
        (pPort->sPortDef.nBufferCountActual *
         sizeof(OMX_BUFFERHEADERTYPE *)),
        pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(NULL != pPort->pBufferlist,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(pPort->pBufferlist, 0, (pPort->sPortDef.nBufferCountActual
                                             * sizeof(OMX_BUFFERHEADERTYPE *)));

    /* create a buffer header pool */
    pContext->pHdrPool = (OMX_PTR)OMX_BASE_Malloc(
        sizeof(OMX_BUFFERHEADERTYPE) *
        (pPort->sPortDef.nBufferCountActual),
        pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(NULL != pContext->pHdrPool,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(pContext->pHdrPool, 0, sizeof(OMX_BUFFERHEADERTYPE) *
                     (pPort->sPortDef.nBufferCountActual));
    if( pPort->sPortDef.eDir == OMX_DirOutput ) {
        /*Allocating pool for output port private*/
        pContext->pOutPrivatePool = (OMX_PTR)OMX_BASE_Malloc(
            pPort->sPortDef.nBufferCountActual *
            sizeof(OMX_BASE_OUTPUTPORTPRIVATE),
            pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(NULL != pContext->pOutPrivatePool,
                        OMX_ErrorInsufficientResources);
        /*Setting output port pvt pool to 0*/
        TIMM_OSAL_Memset(pContext->pOutPrivatePool, 0,
                         pPort->sPortDef.nBufferCountActual *
                         sizeof(OMX_BASE_OUTPUTPORTPRIVATE));
    }

    pContext->pPlatformPrivatePool = (OMX_PTR)OMX_BASE_Malloc(
        pPort->sPortDef.nBufferCountActual *
        sizeof(OMX_TI_PLATFORMPRIVATE),
        pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(NULL != pContext->pPlatformPrivatePool,
                    OMX_ErrorInsufficientResources);

    /*Setting platform port pvt pool to 0*/
    TIMM_OSAL_Memset(pContext->pPlatformPrivatePool, 0,
                     pPort->sPortDef.nBufferCountActual *
                     sizeof(OMX_TI_PLATFORMPRIVATE));

    if( PORT_IS_SUPPLIER(pPort)) {
        //Setting up heap
        if( pBaseComPvt->tDerToBase.
            pPortProperties[nPortIndex - nStartPortNumber]->hBufHeapPerPort ==
            NULL && pBaseComPvt->tDerToBase.hDefaultHeap == NULL ) {
            pInitParams = NULL;
        } else {
            pInitParams = OMX_BASE_Malloc(sizeof(MEMPLUGIN_INIT_PARAMS),
                                          pBaseComPvt->tDerToBase.hDefaultHeap);
            OMX_BASE_ASSERT(NULL != pInitParams,
                            OMX_ErrorInsufficientResources);
            if( pBaseComPvt->tDerToBase.
                pPortProperties[nPortIndex - nStartPortNumber]->
                hBufHeapPerPort == NULL ) {
                pInitParams->hHeapHandle =
                    pBaseComPvt->tDerToBase.hDefaultHeap;
            } else {
                pInitParams->hHeapHandle = pBaseComPvt->tDerToBase.
                                           pPortProperties[nPortIndex - nStartPortNumber]->
                                           hBufHeapPerPort;
            }
        }

        //Calling init
        switch( pBaseComPvt->tDerToBase.
                pPortProperties[nPortIndex - nStartPortNumber]->eBufMemoryType ) {
            case OMX_BASE_BUFFER_MEMORY_DEFAULT :
                eMemError = MemPlugin_Init("MEMPLUGIN_ION",&(pPort->pMemPluginHandle));
                OMX_BASE_ASSERT(MEMPLUGIN_ERROR_NONE == eMemError,
                                OMX_ErrorInsufficientResources);

	        eMemError = MemPlugin_Open(pPort->pMemPluginHandle,&(pPort->nMemmgrClientDesc));
                OMX_BASE_ASSERT(MEMPLUGIN_ERROR_NONE == eMemError,
                                OMX_ErrorInsufficientResources);
	        pPort->bMapBuffers = OMX_TRUE;
                break;

            default :
                OMX_BASE_ASSERT(OMX_FALSE, OMX_ErrorUndefined);
                //break;
        }

        //Setting up fields for calling configure
        nLocalComBuffers = pBaseComPvt->tDerToBase.
                           pPortProperties[nPortIndex - nStartPortNumber]->
                           nNumComponentBuffers;
        if( nLocalComBuffers != 1 && pBaseComPvt->tDerToBase.
            pPortProperties[nPortIndex - nStartPortNumber]->eBufMemoryType !=
            OMX_BASE_BUFFER_MEMORY_2D ) {
            //For non 2D buffers multiple component buffers not supported
            OMX_BASE_ASSERT(OMX_FALSE, OMX_ErrorBadParameter);
        }
        pBufParams = OMX_BASE_Malloc(sizeof(MEMPLUGIN_BUFFER_PARAMS) *
                                     nLocalComBuffers, pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(NULL != pBufParams, OMX_ErrorInsufficientResources);
        //Allocating array of buffer pointers
        pBufArr = OMX_BASE_Malloc(sizeof(OMX_PTR) * nLocalComBuffers,
                                  pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(NULL != pBufArr, OMX_ErrorInsufficientResources);

        pLocal2DParams = ((OMX_BASE_2D_MEM_PARAMS *)pBaseComPvt->tDerToBase.
                          pPortProperties[nPortIndex - nStartPortNumber]->
                          pBufParams);
        if( pLocal2DParams != NULL ) {
            for( i = 0; i < nLocalComBuffers; i++ ) {
                pBufParams[i].nHeight = pLocal2DParams[i].nHeight;
                pBufParams[i].nLength = pLocal2DParams[i].nWidth;
                //For 2D buffers alignment does not make sense
                pBufParams[i].nAlignment = 0;

                switch( pLocal2DParams[i].eAccessMode ) {
                    case OMX_BASE_MEMORY_ACCESS_8BIT :
                        pBufParams[i].eAccessMode = MEMPLUGIN_ACCESS_MODE_8Bit;
                        break;
                    case OMX_BASE_MEMORY_ACCESS_16BIT :
                        pBufParams[i].eAccessMode = MEMPLUGIN_ACCESS_MODE_16Bit;
                        break;
                    case OMX_BASE_MEMORY_ACCESS_32BIT :
                        pBufParams[i].eAccessMode = MEMPLUGIN_ACCESS_MODE_32Bit;
                        break;
                    //Ignore not supported for 2D buffers
                    case OMX_BASE_MEMORY_ACCESS_IGNORE :
                    default :
                        OMX_BASE_ASSERT(OMX_FALSE, OMX_ErrorUndefined);
                        //break;
                }
            }
        } else {
            /*For 1D buffers access mode and height is fixed. Also there is only
            one buffer*/
            pBufParams[0].eAccessMode = MEMPLUGIN_ACCESS_MODE_DontCare;
            pBufParams[0].nHeight = 1;
            pBufParams[0].nLength = pParams->nBufSize;
            pBufParams[0].nAlignment = pPort->sPortDef.nBufferAlignment;
        }

    }

    /* update buffer list with buffer and buffer headers */
    for( i = 0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
        pPort->pBufferlist[i] = (OMX_BUFFERHEADERTYPE *)(pContext->pHdrPool) + i;
        OMX_BASE_INIT_STRUCT_PTR(pPort->pBufferlist[i], OMX_BUFFERHEADERTYPE);

        pPort->pBufferlist[i]->pPlatformPrivate =
            (OMX_TI_PLATFORMPRIVATE *)(pContext->pPlatformPrivatePool) + i;
        ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->
         pPlatformPrivate)->nSize = sizeof(OMX_TI_PLATFORMPRIVATE);
        /*TBD - Hack for now, use memplugin later*/
        if( pBaseComPvt->tDerToBase.pPortProperties[nPortIndex -
                                                    nStartPortNumber]->nNumMetaDataFields != 0 ) {
            /*Allocate meta data buffer*/
            if( pBaseComPvt->tDerToBase.pPortProperties
                [nPortIndex - nStartPortNumber]->eBufMemoryType ==
                OMX_BASE_BUFFER_MEMORY_DEFAULT ) {
                ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->
                 pPlatformPrivate)->pMetaDataBuffer = (OMX_PTR)OMX_BASE_Malloc(
                    pPort->nMetaDataSize,
                    pBaseComPvt->tDerToBase.hDefaultHeap);
                OMX_BASE_ASSERT(((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->
                                 pPlatformPrivate)->pMetaDataBuffer != NULL,
                                OMX_ErrorInsufficientResources);
                ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->
                 pPlatformPrivate)->nMetaDataSize = pPort->nMetaDataSize;
            } else {
                ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->
                 pPlatformPrivate)->nMetaDataSize = pPort->nMetaDataSize;
            }
        }
        /*This has to be updated at each dequeue call so that for
        multiprocessor tunneling, address of the orig header on that
        processor is stored*/
        ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pExtendedPlatformPrivate =
            pPort->pBufferlist[i];
        if( pPort->sPortDef.eDir == OMX_DirOutput ) {
            /*Each buffer header has its o/p port private and each o/p port private has its mutex created*/
            pPort->pBufferlist[i]->pOutputPortPrivate =
                (OMX_BASE_OUTPUTPORTPRIVATE *)(pContext->pOutPrivatePool) + i;
            tStatus = TIMM_OSAL_MutexCreate(&(((OMX_BASE_OUTPUTPORTPRIVATE *)
                                               pPort->pBufferlist[i]->
                                               pOutputPortPrivate)->pBufCountMutex));
            OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                            OMX_ErrorInsufficientResources);
        }

        if( PORT_IS_SUPPLIER(pPort)) {
	    MEMPLUGIN_BUFFER_PARAMS_INIT(pPort->sBufferParams);
            OMX_U32 nSize = (pParams->nBufSize + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE - 1);
            pPort->sBufferParams.nWidth = nSize;
            pPort->sBufferParams.bMap = pPort->bMapBuffers;

            eMemError = MemPlugin_Alloc(pPort->pMemPluginHandle,pPort->nMemmgrClientDesc,&pPort->sBufferParams,&pPort->sBufferProp);
            OMX_BASE_ASSERT(MEMPLUGIN_ERROR_NONE == eMemError,
                            OMX_ErrorInsufficientResources);

            pTmpBuffer = pPort->sBufferProp.sBuffer_accessor.pBufferMappedAddress;
            pPort->pBufferlist[i]->pBuffer = pTmpBuffer;
            TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE,
                               "\nOMX_BASE: 1st buffer allocated = 0x%x\n", pTmpBuffer);
            if( nLocalComBuffers == 2 ) {
                TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE,
                                   "\nOMX_BASE: 2nd buffer allocated = 0x%x\n", pBufArr[1]);
                ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->
                 pPlatformPrivate)->pAuxBuf1 = pBufArr[1];
            }
        }
    }

    /* create a fixed size OS pipe */
    tStatus = TIMM_OSAL_CreatePipe(&pContext->pPipeHandle,
                                   (sizeof(OMX_BUFFERHEADERTYPE *) *
                                    pPort->sPortDef.nBufferCountActual),
                                   sizeof(OMX_BUFFERHEADERTYPE *), 1);
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorInsufficientResources);

    pPort->nCachedNumBuffers = pPort->sPortDef.nBufferCountActual;

EXIT:
    if( pInitParams != NULL ) {
        TIMM_OSAL_Free(pInitParams);
        pInitParams = NULL;
    }
    if( pBufParams != NULL ) {
        TIMM_OSAL_Free(pBufParams);
        pBufParams = NULL;
    }
    if( pBufArr != NULL ) {
        TIMM_OSAL_Free(pBufArr);
        pBufArr = NULL;
    }
    if( OMX_ErrorNone != eError ) {
        OMX_DIO_NonTunnel_Close(handle);
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Open", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Close :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Close (OMX_HANDLETYPE handle)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE        tStatus = TIMM_OSAL_ERR_NONE;
    MEMPLUGIN_ERRORTYPE        eMemError = MEMPLUGIN_ERROR_NONE;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_U32                    i = 0, nPortIndex = 0, nStartPortNumber = 0, nCompBufs = 0;
    OMX_PTR                    pTmpBuffer = NULL;
    OMX_COMPONENTTYPE         *pComp = NULL;
    OMX_BASE_PRIVATETYPE      *pBaseComPvt = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Close");
    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    OMX_BASE_ASSERT(pContext != NULL, OMX_ErrorNone);

    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    if( pPort ) {
        pComp = (OMX_COMPONENTTYPE *)(pContext->sCreateParams.hComponent);
        if( pComp ) {
            pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
        }
        if( pBaseComPvt ) {
            nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
            nPortIndex = pPort->sPortDef.nPortIndex;
            nCompBufs = pBaseComPvt->tDerToBase.pPortProperties
                        [nPortIndex - nStartPortNumber]->nNumComponentBuffers;
        }
        if( pPort->pBufferlist ) {
            /*Deleting mutexes and output port pvt pool*/
            if( pPort->sPortDef.eDir == OMX_DirOutput ) {
                for( i = 0; i < pPort->nCachedNumBuffers; i++ ) {
                    if( pPort->pBufferlist[i]->pOutputPortPrivate ) {
                        if(((OMX_BASE_OUTPUTPORTPRIVATE *)
                            pPort->pBufferlist[i]->pOutputPortPrivate)->
                           pBufCountMutex ) {
                            tStatus = TIMM_OSAL_MutexDelete
                                          (((OMX_BASE_OUTPUTPORTPRIVATE *)
                                            pPort->pBufferlist[i]->
                                            pOutputPortPrivate)->pBufCountMutex);
                            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                                eError = OMX_ErrorUndefined;
                            }
                        }
                    }
                }
            }
            if( pBaseComPvt ) {
                if( pBaseComPvt->tDerToBase.pPortProperties[nPortIndex -
                                                            nStartPortNumber]->nNumMetaDataFields != 0 ) {

                    for( i = 0; i < pPort->nCachedNumBuffers; i++ ) {
                        if( pPort->pBufferlist[i] ) {
                            if( pPort->pBufferlist[i]->pPlatformPrivate ) {
                                if(((OMX_TI_PLATFORMPRIVATE *)pPort->
                                    pBufferlist[i]->pPlatformPrivate)->
                                   pMetaDataBuffer ) {
                                    if( pBaseComPvt->tDerToBase.pPortProperties
                                        [nPortIndex - nStartPortNumber]->
                                        eBufMemoryType ==
                                        OMX_BASE_BUFFER_MEMORY_DEFAULT ) {
                                        TIMM_OSAL_Free(((OMX_TI_PLATFORMPRIVATE *)
                                                        pPort->pBufferlist[i]->
                                                        pPlatformPrivate)->
                                                       pMetaDataBuffer);
                                    } 
                                    ((OMX_TI_PLATFORMPRIVATE *)pPort->
                                     pBufferlist[i]->pPlatformPrivate)->
                                    pMetaDataBuffer = NULL;
                                }
                            }
                        }
                    }
                }
            }
            if( PORT_IS_SUPPLIER(pPort)) {
                for( i = 0; i < pPort->nCachedNumBuffers; i++ ) {

                    if( pPort->pBufferlist[i] ) {
                        /*Caling free on the main buffer*/
                        pTmpBuffer = pPort->pBufferlist[i]->pBuffer;
                        TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE,
                                           "\nOMX_BASE: Calling free on 0x%x\n", pTmpBuffer);
                        if( pTmpBuffer ) {
                            eMemError = MemPlugin_Free(pPort->pMemPluginHandle, pPort->nMemmgrClientDesc,&pPort->sBufferParams,&pPort->sBufferProp);
                            if( eMemError != MEMPLUGIN_ERROR_NONE ) {
                                eError = OMX_ErrorUndefined;
                            }
                        }
                        if( nCompBufs == 2 ) {
                            OMX_BASE_GetUVBuffer(pComp, nPortIndex,
                                                 pPort->pBufferlist[i], &pTmpBuffer);
                            TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE,
                                               "\nOMX_BASE: Calling free on 0x%x\n", pTmpBuffer);
                            if( pTmpBuffer ) {
                                eMemError = MemPlugin_Free(pPort->pMemPluginHandle, pPort->nMemmgrClientDesc,&pPort->sBufferParams,&pPort->sBufferProp);
                                if( eMemError != MEMPLUGIN_ERROR_NONE ) {
                                    eError = OMX_ErrorUndefined;
                                }
                            }
                        }
                    }
                }
            }
            /* freeup the buffer list */
            TIMM_OSAL_Free(pPort->pBufferlist);
            pPort->pBufferlist = NULL;
        }
        if( PORT_IS_SUPPLIER(pPort)) {
            if( pPort->pMemPluginHandle ) {
                eMemError = MemPlugin_DeInit(pPort->pMemPluginHandle);
                if( eMemError != MEMPLUGIN_ERROR_NONE ) {
                    eError = OMX_ErrorUndefined;
                }
            }
        }
    }
    if( pContext->pOutPrivatePool ) {
        TIMM_OSAL_Free(pContext->pOutPrivatePool);
    }
    pContext->pOutPrivatePool = NULL;
    if( pContext->pPlatformPrivatePool ) {
        TIMM_OSAL_Free(pContext->pPlatformPrivatePool);
    }
    pContext->pPlatformPrivatePool = NULL;
    if( pContext->pHdrPool ) {
        TIMM_OSAL_Free(pContext->pHdrPool);
    }
    pContext->pHdrPool = NULL;
    /* delete a OS pipe */
    if( pContext->pPipeHandle != NULL ) {
        tStatus = TIMM_OSAL_DeletePipe(pContext->pPipeHandle);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
        pContext->pPipeHandle = NULL;
    }
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Close", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Queue :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Queue (OMX_HANDLETYPE handle,
                                              OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE                 eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE           tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE      *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs          *pContext = NULL;
    OMX_BASE_PORTTYPE            *pPort = NULL;
    OMX_BUFFERHEADERTYPE         *pOMXBufHeader;
    OMX_BASE_OUTPUTPORTPRIVATE   *pOutPvt;
    OMX_COMPONENTTYPE            *pComp;
    OMX_BASE_PRIVATETYPE         *pBaseComPvt;
    OMX_BASE_INTERNALTYPE        *pBaseComInt;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Queue");
    pOMXBufHeader = (OMX_BUFFERHEADERTYPE *) pBuffHeader;
    pOutPvt = (OMX_BASE_OUTPUTPORTPRIVATE *)(pOMXBufHeader->pOutputPortPrivate);

    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = pContext->sCreateParams.pPort;

    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    if( pPort->sPortDef.eDir == OMX_DirInput ) {
        tStatus = TIMM_OSAL_WriteToPipe(pContext->pPipeHandle, &pBuffHeader,
                                        sizeof(pBuffHeader), TIMM_OSAL_SUSPEND);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
    } else {
        if( pOutPvt->nDupBufCount == 0 ) {
            /* queue the buffer into the datapipe */
            tStatus = TIMM_OSAL_WriteToPipe(pContext->pPipeHandle, &pBuffHeader,
                                            sizeof(pBuffHeader), TIMM_OSAL_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
        }
        /*For Duped buffers only*/
        else {
            TIMM_OSAL_MutexObtain(pOutPvt->pBufCountMutex, TIMM_OSAL_SUSPEND);
            if( pOutPvt->nDupBufCount == 1 ) {
                pOutPvt->nDupBufCount = 0;
                tStatus = TIMM_OSAL_WriteToPipe(pContext->pPipeHandle,
                                                &pBuffHeader, sizeof(pBuffHeader),
                                                TIMM_OSAL_SUSPEND);
                if( TIMM_OSAL_ERR_NONE != tStatus ) {
                    //Ensure that the mutex is released even if there is an error
                    TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
                }
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
            } else {
                --(pOutPvt->nDupBufCount);
            }
            /*Send callback if client wants to be notified on decrease and ref count
            is equal to the client request or client request is 0 (notify always)*/
            if((pPort->sBufRefNotify.bNotifyOnDecrease) && ((pPort->
                                                             sBufRefNotify.nCountForNotification == pOutPvt->nDupBufCount) ||
                                                            (pPort->sBufRefNotify.nCountForNotification == 0))) {
                eError = pBaseComInt->sAppCallbacks.EventHandler(
                    (OMX_HANDLETYPE)pComp,
                    pComp->pApplicationPrivate,
                    (OMX_EVENTTYPE)OMX_TI_EventBufferRefCount,
                    (OMX_U32)pBuffHeader,
                    pOutPvt->nDupBufCount, NULL);
            }
            TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
        }
    }
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Queue", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Dequeue :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Dequeue (OMX_HANDLETYPE handle,
                                                OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE        tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort;
    TIMM_OSAL_U32              actualSize = 0;
    OMX_BUFFERHEADERTYPE      *pOrigOMXBufHeader = NULL;
    OMX_PTR                    pRunningPtr = NULL;
    OMX_COMPONENTTYPE         *pComp = NULL;
    OMX_BASE_PRIVATETYPE      *pBaseComPvt = NULL;
    OMX_OTHER_EXTRADATATYPE   *pExtra = NULL, *pExtraStart = NULL;
    OMX_U8                    *pTmp = NULL;
    OMX_U32                    nBitField = 0, nLocalBitField = 0, i = 0, nMask = 1, nNumFields = 0;
    OMX_BOOL                   bFound;
    OMX_S32                    nPosInTable;
    OMX_U32                    nPortIndex, nTimeout;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Dequeue");
    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    nPortIndex = pPort->sPortDef.nPortIndex;
    nTimeout = pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->nTimeoutForDequeue;
    /* dequeue the buffer from the data pipe */
    tStatus = TIMM_OSAL_ReadFromPipe(pContext->pPipeHandle, &pOrigOMXBufHeader,
                                     sizeof(pOrigOMXBufHeader), &actualSize, nTimeout);
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
    /*This has to be updated at each dequeue call so that for multiprocessor
    tunneling, address of the orig header on that processor is stored*/
    ((OMX_TI_PLATFORMPRIVATE *)pOrigOMXBufHeader->pPlatformPrivate)->
    pExtendedPlatformPrivate = pOrigOMXBufHeader;
    /*Cancel the buffer and return warning so that derived component may call
    GetAttribute*/
    if( pOrigOMXBufHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG ) {
        /*Reset codec config flag on o/p port*/
        if( OMX_DirOutput == pPort->sPortDef.eDir ) {
            pOrigOMXBufHeader->nFlags &= (~OMX_BUFFERFLAG_CODECCONFIG);
        } else {
            tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pPipeHandle, &pOrigOMXBufHeader,
                                                   sizeof(pOrigOMXBufHeader), TIMM_OSAL_NO_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
            eError = (OMX_ERRORTYPE)OMX_TI_WarningAttributePending;
            goto EXIT;
        }
    }
    nNumFields = pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                 nNumMetaDataFields;
    /*If derived component has an extended header with metadata fields*/
    if( nNumFields != 0 ) {
        pRunningPtr = (OMX_PTR)((OMX_U32)pBuffHeader +
                                sizeof(OMX_BUFFERHEADERTYPE));
        nBitField = *(OMX_U32 *)pRunningPtr;
        /*Derived comp is expecting some meta data*/
        if( nBitField != 0 ) {
            /*Moving to 1st extra data in header*/
            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr + sizeof(OMX_U32));
            /*For o/p port only update the pointers for any packet data with
            valid memory from the buffer*/
            if( OMX_DirOutput == pPort->sPortDef.eDir ) {
                /*Reset extra data flag on o/p port*/
                if( pOrigOMXBufHeader->nFlags &
                    OMX_TI_BUFFERFLAG_DETACHEDEXTRADATA ) {
                    pOrigOMXBufHeader->nFlags &=
                        (~OMX_TI_BUFFERFLAG_DETACHEDEXTRADATA);
                }
                /*
                    pTmp = pOrigOMXBufHeader->pBuffer +
                           ((OMX_BUFFERHEADERTYPE*)pBuffHeader)->nOffset +
                           ((OMX_BUFFERHEADERTYPE*)pBuffHeader)->nFilledLen + 3;
                */
                pTmp = (OMX_U8 *)((OMX_U32)(((OMX_TI_PLATFORMPRIVATE *)
                                             pOrigOMXBufHeader->pPlatformPrivate)->
                                            pMetaDataBuffer) + 3);
                /*Moving to location for 1st extra data in buffer*/
                pExtra = (OMX_OTHER_EXTRADATATYPE *) (((OMX_U32) pTmp) & ~3);

                for( i = 0; i < nNumFields; i++ ) {
                    nPosInTable = _OMX_BASE_FindInTable(
                        pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                        pMetaDataFieldTypesArr[i]);
                    if( nPosInTable == -1 ) {
                        eError = OMX_ErrorNoMore;
                        goto EXIT;
                    }
                    /*Derived component is not expecting this field, so move on*/
                    if( !(nBitField & nMask)) {
                        pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                OMX_BASE_ExtraDataTable[nPosInTable].nSize);
                        nMask *= 2;
                        continue;
                    }
                    /*Derived component will use this field*/
                    else {
                        if( OMX_BASE_ExtraDataTable[nPosInTable].bPacketData ==
                            OMX_FALSE ) {
                            pExtra->nDataSize = OMX_BASE_ExtraDataTable[nPosInTable].nSize;
                            pExtra->nSize = (EXTRADATATYPE_SIZE) +
                                            OMX_BASE_ExtraDataTable[nPosInTable].nSize;
                            /*To take care of 4 byte alignment*/
                            pExtra->nSize = 4 * ((pExtra->nSize + 3) / 4);
                            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr + OMX_BASE_ExtraDataTable[nPosInTable].nSize);
                        } else {
                            //TIMM_OSAL_Memcpy(pRunningPtr, &(pExtra->data), sizeof(OMX_PTR));
                            *(OMX_U32 *)pRunningPtr = (OMX_U32)(pExtra->data);

                            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                    sizeof(OMX_U32));
                            pExtra->nDataSize = *(OMX_U32 *)pRunningPtr;
                            pExtra->nSize = (EXTRADATATYPE_SIZE) +
                                            pExtra->nDataSize;
                            pExtra->nSize = 4 * ((pExtra->nSize + 3) / 4);
                            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                    sizeof(OMX_U32));
                        }
                        pExtra = (OMX_OTHER_EXTRADATATYPE *) (((OMX_U8 *) pExtra) + pExtra->nSize);
                    }
                    nMask *= 2;
                }
            }
            /*For i/p port, transfer the extra data from the buffer to the
            header and update the bitfield if some extra data that the derived
            comp is expecting is not present*/
            else {
                nLocalBitField = nBitField;
                /*Buffer contains extra data*/
                if( pOrigOMXBufHeader->nFlags & OMX_TI_BUFFERFLAG_DETACHEDEXTRADATA ) {
                    /*Clear the flag*/
                    pOrigOMXBufHeader->nFlags &= (~OMX_TI_BUFFERFLAG_DETACHEDEXTRADATA);
                    /*
                    pTmp = pOrigOMXBufHeader->pBuffer +
                           pOrigOMXBufHeader->nOffset +
                           pOrigOMXBufHeader->nFilledLen + 3;
                    */
                    pTmp = (OMX_U8 *)((OMX_U32)(((OMX_TI_PLATFORMPRIVATE *)
                                                 pOrigOMXBufHeader->pPlatformPrivate)->
                                                pMetaDataBuffer) + 3);
                    pExtraStart = (OMX_OTHER_EXTRADATATYPE *)(((OMX_U32) pTmp) & ~3);

                    for( i = 0; i < nNumFields; i++ ) {
                        nPosInTable = _OMX_BASE_FindInTable(
                            pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                            pMetaDataFieldTypesArr[i]);
                        if( nPosInTable == -1 ) {
                            eError = OMX_ErrorNoMore;
                            goto EXIT;
                        }
                        /*Derived component is not expecting this field, so move on*/
                        if( !(nBitField & nMask)) {
                            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                    OMX_BASE_ExtraDataTable[nPosInTable].nSize);
                            nMask *= 2;
                            continue;
                        }
                        /*Derived component will use this field*/
                        else {
                            pExtra = pExtraStart;
                            bFound = OMX_FALSE;

                            while( pExtra->eType != OMX_ExtraDataNone ) {
                                /*Expected data was available in buffer*/
                                if( pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                                    pMetaDataFieldTypesArr[i] == pExtra->eType ) {
                                    bFound = OMX_TRUE;
                                    /*For packet data, update pointer in header and copy size*/
                                    if( OMX_BASE_ExtraDataTable[nPosInTable].
                                        bPacketData == OMX_TRUE ) {
                                        //TIMM_OSAL_Memcpy(pRunningPtr,
                                        //               &(pExtra->data),
                                        //             sizeof(OMX_PTR));
                                        *(OMX_U32 *)pRunningPtr = (OMX_U32)(pExtra->data);
                                        pRunningPtr = (OMX_PTR)(
                                            (OMX_U32)pRunningPtr +
                                            sizeof(OMX_PTR));
                                        TIMM_OSAL_Memcpy(pRunningPtr,
                                                         &(pExtra->nDataSize),
                                                         sizeof(OMX_U32));
                                        pRunningPtr = (OMX_PTR)(
                                            (OMX_U32)pRunningPtr -
                                            sizeof(OMX_PTR));
                                    }
                                    /*For other data, just copy to header*/
                                    else {
                                        TIMM_OSAL_Memcpy(pRunningPtr,
                                                         pExtra->data,
                                                         pExtra->nDataSize);
                                    }
                                    break;
                                }
                                pExtra = (OMX_OTHER_EXTRADATATYPE *)
                                         (((OMX_U8 *) pExtra) +
                                          pExtra->nSize);
                            }

                            /*Expected data was not available - update bitfield*/
                            if( !bFound ) {
                                nLocalBitField &= (~nMask);
                            }
                        }
                        pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                OMX_BASE_ExtraDataTable[nPosInTable].nSize);
                        nMask *= 2;
                    }
                }
                /*Buffer contains no extra data - update bit field and return*/
                else {
                    nLocalBitField = 0;
                }
                /*Copy the bit field to the header*/
                pRunningPtr = (OMX_PTR)((OMX_U32)pBuffHeader +
                                        sizeof(OMX_BUFFERHEADERTYPE));
                TIMM_OSAL_Memcpy(pRunningPtr, &nLocalBitField, sizeof(OMX_U32));
            }
        }
    }
    TIMM_OSAL_Memcpy(pBuffHeader, pOrigOMXBufHeader,
                     sizeof(OMX_BUFFERHEADERTYPE));

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Dequeue", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Send :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Send (OMX_HANDLETYPE handle,
                                             OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE                 eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE      *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs          *pContext = NULL;
    OMX_BASE_PORTTYPE            *pPort = NULL;
    OMX_COMPONENTTYPE            *pComp = NULL;
    OMX_BASE_PRIVATETYPE         *pBaseComPvt = NULL;
    OMX_CALLBACKTYPE             *pAppCallbacks = NULL;
    OMX_BUFFERHEADERTYPE         *pOMXBufHeader;
    OMX_BUFFERHEADERTYPE         *pOrigOMXBufHeader = NULL;
    OMX_PTR                       pRunningPtr = NULL;
    OMX_S32                       nPosInTable;
    OMX_U32                       nPortIndex, nBitField = 0, nMask = 1, i = 0, nNumFields = 0;
    OMX_OTHER_EXTRADATATYPE      *pExtra = NULL;
    OMX_U8                       *pTmp = NULL;
    OMX_BASE_OUTPUTPORTPRIVATE   *pOutPvt;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Send");
    pOMXBufHeader = (OMX_BUFFERHEADERTYPE *) pBuffHeader;
    pOrigOMXBufHeader = ((OMX_TI_PLATFORMPRIVATE *)pOMXBufHeader->
                         pPlatformPrivate)->pExtendedPlatformPrivate;

    TIMM_OSAL_Memcpy(pOrigOMXBufHeader, pBuffHeader, sizeof(OMX_BUFFERHEADERTYPE));

    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pAppCallbacks = (OMX_CALLBACKTYPE *)pContext->sCreateParams.pAppCallbacks;
    nPortIndex = pPort->sPortDef.nPortIndex;

    nNumFields = pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                 nNumMetaDataFields;
    if( OMX_DirOutput == pPort->sPortDef.eDir ) {
        if( nNumFields != 0 ) {
            /*
            pTmp = pOrigOMXBufHeader->pBuffer + pOrigOMXBufHeader->nOffset +
                   pOrigOMXBufHeader->nFilledLen + 3;
            */
            pTmp = (OMX_U8 *)((OMX_U32)(((OMX_TI_PLATFORMPRIVATE *)
                                         pOrigOMXBufHeader->pPlatformPrivate)->
                                        pMetaDataBuffer) + 3);
            pExtra = (OMX_OTHER_EXTRADATATYPE *) (((OMX_U32) pTmp) & ~3);

            pRunningPtr = (OMX_PTR)((OMX_U32)pBuffHeader +
                                    sizeof(OMX_BUFFERHEADERTYPE));
            nBitField = *(OMX_U32 *)pRunningPtr;
            /*Moving to 1st extra data in header*/
            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr + sizeof(OMX_U32));
            if( nBitField != 0 ) {
                pOrigOMXBufHeader->nFlags |= OMX_TI_BUFFERFLAG_DETACHEDEXTRADATA;

                for( i = 0; i < nNumFields; i++ ) {
                    nPosInTable = _OMX_BASE_FindInTable(
                        pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                        pMetaDataFieldTypesArr[i]);
                    if( nPosInTable == -1 ) {
                        eError = OMX_ErrorNoMore;
                        goto EXIT;
                    }
                    /*Derived component does not want to send this field, so move on*/
                    if( !(nBitField & nMask)) {
                        pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                OMX_BASE_ExtraDataTable[nPosInTable].nSize);
                        nMask *= 2;
                        continue;
                    }
                    /*Adding extra data*/
                    else {
                        pExtra->nVersion.s.nVersionMajor = 0x1;
                        pExtra->nVersion.s.nVersionMinor = 0x1;
                        pExtra->nVersion.s.nRevision     = 0x2;
                        pExtra->nVersion.s.nStep         = 0x0;
                        pExtra->nPortIndex = pPort->sPortDef.nPortIndex;
                        pExtra->eType = pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                                        pMetaDataFieldTypesArr[i];
                        if( OMX_BASE_ExtraDataTable[nPosInTable].bPacketData ==
                            OMX_FALSE ) {
                            pExtra->nDataSize = OMX_BASE_ExtraDataTable
                                                [nPosInTable].nSize;
                            pExtra->nSize = (EXTRADATATYPE_SIZE) +
                                            OMX_BASE_ExtraDataTable[nPosInTable].nSize;
                            /*To take care of 4 byte alignment*/
                            pExtra->nSize = 4 * ((pExtra->nSize + 3) / 4);
                            TIMM_OSAL_Memcpy(pExtra->data, pRunningPtr,
                                             pExtra->nDataSize);
                            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                    OMX_BASE_ExtraDataTable[nPosInTable].nSize);
                        } else {
                            if((OMX_U32)(pExtra->data) != *((OMX_U32 *)pRunningPtr)) {
                                /*The pointer for the extra data has moved
                                between dequeue and send - ERROR!!*/
                                eError = OMX_ErrorUndefined;
                                goto EXIT;
                            }
                            //printf("\npExtra->data = %x, *pRunningPtr = %x\n" , pExtra->data, *((OMX_U32*)pRunningPtr));

                            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                    sizeof(OMX_U32));
                            pExtra->nDataSize = *(OMX_U32 *)pRunningPtr;
                            pExtra->nSize = (EXTRADATATYPE_SIZE) +
                                            pExtra->nDataSize;
                            pExtra->nSize = 4 * ((pExtra->nSize + 3) / 4);
                            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                                    sizeof(OMX_U32));
                        }
                    }
                    pExtra = (OMX_OTHER_EXTRADATATYPE *)(((OMX_U8 *) pExtra) +
                                                         pExtra->nSize);
                    nMask *= 2;
                }

                /*For last extra data header with no data*/
                pExtra->nVersion.s.nVersionMajor = 0x1;
                pExtra->nVersion.s.nVersionMinor = 0x1;
                pExtra->nVersion.s.nRevision     = 0x2;
                pExtra->nVersion.s.nStep         = 0x0;
                pExtra->nPortIndex = pPort->sPortDef.nPortIndex;
                pExtra->eType = OMX_ExtraDataNone;
                pExtra->nDataSize = 0;
                pExtra->nSize = (sizeof(OMX_OTHER_EXTRADATATYPE));
                pExtra->nSize = 4 * ((pExtra->nSize + 3) / 4);
            }
        }
    }
    /* return the buffer back to the Application using EBD or FBD
     * depending on the direction of the port (input or output) */
    if( OMX_DirInput == pPort->sPortDef.eDir ) {
        eError = pAppCallbacks->EmptyBufferDone(pComp,
                                                pComp->pApplicationPrivate, pOrigOMXBufHeader);
    } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
        pOutPvt = (OMX_BASE_OUTPUTPORTPRIVATE *)
                  (pOrigOMXBufHeader->pOutputPortPrivate);
        TIMM_OSAL_MutexObtain(pOutPvt->pBufCountMutex, TIMM_OSAL_SUSPEND);
        if( pOutPvt->nDupBufCount > 1 ) {
            pOrigOMXBufHeader->nFlags |= OMX_TI_BUFFERFLAG_READONLY;
        }
        TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);

        eError = pAppCallbacks->FillBufferDone(pComp,
                                               pComp->pApplicationPrivate, pOrigOMXBufHeader);
    }
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Send", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Cancel :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Cancel (OMX_HANDLETYPE handle,
                                               OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE                 eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE           tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE      *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs          *pContext = NULL;
    OMX_BASE_PORTTYPE            *pPort = NULL;
    OMX_BUFFERHEADERTYPE         *pOMXBufHeader;
    OMX_BUFFERHEADERTYPE         *pOrigOMXBufHeader = NULL;
    OMX_BASE_OUTPUTPORTPRIVATE   *pOutPvt;
    OMX_COMPONENTTYPE            *pComp;
    OMX_BASE_PRIVATETYPE         *pBaseComPvt;
    OMX_BASE_INTERNALTYPE        *pBaseComInt;
    OMX_BOOL                      bCallProcess = OMX_FALSE;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Cancel");
    pOMXBufHeader = (OMX_BUFFERHEADERTYPE *) pBuffHeader;
    pOutPvt = (OMX_BASE_OUTPUTPORTPRIVATE *)(pOMXBufHeader->pOutputPortPrivate);
    pOrigOMXBufHeader = ((OMX_TI_PLATFORMPRIVATE *)pOMXBufHeader->
                         pPlatformPrivate)->pExtendedPlatformPrivate;

    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = pContext->sCreateParams.pPort;

    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    if( pPort->sPortDef.eDir == OMX_DirInput ) {
        TIMM_OSAL_Memcpy(pOrigOMXBufHeader, pBuffHeader, sizeof(OMX_BUFFERHEADERTYPE));

        tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pPipeHandle, &pOrigOMXBufHeader,
                                               sizeof(pOrigOMXBufHeader), TIMM_OSAL_NO_SUSPEND);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
    } else {
        if( pOutPvt->nDupBufCount == 0 ) {
            TIMM_OSAL_Memcpy(pOrigOMXBufHeader, pBuffHeader, sizeof(OMX_BUFFERHEADERTYPE));
            /* queue the buffer to the front */
            tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pPipeHandle, &pOrigOMXBufHeader,
                                                   sizeof(pOrigOMXBufHeader), TIMM_OSAL_NO_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
        }
        /*For Duped buffers only*/
        else {
            TIMM_OSAL_MutexObtain(pOutPvt->pBufCountMutex, TIMM_OSAL_SUSPEND);
            if( pOutPvt->nDupBufCount == 1 ) {
                pOutPvt->nDupBufCount = 0;
                TIMM_OSAL_Memcpy(pOrigOMXBufHeader, pBuffHeader, sizeof(OMX_BUFFERHEADERTYPE));
                tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pPipeHandle,
                                                       &pOrigOMXBufHeader, sizeof(pOrigOMXBufHeader),
                                                       TIMM_OSAL_NO_SUSPEND);
                if( TIMM_OSAL_ERR_NONE != tStatus ) {
                    //Ensure that the mutex is released even if there is an error
                    TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
                }
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
                bCallProcess = OMX_TRUE;
            } else {
                --(pOutPvt->nDupBufCount);
            }
            /*Send callback if client wants to be notified on decrease and ref count
            is equal to the client request or client request is 0 (notify always)*/
            if((pPort->sBufRefNotify.bNotifyOnDecrease) && ((pPort->
                                                             sBufRefNotify.nCountForNotification == pOutPvt->nDupBufCount) ||
                                                            (pPort->sBufRefNotify.nCountForNotification == 0))) {
                eError = pBaseComInt->sAppCallbacks.EventHandler(
                    (OMX_HANDLETYPE)pComp,
                    pComp->pApplicationPrivate,
                    (OMX_EVENTTYPE)OMX_TI_EventBufferRefCount,
                    (OMX_U32)pOrigOMXBufHeader,
                    pOutPvt->nDupBufCount, NULL);
            }
            TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
        }
    }
    if( bCallProcess ) {
        /*Calling process fn so that event to process the buffer can be generated*/
        pBaseComInt->fpInvokeProcessFunction(pComp, DATAEVENT);
    }
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Cancel", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Dup :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Dup (OMX_HANDLETYPE handle,
                                            OMX_PTR pBuffHeader,
                                            OMX_PTR pDupBuffHeader)
{
    OMX_ERRORTYPE                 eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE           tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE      *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs          *pContext = NULL;
    OMX_BASE_PORTTYPE            *pPort = NULL;
    OMX_BUFFERHEADERTYPE         *pOMXBuffheader;
    OMX_BASE_OUTPUTPORTPRIVATE   *pOutPvt;
    OMX_COMPONENTTYPE            *pComp = NULL;
    OMX_BASE_PRIVATETYPE         *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE        *pBaseComInt = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Dup");
    pOMXBuffheader = (OMX_BUFFERHEADERTYPE *)(pBuffHeader);
    pOutPvt = (OMX_BASE_OUTPUTPORTPRIVATE *)(pOMXBuffheader->pOutputPortPrivate);

    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;

    pComp = (OMX_COMPONENTTYPE *)(pContext->sCreateParams.hComponent);
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    if( pPort->sPortDef.eDir == OMX_DirInput ) {
        OMX_BASE_Error("\nDup not valid Input Port\n");
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    tStatus = TIMM_OSAL_MutexObtain(pOutPvt->pBufCountMutex, TIMM_OSAL_SUSPEND);
    OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                    OMX_ErrorInsufficientResources);
    if( pOutPvt->nDupBufCount == 0 ) {
        pOutPvt->nDupBufCount = 2;
    } else {
        pOutPvt->nDupBufCount++;
    }
    /*Send callback if client wants to be notified on increase and ref count
    is equal to the client request or client request is 0 (notify always)*/
    if((pPort->sBufRefNotify.bNotifyOnIncrease) && ((pPort->sBufRefNotify.
                                                     nCountForNotification == pOutPvt->nDupBufCount) || pPort->sBufRefNotify.
                                                    nCountForNotification == 0)) {
        eError = pBaseComInt->sAppCallbacks.EventHandler((OMX_HANDLETYPE)pComp,
                                                         pComp->pApplicationPrivate,
                                                         (OMX_EVENTTYPE)OMX_TI_EventBufferRefCount,
                                                         (OMX_U32)pOMXBuffheader,
                                                         pOutPvt->nDupBufCount, NULL);
    }
    TIMM_OSAL_Memcpy(pDupBuffHeader, pOMXBuffheader,
                     sizeof(OMX_BUFFERHEADERTYPE));
    tStatus = TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
    OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                    OMX_ErrorInsufficientResources);

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Dup", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Control :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Control (OMX_HANDLETYPE handle,
                                                OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                                                OMX_PTR pParams)
{
    OMX_ERRORTYPE                            eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE                      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE                 *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs                     *pContext = NULL;
    OMX_BASE_PORTTYPE                       *pPort = NULL;
    OMX_COMPONENTTYPE                       *pComp = NULL;
    OMX_BASE_PRIVATETYPE                    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE                   *pBaseComInt = NULL;
    OMX_CALLBACKTYPE                        *pAppCallbacks = NULL;
    OMX_BUFFERHEADERTYPE                    *pBuffHeader = NULL;
    OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR   *pAttrDesc = NULL;
    TIMM_OSAL_U32                            elementsInpipe = 0;
    TIMM_OSAL_U32                            actualSize = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Control");
    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    pAppCallbacks = (OMX_CALLBACKTYPE *)pContext->sCreateParams.pAppCallbacks;

    switch( nCmdType ) {
        /* Flush the queues or buffers on a port. Both Flush and Stop perform
           the same operation i.e. send all the buffers back to the client */
        case OMX_BASE_DIO_CTRLCMD_Stop :
        case OMX_BASE_DIO_CTRLCMD_Flush :
            /* return all buffers to the IL client using EBD/FBD depending
             * on the direction(input or output) of port */

            TIMM_OSAL_GetPipeReadyMessageCount(pContext->pPipeHandle, &elementsInpipe);

            while( elementsInpipe ) {
                TIMM_OSAL_ReadFromPipe(pContext->pPipeHandle, &pBuffHeader,
                                       sizeof(pBuffHeader), &actualSize, TIMM_OSAL_NO_SUSPEND);
                elementsInpipe--;
                if( OMX_DirInput == pPort->sPortDef.eDir ) {
                    eError = pAppCallbacks->EmptyBufferDone(pComp,
                                                            pComp->pApplicationPrivate, pBuffHeader);
                } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
                    pBuffHeader->nFilledLen = 0;
                    eError = pAppCallbacks->FillBufferDone(pComp,
                                                           pComp->pApplicationPrivate, pBuffHeader);
                }
            }

            break;

        case OMX_BASE_DIO_CTRLCMD_Start :
            /*If there are buffers in the pipe in case of pause to executing
              then start processing them*/
            TIMM_OSAL_GetPipeReadyMessageCount(pContext->pPipeHandle, &elementsInpipe);
            if( elementsInpipe ) {
                pBaseComInt->fpInvokeProcessFunction(pComp, DATAEVENT);
            } else {
                OMX_BASE_Trace(" Nothing to do ");
            }
            break;

        case OMX_BASE_DIO_CTRLCMD_GetCtrlAttribute :
            /*Buffer should be available when calling GetAttribute*/
            tStatus = TIMM_OSAL_IsPipeReady(pContext->pPipeHandle);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);

            tStatus = TIMM_OSAL_ReadFromPipe(pContext->pPipeHandle, &pBuffHeader,
                                             sizeof(pBuffHeader), &actualSize, TIMM_OSAL_NO_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
            if( !(pBuffHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG)) {
                /*This buffer does not contain codec config*/
                eError = OMX_ErrorUndefined;
                /*Write the buffer back to front of pipe*/
                TIMM_OSAL_WriteToFrontOfPipe(pContext->pPipeHandle, &pBuffHeader,
                                             sizeof(pBuffHeader), TIMM_OSAL_SUSPEND);
                goto EXIT;
            }
            pAttrDesc = (OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR *)pParams;
            if( pAttrDesc->nSize < pBuffHeader->nFilledLen ) {
                tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pPipeHandle,
                                                       &pBuffHeader, sizeof(pBuffHeader), TIMM_OSAL_SUSPEND);
                pAttrDesc->nSize = pBuffHeader->nFilledLen;

                eError = (OMX_ERRORTYPE)OMX_TI_WarningInsufficientAttributeSize;
                goto EXIT;
            }
            pAttrDesc->nSize = pBuffHeader->nFilledLen;
            TIMM_OSAL_Memcpy(pAttrDesc->pAttributeData, pBuffHeader->pBuffer + pBuffHeader->nOffset,
                             pAttrDesc->nSize);

            /*Send the buffer back*/
            pBuffHeader->nFlags &= (~OMX_BUFFERFLAG_CODECCONFIG);
            if( OMX_DirInput == pPort->sPortDef.eDir ) {
                eError = pAppCallbacks->EmptyBufferDone(pComp,
                                                        pComp->pApplicationPrivate, pBuffHeader);
            } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
                /*So that the other port does not try to interpret any garbage
                data that may be present*/
                pBuffHeader->nFilledLen = 0;
                eError = pAppCallbacks->FillBufferDone(pComp,
                                                       pComp->pApplicationPrivate, pBuffHeader);
            }
            break;

        case OMX_BASE_DIO_CTRLCMD_SetCtrlAttribute :
            /*Buffer should be available when calling SetAttribute*/
            tStatus = TIMM_OSAL_IsPipeReady(pContext->pPipeHandle);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);

            tStatus = TIMM_OSAL_ReadFromPipe(pContext->pPipeHandle, &pBuffHeader,
                                             sizeof(pBuffHeader), &actualSize, TIMM_OSAL_NO_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);

            pBuffHeader->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
            pAttrDesc = (OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR *)pParams;
            if( pBuffHeader->nAllocLen < pAttrDesc->nSize ) {
                TIMM_OSAL_Error("Cannot send attribute data, size is too large");
                tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pPipeHandle,
                                                       &pBuffHeader, sizeof(pBuffHeader), TIMM_OSAL_SUSPEND);
                eError = OMX_ErrorInsufficientResources;
                goto EXIT;
            }
            pBuffHeader->nFilledLen = pAttrDesc->nSize;
            TIMM_OSAL_Memcpy(pBuffHeader->pBuffer, pAttrDesc->pAttributeData,
                             pAttrDesc->nSize);
            /*Send the buffer*/
            if( OMX_DirInput == pPort->sPortDef.eDir ) {
                eError = pAppCallbacks->EmptyBufferDone(pComp,
                                                        pComp->pApplicationPrivate, pBuffHeader);
            } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
                eError = pAppCallbacks->FillBufferDone(pComp,
                                                       pComp->pApplicationPrivate, pBuffHeader);
            }
            break;

        default :
            OMX_BASE_Trace(" Invalid Command received \n");
            eError = OMX_ErrorUnsupportedIndex;
            break;

    }

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Control", eError);
    return (eError);
}

OMX_ERRORTYPE OMX_DIO_NonTunnel_Util (OMX_HANDLETYPE handle,
                                      OMX_BASE_DIO_UTIL_TYPE nUtilType,
                                      OMX_PTR pParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_COMPONENTTYPE         *pComp = NULL;
    OMX_U32                    nPortIndex;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Util");
    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    nPortIndex = pPort->sPortDef.nPortIndex;

    switch( nUtilType ) {
        case OMX_BASE_DIO_UTIL_GetTotalBufferSize :
            eError = _OMX_BASE_DIO_DefaultGetTotalBufferSize((OMX_HANDLETYPE)pComp,
                                                             nPortIndex,
                                                             pParams);
            break;
        case OMX_BASE_DIO_UTIL_UpdatePacketExtraDataPointer :
            eError = _OMX_BASE_DIO_DefaultUpdatePacketExtraDataPointer(
                (OMX_HANDLETYPE)pComp,
                nPortIndex,
                pParams);
            break;
        default :
            OMX_BASE_Trace("Unsupported Index received ");
            eError = OMX_ErrorUnsupportedIndex;
            break;
    }

    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Util", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_NonTunnel_Getcount :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_NonTunnel_Getcount (OMX_HANDLETYPE handle,
                                                 OMX_U32 *pCount)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_NonTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Getcount");
    pContext = (DIO_NonTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    if( pPort->bEosRecd && pPort->sPortDef.eDir == OMX_DirInput ) {
        eError = (OMX_ERRORTYPE)OMX_TI_WarningEosReceived;
    }
    TIMM_OSAL_GetPipeReadyMessageCount(pContext->pPipeHandle, (TIMM_OSAL_U32*)pCount);

    /*EXIT:*/
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_DIO_NonTunnel_Getcount", eError);
    return (eError);
}

