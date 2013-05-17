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

#ifndef _OMX_BASE_UTILS_H_
#define _OMX_BASE_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <timm_osal_trace.h>
#include <timm_osal_types.h>

/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*----------         function prototypes      ------------------- */
/*----------         data declarations        ------------------- */
/*******************************************************************
 * PUBLIC DECLARATIONS: defined here, used elsewhere
 *******************************************************************/

/*----------         data declarations        ------------------- */

/*----------   Utility stuff like asserts, traces etc.   ------------------- */
#define OMX_BASE_Menu(fmt, ...) \
    TIMM_OSAL_MenuExt(TIMM_OSAL_TRACEGRP_OMXBASE, fmt, ##__VA_ARGS__)

#define OMX_BASE_Warn(fmt, ...) \
    TIMM_OSAL_WarningExt(TIMM_OSAL_TRACEGRP_OMXBASE, fmt, ##__VA_ARGS__)

#define OMX_BASE_Info(fmt, ...) \
    TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE, fmt, ##__VA_ARGS__)

#define OMX_BASE_Error(fmt, ...) \
    TIMM_OSAL_ErrorExt(TIMM_OSAL_TRACEGRP_OMXBASE, fmt, ##__VA_ARGS__)

#define OMX_BASE_Trace(fmt, ...) \
    TIMM_OSAL_InfoExt(TIMM_OSAL_TRACEGRP_OMXBASE, fmt, ##__VA_ARGS__)


#define OMX_BASE_Malloc(_nSize_, _hHeap_) \
    TIMM_OSAL_MallocExtn(_nSize_, TIMM_OSAL_TRUE, 0, TIMMOSAL_MEM_SEGMENT_EXT, _hHeap_)

#define OMX_BASE_MALLOC_STRUCT(_pStruct_, _sName_, _hHeap_) do { \
        _pStruct_ = (_sName_ *)OMX_BASE_Malloc(sizeof(_sName_), _hHeap_); \
        if( _pStruct_ == NULL ) { \
            OMX_BASE_Error(" Malloc failed "); \
            eError = OMX_ErrorInsufficientResources; \
            goto EXIT; } \
} while( 0 )

#define OMX_BASE_REQUIRE OMX_BASE_PARAMCHECK
#define OMX_BASE_ASSERT  OMX_BASE_PARAMCHECK
#define OMX_BASE_ENSURE  OMX_BASE_PARAMCHECK

#define OMX_BASE_PARAMCHECK(_COND_, _ERRORCODE_) do { \
        if( !(_COND_)) { eError = _ERRORCODE_; \
                         OMX_BASE_Error("Failed check: " # _COND_); \
                         OMX_BASE_Error("Returning error: " # _ERRORCODE_); \
                         goto EXIT; } \
} while( 0 )

#define OMX_BASE_CHK_VERSION(_pStruct_, _sName_, _e_) do { \
        if(((_sName_ *)_pStruct_)->nSize != sizeof(_sName_)) { \
            _e_ = OMX_ErrorBadParameter; \
            OMX_BASE_Error("Incorrect 'nSize' field. Returning OMX_ErrorBadParameter"); \
            goto EXIT; } \
        if((((_sName_ *)_pStruct_)->nVersion.s.nVersionMajor != 0x1) || \
           ((((_sName_ *)_pStruct_)->nVersion.s.nVersionMinor != 0x1) && \
           ((_sName_ *)_pStruct_)->nVersion.s.nVersionMinor != 0x0 )) { \
            _e_ = OMX_ErrorVersionMismatch; \
            OMX_BASE_Error("Version mismatch. Returning OMX_ErrorVersionMismatch"); \
            goto EXIT; } \
} while( 0 )


#define OMX_BASE_INIT_STRUCT_PTR(_pStruct_, _sName_) do { \
        TIMM_OSAL_Memset((_pStruct_), 0x0, sizeof(_sName_)); \
        (_pStruct_)->nSize = sizeof(_sName_); \
        (_pStruct_)->nVersion.s.nVersionMajor = 0x1; \
        (_pStruct_)->nVersion.s.nVersionMinor = 0x1; \
        (_pStruct_)->nVersion.s.nRevision     = 0x2; \
        (_pStruct_)->nVersion.s.nStep         = 0x0; \
} while( 0 )



/*----------   function prototypes.   ------------------- */

/*===============================================================*/
/** @fn OMX_BASE_GetUVBuffer    : In case of packed format for NV12 buffers
 *                                with 2D buffers, the
 *                                pBuffer field in the buffer header points to
 *                                only the Y buffer. Calling this function
 *                                returns the corresponding UV buffer.
 *
 *  @ params [IN] hComponent    : Component handle.
 *  @ params [IN] nPortIndex    : Port index.
 *  @ params [IN] pBufHdr       : Pointer to the buffer header.
 *  @ params [OUT] pUVBuffer    : Pointer to the UV buffer is filled here and
 *                                returned to the caller.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetUVBuffer(OMX_HANDLETYPE hComponent,
                                   OMX_U32 nPortIndex,
                                   OMX_PTR pBufHdr, OMX_PTR *pUVBuffer);



/*===============================================================*/
/** @fn OMX_BASE_IsDioReady    : This function can be used by derived components
 *                               to query whether DIO APIs can be called on this
 *                               port. A return value of TRUE indicates that DIO
 *                               APIs can be used on this port. This API will
 *                               return true once both DIO Init and Open have
 *                               been successfully called on the port and will
 *                               return false as soon as DIO Close is called.
 *
 *  @ params [IN] hComponent    : Component handle.
 *  @ params [IN] nPortIndex    : Port index.
 */
/*===============================================================*/
OMX_BOOL OMX_BASE_IsDioReady(OMX_HANDLETYPE hComponent, OMX_U32 nPortIndex);

/*===============================================================*/
/** @fn OMX_BASE_UtilCleanupIfError    : This function can be used by derived components
 *                               to bring the component to Loaded state in case of handling freehandle
 *                               command in any state.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_UtilCleanupIfError(OMX_HANDLETYPE hComponent);

/*===============================================================*/
/** @fn OMX_BASE_SetTaskPriority    : This function can be used by derived components to set Task Priority
 *                               of the Component at runtime. It returns the previous Task priority.
 *
 *  @ params [IN] hComponent    : Component handle.
 *  @ params [IN] uPriority        : Priority to be set.
 */
/*===============================================================*/
OMX_U32 OMX_BASE_SetTaskPriority(OMX_HANDLETYPE hComponent, OMX_U32 uPriority);


/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------          data declarations        ------------------- */
/*----------          function prototypes      ------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_BASE_UTILS_H_ */

