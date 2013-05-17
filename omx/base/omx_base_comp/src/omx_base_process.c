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

#include <OMX_Core.h>
#include <OMX_TI_Core.h>
#include <OMX_Component.h>
#include <omx_base.h>
#include <omx_base_internal.h>
#include <timm_osal_interfaces.h>
#include <omx_base_utils.h>
#include <omx_base_dio_plugin.h>

/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*----------         function prototypes      ------------------- */
/*----------         data declarations        ------------------- */
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
static OMX_ERRORTYPE OMX_BASE_PROCESS_DataEvent(OMX_HANDLETYPE hComponent);

static OMX_ERRORTYPE OMX_BASE_PROCESS_CmdEvent(OMX_HANDLETYPE hComponent,
                                               OMX_COMMANDTYPE Cmd,
                                               OMX_U32 nParam,
                                               OMX_PTR pCmdData);

extern void _OMX_BASE_HandleFailEvent(OMX_HANDLETYPE hComponent, OMX_COMMANDTYPE eCmd, OMX_U32 nPortIndex);

/*===============================================================*/
/** @fn OMX_BASE_CompThreadEntry :
 *   see omx_base_internal.h
 */
/*===============================================================*/
void OMX_BASE_CompThreadEntry(void *arg)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)arg;
    TIMM_OSAL_U32            retrievedEvents = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_CompThreadEntry");
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    while( 1 ) {
        /*  wait for Any of the event/s to process  */
        tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pTriggerEvent,
                                          (CMDEVENT | DATAEVENT | ENDEVENT),
                                          TIMM_OSAL_EVENT_OR_CONSUME,
                                          &retrievedEvents, TIMM_OSAL_SUSPEND);
        OMX_BASE_ASSERT(tStatus == TIMM_OSAL_ERR_NONE,
                        OMX_ErrorInsufficientResources);
        /* terminate the process when it acquires an ENDEVENT */
        if( retrievedEvents & ENDEVENT ) {
            break;
        }
        /* Process Event that has retrieved */
        if( retrievedEvents & CMDEVENT ) {
            while( TIMM_OSAL_IsPipeReady(pBaseComInt->pCmdPipe) ==
                   TIMM_OSAL_ERR_NONE ) {
                eError = OMX_BASE_PROCESS_Events(pComp, retrievedEvents);
                /*Callback for error will be sent in the above function*/
                eError = OMX_ErrorNone;
            }

            retrievedEvents &= ~CMDEVENT;
        }
        if( retrievedEvents & DATAEVENT ) {
            eError = OMX_BASE_PROCESS_Events(pComp, retrievedEvents);
            /*Callback for error will be sent in the above function*/
            eError = OMX_ErrorNone;
        }
    }

EXIT:
    if( OMX_ErrorNone != eError ) {
        pBaseComInt->sAppCallbacks.EventHandler((OMX_HANDLETYPE)pComp,
                                                pComp->pApplicationPrivate,
                                                OMX_EventError, eError,
                                                0, NULL);
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_CompThreadEntry", 0);
}

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_TriggerEvent :
 *   see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_PROCESS_TriggerEvent(OMX_HANDLETYPE hComponent,
                                            OMX_U32 EventToSet)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  i, nStartPort, nCount = 0;
    OMX_BOOL                 bNotify = OMX_TRUE;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_TriggerEvent");
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    nStartPort = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    if( EventToSet == DATAEVENT ) {
        /*This var mey be accessed by multiple threads but it need not be mutex
        protected*/
        if( pBaseComInt->bForceNotifyOnce ) {
            pBaseComInt->bForceNotifyOnce = OMX_FALSE;
            bNotify = OMX_TRUE;
            goto EXIT;
        }

        for( i = 0; i < (pBaseComPvt->tDerToBase.nNumPorts); i++ ) {
            pPort = pBaseComInt->ports[i];
            /*If port is disabled then move on*/
            if( !pPort->sPortDef.bEnabled ) {
                continue;
            }
            /*If EOS has been recd. on any one port then always send notification*/
            if( pPort->sPortDef.eDir == OMX_DirInput && pPort->bEosRecd ) {
                bNotify = OMX_TRUE;
                goto EXIT;
            }
        }

        if( pBaseComPvt->tDerToBase.bNotifyForAnyPort == OMX_TRUE ) {
            bNotify = OMX_FALSE;

            for( i = 0; i < (pBaseComPvt->tDerToBase.nNumPorts); i++ ) {
                pPort = pBaseComInt->ports[i];
                /*If port is disabled then move on*/
                if( !pPort->sPortDef.bEnabled ||
                     pPort->bIsTransitionToEnabled) {
                    continue;
                }
                eError = pBaseComPvt->fpDioGetCount(hComponent, nStartPort + i,
                                                    &nCount);
                OMX_BASE_ASSERT((eError == OMX_ErrorNone || eError ==
                                 (OMX_ERRORTYPE)OMX_TI_WarningEosReceived), eError);
                /*Resetting to ErrorNone in case EOS warning is recd.*/
                eError = OMX_ErrorNone;
                if((nCount >=
                    pBaseComPvt->tDerToBase.pPortProperties[i]->nWatermark)) {
                    bNotify = OMX_TRUE;
                    break;
                }
            }
        } else {
            for( i = 0; i < (pBaseComPvt->tDerToBase.nNumPorts); i++ ) {
                pPort = pBaseComInt->ports[i];
                /*If port is disabled then move on*/
                if( !pPort->sPortDef.bEnabled ||
                     pPort->bIsTransitionToEnabled) {
                    continue;
                }
                eError = pBaseComPvt->fpDioGetCount(hComponent, nStartPort + i,
                                                    &nCount);
                OMX_BASE_ASSERT((eError == OMX_ErrorNone || eError ==
                                 (OMX_ERRORTYPE)OMX_TI_WarningEosReceived), eError);
                /*Resetting to ErrorNone in case EOS warning is recd.*/
                eError = OMX_ErrorNone;
                if((nCount <
                    pBaseComPvt->tDerToBase.pPortProperties[i]->nWatermark)) {
                    bNotify = OMX_FALSE;
                    break;
                }
            }
        }
    }
EXIT:
    if( bNotify == OMX_TRUE && eError == OMX_ErrorNone ) {
        tStatus = TIMM_OSAL_EventSet(pBaseComInt->pTriggerEvent, EventToSet,
                                     TIMM_OSAL_EVENT_OR);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_TriggerEvent", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_DataNotify :
 *   see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_PROCESS_DataNotify(OMX_HANDLETYPE hComponent)
{
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_DataNotify");
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    pBaseComInt->bForceNotifyOnce = OMX_FALSE;
    return (OMX_BASE_PROCESS_TriggerEvent(hComponent, DATAEVENT));
}

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_PROCESS_Events(OMX_HANDLETYPE hComponent,
                                      OMX_U32 retEvent)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    OMX_ERRORTYPE            eErrorAux = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_CMDPARAMS       sCmdParams;
    TIMM_OSAL_U32            actualSize = 0;
    OMX_BOOL                 bHandleFailEvent = OMX_FALSE;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_Events");

    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    if( retEvent & CMDEVENT ) {
        tStatus = TIMM_OSAL_MutexObtain(pBaseComInt->pCmdPipeMutex,
                                        TIMM_OSAL_SUSPEND);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
        /* process command event */
        tStatus = TIMM_OSAL_IsPipeReady(pBaseComInt->pCmdPipe);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            /*No command in pipe - return*/
            tStatus = TIMM_OSAL_MutexRelease(pBaseComInt->pCmdPipeMutex);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
            goto EXIT;
        }
        tStatus = TIMM_OSAL_ReadFromPipe(pBaseComInt->pCmdPipe, &sCmdParams,
                                         sizeof(OMX_BASE_CMDPARAMS), &actualSize, TIMM_OSAL_NO_SUSPEND);
        if( TIMM_OSAL_ERR_NONE != tStatus ) {
            eError = OMX_ErrorUndefined;
        }
        tStatus = TIMM_OSAL_MutexRelease(pBaseComInt->pCmdPipeMutex);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

        eError = OMX_BASE_PROCESS_CmdEvent(hComponent, sCmdParams.eCmd,
                                           sCmdParams.unParam, sCmdParams.pCmdData);
        if( OMX_ErrorNone != eError ) {
            bHandleFailEvent = OMX_TRUE;
            goto EXIT;
        }
    } else if( retEvent & DATAEVENT ) {
        /* process data Event */
        eError = OMX_BASE_PROCESS_DataEvent(hComponent);
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
    }
EXIT:
    if( OMX_ErrorNone != eError ) {
        if(eError != OMX_ErrorDynamicResourcesUnavailable) {
            eErrorAux = pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                                pComp->pApplicationPrivate,
                                                                OMX_EventError, eError,
                                                                OMX_StateInvalid, NULL);
        } else {
            eErrorAux = pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                                pComp->pApplicationPrivate,
                                                                OMX_EventError, eError,
                                                                OMX_StateLoaded, NULL);
        }
        /*Component can do nothing if callback returns error*/
        //Just calling _OMX_BASE_HandleFailEvent for SetState since it was the
        //the intention. When Allocation fails while Dynamic Resources Allocation
        //we are experiencing hang waiting forever for PortDisable event completion
        //after attempted an unsuccessful PortEnable.

        //_OMX_BASE_HandleFailEvent will be called only in case of a real error
        if( bHandleFailEvent && (eError != OMX_ErrorDynamicResourcesUnavailable) ) {
            _OMX_BASE_HandleFailEvent(hComponent, sCmdParams.eCmd, sCmdParams.unParam);
        }

    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_Events", eError);
    if( (OMX_ErrorNone != eError) && (eError != OMX_ErrorDynamicResourcesUnavailable) ) {
        return (eError); //return actual error if any
    }
    //in previous implementation if there were any error in the eError variable,
    //it would have been overwritten by a call to the pBaseComInt->sAppCallbacks.EventHandler().
    //If there isn't any error, the default value of the eErrorAux is OMX_ErrorNone
    //and we return no error.
    return (eErrorAux);
}

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_CmdEvent :
 *      This function process the command events and notifies
 *      the derived component to further process if any.
 *
 *  @param hComponent  : handle of the component
 *  @param Cmd         : Command to process
 *  @param nParam      : param associated with the command
 *  @param pCmdData    : pointer to command data
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_BASE_PROCESS_CmdEvent(OMX_HANDLETYPE hComponent,
                                               OMX_COMMANDTYPE Cmd,
                                               OMX_U32 nParam,
                                               OMX_PTR pCmdData)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_U32                  i, nPorts, nStartPortNum;
    TIMM_OSAL_U32            retEvents = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_CmdEvent");
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nPorts =  pBaseComPvt->tDerToBase.nNumPorts;
    nStartPortNum = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    switch( Cmd ) {
        case OMX_CommandStateSet :
            eError = _OMX_BASE_HandleStateTransition(hComponent, nParam);
            /* For experiment purpose: adding delay here makes state transition to take more time
                        TIMM_OSAL_SleepTask(TRACE_REG16(0x8047FC00));
            */
            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
            break;

        case OMX_CommandPortDisable :
            /* Notify to Derived Component here so that derived component
               receives correct param - ALL or specific port no. */
            eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                             OMX_CommandPortDisable, nParam, pCmdData);
            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);

            tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent,
                                              OMX_BASE_CmdPortDisable, TIMM_OSAL_EVENT_OR_CONSUME,
                                              &retEvents, TIMM_OSAL_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                            OMX_ErrorInsufficientResources);
            if( nParam == OMX_ALL ) {
                for( i = nStartPortNum; i < nPorts; i++ ) {
                    eError = _OMX_BASE_DisablePort(hComponent, i);
                    OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                }
            } else {
                eError = _OMX_BASE_DisablePort(hComponent, nParam);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
            }
            break;

        case OMX_CommandPortEnable :
            /* Notify to Derived Component here so that derived component
               receives correct param - ALL or specific port no. */
            eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                             OMX_CommandPortEnable, nParam, pCmdData);
            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
            tStatus = TIMM_OSAL_EventRetrieve(pBaseComInt->pCmdCompleteEvent,
                                              OMX_BASE_CmdPortEnable, TIMM_OSAL_EVENT_OR_CONSUME,
                                              &retEvents, TIMM_OSAL_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                            OMX_ErrorInsufficientResources);
            if( nParam == OMX_ALL ) {
                for( i = nStartPortNum; i < nPorts; i++ ) {
                    eError = _OMX_BASE_EnablePort(hComponent, i);
                    OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                }
            } else {
                eError = _OMX_BASE_EnablePort(hComponent, nParam);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
            }
            break;

        case OMX_CommandFlush :
            if( pBaseComPvt->tCurState == OMX_StateLoaded ||
                pBaseComPvt->tCurState == OMX_StateWaitForResources ) {
                /*Nothing to be done in loaded state, just exit*/
                TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE,
                                   "Flush command called in loaded state - nothing to be done");
            } else if((nParam != OMX_ALL) && (pBaseComInt->ports
                                              [nParam - nStartPortNum]->sPortDef.bEnabled == OMX_FALSE)) {
                /*Nothing to be done for disabled port, just exit*/
                TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE,
                                   "Flush command called on disabled port - nothing to be done");
            } else {
                /* Notify to Derived Component here so that derived component
                   receives correct param - ALL or specific port no. */
                eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent,
                                                                 OMX_CommandFlush, nParam, pCmdData);
                OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                tStatus = TIMM_OSAL_EventRetrieve(
                    pBaseComInt->pCmdCompleteEvent,
                    OMX_BASE_CmdFlush, TIMM_OSAL_EVENT_OR_CONSUME,
                    &retEvents, TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                                OMX_ErrorInsufficientResources);
                if( nParam == OMX_ALL ) {
                    for( i = nStartPortNum; i < nPorts; i++ ) {
                        eError = _OMX_BASE_FlushBuffers(hComponent, i);
                        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                    }
                } else {
                    eError = _OMX_BASE_FlushBuffers(hComponent, nParam);
                    OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
                }
            }
            break;

        case OMX_CommandMarkBuffer :
            eError = pBaseComPvt->tDerToBase.fpCommandNotify(hComponent, Cmd,
                                                             nParam, pCmdData);
            OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
            break;

        default :
            OMX_BASE_Trace(" unknown command received ");
            break;
    }

    eError = _OMX_BASE_EventNotifyToClient(hComponent, Cmd, nParam, pCmdData);
    OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_CmdEvent", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_DataEvent :
 *      This function process the data events and notifies
 *      derived component to process data(buffers)
 *
 * @param hComponent : handle of the component
 */
/*===============================================================*/
static OMX_ERRORTYPE OMX_BASE_PROCESS_DataEvent(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE           eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE      *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = NULL;

    //OMX_BASE_INTERNALTYPE* pBaseComInt = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_DataEvent");
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    //pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    eError = pBaseComPvt->tDerToBase.fpDataNotify(hComponent);
    OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_PROCESS_DataEvent", eError);
    return (eError);
}

