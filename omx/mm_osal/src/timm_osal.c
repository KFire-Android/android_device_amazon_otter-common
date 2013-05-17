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
#include "timm_osal_error.h"
#include "timm_osal_memory.h"
/*#include "timm_osal_trace.h"*/


/******************************************************************************
* Function Prototypes
******************************************************************************/

/****************************************************************
*  PRIVATE DECLARATIONS  : only used in this file
****************************************************************/
/*--------data declarations -----------------------------------*/


/* ========================================================================== */
/**
* @fn TIMM_OSAL_Init function initilize the osal with initial settings.
*
* @return  TIMM_OSAL_ERR_NONE if successful
*               !TIMM_OSAL_ERR_NONE if an error occurs
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_Init(void)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_NONE;
	return bReturnStatus;
}



/* ========================================================================== */
/**
* @fn TIMM_OSAL_Init function de-initilize the osal.
*
* @return  TIMM_OSAL_ERR_NONE if successful
*               !TIMM_OSAL_ERR_NONE if an error occurs
*/
/* ========================================================================== */

TIMM_OSAL_ERRORTYPE TIMM_OSAL_Deinit(void)
{
	TIMM_OSAL_ERRORTYPE bReturnStatus = TIMM_OSAL_ERR_NONE;
	return bReturnStatus;
}
