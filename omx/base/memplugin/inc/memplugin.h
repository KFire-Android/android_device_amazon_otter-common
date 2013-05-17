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

 /* ----- system and platform files ----------------------------*/
#ifndef MEMPLUGIN_H
#define MEMPLUGIN_H
#include <string.h>

#include "OMX_TI_Core.h"
#include <timm_osal_trace.h>

#define MEMPLUGIN_ERROR(fmt,...)  TIMM_OSAL_Error(fmt, ##__VA_ARGS__)
#define MEMPLUGIN_WARN(fmt,...)   TIMM_OSAL_Warning(fmt, ##__VA_ARGS__)
#define MEMPLUGIN_PROF(fmt,...)   TIMM_OSAL_Profiling(fmt, ##__VA_ARGS__)
#define MEMPLUGIN_INFO(fmt,...)   TIMM_OSAL_Info(fmt, ##__VA_ARGS__)
#define MEMPLUGIN_DEBUG(fmt,...)  TIMM_OSAL_Debug(fmt, ##__VA_ARGS__)
#define MEMPLUGIN_ENTER(fmt,...)  TIMM_OSAL_Entering(fmt, ##__VA_ARGS__)
#define MEMPLUGIN_EXIT(fmt,...)   TIMM_OSAL_Exiting(fmt, ##__VA_ARGS__)

/***************************************************************
 * PART 1
 * PLUGIN RELATED TYPE DEFINITIONS
 *
 *
 * *************************************************************
 */
typedef enum MEMPLUGIN_ERRORTYPE
{
     MEMPLUGIN_ERROR_NONE = 0x00,
     MEMPLUGIN_ERROR_BADPARAMETER,
     MEMPLUGIN_ERROR_NORESOURCES,
     MEMPLUGIN_ERROR_UNDEFINED,
     MEMPLUGIN_ERROR_NOTIMPLEMENTED
}MEMPLUGIN_ERRORTYPE;

typedef enum MEMPLUGIN_BUFFERTYPE
{
   DEFAULT = 0x00, //NON TILER
   TILER1D,
   TILER2D
}MEMPLUGIN_BUFFERTYPE;

typedef enum MEMPLUGIN_TILER_FORMAT
{
    MEMPLUGIN_TILER_FORMAT_8BIT = 0,
    MEMPLUGIN_TILER_FORMAT_16BIT = 1,
    MEMPLUGIN_TILER_FORMAT_32BIT = 2,
    MEMPLUGIN_TILER_FORMAT_PAGE = 3
}MEMPLUGIN_TILER_FORMAT;
/**
 * MEMPLUGIN_BUFFER_ACCESSOR: This structure maintaines all possible accessors for an allocated buffer
 *
 * @param pBufferHandle:    This is a pointer to buffer handle
 * @param bufferFd:        This is the context free file descriptor obtained by mapping
 * @param pRegBufferHandle:    This is the handle obtained after registration of the buffer with rpmsg
 * @param pBufferMappedAddress:    This is the mapped address of the buffer obtained after mapping
 *
 */
 //TBD: if pRegBufferHandle can be part of this structure - it is kind of ION specific
typedef struct MEMPLUGIN_BUFFER_ACCESSOR
{
    OMX_PTR pBufferHandle;
    OMX_U32 bufferFd;
    OMX_PTR pRegBufferHandle;
    OMX_PTR pBufferMappedAddress;
}MEMPLUGIN_BUFFER_ACCESSOR;

//Default is NULL
typedef struct MEMPLUGIN_INIT_PARAMS {
    void *hHeapHandle;
}MEMPLUGIN_INIT_PARAMS;

typedef enum MEMPLUGIN_ACCESS_MODETYPE {
    /*For default memory (non tiler)*/
    MEMPLUGIN_ACCESS_MODE_DontCare = 0,
    MEMPLUGIN_ACCESS_MODE_8Bit,
    MEMPLUGIN_ACCESS_MODE_16Bit,
    MEMPLUGIN_ACCESS_MODE_32Bit
}MEMPLUGIN_ACCESS_MODETYPE;

// @struct MEMPLUGIN_BUFFER_INPARAMS - comprises of all buffer properties required as input for Plugin APIs
typedef struct MEMPLUGIN_BUFFER_PARAMS
{
    OMX_U32 nHeight;
    OMX_U32 nWidth;
    OMX_BOOL bMap;
    MEMPLUGIN_TILER_FORMAT eTiler_format; //expected to be passed from user for tiler1D/2D == in case of nontiler to tiler upgrade, defaulted to TILER_PIXEL_FMT_PAGE
    MEMPLUGIN_BUFFERTYPE eBuffer_type;
    MEMPLUGIN_ACCESS_MODETYPE eAccessMode;
    OMX_U32                  nAlignment;
    //Height to be 1 for 1D buffers
    OMX_U32 nLength;
}MEMPLUGIN_BUFFER_PARAMS;
// @struct MEMPLUGIN_BUFFER_OUTPARAMS - comprises of all buffer properties required as input for Plugin APIs
typedef struct MEMPLUGIN_BUFFER_PROPERTIES
{
    MEMPLUGIN_BUFFER_ACCESSOR sBuffer_accessor;
    OMX_U32 nStride;
}MEMPLUGIN_BUFFER_PROPERTIES;

/***************************************************************
 * PART 2
 * PLUGIN INTERFACE RELATED TYPE AND METHOD DEFINITIONS
 *
 *
 * *************************************************************
 */

/** @struct MEMPLUGIN_OBJECT - comprises of all plugin related data including:
 *
 * pPluginExtendedInfo - plugin specific extended data structure
 * fpOpen - function pointer interface to Plugin Open()
 * fpAlloc - function pointer interface to Plugin Allocation method
 * fpFree - function pointer interface to Plugin Freeing method
 * fpClose - function pointer interface to Plugin Close()
 *
 */
typedef struct MEMPLUGIN_OBJECT
{
    OMX_PTR pPluginExtendedInfo;
    MEMPLUGIN_ERRORTYPE (*fpConfig)(void *pMemPluginHandle,
                                    void *pConfigData);
    MEMPLUGIN_ERRORTYPE (*fpOpen)(void *pMemPluginHandle,
                                    OMX_U32 *pClient);
    MEMPLUGIN_ERRORTYPE (*fpAlloc)(void *pMemPluginHandle,
                                    OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp);
    MEMPLUGIN_ERRORTYPE (*fpFree)(void *pMemPluginHandle,
                                    OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp);
    MEMPLUGIN_ERRORTYPE (*fpClose)(void *pMemPluginHandle,
                                    OMX_U32 nClient);
    MEMPLUGIN_ERRORTYPE (*fpDeInit)(void *pMemPluginHandle);
}MEMPLUGIN_OBJECT;

typedef MEMPLUGIN_ERRORTYPE (*MEMPLUGIN_CONFIGTYPE)(MEMPLUGIN_OBJECT **);

typedef struct MEMPLUGIN_TABLETYPE
{
    char cMemPluginName[50];
    MEMPLUGIN_CONFIGTYPE pMemPluginConfig;
}MEMPLUGIN_TABLETYPE;

/******************************************************************
 *   FUNCTIONS DEFINITION
 ******************************************************************/
 /*MemPlugin_Init: To initialize a MemPlugin from DOMX PROXY
  *                Must be called before any other API calls.
  *                Maps the plugin handle to corresponding proxy
  *                element for further access
  *
  * @param cMemPluginName: string that identifies plugin
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Init(char *cMemPluginName,
                                            void **pMemPluginHandle);
 /*MemPlugin_Configure: To do different types of plugin specific
  *                     config operations required.
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param pConfigData: data specific to configuration to be done
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Configure(void *pMemPluginHandle,
                                            void *pConfigData);
 /*MemPlugin_Open: To open and obtain a client for the MemPlugin
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param pClient: MemPlugin client used for operations
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Open(void *pMemPluginHandle,
                                    OMX_U32 *pClient);
 /*MemPlugin_Close: To close the client of MemPlugin
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Close(void  *pMemPluginHandle,
                                        OMX_U32 nClient);
 /*MemPlugin_Alloc: To allocate a buffer
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  * @param pBufferParams: All buffer IN params required for allocation
  * @param pBufferProp: All buffer properties obtained from allocation
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Alloc(void *pMemPluginHandle,
                                        OMX_U32 nClient,
                                        MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                        MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp);
 /*MemPlugin_Free: To free a buffer
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  * @param pBufferParams: All buffer IN params required for freeing
  * @param pBufferProp: All buffer properties required for freeing
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_Free(void *pMemPluginHandle,
                                    OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp);
 /*MemPlugin_DeInit: To destroy a MemPlugin from DOMX PROXY
  *
  * @param pMemPluginHandle: handle that provides identifies the
  *                          plugin to be destroyed
  */
 MEMPLUGIN_ERRORTYPE MemPlugin_DeInit(void *pMemPluginHandle);
#endif
