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

#include <string.h>
#include <OMX_TI_Common.h>
#include <OMX_TI_Index.h>
#include "omx_base_internal.h"
#include <timm_osal_interfaces.h>
#include "omx_base_utils.h"
#include "omx_base_dio_plugin.h"
#include "iframeq.h"
#include "frameq.h"
#include "shmem.h"
#include "avc.h"

/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*---------------- function prototypes  -------------------------- */

/*******************************************************************
 * PUBLIC DECLARATIONS: defined here, used elsewhere
 *******************************************************************/
/*---------------- data declarations    -------------------------- */
/*---------------- function prototypes  -------------------------- */

/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------------   data declarations  -------------------------- */

static OMX_U32    gOutPvtPoolCounter = 0;

inline OMX_U32 max(OMX_U32 a, OMX_U32 b)
{
    return ((a > b) ? a : b);
}

/** DIO_FrameHdr:
 *  strucuture used for DIO type frame header  */
typedef struct DIO_FrameHdr {

    IFrameQ_Frame        hdr;
    OMX_BUFFERHEADERTYPE sBuffHdr;

}DIO_FrameHdr;

/** DIO_FrameqTunnel_Attrs:
 *  This structure contains the attributes required
 *  for frameq based tunneling */
typedef struct DIO_FrameqTunnel_Attrs {

    OMX_BASE_DIO_CreateParams sCreateParams;
    IFrameQ_Handle            hFrameq;
    OMX_U32                   nFlags;
    OMX_U32                   nModetype;
    OMX_PTR                   pOutPrivatePool;

}DIO_FrameqTunnel_Attrs;

/*---------------- function prototypes  ------------------------ */

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Open (OMX_HANDLETYPE hDIO,
                                                OMX_BASE_DIO_OpenParams *pParams);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Close (OMX_HANDLETYPE hDIO);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Queue (OMX_HANDLETYPE hDIO,
                                                 OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Dequeue (OMX_HANDLETYPE hDIO,
                                                   OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Send (OMX_HANDLETYPE hDIO,
                                                OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Cancel (OMX_HANDLETYPE hDIO,
                                                  OMX_PTR pBuffHeader);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Dup (OMX_HANDLETYPE hDIO,
                                               OMX_PTR pBuffHeader,
                                               OMX_PTR pDupBuffHeader);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Control (OMX_HANDLETYPE hDIO,
                                                   OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                                                   OMX_PTR pParams);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Getcount (OMX_HANDLETYPE hDIO,
                                                    OMX_U32 *pCount);

static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Deinit (OMX_HANDLETYPE hDIO);

static OMX_ERRORTYPE _OMX_DIO_ChannelCommunicate(DIO_FrameqTunnel_Attrs *pContext);

static void _OMX_DIO_FrameQNotify(void *ctx, void *handle);

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Init :
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Init(OMX_HANDLETYPE handle,
                                        OMX_PTR pCreateParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    AVC_Err                    eAvcError = AVC_SUCCESS;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    FrameQ_CreateParams        fqcp;

    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)(((OMX_BASE_DIO_CreateParams *)(pCreateParams))->hComponent);
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    /* creating memory for DIO object private area */
    hDIO->pDIOPrivate = OMX_BASE_Malloc(sizeof(DIO_FrameqTunnel_Attrs), pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(NULL != hDIO->pDIOPrivate, OMX_ErrorInsufficientResources);

    TIMM_OSAL_Memset(hDIO->pDIOPrivate, 0x0, sizeof(DIO_FrameqTunnel_Attrs));

    hDIO->open        =  OMX_DIO_FrameqTunnel_Open;
    hDIO->close       =  OMX_DIO_FrameqTunnel_Close;
    hDIO->queue       =  OMX_DIO_FrameqTunnel_Queue;
    hDIO->dequeue     =  OMX_DIO_FrameqTunnel_Dequeue;
    hDIO->send        =  OMX_DIO_FrameqTunnel_Send;
    hDIO->cancel      =  OMX_DIO_FrameqTunnel_Cancel;
    hDIO->dup         =  OMX_DIO_FrameqTunnel_Dup;
    hDIO->control     =  OMX_DIO_FrameqTunnel_Control;
    hDIO->getcount    =  OMX_DIO_FrameqTunnel_Getcount;
    hDIO->deinit      =  OMX_DIO_FrameqTunnel_Deinit;

    pContext = hDIO->pDIOPrivate;

    /* Initialize private data */
    pContext->sCreateParams = *(OMX_BASE_DIO_CreateParams *)pCreateParams;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    if( PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort)) {
        fqcp.base.size              =  sizeof(fqcp);
        fqcp.base.frameHeaderSize   =  sizeof(DIO_FrameHdr);
        fqcp.base.frameHeaderAlign  =  0x0; /* default header alignment */
        fqcp.base.frameBufSize      =  pPort->sPortDef.nBufferSize;
        fqcp.base.frameBufAlign     =  pPort->sPortDef.nBufferAlignment;
        strcpy(fqcp.base.frameBufAllocatorName, pContext->sCreateParams.cBufAllocatorName);
        strcpy(fqcp.base.frameHdrAllocatorName, pContext->sCreateParams.cHdrAllocatorName);

        /* create a FrameQ */
        eAvcError = FrameQ_create(pContext->sCreateParams.cChannelName, &fqcp);
        if( eAvcError != AVC_SUCCESS ) {
            TIMM_OSAL_Free(hDIO->pDIOPrivate);
            hDIO->pDIOPrivate = NULL;
            eError = OMX_ErrorUndefined;
            goto EXIT;
        }
        //OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);
        //AD - Moved to Open
        /* Communicate channel information to the Tunneled Component*/
        /*eError = _OMX_DIO_ChannelCommunicate(pContext);
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);*/
    }
EXIT:
    return (eError);

}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Deinit :
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Deinit (OMX_HANDLETYPE handle)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    AVC_Err                    eAvcError = AVC_SUCCESS;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    if( PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort)) {
        /* deleting frameq on the supplier port */
        eAvcError = FrameQ_delete(pContext->sCreateParams.cChannelName);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);

    }
    TIMM_OSAL_Free(pContext);
    pContext = NULL;

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Open :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Open (OMX_HANDLETYPE handle,
                                                OMX_BASE_DIO_OpenParams *pParams)
{
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    AVC_Err                         eAvcError = AVC_SUCCESS;
    OMX_BASE_DIO_OBJECTTYPE        *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs         *pContext = NULL;
    OMX_BASE_PORTTYPE              *pPort = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE    sTunPorDef;
    Shmem_CreateParams              shmemcp;
    IFrameQ_OpenParams              fqop;
    OMX_U32                         nMaxCnt = 0;

    OMX_U32    nPortIndex = 0;
    OMX_U32    nStartPortNumber = 0;

    OMX_COMPONENTTYPE      *pComp = NULL;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = NULL;

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;
    pContext->nModetype = pParams->nMode;

    pComp = (OMX_COMPONENTTYPE *)(pContext->sCreateParams.hComponent);
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    nPortIndex = pPort->sPortDef.nPortIndex;

    fqop.mode      = pParams->nMode;
    if( pParams->bCacheFlag ) {
        fqop.cacheFlag = 1;
    } else {
        fqop.cacheFlag = 0;
    }
    /* Generally supplier(writer) allocates the buffers and
     * supplier port should be a output port */
    if( PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort)) {
        /* Get the buffer requirements from the tunneled port */
        OMX_BASE_INIT_STRUCT_PTR(&sTunPorDef, OMX_PARAM_PORTDEFINITIONTYPE);
        sTunPorDef.nPortIndex = pPort->nTunnelPort;

        eError = OMX_GetParameter(pPort->hTunnelComp,
                                  OMX_IndexParamPortDefinition, &sTunPorDef);
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, OMX_ErrorUndefined);

        nMaxCnt = max(pPort->sPortDef.nBufferCountActual, sTunPorDef.nBufferCountActual);
        pPort->sPortDef.nBufferCountActual = nMaxCnt;

        /* create shared memory for buffers of both Writer and Reader port */
        shmemcp.ishmemCp.size      =  sizeof(shmemcp);
        shmemcp.ishmemCp.numBufs   =  pPort->sPortDef.nBufferCountActual;
        shmemcp.ishmemCp.bufSize   =  pPort->sPortDef.nBufferSize;
        if( pBaseComPvt->tDerToBase.
            pPortProperties[nPortIndex - nStartPortNumber]->hBufHeapPerPort ==
            NULL ) {
            shmemcp.memHeapId =  (int)(pBaseComPvt->tDerToBase.hDefaultHeap);
        } else {
            shmemcp.memHeapId =  (int)(pBaseComPvt->tDerToBase.
                                       pPortProperties[nPortIndex - nStartPortNumber]->hBufHeapPerPort);
        }
        eAvcError= Shmem_create(pContext->sCreateParams.cBufAllocatorName, &shmemcp);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);

        /* create a shared memory for headers of both reader and writer */
        shmemcp.ishmemCp.size      =  sizeof(shmemcp);
        shmemcp.ishmemCp.numBufs   =  ((pPort->sPortDef.nBufferCountActual) * 2);
        shmemcp.ishmemCp.bufSize   =  sizeof(DIO_FrameHdr);
        shmemcp.memHeapId          =  (int)(pBaseComPvt->tDerToBase.hDefaultHeap);

        eAvcError = Shmem_create(pContext->sCreateParams.cHdrAllocatorName, &shmemcp);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);

        /* open FrameQ depending on the mode type */
        FrameQ_open(pContext->sCreateParams.cChannelName, &fqop, &pContext->hFrameq);

        /*Allocating pool for output port private*/
        pContext->pOutPrivatePool = (OMX_PTR)OMX_BASE_Malloc(pPort->sPortDef.nBufferCountActual * 2 *
                                                             sizeof(OMX_BASE_OUTPUTPORTPRIVATE),
                                                             pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(NULL != pContext->pOutPrivatePool, OMX_ErrorInsufficientResources);
        /*Setting output port pvt pool to 0*/
        TIMM_OSAL_Memset(pContext->pOutPrivatePool, 0,
                         pPort->sPortDef.nBufferCountActual * 2
                         * sizeof(OMX_BASE_OUTPUTPORTPRIVATE));
        gOutPvtPoolCounter = 0;


        /* Communicate channel information to the Tunneled Component
        Will also release semaphore to allow non-supplier to open*/
        eError = _OMX_DIO_ChannelCommunicate(pContext);
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);

    } else {
        /*Non supplier waits for supplier to release semaphore which implies that shmem has
        been created*/
        TIMM_OSAL_SemaphoreObtain(pPort->pDioOpenCloseSem, TIMM_OSAL_SUSPEND);
        /* open FrameQ depending on the mode type */
        FrameQ_open(pContext->sCreateParams.cChannelName, &fqop, &pContext->hFrameq);
    }
    pPort->sPortDef.bPopulated = OMX_TRUE;

    TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent, BUF_ALLOC_EVENT,
                       TIMM_OSAL_EVENT_OR);

EXIT:
    return (eError);

}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Close :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Close (OMX_HANDLETYPE handle)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    char                       cChannelNameCopy[OMX_BASE_MAXNAMELEN];

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    pPort = (OMX_BASE_PORTTYPE *)pContext->sCreateParams.pPort;


    /* Closing FrameQ */
    IFrameQ_close(pContext->hFrameq);
    /* delete the shared memory pool created for both buffers
     * and buffer headers incase of a writer port */
    if( PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort)) {

        TIMM_OSAL_Free(pContext->pOutPrivatePool);
        /*If port is supplier, ensure that non supplier closes before supplier can
            delete shmem */

        /*Keep copy of original channel name*/
        strcpy(cChannelNameCopy, pContext->sCreateParams.cChannelName);
        strcpy(pContext->sCreateParams.cChannelName, "PropTunClose");
        eError = _OMX_DIO_ChannelCommunicate(pContext);
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
        /*Orig channel name copied back*/
        strcpy(pContext->sCreateParams.cChannelName, cChannelNameCopy);

        Shmem_delete(pContext->sCreateParams.cBufAllocatorName);
        Shmem_delete(pContext->sCreateParams.cHdrAllocatorName);

        TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent, BUF_FREE_EVENT,
                           TIMM_OSAL_EVENT_OR);
    } else {
        /*Non-supplier releases this sem so that the supplier port can close*/
        TIMM_OSAL_SemaphoreRelease(pPort->pDioOpenCloseSem);
    }
    pPort->sPortDef.bPopulated = OMX_FALSE;


EXIT:
    return (eError);

}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Queue :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Queue (OMX_HANDLETYPE handle,
                                                 OMX_PTR pBuffHeader)
{
    /* Not Implemented for PT FrameQ */

    OMX_ERRORTYPE    eError = OMX_ErrorNone;

    /*
    OMX_BASE_DIO_OBJECTTYPE* hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs* pContext = NULL;

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    */


    /*EXIT:*/
    return (eError);

}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Dequeue :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Dequeue (OMX_HANDLETYPE handle,
                                                   OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    AVC_Err                    eAvcError = AVC_SUCCESS;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;
    DIO_FrameHdr              *pDIOFrameHdr = NULL;

    //OMX_BUFFERHEADERTYPE *pOMXBuffHeader; = (OMX_BUFFERHEADERTYPE *)(pBuffHeader);

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    if( OMX_BASE_DIO_WRITER == pContext->nModetype ) {
        /* writer allocates buffer header from the shared memory pool */
        eAvcError = IFrameQ_alloc(pContext->hFrameq,
                                  (IFrameQ_Frame * *)&pDIOFrameHdr, NULL);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);

        /* update the omx buffer header from DIO Frame Header */
        //(pOMXBuffHeader) = &(pDIOFrameHdr->sBuffHdr);

        //This initialization along should happen at open time
        OMX_BASE_INIT_STRUCT_PTR((&(pDIOFrameHdr->sBuffHdr)), OMX_BUFFERHEADERTYPE);
        //This will happen at each dequeue
        pDIOFrameHdr->sBuffHdr.pBuffer          = pDIOFrameHdr->hdr.frameBufPtr;
        pDIOFrameHdr->sBuffHdr.nAllocLen        = pDIOFrameHdr->hdr.frameBufSize;
        pDIOFrameHdr->sBuffHdr.pPlatformPrivate = pDIOFrameHdr;
        if( pDIOFrameHdr->sBuffHdr.pOutputPortPrivate == NULL ) {
            pDIOFrameHdr->sBuffHdr.pOutputPortPrivate = (OMX_BASE_OUTPUTPORTPRIVATE *)(pContext->pOutPrivatePool) + gOutPvtPoolCounter;
            gOutPvtPoolCounter++;
        }
        TIMM_OSAL_Memcpy(pBuffHeader, &(pDIOFrameHdr->sBuffHdr), sizeof(OMX_BUFFERHEADERTYPE));

    } else if( OMX_BASE_DIO_READER == pContext->nModetype ) {
        /* reader gets buffer and header from FrameQ
         * and update the omx buffer header from DIO Frame header */
        eAvcError = IFrameQ_get(pContext->hFrameq,
                                (IFrameQ_Frame * *)&pDIOFrameHdr);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);

        pDIOFrameHdr->sBuffHdr.pBuffer          = pDIOFrameHdr->hdr.frameBufPtr;
        pDIOFrameHdr->sBuffHdr.pPlatformPrivate = pDIOFrameHdr;
        TIMM_OSAL_Memcpy(pBuffHeader, &(pDIOFrameHdr->sBuffHdr), sizeof(OMX_BUFFERHEADERTYPE));
    }
EXIT:
    return (eError);

}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Send :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Send (OMX_HANDLETYPE handle,
                                                OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    AVC_Err                    eAvcError = AVC_SUCCESS;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;
    DIO_FrameHdr              *pDIOFrameHdr = NULL;

    OMX_BUFFERHEADERTYPE   *pOMXBuffHeader = (OMX_BUFFERHEADERTYPE *)(pBuffHeader);

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    pDIOFrameHdr = pOMXBuffHeader->pPlatformPrivate;

    TIMM_OSAL_Memcpy(&(pDIOFrameHdr->sBuffHdr), pBuffHeader, sizeof(OMX_BUFFERHEADERTYPE));
    if( OMX_BASE_DIO_WRITER == pContext->nModetype ) {
        /* writer puts the DIO frame header into the frameq */

        eAvcError = IFrameQ_put(pContext->hFrameq, (IFrameQ_Frame *)pDIOFrameHdr);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);

    } else if( OMX_BASE_DIO_READER == pContext->nModetype ) {
        /* Reader frees the DIO frame header into the Frameq */
        eAvcError = IFrameQ_free(pContext->hFrameq, (IFrameQ_Frame *)pDIOFrameHdr);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);

    }
EXIT:
    return (eError);

}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Cancel :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Cancel (OMX_HANDLETYPE handle,
                                                  OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    AVC_Err                    eAvcError = AVC_SUCCESS;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;
    DIO_FrameHdr              *pDIOFrameHdr = NULL;

    OMX_BUFFERHEADERTYPE   *pOMXBuffHeader = (OMX_BUFFERHEADERTYPE *)(pBuffHeader);

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    pDIOFrameHdr = pOMXBuffHeader->pPlatformPrivate;

    TIMM_OSAL_Memcpy(&(pDIOFrameHdr->sBuffHdr), pBuffHeader, sizeof(OMX_BUFFERHEADERTYPE));
    if( OMX_BASE_DIO_WRITER == pContext->nModetype ) {
        /* writer port frees the buffer header */
        eAvcError = IFrameQ_free(pContext->hFrameq, (IFrameQ_Frame *)pDIOFrameHdr);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);
    } else if( OMX_BASE_DIO_READER == pContext->nModetype ) {
        /* reader port cancels a frame back into the queue */
        eAvcError = IFrameQ_cancel(pContext->hFrameq, (IFrameQ_Frame *)pDIOFrameHdr);
        OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);
    }
EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Dup :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Dup (OMX_HANDLETYPE handle,
                                               OMX_PTR pBuffHeader,
                                               OMX_PTR pDupBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    AVC_Err                    eAvcError = AVC_SUCCESS;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;
    DIO_FrameHdr              *pDIOFrameHdr = NULL;
    DIO_FrameHdr              *pDupDIOFrameHdr = NULL;

    OMX_BUFFERHEADERTYPE   *pOMXBuffHeader = (OMX_BUFFERHEADERTYPE *)(pBuffHeader);

    //OMX_BUFFERHEADERTYPE **pOMXDupBuffHeader = (OMX_BUFFERHEADERTYPE **)(pDupBuffHeader);

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    pDIOFrameHdr = pOMXBuffHeader->pPlatformPrivate;

    eAvcError = IFrameQ_dup(pContext->hFrameq, (IFrameQ_Frame *)pDIOFrameHdr,
                            (IFrameQ_Frame * *)&pDupDIOFrameHdr);
    OMX_BASE_ASSERT(eAvcError == AVC_SUCCESS, OMX_ErrorUndefined);

    pDupDIOFrameHdr->sBuffHdr.pPlatformPrivate = pDupDIOFrameHdr;

    /* update omx buffer header from DIO Frame Header */
    TIMM_OSAL_Memcpy(pDupBuffHeader, &(pDupDIOFrameHdr->sBuffHdr), sizeof(OMX_BUFFERHEADERTYPE));

EXIT:
    return (eError);

}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Control :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Control (OMX_HANDLETYPE handle,
                                                   OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                                                   OMX_PTR pParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;
    OMX_U32                    nWaterMark = 0;
    OMX_U32                    nFlushCnt = 0;

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;

    switch( nCmdType ) {
        case OMX_BASE_DIO_CTRLCMD_Start :
            IFrameQ_setNotifier(pContext->hFrameq, AVC_NOTIFICATION_TYPE_ALWAYS,
                                nWaterMark, &(_OMX_DIO_FrameQNotify), pContext);

            break;

        case OMX_BASE_DIO_CTRLCMD_Stop :
        case OMX_BASE_DIO_CTRLCMD_Flush :
            if( OMX_BASE_DIO_WRITER == pContext->nModetype ) {
            } else if( OMX_BASE_DIO_READER == pContext->nModetype ) {
                IFrameQ_flush(pContext->hFrameq, (uint32_t *)&nFlushCnt);
            }
            break;

        default :
            break;
    }

    goto EXIT;
EXIT:
    return (eError);

}

/*===============================================================*/
/** @fn OMX_DIO_FrameqTunnel_Getcount :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Getcount (OMX_HANDLETYPE handle,
                                                    OMX_U32 *pCount)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = (OMX_BASE_DIO_OBJECTTYPE *)handle;
    DIO_FrameqTunnel_Attrs    *pContext = NULL;

    pContext = (DIO_FrameqTunnel_Attrs *)hDIO->pDIOPrivate;
    if( OMX_BASE_DIO_WRITER == pContext->nModetype ) {
        /* return number of frames available for allocations */
        IFrameQ_getFreeBufCnt(pContext->hFrameq, (uint32_t *)pCount);

    } else if( OMX_BASE_DIO_READER == pContext->nModetype ) {
        /* get number of frames queued in the FrameQ */
        IFrameQ_getLength(pContext->hFrameq, (uint32_t *)pCount);
    }
    goto EXIT;
EXIT:
    return (eError);

}

/*===============================================================*/
/* PRIVATE FUNCTIONS    */
/*===============================================================*/

/*===============================================================*/
/** @fn _OMX_DIO_ChannelCommunicate:
 *  To Communicate the Channel Information to the Tunneled comp.
 */
/*===============================================================*/
static OMX_ERRORTYPE _OMX_DIO_ChannelCommunicate(DIO_FrameqTunnel_Attrs *pContext)
{
    OMX_ERRORTYPE             eError = OMX_ErrorNone;
    OMX_CONFIG_CHANNELNAME    sConfigChannelName;
    OMX_BASE_PORTTYPE        *pPort = NULL;

    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)(pContext->sCreateParams.hComponent);
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    pPort = pContext->sCreateParams.pPort;

    OMX_BASE_INIT_STRUCT_PTR(&sConfigChannelName, OMX_CONFIG_CHANNELNAME);
    sConfigChannelName.nPortIndex = pPort->nTunnelPort;

    sConfigChannelName.cChannelName = (OMX_STRING)OMX_BASE_Malloc(
        sizeof(OMX_U8 *) *
        OMX_BASE_MAXNAMELEN,
        pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(NULL != sConfigChannelName.cChannelName, OMX_ErrorUndefined);
    strcpy(sConfigChannelName.cChannelName,
           pContext->sCreateParams.cChannelName);

    /* set channel information of the tunneled component  */
    eError = OMX_SetConfig(pPort->hTunnelComp,
                           (OMX_INDEXTYPE)(OMX_TI_IndexConfigChannelName),
                           &sConfigChannelName);
    OMX_BASE_ASSERT(OMX_ErrorNone == eError, OMX_ErrorUndefined);

EXIT:
    if( sConfigChannelName.cChannelName != NULL ) {
        TIMM_OSAL_Free(sConfigChannelName.cChannelName);
    }
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_DIO_FrameQNotify:
 *  This function triggers the data event to process data
 */
/*===============================================================*/
static void _OMX_DIO_FrameQNotify(void *ctx, void *handle)
{

    DIO_FrameqTunnel_Attrs   *pContext = (DIO_FrameqTunnel_Attrs *)ctx;
    OMX_COMPONENTTYPE        *pComp = (OMX_COMPONENTTYPE *)pContext->sCreateParams.hComponent;
    OMX_BASE_PRIVATETYPE     *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE    *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    pBaseComInt->fpInvokeProcessFunction(pComp, DATAEVENT);

}

