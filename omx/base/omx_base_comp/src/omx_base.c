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
#include <OMX_TI_Common.h>
#include <OMX_TI_Core.h>
#include <OMX_TI_Index.h>
#include <OMX_TI_IVCommon.h>

#define HAL_NV12_PADDED_PIXEL_FORMAT (OMX_TI_COLOR_FormatYUV420PackedSemiPlanar - OMX_COLOR_FormatVendorStartUnused)

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
#define OMX_BASE_TASKDEL_TRIES 1000
#define OMX_BASE_TASKDEL_SLEEP 2
#define OMX_BASE_MAXCMDS 10

/*----------          function prototypes      ------------------- */
/*===============================================================*/
/** @fn OMX_BASE_ComponentInit :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone, eTmpError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_ComponentInit");

    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_REQUIRE(pComp->pComponentPrivate != NULL, OMX_ErrorBadParameter);
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    /* populate component specific function pointers  */
    pComp->GetComponentVersion      =  OMX_BASE_GetComponentVersion;
    pComp->SendCommand              =  OMX_BASE_SendCommand;
    pComp->GetParameter             =  OMX_BASE_GetParameter;
    pComp->SetParameter             =  OMX_BASE_SetParameter;
    pComp->GetConfig                =  OMX_BASE_GetConfig;
    pComp->SetConfig                =  OMX_BASE_SetConfig;
    pComp->GetExtensionIndex        =  OMX_BASE_GetExtensionIndex;
    pComp->GetState                 =  OMX_BASE_GetState;
    pComp->ComponentTunnelRequest   =  OMX_BASE_ComponentTunnelRequest;
    pComp->UseBuffer                =  OMX_BASE_UseBuffer;
    pComp->AllocateBuffer           =  OMX_BASE_AllocateBuffer;
    pComp->FreeBuffer               =  OMX_BASE_FreeBuffer;
    pComp->EmptyThisBuffer          =  OMX_BASE_EmptyThisBuffer;
    pComp->FillThisBuffer           =  OMX_BASE_FillThisBuffer;
    pComp->SetCallbacks             =  OMX_BASE_SetCallbacks;
    pComp->ComponentDeInit          =  OMX_BASE_ComponentDeinit;
    pComp->UseEGLImage              =  OMX_BASE_UseEGLImage;
    pComp->ComponentRoleEnum        =  OMX_BASE_ComponentRoleEnum;

    /* create a mutex to handle race conditions */
    tStatus = TIMM_OSAL_MutexCreate(&(pBaseComPvt->pMutex));
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                    OMX_ErrorInsufficientResources);

    /* Allocate internal area for Base component */
    OMX_BASE_MALLOC_STRUCT(pBaseComPvt->hOMXBaseInt, OMX_BASE_INTERNALTYPE,
                           pBaseComPvt->tDerToBase.hDefaultHeap);
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    TIMM_OSAL_Memset(pBaseComInt, 0, sizeof(OMX_BASE_INTERNALTYPE));

    /*Create the new state mutex*/
    tStatus = TIMM_OSAL_MutexCreate(&(pBaseComInt->pNewStateMutex));
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                    OMX_ErrorInsufficientResources);

    /* create a fixed size command pipe to queueup commands */
    tStatus = TIMM_OSAL_CreatePipe(&(pBaseComInt->pCmdPipe),
                                   OMX_BASE_MAXCMDS * sizeof(OMX_BASE_CMDPARAMS),
                                   sizeof(OMX_BASE_CMDPARAMS), 1);
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                    OMX_ErrorInsufficientResources);


    /* create a fixed size pipe to queueup command data pointers */
    tStatus = TIMM_OSAL_CreatePipe(&(pBaseComInt->pCmdDataPipe),
                                   OMX_BASE_MAXCMDS * sizeof(OMX_PTR),
                                   sizeof(OMX_PTR), 1);
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                    OMX_ErrorInsufficientResources);

    /*create an Event for Command completion to be set by Dervived Component  */
    tStatus = TIMM_OSAL_EventCreate(&(pBaseComInt->pCmdCompleteEvent));
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                    OMX_ErrorInsufficientResources);

    /*create an Event for state transition notifications for  complete tear down of compoenent */
    tStatus = TIMM_OSAL_EventCreate(&(pBaseComInt->pErrorCmdcompleteEvent));
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                    OMX_ErrorInsufficientResources);


    /*Create mutex for cmd pipe*/
    tStatus = TIMM_OSAL_MutexCreate(&(pBaseComInt->pCmdPipeMutex));
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                    OMX_ErrorInsufficientResources);

    /* Initialize ports */
    eError = _OMX_BASE_InitializePorts(hComponent);
    OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);

    pBaseComInt->fpInvokeProcessFunction = OMX_BASE_PROCESS_Events;

    if( OMX_BASE_CONTEXT_ACTIVE == pBaseComPvt->tDerToBase.ctxType ) {
        tStatus = TIMM_OSAL_EventCreate(&(pBaseComInt->pTriggerEvent));
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                        OMX_ErrorInsufficientResources);

        /*If stack size is 0, return error*/
        OMX_BASE_REQUIRE(pBaseComPvt->tDerToBase.nStackSize != 0,
                         OMX_ErrorBadParameter);

        tStatus = TIMM_OSAL_CreateTask(&pBaseComInt->pThreadId,
                                       OMX_BASE_CompThreadEntry, 0,
                                       (void *)hComponent,
                                       pBaseComPvt->tDerToBase.nStackSize,
                                       pBaseComPvt->tDerToBase.nPrioirty,
                                       (TIMM_OSAL_S8 *)pBaseComPvt->tDerToBase.cTaskName);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                        OMX_ErrorInsufficientResources);

        pBaseComInt->fpInvokeProcessFunction = OMX_BASE_PROCESS_TriggerEvent;
    }
    /* Set hooks from Derived to Base communicattion */
    pBaseComPvt->fpReturnEventNotify    =  OMX_BASE_CB_ReturnEventNotify;
    pBaseComPvt->fpDioGetCount          =  OMX_BASE_DIO_GetCount;
    pBaseComPvt->fpDioDequeue           =  OMX_BASE_DIO_Dequeue;
    pBaseComPvt->fpDioSend              =  OMX_BASE_DIO_Send;
    pBaseComPvt->fpDioCancel            =  OMX_BASE_DIO_Cancel;
    pBaseComPvt->fpDioDup               =  OMX_BASE_DIO_Dup;
    pBaseComPvt->fpDioControl           =  OMX_BASE_DIO_Control;
    pBaseComPvt->fpDioUtil              =  OMX_BASE_DIO_Util;

    /* Component is initialized successfully, set the
     *  component to loaded state */
    pBaseComPvt->tCurState =  OMX_StateLoaded;
    pBaseComPvt->tNewState =  OMX_StateMax;

EXIT:
    /* incase of an error, deinitialize the component */
    if((OMX_ErrorNone != eError) && (pComp != NULL)) {
        eTmpError = eError;
        eError = pComp->ComponentDeInit(hComponent);
        eError = eTmpError;
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_ComponentInit", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_SetCallbacks :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SetCallbacks(OMX_HANDLETYPE hComponent,
                                    OMX_CALLBACKTYPE *pCallbacks,
                                    OMX_PTR pAppData)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_U32                  i = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetCallbacks");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pCallbacks != NULL), OMX_ErrorBadParameter);
    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    pComp->pApplicationPrivate = pAppData;
    pBaseComInt->sAppCallbacks = *pCallbacks;

    /*WORKAROUND*/
    /*Initializing port supplier preferences here - this is a workaround
    since at the time of calling initialize port, we don't know the direction
    of the port*/

    for( i = 0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
        switch( pBaseComPvt->tDerToBase.pPortProperties[i]->eBufAllocPref ) {
            case OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_DEFAULT :
            case OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_NEVER :
                pBaseComInt->ports[i]->eSupplierSetting =
                    OMX_BufferSupplyUnspecified;
                break;
            case OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_PREFERRED :
            case OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_MUST :
                if( pBaseComInt->ports[i]->sPortDef.eDir == OMX_DirOutput ) {
                    pBaseComInt->ports[i]->eSupplierSetting =
                        OMX_BufferSupplyOutput;
                } else {
                    pBaseComInt->ports[i]->eSupplierSetting =
                        OMX_BufferSupplyInput;
                }
                break;
            default :
                eError = OMX_ErrorUndefined;
                break;
        }
    }

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetCallbacks", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_GetComponentVersion :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetComponentVersion(OMX_HANDLETYPE hComponent,
                                           OMX_STRING pComponentName,
                                           OMX_VERSIONTYPE *pComponentVersion,
                                           OMX_VERSIONTYPE *pSpecVersion,
                                           OMX_UUIDTYPE *pComponentUUID)
{
    OMX_ERRORTYPE           eError  = OMX_ErrorNone;
    OMX_COMPONENTTYPE      *pComp = NULL;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetComponentVersion");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pComponentName != NULL) &&
                     (pComponentVersion != NULL) &&
                     (pSpecVersion != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);

    TIMM_OSAL_Memcpy(pComponentName, pBaseComPvt->tDerToBase.cComponentName,
                     OMX_MAX_STRINGNAME_SIZE);

    *pComponentVersion = pBaseComPvt->tDerToBase.nComponentVersion;
    *pSpecVersion      = pComp->nVersion;



EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetComponentVersion", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_GetState :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetState(OMX_HANDLETYPE hComponent,
                                OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE           eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE      *pComp = NULL;
    OMX_BASE_PRIVATETYPE   *pBaseComPvt = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetState");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pState != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;

    *pState = pBaseComPvt->tCurState;

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetState", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_ComponentDeInit :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_ComponentDeinit(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE            eError  = OMX_ErrorNone, eTmpError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  i = 0, nTries = 0;
    TIMM_OSAL_U32            nActualSize = 0;
    TIMM_OSAL_U32            nCount = 0;
    OMX_PTR                  pData = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_ComponentDeinit");
    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);
    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    OMX_BASE_REQUIRE(pBaseComPvt != NULL, OMX_ErrorBadParameter);
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    /*If pBaseComInt is NULL, just free up the pointers in private structure
    and return*/
    if( pBaseComInt == NULL ) {
        if( pBaseComPvt->pPortdefs ) {
            TIMM_OSAL_Free(pBaseComPvt->pPortdefs);
            pBaseComPvt->pPortdefs = NULL;
        }
        if( pBaseComPvt->pMutex ) {
            tStatus = TIMM_OSAL_MutexDelete(pBaseComPvt->pMutex);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
            }
        }
        goto EXIT;
    }

    TIMM_OSAL_MutexObtain(pBaseComInt->pNewStateMutex, TIMM_OSAL_SUSPEND);

    /*If component transitioned to invalid state suddenly then dio close and
      deinit might have not been called - in that case calle them now */
    if( pBaseComPvt->tCurState != OMX_StateLoaded ) {
        for( i=0; i < pBaseComPvt->tDerToBase.nNumPorts; i++ ) {
            pPort = pBaseComInt->ports[i];
            if( pPort != NULL ) {
                if( pPort->hDIO != NULL ) {
                    eTmpError = OMX_BASE_DIO_Close(hComponent,
                                                   (i + pBaseComPvt->tDerToBase.nMinStartPortIndex));
                    if( eTmpError != OMX_ErrorNone ) {
                        eError = eTmpError;
                    }
                    eTmpError = OMX_BASE_DIO_Deinit(hComponent,
                                                    (i + pBaseComPvt->tDerToBase.nMinStartPortIndex));
                    if( eTmpError != OMX_ErrorNone ) {
                        eError = eTmpError;
                    }
                }
            }
        }
    }

    /* deinitialize ports and freeup the memory */
    eTmpError = _OMX_BASE_DeinitializePorts(hComponent);
    if( eTmpError != OMX_ErrorNone ) {
        eError = eTmpError;
    }
    /* destroy thread incase of an Active context */
    if( OMX_BASE_CONTEXT_ACTIVE == pBaseComPvt->tDerToBase.ctxType ) {
        /* set an ENDEVENT before destroying thread  */
        pBaseComInt->fpInvokeProcessFunction(hComponent, ENDEVENT);
        tStatus = TIMM_OSAL_DeleteTask(pBaseComInt->pThreadId);

        while( tStatus == TIMM_OSAL_ERR_NOT_READY &&
               nTries < OMX_BASE_TASKDEL_TRIES ) {
            //Wait for some time and try again
            TIMM_OSAL_SleepTask(OMX_BASE_TASKDEL_SLEEP);
            nTries++;
            tStatus = TIMM_OSAL_DeleteTask(pBaseComInt->pThreadId);
        }

        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorTimeout;
            TIMM_OSAL_Error("Error while deleting task");
        }
        if( pBaseComInt->pTriggerEvent ) {
            tStatus = TIMM_OSAL_EventDelete(pBaseComInt->pTriggerEvent);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                eError = OMX_ErrorUndefined;
            }
        }
    }
    if( pBaseComInt->pCmdCompleteEvent ) {
        tStatus = TIMM_OSAL_EventDelete(pBaseComInt->pCmdCompleteEvent);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
    }
    if( pBaseComInt->pErrorCmdcompleteEvent ) {
        tStatus = TIMM_OSAL_EventDelete(pBaseComInt->pErrorCmdcompleteEvent);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
    }
    if( pBaseComInt->pCmdPipe ) {
        tStatus = TIMM_OSAL_DeletePipe(pBaseComInt->pCmdPipe);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
    }
    if( pBaseComInt->pCmdDataPipe ) {
        /*If pipe still has some data then empty the data and free the memory*/
        tStatus = TIMM_OSAL_GetPipeReadyMessageCount(pBaseComInt->pCmdDataPipe,
                                                     &nCount);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        } else {
            while( nCount > 0 ) {
                tStatus = TIMM_OSAL_ReadFromPipe(pBaseComInt->pCmdDataPipe,
                                                 pData, sizeof(OMX_PTR), &nActualSize, TIMM_OSAL_NO_SUSPEND);
                if( tStatus != TIMM_OSAL_ERR_NONE ) {
                    eError = OMX_ErrorUndefined;
                    break;
                }
                TIMM_OSAL_Free(pData);
                nCount--;
            }
        }
        tStatus = TIMM_OSAL_DeletePipe(pBaseComInt->pCmdDataPipe);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
    }
    if( pBaseComInt->pCmdPipeMutex ) {
        tStatus = TIMM_OSAL_MutexDelete(pBaseComInt->pCmdPipeMutex);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
    }

    TIMM_OSAL_MutexRelease(pBaseComInt->pNewStateMutex);

    if( pBaseComInt->pNewStateMutex ) {
        tStatus = TIMM_OSAL_MutexDelete(pBaseComInt->pNewStateMutex);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
    }
    if( pBaseComPvt->pMutex ) {
        tStatus = TIMM_OSAL_MutexDelete(pBaseComPvt->pMutex);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            eError = OMX_ErrorUndefined;
        }
    }
    TIMM_OSAL_Free(pBaseComInt);
    pBaseComInt = NULL;

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_ComponentDeinit", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_SendCommand :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SendCommand(OMX_HANDLETYPE hComponent,
                                   OMX_COMMANDTYPE Cmd,
                                   OMX_U32 nParam1,
                                   OMX_PTR pCmdData)
{
    OMX_ERRORTYPE            eError  = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_BASE_CMDPARAMS       sCmdParams, sErrorCmdParams;
    OMX_U32                  nPorts, nStartPortNumber, nIndex;
    OMX_PTR                  pLocalCmdData = NULL;
    OMX_BOOL                 bFreeCmdDataIfError = OMX_TRUE;
    OMX_U32                  i = 0;
    TIMM_OSAL_U32            nActualSize = 0, nCmdCount = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SendCommand");
    OMX_BASE_REQUIRE(hComponent != NULL, OMX_ErrorBadParameter);
    if( OMX_CommandMarkBuffer == Cmd ) {
        OMX_BASE_REQUIRE(pCmdData != NULL, OMX_ErrorBadParameter);
    }
    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nPorts = pBaseComPvt->tDerToBase.nNumPorts;
    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);

    switch( Cmd ) {
        case OMX_CommandStateSet :
            /*Return error if unknown state is provided*/
            OMX_BASE_REQUIRE(((OMX_STATETYPE)nParam1 <=
                              OMX_StateWaitForResources), OMX_ErrorBadParameter);

            /*Mutex protection is for multiple SendCommands on the same
            component parallely. This can especially happen in error handling
            scenarios. Mutex protection ensure that the NewState variable is not
            overwritten.*/
            TIMM_OSAL_MutexObtain(pBaseComInt->pNewStateMutex,
                                  TIMM_OSAL_SUSPEND);
            /*Multiple state transitions at the same time is not allowed.
            Though it is allowed by the spec, we prohibit it in our
            implementation*/
            if( OMX_StateMax != pBaseComPvt->tNewState ) {
                eError = OMX_ErrorIncorrectStateTransition;
                OMX_BASE_Error("Returning error since state transition is ongoing. \
                    Our implementation doe not support multiple state transitions at the same time");
                OMX_BASE_Error("[handle = 0x%x] Cur= %d :: New= %d", hComponent, pBaseComPvt->tCurState, pBaseComPvt->tNewState);
                TIMM_OSAL_MutexRelease(pBaseComInt->pNewStateMutex);
                goto EXIT;
            }
            pBaseComPvt->tNewState = (OMX_STATETYPE)nParam1;

            TIMM_OSAL_MutexRelease(pBaseComInt->pNewStateMutex);
            break;

        case OMX_CommandPortDisable :
            /* Index of the port to disable should be less than
             * no of ports or equal to OMX_ALL */
            OMX_BASE_ASSERT((nParam1 < (nStartPortNumber + nPorts)) ||
                            (nParam1 == OMX_ALL), OMX_ErrorBadPortIndex);
            if( OMX_ALL == nParam1 ) {
                /*Dont want to return same port state error if it never enters
                the for loop*/
                if( nPorts > 0 ) {
                    eError = (OMX_ERRORTYPE)OMX_TI_ErrorPortIsAlreadyDisabled;
                }

                for( nIndex = 0; nIndex < nPorts; nIndex++ ) {
                    pPort = pBaseComInt->ports[nIndex];
                    /*Atleast 1 port is enabled - dont send same port state error*/
                    if( pPort->sPortDef.bEnabled ) {
                        pPort->sPortDef.bEnabled = OMX_FALSE;
                        pPort->bIsTransitionToDisabled = OMX_TRUE;
                        eError = OMX_ErrorNone;
                    }
                }
            } else {
                nIndex = nParam1 - nStartPortNumber;
                pPort = pBaseComInt->ports[nIndex];
                if( pPort->sPortDef.bEnabled ) {
                    pPort->sPortDef.bEnabled = OMX_FALSE;
                    pPort->bIsTransitionToDisabled = OMX_TRUE;
                } else {
                    eError = (OMX_ERRORTYPE)OMX_TI_ErrorPortIsAlreadyDisabled;
                }
            }
            break;

        case OMX_CommandPortEnable :
            /* Index of the port to enable should be less than
             * no of ports or equal to OMX_ALL */
            OMX_BASE_ASSERT((nParam1 < (nStartPortNumber + nPorts)) ||
                            (nParam1 == OMX_ALL), OMX_ErrorBadPortIndex);
            if( OMX_ALL == nParam1 ) {
                /*Dont want to return same port state error if it never enters
                the for loop*/
                if( nPorts > 0 ) {
                    eError = (OMX_ERRORTYPE)OMX_TI_ErrorPortIsAlreadyEnabled;
                }

                for( nIndex = 0; nIndex < nPorts; nIndex++ ) {
                    pPort = pBaseComInt->ports[nIndex];
                    /*Atleast 1 port is disabled - dont send same port state error*/
                    if( !pPort->sPortDef.bEnabled ) {
                        pPort->sPortDef.nBufferCountActual =
                            pBaseComPvt->pPortdefs[nIndex]->nBufferCountActual;
                        eError = OMX_ErrorNone;
                        pPort->sPortDef.bEnabled = OMX_TRUE;
                        pPort->bIsTransitionToEnabled = OMX_TRUE;
                    }
                }
            } else {
                nIndex = nParam1 - nStartPortNumber;
                pPort = pBaseComInt->ports[nIndex];
                if( !pPort->sPortDef.bEnabled ) {
                    pPort->sPortDef.nBufferCountActual =
                        pBaseComPvt->pPortdefs[nIndex]->nBufferCountActual;
                    pPort->sPortDef.bEnabled = OMX_TRUE;
                    pPort->bIsTransitionToEnabled = OMX_TRUE;
                } else {
                    eError = (OMX_ERRORTYPE)OMX_TI_ErrorPortIsAlreadyEnabled;
                }
            }
            break;

        case OMX_CommandMarkBuffer :
            /*For mark buffer pCmdData points to a structure of type OMX_MARKTYPE.
            This may not be valid once send commmand returns so allocate memory and
            copy this info there. This memory will be freed up during the command
            complete callback for mark buffer.*/
            OMX_BASE_ASSERT((nParam1 < (nStartPortNumber + nPorts)) ||
                            (nParam1 == OMX_ALL), OMX_ErrorBadPortIndex);
            pLocalCmdData = OMX_BASE_Malloc(sizeof(OMX_MARKTYPE),
                                            pBaseComPvt->tDerToBase.hDefaultHeap);
            OMX_BASE_ASSERT(pLocalCmdData != NULL,
                            OMX_ErrorInsufficientResources);
            TIMM_OSAL_Memcpy(pLocalCmdData, pCmdData, sizeof(OMX_MARKTYPE));
            break;
        case OMX_CommandFlush :
            OMX_BASE_Info("[handle 0x%x] ur asking me to do Flush on %d", hComponent, nParam1);
            OMX_BASE_ASSERT((nParam1 < (nStartPortNumber + nPorts)) ||
                            (nParam1 == OMX_ALL), OMX_ErrorBadPortIndex);
            break;

        default :
            OMX_BASE_Error(" Unknown command received ");
            OMX_BASE_ASSERT(OMX_FALSE, OMX_ErrorBadParameter);
    }

    /*Return error if port enable/disable command is sent on an already
    enabled/disabled port*/
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    /*For mark buffer store pCmdData in a separate pipe - to be freed up during
    command complete callback*/
    if( Cmd == OMX_CommandMarkBuffer ) {
        /*If pipe is full, return error - thus a limitation that currently
        cannot queue up more than OMX_BASE_MAXCMDS mark buffer commands*/
        tStatus = TIMM_OSAL_WriteToPipe(pBaseComInt->pCmdDataPipe,
                                        &pLocalCmdData, sizeof(OMX_PTR), TIMM_OSAL_NO_SUSPEND);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                        OMX_ErrorInsufficientResources);
    }
    /*Obtain mutex for writing to command pipe*/
    tStatus = TIMM_OSAL_MutexObtain(pBaseComInt->pCmdPipeMutex,
                                    TIMM_OSAL_SUSPEND);
    if((tStatus != TIMM_OSAL_ERR_NONE) && (Cmd == OMX_CommandMarkBuffer)) {
        bFreeCmdDataIfError = OMX_FALSE;
    }
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);

    /* keep this info, and process later */
    sCmdParams.eCmd     = Cmd;
    sCmdParams.unParam  = nParam1;
    if( Cmd == OMX_CommandMarkBuffer ) {
        sCmdParams.pCmdData = pLocalCmdData;
    } else {
        sCmdParams.pCmdData = pCmdData;
    }
    tStatus = TIMM_OSAL_WriteToPipe(pBaseComInt->pCmdPipe, &sCmdParams,
                                    sizeof(sCmdParams), TIMM_OSAL_SUSPEND);
    if( tStatus != TIMM_OSAL_ERR_NONE ) {
        /*Do not free pLocalCmdData in this case since it has been pushed to
        pipe and will now be freed during deinit when pipe is deleted*/
        if( Cmd == OMX_CommandMarkBuffer ) {
            bFreeCmdDataIfError = OMX_FALSE;
        }
        /*Release the locked mutex and exit with error*/
        OMX_BASE_Error("Write to pipe failed");
        eError = OMX_ErrorInsufficientResources;
        tStatus = TIMM_OSAL_MutexRelease(pBaseComInt->pCmdPipeMutex);
        goto EXIT;
    }
    /* This call invokes the process function directly incase if comp
     * does process in client context, otherwise triggers compo thread */
    eError = pBaseComInt->fpInvokeProcessFunction(hComponent, CMDEVENT);
    if( eError != OMX_ErrorNone ) {
        if( Cmd == OMX_CommandMarkBuffer ) {
            /*Do not free pLocalCmdData in this case since it has been pushed to
            pipe and will now be freed during deinit when pipe is deleted*/
            bFreeCmdDataIfError = OMX_FALSE;
        }
        /*Get the count in cmd pipe - this is to be used for popping the
        recently added cmd to the pipe since that is no longer valid*/
        tStatus = TIMM_OSAL_GetPipeReadyMessageCount(pBaseComInt->pCmdPipe,
                                                     &nCmdCount);
        if( tStatus != TIMM_OSAL_ERR_NONE ) {
            /*Release mutex and return error*/
            OMX_BASE_Error("Get count of pipe failed");
            eError = OMX_ErrorUndefined;
            tStatus = TIMM_OSAL_MutexRelease(pBaseComInt->pCmdPipeMutex);
            goto EXIT;
        }

        for( i = 0; i < nCmdCount; i++ ) {
            /*Clear the last command from pipe since error has occured*/
            tStatus = TIMM_OSAL_ReadFromPipe(pBaseComInt->pCmdPipe,
                                             &sErrorCmdParams, sizeof(sErrorCmdParams), &nActualSize,
                                             TIMM_OSAL_SUSPEND);
            if( tStatus != TIMM_OSAL_ERR_NONE ) {
                OMX_BASE_Error("Read pipe failed");
                eError = OMX_ErrorUndefined;
                break;
            }
            if( TIMM_OSAL_Memcmp(&sErrorCmdParams, &sCmdParams,
                                 sizeof(OMX_BASE_CMDPARAMS)) == 0 ) {
                OMX_BASE_Trace("Found the command to discard");
                break;
            } else {
                /*This is not the command to be discarded - write it back to
                pipe*/
                tStatus = TIMM_OSAL_WriteToPipe(pBaseComInt->pCmdPipe,
                                                &sErrorCmdParams, sizeof(sErrorCmdParams),
                                                TIMM_OSAL_SUSPEND);
                if( tStatus != TIMM_OSAL_ERR_NONE ) {
                    OMX_BASE_Error("Write to pipe failed");
                    eError = OMX_ErrorUndefined;
                    break;
                }
            }
        }

        if( i == nCmdCount ) {
            /*The command to discard was not found even after going through the
            pipe*/
            OMX_BASE_Error("Command to be discarded not found in pipe");
            eError = OMX_ErrorUndefined;
        }
    }
    tStatus = TIMM_OSAL_MutexRelease(pBaseComInt->pCmdPipeMutex);
    OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus, OMX_ErrorUndefined);

EXIT:
    if( eError != OMX_ErrorNone ) {
        if( pLocalCmdData && bFreeCmdDataIfError ) {
            TIMM_OSAL_Free(pLocalCmdData);
            pLocalCmdData = NULL;
        }
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SendCommand", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_GetParameter :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetParameter(OMX_HANDLETYPE hComponent,
                                    OMX_INDEXTYPE nParamIndex,
                                    OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                      eError  = OMX_ErrorNone;
    OMX_COMPONENTTYPE                 *pComp = NULL;
    OMX_BASE_PRIVATETYPE              *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE             *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE                 *pPort = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE      *pPortDef = NULL;
    OMX_PRIORITYMGMTTYPE              *pPriorityMgmt = NULL;
    OMX_PARAM_BUFFERSUPPLIERTYPE      *pBufSupplier = NULL;
    OMX_U32                            nStartPortNumber, nPorts, nPortIndex;
    OMX_TI_PARAM_BUFFERPREANNOUNCE    *pBufPreAnnounce = NULL;
    OMX_TI_PARAM_METADATABUFFERINFO   *pMetaDataProperty = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetParameter");

    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pParamStruct != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nPorts = pBaseComPvt->tDerToBase.nNumPorts;
    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);

    switch( nParamIndex ) {
        case OMX_IndexParamAudioInit :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PORT_PARAM_TYPE, eError);
            if( pBaseComPvt->tDerToBase.pAudioPortParams == NULL ) {
                OMX_BASE_INIT_STRUCT_PTR((OMX_PORT_PARAM_TYPE *)(pParamStruct),
                                         OMX_PORT_PARAM_TYPE);
                ((OMX_PORT_PARAM_TYPE *)pParamStruct)->nPorts           = 0;
                ((OMX_PORT_PARAM_TYPE *)pParamStruct)->nStartPortNumber = 0;
                break;
            }
            *(OMX_PORT_PARAM_TYPE *)pParamStruct =
                *(pBaseComPvt->tDerToBase.pAudioPortParams);
            break;

        case OMX_IndexParamImageInit :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PORT_PARAM_TYPE, eError);
            if( pBaseComPvt->tDerToBase.pImagePortParams == NULL ) {
                OMX_BASE_INIT_STRUCT_PTR((OMX_PORT_PARAM_TYPE *)(pParamStruct),
                                         OMX_PORT_PARAM_TYPE);
                ((OMX_PORT_PARAM_TYPE *)pParamStruct)->nPorts           = 0;
                ((OMX_PORT_PARAM_TYPE *)pParamStruct)->nStartPortNumber = 0;
                break;
            }
            *(OMX_PORT_PARAM_TYPE *)pParamStruct =
                *(pBaseComPvt->tDerToBase.pImagePortParams);
            break;

        case OMX_IndexParamVideoInit :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PORT_PARAM_TYPE, eError);
            if( pBaseComPvt->tDerToBase.pVideoPortParams == NULL ) {
                OMX_BASE_INIT_STRUCT_PTR((OMX_PORT_PARAM_TYPE *)(pParamStruct),
                                         OMX_PORT_PARAM_TYPE);
                ((OMX_PORT_PARAM_TYPE *)pParamStruct)->nPorts           = 0;
                ((OMX_PORT_PARAM_TYPE *)pParamStruct)->nStartPortNumber = 0;
                break;
            }
            *(OMX_PORT_PARAM_TYPE *)pParamStruct =
                *(pBaseComPvt->tDerToBase.pVideoPortParams);
            break;

        case OMX_IndexParamOtherInit :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PORT_PARAM_TYPE, eError);
            if( pBaseComPvt->tDerToBase.pOtherPortParams == NULL ) {
                OMX_BASE_INIT_STRUCT_PTR((OMX_PORT_PARAM_TYPE *)(pParamStruct),
                                         OMX_PORT_PARAM_TYPE);
                ((OMX_PORT_PARAM_TYPE *)pParamStruct)->nPorts           = 0;
                ((OMX_PORT_PARAM_TYPE *)pParamStruct)->nStartPortNumber = 0;
                break;
            }
            *(OMX_PORT_PARAM_TYPE *)pParamStruct =
                *(pBaseComPvt->tDerToBase.pOtherPortParams);
            break;

        case OMX_IndexParamPortDefinition :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_PORTDEFINITIONTYPE,
                                 eError);
            pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pParamStruct;
            nPortIndex = pPortDef->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts,
                            OMX_ErrorBadPortIndex);
            *pPortDef = pBaseComInt->ports[nPortIndex]->sPortDef;
            if (pPortDef->eDir == OMX_DirOutput)
                pPortDef->format.video.eColorFormat = HAL_NV12_PADDED_PIXEL_FORMAT;

            break;

        case OMX_IndexParamCompBufferSupplier :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_BUFFERSUPPLIERTYPE,
                                 eError);
            pBufSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)pParamStruct;
            nPortIndex = pBufSupplier->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts,
                            OMX_ErrorBadPortIndex);
            pPort = pBaseComInt->ports[nPortIndex];
            pBufSupplier->eBufferSupplier = pPort->eSupplierSetting;
            break;

        case OMX_IndexParamPriorityMgmt :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PRIORITYMGMTTYPE, eError);
            pPriorityMgmt = (OMX_PRIORITYMGMTTYPE *)pParamStruct;

            pPriorityMgmt->nGroupID = pBaseComPvt->tDerToBase.nGroupID;
            pPriorityMgmt->nGroupPriority =
                pBaseComPvt->tDerToBase.nGroupPriority;
            break;

        case (OMX_INDEXTYPE)OMX_TI_IndexParamBufferPreAnnouncement :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_BUFFERPREANNOUNCE,
                                 eError);
            pBufPreAnnounce = (OMX_TI_PARAM_BUFFERPREANNOUNCE *)pParamStruct;
            nPortIndex = pBufPreAnnounce->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts, OMX_ErrorBadPortIndex);
            pBufPreAnnounce->bEnabled = pBaseComPvt->pBufPreAnnouncePerPort
                                        [nPortIndex]->bEnabled;
            break;


        case (OMX_INDEXTYPE)OMX_TI_IndexParamMetaDataBufferInfo :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_METADATABUFFERINFO,
                                 eError);
            pMetaDataProperty = (OMX_TI_PARAM_METADATABUFFERINFO *)pParamStruct;
            pMetaDataProperty->bIsMetaDataEnabledOnPort = (OMX_BOOL)
                (pBaseComPvt->tDerToBase.pPortProperties[pMetaDataProperty->nPortIndex]->nNumMetaDataFields != 0);
            pMetaDataProperty->nMetaDataSize = pBaseComInt->ports[pMetaDataProperty->nPortIndex]->nMetaDataSize;
            break;

        default :
            OMX_BASE_Trace("Unknown Index received ");
            eError = OMX_ErrorUnsupportedIndex;
            break;
    }

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetParameter", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_SetParameter :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SetParameter(OMX_HANDLETYPE hComponent,
                                    OMX_INDEXTYPE nIndex,
                                    OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                     eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                *pComp = NULL;
    OMX_BASE_PRIVATETYPE             *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE            *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE                *pPort = NULL;
    OMX_PRIORITYMGMTTYPE             *pPriorityMgmt = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE     *pPortDef = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE     *pLocalPortDef;
    OMX_PARAM_BUFFERSUPPLIERTYPE     *pBufSupplier = NULL;
    OMX_U32                           nStartPortNumber, nPorts, nPortIndex;
    OMX_PARAM_BUFFERSUPPLIERTYPE      sTunBufSupplier;
    OMX_TI_PARAM_BUFFERPREANNOUNCE   *pBufPreAnnounce = NULL;
	OMX_TI_PARAMUSENATIVEBUFFER *pParamNativeBuffer = NULL;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetParameter");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pParamStruct != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nPorts = pBaseComPvt->tDerToBase.nNumPorts;
    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);
    /* This method is not allowed when the component is not in the loaded
     * state or the the port is not disabled */
    if((OMX_IndexParamPriorityMgmt == nIndex
        || OMX_IndexParamAudioInit == nIndex
        || OMX_IndexParamVideoInit == nIndex
        || OMX_IndexParamImageInit == nIndex
        || OMX_IndexParamOtherInit == nIndex)
       && (pBaseComPvt->tCurState != OMX_StateLoaded)) {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    switch( nIndex ) {
        case OMX_IndexParamPriorityMgmt :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PRIORITYMGMTTYPE, eError);
            pPriorityMgmt = (OMX_PRIORITYMGMTTYPE *)pParamStruct;

            pBaseComPvt->tDerToBase.nGroupID = pPriorityMgmt->nGroupID;
            pBaseComPvt->tDerToBase.nGroupPriority =
                pPriorityMgmt->nGroupPriority;
            break;

        case OMX_IndexParamPortDefinition :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_PORTDEFINITIONTYPE,
                                 eError);
            pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pParamStruct;

            nPortIndex = pPortDef->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts,
                            OMX_ErrorBadPortIndex);
            pPort = pBaseComInt->ports[nPortIndex];
            /* successfully only when the comp is in loaded or disabled port */
            OMX_BASE_ASSERT((pBaseComPvt->tCurState == OMX_StateLoaded) ||
                            (pPort->sPortDef.bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            pLocalPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pParamStruct;
            /*Copying only the modifiabke fields. Rest are all read only fields*/
            pBaseComInt->ports[nPortIndex]->sPortDef.nBufferCountActual =
                pLocalPortDef->nBufferCountActual;
            pBaseComInt->ports[nPortIndex]->sPortDef.format =
                pLocalPortDef->format;
            break;

        /*This will update the buffer supplier setting to the setting requested
        by the caller of this function. If the port setting is SUPPLIER_MUST or
        SUPPLIER_NEVER and caller request is against this then a warning will be
        thrown out as well. After the supplier setting is updated, if this is a
        tunneled port then certain additional tasks need to be performed. If
        this is an input port then the supplier settings need to be communicated
        with the tunneled output port. If this is an output port then only local
        settings need to be updated (In tunneling cases, client only call
        SetParam for this index on i/p port while i/p port communicats with the
        o/p port)*/
        case OMX_IndexParamCompBufferSupplier :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_BUFFERSUPPLIERTYPE,
                                 eError);
            pBufSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)pParamStruct;

            nPortIndex = pBufSupplier->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts,
                            OMX_ErrorBadPortIndex);
            pPort = pBaseComInt->ports[nPortIndex];
            /*successfull only when the comp is in loaded or port is disabled*/
            OMX_BASE_ASSERT((pBaseComPvt->tCurState == OMX_StateLoaded) ||
                            (pPort->sPortDef.bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);
            if( pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                eBufAllocPref == OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_MUST ) {
                if(((pBufSupplier->eBufferSupplier == OMX_BufferSupplyInput) &&
                    (pPort->sPortDef.eDir == OMX_DirInput)) ||
                   ((pBufSupplier->eBufferSupplier == OMX_BufferSupplyOutput) &&
                    (pPort->sPortDef.eDir == OMX_DirOutput))) {
                    //fine
                } else {
                    TIMM_OSAL_Warning(" This port wants to be the buffer \
                                      supplier. Changing the supplier setting \
                                      may cause the component to malfunction ");
                }
            } else if( pBaseComPvt->tDerToBase.pPortProperties[nPortIndex]->
                       eBufAllocPref == OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_NEVER ) {
                if(((pBufSupplier->eBufferSupplier == OMX_BufferSupplyInput) &&
                    (pPort->sPortDef.eDir == OMX_DirInput)) ||
                   ((pBufSupplier->eBufferSupplier == OMX_BufferSupplyOutput) &&
                    (pPort->sPortDef.eDir == OMX_DirOutput))) {
                    TIMM_OSAL_Warning(" This port does not want to be the \
                                      buffer supplier. Making it the supplier \
                                      may cause the component to malfunction ");
                }
            }
            pPort->eSupplierSetting = pBufSupplier->eBufferSupplier;
            /* Incase of tunneled port, communicate the change in
             * supplier setting to the Tunneled port also */
            if( pPort->hTunnelComp == NULL ) {
                break;
            }
            /*Input port can receive SetParam on this index only from the client
            It then communicates the supplier settings to the output port*/
            if((pBufSupplier->eBufferSupplier == OMX_BufferSupplyInput) &&
               (pPort->sPortDef.eDir == OMX_DirInput)) {
                pPort->nTunnelFlags |= OMX_BASE_DIO_SUPPLIER;
                sTunBufSupplier = *pBufSupplier;
                sTunBufSupplier.nPortIndex = pPort->nTunnelPort;

                eError = OMX_SetParameter(pPort->hTunnelComp,
                                          OMX_IndexParamCompBufferSupplier,
                                          &sTunBufSupplier);
            } else if((pBufSupplier->eBufferSupplier == OMX_BufferSupplyOutput) &&
                      (pPort->sPortDef.eDir == OMX_DirInput)) {
                if( PORT_IS_SUPPLIER(pPort)) {
                    pPort->nTunnelFlags &= ~OMX_BASE_DIO_SUPPLIER;
                    sTunBufSupplier = *pBufSupplier;
                    sTunBufSupplier.nPortIndex = pPort->nTunnelPort;

                    eError = OMX_SetParameter(pPort->hTunnelComp,
                                              OMX_IndexParamCompBufferSupplier,
                                              &sTunBufSupplier);

                }
            }
            /*Output port can only receive this call from the tunneled input
            port*/
            else if((pBufSupplier->eBufferSupplier == OMX_BufferSupplyOutput)
                    && (pPort->sPortDef.eDir == OMX_DirOutput)) {
                pPort->nTunnelFlags |= OMX_BASE_DIO_SUPPLIER;
            } else {
                if( PORT_IS_SUPPLIER(pPort)) {
                    pPort->nTunnelFlags &= ~OMX_BASE_DIO_SUPPLIER;
                }
            }
            break;

        case (OMX_INDEXTYPE)OMX_TI_IndexParamBufferPreAnnouncement :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_TI_PARAM_BUFFERPREANNOUNCE,
                                 eError);
            pBufPreAnnounce = (OMX_TI_PARAM_BUFFERPREANNOUNCE *)pParamStruct;
            nPortIndex = pBufPreAnnounce->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts, OMX_ErrorBadPortIndex);
            pPort = pBaseComInt->ports[nPortIndex];
            /*successfull only when the comp is in loaded or port is disabled*/
            OMX_BASE_ASSERT((pBaseComPvt->tCurState == OMX_StateLoaded) ||
                            (pPort->sPortDef.bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);
            pBaseComPvt->pBufPreAnnouncePerPort[nPortIndex]->bEnabled =
                pBufPreAnnounce->bEnabled;
            break;

        /*These are compulsory parameters hence being supported by base*/
        case OMX_IndexParamAudioInit :
        case OMX_IndexParamVideoInit :
        case OMX_IndexParamImageInit :
        case OMX_IndexParamOtherInit :
            /*All fields of OMX_PORT_PARAM_TYPE are read only so SetParam will just
            return and not overwrite anything*/
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PORT_PARAM_TYPE, eError);
            break;

        case OMX_TI_IndexUseNativeBuffers:
            pParamNativeBuffer = (OMX_TI_PARAMUSENATIVEBUFFER* )pParamStruct;
            if(pParamNativeBuffer->bEnable == OMX_TRUE) {
                OMX_BASE_Trace("Enabling GRALLOC Pointer for port[%d]", pParamNativeBuffer->nPortIndex);
                pBaseComPvt->tDerToBase.pPortProperties[pParamNativeBuffer->nPortIndex - nStartPortNumber]->eBufMemoryType =   OMX_BASE_BUFFER_MEMORY_GRALLOC;
            } else {
                OMX_BASE_Trace("Enabling def Pointer for port[%d]", pParamNativeBuffer->nPortIndex);
                pBaseComPvt->tDerToBase.pPortProperties[pParamNativeBuffer->nPortIndex - nStartPortNumber]->eBufMemoryType = OMX_BASE_BUFFER_MEMORY_DEFAULT;
            }
            break;

        default :
            OMX_BASE_Trace("Unsupported Index received ");
            eError = OMX_ErrorUnsupportedIndex;
            break;
    }

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetParameter", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_AllocateBuffer :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_AllocateBuffer(OMX_HANDLETYPE hComponent,
                                      OMX_BUFFERHEADERTYPE * *ppBufferHdr,
                                      OMX_U32 nPortIndex,
                                      OMX_PTR pAppPrivate,
                                      OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE                eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE          tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE           *pComp = NULL;
    OMX_BASE_PRIVATETYPE        *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE       *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE           *pPort = NULL;
    OMX_BASE_DIO_CreateParams    sDIOCreateParams;
    OMX_BASE_DIO_OpenParams      sDIOOpenParams;
    OMX_U32                      nStartPortNumber = 0, nPorts = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_AllocateBuffer");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (nSizeBytes > 0), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    nPorts = pBaseComPvt->tDerToBase.nNumPorts;
    OMX_BASE_REQUIRE(nPortIndex >= nStartPortNumber && nPortIndex < nPorts,
                     OMX_ErrorBadPortIndex);
    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);

    pPort = pBaseComInt->ports[nPortIndex - nStartPortNumber];
    /*Allocate buffer cannot be called on a port with pre announcement disabled*/
    OMX_BASE_ASSERT(pBaseComPvt->pBufPreAnnouncePerPort
                    [nPortIndex - nStartPortNumber]->bEnabled == OMX_TRUE,
                    OMX_ErrorBadParameter);

    /*Buffer size should be >= the minimum buffer size specified in
      port definition*/
    OMX_BASE_REQUIRE(nSizeBytes >= pPort->sPortDef.nBufferSize,
                     OMX_ErrorBadParameter);
    if((pBaseComPvt->tCurState == OMX_StateLoaded ||
        pBaseComPvt->tCurState == OMX_StateWaitForResources) &&
       (pBaseComPvt->tNewState == OMX_StateIdle) &&
       (pPort->sPortDef.bEnabled)) {
        /*Allowed during loaded/waitforresources --> idle transition if port is
        enabled*/
        OMX_BASE_Trace("Component can Allocate buffers");
    } else if((pPort->bIsTransitionToEnabled) &&
              (pBaseComPvt->tCurState != OMX_StateLoaded) &&
              (pBaseComPvt->tCurState != OMX_StateWaitForResources)) {
        /*Allowed when port is transitioning to enabled if current state is not
        loaded/waitforresources*/
        OMX_BASE_Trace("Component can Allocate buffers");
    } else {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }
    /*Port should not be already populated*/
    OMX_BASE_ASSERT(pPort->sPortDef.bPopulated == OMX_FALSE,
                    OMX_ErrorBadParameter);
    /*Cannot allocate bufers for tunneled ports*/
    OMX_BASE_ASSERT(pPort->hTunnelComp == NULL, OMX_ErrorBadParameter);

    /* Allocate buffer is called only on the Non tunneled port */
    pPort->nTunnelFlags |= OMX_BASE_DIO_SUPPLIER;
    if( pPort->nBufferCnt == 0 ) {
        /* Initialize DIO and open as a supplier  */
        sDIOCreateParams.hComponent        = hComponent;
        sDIOCreateParams.pPort             = pPort;
        sDIOCreateParams.pAppCallbacks     = &(pBaseComInt->sAppCallbacks);
        if( pPort->sPortDef.eDir == OMX_DirOutput ) {
            sDIOOpenParams.nMode = OMX_BASE_DIO_WRITER;
        } else {
            sDIOOpenParams.nMode = OMX_BASE_DIO_READER;
        }
        if( pBaseComPvt->tDerToBase.pPortProperties[nPortIndex -
                                                    nStartPortNumber]->bDMAAccessedBuffer ) {
            sDIOOpenParams.bCacheFlag = OMX_FALSE;
        } else {
            sDIOOpenParams.bCacheFlag = OMX_TRUE;
        }
        sDIOOpenParams.nBufSize = nSizeBytes;

        eError = _OMX_BASE_GetChannelInfo(pPort, sDIOCreateParams.cChannelName);
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

        eError = OMX_BASE_DIO_Init(hComponent, nPortIndex, "OMX.DIO.NONTUNNEL",
                                   &sDIOCreateParams);
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
        eError = OMX_BASE_DIO_Open(hComponent, nPortIndex, &sDIOOpenParams);
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    }
    /*update buffer header from buffer list  */
    *ppBufferHdr = pPort->pBufferlist[pPort->nBufferCnt];
    if( pPort->sPortDef.eDir == OMX_DirInput ) {
        (*ppBufferHdr)->nInputPortIndex  = pPort->sPortDef.nPortIndex;
        (*ppBufferHdr)->nOutputPortIndex = OMX_NOPORT;
        /*If component will read this buffer via CPU then set the flag*/
        if( pBaseComPvt->tDerToBase.pPortProperties
            [nPortIndex - nStartPortNumber]->bDMAAccessedBuffer == OMX_FALSE ) {
            ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->
            bReadViaCPU = OMX_TRUE;
        } else {
            ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->
            bReadViaCPU = OMX_FALSE;
        }
    } else if( pPort->sPortDef.eDir == OMX_DirOutput ) {
        (*ppBufferHdr)->nOutputPortIndex  = pPort->sPortDef.nPortIndex;
        (*ppBufferHdr)->nInputPortIndex   = OMX_NOPORT;
        /*If component will write this buffer via CPU then set the flag*/
        if( pBaseComPvt->tDerToBase.pPortProperties
            [nPortIndex - nStartPortNumber]->bDMAAccessedBuffer == OMX_FALSE ) {
            ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->
            bWriteViaCPU = OMX_TRUE;
        } else {
            ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->
            bWriteViaCPU = OMX_FALSE;
        }
    }
    (*ppBufferHdr)->pAppPrivate = pAppPrivate;
    (*ppBufferHdr)->nAllocLen = nSizeBytes;

    pPort->nBufferCnt++;
    if( pPort->sPortDef.nBufferCountActual == pPort->nBufferCnt ) {
        pPort->sPortDef.bPopulated = OMX_TRUE;
        tStatus = TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent, BUF_ALLOC_EVENT,
                                     TIMM_OSAL_EVENT_OR);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                        OMX_ErrorInsufficientResources);
    }



EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_AllocateBuffer", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_UseBuffer :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_UseBuffer(OMX_HANDLETYPE hComponent,
                                 OMX_BUFFERHEADERTYPE * *ppBufferHdr,
                                 OMX_U32 nPortIndex,
                                 OMX_PTR pAppPrivate,
                                 OMX_U32 nSizeBytes,
                                 OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE                eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE          tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE           *pComp = NULL;
    OMX_BASE_PRIVATETYPE        *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE       *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE           *pPort = NULL;
    OMX_BASE_DIO_CreateParams    sDIOCreateParams;
    OMX_BASE_DIO_OpenParams      sDIOOpenParams;
    OMX_U32                      nStartPortNumber = 0, nPorts = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_UseBuffer");
    OMX_BASE_REQUIRE((hComponent != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    nPorts = pBaseComPvt->tDerToBase.nNumPorts;
    OMX_BASE_REQUIRE(nPortIndex >= nStartPortNumber && nPortIndex < nPorts,
                     OMX_ErrorBadPortIndex);
    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);

    pPort = pBaseComInt->ports[nPortIndex - nStartPortNumber];

    if((pBaseComPvt->tCurState == OMX_StateLoaded ||
        pBaseComPvt->tCurState == OMX_StateWaitForResources) &&
       (pBaseComPvt->tNewState == OMX_StateIdle) &&
       (pPort->sPortDef.bEnabled)) {
        /*Allowed during loaded/waitforresources --> idle transition if port is
        enabled*/
        OMX_BASE_Trace("Component can Use buffers");
    } else if((pPort->bIsTransitionToEnabled) &&
              (pBaseComPvt->tCurState != OMX_StateLoaded) &&
              (pBaseComPvt->tCurState != OMX_StateWaitForResources)) {
        /*Allowed when port is transitioning to enabled if current state is not
        loaded/waitforresources*/
        OMX_BASE_Trace("Component can Use buffers");
    } else {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }
    OMX_BASE_ASSERT(pPort->sPortDef.bPopulated == OMX_FALSE,
                    OMX_ErrorBadParameter);
    /*For pre-announcement enabled ports, these checks are applicable*/
    if( pBaseComPvt->pBufPreAnnouncePerPort[nPortIndex - nStartPortNumber]->
        bEnabled ) {
        /*Buffer size should be >= the minimum buffer size specified in
          port definition*/
        OMX_BASE_REQUIRE(nSizeBytes >= pPort->sPortDef.nBufferSize,
                         OMX_ErrorBadParameter);
        OMX_BASE_REQUIRE(pBuffer != NULL, OMX_ErrorBadParameter);
        if( pPort->sPortDef.nBufferAlignment != 0 ) {
            /*Return error if buffer is not correctly aligned*/
           //SN:TBD
           // OMX_BASE_REQUIRE(((OMX_U32)pBuffer % pPort->sPortDef.
            //                  nBufferAlignment) == 0, OMX_ErrorBadParameter);
        }
    }
    /* Use buffer is always called on  non supplier port  */
    pPort->nTunnelFlags |= OMX_BASE_DIO_NONSUPPLIER;
    if( pPort->nBufferCnt == 0 ) {
        /* Initialize DIO if not initialized and open as a non supplier  */
        if( pPort->hDIO == NULL && (!PORT_IS_TUNNELED(pPort))) {
            sDIOCreateParams.hComponent        = hComponent;
            sDIOCreateParams.pPort             = pPort;
            sDIOCreateParams.pAppCallbacks     = &(pBaseComInt->sAppCallbacks);

            eError = _OMX_BASE_GetChannelInfo(pPort,
                                              sDIOCreateParams.cChannelName);
            OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
            if( pPort->sPortDef.eDir == OMX_DirOutput ) {
                sDIOOpenParams.nMode = OMX_BASE_DIO_WRITER;
            } else {
                sDIOOpenParams.nMode = OMX_BASE_DIO_READER;
            }
            if( pBaseComPvt->tDerToBase.pPortProperties[nPortIndex -
                                                        nStartPortNumber]->bDMAAccessedBuffer ) {
                sDIOOpenParams.bCacheFlag = OMX_FALSE;
            } else {
                sDIOOpenParams.bCacheFlag = OMX_TRUE;
            }
            sDIOOpenParams.nBufSize = nSizeBytes;

            eError = OMX_BASE_DIO_Init(hComponent, nPortIndex,
                                       "OMX.DIO.NONTUNNEL", &sDIOCreateParams);
            OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
            eError = OMX_BASE_DIO_Open(hComponent, nPortIndex, &sDIOOpenParams);
            OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
        }
        if( PORT_IS_TUNNELED(pPort)) {
            /*Supplier port waits until non-supplier has opened (so that all headers
            are allocated)*/
            tStatus = TIMM_OSAL_SemaphoreObtain(pPort->pDioOpenCloseSem,
                                                TIMM_OSAL_SUSPEND);
            OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                            OMX_ErrorInsufficientResources);
        }
    }
    /*update buffer header from buffer list  */
    pPort->pBufferlist[pPort->nBufferCnt]->pBuffer = pBuffer;
    *ppBufferHdr = pPort->pBufferlist[pPort->nBufferCnt];
    if( pPort->sPortDef.eDir == OMX_DirInput ) {
        (*ppBufferHdr)->nInputPortIndex  = pPort->sPortDef.nPortIndex;
        (*ppBufferHdr)->nOutputPortIndex = OMX_NOPORT;
        /*If component will read this buffer via CPU then set the flag*/
        if( pBaseComPvt->tDerToBase.pPortProperties
            [nPortIndex - nStartPortNumber]->bDMAAccessedBuffer == OMX_FALSE ) {
            ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->
            bReadViaCPU = OMX_TRUE;
        } else {
            ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->
            bReadViaCPU = OMX_FALSE;
        }
    } else if( pPort->sPortDef.eDir == OMX_DirOutput ) {
        (*ppBufferHdr)->nOutputPortIndex  = pPort->sPortDef.nPortIndex;
        (*ppBufferHdr)->nInputPortIndex   = OMX_NOPORT;
        /*If component will write this buffer via CPU then set the flag*/
        if( pBaseComPvt->tDerToBase.pPortProperties
            [nPortIndex - nStartPortNumber]->bDMAAccessedBuffer == OMX_FALSE ) {
            ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->
            bWriteViaCPU = OMX_TRUE;
        } else {
            ((OMX_TI_PLATFORMPRIVATE *)((*ppBufferHdr)->pPlatformPrivate))->
            bWriteViaCPU = OMX_FALSE;
        }
    }
    (*ppBufferHdr)->pAppPrivate = pAppPrivate;
    (*ppBufferHdr)->nAllocLen = nSizeBytes;

    pPort->nBufferCnt++;
    if( pPort->sPortDef.nBufferCountActual == pPort->nBufferCnt ) {
        pPort->sPortDef.bPopulated = OMX_TRUE;
        tStatus = TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent, BUF_ALLOC_EVENT,
                                     TIMM_OSAL_EVENT_OR);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                        OMX_ErrorInsufficientResources);
    }
EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_UseBuffer", eError);
    return (eError);

}

/*===============================================================*/
/** @fn OMX_BASE_FreeBuffer :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_FreeBuffer(OMX_HANDLETYPE hComponent,
                                  OMX_U32 nPortIndex,
                                  OMX_BUFFERHEADERTYPE *pBuffHeader)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone, eTmpError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE      tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  nStartPortNumber = 0, nPorts = 0;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_FreeBuffer");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pBuffHeader != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    nPorts = pBaseComPvt->tDerToBase.nNumPorts;
    OMX_BASE_REQUIRE(nPortIndex >= nStartPortNumber && nPortIndex < nPorts,
                     OMX_ErrorBadPortIndex);

    pPort = pBaseComInt->ports[nPortIndex - nStartPortNumber];

    OMX_BASE_CHK_VERSION(pBuffHeader, OMX_BUFFERHEADERTYPE, eError);
    /*Free buffer should not be called on a port after all buffers have been
      freed*/
    OMX_BASE_ASSERT(pPort->nBufferCnt != 0, OMX_ErrorBadParameter);
    if( pBaseComPvt->pBufPreAnnouncePerPort[nPortIndex - nStartPortNumber]->
        bEnabled ) {
        /*This is an error in PA case. However, we'll continue cleanup so that
        there is no resource leak. An error will be returned at the end though*/
        if( pBuffHeader->pBuffer == NULL ) {
            OMX_BASE_Error("NULL buffer received! \
                Will continue cleanup inspite of this error");
            eTmpError = OMX_ErrorBadParameter;
        }
    }
    /* Just decrement the buffer count and unpopulate the port,
     * buffer header pool is freed up once all the buffers are received */
    pPort->nBufferCnt--;
    pPort->sPortDef.bPopulated = OMX_FALSE;
    if( pBaseComPvt->tCurState == OMX_StateIdle &&
        pBaseComPvt->tNewState == OMX_StateLoaded && pPort->sPortDef.bEnabled ) {
        /*Allowed on idle --> loaded transition if port is enabled*/
        OMX_BASE_Trace("FreeBuffer call is allowed");
    } else if((pPort->bIsTransitionToDisabled) &&
              (pBaseComPvt->tCurState != OMX_StateLoaded) &&
              (pBaseComPvt->tCurState != OMX_StateWaitForResources)) {
        /*Allowed during port disable if current state is not
        loaded/waitforresources*/
        OMX_BASE_Trace("FreeBuffer call is allowed");
    } else {
        eError = pBaseComInt->sAppCallbacks.EventHandler(hComponent,
                                                         pComp->pApplicationPrivate,
                                                         OMX_EventError, (OMX_U32)OMX_ErrorPortUnpopulated,
                                                         nPortIndex, " PortUnpopulated ");
        /*Ideally callback should never return error*/
        OMX_BASE_ASSERT(OMX_ErrorNone == eError, eError);
    }
    if( pPort->nBufferCnt == 0 ) {
        tStatus = TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent, BUF_FREE_EVENT,
                                     TIMM_OSAL_EVENT_OR);
        OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                        OMX_ErrorUndefined);
    }
EXIT:
    if((eTmpError != OMX_ErrorNone) && (eError == OMX_ErrorNone)) {
        /*An error occured earlier but we still continued cleanup to avoid leaks.
        Setting the return value to the error code now*/
        eError = eTmpError;
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_FreeBuffer", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_EmptyThisBuffer :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_EmptyThisBuffer(OMX_HANDLETYPE hComponent,
                                       OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  nPorts, nStartPortNumber;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_EmptyThisBuffer");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pBuffer != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nPorts = pBaseComPvt->tDerToBase.nNumPorts;
    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);
    /* check for valid port index */
    OMX_BASE_ASSERT(pBuffer->nInputPortIndex < (nPorts +
                                                nStartPortNumber), OMX_ErrorBadPortIndex);

    pPort = pBaseComInt->ports[pBuffer->nInputPortIndex - nStartPortNumber];
    if( pPort->sPortDef.eDir != OMX_DirInput ) {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }
    /*For buffer preannouncement disabled ports, these checks need to be done
      here*/
    if( !pBaseComPvt->pBufPreAnnouncePerPort[pBuffer->nInputPortIndex -
                                             nStartPortNumber]->bEnabled ) {
        /*Buffer size should be >= the minimum buffer size specified in
          port definition*/
        OMX_BASE_REQUIRE(pBuffer->nAllocLen >= pPort->sPortDef.nBufferSize,
                         OMX_ErrorBadParameter);
        if( pPort->sPortDef.nBufferAlignment != 0 ) {
            /*Return error if buffer is not correctly aligned*/
            OMX_BASE_REQUIRE(((OMX_U32)(pBuffer->pBuffer) %
                              pPort->sPortDef.nBufferAlignment) == 0,
                             OMX_ErrorBadParameter);
        }
    }
    /* This method is allowed only when the comp is in or a transition
         * to executing or pause state */
    OMX_BASE_ASSERT((pBaseComPvt->tCurState == OMX_StateIdle &&
                     pBaseComPvt->tNewState == OMX_StateExecuting) ||
                    (pBaseComPvt->tCurState == OMX_StateExecuting ||
                     pBaseComPvt->tCurState == OMX_StatePause),
                    OMX_ErrorIncorrectStateOperation);
    /*Following two checks are based on the 1.1.2 AppNote*/
    /*Supplier ports can accept buffers even if current state is disabled
      if they are transitioning from port enable to disable*/
    if( pPort->sPortDef.bEnabled != OMX_TRUE ) {
        if((!pPort->bIsTransitionToDisabled) ||
           (!PORT_IS_SUPPLIER(pPort))) {
            eError = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }
    }
    /*Non-supplier ports can't accept buffers when transitioning to Idle
      or when port is being transitioned to disabled*/
    if( !PORT_IS_SUPPLIER(pPort)) {
        if((pBaseComPvt->tNewState == OMX_StateIdle) ||
           (pPort->bIsTransitionToDisabled)) {
            eError = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }
    }
    eError = OMX_BASE_DIO_Queue(hComponent, pBuffer->nInputPortIndex, pBuffer);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    /*If another buffer comes after eos then reset the variable that causes
    watermark to become meaningless on this port*/
    if( pPort->bEosRecd == OMX_TRUE ) {
        pPort->bEosRecd = OMX_FALSE;
    }
    /*If EOS buffer or CodecConfig buffer then force notify to derived component*/
    if( pBuffer->nFlags & OMX_BUFFERFLAG_EOS ) {
        pPort->bEosRecd = OMX_TRUE;
    }
    eError = pBaseComInt->fpInvokeProcessFunction(hComponent, DATAEVENT);

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_EmptyThisBuffer", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_FillThisBuffer :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_FillThisBuffer(OMX_HANDLETYPE hComponent,
                                      OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  nPorts, nStartPortNumber;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_FillThisBuffer");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pBuffer != NULL), OMX_ErrorBadParameter);
    OMX_BASE_REQUIRE(pBuffer->pBuffer != NULL, OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nPorts = pBaseComPvt->tDerToBase.nNumPorts;
    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);
    /* check for valid port index */
    OMX_BASE_ASSERT(pBuffer->nOutputPortIndex < (nPorts +
                                                 nStartPortNumber), OMX_ErrorBadPortIndex);

    pPort = pBaseComInt->ports[pBuffer->nOutputPortIndex - nStartPortNumber];
    if( pPort->sPortDef.eDir != OMX_DirOutput ) {
        eError = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }
    /*For buffer preannouncement disabled ports, these checks need to be done
      here*/
    if( !pBaseComPvt->pBufPreAnnouncePerPort[pBuffer->nOutputPortIndex -
                                             nStartPortNumber]->bEnabled ) {
        /*Buffer size should be >= the minimum buffer size specified in
          port definition*/
        OMX_BASE_REQUIRE(pBuffer->nAllocLen >= pPort->sPortDef.nBufferSize,
                         OMX_ErrorBadParameter);
        if( pPort->sPortDef.nBufferAlignment != 0 ) {
            /*Return error if buffer is not correctly aligned*/
            OMX_BASE_REQUIRE(((OMX_U32)(pBuffer->pBuffer) %
                              pPort->sPortDef.nBufferAlignment) == 0,
                             OMX_ErrorBadParameter);
        }
    }
    /* This method is allowed only when the comp is in or a transition
         * to executing or pause state */
    OMX_BASE_ASSERT((pBaseComPvt->tCurState == OMX_StateIdle &&
                     pBaseComPvt->tNewState == OMX_StateExecuting) ||
                    (pBaseComPvt->tCurState == OMX_StateExecuting ||
                     pBaseComPvt->tCurState == OMX_StatePause),
                    OMX_ErrorIncorrectStateOperation);
    /*Following two checks are based on the 1.1.2 AppNote*/
    /*Supplier ports can accept buffers even if current state is disabled
      if they are transitioning from port enable to disable*/
    if( pPort->sPortDef.bEnabled != OMX_TRUE ) {
        if((!pPort->bIsTransitionToDisabled) ||
           (!PORT_IS_SUPPLIER(pPort))) {
            eError = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }
    }
    /*Non-supplier ports can't accept buffers when transitioning to Idle
      or when port is being transitioned to disabled*/
    if( !PORT_IS_SUPPLIER(pPort)) {
        if((pBaseComPvt->tNewState == OMX_StateIdle) ||
           (pPort->bIsTransitionToDisabled)) {
            eError = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }
    }
    eError = OMX_BASE_DIO_Queue(hComponent, pBuffer->nOutputPortIndex, pBuffer);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    eError = pBaseComInt->fpInvokeProcessFunction(hComponent, DATAEVENT);

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_FillThisBuffer", eError);
    return (eError);
}

/*===============================================================*/
/** @fn OMX_BASE_ComponentTunnelRequest :
 *   see omx_base.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_ComponentTunnelRequest(OMX_HANDLETYPE hComponent,
                                              OMX_U32 nPort,
                                              OMX_HANDLETYPE hTunneledComp,
                                              OMX_U32 nTunneledPort,
                                              OMX_TUNNELSETUPTYPE *pTunnelSetup)
{
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE              *pComp = NULL;
    OMX_BASE_PRIVATETYPE           *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE          *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE              *pPort = NULL;
    OMX_PARAM_BUFFERSUPPLIERTYPE    sBufferSupplier;
    OMX_U32                         nStartPortNumber;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_ComponentTunnelRequest");
    OMX_BASE_REQUIRE((hComponent != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    pPort = pBaseComInt->ports[nPort - nStartPortNumber];

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);
    if( pTunnelSetup == NULL || hTunneledComp == NULL ) {
        /* cancel previous tunnel */
        pPort->hTunnelComp = NULL;
        pPort->nTunnelPort = 0;
        pPort->nTunnelFlags = OMX_BASE_DIO_NONTUNNELED;
        /* TODO: Change this so that it takes the supplier settings from the derived
                   component*/
        pPort->eSupplierSetting = OMX_BufferSupplyUnspecified;
    } else {
        pPort->hTunnelComp = hTunneledComp;
        pPort->nTunnelPort = nTunneledPort;
        /* if output, return the supplier preference */
        if( pPort->sPortDef.eDir == OMX_DirOutput ) {
            /*Incase o/p has supplier unspecified then internally it assumes
            that o/p is supplier even though it sends back supplier
            unspecified. If i/p port wants to be supplier then it has to do
            a set parameter otherwise o/p port will assume that it is the
            supplier*/
            pPort->nTunnelFlags |= OMX_BASE_DIO_TUNNELED;
            pTunnelSetup->eSupplier = pPort->eSupplierSetting;
            if( pPort->eSupplierSetting != OMX_BufferSupplyInput ) {
                pPort->nTunnelFlags |= OMX_BASE_DIO_SUPPLIER;
            }
        } else {
            /* check for data compatablity b/w tunneled ports */
            eError = _OMX_BASE_VerifyTunnelConnection(pPort, hTunneledComp);
            OMX_BASE_ASSERT(OMX_ErrorNone == eError,
                            OMX_ErrorPortsNotCompatible);
            if((pPort->eSupplierSetting != pTunnelSetup->eSupplier) &&
               (pPort->eSupplierSetting != OMX_BufferSupplyUnspecified)) {
                pTunnelSetup->eSupplier = pPort->eSupplierSetting;
                /*Communicate i/p port supplier preferences to o/p port*/
                OMX_BASE_INIT_STRUCT_PTR(&sBufferSupplier,
                                         OMX_PARAM_BUFFERSUPPLIERTYPE);
                sBufferSupplier.nPortIndex =  nTunneledPort;
                sBufferSupplier.eBufferSupplier = pPort->eSupplierSetting;

                eError = OMX_SetParameter(hTunneledComp,
                                          OMX_IndexParamCompBufferSupplier,
                                          &sBufferSupplier);
                OMX_BASE_ASSERT(eError == OMX_ErrorNone,
                                OMX_ErrorPortsNotCompatible);
            } else if((OMX_BufferSupplyUnspecified == pPort->eSupplierSetting) &&
                      (OMX_BufferSupplyUnspecified == pTunnelSetup->eSupplier)) {
                pPort->eSupplierSetting = pTunnelSetup->eSupplier =
                                              OMX_BufferSupplyOutput;
                /*Communicate supplier preferences to o/p port*/
                OMX_BASE_INIT_STRUCT_PTR(&sBufferSupplier,
                                         OMX_PARAM_BUFFERSUPPLIERTYPE);
                sBufferSupplier.nPortIndex =  nTunneledPort;
                sBufferSupplier.eBufferSupplier = pPort->eSupplierSetting;

                eError = OMX_SetParameter(hTunneledComp,
                                          OMX_IndexParamCompBufferSupplier,
                                          &sBufferSupplier);
                OMX_BASE_ASSERT(eError == OMX_ErrorNone,
                                OMX_ErrorPortsNotCompatible);
            } else {
                pPort->eSupplierSetting = pTunnelSetup->eSupplier;
            }
            if( pPort->eSupplierSetting == OMX_BufferSupplyInput ) {
                pPort->nTunnelFlags = OMX_BASE_DIO_SUPPLIER;
            }
            pPort->nTunnelFlags |= OMX_BASE_DIO_TUNNELED;
        }
        /* establish Tunnel(Standard, FrameQ, FrameQRPC b/w tunneled ports*/
        eError = _OMX_BASE_TunnelEstablish(hComponent, nPort, hTunneledComp,
                                           nTunneledPort);
        OMX_BASE_ASSERT(eError == OMX_ErrorNone,
                        OMX_ErrorPortsNotCompatible);
    }
EXIT:
    if((OMX_ErrorNone != eError) && (pPort != NULL)) {
        pPort->hTunnelComp = NULL;
        pPort->nTunnelPort = 0;
        pPort->eSupplierSetting = OMX_BufferSupplyUnspecified;
    }
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_ComponentTunnelRequest", eError);
    return (eError);
}

/*==============================================================*/
/** @fn OMX_BASE_SetConfig :
 *   see omx_base.h
 */
/*==============================================================*/
OMX_ERRORTYPE OMX_BASE_SetConfig(OMX_HANDLETYPE hComponent,
                                 OMX_INDEXTYPE nIndex,
                                 OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE             eError = OMX_ErrorNone;
    TIMM_OSAL_ERRORTYPE       tStatus = TIMM_OSAL_ERR_NONE;
    OMX_COMPONENTTYPE        *pComp = NULL;
    OMX_BASE_PRIVATETYPE     *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE    *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE        *pPort = NULL;
    OMX_CONFIG_CHANNELNAME   *pChannelParams = NULL;
    OMX_U32                   nStartPortNumber, nPortIndex, nPorts;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetConfig");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pComponentConfigStructure != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    nPorts = pBaseComPvt->tDerToBase.nNumPorts;

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);

    switch( nIndex ) {
        /* Called only to close the channel */
        case (OMX_INDEXTYPE)OMX_TI_IndexConfigChannelName :
            OMX_BASE_CHK_VERSION(pComponentConfigStructure,
                                 OMX_CONFIG_CHANNELNAME, eError);
            pChannelParams = (OMX_CONFIG_CHANNELNAME *)
                             pComponentConfigStructure;

            nPortIndex = pChannelParams->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts, OMX_ErrorBadPortIndex);
            pPort = (OMX_BASE_PORTTYPE *)pBaseComInt->ports[nPortIndex];
            if( strcmp(pChannelParams->cChannelName, "PropTunClose") == 0 ) {
                /*Release sem so that non-supplier can close*/
                tStatus = TIMM_OSAL_EventSet(pPort->pBufAllocFreeEvent,
                                             BUF_FREE_EVENT, TIMM_OSAL_EVENT_OR);
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                                OMX_ErrorUndefined);
                /*Supplier is now pending on this sem which will be released
                  only when non-supplier closes*/
                tStatus = TIMM_OSAL_SemaphoreObtain(pPort->pDioOpenCloseSem,
                                                    TIMM_OSAL_SUSPEND);
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                                OMX_ErrorUndefined);
            } else {
                TIMM_OSAL_Memcpy(pPort->cChannelName,
                                 pChannelParams->cChannelName, OMX_BASE_MAXNAMELEN);
                /*Release sem so Non-Supplier can now complete open*/
                tStatus = TIMM_OSAL_SemaphoreRelease(pPort->pDioOpenCloseSem);
                OMX_BASE_ASSERT(TIMM_OSAL_ERR_NONE == tStatus,
                                OMX_ErrorUndefined);
            }
            break;
        case (OMX_INDEXTYPE)OMX_TI_IndexConfigBufferRefCountNotification :
            OMX_BASE_CHK_VERSION(pComponentConfigStructure,
                                 OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE, eError);
            nPortIndex = ((OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE *)
                          pComponentConfigStructure)->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts,
                            OMX_ErrorBadPortIndex);
            pPort = (OMX_BASE_PORTTYPE *)(pBaseComInt->ports[nPortIndex]);
            pPort->sBufRefNotify = *(OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE *)
                                   pComponentConfigStructure;
            break;
        default :
            eError = OMX_ErrorUnsupportedIndex;
            break;
    }

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_SetConfig", eError);
    return (eError);
}

/*==============================================================*/
/** @fn OMX_BASE_GetConfig :
 *   see omx_base.h
 */
/*==============================================================*/
OMX_ERRORTYPE OMX_BASE_GetConfig(OMX_HANDLETYPE hComponent,
                                 OMX_INDEXTYPE nIndex,
                                 OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE            eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE       *pComp = NULL;
    OMX_BASE_PRIVATETYPE    *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE   *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE       *pPort = NULL;
    OMX_U32                  nStartPortNumber, nPortIndex, nPorts;

    TIMM_OSAL_EnteringExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetConfig");
    OMX_BASE_REQUIRE((hComponent != NULL) &&
                     (pComponentConfigStructure != NULL), OMX_ErrorBadParameter);

    pComp = (OMX_COMPONENTTYPE *)hComponent;
    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;

    nStartPortNumber = pBaseComPvt->tDerToBase.nMinStartPortIndex;
    nPorts = pBaseComPvt->tDerToBase.nNumPorts;

    /*Can't be invoked when the comp is in invalid state*/
    OMX_BASE_REQUIRE(OMX_StateInvalid != pBaseComPvt->tCurState,
                     OMX_ErrorInvalidState);

    switch( nIndex ) {
        case (OMX_INDEXTYPE)OMX_TI_IndexConfigBufferRefCountNotification :
            OMX_BASE_CHK_VERSION(pComponentConfigStructure,
                                 OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE, eError);
            nPortIndex = ((OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE *)
                          pComponentConfigStructure)->nPortIndex - nStartPortNumber;
            /* check for valid port index */
            OMX_BASE_ASSERT(nPortIndex < nPorts,
                            OMX_ErrorBadPortIndex);
            pPort = (OMX_BASE_PORTTYPE *)(pBaseComInt->ports[nPortIndex]);
            *(OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE *)pComponentConfigStructure
                = (pPort->sBufRefNotify);
            break;
        default :
            eError = OMX_ErrorUnsupportedIndex;
            break;
    }

EXIT:
    TIMM_OSAL_ExitingExt(TIMM_OSAL_TRACEGRP_OMXBASE, "OMX_BASE_GetConfig", eError);
    return (eError);
}

/*==============================================================*/
/*         UN I M P L E M E N T E D       A P I'S               */
/*==============================================================*/

/*==============================================================*/
/** @fn OMX_BASE_UseEGLImage :
 *   see omx_base.h
 */
/*==============================================================*/
OMX_ERRORTYPE OMX_BASE_UseEGLImage(OMX_HANDLETYPE hComponent,
                                   OMX_BUFFERHEADERTYPE * *ppBufferHdr,
                                   OMX_U32 nPortIndex,
                                   OMX_PTR pAppPrivate,
                                   void *eglImage)
{
    return (OMX_ErrorNotImplemented);
}

/*==============================================================*/
/** @fn OMX_BASE_GetExtensionIndex :
 *   see omx_base.h
 */
/*==============================================================*/
OMX_ERRORTYPE OMX_BASE_GetExtensionIndex(OMX_HANDLETYPE hComponent,
                                         OMX_STRING cParameterName,
                                         OMX_INDEXTYPE *pIndexType)
{
    return (OMX_ErrorNotImplemented);
}

/*==============================================================*/
/** @fn OMX_BASE_ComponentRoleEnum :
 *   see omx_base.h
 */
/*==============================================================*/
OMX_ERRORTYPE OMX_BASE_ComponentRoleEnum(OMX_HANDLETYPE hComponent,
                                         OMX_U8 *cRole,
                                         OMX_U32 nIndex)
{
    return (OMX_ErrorNotImplemented);
}

