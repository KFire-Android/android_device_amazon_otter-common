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
#include <omx_base.h>
#include <omx_base_internal.h>
#include <timm_osal_interfaces.h>
#include <omx_base_utils.h>

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

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_CB_ReturnEventNotify(OMX_HANDLETYPE hComponent,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 nData1, OMX_U32 nData2,
                                            OMX_PTR pEventData)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    TIMM_OSAL_U32            nActualSize = 0;
    OMX_PTR                  pCmdData = NULL;

    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    switch( eEvent ) {
        case OMX_EventCmdComplete :
            if((OMX_COMMANDTYPE)nData1 == OMX_CommandStateSet ) {
                TIMM_OSAL_EventSet(pBaseComInt->pCmdCompleteEvent,
                                   OMX_BASE_CmdStateSet, TIMM_OSAL_EVENT_OR);
            } else if((OMX_COMMANDTYPE)nData1 == OMX_CommandPortEnable ) {
                TIMM_OSAL_EventSet(pBaseComInt->pCmdCompleteEvent,
                                   OMX_BASE_CmdPortEnable, TIMM_OSAL_EVENT_OR);
            } else if((OMX_COMMANDTYPE)nData1 == OMX_CommandPortDisable ) {
                TIMM_OSAL_EventSet(pBaseComInt->pCmdCompleteEvent,
                                   OMX_BASE_CmdPortDisable, TIMM_OSAL_EVENT_OR);
            } else if((OMX_COMMANDTYPE)nData1 == OMX_CommandFlush ) {
                TIMM_OSAL_EventSet(pBaseComInt->pCmdCompleteEvent,
                                   OMX_BASE_CmdFlush, TIMM_OSAL_EVENT_OR);
            } else if((OMX_COMMANDTYPE)nData1 == OMX_CommandMarkBuffer ) {
                /*The derived component has completed the mark buffer command so
                the memory allocated for pCmdData is no longer needed - it can
                be freed up*/
                tStatus = TIMM_OSAL_ReadFromPipe(pBaseComInt->pCmdDataPipe,
                                                 &pCmdData, sizeof(OMX_PTR), &nActualSize,
                                                 TIMM_OSAL_NO_SUSPEND);
                /*Read from pipe should return immediately with valid value - if
                it does not, return error callback to the client*/
                if( TIMM_OSAL_ERR_NONE != tStatus ) {
                    OMX_BASE_Error("pCmdData not available to freed up at mark \
                        buffer command completion. Returning error event.");
                    pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                            pComp->pApplicationPrivate, OMX_EventError,
                                                            (OMX_U32)OMX_ErrorInsufficientResources, 0, NULL);
                    goto EXIT;
                }
                TIMM_OSAL_Free(pCmdData);
                /*For mark buffer cmd complete, directly send callback to the
                client. This callback is not being handled in
                _OMX_BASE_EventNotifyToClient since this can happen much later
                than the time when the mark comand was received*/
                pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                        pComp->pApplicationPrivate, eEvent, nData1, nData2,
                                                        pEventData);
            }
            break;

        case OMX_EventError :
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;

        case OMX_EventMark :
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;

        case OMX_EventPortSettingsChanged :
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;

        case OMX_EventBufferFlag :
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;

        case OMX_EventResourcesAcquired :
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;

        case OMX_EventComponentResumed :
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;

        case OMX_EventDynamicResourcesAvailable :
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;

        case OMX_EventPortFormatDetected :
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;

        default :
            OMX_BASE_Trace("Unknown event received - still making callback");
            pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                    pComp->pApplicationPrivate, eEvent, nData1, nData2, pEventData);
            break;
    }

EXIT:
    return (eError);
}

