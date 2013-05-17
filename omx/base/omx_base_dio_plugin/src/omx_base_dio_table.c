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

#include <string.h>
#include "omx_base_internal.h"
#include "omx_base_dio_plugin.h"

/*******************************************************************
 * EXTERNAL REFERENCE NOTE: only use if not found in header file
 *******************************************************************/
/*----------         function prototypes      ------------------- */
/*----------         data declarations        ------------------- */
extern OMX_ERRORTYPE OMX_DIO_NonTunnel_Init(OMX_HANDLETYPE handle,
                                            OMX_PTR pCreateParams);

//extern OMX_ERRORTYPE OMX_DIO_StdTunnel_Init(OMX_HANDLETYPE handle,
  //                                          OMX_PTR pCreateParams);

//extern OMX_ERRORTYPE OMX_DIO_FrameqTunnel_Init(OMX_HANDLETYPE handle,
  //                                             OMX_PTR pCreateParams);

/*******************************************************************
 * PUBLIC DECLARATIONS: defined here, used elsewhere
 *******************************************************************/
/*----------         function prototypes      ------------------- */
/*----------         data declarations        ------------------- */

/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------          function prototypes      ------------------- */
/*----------          data declarations        ------------------- */

/** This structure enumerates the list of DIO objests
 *  registered in the system and function pointer for
 *  the respective DIO object initialization functions.
 */

OMX_BASE_DIO_REGISTERTYPE    OMX_BASE_DIO_Registered[] =
{

    { "OMX.DIO.NONTUNNEL",   &OMX_DIO_NonTunnel_Init },
    /*TBD: Enable it for Tunneling support */
    // { "OMX.DIO.STDTUNNEL",   &OMX_DIO_StdTunnel_Init },
    //{ "OMX.DIO.PTFRAMEQ",    &OMX_DIO_FrameqTunnel_Init },
    { NULL, NULL }

};

