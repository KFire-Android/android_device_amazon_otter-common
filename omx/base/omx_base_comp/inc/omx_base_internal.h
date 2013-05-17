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

#ifndef _OMX_BASE_INTERNAL_H_
#define _OMX_BASE_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <omx_base.h>
#include <memplugin.h>

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

#define CMDEVENT  (0x00000100)

#define DATAEVENT (0x00000200)

#define ENDEVENT  (0x00f00000)

#define OMX_BASE_CmdStateSet (0x00001000)

#define OMX_BASE_CmdPortEnable (0x00002000)

#define OMX_BASE_CmdPortDisable (0x00003000)

#define OMX_BASE_CmdFlush (0x00004000)

#define OMX_BASE_CmdMarkBuffer  (0x00005000)

#define BUF_ALLOC_EVENT (0x00000001)

#define BUF_FREE_EVENT (0x00000002)

#define BUF_FAIL_EVENT (0x00000004)

#define PORT_IS_SUPPLIER(pPort)   \
    (pPort->nTunnelFlags & OMX_BASE_DIO_SUPPLIER)

#define PORT_IS_TUNNELED(pPort) \
    (pPort->nTunnelFlags & OMX_BASE_DIO_TUNNELED)

#define PORT_IS_NONSUPPLIER(pPort) \
    (pPort->nTunnelFlags & OMX_BASE_DIO_NONSUPPLIER)

#define STATE_LOADED_EVENT  (0x00000001)
#define STATE_IDLE_EVENT  (0x00000002)
#define STATE_EXEC_EVENT  (0x00000004)
#define STATE_PAUSE_EVENT  (0x00000008)
#define ERROR_EVENT  (0x00000010)
#define PORT_ENABLE_EVENT  (0x00000020)
#define PORT_DISABLE_EVENT  (0x00000040)

#define STATE_TRANSITION_TIMEOUT 500
#define STATE_TRANSITION_LONG_TIMEOUT 5000

/*Size of OMX_OTHER_EXTRADATATYPE struct. Has to be updated if the structure
changes. The actual size is 21 bytes of which 20 bytes consists of the header.
If there is data then that is appended to this 20 bytes. If there is no data
then there is 1 byte empty placeholder which has to be 4 byte aligned to some
padding will be added at the end*/
#define EXTRADATATYPE_SIZE 20

#define MEMPLUGIN_BUFFER_PARAMS_INIT(MEMPLUGIN_bufferinfo) do {\
        MEMPLUGIN_bufferinfo.eBuffer_type = DEFAULT;\
        MEMPLUGIN_bufferinfo.nHeight = 1;\
        MEMPLUGIN_bufferinfo.nWidth  = -1;\
        MEMPLUGIN_bufferinfo.bMap   = OMX_FALSE;\
        MEMPLUGIN_bufferinfo.eTiler_format = -1;\
} while(0)


/** The OMX_BASE_TUNNEL_CONFIGTYPE enumeration is used to
 *  sepcify the port type i.e Supplier, NonSupplier..
 */
typedef enum OMX_BASE_TUNNEL_CONFIGTYPE {

    OMX_BASE_DIO_NONTUNNELED = 0x1,
    OMX_BASE_DIO_NONSUPPLIER = 0x2,
    OMX_BASE_DIO_TUNNELED    = 0x4,
    OMX_BASE_DIO_SUPPLIER    = 0x8

}OMX_BASE_TUNNEL_CONFIGTYPE;


/** OMX_BASE_CMDPARAMS
 *  This structure contains the params required to execute command
 *  @param cmd       :  command to execute
 *  @param nParam    :  parameter for the command to be executed
 *  @param pCmdData  :  pointer to the command data
 */
typedef struct OMX_BASE_CMDPARAMS {

    OMX_COMMANDTYPE eCmd;
    OMX_U32         unParam;
    OMX_PTR         pCmdData;

}OMX_BASE_CMDPARAMS;

/** OMX_BASE_PORTTYPE :
 *  This structure contains the elements of a port
 *  @param hTunnelComp             :  handle of tunneled comp
 *  @param nTunnelFlags            :  tunnel flags
 *  @param nTunnelPort             :  tunneled port index
 *  @param bIsTransitionToDisabled :  is in transition to disabled
 *  @param bIsTransitionToEnabled  :  is in transition to enabled
 *  @param bIsFlushingBuffers      :  is in process of flushing
 *  @param sPortDefParams          :  port specific params
 *  @param eSupplierSetting        :  supplier setting of a port
 *  @param pBufferlist             :  list of buffers
 *  @param nBufferCnt              :  Buffer count index
 *  @param pBufAllocSem            :  Event for synchronising both buffer
 *                                    allocation and freeing up
 *  @param pDioOpenCloseSem        :  Semaphore to synchronise DIO open/close
 *  @param hDIO                    :  handle to access DIO object
 *  @param cChannelName            :  Channel name
 */
typedef struct OMX_BASE_PORTTYPE {

    OMX_HANDLETYPE                         hTunnelComp;
    OMX_U32                                nTunnelFlags;
    OMX_U32                                nTunnelPort;
    OMX_BOOL                               bIsTransitionToDisabled;
    OMX_BOOL                               bIsTransitionToEnabled;
    OMX_BOOL                               bIsFlushingBuffers;
    OMX_BOOL                               bEosRecd;
    OMX_PARAM_PORTDEFINITIONTYPE           sPortDef;
    OMX_BUFFERSUPPLIERTYPE                 eSupplierSetting;
    OMX_BUFFERHEADERTYPE                * *pBufferlist;
    OMX_U32                                nBufferCnt;
    OMX_PTR                                pBufAllocFreeEvent;
    OMX_PTR                                pDioOpenCloseSem;
    OMX_PTR                                hDIO;
    OMX_PTR                                pMemPluginHandle;
    OMX_U8                                 cChannelName[OMX_BASE_MAXNAMELEN];
    OMX_TI_CONFIG_BUFFERREFCOUNTNOTIFYTYPE sBufRefNotify;
    OMX_U32                                nMetaDataSize;
    OMX_U32                                nCachedNumBuffers;
    OMX_U32                                nMemmgrClientDesc;
    OMX_BOOL                               bMapBuffers;
    MEMPLUGIN_BUFFER_PARAMS                sBufferParams;
    MEMPLUGIN_BUFFER_PROPERTIES            sBufferProp;
}OMX_BASE_PORTTYPE;

/** OMX_BASE_INTERNALTYPE
 *  This structure contains the internals of a Base component
 *  @param ports                   : info related to ports
 *  @param sAppCallbacks           : Application callbacks
 *  @param pCmdPipe                : pointer to command pipe
 *  @param pThreadId               : pointer to thread
 *  @param pTriggerEvent           : pointer to trigger an Event
 *  @param pCmdCompleteEvent       : Command comletion Event
 *  @param pCmdDataPipe            : Pipe to queue up cmd data pointers which
 *                                   have been allocated by base.
 *  @param pCmdPipeMutex           : Used to synchronize r/w to cmd data pipe.
 *  @param pNewStateMutex          : Used to protec accesses to the NewState
 *                                   variable in base.
 *  @param fpInvokeProcessFunction : hook to process command and data
 */
typedef struct OMX_BASE_INTERNALTYPE {
    OMX_BASE_PORTTYPE * *ports;
    OMX_CALLBACKTYPE     sAppCallbacks;
    OMX_BOOL             bForceNotifyOnce;
    OMX_PTR              pCmdPipe;
    OMX_PTR              pThreadId;
    OMX_PTR              pTriggerEvent;
    OMX_PTR              pCmdCompleteEvent;
    OMX_PTR              pCmdDataPipe;
    OMX_PTR              pCmdPipeMutex;
    OMX_PTR              pNewStateMutex;
    OMX_PTR              pErrorCmdcompleteEvent;
    OMX_ERRORTYPE (*fpInvokeProcessFunction)(OMX_HANDLETYPE hComponent,
                                             OMX_U32 retEvent);
}OMX_BASE_INTERNALTYPE;


/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *      This function is called in both Active/passive context of
 *      a component which basically does process the command
 *      and data events
 *
 * @param hComponent  : handle of the component
 * @param retEvent    : Event to be processed
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_PROCESS_Events(OMX_HANDLETYPE hComponent,
                                      OMX_U32 retEvent);

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_TriggerEvent :
 *      This is executed only in Active context to trigger
 *      component thread
 *
 *  @param hComponent  : handle of component
 *  @param EventToSet  : event to be set
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_PROCESS_TriggerEvent(OMX_HANDLETYPE hComponent,
                                            OMX_U32 EventToSet);

/*===============================================================*/
/** @fn OMX_BASE_CompThreadEntry :
 *      Component Thread function entry to process
 */
/*===============================================================*/
void OMX_BASE_CompThreadEntry(void *arg);


/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_CB_ReturnEventNotify(OMX_HANDLETYPE hComponent,
                                            OMX_EVENTTYPE eEvent,
                                            OMX_U32 nData1, OMX_U32 nData2,
                                            OMX_PTR pEventData);

/*===============================================================*/
/** @fn _OMX_BASE_HandleStateTransition :
 *      To handle state transitions
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_HandleStateTransition(OMX_HANDLETYPE hComponent,
                                              OMX_U32 nParam);

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_InitializePorts(OMX_HANDLETYPE hComponent);

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_DeinitializePorts(OMX_HANDLETYPE hComponent);

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_TunnelEstablish(OMX_HANDLETYPE hComponent,
                                        OMX_U32 nPort,
                                        OMX_HANDLETYPE hTunnelComp,
                                        OMX_U32 nTunnPort);

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_DisablePort(OMX_HANDLETYPE hComponent,
                                    OMX_U32 nParam);

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_EnablePort(OMX_HANDLETYPE hComponent,
                                   OMX_U32 nParam);

/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_FlushBuffers(OMX_HANDLETYPE hComponent,
                                     OMX_U32 nParam);

/*===============================================================*/
/** @fn _OMX_BASE_GetChannelInfo :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_GetChannelInfo(OMX_BASE_PORTTYPE *pPort,
                                       OMX_STRING cChannelName);

/*===============================================================*/
/** @fn _OMX_BASE_NotifyClient :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_EventNotifyToClient(OMX_HANDLETYPE hComponent,
                                            OMX_COMMANDTYPE Cmd,
                                            OMX_U32 nParam,
                                            OMX_PTR pCmdData);

/*===============================================================*/
/** @fn _OMX_BASE_VerifyTunnelConnection :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_VerifyTunnelConnection(OMX_BASE_PORTTYPE *pPort,
                                               OMX_HANDLETYPE hTunneledComp);



OMX_S32 _OMX_BASE_FindInTable(OMX_EXTRADATATYPE eExtraDataType);

/*----------        DIO function prototypes      ------------------ */
/*===============================================================*/
/** @fn OMX_BASE_DIO_Init      : Initialize DIO
 *
 * @ param hComponent [in]     : Handle of the component.
 * @ param nPortIndex [in]     : Index of the port.
 * @ param cChannelType [in]   : Channel type.
 * @ param pCreateParams [in]  : Create time parameters.
 *                               Also see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Init (OMX_HANDLETYPE hComponent,
                                 OMX_U32 nPortIndex,
                                 OMX_STRING cChannelType,
                                 OMX_PTR pCreateParams);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Open      : Open DIO. Allocates buffers and buffer headers
 *                               depending on supplier/non-supplier.
 *
 * @ param hComponent [in]     : Handle of the component.
 * @ param nPortIndex [in]     : Index of the port.
 * @ param pOprnParams [in]    : Open parameters.
 *                               Also see omx_base_dio_plugin.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Open (OMX_HANDLETYPE hComponent,
                                 OMX_U32 nPortIndex,
                                 OMX_PTR pOpenParams);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Close     : Close DIO. Frees up buffers if supplier.
 *
 * @ param hComponent [in]     : Handle of the component.
 * @ param nPortIndex [in]     : Index of the port.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Close (OMX_HANDLETYPE hComponent,
                                  OMX_U32 nPortIndex);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Queue     : Queue up the buffer internally.
 *
 * @ param hComponent [in]     : Handle of the component.
 * @ param nPortIndex [in]     : Index of the port.
 * @ param pBuffHeader [in]    : Buffer header to be queued up.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Queue (OMX_HANDLETYPE hComponent,
                                  OMX_U32 nPortIndex,
                                  OMX_PTR pBuffHeader);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Dequeue    : Dequeue a buffer header from the internal
 *                                queue for processing. The caller provides
 *                                a local buffer header which the DIO layer
 *                                can read for certain parameters such as
 *                                buffer size required. Rest of the parameters
 *                                in the available header are then copied to
 *                                the local header to be used by the caller.
 *
 *
 * @ param hComponent [in]      : Handle of the component.
 * @ param nPortIndex [in]      : Index of the port.
 * @ param pBuffHeader [in out] : Buffer header.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Dequeue (OMX_HANDLETYPE hComponent,
                                    OMX_U32 nPortIndex,
                                    OMX_PTR pBuffHeader);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Send    : Send the buffer. The caller will provide  a
 *                             local buffer header which will be copied and the
 *                             copy will be sent across.
 *
 * @ param hComponent [in]   : Handle of the component.
 * @ param nPortIndex [in]   : Index of the port.
 * @ param pBuffHeader [in]  : Buffer header to be sent.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Send (OMX_HANDLETYPE hComponent,
                                 OMX_U32 nPortIndex,
                                 OMX_PTR pBuffHeader);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Cancel  : Cancel the buffer. The next dequeue call will
 *                             return the same buffer.
 *
 * @ param hComponent [in]   : Handle of the component.
 * @ param nPortIndex [in]   : Index of the port.
 * @ param pBuffHeader [in]  : Buffer header to be canceled.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Cancel (OMX_HANDLETYPE hComponent,
                                   OMX_U32 nPortIndex,
                                   OMX_PTR pBuffHeader);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Dup           : Duplicate a buffer header. Used when the
 *                                   caller wants to hold on to a buffer and
 *                                   share with others. The duplicate buffer
 *                                   header will be a local header provided by
 *                                   the caller. After returning, the caller
 *                                   can send the original buffer and retain
 *                                   the duplicate, to be canceled later.
 *
 * @ param hComponent [in]         : Handle of the component.
 * @ param nPortIndex [in]         : Index of the port.
 * @ param pBuffHeader [in]        : Buffer header to be duplicated.
 * @ param pDupBuffHeader [in out] : Duplicate buffer header.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Dup (OMX_HANDLETYPE hComponent,
                                OMX_U32 nPortIndex,
                                OMX_PTR pBuffHeader,
                                OMX_PTR pDupBuffHeader);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Control : Control call.
 *
 * @ param hComponent [in]   : Handle of the component.
 * @ param nPortIndex [in]   : Index of the port.
 * @ param nCmdType [in]     : Control command. See omx_base.h
 * @ param pParams [in]      : Any control parameters.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Control (OMX_HANDLETYPE hComponent,
                                    OMX_U32 nPortIndex,
                                    OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                                    OMX_PTR pParams);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Util    : Utility functions.
 *
 * @ param hComponent [in]   : Handle of the component.
 * @ param nPortIndex [in]   : Index of the port.
 * @ param nUtilType [in]    : Utility type. See omx_base.h
 * @ param pParams [in]      : Any parameters.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Util    (OMX_HANDLETYPE hComponent,
                                    OMX_U32 nPortIndex,
                                    OMX_BASE_DIO_UTIL_TYPE nUtilType,
                                    OMX_PTR pParams);



/*===============================================================*/
/** @fn OMX_BASE_DIO_GetCount : Get no. of buffers available.
 *
 * @ param hComponent [in]    : Handle of the component.
 * @ param nPortIndex [in]    : Index of the port.
 * @ param pCount [out]       : Count of the buffers available.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_GetCount (OMX_HANDLETYPE hComponent,
                                     OMX_U32 nPortIndex,
                                     OMX_U32 *pCount);


/*===============================================================*/
/** @fn OMX_BASE_DIO_Deinit   : Deinitialize DIO.
 *
 * @ param hComponent [in]    : Handle of the component.
 * @ param nPortIndex [in]    : Index of the port.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_DIO_Deinit (OMX_HANDLETYPE hComponent,
                                   OMX_U32 nPortIndex);

/*===============================================================*/
/** @fn OMX_BASE_Error_EventHandler   :
 *
 * This overrides the event handler callback in case of error
 */
/*===============================================================*/

OMX_ERRORTYPE OMX_BASE_Error_EventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                          OMX_EVENTTYPE eEvent, OMX_U32 nData1,
                                          OMX_U32 nData2, OMX_PTR pEventData);
/*===============================================================*/
/** @fn OMX_BASE_Error_FillBufferDone   :
 *
 * This overrides the FBD callback in case of error
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_Error_FillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                            OMX_BUFFERHEADERTYPE *pBuffer);
/*===============================================================*/
/** @fn OMX_BASE_Error_EmptyBufferDone   :
 *
 * This overrides the EBD callback in case of error
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_Error_EmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                             OMX_BUFFERHEADERTYPE *pBuffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_BASE_INTERNAL_H_ */

