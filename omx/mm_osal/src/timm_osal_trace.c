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

/*#include "typedefs.h"*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "timm_osal_trace.h"

#ifdef _Android
#define LOG_TAG "DOMX"
#include <utils/Log.h>
#include <cutils/properties.h>
#endif

/**
* The OSAL debug trace detail can be set at compile time by defining the flag
* TIMM_OSAL_DEBUG_TRACE_DETAIL=<Details>
* detail - 0 - no detail
*          1 - function name
*          2 - function name, line number
* Prefix is added to every debug trace message
*/
#ifndef TIMM_OSAL_DEBUG_TRACE_DETAIL
#define TIMM_OSAL_DEBUG_TRACE_DETAIL 2
#endif

#define DEFAULT_TRACE_LEVEL TIMM_OSAL_TRACE_LEVEL_ERROR

static int trace_level = -1;

/* strip out leading ../ stuff that happens to __FILE__ for out-of-tree builds */
static const char *simplify_path(const char *file)
{
	while (file)
	{
		char c = file[0];
		if ((c != '.') && (c != '/') && (c != '\\'))
			break;
		file++;
	}
	return file;
}

void TIMM_OSAL_UpdateTraceLevel(void)
{
	char *val = getenv("TIMM_OSAL_DEBUG_TRACE_LEVEL");

	if (val)
	{
		trace_level = strtol(val, NULL, 0);
	}
	else
	{
#ifdef _Android
		char value[PROPERTY_VALUE_MAX];
		int val;

		property_get("debug.domx.trace_level", value, "0");
		val = atoi(value);
		if ( (!val) || (val < 0) )
		{
			trace_level = DEFAULT_TRACE_LEVEL;
		}
		else
			trace_level = val;
#else
		trace_level = DEFAULT_TRACE_LEVEL;
#endif
	}
}

void __TIMM_OSAL_TraceFunction(const __TIMM_OSAL_TRACE_LOCATION * loc,
    const char *fmt, ...)
{
	if (trace_level == -1)
	{
		char *val = getenv("TIMM_OSAL_DEBUG_TRACE_LEVEL");
		trace_level =
		    val ? strtol(val, NULL, 0) : DEFAULT_TRACE_LEVEL;
	}

	if (trace_level >= loc->level)
	{
		va_list ap;

		va_start(ap, fmt);	/* make ap point to first arg after 'fmt' */

#ifdef _Android

#if 0 // Original for reference
#if ( TIMM_OSAL_DEBUG_TRACE_DETAIL > 1 )
		LOG_PRI(ANDROID_LOG_DEBUG, LOG_TAG, "%s:%d\t%s()\t", simplify_path(loc->file), loc->line,
		    loc->function);
#endif
#else // Prints function_name for ERROR, WARNING and ENTRY/EXIT
	if ( (loc->level == TIMM_OSAL_TRACE_LEVEL_ERROR) || (loc->level == TIMM_OSAL_TRACE_LEVEL_WARNING) || (loc->level == TIMM_OSAL_TRACE_LEVEL_ENTERING) )
		LOG_PRI(ANDROID_LOG_DEBUG, LOG_TAG, "%s:%d\t%s()\t", simplify_path(loc->file), loc->line,
		    loc->function);
#endif

		LOG_PRI_VA(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ap);

#else

#if 0 // Original for reference
#if ( TIMM_OSAL_DEBUG_TRACE_DETAIL > 1 )
		printf("%s:%d\t%s()\t", simplify_path(loc->file), loc->line,
		    loc->function);
#endif
#else // Prints function_name for ERROR, WARNING and ENTRY/EXIT
		if ( (loc->level == 1) || (loc->level == 2) || (loc->level == 5) )
			printf("%s:%d\t%s()\t", simplify_path(loc->file), loc->line,
		    loc->function);
#endif

		vprintf(fmt, ap);

#endif

		va_end(ap);
	}
}
