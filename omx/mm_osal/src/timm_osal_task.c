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

/******************************************************************************
* Includes
******************************************************************************/

#include <stdio.h>
#include <pthread.h>		/*for POSIX calls */
#include <sched.h>		/*for sched structure */
#include <unistd.h>



#include "timm_osal_types.h"
#include "timm_osal_trace.h"
#include "timm_osal_error.h"
#include "timm_osal_memory.h"
#include "timm_osal_task.h"




/**
* TIMM_OSAL_TASK describe the different task information
*/
typedef struct TIMM_OSAL_TASK
{
	pthread_t threadID;	/*SHM check */
	/* To set the priority and stack size */
	pthread_attr_t ThreadAttr;	/*For setting the priority and stack size */
    /** Name of the task */
	/*    TIMM_OSAL_S8  name[8];*//* eight character plus null char */
    /** Pointer to the task stack memory */
/*    TIMM_OSAL_PTR stackPtr;*/
    /** Size of the task stack */
/*    TIMM_OSAL_S32 stackSize;*/
	/*parameters to the task */
	TIMM_OSAL_U32 uArgc;
	TIMM_OSAL_PTR pArgv;
    /** task priority */
/*    TIMM_OSAL_S32 priority;*/
    /** flag to check if task got created */
	TIMM_OSAL_BOOL isCreated;
} TIMM_OSAL_TASK;


/******************************************************************************
* Function Prototypes
******************************************************************************/


/* ========================================================================== */
/**
* @fn TIMM_OSAL_CreateTask function
*
* @see
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_CreateTask(TIMM_OSAL_PTR * pTask,
    TIMM_OSAL_TaskProc pFunc,
    TIMM_OSAL_U32 uArgc,
    TIMM_OSAL_PTR pArgv,
    TIMM_OSAL_U32 uStackSize, TIMM_OSAL_U32 uPriority, TIMM_OSAL_S8 * pName)
{

	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	TIMM_OSAL_TASK *pHandle = TIMM_OSAL_NULL;
	struct sched_param sched;
	size_t stackSize;
	*pTask = TIMM_OSAL_NULL;


	/*Task structure allocation */
	pHandle =
	    (TIMM_OSAL_TASK *) TIMM_OSAL_Malloc(sizeof(TIMM_OSAL_TASK), 0, 0,
	    0);
	if (pHandle == TIMM_OSAL_NULL)
	{
		bReturnStatus = TIMM_OSAL_ERR_ALLOC;
		goto EXIT;
	}

	/* Initial cleaning of the task structure */
	TIMM_OSAL_Memset((TIMM_OSAL_PTR) pHandle, 0, sizeof(TIMM_OSAL_TASK));

	/*Arguments for task */
	pHandle->uArgc = uArgc;
	pHandle->pArgv = pArgv;

	pHandle->isCreated = TIMM_OSAL_FALSE;


	if (SUCCESS != pthread_attr_init(&pHandle->ThreadAttr))
	{
		/*TIMM_OSAL_Error("Task Init Attr Init failed!"); */
		goto EXIT;
	}
	/* Updation of the priority and the stack size */

	if (SUCCESS != pthread_attr_getschedparam(&pHandle->ThreadAttr,
		&sched))
	{
		/*TIMM_OSAL_Error("Task Init Get Sched Params failed!"); */
		goto EXIT;
	}

	sched.sched_priority = uPriority;	/* relative to the default priority */
	if (SUCCESS != pthread_attr_setschedparam(&pHandle->ThreadAttr,
		&sched))
	{
		/*TIMM_OSAL_Error("Task Init Set Sched Paramsfailed!"); */
		goto EXIT;
	}

	/*First get the default stack size */
	if (SUCCESS != pthread_attr_getstacksize(&pHandle->ThreadAttr,
		&stackSize))
	{
		/*TIMM_OSAL_Error("Task Init Set Stack Size failed!"); */
		goto EXIT;
	}

	/*Check if requested stack size is larger than the current default stack size */
	if (uStackSize > stackSize)
	{
		stackSize = uStackSize;
		if (SUCCESS != pthread_attr_setstacksize(&pHandle->ThreadAttr,
			stackSize))
		{
			/*TIMM_OSAL_Error("Task Init Set Stack Size failed!"); */
			goto EXIT;
		}
	}



	if (SUCCESS != pthread_create(&pHandle->threadID,
		&pHandle->ThreadAttr, pFunc, pArgv))
	{
		/*TIMM_OSAL_Error ("Create_Task failed !"); */
		goto EXIT;
	}


	/* Task was successfully created */
	pHandle->isCreated = TIMM_OSAL_TRUE;
	*pTask = (TIMM_OSAL_PTR) pHandle;
	bReturnStatus = TIMM_OSAL_ERR_NONE;
    /**pTask = (TIMM_OSAL_PTR *)pHandle;*/

      EXIT:
/*    if((TIMM_OSAL_ERR_NONE != bReturnStatus) && (TIMM_OSAL_NULL != pHandle)) {
       TIMM_OSAL_Free (pHandle->stackPtr);*/
	if ((TIMM_OSAL_ERR_NONE != bReturnStatus))
	{
		TIMM_OSAL_Free(pHandle);
	}
	return bReturnStatus;

}

/* ========================================================================== */
/**
* @fn TIMM_OSAL_DeleteTask
*
* @see
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_DeleteTask(TIMM_OSAL_PTR pTask)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;

	TIMM_OSAL_TASK *pHandle = (TIMM_OSAL_TASK *) pTask;
	void *retVal;

	if ((NULL == pHandle) || (TIMM_OSAL_TRUE != pHandle->isCreated))
	{
		/* this task was never created */
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}
	if (pthread_attr_destroy(&pHandle->ThreadAttr))
	{
		/*TIMM_OSAL_Error("Delete_Task failed !"); */
		goto EXIT;
	}
	if (pthread_join(pHandle->threadID, &retVal))
	{
		/*TIMM_OSAL_Error("Delete_Task failed !"); */
		goto EXIT;
		/*	bReturnStatus = TIMM_OSAL_ERR_CREATE(TIMM_OSAL_ERR, TIMM_OSAL_COMP_TASK, status);*//*shm to be done */
	}
	bReturnStatus = TIMM_OSAL_ERR_NONE;
	TIMM_OSAL_Free(pHandle);
      EXIT:
	return bReturnStatus;
}


TIMM_OSAL_ERRORTYPE TIMM_OSAL_SleepTask(TIMM_OSAL_U32 mSec)
{
	TIMM_OSAL_S32 nReturn = 0;

#ifdef _POSIX_VERSION_1_
	usleep(1000 * mSec);
#else
	nReturn = usleep(1000 * mSec);
#endif
	if (nReturn == 0)
		return TIMM_OSAL_ERR_NONE;
	else
		return TIMM_OSAL_ERR_UNKNOWN;
}
