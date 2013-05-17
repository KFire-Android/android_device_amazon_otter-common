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


#include "timm_osal_types.h"
#include "timm_osal_trace.h"
#include "timm_osal_error.h"
#include "timm_osal_memory.h"
#include "timm_osal_semaphores.h"

#include <errno.h>

#include <pthread.h>
#include <sys/time.h>


/* ========================================================================== */
/**
* @fn TIMM_OSAL_MutexCreate function
*
*
*/
/* ========================================================================== */
TIMM_OSAL_ERRORTYPE TIMM_OSAL_MutexCreate(TIMM_OSAL_PTR * pMutex)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	pthread_mutex_t *plMutex = TIMM_OSAL_NULL;

	plMutex =
	    (pthread_mutex_t *) TIMM_OSAL_Malloc(sizeof(pthread_mutex_t), 0,
	    0, 0);
	if (TIMM_OSAL_NULL == plMutex)
	{
		bReturnStatus = TIMM_OSAL_ERR_ALLOC;
		goto EXIT;
	}

	/*if (SUCCESS != pthread_mutex_init(plMutex, pAttr)) */
	if (SUCCESS != pthread_mutex_init(plMutex, TIMM_OSAL_NULL))
	{
		/*TIMM_OSAL_Error ("Mutex Create failed !"); */
		/*goto EXIT; */
	} else
	{
	/**pMutex = (TIMM_OSAL_PTR *)plMutex;*/
		*pMutex = (TIMM_OSAL_PTR) plMutex;
		bReturnStatus = TIMM_OSAL_ERR_NONE;
	}
      EXIT:
	/*if((TIMM_OSAL_ERR_NONE != bReturnStatus)) {
	   TIMM_OSAL_Free(plMutex);
	   } */
	if ((TIMM_OSAL_ERR_NONE != bReturnStatus) &&
	    (TIMM_OSAL_NULL != plMutex))
	{
		TIMM_OSAL_Free(plMutex);
	}
	return bReturnStatus;

/**********************************************************/
/*return TIMM_OSAL_SemaphoreCreate(pMutex, 1);*/
/**********************************************************/
}

/* ========================================================================== */
/**
* @fn TIMM_OSAL_MutexDelete function
*
*
*/
/* ========================================================================== */
TIMM_OSAL_ERRORTYPE TIMM_OSAL_MutexDelete(TIMM_OSAL_PTR pMutex)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_NONE;
	pthread_mutex_t *plMutex = (pthread_mutex_t *) pMutex;

	if (plMutex == TIMM_OSAL_NULL)
	{
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}
	/*can we do away with if or with switch case */
	if (SUCCESS != pthread_mutex_destroy(plMutex))
	{
		/*TIMM_OSAL_Error("Delete Mutex failed !"); */
		bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;;
	}

	TIMM_OSAL_Free(plMutex);
      EXIT:
	return bReturnStatus;
/**********************************************************/
/*return TIMM_OSAL_SemaphoreDelete(pMutex);*/
/**********************************************************/

}

/* ========================================================================== */
/**
* @fn TIMM_OSAL_MutexObtain function
*
*
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_MutexObtain(TIMM_OSAL_PTR pMutex,
    TIMM_OSAL_U32 uTimeOut)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	struct timespec abs_timeout;
	struct timeval ltime_now;
	TIMM_OSAL_U32 ltimenow_us;
	pthread_mutex_t *plMutex = (pthread_mutex_t *) pMutex;

	if (plMutex == TIMM_OSAL_NULL)
	{
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	if (TIMM_OSAL_SUSPEND == uTimeOut)
	{
		if (SUCCESS != pthread_mutex_lock(plMutex))
		{
			/*TIMM_OSAL_Error("Lock Mutex failed !"); */
			goto EXIT;
		}
	} else if (TIMM_OSAL_NO_SUSPEND == uTimeOut)
	{
		if (SUCCESS != pthread_mutex_trylock(plMutex))
		{
			/*TIMM_OSAL_Error("Lock Mutex failed !"); */
			goto EXIT;
		}
	} else
	{
		gettimeofday(&ltime_now, NULL);
		/*uTimeOut is assumed to be in milliseconds */
		ltimenow_us = ltime_now.tv_usec + 1000 * uTimeOut;
		abs_timeout.tv_sec = ltime_now.tv_sec + uTimeOut / 1000;
		abs_timeout.tv_nsec = (ltimenow_us % 1000000) * 1000;

#ifdef _POSIX_VERSION_1_
		if (SUCCESS != pthread_mutex_lock(plMutex))
		{		//Some Posix versions dont support timeout
#else
		if (SUCCESS != pthread_mutex_timedlock(plMutex, &abs_timeout))
		{
#endif
			/*TIMM_OSAL_Error("Lock Mutex failed !"); */
			goto EXIT;
		}
	}
	bReturnStatus = TIMM_OSAL_ERR_NONE;

      EXIT:
	return bReturnStatus;
/**********************************************************/
/*return TIMM_OSAL_SemaphoreObtain(pMutex, uTimeOut);*/
/**********************************************************/
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_MutexRelease function
*
*
*/
/* ========================================================================== */
TIMM_OSAL_ERRORTYPE TIMM_OSAL_MutexRelease(TIMM_OSAL_PTR pMutex)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_UNKNOWN;
	pthread_mutex_t *plMutex = (pthread_mutex_t *) pMutex;

	if (TIMM_OSAL_NULL == plMutex)
	{
		bReturnStatus = TIMM_OSAL_ERR_PARAMETER;
		goto EXIT;
	}

	if (SUCCESS != pthread_mutex_unlock(plMutex))
	{
		/*TIMM_OSAL_Error("Unlock Mutex failed !"); */
	} else
	{
		bReturnStatus = TIMM_OSAL_ERR_NONE;
	}
      EXIT:
	return bReturnStatus;
/**********************************************************/
/*return TIMM_OSAL_SemaphoreRelease(pMutex);*/
/**********************************************************/
}
