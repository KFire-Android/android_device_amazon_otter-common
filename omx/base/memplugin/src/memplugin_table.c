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
#include "memplugin.h"

extern MEMPLUGIN_ERRORTYPE MemPlugin_ION_Init(MEMPLUGIN_OBJECT * *pMemPluginHandle);

//extern MEMPLUGIN_ERRORTYPE MemPlugin_DRM_Configure(MEMPLUGIN_OBJECT * *pMemPluginHandle);


MEMPLUGIN_TABLETYPE    MemPlugins_Map[] =
{
    { "MEMPLUGIN_ION",    &MemPlugin_ION_Init },
    //   {"MEMPLUGIN_DRM" ,  &MemPlugin_DRM_Configure},
    { NULL, NULL }
};
