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

#ifndef _TIMM_OSAL_EVENTS_H_
#define _TIMM_OSAL_EVENTS_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

#include "timm_osal_types.h"

	typedef enum TIMM_OSAL_EVENT_OPERATION
	{
		TIMM_OSAL_EVENT_AND,
		TIMM_OSAL_EVENT_AND_CONSUME,
		TIMM_OSAL_EVENT_OR,
		TIMM_OSAL_EVENT_OR_CONSUME
	} TIMM_OSAL_EVENT_OPERATION;

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_EventCreate(TIMM_OSAL_PTR * pEvents);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_EventDelete(TIMM_OSAL_PTR pEvents);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_EventSet(TIMM_OSAL_PTR pEvents,
	    TIMM_OSAL_U32 uEventFlag, TIMM_OSAL_EVENT_OPERATION eOperation);

	TIMM_OSAL_ERRORTYPE TIMM_OSAL_EventRetrieve(TIMM_OSAL_PTR pEvents,
	    TIMM_OSAL_U32 uRequestedEvents,
	    TIMM_OSAL_EVENT_OPERATION eOperation,
	    TIMM_OSAL_U32 * pRetrievedEvents, TIMM_OSAL_U32 uTimeOut);

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* _TIMM_OSAL_EVENTS_H_ */
