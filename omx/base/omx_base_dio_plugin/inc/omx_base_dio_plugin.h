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

#ifndef _OMX_BASE_DIO_PLUGIN_H
#define _OMX_BASE_DIO_PLUGIN_H

#ifdef _cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <OMX_Core.h>
#include <OMX_Component.h>
#include "omx_base.h"

/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*----------         function prototypes      ------------------- */
/*----------         data declarations        ------------------- */
/*******************************************************************
 * PUBLIC DECLARATIONS: defined here, used elsewhere
 *******************************************************************/
/*----------         data declarations        ------------------- */



/** The OMX_BASE_DIO_COMMANDTYPE enumeration is used to
 *  specify the action in the OMX_BASE_DIO_Control Macro
 */
/* This is commented out here so that DIO developers can check the definition
     from here. Actual definition is in onx_base.h */
/*
typedef enum OMX_BASE_DIO_CTRLCMD_TYPE
{
    OMX_BASE_DIO_CTRLCMD_Flush            = 0x1,
    OMX_BASE_DIO_CTRLCMD_Start            = 0x2,
    OMX_BASE_DIO_CTRLCMD_Stop             = 0x3,
    OMX_BASE_DIO_CTRLCMD_GetCtrlAttribute = 0x4,
    OMX_BASE_DIO_CTRLCMD_SetCtrlAttribute = 0x5,

    OMX_BASE_DIO_CTRLCMD_ExtnStart        = 0xA
}OMX_BASE_DIO_CTRLCMD_TYPE;
*/

/*Similarly for UTIL_TYPE*/
/*
typedef enum OMX_BASE_DIO_UTIL_TYPE
{
    OMX_BASE_DIO_UTIL_GetTotalBufferSize                      = 0x1,
    OMX_BASE_DIO_UTIL_UpdatePacketExtraDataPointer            = 0x2,

    OMX_BASE_DIO_UTIL_ExtnStart                               = 0xA
}OMX_BASE_DIO_UTIL_TYPE;
*/


/** The OMX_BASE_DIO_OPENMODETYPE enumeration is used to
 *  sepcify the open mode type i.e reader or writer
 */
typedef enum OMX_BASE_DIO_OPENMODETYPE {

    OMX_BASE_DIO_READER  =  0x0,
    OMX_BASE_DIO_WRITER  =  0x1

}OMX_BASE_DIO_OPENMODETYPE;



typedef OMX_ERRORTYPE (*OMX_BASE_DIO_INITTYPE)(OMX_IN OMX_HANDLETYPE hComponent,
                                               OMX_IN OMX_PTR pCreateParams);

/** OMX_BASE_DIOREGISTERTYPE  :
 *  This structure contains the params used to register the DIO object
 *  @param pChannelName  :  Channel Name
 *  @param pInitialize   :  DIO instace initialization function
 */
typedef struct OMX_BASE_DIO_REGISTERTYPE {

    const char           *cChannelType;
    OMX_BASE_DIO_INITTYPE pInitialize;

}OMX_BASE_DIO_REGISTERTYPE;

OMX_BASE_DIO_REGISTERTYPE    OMX_BASE_DIO_Registered[];

/** OMX_BASE_DIO_CreateParams :
 *  This structure contains the parameters required to
 *  create DIO object
 *
 *  @param cChannelName      :  channel name
 *  @param hComponent        :  Handle of Component
 *  @param hTunComponent     :  Handle of Tunneled Component
 *  @param nPortIndex        :  Index of the port
 *  @param nTunPortIndex     :  Index of Tunneled Port
 *  @param pAppCallbacks     :  Application callbacks
 *  @param cBufAllocatorName :  Name of memory segment to allocate buffers
 *  @param cHdrAllocatorName :  Name of memroy segment to allocate headers
 *  @param pExtendedParams :  Ptr for extra parameters which might be required
 *                                             by other DIO types
 */
typedef struct OMX_BASE_DIO_CreateParams {

    char               cChannelName[OMX_BASE_MAXNAMELEN];
    OMX_HANDLETYPE     hComponent;
    OMX_BASE_PORTTYPE *pPort;
    OMX_CALLBACKTYPE  *pAppCallbacks;
    char               cBufAllocatorName[OMX_BASE_MAXNAMELEN];
    char               cHdrAllocatorName[OMX_BASE_MAXNAMELEN];
    OMX_PTR            pExtendedParams;

}OMX_BASE_DIO_CreateParams;


/** OMX_BASE_DIO_OpenParams :
 *  This structure contains the open parameters for DIO object
 *  @param nMode           : open mode reader or writer
 *  @param bCacheFlag      : cache access flag - true if buffer is accessed via
 *                           processor/cache
 *  @param nBufSize        : used by non-tunnel open as allocate buffer call
 *                           can specify a different size
 */
typedef struct OMX_BASE_DIO_OpenParams {

    OMX_U32  nMode;
    OMX_BOOL bCacheFlag;
    OMX_U32  nBufSize;

}OMX_BASE_DIO_OpenParams;


/** OMX_BASE_DIO_OBJECTTYPE :
 *  This structure contains the params and interface to access the DIO object
 *
 * @param pDIOPrivate      :  pointer to the DIO private data area
 * @param (*open)          :  DIO object open Implementation
 * @param (*close)         :  DIO object close Implementation
 * @param (*queue)         :  DIO object queue Implementation
 * @param (*dequeue)       :  DIO object dequeu Implementation
 * @param (*send)          :  DIO object send Implementation
 * @param (*cancel)        :  DIO object cancel Implementation
 * @param (*dup)           :  DIO object dup Implementation
 * @param (*control)       :  DIO object control Implementation
 * @param (*util)          :  DIO object utility Implementation
 * @param (*getcount)      :  DIO object getcount Implementation
 * @param (*deinit)        :  DIO object deinit Implementation
 * @param bOpened          :  Indicates whether DIO has been opened on this port
 */
typedef struct OMX_BASE_DIO_OBJECTTYPE {

    OMX_PTR pDIOPrivate;

    OMX_ERRORTYPE (*open)(OMX_HANDLETYPE handle,
                          OMX_BASE_DIO_OpenParams *pParams);

    OMX_ERRORTYPE (*close)(OMX_HANDLETYPE handle);

    OMX_ERRORTYPE (*queue)(OMX_HANDLETYPE handle,
                           OMX_PTR pBuffHeader);

    OMX_ERRORTYPE (*dequeue)(OMX_HANDLETYPE handle,
                             OMX_PTR pBuffHeader);

    OMX_ERRORTYPE (*send)(OMX_HANDLETYPE handle,
                          OMX_PTR pBuffHeader);

    OMX_ERRORTYPE (*cancel)(OMX_HANDLETYPE handle,
                            OMX_PTR pBuffHeader);

    OMX_ERRORTYPE (*dup)(OMX_HANDLETYPE handle,
                         OMX_PTR pBuffHeader,
                         OMX_PTR pDupBuffHeader);

    OMX_ERRORTYPE (*control)(OMX_HANDLETYPE handle,
                             OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,
                             OMX_PTR pParams);

    OMX_ERRORTYPE (*util)(OMX_HANDLETYPE handle,
                          OMX_BASE_DIO_UTIL_TYPE nUtilType,
                          OMX_PTR pParams);

    OMX_ERRORTYPE (*getcount)(OMX_HANDLETYPE handle,
                              OMX_U32 *pCount);

    OMX_ERRORTYPE (*deinit)(OMX_HANDLETYPE handle);

    OMX_BOOL bOpened;

}OMX_BASE_DIO_OBJECTTYPE;



/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------          data declarations        ------------------- */
/*----------          function prototypes      ------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
/* File EOF */

