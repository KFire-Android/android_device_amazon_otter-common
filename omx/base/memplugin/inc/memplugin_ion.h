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

#include <unistd.h>
#include <ion_ti/ion.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <linux/rpmsg_omx.h>
#include <errno.h>

#include "memplugin.h"

/* struct MEMPLUGIN_ION_PARAMS : contains all params that are specific to
 *                               ION APIs for buffer allocation.
 * @param nAlign: alignment of buffers
 * @param nOffset: offset needed for buffers
 * @param alloc_flags: allocation flags depending on type of buffer needed
 * @param map_flags: mapping flags depending on type of mapping
 * @param prot: buffer protection flag
 */
typedef struct MEMPLUGIN_ION_PARAMS {
    size_t nAlign;
    off_t nOffset;
    OMX_U32 alloc_flags;
    OMX_S32 map_flags;
    OMX_S32 prot;
}MEMPLUGIN_ION_PARAMS;
/******************************************************************
 *   MACROS DEFINITION
 ******************************************************************/
#define MEMPLUGIN_ION_PARAMS_INIT(buffer_ion_params) do {\
        (buffer_ion_params)->alloc_flags = 1 << OMAP_ION_HEAP_SECURE_INPUT;\
        (buffer_ion_params)->map_flags = MAP_SHARED;\
        (buffer_ion_params)->nAlign = 0x1000;\
        (buffer_ion_params)->prot = PROT_READ | PROT_WRITE;\
        (buffer_ion_params)->nOffset = 0;\
}while(0)
/******************************************************************
 *   FUNCTIONS DEFINITION
 ******************************************************************/
 /*MemPlugin_ION_Init: To initialize a MemPlugin from DOMX PROXY
  *                Must be called before any other API calls.
  *                Maps the plugin handle to corresponding proxy
  *                element for further access
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Init(void **pMemPluginHandle);
 /*MemPlugin_Configure: To do different types of plugin specific
  *                     config operations required.
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param pConfigData: data specific to configuration to be done
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Configure(void *pMemPluginHandle,
                                        void *pConfigData);
/*MemPlugin_Open: To open and obtain a client for the MemPlugin
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param pClient: MemPlugin client used for operations
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Open(void *pMemPluginHandle,
                                        OMX_U32 *pClient);
/*MemPlugin_Close: To close the client of MemPlugin
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Close(void *pMemPluginHandle,
                                        OMX_U32 nClient);
/*MemPlugin_Alloc: To allocate a buffer
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  * @param pBufferParams: All buffer IN params required for allocation
  * @param pBufferProp: All buffer properties obtained from allocation
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Alloc(void *pMemPluginHandle,
                                        OMX_U32 nClient,
                                        MEMPLUGIN_BUFFER_PARAMS *pIonBufferParams,
                                        MEMPLUGIN_BUFFER_PROPERTIES *pIonBufferProp);
/*MemPlugin_Free: To free a buffer
  *
  * @param pMemPluginHandle: handle that provides access for APIs
  *                          corresponding to the plugin
  * @param nClient: MemPlugin client used for operations
  * @param pBufferParams: All buffer IN params required for freeing
  * @param pBufferProp: All buffer properties required for freeing
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_Free(void *pMemPluginHandle,
                                        OMX_U32 nClient,
                                        MEMPLUGIN_BUFFER_PARAMS *pIonBufferParams,
                                        MEMPLUGIN_BUFFER_PROPERTIES *pIonBufferProp);
 /*MemPlugin_DeInit: To destroy a MemPlugin from DOMX PROXY
  *
  * @param pMemPluginHandle: handle that provides identifies the
  *                          plugin to be destroyed
  */
MEMPLUGIN_ERRORTYPE MemPlugin_ION_DeInit(void *pMemPluginHandle);
