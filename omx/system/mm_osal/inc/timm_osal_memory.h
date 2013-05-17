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

#ifndef _TIMM_OSAL_MEMORY_H_
#define _TIMM_OSAL_MEMORY_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

/*******************************************************************************
* Includes
*******************************************************************************/

#include "timm_osal_types.h"
#include "timm_osal_error.h"


/* Enumeration Flag for Memory Segmenation Id */
	typedef enum TIMMOSAL_MEM_SEGMENTID
	{

		TIMMOSAL_MEM_SEGMENT_EXT = 0,
		TIMMOSAL_MEM_SEGMENT_INT,
		TIMMOSAL_MEM_SEGMENT_UNCACHED
	} TIMMOSAL_MEM_SEGMENTID;


/*******************************************************************************
* External interface
*******************************************************************************/

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_CreateMemoryPool(void);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_DeleteMemoryPool(void);

	TIMM_OSAL_PTR TIMM_OSAL_Malloc(TIMM_OSAL_U32 size,
	    TIMM_OSAL_BOOL bBlockContiguous, TIMM_OSAL_U32 unBlockAlignment,
	    TIMMOSAL_MEM_SEGMENTID tMemSegId);

	void TIMM_OSAL_Free(TIMM_OSAL_PTR pData);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_Memset(TIMM_OSAL_PTR pBuffer,
	    TIMM_OSAL_U8 uValue, TIMM_OSAL_U32 uSize);

	TIMM_OSAL_S32 TIMM_OSAL_Memcmp(TIMM_OSAL_PTR pBuffer1,
	    TIMM_OSAL_PTR pBuffer2, TIMM_OSAL_U32 uSize);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_Memcpy(TIMM_OSAL_PTR pBufDst,
	    TIMM_OSAL_PTR pBufSrc, TIMM_OSAL_U32 uSize);

	TIMM_OSAL_U32 TIMM_OSAL_GetMemCounter(void);

#define TIMM_OSAL_GetMemUsage() 0

#define TIMM_OSAL_MallocExtn(size, bBlockContiguous, unBlockAlignment, tMemSegId, hHeap) \
    TIMM_OSAL_Malloc(size, bBlockContiguous, unBlockAlignment, tMemSegId )

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* _TIMM_OSAL_DEFINES_H_ */
