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

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <omx_base.h>
#include <omx_base_internal.h>
#include <timm_osal_interfaces.h>
#include <omx_base_utils.h>
#include <omx_base_dio_plugin.h>

#define _OMX_BASE_HANDLE_IDLE_TO_LOADED_TRANSITION() do { \
        for( i=0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) \
        {   \
            pPort = pBaseComInt->ports[i]; \
            if( pPort->pBufAllocFreeEvent ) \
            { \
                tStatus = TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent, \
                                             BUF_FAIL_EVENT, TIMM_OSAL_EVENT_OR); \
                if( tStatus != TIMM_OSAL_ERR_NONE ) { \
                    eError = OMX_ErrorUndefined; } \
            } \
        } \
        retEvents = 0; \
        tStatus = TIMM_OSAL_EventRetrieve( \
            pBaseComInt->pErrorCmdcompleteEvent, \
            ERROR_EVENT, TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, \
            STATE_TRANSITION_LONG_TIMEOUT); \
        OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, \
                        OMX_ErrorUndefined); \
} while( 0 )


/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*----------         function prototypes      ------------------- */
/*----------         data declarations        ------------------- */
extern OMX_BASE_EXTRA_DATA_TABLE    OMX_BASE_ExtraDataTable[];
/*******************************************************************
 * PUBLIC DECLARATIONS: defined here, used elsewhere
 *******************************************************************/
/*----------         data declarations        ------------------- */
/*----------         function prototypes      ------------------- */
/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------          data declarations        ------------------- */
/*----------          function prototypes      ------------------- */
void _OMX_BASE_HandleFailEvent(OMX_HANDLETYPE hComponent, OMX_COMMANDTYPE eCmd,
                               OMX_U32 nPortIndex);

/*===============================================================*/
/** @fn _OMX_BASE_InitializePorts :
 *
 *  @ see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_InitializePorts(OMX_HANDLETYPE hComponent)
{
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_ERRORTYPE            eError = OMX_ErrorNone, eTmpError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_U32                  i = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_InitializePorts");
    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    pBaseComInt->ports = (OMX_BASE_PORTTYPE * *)OMX_BASE_Malloc(
        sizeof(OMX_BASE_PORTTYPE *) *
        pBaseComPvt->tDerToBase.nNumPorts,
        pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(pBaseComInt->ports != NULL, OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(pBaseComInt->ports, 0, sizeof(OMX_BASE_PORTTYPE *) *
                     pBaseComPvt->tDerToBase.nNumPorts);

    pBaseComPvt->pPortdefs = (OMX_PARAM_PORTDEFINITIONTYPE * *)OMX_BASE_Malloc(
        sizeof(OMX_PARAM_PORTDEFINITIONTYPE *) *
        pBaseComPvt->tDerToBase.nNumPorts,
        pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(pBaseComPvt->pPortdefs != NULL,
                    OMX_ErrorInsufficientResources);
    TIMM_OSAL_Memset(pBaseComPvt->pPortdefs, 0,
                     sizeof(OMX_PARAM_PORTDEFINITIONTYPE *) *
                     pBaseComPvt->tDerToBase.nNumPorts);

    pBaseComPvt->pBufPreAnnouncePerPort = (OMX_TI_PARAM_BUFFERPREANNOUNCE * *)
                                          OMX_BASE_Malloc(sizeof(
                                                              OMX_TI_PARAM_BUFFERPREANNOUNCE *) *
                                                          pBaseComPvt->tDerToBase.nNumPorts,
                                                          pBaseComPvt->tDerToBase.hDefaultHeap);
    OMX_BASE_ASSERT(pBaseComPvt->pBufPreAnnouncePerPort != NULL,
                    OMX_ErrorInsufficientResources);

    for( i = 0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
        pBaseComInt->ports[i] = (OMX_BASE_PORTTYPE *)OMX_BASE_Malloc(
            sizeof(OMX_BASE_PORTTYPE),
            pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(pBaseComInt->ports[i] != NULL,
                        OMX_ErrorInsufficientResources);
        TIMM_OSAL_Memset(pBaseComInt->ports[i], 0x0,
                         sizeof(OMX_BASE_PORTTYPE));

        pBaseComInt->ports[i]->hTunnelComp      =  NULL;
        pBaseComInt->ports[i]->nTunnelFlags     =  OMX_BASE_DIO_NONTUNNELED;

        /*This is being set to unspecified here. This will be updated in
        SetCallbacks function based on the allocator preferences set by the
        derived component*/
        pBaseComInt->ports[i]->eSupplierSetting = OMX_BufferSupplyUnspecified;

        OMX_BASE_INIT_STRUCT_PTR(&(pBaseComInt->ports[i]->sPortDef),
                                 OMX_PARAM_PORTDEFINITIONTYPE);
        pBaseComPvt->pPortdefs[i] = &(pBaseComInt->ports[i]->sPortDef);

        OMX_BASE_INIT_STRUCT_PTR(&(pBaseComInt->ports[i]->sBufRefNotify),
                                 OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE);
        pBaseComInt->ports[i]->sBufRefNotify.nPortIndex =
            pBaseComPvt->tDerToBase.nMinStartPortIndex + i;

        tStatus = TIMM_OSAL_EventCreate(
            &(pBaseComInt->ports[i]->pBufAllocFreeEvent));
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                        OMX_ErrorInsufficientResources);
        tStatus = TIMM_OSAL_SemaphoreCreate(
            &(pBaseComInt->ports[i]->pDioOpenCloseSem), 0);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                        OMX_ErrorInsufficientResources);

        pBaseComPvt->pBufPreAnnouncePerPort[i] =
            (OMX_TI_PARAM_BUFFERPREANNOUNCE *)
            OMX_BASE_Malloc(sizeof(
                                OMX_TI_PARAM_BUFFERPREANNOUNCE),
                            pBaseComPvt->tDerToBase.hDefaultHeap);
        OMX_BASE_ASSERT(pBaseComPvt->pBufPreAnnouncePerPort[i] != NULL,
                        OMX_ErrorInsufficientResources);
        OMX_BASE_INIT_STRUCT_PTR(pBaseComPvt->pBufPreAnnouncePerPort[i],
                                 OMX_TI_PARAM_BUFFERPREANNOUNCE);
        pBaseComPvt->pBufPreAnnouncePerPort[i]->nPortIndex =
            pBaseComPvt->tDerToBase.nMinStartPortIndex + i;
        pBaseComPvt->pBufPreAnnouncePerPort[i]->bEnabled = OMX_TRUE;
    }

EXIT:
    if( OMX_ErrorNone != eError ) {
        eTmpError = eError;
        eError = _OMX_BASE_DeinitializePorts(hComponent);
        eError = eTmpError;
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_InitializePorts", eError);
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_BASE_DeinitializePorts :
 *
 *  @ see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_DeinitializePorts(OMX_HANDLETYPE hComponent)
{
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_U32                  i = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_DeinitializePorts");
    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_ASSERT(pBaseComPvt != NULL, eError);

    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    for( i=0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
        if( !pBaseComInt ) {
            break;
        }
        if( !(pBaseComInt->ports)) {
            break;
        }
        pPort = pBaseComInt->ports[i];
        if( pPort == NULL ) {
            continue;
        }
        if( pPort->pDioOpenCloseSem ) {
            tStatus = TIMM_OSAL_SemaphoreDelete(pPort->pDioOpenCloseSem);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
            }
        }
        /*If any tasks are waiting on this event then send fail event to
          indicate that component is being unloaded*/
        if( pPort->pBufAllocFreeEvent ) {
            tStatus = TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent,
                                         BUF_FAIL_EVENT, TIMM_OSAL_EVENT_OR);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
            }
            tStatus = TIMM_OSAL_EventDelete(pPort->pBufAllocFreeEvent);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
            }
        }
        if( pBaseComPvt->pBufPreAnnouncePerPort ) {
            if( pBaseComPvt->pBufPreAnnouncePerPort[i] ) {
                TIMM_OSAL_Free(pBaseComPvt->pBufPreAnnouncePerPort[i]);
                pBaseComPvt->pBufPreAnnouncePerPort[i] = NULL;
            }
        }
        TIMM_OSAL_Free(pPort);
        pPort = NULL;
    }

    if( pBaseComPvt->pBufPreAnnouncePerPort ) {
        TIMM_OSAL_Free(pBaseComPvt->pBufPreAnnouncePerPort);
        pBaseComPvt->pBufPreAnnouncePerPort = NULL;
    }
    if( pBaseComPvt->pPortdefs ) {
        TIMM_OSAL_Free(pBaseComPvt->pPortdefs);
        pBaseComPvt->pPortdefs = NULL;
    }
    if( pBaseComInt ) {
        if( pBaseComInt->ports ) {
            TIMM_OSAL_Free(pBaseComInt->ports);
            pBaseComInt->ports = NULL;
        }
    }
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_DeinitializePorts", eError);
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_BASE_HandleStateTransition :
 *
 *  @ see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_HandleStateTransition(OMX_HANDLETYPE hComponent,
                                              OMX_U32 nParam)
{
    OMX_ERRORTYPE           eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE     tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)
                                          pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)
                                           pBaseComPvt->hOMXBaseInt;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_U32                    i = 0, nStartPortNum = 0, nPorts = 0;
    TIMM_OSAL_U32              retEvents = 0;
    OMX_BASE_DIO_OpenParams    sDIOParams;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_HandleStateTransition");
    nPorts =  pBaseComPvt->tDerToBase.nNumPorts;
    nStartPortNum = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    /* currnet and new state should not be same */
    OMX_BASE_ASSERT(pBaseComPvt->tCurState != pBaseComPvt->tNewState,
                    OMX_ErrorSameState);
    /* Transition to invaild state by IL client is disallowed */
    if( pBaseComPvt->tNewState == OMX_StateInvalid ) {
        /* Notify to Derived Component */
        pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                OMX_CommandStateSet, nParam, NULL);
        /*Derived component has returned*/
        tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent,
                                          OMX_BASE_CmdStateSet, TIMM_OSAL_EVENT_OR_CONSUME,
                                          &retEvents, TIMM_OSAL_SUSPEND);
        OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                        OMX_ErrorInsufficientResources);

        /*
        For this case we wont go through _OMX_BASE_EventNotifyToClient function
        The state will be set here and error callback will be made by
        OMX_BASE_PROCESS_Events function
        */
        pBaseComPvt->tCurState = pBaseComPvt->tNewState;
        eError = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch( pBaseComPvt->tCurState ) {
        case OMX_StateLoaded :
            if( pBaseComPvt->tNewState == OMX_StateIdle ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(
                    pBaseComInt->pCmdCompleteEvent,
                    OMX_BASE_CmdStateSet, TIMM_OSAL_EVENT_OR_CONSUME,
                    &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                                OMX_ErrorInsufficientResources);

                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = (OMX_BASE_PORTTYPE *)
                            pBaseComInt->ports[i - nStartPortNum];
                    /*If port is disabled then nothing needs to be done*/
                    if( pPort->sPortDef.bEnabled == OMX_FALSE ) {
                        continue;
                    }
                    if( pPort->sPortDef.eDir == OMX_DirOutput ) {
                        sDIOParams.nMode = OMX_BASE_DIO_WRITER;
                    } else {
                        sDIOParams.nMode = OMX_BASE_DIO_READER;
                    }
                    if( pBaseComPvt->tDerToBase.pPortProperties[i -
                                                                nStartPortNum]->bDMAAccessedBuffer ) {
                        sDIOParams.bCacheFlag = OMX_FALSE;
                    } else {
                        sDIOParams.bCacheFlag = OMX_TRUE;
                    }
                    /*Same tunnel is established again if move from idle
                    to loaded and back to idle again*/
                    if( pPort->hDIO == NULL && PORT_IS_TUNNELED(pPort)) {
                        eError = _OMX_BASE_TunnelEstablish(hComponent, i,
                                                           pPort->hTunnelComp, pPort->nTunnelPort);
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                    /*Call Open for tunneled ports*/
                    if( pPort->hDIO != NULL && PORT_IS_TUNNELED(pPort)) {
                        eError = OMX_BASE_DIO_Open(hComponent, i,
                                                   (OMX_PTR)(&sDIOParams));
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                }
            } else if( pBaseComPvt->tNewState == OMX_StateWaitForResources ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);
            } else {
                eError = OMX_ErrorIncorrectStateTransition;
                goto EXIT;
            }
            break;

        case OMX_StateIdle :
            if( pBaseComPvt->tNewState == OMX_StateLoaded ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);

                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = (OMX_BASE_PORTTYPE *)pBaseComInt->ports[i - nStartPortNum];
                    if( pPort->sPortDef.bEnabled == OMX_TRUE ) {
                        if( PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort)) {
                            pPort->sPortDef.bPopulated = OMX_FALSE;
                            eError = OMX_BASE_DIO_Close(hComponent, i);
                            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                            eError = OMX_BASE_DIO_Deinit(hComponent, i);
                            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                        }
                    }
                }
            } else if( pBaseComPvt->tNewState == OMX_StateExecuting ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);
            } else if( pBaseComPvt->tNewState == OMX_StatePause ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);
            } else {
                eError = OMX_ErrorIncorrectStateTransition;
                goto EXIT;
            }
            break;

        case OMX_StateExecuting :
            if( pBaseComPvt->tNewState == OMX_StateIdle ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);

                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = (OMX_BASE_PORTTYPE *)pBaseComInt->ports[i - nStartPortNum];
                    if( pPort->hDIO != NULL ) {
                        eError = OMX_BASE_DIO_Control(hComponent, i,
                                                      OMX_BASE_DIO_CTRLCMD_Stop, NULL);
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                }
            } else if( pBaseComPvt->tNewState == OMX_StatePause ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);
            } else {
                eError = OMX_ErrorIncorrectStateTransition;
                goto EXIT;
            }
            break;

        case OMX_StatePause :
            if( pBaseComPvt->tNewState == OMX_StateExecuting ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);

                /*Pause to Executing so start processing buffers*/
                pBaseComInt->fpInvokeProcessFunction(pComp, DATAEVENT);
            } else if( pBaseComPvt->tNewState == OMX_StateIdle ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);

                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = (OMX_BASE_PORTTYPE *)pBaseComInt->ports[i - nStartPortNum];
                    if( pPort->hDIO != NULL ) {
                        eError = OMX_BASE_DIO_Control(hComponent, i,
                                                      OMX_BASE_DIO_CTRLCMD_Stop, NULL);
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                }
            } else {
                eError = OMX_ErrorIncorrectStateTransition;
                goto EXIT;
            }
            break;

        case OMX_StateWaitForResources :
            if( pBaseComPvt->tNewState == OMX_StateLoaded ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);
            } else if( pBaseComPvt->tNewState == OMX_StateIdle ) {
                /* Notify to Derived Component */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandStateSet, nParam, NULL);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent, OMX_BASE_CmdStateSet,
                                                  TIMM_OSAL_EVENT_OR_CONSUME, &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE, OMX_ErrorUndefined);

                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = (OMX_BASE_PORTTYPE *)
                            pBaseComInt->ports[i - nStartPortNum];
                    /*If port is disabled then nothing needs to be done*/
                    if( pPort->sPortDef.bEnabled == OMX_FALSE ) {
                        continue;
                    }
                    if( pPort->sPortDef.eDir == OMX_DirOutput ) {
                        sDIOParams.nMode = OMX_BASE_DIO_WRITER;
                    } else {
                        sDIOParams.nMode = OMX_BASE_DIO_READER;
                    }
                    if( pBaseComPvt->tDerToBase.pPortProperties[i -
                                                                nStartPortNum]->bDMAAccessedBuffer ) {
                        sDIOParams.bCacheFlag = OMX_FALSE;
                    } else {
                        sDIOParams.bCacheFlag = OMX_TRUE;
                    }
                    /*Same tunnel is established again if move from idle
                    to loaded and back to idle again*/
                    if( pPort->hDIO == NULL && PORT_IS_TUNNELED(pPort)) {
                        eError = _OMX_BASE_TunnelEstablish(hComponent, i,
                                                           pPort->hTunnelComp, pPort->nTunnelPort);
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                    /*Call Open for tunneled ports*/
                    if( pPort->hDIO != NULL && PORT_IS_TUNNELED(pPort)) {
                        eError = OMX_BASE_DIO_Open(hComponent, i,
                                                   (OMX_PTR)(&sDIOParams));
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                }
            } else {
                eError = OMX_ErrorIncorrectStateTransition;
                goto EXIT;
            }
            break;

        default :
            OMX_BASE_Trace(" unknown command ");
            break;

    }

EXIT:
    if( eError != OMX_ErrorNone ) {
        /* Since no state transition is in progress put the new state again to OMX_StateMax */
        pBaseComPvt->tNewState = OMX_StateMax;
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_HandleStateTransition", eError);
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_BASE_DisablePort :
 *
 *  @ see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_DisablePort(OMX_HANDLETYPE hComponent,
                                    OMX_U32 nParam)
{

    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  nStartPortNum;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_DisablePort");
    nStartPortNum = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    pPort = pBaseComInt->ports[nParam - nStartPortNum];
    /* If comp is in loaded state, then there wont be any buffers to free up */
    if((pBaseComPvt->tCurState == OMX_StateLoaded)
       || (pBaseComPvt->tCurState == OMX_StateWaitForResources)) {
        goto EXIT;
    }
    eError = OMX_BASE_DIO_Control(hComponent, nParam,
                                  OMX_BASE_DIO_CTRLCMD_Stop, NULL);
    OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
    if( PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort)) {
        pPort->sPortDef.bPopulated = OMX_FALSE;
        eError = OMX_BASE_DIO_Close(hComponent, nParam);
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
        eError = OMX_BASE_DIO_Deinit(hComponent, nParam);
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
    }
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_DisablePort", eError);
    return (eError);

}

/*===============================================================*/
/** @fn _OMX_BASE_EnablePort :
 *
 *  @ see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_EnablePort(OMX_HANDLETYPE hComponent,
                                   OMX_U32 nParam)
{
    OMX_ERRORTYPE           eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)
                                          pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)
                                           pBaseComPvt->hOMXBaseInt;
    OMX_BASE_PORTTYPE         *pPort = NULL;
    OMX_U32                    nStartPortNum = 0;
    OMX_BASE_DIO_OpenParams    sDIOParams;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_EnablePort");
    nStartPortNum = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    pPort = pBaseComInt->ports[nParam - nStartPortNum];
    if( pBaseComPvt->tCurState != OMX_StateLoaded &&
        pBaseComPvt->tCurState != OMX_StateWaitForResources ) {
        if( pPort->sPortDef.eDir == OMX_DirOutput ) {
            sDIOParams.nMode = OMX_BASE_DIO_WRITER;
        } else {
            sDIOParams.nMode = OMX_BASE_DIO_READER;
        }
        if( pBaseComPvt->tDerToBase.pPortProperties[nParam - nStartPortNum]->
            bDMAAccessedBuffer ) {
            sDIOParams.bCacheFlag = OMX_FALSE;
        } else {
            sDIOParams.bCacheFlag = OMX_TRUE;
        }
        /*If setup tunnel was not called after disablement then establish
        tunnel for tunneled ports*/
        if( pPort->hDIO == NULL && PORT_IS_TUNNELED(pPort)) {
            eError = _OMX_BASE_TunnelEstablish(hComponent, nParam,
                                               pPort->hTunnelComp, pPort->nTunnelPort);
            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
        }
        /*Call Open for tunneled ports*/
        if( pPort->hDIO != NULL && PORT_IS_TUNNELED(pPort)) {
            eError = OMX_BASE_DIO_Open(hComponent, nParam,
                                       (OMX_PTR)(&sDIOParams));
            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
        }
    }
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_EnablePort", eError);
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_BASE_FlushBuffers :
 *
 *  @ see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_FlushBuffers(OMX_HANDLETYPE hComponent,
                                     OMX_U32 nParam)
{
    OMX_ERRORTYPE    eError = OMX_ErrorNone;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_FlushBuffers");
    eError = OMX_BASE_DIO_Control(hComponent, nParam, OMX_BASE_DIO_CTRLCMD_Flush,
                                  NULL);

    goto EXIT;

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_FlushBuffers", eError);
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_BASE_GetChannelInfo :
 *
 *  @ see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_GetChannelInfo(OMX_BASE_PORTTYPE *pPort,
                                       OMX_STRING cChannelName)
{
    OMX_ERRORTYPE    eError = OMX_ErrorNone;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_GetChannelInfo");
    strncpy(cChannelName, (char *)pPort->cChannelName, OMX_BASE_MAXNAMELEN);

    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_GetChannelInfo", eError);
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_BASE_EventNotifyToClient :
 *
 *  @ see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_EventNotifyToClient(OMX_HANDLETYPE hComponent,
                                            OMX_COMMANDTYPE Cmd,
                                            OMX_U32 nParam,
                                            OMX_PTR pCmdData)
{

    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    TIMM_OSAL_U32            retEvents = 0;
    OMX_U32                  i, nStartPortNum, nPorts;
    OMX_BOOL                 bStartFlag = OMX_FALSE;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_EventNotifyToClient");
    nPorts =  pBaseComPvt->tDerToBase.nNumPorts;
    nStartPortNum = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    OMX_BASE_Info("{base} NotfyEvent-2-A9: Cmd %d Param %d", Cmd, nParam);

    switch( Cmd ) {

        case OMX_CommandStateSet :
            if(((pBaseComPvt->tCurState == OMX_StateLoaded ||
                 pBaseComPvt->tCurState == OMX_StateWaitForResources) &&
                pBaseComPvt->tNewState == OMX_StateIdle) ||
               (pBaseComPvt->tCurState == OMX_StateIdle &&
                pBaseComPvt->tNewState == OMX_StateLoaded)) {
                /* Incase of loaded to idle and idle to loaded state transition, comp
                 * should wait for buffers to be allocated/freed for enabled ports */
                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = pBaseComInt->ports[i - nStartPortNum];
                    if( pPort->sPortDef.bEnabled == OMX_TRUE ) {
                        retEvents = 0;
                        tStatus = TIMM_OSAL_EventRetrieve(
                            pPort->pBufAllocFreeEvent,
                            (BUF_ALLOC_EVENT | BUF_FREE_EVENT |
                             BUF_FAIL_EVENT),
                            TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                            TIMM_OSAL_SUSPEND);
                        OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                                        OMX_ErrorUndefined);
                        if( retEvents & BUF_FAIL_EVENT ) {
                            /*Fail event so free up all DIO resources and move
                            back to loaded state*/
                            _OMX_BASE_HandleFailEvent(hComponent, Cmd, nParam);
                            if( pBaseComPvt->tCurState == OMX_StateIdle &&
                                pBaseComPvt->tNewState == OMX_StateLoaded ) {
                                eError =
                                    OMX_ErrorPortUnresponsiveDuringDeallocation;
                            } else {
                                eError =
                                    OMX_ErrorPortUnresponsiveDuringAllocation;
                            }
                            goto EXIT;
                        }
                        /* free up the pool incase if idle to loaded */
                        if( pBaseComPvt->tCurState == OMX_StateIdle &&
                            pBaseComPvt->tNewState == OMX_StateLoaded ) {
                            if( !(PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort))) {
                                eError = OMX_BASE_DIO_Close(hComponent, i);
                                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                                eError = OMX_BASE_DIO_Deinit(hComponent, i);
                                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                            }
                            /*Tunneling info is not touched on an Idle to Loaded
                              transition. Component remembers tunneling info*/
                        }
                    }
                }
            } else if( pBaseComPvt->tCurState == OMX_StateIdle &&
                       (pBaseComPvt->tNewState == OMX_StatePause ||
                        pBaseComPvt->tNewState == OMX_StateExecuting)) {
                bStartFlag = OMX_TRUE;
            }
            TIMM_OSAL_MutexObtain(pBaseComInt->pNewStateMutex,
                                  TIMM_OSAL_SUSPEND);

            TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE, "STATE Transition conmpleted");
            TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE, "handle 0x%x DONE %d  --> %d ", hComponent, pBaseComPvt->tCurState, pBaseComPvt->tNewState);

            pBaseComPvt->tCurState = pBaseComPvt->tNewState;
            pBaseComPvt->tNewState = OMX_StateMax;
            /* Notify Completion to the Client */
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate,
                                                    OMX_EventCmdComplete, OMX_CommandStateSet,
                                                    nParam, NULL);
            if( bStartFlag ) {
                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = (OMX_BASE_PORTTYPE *)pBaseComInt->ports[i - nStartPortNum];
                    if( pPort->hDIO != NULL ) {
                        eError = OMX_BASE_DIO_Control(hComponent, i,
                                                      OMX_BASE_DIO_CTRLCMD_Start, NULL);
                        if( OMX_ErrorNone != eError ) {
                            TIMM_OSAL_MutexRelease(pBaseComInt->pNewStateMutex);
                            goto EXIT;
                        }
                    }
                }
            }
            TIMM_OSAL_MutexRelease(pBaseComInt->pNewStateMutex);
            break;

        case OMX_CommandPortEnable :
            if( OMX_ALL == nParam ) {
                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = pBaseComInt->ports[i - nStartPortNum];
                    if( pBaseComPvt->tCurState != OMX_StateLoaded ) {
                        retEvents = 0;

                        tStatus = TIMM_OSAL_EventRetrieve(
                            pPort->pBufAllocFreeEvent,
                            (BUF_ALLOC_EVENT | BUF_FAIL_EVENT),
                            TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                            TIMM_OSAL_SUSPEND);
                        OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                                        OMX_ErrorUndefined);
                        if( retEvents & BUF_FAIL_EVENT ) {
                            /*Fail event so free up all DIO resources and move
                            back to port in disabled state*/
                            _OMX_BASE_HandleFailEvent(hComponent, Cmd, nParam);
                            eError = OMX_ErrorPortUnresponsiveDuringAllocation;

                            for( i = 0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
                                pPort = pBaseComInt->ports[i];
                                pPort->bIsTransitionToEnabled = OMX_FALSE;
                                pPort->sPortDef.bEnabled = OMX_FALSE;
                            }

                            goto EXIT;
                        }
                    }
                    pPort->bIsTransitionToEnabled = OMX_FALSE;
                    //pPort->sPortDef.bEnabled = OMX_TRUE;

                    /* Notify Completion to the Client */
                    pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                            pComp->pApplicationPrivate,
                                                            OMX_EventCmdComplete, OMX_CommandPortEnable,
                                                            i, NULL);
                    /*If current state is executing, start buffer transfer*/
                    if( pBaseComPvt->tCurState == OMX_StateExecuting ) {
                        eError = OMX_BASE_DIO_Control(hComponent, i,
                                                      OMX_BASE_DIO_CTRLCMD_Start, NULL);
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                }
            } else {
                pPort = pBaseComInt->ports[nParam - nStartPortNum];
                if( pBaseComPvt->tCurState != OMX_StateLoaded ) {
                    retEvents = 0;
                    tStatus = TIMM_OSAL_EventRetrieve(
                        pPort->pBufAllocFreeEvent,
                        (BUF_ALLOC_EVENT | BUF_FAIL_EVENT),
                        TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                        3 * 1000); /* wait for 3sec */

                    if( tStatus == TIMM_OSAL_WAR_TIMEOUT ) {
                        pPort->bIsTransitionToEnabled = OMX_FALSE;
                        pPort->sPortDef.bEnabled = OMX_FALSE;
                    }

                    OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                                    tStatus != TIMM_OSAL_WAR_TIMEOUT ?
                                    OMX_ErrorUndefined : OMX_ErrorPortUnresponsiveDuringAllocation);

                    if( retEvents & BUF_FAIL_EVENT ) {
                        /*Fail event so free up all DIO resources and move
                            back to port in disabled state*/
                        _OMX_BASE_HandleFailEvent(hComponent, Cmd, nParam);
                        eError = OMX_ErrorPortUnresponsiveDuringAllocation;
                        pPort->bIsTransitionToEnabled = OMX_FALSE;
                        pPort->sPortDef.bEnabled = OMX_FALSE;
                        goto EXIT;
                    }
                }
                pPort->bIsTransitionToEnabled = OMX_FALSE;
                //pPort->sPortDef.bEnabled = OMX_TRUE;

                /* Notify Completion to the Client */
                pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                        pComp->pApplicationPrivate,
                                                        OMX_EventCmdComplete, OMX_CommandPortEnable,
                                                        nParam, NULL);
                /*If current state is executing, start buffer transfer*/
                if( pBaseComPvt->tCurState == OMX_StateExecuting ) {
                    eError = OMX_BASE_DIO_Control(hComponent, nParam,
                                                  OMX_BASE_DIO_CTRLCMD_Start, NULL);
                    OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                }
            }
            break;

        case OMX_CommandPortDisable :
            if( OMX_ALL == nParam ) {
                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = pBaseComInt->ports[i - nStartPortNum];
                    if( pBaseComPvt->tCurState != OMX_StateLoaded ) {
                        retEvents = 0;
                        tStatus = TIMM_OSAL_EventRetrieve(
                            pPort->pBufAllocFreeEvent,
                            (BUF_FREE_EVENT | BUF_FAIL_EVENT),
                            TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                            TIMM_OSAL_SUSPEND);
                        OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                                        OMX_ErrorUndefined);
                        if( retEvents & BUF_FAIL_EVENT ) {
                            /*Fail event so free up all DIO resources and move
                            port to disabled state*/
                            _OMX_BASE_HandleFailEvent(hComponent, Cmd, nParam);
                            eError =
                                OMX_ErrorPortUnresponsiveDuringDeallocation;

                            for( i = 0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
                                pPort = pBaseComInt->ports[i];
                                pPort->bIsTransitionToDisabled = OMX_FALSE;
                            }

                            goto EXIT;
                        }
                        if( !(PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort))) {
                            eError = OMX_BASE_DIO_Close(hComponent, i);
                            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                            eError = OMX_BASE_DIO_Deinit(hComponent, i);
                            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                        }
                        /*Tunneling info is not touched on Idle to Loaded
                        transition. Component remembers its tunneling info*/
                    }
                    pPort->bIsTransitionToDisabled = OMX_FALSE;
                    //pPort->sPortDef.bEnabled = OMX_FALSE;

                    /* Notify Completion to the Client */
                    pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                            pComp->pApplicationPrivate,
                                                            OMX_EventCmdComplete, OMX_CommandPortDisable,
                                                            i, NULL);
                }
            } else {
                pPort = pBaseComInt->ports[nParam - nStartPortNum];
                if( pBaseComPvt->tCurState != OMX_StateLoaded ) {
                    retEvents = 0;
                    tStatus = TIMM_OSAL_EventRetrieve(
                        pPort->pBufAllocFreeEvent,
                        (BUF_FREE_EVENT | BUF_FAIL_EVENT),
                        TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                        TIMM_OSAL_SUSPEND);
                    OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                                    OMX_ErrorUndefined);
                    if( retEvents & BUF_FAIL_EVENT ) {
                        /*Fail event so free up all DIO resources and move
                        back to port in disabled state*/
                        _OMX_BASE_HandleFailEvent(hComponent, Cmd, nParam);
                        eError = OMX_ErrorPortUnresponsiveDuringDeallocation;
                        pPort->bIsTransitionToDisabled = OMX_FALSE;
                        goto EXIT;
                    }
                    if( !(PORT_IS_TUNNELED(pPort) && PORT_IS_SUPPLIER(pPort))) {
                        eError = OMX_BASE_DIO_Close(hComponent, nParam);
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                        eError = OMX_BASE_DIO_Deinit(hComponent, nParam);
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                    /*Tunneling info is not touched on Idle to Loaded
                    transition. Component remembers its tunneling info*/
                }
                pPort->bIsTransitionToDisabled = OMX_FALSE;
                //pPort->sPortDef.bEnabled = OMX_FALSE;

                /* Notify Completion to the Client */
                pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                        pComp->pApplicationPrivate,
                                                        OMX_EventCmdComplete, OMX_CommandPortDisable,
                                                        nParam, NULL);
            }
            break;

        case OMX_CommandFlush :
            if( nParam == OMX_ALL ) {
                for( i = nStartPortNum; i < (nStartPortNum + nPorts); i++ ) {
                    pPort = pBaseComInt->ports[i - nStartPortNum];
                    /* Notify Completion to the Client */
                    pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                            pComp->pApplicationPrivate,
                                                            OMX_EventCmdComplete, OMX_CommandFlush,
                                                            i, NULL);
                }
            } else {
                /* Notify Completion to the Client */
                pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                        pComp->pApplicationPrivate,
                                                        OMX_EventCmdComplete, OMX_CommandFlush,
                                                        nParam, NULL);
            }
            break;

        case OMX_CommandMarkBuffer :

            break;

    }

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_EventNotifyToClient", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_IsCmdPending() :
 *
 *  @ see omx_base.h.
 */
/*===============================================================*/
OMX_BOOL OMX_BASE_IsCmdPending (OMX_HANDLETYPE hComponent)
{
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)
                                          pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)
                                           pBaseComPvt->hOMXBaseInt;
    OMX_BOOL    bRetVal = OMX_FALSE;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_IsCmdPending");
    if( TIMM_OSAL_IsPipeReady(pBaseComInt->pCmdPipe) == TIMM_OSAL_ERR_NONE ) {
        /*Set data event so that derived component can get data notification
          after it processes the pending command*/
        pBaseComInt->bForceNotifyOnce = OMX_TRUE;
        pBaseComInt->fpInvokeProcessFunction(hComponent, DATAEVENT);
        bRetVal = OMX_TRUE;
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_IsCmdPending", bRetVal);
    return (bRetVal);
}

/*===============================================================*/
/** @fn OMX_BASE_SetDefaultProperties() :
 *
 *  @ see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SetDefaultProperties(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE           eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)
                                          pComp->pComponentPrivate;
    OMX_U32    nIndx = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetDefaultProperties");

    for( nIndx = 0; nIndx < (pBaseComPvt->tDerToBase.nNumPorts); nIndx++ ) {
        //pBaseComPvt->tDerToBase.pPortProperties[nIndx]->sTunPref.nTunnelFlags = OMX_PORTTUNNELFLAG_READONLY;
        //pBaseComPvt->tDerToBase.pPortProperties[nIndx]->sTunPref.eSupplier = OMX_BufferSupplyUnspecified;

        /*Buf allocation preferences is a don't care*/
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->eBufAllocPref =
            OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_DEFAULT;
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->bReadOnlyBuffers =
            OMX_TRUE;

        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->nWatermark = 1;

        /*This is set to NULL so hDefaultHeap will be used for allocating buffers*/
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->hBufHeapPerPort = NULL;

        /*Frame mode is the default mode*/
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->eDataAccessMode =
            OMX_BASE_DATA_ACCESS_MODE_FRAME;
        /*By default buffers are not accessed via DMA*/
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->bDMAAccessedBuffer =
            OMX_FALSE;
        /*No meta data is being sent*/
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->nNumMetaDataFields = 0;
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->pMetaDataFieldTypesArr =
            NULL;

        /*Buffer allocation type is set to default*/
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->eBufMemoryType =
            OMX_BASE_BUFFER_MEMORY_DEFAULT;
        /*Number of component buffers set to 1*/
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->nNumComponentBuffers =
            1;
        /*No bufefr params by default. To be used in case of 2D buffers*/
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->pBufParams = NULL;
        pBaseComPvt->tDerToBase.pPortProperties[nIndx]->nTimeoutForDequeue = TIMM_OSAL_SUSPEND;
    }

    pBaseComPvt->tDerToBase.bNotifyForAnyPort = OMX_TRUE;

    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetDefaultProperties", eError);
    return (eError);
}

OMX_ERRORTYPE OMX_BASE_GetUVBuffer(OMX_HANDLETYPE hComponent,
                                   OMX_U32 nPortIndex,
                                   OMX_PTR pBufHdr, OMX_PTR *pUVBuffer)
{
    OMX_ERRORTYPE    eError = OMX_ErrorNone;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetUVBuffer");
    OMX_BASE_REQUIRE(pBufHdr != NULL, OMX_ErrorBadParameter);
    OMX_BASE_REQUIRE(((OMX_BUFFERHEADERTYPE *)pBufHdr)->pPlatformPrivate !=
                     NULL, OMX_ErrorBadParameter);

    *pUVBuffer = ((OMX_TI_PLATFORMPRIVATE *)((OMX_BUFFERHEADERTYPE *)pBufHdr)->
                  pPlatformPrivate)->pAuxBuf1;
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetUVBuffer", eError);
    return (eError);
}

//Returns position if found in table else returns -1
OMX_S32 _OMX_BASE_FindInTable(OMX_EXTRADATATYPE eExtraDataType)
{
    OMX_U32    i = 0;
    OMX_S32    nRetVal = -1;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_FindInTable");

    for( i = 0; OMX_BASE_ExtraDataTable[i].eExtraDataType != 0; i++ ) {
        if( eExtraDataType == OMX_BASE_ExtraDataTable[i].eExtraDataType ) {
            /*The extra data type was found in the table*/
            nRetVal = i;
            break;
        }
    }

    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_FindInTable", nRetVal);
    return (nRetVal);
}

/*===============================================================*/
/** @fn OMX_BASE_IsDioReady :
 *
 *  @ see omx_base_utils.h
 */
/*===============================================================*/
OMX_BOOL OMX_BASE_IsDioReady(OMX_HANDLETYPE hComponent, OMX_U32 nPortIndex)
{
    OMX_BOOL                bRet = OMX_TRUE;
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt =
        (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  nStartPortNumber = 0;

    if( pBaseComPvt == NULL ) {
        OMX_BASE_Trace("Pvt structure is NULL - DIO cannot be used");
        bRet = OMX_FALSE;
        goto EXIT;
    }
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    if( pBaseComInt == NULL ) {
        OMX_BASE_Trace("Base internal structure is NULL - DIO cannot be used");
        bRet = OMX_FALSE;
        goto EXIT;
    }
    if( pBaseComInt->ports == NULL ) {
        OMX_BASE_Trace("No port has been initialized - DIO cannot be used");
        bRet = OMX_FALSE;
        goto EXIT;
    }
    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    pPort = (OMX_BASE_PORTTYPE *)
            pBaseComInt->ports[nPortIndex - nStartPortNumber];
    if( pPort == NULL ) {
        OMX_BASE_Trace("This port is not initialized - DIO cannot be used");
        bRet = OMX_FALSE;
        goto EXIT;
    }
    if( pPort->hDIO == NULL ) {
        OMX_BASE_Trace("DIO handle is NULL - DIO cannot be used");
        bRet = OMX_FALSE;
        goto EXIT;
    }
    if( !(((OMX_BASE_DIO_OBJECTTYPE *)pPort->hDIO)->bOpened)) {
        OMX_BASE_Trace("DIO has not yet been opened");
        bRet = OMX_FALSE;
        goto EXIT;
    }
EXIT:
    return (bRet);
}

OMX_ERRORTYPE _OMX_BASE_Error_HandleWFRState(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE            eError  = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    TIMM_OSAL_U32            retEvents = 0;

    eError = OMX_BASE_SendCommand(hComponent, OMX_CommandStateSet, OMX_StateLoaded, NULL);

    //Wait for WaitForResources state transition to complete.
    retEvents = 0;
    tStatus = TIMM_OSAL_EventRetrieve(
        pBaseComInt->pErrorCmdcompleteEvent,
        (STATE_LOADED_EVENT),
        TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
        STATE_TRANSITION_LONG_TIMEOUT);
    OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                    OMX_ErrorUndefined);

EXIT:
    return (eError);

}

/*===============================================================*/
/** @fn OMX_BASE_Error_HandleIdleToLoadedTransition :
 *
 *  Description: To be used internally by base in the function in OMX_BASE_UtilCleanupIfError()
 */
/*===============================================================*/

OMX_ERRORTYPE _OMX_BASE_Error_HandleIdleState(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE            eError  = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                 i = 0, j = 0;
    TIMM_OSAL_U32           retEvents = 0;

    eError = OMX_BASE_SendCommand(hComponent, OMX_CommandStateSet, OMX_StateLoaded, NULL);

    //Send free buffers to complete the Idle transition.
    for( i=0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
        pPort = pBaseComInt->ports[i];
        if( pPort->sPortDef.bEnabled == OMX_TRUE ) {
            for( j=0; j < pPort->sPortDef.nBufferCountActual; j++ ) {
                OMX_BASE_FreeBuffer(hComponent,
                                    i + pBaseComPvt->tDerToBase.nMinStartPortIndex,
                                    pPort->pBufferlist[j]);
            }
        }
    }

    //Wait for Idle state transition to complete.
    retEvents = 0;
    tStatus = TIMM_OSAL_EventRetrieve(
        pBaseComInt->pErrorCmdcompleteEvent,
        (STATE_LOADED_EVENT),
        TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
        STATE_TRANSITION_LONG_TIMEOUT);
    OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                    OMX_ErrorUndefined);

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_UtilCleanupIfError :
 *
 *  @ see omx_base_utils.h
 */
/*===============================================================*/

OMX_ERRORTYPE OMX_BASE_UtilCleanupIfError(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE            eError  = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  i = 0;
    TIMM_OSAL_U32            retEvents = 0;
    OMX_BOOL                 bPortTransitioning = OMX_FALSE;

    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_UtilCleanupIfError");
    /* One race condition still possible where a Loaded to Idle transition command has been sent
        from A9 which did not get serviced before this point*/
    if((pBaseComPvt->tCurState == OMX_StateLoaded) && (pBaseComPvt->tNewState == OMX_StateMax)) {
        for( i=0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
            pPort = pBaseComInt->ports[i];
            if( pPort->bIsTransitionToDisabled || pPort->bIsTransitionToEnabled ) {
                bPortTransitioning = OMX_TRUE;
                break;
            }
        }

        if( bPortTransitioning == OMX_FALSE ) {
            OMX_BASE_Info("Free Handle called in Loaded state, exiting normally");
            goto EXIT;
        }
    }
    pBaseComInt->sAppCallbacks.EventHandler = OMX_BASE_Error_EventHandler;
    pBaseComInt->sAppCallbacks.EmptyBufferDone = OMX_BASE_Error_EmptyBufferDone;
    pBaseComInt->sAppCallbacks.FillBufferDone = OMX_BASE_Error_FillBufferDone;
    /* Another race condition still possible where a WaitForResources to Idle transition command has been sent
        from A9 which did not get serviced before this point*/
    if((pBaseComPvt->tCurState == OMX_StateWaitForResources) && (pBaseComPvt->tNewState == OMX_StateMax)) {
        for( i=0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
            pPort = pBaseComInt->ports[i];
            if( pPort->bIsTransitionToDisabled || pPort->bIsTransitionToEnabled ) {
                bPortTransitioning = OMX_TRUE;
                break;
            }
        }

        if( bPortTransitioning == OMX_FALSE ) {
            OMX_BASE_Trace("Free Handle called in WaitForResources state, attempting a transition to LOADED state");
            eError = _OMX_BASE_Error_HandleWFRState(pComp);
            if( eError != OMX_ErrorNone ) {
                OMX_BASE_Error("Error occured. Cleanup might not be complete");
            } else {
                OMX_BASE_Trace("Cleanup from WaitForResources state is successful");
            }
            goto EXIT;
        }
    }
    //Wait for timeout to let any pending send commands complete.
    retEvents = 0;
    tStatus = TIMM_OSAL_EventRetrieve(
        pBaseComInt->pErrorCmdcompleteEvent,
        (STATE_LOADED_EVENT | STATE_IDLE_EVENT
         | STATE_EXEC_EVENT | STATE_PAUSE_EVENT
         | ERROR_EVENT),
        TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
        STATE_TRANSITION_TIMEOUT);
    if( tStatus != TIMM_OSAL_ERR_NONE && tStatus != TIMM_OSAL_WAR_TIMEOUT ) {
        eError = OMX_ErrorUndefined;
        OMX_BASE_Error("Error occured. Cleanup might not be complete");
    }
    //Clear any port disable/enable events which are pending.
    retEvents = 0;
    tStatus = TIMM_OSAL_EventRetrieve(
        pBaseComInt->pErrorCmdcompleteEvent,
        (PORT_ENABLE_EVENT | PORT_DISABLE_EVENT),
        TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
        TIMM_OSAL_NO_SUSPEND);
    if( tStatus != TIMM_OSAL_ERR_NONE && tStatus != TIMM_OSAL_WAR_TIMEOUT ) {
        eError = OMX_ErrorUndefined;
        OMX_BASE_Error("Error occured. Cleanup might not be complete");
    }

    //This is for the condition if freehandle is being sent while port enable/disable is ongoing.
    for( i=0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
        pPort = pBaseComInt->ports[i];
        if( pPort->bIsTransitionToDisabled || pPort->bIsTransitionToEnabled ) {
            tStatus = TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent,
                                         BUF_FAIL_EVENT, TIMM_OSAL_EVENT_OR);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
            }
            //Wait for port to transition to disable.
            retEvents = 0;
            tStatus = TIMM_OSAL_EventRetrieve(
                pBaseComInt->pErrorCmdcompleteEvent,
                ERROR_EVENT, TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                STATE_TRANSITION_TIMEOUT);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
                OMX_BASE_Error("Error occured. Cleanup might not be complete");
            }
            if( pPort->bIsTransitionToDisabled ) {
                pPort->sPortDef.bEnabled = OMX_TRUE;
                pPort->bIsTransitionToDisabled = OMX_FALSE;
            } else if( pPort->bIsTransitionToEnabled ) {
                pPort->sPortDef.bEnabled = OMX_FALSE;
                pPort->bIsTransitionToDisabled = OMX_FALSE;
            }
        }
    }

    if( pBaseComPvt->tCurState == OMX_StateLoaded && pBaseComPvt->tNewState == OMX_StateMax ) {
        goto EXIT;
    }

    switch( pBaseComPvt->tCurState ) {
        case OMX_StateLoaded :
            OMX_BASE_Info("[handle 0x%x]Current state is loaded", hComponent);
            _OMX_BASE_HANDLE_IDLE_TO_LOADED_TRANSITION();
            break;

        case OMX_StateWaitForResources :
            OMX_BASE_Trace("Current state is WaitForResources and it failed while transitioning to Idle");
            // Since BUFFER FAIL done in cleanup utility happened successfully, all that is required is switch to LOADED state.
            if( pBaseComPvt->tCurState == OMX_StateWaitForResources && pBaseComPvt->tNewState == OMX_StateMax ) {
                eError = _OMX_BASE_Error_HandleWFRState(hComponent);
            } else {
                OMX_BASE_Error("Error occured. Cleanup might not be complete");
            }
            break;

        case OMX_StateIdle :
            OMX_BASE_Info("[handle 0x%x] Current state is Idle", hComponent);
            if( pBaseComPvt->tCurState == OMX_StateIdle && pBaseComPvt->tNewState == OMX_StateMax ) {
                eError = _OMX_BASE_Error_HandleIdleState(hComponent);
            } else { // This is to handle scenerio if there is a crash in Idle to Loaded transition
                _OMX_BASE_HANDLE_IDLE_TO_LOADED_TRANSITION();
            }
            break;

        case OMX_StateExecuting :
            OMX_BASE_Info("[handle 0x%x]Current state is Executing", hComponent);
            // Move the component to Idle State.
            OMX_BASE_SendCommand(hComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
            retEvents = 0;
            tStatus = TIMM_OSAL_EventRetrieve(
                pBaseComInt->pErrorCmdcompleteEvent,
                STATE_IDLE_EVENT,
                TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                STATE_TRANSITION_LONG_TIMEOUT);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
                OMX_BASE_Error("I am here b'cs of TIMEOUT in ER");
            }
            eError = _OMX_BASE_Error_HandleIdleState(hComponent);

            break;

        case OMX_StatePause :
            OMX_BASE_Info("[handle 0x%x]Current state is Pause", hComponent);
            // Move the component to Idle State.
            OMX_BASE_SendCommand(hComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
            retEvents = 0;
            tStatus = TIMM_OSAL_EventRetrieve(
                pBaseComInt->pErrorCmdcompleteEvent,
                (STATE_IDLE_EVENT),
                TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                STATE_TRANSITION_LONG_TIMEOUT);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
                OMX_BASE_Error("I am here b'cs of TIMEOUT in ER");
            }
            eError = _OMX_BASE_Error_HandleIdleState(hComponent);

            break;
    }

EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_Error_EmptyBufferDone :
 *
 *  @ see omx_base_utils.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_Error_EmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                             OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE    eError  = OMX_ErrorNone;

    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_Error_FillBufferDone :
 *
 *  @ see omx_base_utils.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_Error_FillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                            OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE    eError  = OMX_ErrorNone;

    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_Error_EventHandler :
 *
 *  @ see omx_base_utils.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_Error_EventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                          OMX_EVENTTYPE eEvent, OMX_U32 nData1,
                                          OMX_U32 nData2, OMX_PTR pEventData)
{
    OMX_ERRORTYPE            eError  = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    if( eEvent == OMX_EventCmdComplete ) {
        if((OMX_COMMANDTYPE)nData1 == OMX_CommandPortEnable ) {
            tStatus = TIMM_OSAL_EventSet(pBaseComInt->pErrorCmdcompleteEvent,
                                         PORT_ENABLE_EVENT, TIMM_OSAL_EVENT_OR);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
            }
        } else if((OMX_COMMANDTYPE)nData1 == OMX_CommandPortEnable ) {
            tStatus = TIMM_OSAL_EventSet(pBaseComInt->pErrorCmdcompleteEvent,
                                         PORT_DISABLE_EVENT, TIMM_OSAL_EVENT_OR);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
            }
        } else if((OMX_COMMANDTYPE)nData1 == OMX_CommandStateSet ) {
            switch((OMX_STATETYPE) nData2 ) {
                case OMX_StateLoaded :
                    tStatus = TIMM_OSAL_EventSet(pBaseComInt->pErrorCmdcompleteEvent,
                                                 STATE_LOADED_EVENT, TIMM_OSAL_EVENT_OR);
                    if( tStatus != TIMM_OSAL_ERR_NONE ) {
                        eError = OMX_ErrorUndefined;
                    }
                    break;
                case OMX_StateIdle :
                    tStatus = TIMM_OSAL_EventSet(pBaseComInt->pErrorCmdcompleteEvent,
                                                 STATE_IDLE_EVENT, TIMM_OSAL_EVENT_OR);
                    if( tStatus != TIMM_OSAL_ERR_NONE ) {
                        eError = OMX_ErrorUndefined;
                    }
                    break;
                case OMX_StateExecuting :
                    tStatus = TIMM_OSAL_EventSet(pBaseComInt->pErrorCmdcompleteEvent,
                                                 STATE_EXEC_EVENT, TIMM_OSAL_EVENT_OR);
                    if( tStatus != TIMM_OSAL_ERR_NONE ) {
                        eError = OMX_ErrorUndefined;
                    }
                    break;
                case OMX_StatePause :
                    tStatus = TIMM_OSAL_EventSet(pBaseComInt->pErrorCmdcompleteEvent,
                                                 STATE_PAUSE_EVENT, TIMM_OSAL_EVENT_OR);
                    if( tStatus != TIMM_OSAL_ERR_NONE ) {
                        eError = OMX_ErrorUndefined;
                    }
                    break;

            }
        }
    } else if( eEvent == OMX_EventError ) {
        if(((OMX_ERRORTYPE)nData1 == OMX_ErrorPortUnresponsiveDuringDeallocation) ||
           ((OMX_ERRORTYPE)nData1 == OMX_ErrorPortUnresponsiveDuringAllocation)) {
            tStatus = TIMM_OSAL_EventSet(pBaseComInt->pErrorCmdcompleteEvent,
                                         ERROR_EVENT, TIMM_OSAL_EVENT_OR);
        }
    }
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_BASE_HandleFailEvent : This function is called when a
 *                                  BUF_FAIL_EVENT is sent for the event
 *                                  pBufAllocFreeEvent. This indicates that a
 *                                  state transition pending on alloc/free of
 *                                  buffers could not be completed. This
 *                                  indicates a serious error. The expectation
 *                                  is that FreeHandle will be called soon as an
 *                                  error event will be sent after this. Hence,
 *                                  this function cleans up buffers and DIO in
 *                                  preparation of the expected FreeHandle. It
 *                                  does not have a return value since an error
 *                                  has already occured so an error will be
 *                                  returned to client anyway.
 *
 *  @param hComponent : Component handle.
 *  @param eCmd       : Indicates the type of command that was pending when the
 *                      BUF_FAIL_EVENT was generated.
 *  @param nPortIndex : Port Index - valid only if command type is port enable
 *                      or port disable.
 */
/*===============================================================*/
void _OMX_BASE_HandleFailEvent(OMX_HANDLETYPE hComponent, OMX_COMMANDTYPE eCmd,
                               OMX_U32 nPortIndex)
{
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt =
        (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt =
        (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    OMX_BASE_PORTTYPE     *pPort = NULL;
    TIMM_OSAL_ERRORTYPE    retval = TIMM_OSAL_ERR_NONE;
    OMX_U32                i = 0;
    TIMM_OSAL_U32          retEvents = 0;
    OMX_ERRORTYPE          eError  = OMX_ErrorNone;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_HandleFailEvent");

    switch( eCmd ) {
        case OMX_CommandStateSet :
            TIMM_OSAL_MutexObtain(pBaseComInt->pNewStateMutex,
                                  TIMM_OSAL_SUSPEND);
            if(((pBaseComPvt->tCurState == OMX_StateLoaded ||
                 pBaseComPvt->tCurState == OMX_StateWaitForResources) &&
                pBaseComPvt->tNewState == OMX_StateIdle) ||
               (pBaseComPvt->tCurState == OMX_StateIdle &&
                pBaseComPvt->tNewState == OMX_StateLoaded)) {
                /*Failure in L --> I transition. First step is to tell the derived component
                          to revert back to loaded state*/
                if(((pBaseComPvt->tCurState == OMX_StateLoaded) ||
                    (pBaseComPvt->tCurState == OMX_StateWaitForResources)) &&
                   (pBaseComPvt->tNewState == OMX_StateIdle)) {
                    /*Force setting states*/
                    pBaseComPvt->tCurState = OMX_StateIdle;
                    pBaseComPvt->tNewState = OMX_StateLoaded;
                    /*Return error values dont matter here since this is cleanup*/
                    eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                     OMX_CommandStateSet, OMX_StateLoaded, NULL);
                    if( eError == OMX_ErrorNone ) {
                        TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent,
                                                OMX_BASE_CmdStateSet, TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                                                TIMM_OSAL_SUSPEND);
                    }
                }

                /*If DIO for any port is open close those*/
                for( i = 0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
                    pPort = pBaseComInt->ports[i];

                    retval = TIMM_OSAL_EventRetrieve(pPort->pBufAllocFreeEvent,
                                                     BUF_FREE_EVENT, TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                                                     500);
                    if( retval == TIMM_OSAL_WAR_TIMEOUT ) {
                        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_SYSTEM, "Event retrieve timed out on BUF_FREE_EVENT within function:%s", __FUNCTION__);
                    }
                    if( pPort != NULL ) {
                        if( pPort->hDIO != NULL ) {
                            OMX_BASE_DIO_Close(hComponent,
                                               (i + pBaseComPvt->tDerToBase.nMinStartPortIndex));

                            OMX_BASE_DIO_Deinit(hComponent,
                                                (i + pBaseComPvt->tDerToBase.nMinStartPortIndex));
                        }
                    }
                }

                /*Force setting states*/
                pBaseComPvt->tCurState = OMX_StateLoaded;
                pBaseComPvt->tNewState = OMX_StateMax;
            }
            TIMM_OSAL_MutexRelease(pBaseComInt->pNewStateMutex);
            break;
        case OMX_CommandPortEnable :
            /*Tell derived comp to move back to disabled state*/
            pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                    OMX_CommandPortDisable, nPortIndex, NULL);

            TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent,
                                    OMX_BASE_CmdPortDisable, TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                                    TIMM_OSAL_SUSPEND);
            if( nPortIndex == OMX_ALL ) {
                for( i = 0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
                    pPort = pBaseComInt->ports[i];
                    pPort->bIsTransitionToEnabled = OMX_FALSE;
                }
            } else {
                pPort = pBaseComInt->ports[
                    nPortIndex - pBaseComPvt->tDerToBase.nMinStartPortIndex];
                pPort->bIsTransitionToEnabled = OMX_FALSE;
            }
        /*NO break to OMX_CommandPortEnable case :::Intention is to have a fall through logic to the port disable*/
        case OMX_CommandPortDisable :
            /*Close DIO on the relevant ports for both enable as well as disable
            commands*/
            if( nPortIndex == OMX_ALL ) {
                for( i = 0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
                    pPort = pBaseComInt->ports[i];
                    retval = TIMM_OSAL_EventRetrieve(pPort->pBufAllocFreeEvent,
                                                     BUF_FREE_EVENT, TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                                                     500);
                    if( retval == TIMM_OSAL_WAR_TIMEOUT ) {
                        TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_SYSTEM, "Event retrieve timed out on BUF_FREE_EVENT within function:%s", __FUNCTION__);
                    }
                    if( pPort != NULL ) {
                        if( pPort->hDIO != NULL ) {
                            OMX_BASE_DIO_Close(hComponent, (i +
                                                            pBaseComPvt->tDerToBase.nMinStartPortIndex));

                            OMX_BASE_DIO_Deinit(hComponent, (i +
                                                             pBaseComPvt->tDerToBase.nMinStartPortIndex));
                        }
                    }
                }
            } else {
                pPort = pBaseComInt->ports[
                    nPortIndex - pBaseComPvt->tDerToBase.nMinStartPortIndex];
                retval = TIMM_OSAL_EventRetrieve(pPort->pBufAllocFreeEvent,
                                                 BUF_FREE_EVENT, TIMM_OSAL_EVENT_OR_CONSUME, &retEvents,
                                                 500);
                if( retval == TIMM_OSAL_WAR_TIMEOUT ) {
                    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_SYSTEM, "Event retrieve timed out on BUF_FREE_EVENT within function:%s", __FUNCTION__);
                }
                if( pPort != NULL ) {
                    if( pPort->hDIO != NULL ) {
                        OMX_BASE_DIO_Close(hComponent, nPortIndex);
                        OMX_BASE_DIO_Deinit(hComponent, nPortIndex);
                    }
                }
            }
            break;
    }

    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "_OMX_BASE_HandleFailEvent", 0);
    return;
}

OMX_U32 OMX_BASE_SetTaskPriority(OMX_HANDLETYPE hComponent, OMX_U32 uPriority)
{
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    OMX_U32                  tStatus = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetTaskPriority");
    /* TIMM_OSAL_setTaskPriority retuns the Previous Task Priority */
    //tStatus = TIMM_OSAL_setTaskPriority(pBaseComInt->pThreadId, uPriority);
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetTaskPriority", 0);
    return (tStatus);
}

