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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <timm_osal_types.h>
#include <timm_osal_memory.h>
#include <timm_osal_trace.h>

#include <memplugin.h>

extern MEMPLUGIN_TABLETYPE    MemPlugins_Map[];

MEMPLUGIN_ERRORTYPE MemPlugin_Init(char *cMemPluginName, void **pMemPluginHandle)
{
    MEMPLUGIN_OBJECT *pMemPluginHdl;
    OMX_BOOL bFound;
    OMX_U16 i = 0;
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;

    if(cMemPluginName == NULL || pMemPluginHandle == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        MEMPLUGIN_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }

    while(MemPlugins_Map[i].cMemPluginName != NULL)
    {
        if(strcmp(MemPlugins_Map[i].cMemPluginName,cMemPluginName) == 0)
        {
            bFound = OMX_TRUE;
            break;
        }
        else
        {
            i++;

        }
    }
    if(bFound)
    {
        eError = MemPlugins_Map[i].pMemPluginConfig(&pMemPluginHdl);
        if(eError != MEMPLUGIN_ERROR_NONE)
        {
            goto EXIT;
        }
        else
        {
            *pMemPluginHandle = pMemPluginHdl;
        }

    }
    else
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        MEMPLUGIN_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }
EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        MEMPLUGIN_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        MEMPLUGIN_EXIT("%s: executed successfully",__FUNCTION__);
    }
    return eError;
}

MEMPLUGIN_ERRORTYPE MemPlugin_Open(void *pMemPluginHandle, OMX_U32 *pClient)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;
    OMX_U32 pClientDesc;

    if(pMemPluginHandle == NULL || pClient == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        MEMPLUGIN_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }

    eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpOpen(pMemPluginHandle,&pClientDesc);
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        MEMPLUGIN_ERROR("%s: Client Open() failed",__FUNCTION__);
        goto EXIT;
    }
    else
    {
        *pClient = pClientDesc;
    }

EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        MEMPLUGIN_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        MEMPLUGIN_EXIT("%s: executed successfully with client desc %d",__FUNCTION__,*pClient);
    }
    return eError;
}

MEMPLUGIN_ERRORTYPE MemPlugin_Close(void *pMemPluginHandle, OMX_U32 nClient)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;

    if(pMemPluginHandle == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        MEMPLUGIN_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }
    eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpClose(pMemPluginHandle,nClient);
EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        MEMPLUGIN_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        MEMPLUGIN_EXIT("%s: executed successfully",__FUNCTION__);
    }
    return eError;
}
MEMPLUGIN_ERRORTYPE MemPlugin_Configure(void *pMemPluginHandle, void *pConfigData)
{
    //implementation to be added later
EXIT:
    return(MEMPLUGIN_ERROR_NONE);
}

MEMPLUGIN_ERRORTYPE MemPlugin_Alloc(void *pMemPluginHandle, OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;

    if(pMemPluginHandle == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        MEMPLUGIN_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }
    eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpAlloc(pMemPluginHandle,nClient,pBufferParams,pBufferProp);
EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        MEMPLUGIN_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        MEMPLUGIN_EXIT("%s: executed successfully",__FUNCTION__);
    }
    return eError;
}

MEMPLUGIN_ERRORTYPE MemPlugin_Free(void *pMemPluginHandle, OMX_U32 nClient,
                                    MEMPLUGIN_BUFFER_PARAMS *pBufferParams,
                                    MEMPLUGIN_BUFFER_PROPERTIES *pBufferProp)
{
    MEMPLUGIN_ERRORTYPE eError = MEMPLUGIN_ERROR_NONE;

    if(pMemPluginHandle == NULL)
    {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        MEMPLUGIN_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
    }
    eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpFree(pMemPluginHandle,nClient,pBufferParams,pBufferProp);
EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        MEMPLUGIN_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        MEMPLUGIN_EXIT("%s: executed successfully",__FUNCTION__);
    }
    return eError;
}

MEMPLUGIN_ERRORTYPE MemPlugin_DeInit(void *pMemPluginHandle)
{
      MEMPLUGIN_ERRORTYPE    eError = MEMPLUGIN_ERROR_NONE;

      if(pMemPluginHandle == NULL)
      {
        eError = MEMPLUGIN_ERROR_BADPARAMETER;
        MEMPLUGIN_ERROR("%s: Invalid parameter to function",__FUNCTION__);
        goto EXIT;
      }

     eError = ((MEMPLUGIN_OBJECT *)pMemPluginHandle)->fpDeInit(pMemPluginHandle);


EXIT:
    if(eError != MEMPLUGIN_ERROR_NONE)
    {
        MEMPLUGIN_EXIT("%s: failed with error %d",__FUNCTION__,eError);
    }
    else
    {
        MEMPLUGIN_EXIT("%s: executed successfully",__FUNCTION__);
    }
    return eError;

}
