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

#include "omx_base_internal.h"
#include <timm_osal_interfaces.h>
#include "omx_base_utils.h"
#include "omx_base_dio_plugin.h"

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

/*Sleep for this time if tunneled component has not transitioned yet*/
#define SLEEP_TIME 5
/*No. of times tunneled component is called if it has not transitioned yet*/
#define TIMEOUT_VAL 200

inline OMX_U32 max(OMX_U32 a, OMX_U32 b)
{
    return ((a > b) ? a : b);
}

typedef struct DIO_StdTunnel_Attrs {
    OMX_BASE_DIO_CreateParams sCreateParams;
    OMX_U32                   nFlags;
    OMX_PTR                   pBufPool;
    OMX_PTR                   pHdrPool;
    OMX_PTR                   pOutPrivatePool;
    OMX_PTR                   pPipeHandle;
    /* If tunneled component is not accepting buffers then buffers are stored
       in this pipe. Send command will 1st check this pipe before sending any
       buffers */
    OMX_PTR pWaitPipeHandle;
    OMX_PTR pPlatformPrivatePool;
}DIO_StdTunnel_Attrs;

/*---------------- function prototypes  ------------------------- */

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Open (OMX_HANDLETYPE handle,
                                             OMX_BASE_DIO_OpenParams *pParams);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Close (OMX_HANDLETYPE handle);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Queue (OMX_HANDLETYPE handle,
                                              OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Dequeue (OMX_HANDLETYPE handle,
                                                OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Send (OMX_HANDLETYPE handle,
                                             OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Cancel (OMX_HANDLETYPE handle,
                                               OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Dup (OMX_HANDLETYPE handle,
                                            OMX_PTR pBuffHeader,
                                            OMX_PTR pDupBuffHeader);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Control (OMX_HANDLETYPE handle,
                                                OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                                                OMX_PTR pParams);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Util (OMX_HANDLETYPE handle,
                                             OMX_BASE_DIO_UTIL_TYPE nUtilType,
                                             OMX_PTR pParams);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Getcount (OMX_HANDLETYPE handle,
                                                 OMX_U32 *pCount);

static OMX_ERRORTYPE OMX_DIO_StdTunnel_Deinit (OMX_HANDLETYPE handle);



/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Init :
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_DIO_StdTunnel_Init(OMX_HANDLETYPE handle,
                                     OMX_PTR pCreateParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;

    OMX_COMPONENTTYPE   *pComp = (OMX_COMPONENTTYPE *)
                                 (((OMX_BASE_DIO_CreateParams *)(pCreateParams))->hComponent);
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)
                                          (pComp->pComponentPrivate);

    /* creating memory for DIO object private area */
    hDIO->pDIOPrivate = OMX_BASE_Malloc(sizeof(DIO_StdTunnel_Attrs),
                                        pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(NULL != hDIO->pDIOPrivate, OMX_ErrorInsufficientResources);

    TIMM_OSAL_Memset(hDIO->pDIOPrivate, 0x0, sizeof(DIO_StdTunnel_Attrs));

    hDIO->open        =  OMX_DIO_StdTunnel_Open;
    hDIO->close       =  OMX_DIO_StdTunnel_Close;
    hDIO->queue       =  OMX_DIO_StdTunnel_Queue;
    hDIO->dequeue     =  OMX_DIO_StdTunnel_Dequeue;
    hDIO->send        =  OMX_DIO_StdTunnel_Send;
    hDIO->cancel      =  OMX_DIO_StdTunnel_Cancel;
    hDIO->dup         =  OMX_DIO_StdTunnel_Dup;
    hDIO->control     =  OMX_DIO_StdTunnel_Control;
    hDIO->getcount    =  OMX_DIO_StdTunnel_Getcount;
    hDIO->deinit      =  OMX_DIO_StdTunnel_Deinit;
    hDIO->util        =  OMX_DIO_StdTunnel_Util;

    pContext = hDIO->pDIOPrivate;

    /* Initialize private data */
    pContext->sCreateParams = *(OMX_BASE_DIO_CreateParams *)pCreateParams;

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Deinit :
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Deinit (OMX_HANDLETYPE handle)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
    if( NULL != pContext ) {
        TIMM_OSAL_Free(pContext);
        pContext = NULL;
    }
    /*EXIT:*/
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Open :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Open (OMX_HANDLETYPE handle,
                                             OMX_BASE_DIO_OpenParams *pParams)
{
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE             tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE        *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs            *pContext = NULL;
    OMX_COMPONENTTYPE              *pComp = NULL;
    OMX_BASE_PRIVATETYPE           *pBaseComPvt = NULL;
    OMX_BASE_PORTTYPE              *pPort = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE    sTunPorDef;
    OMX_U32                         nMaxCnt = 0;
    OMX_U32                         nTimeout = 0;
    OMX_U32                         i = 0;
    OMX_U32                         nPortIndex = 0;
    OMX_U32                         nStartPortNumber = 0;
    OMX_U32                         nPadding = 0; //To be used in case of alignment issues
    OMX_U8                         *pTmpBuffer;

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    nPortIndex = pPort->sPortDef.nPortIndex;

    /* Tunneled and Supplier port allocates only the buffer and passes
     * the buffer to the non supplier port via OMX_UseBuffer call */
    OMX_BASE_INIT_STRUCT_PTR(&sTunPorDef, OMX_PARAM_PORTDEFINITIONTYPE);
    sTunPorDef.nPortIndex = pPort->nTunnelPort;

    eError = OMX_GetParameter(pPort->hTunnelComp,
                              OMX_IndexParamPortDefinition, &sTunPorDef);
    OMX_BASE_ASSERT(OMX_ErrorNone == eError, OMX_ErrorUndefined);

    nMaxCnt = max(pPort->sPortDef.nBufferCountActual, sTunPorDef.nBufferCountActual);
    pPort->sPortDef.nBufferCountActual = nMaxCnt;

    /* Create a buffer list */
    pPort->pBufferlist = (OMX_BUFFERHEADERTYPE * *)OMX_BASE_Malloc(
        pPort->sPortDef.nBufferCountActual *
        sizeof(OMX_BUFFERHEADERTYPE *),
        pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(NULL != pPort->pBufferlist,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(pPort->pBufferlist, 0, (sizeof
                                             (OMX_BUFFERHEADERTYPE *) *
                                             (pPort->sPortDef.nBufferCountActual)));
    OMX_BASE_Trace("Entered OMX_DIO_StdTunnel_Open");
    if( PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort)) {
        if( pPort->sPortDef.nBufferAlignment == 0 ) {
            nPadding = 0;
        } else if( pParams->nBufSize % pPort->sPortDef.nBufferAlignment == 0 ) {
            nPadding = 0;
        } else {
            nPadding = pPort->sPortDef.nBufferAlignment -
                       (pParams->nBufSize % pPort->sPortDef.nBufferAlignment);
        }
        if( pBaseComPvt->tDerToBase.pPortProperties[nPortIndex - nStartPortNumber]->
            hBufHeapPerPort == NULL ) {
            OMX_BASE_Trace("Inside hBufHeapPerPort == NULL: Asking for memory size(%d) * count(%d) =  %d\n",
                           (pPort->sPortDef.nBufferSize + nPadding), pPort->sPortDef.nBufferCountActual, (pPort->sPortDef.nBufferSize + nPadding) * pPort->sPortDef.nBufferCountActual);
            pContext->pBufPool = (OMX_PTR)TIMM_OSAL_MallocExtn(
                (pPort->sPortDef.nBufferSize + nPadding) *
                pPort->sPortDef.nBufferCountActual,
                TIMM_OSAL_TRUE,
                pPort->sPortDef.nBufferAlignment,
                TIMMOSAL_MEM_SEGMENT_EXT,
                pBaseComPvt->tDerToBase.hDefaultHeap);
        } else {
            OMX_BASE_Trace("Inside hBufHeapPerPort != NULL");
            pContext->pBufPool = (OMX_PTR)TIMM_OSAL_MallocExtn(
                (pPort->sPortDef.nBufferSize + nPadding) *
                pPort->sPortDef.nBufferCountActual,
                TIMM_OSAL_TRUE,
                pPort->sPortDef.nBufferAlignment,
                TIMMOSAL_MEM_SEGMENT_EXT,
                pBaseComPvt->tDerToBase.pPortProperties
                [nPortIndex - nStartPortNumber]->hBufHeapPerPort);
        }
        OMX_BASE_ASSERT(NULL != pContext->pBufPool,
                        OMX_ErrorInsufficientResources);
        OMX_BASE_Trace("Obtained memory for pBufPool");
        if( pPort->sPortDef.eDir == OMX_DirOutput ) {
            /*Allocating extra headers if this is the o/p port so that TI
            components can still have dup functionality. Original buffer
            headers will be provided by the other component, these headers
            will be used only in case of dup*/
            /*pContext->pHdrPool = (OMX_PTR)OMX_BASE_Malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                         (pPort->sPortDef.nBufferCountActual), pBaseComPvt->tDerToBase.hDefaultHeap);
            OMX_BASE_ASSERT (NULL != pContext->pHdrPool, OMX_ErrorInsufficientResources);
            TIMM_OSAL_Memset(pContext->pHdrPool, 0, (sizeof
                             (OMX_BUFFERHEADERTYPE) *
                             (pPort->sPortDef.nBufferCountActual)));*/


            pContext->pOutPrivatePool = (OMX_PTR)OMX_BASE_Malloc(pPort->sPortDef.nBufferCountActual *
                                                                 sizeof(OMX_BASE_OUTPUTPORTPRIVATE),
                                                                 pBaseComPvt->tDerToBase.hDefaultHeap);
            OMX_BASE_ASSERT(NULL != pContext->pOutPrivatePool, OMX_ErrorInsufficientResources);
            OMX_BASE_Trace("Obtained memory for pOutPrivatePool");
            /*Setting output port pvt pool to 0*/
            TIMM_OSAL_Memset(pContext->pOutPrivatePool, 0,
                             pPort->sPortDef.nBufferCountActual *
                             sizeof(OMX_BASE_OUTPUTPORTPRIVATE));
        }
        /*This has to be updated at each dequeue call so that for
        multiprocessor tunneling, address of the orig header on that
        processor is stored*/
        pContext->pPlatformPrivatePool = (OMX_PTR)OMX_BASE_Malloc(
                pPort->sPortDef.nBufferCountActual *
                sizeof(OMX_TI_PLATFORMPRIVATE),
                pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(NULL != pContext->pPlatformPrivatePool,
                            OMX_ErrorInsufficientResources);

        OMX_BASE_Error("Successfully allocated pool for Platform private data structure");
        /*Setting platform port pvt pool to 0*/
        TIMM_OSAL_Memset(pContext->pPlatformPrivatePool, 0,
                             pPort->sPortDef.nBufferCountActual *
                             sizeof(OMX_TI_PLATFORMPRIVATE));

        for( i = 0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
            pTmpBuffer = (OMX_U8 *)((OMX_U32)(pContext->pBufPool) + (OMX_U32)((pPort->sPortDef.nBufferSize + nPadding) * i));

            do {
                OMX_BASE_Trace("OMX_DIO_StdTunnel_Open:Calling UseBuffer pBuffer = %x", pTmpBuffer);
                eError = OMX_UseBuffer(pPort->hTunnelComp,
                                       &pPort->pBufferlist[i],
                                       pPort->nTunnelPort,
                                       pComp->pApplicationPrivate,
                                       pPort->sPortDef.nBufferSize,
                                       pTmpBuffer);
                if((OMX_ErrorIncorrectStateOperation == eError) && (nTimeout < TIMEOUT_VAL)) {
                    /* sleep for while so that the tunneled component may
                     * recieve a command for loaded to idle state transition */
                    TIMM_OSAL_SleepTask(SLEEP_TIME);
                    nTimeout++;
                } else {
                    break;
                }
            } while( OMX_ErrorNone != eError );

            /*Either timeout has occured or UseBuffer returned some other error*/
            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);

            pPort->pBufferlist[i]->pPlatformPrivate =
                (OMX_TI_PLATFORMPRIVATE *)(pContext->pPlatformPrivatePool) + i;
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->nSize =
                sizeof(OMX_TI_PLATFORMPRIVATE);

            /*Other values defulted to NULL*/
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pMetaDataBuffer = NULL;
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->nMetaDataSize = 0;

            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pAuxBuf1 = NULL;
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pAuxBufSize1 = 0;

            /*This has to be updated at each dequeue call so that for
            multiprocessor tunneling, address of the orig header on that
            processor is stored*/
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pExtendedPlatformPrivate =
                pPort->pBufferlist[i];
            if( pPort->sPortDef.eDir == OMX_DirOutput ) {
                /*Each buffer header has its o/p port private and each o/p port private has its mutex created*/
                (pPort->pBufferlist[i]->pOutputPortPrivate) =
                    (OMX_BASE_OUTPUTPORTPRIVATE *)(pContext->pOutPrivatePool) + i;
                tStatus = TIMM_OSAL_MutexCreate(&(((OMX_BASE_OUTPUTPORTPRIVATE *)pPort->pBufferlist[i]->pOutputPortPrivate)->pBufCountMutex));
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorInsufficientResources);
                /* Pointer to OrigBufHeader being updated - this ensures that in dup cases,
                         only the original buffer header is recycled */
                //((OMX_BASE_OUTPUTPORTPRIVATE *)pPort->pBufferlist[i]->pOutputPortPrivate)->pOrigBufHeader = pPort->pBufferlist[i];
            }
            pPort->nBufferCnt++;
            if( pPort->sPortDef.nBufferCountActual == pPort->nBufferCnt ) {
                pPort->sPortDef.bPopulated = OMX_TRUE;
                TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent, BUF_ALLOC_EVENT,
                                   TIMM_OSAL_EVENT_OR);
            }
        }
    } else if( PORT_IS_TUNNELED(pPort)) {
        pContext->pHdrPool = (OMX_PTR)OMX_BASE_Malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                                      (pPort->sPortDef.nBufferCountActual), pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(NULL != pContext->pHdrPool, OMX_ErrorInsufficientResources);
        pContext->pPlatformPrivatePool = (OMX_PTR)OMX_BASE_Malloc(
            pPort->sPortDef.nBufferCountActual *
            sizeof(OMX_TI_PLATFORMPRIVATE),
            pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(NULL != pContext->pPlatformPrivatePool,
                        OMX_ErrorInsufficientResources);

        OMX_BASE_Error("Successfully allocated pool for Platform private data structure");
        /*Setting platform port pvt pool to 0*/
        TIMM_OSAL_Memset(pContext->pPlatformPrivatePool, 0,
                         pPort->sPortDef.nBufferCountActual *
                         sizeof(OMX_TI_PLATFORMPRIVATE));
        /*Allocating pool for output port private if this is the output port*/
        if( pPort->sPortDef.eDir == OMX_DirOutput ) {
            pContext->pOutPrivatePool = (OMX_PTR)OMX_BASE_Malloc(pPort->sPortDef.nBufferCountActual *
                                                                 sizeof(OMX_BASE_OUTPUTPORTPRIVATE),
                                                                 pBaseComPvt->tDerToBase.hDefaultHeap);
            OMX_BASE_ASSERT(NULL != pContext->pOutPrivatePool, OMX_ErrorInsufficientResources);
            /*Setting output port pvt pool to 0*/
            TIMM_OSAL_Memset(pContext->pOutPrivatePool, 0,
                             pPort->sPortDef.nBufferCountActual *
                             sizeof(OMX_BASE_OUTPUTPORTPRIVATE));
        }

        for( i=0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
            pPort->pBufferlist[i] = (OMX_BUFFERHEADERTYPE *)(pContext->pHdrPool) + i;
            OMX_BASE_INIT_STRUCT_PTR(pPort->pBufferlist[i], OMX_BUFFERHEADERTYPE);
            pPort->pBufferlist[i]->pPlatformPrivate =
                (OMX_TI_PLATFORMPRIVATE *)(pContext->pPlatformPrivatePool) + i;
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->nSize =
                sizeof(OMX_TI_PLATFORMPRIVATE);

            /*Other values defulted to NULL*/
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pMetaDataBuffer = NULL;
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->nMetaDataSize = 0;

            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pAuxBuf1 = NULL;
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pAuxBufSize1 = 0;
            /*This has to be updated at each dequeue call so that for
            multiprocessor tunneling, address of the orig header on that
            processor is stored*/
            ((OMX_TI_PLATFORMPRIVATE *)pPort->pBufferlist[i]->pPlatformPrivate)->pExtendedPlatformPrivate =
                pPort->pBufferlist[i];
            if( pPort->sPortDef.eDir == OMX_DirOutput ) {
                /*Each buffer header has its o/p port private and each o/p port private has its mutex created*/
                (pPort->pBufferlist[i]->pOutputPortPrivate) =
                    (OMX_BASE_OUTPUTPORTPRIVATE *)(pContext->pOutPrivatePool) + i;
                TIMM_OSAL_MutexCreate(&(((OMX_BASE_OUTPUTPORTPRIVATE *)pPort->pBufferlist[i]->pOutputPortPrivate)->pBufCountMutex));
                OMX_BASE_ASSERT(NULL != pContext->pHdrPool, OMX_ErrorInsufficientResources);
                /* Pointer to OrigBufHeader being updated - this ensures that in dup cases,
                         only the original buffer header is recycled */
                //((OMX_BASE_OUTPUTPORTPRIVATE *)pPort->pBufferlist[i]->pOutputPortPrivate)->pOrigBufHeader = pPort->pBufferlist[i];
            }
        }

        /*Memsetting non-used buffer headers to 0. These will be used later by Dup
           Non used headers should be memset to 0 bec. dup will pick up extra buffer header only
           if this is the case*/
        /*TIMM_OSAL_Memset((OMX_BUFFERHEADERTYPE*)(pContext->pHdrPool) + (pPort->sPortDef.nBufferCountActual), 0,
                                         (sizeof(OMX_BUFFERHEADERTYPE) * (pPort->sPortDef.nBufferCountActual)));*/

        /*Non-supplier releases this sem to indicate that headers have been allocated*/
        TIMM_OSAL_SemaphoreRelease(pPort->pDioOpenCloseSem);
    }
    /* create a fixed size OS pipe */
    tStatus = TIMM_OSAL_CreatePipe(&pContext->pPipeHandle,
                                   (sizeof(OMX_BUFFERHEADERTYPE *) * pPort->sPortDef.nBufferCountActual),
                                   sizeof(OMX_BUFFERHEADERTYPE *), 1);
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorInsufficientResources);
    /*Only supplier port needs a wait queue*/
    if( PORT_IS_SUPPLIER(pPort)) {
        tStatus = TIMM_OSAL_CreatePipe(&pContext->pWaitPipeHandle,
                                       (sizeof(OMX_BUFFERHEADERTYPE *) * pPort->sPortDef.nBufferCountActual),
                                       sizeof(OMX_BUFFERHEADERTYPE *), 1);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorInsufficientResources);
    }
EXIT:
    if( OMX_ErrorNone != eError ) {
        OMX_DIO_StdTunnel_Close(handle);
    }
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Close :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Close (OMX_HANDLETYPE handle)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_U32                    nTimeout = 0;
    OMX_U32                    i = 0;

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    if( PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort)) {
        for( i=0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
            do {
                eError = OMX_FreeBuffer(pPort->hTunnelComp,
                                        pPort->nTunnelPort,
                                        pPort->pBufferlist[i]);
                if((OMX_ErrorIncorrectStateOperation == eError) && (nTimeout < TIMEOUT_VAL)) {
                    /* sleep for while so that the tunneled component may
                     * recieve a command for idle to loaded state transition */
                    TIMM_OSAL_SleepTask(SLEEP_TIME);
                    nTimeout++;
                } else {
                    break;
                }
            } while( OMX_ErrorNone != eError );

            /*Either timeout has occured or FreeBuffer returned some other error*/
            OMX_BASE_ASSERT(OMX_ErrorNone == eError, OMX_ErrorUndefined);

            pPort->nBufferCnt--;
            if( 0 == pPort->nBufferCnt ) {
                TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent, BUF_FREE_EVENT,
                                   TIMM_OSAL_EVENT_OR);
            }
        }

        TIMM_OSAL_Free(pContext->pBufPool);
        pContext->pBufPool = NULL;
        /*Delete dup headers if port is output port*/
        if( pPort->sPortDef.eDir == OMX_DirOutput ) {

            for( i = 0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
                if( pPort->pBufferlist ) {
                    if( pPort->pBufferlist[i] ) {
                        if( pPort->pBufferlist[i]->pOutputPortPrivate ) {
                            if(((OMX_BASE_OUTPUTPORTPRIVATE *)pPort->pBufferlist[i]->pOutputPortPrivate)->pBufCountMutex ) {

                                TIMM_OSAL_MutexDelete(((OMX_BASE_OUTPUTPORTPRIVATE *)pPort->
                                                       pBufferlist[i]->pOutputPortPrivate)->
                                                      pBufCountMutex);
                            }
                        }
                    }
                }
            }

            TIMM_OSAL_Free(pContext->pOutPrivatePool);
            //TIMM_OSAL_Free(pContext->pHdrPool);
            //pContext->pHdrPool = NULL;
        }
        if( pContext->pPlatformPrivatePool ) {
            TIMM_OSAL_Free(pContext->pPlatformPrivatePool);
            pContext->pPlatformPrivatePool = NULL;
        }
    } else if( PORT_IS_TUNNELED(pPort)) {
        /*Deleting mutexes and output port pvt pool*/
        if( pPort->sPortDef.eDir == OMX_DirOutput ) {

            for( i = 0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
                if( pPort->pBufferlist ) {
                    if( pPort->pBufferlist[i] ) {
                        if( pPort->pBufferlist[i]->pOutputPortPrivate ) {
                            if(((OMX_BASE_OUTPUTPORTPRIVATE *)pPort->pBufferlist[i]->pOutputPortPrivate)->pBufCountMutex ) {
                                TIMM_OSAL_MutexDelete(((OMX_BASE_OUTPUTPORTPRIVATE *)pPort->
                                                       pBufferlist[i]->pOutputPortPrivate)->
                                                      pBufCountMutex);
                            }
                        }
                    }
                }
            }

            TIMM_OSAL_Free(pContext->pOutPrivatePool);
        }
        if( pContext->pPlatformPrivatePool ) {
            TIMM_OSAL_Free(pContext->pPlatformPrivatePool);
            pContext->pPlatformPrivatePool = NULL;
        }
        TIMM_OSAL_Free(pContext->pHdrPool);
        pContext->pHdrPool = NULL;
    }
    TIMM_OSAL_Free(pPort->pBufferlist);
    pPort->pBufferlist = NULL;
    if( NULL != pContext->pPipeHandle ) {
        TIMM_OSAL_DeletePipe(pContext->pPipeHandle);
        pContext->pPipeHandle = NULL;
    }
    if( PORT_IS_SUPPLIER(pPort)) {
        if( NULL != pContext->pWaitPipeHandle ) {
            TIMM_OSAL_DeletePipe(pContext->pWaitPipeHandle);
            pContext->pWaitPipeHandle = NULL;
        }
    }
EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Queue :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Queue (OMX_HANDLETYPE handle,
                                              OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE                 eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE           tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE      *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs          *pContext = NULL;
    OMX_BASE_PORTTYPE            *pPort = NULL;
    OMX_BUFFERHEADERTYPE         *pOMXBufHeader;
    OMX_BASE_OUTPUTPORTPRIVATE   *pOutPvt;

    pOMXBufHeader = (OMX_BUFFERHEADERTYPE *) pBuffHeader;
    pOutPvt = (OMX_BASE_OUTPUTPORTPRIVATE *)(pOMXBufHeader->pOutputPortPrivate);

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = pContext->sCreateParams.pPort;
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
                                                &pBuffHeader, sizeof(pBuffHeader), TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
            } else {
                --(pOutPvt->nDupBufCount);
            }
            /*
                        if(((OMX_BASE_OUTPUTPORTPRIVATE *)pOMXBufHeader->pOutputPortPrivate)->pOrigBufHeader != pOMXBufHeader)
                        {
                            TIMM_OSAL_Memset(pBuffHeader, 0, sizeof(OMX_BUFFERHEADERTYPE));
                        }
            */
            TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
        }
    }
EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Dequeue :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Dequeue (OMX_HANDLETYPE handle,
                                                OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE        tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort;
    OMX_U32                    actualSize = 0;
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

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
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
    ((OMX_TI_PLATFORMPRIVATE *)pOrigOMXBufHeader->pPlatformPrivate)->pExtendedPlatformPrivate = pOrigOMXBufHeader;
    /*Cancel the buffer and return warning so that derived component may call
    GetAttribute*/
    if( pOrigOMXBufHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG ) {
        tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pPipeHandle, &pOrigOMXBufHeader,
                                               sizeof(pOrigOMXBufHeader), TIMM_OSAL_NO_SUSPEND);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
        eError = (OMX_ERRORTYPE)OMX_TI_WarningAttributePending;
        goto EXIT;
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
                pTmp = pOrigOMXBufHeader->pBuffer +
                       ((OMX_BUFFERHEADERTYPE *)pBuffHeader)->nOffset +
                       ((OMX_BUFFERHEADERTYPE *)pBuffHeader)->nFilledLen + 3;

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
                if( pOrigOMXBufHeader->nFlags & OMX_BUFFERFLAG_EXTRADATA ) {
                    /*Clear the flag*/
                    pOrigOMXBufHeader->nFlags &= (~OMX_BUFFERFLAG_EXTRADATA);
                    pTmp = pOrigOMXBufHeader->pBuffer +
                           pOrigOMXBufHeader->nOffset +
                           pOrigOMXBufHeader->nFilledLen + 3;
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
    TIMM_OSAL_Memcpy(pBuffHeader, pOrigOMXBufHeader, sizeof(OMX_BUFFERHEADERTYPE));

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Send :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Send (OMX_HANDLETYPE handle,
                                             OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE        tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_S32                    elementsInPipe = 0;
    OMX_U32                    actualSize = 0;
    OMX_U32                    nTimeout = 0;
    OMX_COMPONENTTYPE         *pComp;
    OMX_BASE_PRIVATETYPE      *pBaseComPvt;
    OMX_PTR                    pRunningPtr = NULL;
    OMX_S32                    nPosInTable;
    OMX_U32                    nPortIndex, nBitField = 0, nMask = 1, i = 0, nNumFields = 0;
    OMX_OTHER_EXTRADATATYPE   *pExtra = NULL;
    OMX_U8                    *pTmp = NULL;
    OMX_BUFFERHEADERTYPE      *pOMXBufHeader;
    OMX_BUFFERHEADERTYPE      *pOrigOMXBufHeader = NULL;

    pOMXBufHeader = (OMX_BUFFERHEADERTYPE *) pBuffHeader;

    pOrigOMXBufHeader = ((OMX_TI_PLATFORMPRIVATE *)
                         pOMXBufHeader->pPlatformPrivate)->pExtendedPlatformPrivate;
    TIMM_OSAL_Memcpy(pOrigOMXBufHeader, pBuffHeader, sizeof(OMX_BUFFERHEADERTYPE));

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    nPortIndex = pPort->sPortDef.nPortIndex;

    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    nNumFields = pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                 nNumMetaDataFields;
    if( OMX_DirOutput == pPort->sPortDef.eDir ) {
        if( nNumFields != 0 ) {
            pTmp = pOrigOMXBufHeader->pBuffer + pOrigOMXBufHeader->nOffset +
                   pOrigOMXBufHeader->nFilledLen + 3;
            pExtra = (OMX_OTHER_EXTRADATATYPE *) (((OMX_U32) pTmp) & ~3);

            pRunningPtr = (OMX_PTR)((OMX_U32)pBuffHeader +
                                    sizeof(OMX_BUFFERHEADERTYPE));
            nBitField = *(OMX_U32 *)pRunningPtr;
            /*Moving to 1st extra data in header*/
            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr + sizeof(OMX_U32));
            if( nBitField != 0 ) {
                pOrigOMXBufHeader->nFlags |= OMX_BUFFERFLAG_EXTRADATA;

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
    if( PORT_IS_SUPPLIER(pPort)) {
        /*If there are any buffers in the wait pipe, they need to be sent first*/
        TIMM_OSAL_GetPipeReadyMessageCount(pContext->pWaitPipeHandle,
                                           (OMX_U32 *)(&elementsInPipe));
        if( elementsInPipe == 0 ) {
            if( OMX_DirInput == pPort->sPortDef.eDir ) {
                pOrigOMXBufHeader->nOutputPortIndex = pPort->nTunnelPort;
                pOrigOMXBufHeader->nInputPortIndex  = pPort->sPortDef.nPortIndex;
                eError = OMX_FillThisBuffer(pPort->hTunnelComp,
                                            pOrigOMXBufHeader);
            } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
                pOrigOMXBufHeader->nInputPortIndex  =  pPort->nTunnelPort;
                pOrigOMXBufHeader->nOutputPortIndex = pPort->sPortDef.nPortIndex;
                eError = OMX_EmptyThisBuffer(pPort->hTunnelComp,
                                             pOrigOMXBufHeader);
            }
            /*Tunneled component is not in a state to accept buffer -
              put in wait queue*/
            if( eError == OMX_ErrorIncorrectStateOperation ) {
                tStatus = TIMM_OSAL_WriteToPipe(pContext->pWaitPipeHandle,
                                                &pOrigOMXBufHeader, sizeof(pOrigOMXBufHeader), TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
            } else if( eError != OMX_ErrorNone ) {
                goto EXIT;
            }
        } else if( elementsInPipe == (pPort->sPortDef.nBufferCountActual - 1)) {
            /*Wait queue is full. If the tunneled component is still
              not accepting buffers then sleep for some time*/
            tStatus = TIMM_OSAL_WriteToPipe(pContext->pWaitPipeHandle, &pOrigOMXBufHeader,
                                            sizeof(pOrigOMXBufHeader), TIMM_OSAL_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);

            for( i = 0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
                TIMM_OSAL_ReadFromPipe(pContext->pWaitPipeHandle, &pOMXBufHeader,
                                       sizeof(pOMXBufHeader), &actualSize, TIMM_OSAL_NO_SUSPEND);
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);

                do {
                    if( OMX_DirInput == pPort->sPortDef.eDir ) {
                        pOMXBufHeader->nOutputPortIndex = pPort->nTunnelPort;
                        pOMXBufHeader->nInputPortIndex  = pPort->sPortDef.nPortIndex;
                        eError = OMX_FillThisBuffer(pPort->hTunnelComp, pOMXBufHeader);
                    } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
                        pOMXBufHeader->nInputPortIndex   =  pPort->nTunnelPort;
                        pOMXBufHeader->nOutputPortIndex  =  pPort->sPortDef.nPortIndex;
                        eError = OMX_EmptyThisBuffer(pPort->hTunnelComp, pOMXBufHeader);
                    }
                    if((OMX_ErrorIncorrectStateOperation == eError) && (nTimeout < TIMEOUT_VAL)) {
                        /* sleep for while so that the tunneled component may
                         * recieve a command for loaded to idle state transition */
                        TIMM_OSAL_SleepTask(SLEEP_TIME);
                        if( pBaseComPvt->tNewState == OMX_StateIdle ||
                            pPort->bIsTransitionToDisabled == OMX_TRUE ) {
                            /*Component has received command to move to idle so stop
                            trying to send buffers and exit this function*/
                            eError = OMX_ErrorNone;
                            tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pWaitPipeHandle, &pOMXBufHeader,
                                                                   sizeof(pOMXBufHeader), TIMM_OSAL_NO_SUSPEND);
                            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
                            goto EXIT;
                        }
                        nTimeout++;
                    } else {
                        break;
                    }
                } while((OMX_ErrorNone != eError));

                /*Either timeout has occured or UseBuffer returned some other error*/
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
            }
        } else {
            /*There are some buffers in the wait queue, 1st try to send those*/
            tStatus = TIMM_OSAL_WriteToPipe(pContext->pWaitPipeHandle, &pOrigOMXBufHeader,
                                            sizeof(pOrigOMXBufHeader), TIMM_OSAL_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);

            while( elementsInPipe >= 0 ) { /*One more element has been added to the pipe*/
                TIMM_OSAL_ReadFromPipe(pContext->pWaitPipeHandle, &pOMXBufHeader,
                                       sizeof(pOMXBufHeader), &actualSize, TIMM_OSAL_NO_SUSPEND);
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
                if( OMX_DirInput == pPort->sPortDef.eDir ) {
                    pOMXBufHeader->nOutputPortIndex = pPort->nTunnelPort;
                    pOMXBufHeader->nInputPortIndex  = pPort->sPortDef.nPortIndex;
                    eError = OMX_FillThisBuffer(pPort->hTunnelComp, pOMXBufHeader);
                } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
                    pOMXBufHeader->nInputPortIndex   =  pPort->nTunnelPort;
                    pOMXBufHeader->nOutputPortIndex  =  pPort->sPortDef.nPortIndex;
                    eError = OMX_EmptyThisBuffer(pPort->hTunnelComp, pOMXBufHeader);
                }
                elementsInPipe--;
                if( eError == OMX_ErrorIncorrectStateOperation ) {
                    tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pWaitPipeHandle, &pOMXBufHeader,
                                                           sizeof(pOMXBufHeader), TIMM_OSAL_SUSPEND);
                    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
                    break;
                } else if( eError != OMX_ErrorNone ) {
                    tStatus = TIMM_OSAL_WriteToFrontOfPipe(pContext->pWaitPipeHandle, &pOMXBufHeader,
                                                           sizeof(pOMXBufHeader), TIMM_OSAL_SUSPEND);
                    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
                    goto EXIT;
                }
            }
        }
    } else {
        if( OMX_DirInput == pPort->sPortDef.eDir ) {
            pOrigOMXBufHeader->nOutputPortIndex = pPort->nTunnelPort;
            pOrigOMXBufHeader->nInputPortIndex  = pPort->sPortDef.nPortIndex;
            eError = OMX_FillThisBuffer(pPort->hTunnelComp, pOrigOMXBufHeader);
        } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
            pOrigOMXBufHeader->nInputPortIndex   =  pPort->nTunnelPort;
            pOrigOMXBufHeader->nOutputPortIndex  =  pPort->sPortDef.nPortIndex;
            eError = OMX_EmptyThisBuffer(pPort->hTunnelComp, pOrigOMXBufHeader);
        }
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    }
EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Cancel :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Cancel (OMX_HANDLETYPE handle,
                                               OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE                 eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE           tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE      *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs          *pContext = NULL;
    OMX_BASE_PORTTYPE            *pPort = NULL;
    OMX_BUFFERHEADERTYPE         *pOMXBufHeader;
    OMX_BUFFERHEADERTYPE         *pOrigOMXBufHeader = NULL;
    OMX_BASE_OUTPUTPORTPRIVATE   *pOutPvt;
    OMX_COMPONENTTYPE            *pComp;
    OMX_BASE_PRIVATETYPE         *pBaseComPvt;
    OMX_BASE_INTERNALTYPE        *pBaseComInt;
    OMX_BOOL                      bCallProcess = OMX_FALSE;

    pOMXBufHeader = (OMX_BUFFERHEADERTYPE *) pBuffHeader;
    pOutPvt = (OMX_BASE_OUTPUTPORTPRIVATE *)(pOMXBufHeader->pOutputPortPrivate);

    pOrigOMXBufHeader = ((OMX_TI_PLATFORMPRIVATE *)
                         pOMXBufHeader->pPlatformPrivate)->pExtendedPlatformPrivate;
    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
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
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
                bCallProcess = OMX_TRUE;
            } else {
                --(pOutPvt->nDupBufCount);
            }
            /*
            if(((OMX_BASE_OUTPUTPORTPRIVATE *)pOMXBufHeader->pOutputPortPrivate)->pOrigBufHeader != pOMXBufHeader)
            {
                TIMM_OSAL_Memset(pBuffHeader, 0, sizeof(OMX_BUFFERHEADERTYPE));
            }
            */
            TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
        }
    }
    if( bCallProcess ) {
        /*Calling process fn so that event to process the buffer can be generated*/
        pBaseComInt->fpInvokeProcessFunction(pComp, DATAEVENT);
    }
EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Dup :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Dup (OMX_HANDLETYPE handle,
                                            OMX_PTR pBuffHeader,
                                            OMX_PTR pDupBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    //OMX_S32 i = 0;
    OMX_BUFFERHEADERTYPE   *pOMXBuffheader; //**pOMXDupBuffheader;
    //OMX_BUFFERHEADERTYPE *pTmpBufHdr;
    OMX_BASE_OUTPUTPORTPRIVATE   *pOutPvt;

    pOMXBuffheader = (OMX_BUFFERHEADERTYPE *)(pBuffHeader);
    //pOMXDupBuffheader = (OMX_BUFFERHEADERTYPE**)(pDupBuffHeader);
    pOutPvt = (OMX_BASE_OUTPUTPORTPRIVATE *)(pOMXBuffheader->pOutputPortPrivate);

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    if( pPort->sPortDef.eDir == OMX_DirInput ) {
        OMX_BASE_Error("\nDup not valid Input Port\n");
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    TIMM_OSAL_MutexObtain(pOutPvt->pBufCountMutex, TIMM_OSAL_SUSPEND);
    if( pOutPvt->nDupBufCount == 0 ) {
        pOutPvt->nDupBufCount = 2;
    } else {
        pOutPvt->nDupBufCount++;
    }
    TIMM_OSAL_Memcpy(pDupBuffHeader, pOMXBuffheader, sizeof(OMX_BUFFERHEADERTYPE));
    TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);

    //Old dup implementation - not required now
#if 0
    /*Supplier port allocates only dup headers while non-supplier ports allocate
      both normal and dup headers*/
    if( PORT_IS_SUPPLIER(pPort)) {
        i = 0;
    } else {
        i = pPort->sPortDef.nBufferCountActual;
    }
    pTmpBufHdr = (OMX_BUFFERHEADERTYPE *)(pContext->pHdrPool) + i;
    pOMXDupBuffheader = &pTmpBufHdr;

    TIMM_OSAL_MutexObtain(pOutPvt->pBufCountMutex, TIMM_OSAL_SUSPEND);

    /*Searching for an empty header from the header pool*/
    for( i = 0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
        /*nSize is the 1st thing that a used header fills so if it is 0 then the header is
           assumed to be unused*/
        if((*pOMXDupBuffheader)->nSize == 0 ) {
            break;
        }
        (*pOMXDupBuffheader)++;
    }

    if( i >= pPort->sPortDef.nBufferCountActual ) {
        TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
        /*No empty header was found*/
        OMX_BASE_ASSERT(OMX_FALSE, OMX_ErrorInsufficientResources);
    }
    TIMM_OSAL_Memcpy((*pOMXDupBuffheader), pOMXBuffheader, sizeof(OMX_BUFFERHEADERTYPE));
    if( pOutPvt->nDupBufCount == 0 ) {
        pOutPvt->nDupBufCount = 2;
    } else {
        pOutPvt->nDupBufCount++;
    }
    TIMM_OSAL_MutexRelease(pOutPvt->pBufCountMutex);
#endif

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Control :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Control (OMX_HANDLETYPE handle,
                                                OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                                                OMX_PTR pParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE        tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_BUFFERHEADERTYPE      *pBuffHeader = NULL;
    OMX_U32                    actualSize = 0;
    OMX_U32                    elementsInPipe = 0, elementsInWaitPipe = 0;
    OMX_U32                    i, nTimeout = 0;
    OMX_COMPONENTTYPE         *pComp;
    OMX_BASE_PRIVATETYPE      *pBaseComPvt;
    OMX_BASE_INTERNALTYPE     *pBaseComInt;

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;

    pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    switch( nCmdType ) {
        /*Return all buffers to output port*/
        case OMX_BASE_DIO_CTRLCMD_Flush :
            if( OMX_DirInput == pPort->sPortDef.eDir ) {
                TIMM_OSAL_GetPipeReadyMessageCount(pContext->pPipeHandle, &elementsInPipe);

                while( elementsInPipe ) {
                    TIMM_OSAL_ReadFromPipe(pContext->pPipeHandle, &pBuffHeader,
                                           sizeof(pBuffHeader), &actualSize,
                                           TIMM_OSAL_NO_SUSPEND);
                    elementsInPipe--;
                    pBuffHeader->nOutputPortIndex = pPort->nTunnelPort;
                    eError = OMX_FillThisBuffer(pPort->hTunnelComp,
                                                pBuffHeader);
                    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
                }
            }
            break;
        /*Called in case of Executing/Pause --> Idle or Port Disable
          Return all buffers to supplier if port is non-supplier*/
        case OMX_BASE_DIO_CTRLCMD_Stop :
            if( !PORT_IS_SUPPLIER(pPort)) {
                TIMM_OSAL_GetPipeReadyMessageCount(pContext->pPipeHandle, &elementsInPipe);

                while( elementsInPipe ) {
                    TIMM_OSAL_ReadFromPipe(pContext->pPipeHandle, &pBuffHeader,
                                           sizeof(pBuffHeader), &actualSize,
                                           TIMM_OSAL_NO_SUSPEND);
                    elementsInPipe--;
                    if( OMX_DirInput == pPort->sPortDef.eDir ) {
                        pBuffHeader->nOutputPortIndex = pPort->nTunnelPort;
                        eError = OMX_FillThisBuffer(pPort->hTunnelComp,
                                                    pBuffHeader);
                    } else if( OMX_DirOutput == pPort->sPortDef.eDir ) {
                        pBuffHeader->nInputPortIndex = pPort->nTunnelPort;
                        eError = OMX_EmptyThisBuffer(pPort->hTunnelComp,
                                                     pBuffHeader);
                    }
                    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
                }
            } else if( PORT_IS_SUPPLIER(pPort)) {
                TIMM_OSAL_GetPipeReadyMessageCount(pContext->pWaitPipeHandle,
                                                   &elementsInWaitPipe);

                while( elementsInWaitPipe ) {
                    elementsInWaitPipe--;
                    /*Put elements from wait pipe to normal pipe. Order does not
                      matter as buffers are no longer valid now*/
                    TIMM_OSAL_ReadFromPipe(pContext->pWaitPipeHandle, &pBuffHeader,
                                           sizeof(pBuffHeader), &actualSize,
                                           TIMM_OSAL_NO_SUSPEND);
                    tStatus = TIMM_OSAL_WriteToPipe(pContext->pPipeHandle, &pBuffHeader,
                                                    sizeof(pBuffHeader), TIMM_OSAL_SUSPEND);
                    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
                }

                TIMM_OSAL_GetPipeReadyMessageCount(pContext->pPipeHandle,
                                                   &elementsInPipe);

                while((elementsInPipe != pPort->sPortDef.nBufferCountActual)
                      && (nTimeout < TIMEOUT_VAL)) {
                    /*Wait for tunneled component to return all buffers*/
                    TIMM_OSAL_SleepTask(SLEEP_TIME);
                    nTimeout++;
                    TIMM_OSAL_GetPipeReadyMessageCount(pContext->pPipeHandle,
                                                       &elementsInPipe);
                }

                if( elementsInPipe != pPort->sPortDef.nBufferCountActual ) {
                    eError = OMX_ErrorPortUnresponsiveDuringStop;
                    goto EXIT;
                }
            }
            break;
        /*Called during Idle to Executing, Idle to Pause, Port Enable*/
        case OMX_BASE_DIO_CTRLCMD_Start :
            if((pPort->sPortDef.eDir == OMX_DirInput) && PORT_IS_SUPPLIER(pPort)) {
                for( i = 0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
                    OMX_DIO_StdTunnel_Send(hDIO, pPort->pBufferlist[i]);
                }
            } else if((pPort->sPortDef.eDir == OMX_DirOutput) && PORT_IS_SUPPLIER(pPort)) {
                for( i = 0; i < pPort->sPortDef.nBufferCountActual; i++ ) {
                    OMX_DIO_StdTunnel_Queue(hDIO, pPort->pBufferlist[i]);
                }

                pBaseComInt->fpInvokeProcessFunction(pComp, DATAEVENT);
            }
            break;

        default :
            TIMM_OSAL_Trace(" Invalid Command received \n");
            break;

    }

EXIT:
    return (eError);

}

OMX_ERRORTYPE OMX_DIO_StdTunnel_Util (OMX_HANDLETYPE handle,
                                      OMX_BASE_DIO_UTIL_TYPE nUtilType,
                                      OMX_PTR pParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_COMPONENTTYPE         *pComp = NULL;
    OMX_U32                    nPortIndex;

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;
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

    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_StdTunnel_Getcount :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_StdTunnel_Getcount (OMX_HANDLETYPE handle,
                                                 OMX_U32 *pCount)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_StdTunnel_Attrs       *pContext = NULL;

    pContext = (DIO_StdTunnel_Attrs *)hDIO->pDIOPrivate;

    TIMM_OSAL_GetPipeReadyMessageCount(pContext->pPipeHandle, pCount);

    /*EXIT:*/
    return (eError);

}

