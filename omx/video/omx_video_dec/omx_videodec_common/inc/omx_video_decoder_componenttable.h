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

#ifndef _OMX_VIDEO_DECODER_COMPONENTTABLE_H_
#define _OMX_VIDEO_DECODER_COMPONENTTABLE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <OMX_Video.h>
#include <OMX_TI_Video.h>

/* ==========================================================================*/
/*
 * \struct OMXDecoderComponentList
 * OMXDecoderComponentList Structure maintain the table for decoder names and
 * function pointer for the respective decoder init functions.
 *
 */
/* ==========================================================================*/

typedef struct OMXDecoderComponentList {
    /*Component role to be specified here*/
    OMX_U8 cRole[OMX_MAX_STRINGNAME_SIZE];

    /*! Video Coding Type Format */
    OMX_VIDEO_CODINGTYPE eCompressionFormat;

    /*! Function pointer to the component Init function of decoder */
    OMX_ERRORTYPE (*fpDecoderComponentInit)(OMX_HANDLETYPE hComponent);

} OMXDecoderComponentList;

/* external definition for the Decoder Component List*/
extern OMXDecoderComponentList    DecoderList[];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_VIDEO_DECODER_COMPONENTTABLE_H_ */

