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
#include <omx_base_internal.h>
#include <timm_osal_interfaces.h>
#include <omx_base_utils.h>
#include <omx_base_dio_plugin.h>

/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*---------------- function prototypes  -------------------------- */
extern OMX_BASE_EXTRA_DATA_TABLE    OMX_BASE_ExtraDataTable[];
/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------------   data declarations  -------------------------- */
/*---------------- function prototypes   ------------------------- */
static OMX_PTR _OMX_BASE_DIO_GetPort(OMX_HANDLETYPE hComponent,
                                     OMX_U32 nPortIndex);

/*******************************************************************
 * PUBLIC DECLARATIONS: defined here, used elsewhere
 *******************************************************************/
/*---------------- data declarations    -------------------------- */
/*---------------- function prototypes  -------------------------- */
OMX_ERRORTYPE _OMX_BASE_DIO_DefaultGetTotalBufferSize(OMX_HANDLETYPE hComponent,
                                                      OMX_U32 nPortIndex,
                                                      OMX_PTR pDioBufHeader);
OMX_ERRORTYPE _OMX_BASE_DIO_DefaultUpdatePacketExtraDataPointer(OMX_HANDLETYPE hComponent,
                                                                OMX_U32 nPortIndex,
                                                                OMX_PTR pDioBufHeader);
/*===============================================================*/
/** @fn OMX_BASE_DIO_Init :
 *   see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Init (OMX_HANDLETYPE hComponent,
                                 OMX_U32 nPortIndex,
                                 OMX_STRING cChannelType,
                                 OMX_PTR pCreateParams)
{

    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_BOOL                   bFound = OMX_FALSE;
    OMX_U32                    i = 0;

    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    OMX_BASE_REQUIRE(NULL != cChannelType, OMX_ErrorBadParameter);

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    while( NULL != OMX_BASE_DIO_Registered[i].cChannelType ) {
        if( strcmp(cChannelType, OMX_BASE_DIO_Registered[i].cChannelType) == 0 ) {
            bFound = OMX_TRUE;
            break;
        }
        i++;
    }

    if( bFound ) {
        hDIO = (OMX_BASE_DIO_OBJECTTYPE *) OMX_BASE_Malloc(
            sizeof(OMX_BASE_DIO_OBJECTTYPE),
            pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(NULL != hDIO, OMX_ErrorInsufficientResources);

        TIMM_OSAL_Memset(hDIO, 0x0, sizeof(OMX_BASE_DIO_OBJECTTYPE));

        /* Initialize the DIO object depending on the ChannelType */
        eError = OMX_BASE_DIO_Registered[i].pInitialize(hDIO, pCreateParams);
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);

        /* Assign DIO handle to port */
        pPort->hDIO = hDIO;
        OMX_BASE_Info("{base}  DIO_Init on port %d", nPortIndex);

    } else {
        OMX_BASE_ASSERT(OMX_FALSE, OMX_ErrorUndefined);
    }
EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Deinit:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Deinit (OMX_HANDLETYPE hComponent,
                                   OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    OMX_BASE_Info("{base}    DIO_Deinit on port %d", nPortIndex);
    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError = hDIO->deinit(hDIO);

    TIMM_OSAL_Free(pPort->hDIO);
    pPort->hDIO = NULL;

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Open:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Open (OMX_HANDLETYPE hComponent,
                                 OMX_U32 nPortIndex,
                                 OMX_PTR pOpenParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError = hDIO->open(hDIO, (OMX_BASE_DIO_OpenParams *)pOpenParams);

EXIT:
    if( eError == OMX_ErrorNone ) {
        hDIO->bOpened = OMX_TRUE;
    }
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Close:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Close (OMX_HANDLETYPE hComponent,
                                  OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    hDIO->bOpened = OMX_FALSE;
    eError =  hDIO->close(hDIO);

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Queue:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Queue (OMX_HANDLETYPE hComponent,
                                  OMX_U32 nPortIndex,
                                  OMX_PTR pBuffHeader)
{

    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError = hDIO->queue(hDIO, pBuffHeader);

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Dequeue:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Dequeue (OMX_HANDLETYPE hComponent,
                                    OMX_U32 nPortIndex,
                                    OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError =  hDIO->dequeue(hDIO, pBuffHeader);

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Send:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Send (OMX_HANDLETYPE hComponent,
                                 OMX_U32 nPortIndex,
                                 OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError = hDIO->send(hDIO, pBuffHeader);

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Cancel:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Cancel (OMX_HANDLETYPE hComponent,
                                   OMX_U32 nPortIndex,
                                   OMX_PTR pBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError = hDIO->cancel(hDIO, pBuffHeader);

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Dup:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Dup (OMX_HANDLETYPE hComponent,
                                OMX_U32 nPortIndex,
                                OMX_PTR pBuffHeader,
                                OMX_PTR pDupBuffHeader)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError = hDIO->dup(hDIO, pBuffHeader, pDupBuffHeader);

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_Control:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Control (OMX_HANDLETYPE hComponent,
                                    OMX_U32 nPortIndex,
                                    OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                                    OMX_PTR pParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError = hDIO->control(hDIO, nCmdType, pParams);

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_DIO_GetCount:
 *  see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_GetCount (OMX_HANDLETYPE hComponent,
                                     OMX_U32 nPortIndex,
                                     OMX_U32 *pCount)
{

    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    /*Resetting count to 0 initially*/
    *pCount = 0;
    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);

    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    OMX_BASE_REQUIRE(hDIO != NULL, OMX_ErrorBadParameter);

    eError = hDIO->getcount(hDIO, pCount);

EXIT:
    return (eError);
}

OMX_ERRORTYPE OMX_BASE_DIO_Util (OMX_HANDLETYPE hComponent,
                                 OMX_U32 nPortIndex,
                                 OMX_BASE_DIO_UTIL_TYPE nUtilType,
                                 OMX_PTR pParams)
{
    OMX_ERRORTYPE              eError = OMX_ErrorNone;
    OMX_BASE_DIO_OBJECTTYPE   *hDIO = NULL;
    OMX_BASE_PORTTYPE         *pPort = NULL;

    pPort = _OMX_BASE_DIO_GetPort(hComponent, nPortIndex);
    OMX_BASE_REQUIRE(pPort != NULL, OMX_ErrorBadParameter);
    hDIO = (OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO;
    if( hDIO == NULL ) {
        switch( nUtilType ) {
            case OMX_BASE_DIO_UTIL_GetTotalBufferSize :
                eError = _OMX_BASE_DIO_DefaultGetTotalBufferSize(hComponent,
                                                                 nPortIndex,
                                                                 pParams);
                break;
            case OMX_BASE_DIO_UTIL_UpdatePacketExtraDataPointer :
                eError = _OMX_BASE_DIO_DefaultUpdatePacketExtraDataPointer(
                    hComponent,
                    nPortIndex,
                    pParams);
                break;
            default :
                OMX_BASE_Trace("Unsupported Index received ");
                eError = OMX_ErrorUnsupportedIndex;
                break;
        }
    } else {
        eError = hDIO->util(hDIO, nUtilType, pParams);
    }
EXIT:
    return (eError);
}

/*===============================================================*/
/* PRIVATE FUNCTIONS    */
/*===============================================================*/

/*===============================================================*/
/** @fn _OMX_BASE_DIO_GetPort:
 *  To return the valid port from the Array of ports
 */
/*===============================================================*/
static OMX_PTR _OMX_BASE_DIO_GetPort(OMX_HANDLETYPE hComponent, OMX_U32 nPortIndex)
{
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  nStartPortNumber = 0;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    if( pBaseComInt->ports == NULL ) {
        pPort = NULL;
        goto EXIT;
    }
    pPort = (OMX_BASE_PORTTYPE *)pBaseComInt->ports[nPortIndex - nStartPortNumber];

EXIT:
    return (pPort);
}

OMX_ERRORTYPE _OMX_BASE_DIO_DefaultGetTotalBufferSize(OMX_HANDLETYPE hComponent,
                                                      OMX_U32 nPortIndex,
                                                      OMX_PTR pDioBufHeader)
{
    OMX_ERRORTYPE           eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)
                                          pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)
                                           pBaseComPvt->hOMXBaseInt;
    //OMX_PTR pOrigHeader = pDioBufHeader;
    OMX_U32    nIndex = nPortIndex - pBaseComPvt->tDerToBase.nMinStartPortIndex;
    OMX_U32    nNumFields = pBaseComPvt->tDerToBase.pPortProperties[nIndex]->
                            nNumMetaDataFields;
    OMX_U32    i = 0, nSize = 0, nBitField = 0, nMask = 1;
    OMX_S32    nPosInTable;

    //nSize += ((OMX_BUFFERHEADERTYPE *)(pDioBufHeader))->nAllocLen;
    /*To take care of 4 byte alignment*/
    nSize = 4 * ((nSize + 3) / 4);
    if( nNumFields == 0 ) {
        goto EXIT;
    }
    /*Moving pointer to 1st extra data*/
    pDioBufHeader = (OMX_PTR)((OMX_U32)pDioBufHeader +
                              sizeof(OMX_BUFFERHEADERTYPE));
    nBitField = *(OMX_U32 *)pDioBufHeader;
    if( nBitField == 0 ) {
        goto EXIT;
    }
    pDioBufHeader =  (OMX_PTR)((OMX_U32)pDioBufHeader + sizeof(OMX_U32));

    for( i = 0; i < nNumFields; i++ ) {
        nPosInTable = _OMX_BASE_FindInTable(
            pBaseComPvt->tDerToBase.pPortProperties[nIndex]->
            pMetaDataFieldTypesArr[i]);
        if( nPosInTable == -1 ) {
            eError = OMX_ErrorNoMore;
            goto EXIT;
        }
        if( !(nBitField & nMask)) {
            pDioBufHeader = (OMX_PTR)((OMX_U32)pDioBufHeader +
                                      OMX_BASE_ExtraDataTable[nPosInTable].nSize);
            nMask *= 2;
            continue;
        }
        if( OMX_BASE_ExtraDataTable[nPosInTable].bPacketData == OMX_TRUE ) {
            pDioBufHeader = (OMX_PTR)((OMX_U32)pDioBufHeader + sizeof(OMX_U32));
            nSize += (*(OMX_U32 *)(pDioBufHeader));
            nSize += (EXTRADATATYPE_SIZE);
            /*To take care of 4 byte alignment*/
            nSize = 4 * ((nSize + 3) / 4);
            pDioBufHeader = (OMX_PTR)((OMX_U32)pDioBufHeader + sizeof(OMX_U32));
        } else {
            nSize += OMX_BASE_ExtraDataTable[nPosInTable].nSize;
            nSize += (EXTRADATATYPE_SIZE);
            /*To take care of 4 byte alignment*/
            nSize = 4 * ((nSize + 3) / 4);
            pDioBufHeader = (OMX_PTR)((OMX_U32)pDioBufHeader +
                                      OMX_BASE_ExtraDataTable[nPosInTable].nSize);
        }
        nMask *= 2;
    }

    /*The last empty structure that will indicate end of extra data*/
    nSize += sizeof(OMX_OTHER_EXTRADATATYPE);
    /*To take care of 4 byte alignment*/
    nSize = 4 * ((nSize + 3) / 4);

EXIT:
    pBaseComInt->ports[nIndex]->nMetaDataSize = nSize;
    //((OMX_BUFFERHEADERTYPE *)(pOrigHeader))->nAllocLen = nSize;
    return (eError);
}

OMX_ERRORTYPE _OMX_BASE_DIO_DefaultUpdatePacketExtraDataPointer(
    OMX_HANDLETYPE hComponent,
    OMX_U32 nPortIndex,
    OMX_PTR pDioBufHeader)
{
    OMX_ERRORTYPE           eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)
                                          pComp->pComponentPrivate;
    OMX_OTHER_EXTRADATATYPE   *pExtra = NULL;
    OMX_U8                    *pTmp = NULL;
    OMX_U32                    nBitField = 0, i = 0, nMask = 1;
    OMX_S32                    nPosInTable;
    OMX_PTR                    pRunningPtr = NULL;
    OMX_U32                    nIndex = nPortIndex - pBaseComPvt->tDerToBase.nMinStartPortIndex;
    OMX_U32                    nNumFields = pBaseComPvt->tDerToBase.pPortProperties[nIndex]->
                                            nNumMetaDataFields;
    OMX_BUFFERHEADERTYPE   *pOMXBufHeader = (OMX_BUFFERHEADERTYPE *)
                                            (pDioBufHeader);

    /*Nothing to do*/
    if( nNumFields == 0 ) {
        goto EXIT;
    }
    pRunningPtr = (OMX_PTR)((OMX_U32)pDioBufHeader +
                            sizeof(OMX_BUFFERHEADERTYPE));
    nBitField = *(OMX_U32 *)pRunningPtr;
    /*Nothing to do*/
    if( nBitField == 0 ) {
        goto EXIT;
    }
    /*Moving to 1st extra data in header*/
    pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr + sizeof(OMX_U32));

    //pTmp = pOMXBufHeader->pBuffer + pOMXBufHeader->nFilledLen + 3;
    pTmp = (OMX_U8 *)((OMX_U32)(((OMX_TI_PLATFORMPRIVATE *)
                                 pOMXBufHeader->pPlatformPrivate)->
                                pMetaDataBuffer) + 3);

    /*Moving to location for 1st extra data in buffer*/
    pExtra = (OMX_OTHER_EXTRADATATYPE *) (((OMX_U32) pTmp) & ~3);

    for( i = 0; i < nNumFields; i++ ) {
        nPosInTable = _OMX_BASE_FindInTable(
            pBaseComPvt->tDerToBase.pPortProperties[nIndex]->
            pMetaDataFieldTypesArr[i]);
        if( nPosInTable == -1 ) {
            eError = OMX_ErrorNoMore;
            goto EXIT;
        }
        /*Derived component does not care about this field, so move on*/
        if( !(nBitField & nMask)) {
            pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                    OMX_BASE_ExtraDataTable[nPosInTable].nSize);
            nMask *= 2;
            continue;
        }
        /*Derived component will use this field*/
        else {
            if( OMX_BASE_ExtraDataTable[nPosInTable].bPacketData == OMX_FALSE ) {
                pExtra->nSize = (EXTRADATATYPE_SIZE) +
                                OMX_BASE_ExtraDataTable[nPosInTable].nSize;
                /*To take care of 4 byte alignment*/
                pExtra->nSize = 4 * ((pExtra->nSize + 3) / 4);
                pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr + OMX_BASE_ExtraDataTable[nPosInTable].nSize);
            } else {
                *(OMX_U32 *)pRunningPtr = (OMX_U32)(pExtra->data);
                pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                        sizeof(OMX_U32));
                pExtra->nSize = (EXTRADATATYPE_SIZE) +
                                (*(OMX_U32 *)pRunningPtr);
                pExtra->nSize = 4 * ((pExtra->nSize + 3) / 4);
                pRunningPtr = (OMX_PTR)((OMX_U32)pRunningPtr +
                                        sizeof(OMX_U32));
            }
            pExtra = (OMX_OTHER_EXTRADATATYPE *) (((OMX_U8 *) pExtra) + pExtra->nSize);
        }
        nMask *= 2;
    }

EXIT:
    return (eError);
}

