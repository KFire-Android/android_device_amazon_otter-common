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

#ifndef _TIMM_OSAL_PIPES_H_
#define _TIMM_OSAL_PIPES_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

#include "timm_osal_types.h"

/*
* Defined for Pipe timeout value
*/
	TIMM_OSAL_ERRORTYPE TIMM_OSAL_CreatePipe(TIMM_OSAL_PTR * pPipe,
	    TIMM_OSAL_U32 pipeSize,
	    TIMM_OSAL_U32 messageSize, TIMM_OSAL_U8 isFixedMessage);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_DeletePipe(TIMM_OSAL_PTR pPipe);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_WriteToPipe(TIMM_OSAL_PTR pPipe,
	    void *pMessage, TIMM_OSAL_U32 size, TIMM_OSAL_S32 timeout);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_WriteToFrontOfPipe(TIMM_OSAL_PTR pPipe,
	    void *pMessage, TIMM_OSAL_U32 size, TIMM_OSAL_S32 timeout);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_ReadFromPipe(TIMM_OSAL_PTR pPipe,
	    void *pMessage,
	    TIMM_OSAL_U32 size,
	    TIMM_OSAL_U32 * actualSize, TIMM_OSAL_S32 timeout);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_ClearPipe(TIMM_OSAL_PTR pPipe);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_IsPipeReady(TIMM_OSAL_PTR pPipe);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_GetPipeReadyMessageCount(TIMM_OSAL_PTR
	    pPipe, TIMM_OSAL_U32 * count);


#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* _TIMM_OSAL_PIPES_H_ */
