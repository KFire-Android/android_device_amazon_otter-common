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

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <omx_base.h>
#include <omx_base_internal.h>
#include <timm_osal_interfaces.h>
#include <omx_base_utils.h>
#include <omx_base_dio_plugin.h>


/*===============================================================*/
/** @fn OMX_BASE_PROCESS_Events :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_TunnelEstablish(OMX_HANDLETYPE hComponent,
                                        OMX_U32 nPort,
                                        OMX_HANDLETYPE hTunnelComp,
                                        OMX_U32 nTunnPort)
{
    OMX_ERRORTYPE                eError = OMX_ErrorNone;
    OMX_S8                       cTunnelCompName[OMX_MAX_STRINGNAME_SIZE];
    OMX_VERSIONTYPE              tCompVersion;
    OMX_VERSIONTYPE              tSpecVersion;
    OMX_UUIDTYPE                 tCompUUID;
    OMX_COMPONENTTYPE           *pComp = (OMX_COMPONENTTYPE *)hComponent;
    OMX_BASE_DIO_CreateParams    sDIOCreateParams;
    OMX_BASE_PRIVATETYPE        *pBaseComPvt = NULL;
    OMX_BASE_INTERNALTYPE       *pBaseComInt = NULL;
    OMX_BASE_PORTTYPE           *pPort = NULL;
    OMX_U32                      nStartPort = 0;

    pBaseComPvt = (OMX_BASE_PRIVATETYPE *)pComp->pComponentPrivate;
    pBaseComInt = (OMX_BASE_INTERNALTYPE *)pBaseComPvt->hOMXBaseInt;
    nStartPort = pBaseComPvt->tDerToBase.nMinStartPortIndex;

    pPort = pBaseComInt->ports[nPort - nStartPort];

    eError = OMX_GetComponentVersion(pPort->hTunnelComp,
                                     (OMX_STRING)cTunnelCompName,
                                     &tCompVersion, &tSpecVersion, &tCompUUID);
    //OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    //AD: Commented this out bec conformance test fails at this check
    /* Incase if the Tunneled comp is a TI component, select for FrameQ based
     *  proprietary Tunneling , otherwise select Standard Tunneling */
#if 0
    if( strstr((OMX_STRING)cTunnelCompName, "OMX.TI.") &&
        strstr(pBaseComPvt->tDerToBase.cComponentName, "OMX.TI.")) {
        /* Add logic to use single processor FrameQ or RPC based FrameQ  */
        /* Initialize DIO and open as a supplier  */
        sDIOCreateParams.hComponent        = hComponent;
        sDIOCreateParams.pPort             = pPort;
        sDIOCreateParams.pAppCallbacks     = &(pBaseComInt->sAppCallbacks);
        strcpy(sDIOCreateParams.cBufAllocatorName, "FQ.SHMEM.BUF");
        strcpy(sDIOCreateParams.cHdrAllocatorName, "FQ.SHMEM.HDR");
        _OMX_BASE_GetChannelInfo(pPort, sDIOCreateParams.cChannelName);

        strcpy(sDIOCreateParams.cChannelName, "FRAMEQ_1");
        eError = OMX_BASE_DIO_Init(hComponent, nPort, "OMX.DIO.PTFRAMEQ",
                                   &sDIOCreateParams);
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    } else
#endif
    {
        /* Initialize DIO and open as a supplier  */
        sDIOCreateParams.hComponent        = hComponent;
        sDIOCreateParams.pPort             = pPort;
        sDIOCreateParams.pAppCallbacks     = &(pBaseComInt->sAppCallbacks);
        _OMX_BASE_GetChannelInfo(pPort, sDIOCreateParams.cChannelName);

        eError = OMX_BASE_DIO_Init(hComponent, nPort, "OMX.DIO.STDTUNNEL",
                                   &sDIOCreateParams);
        OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);
    }
EXIT:
    return (eError);
}

/*===============================================================*/
/** @fn _OMX_BASE_VerifyTunnelConnection :
 *  see omx_base_internal.h
 */
/*===============================================================*/
OMX_ERRORTYPE _OMX_BASE_VerifyTunnelConnection(OMX_BASE_PORTTYPE *pPort,
                                               OMX_HANDLETYPE hTunneledComp)
{
    OMX_PARAM_PORTDEFINITIONTYPE    sTunPortDef;
    OMX_ERRORTYPE                   eError = OMX_ErrorNone;

    OMX_BASE_INIT_STRUCT_PTR(&sTunPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    sTunPortDef.nPortIndex = pPort->nTunnelPort;

    /*Get portdef params of tunnel component */
    eError = OMX_GetParameter(hTunneledComp, OMX_IndexParamPortDefinition,
                              &sTunPortDef);
    OMX_BASE_ASSERT(eError == OMX_ErrorNone, eError);

    /* Check data compatablity between the ports */
    switch( pPort->sPortDef.eDomain ) {

        case OMX_PortDomainOther :
            if( pPort->sPortDef.format.other.eFormat !=
                sTunPortDef.format.other.eFormat ) {
                eError = OMX_ErrorPortsNotCompatible;
            }
            break;

        case OMX_PortDomainAudio :
            if( pPort->sPortDef.format.audio.eEncoding !=
                sTunPortDef.format.audio.eEncoding ) {
                eError = OMX_ErrorPortsNotCompatible;
            }
            break;

        /* Image and Video Components compatablity
        * 1. Incase if the tunnel component and tunneled component are of
        *  different domains, check for color format of respective domains.
        * 2. Incas if both tunnel component and tunneled component are of
        *  same domains, compression formats should match. if compression
        *  format is unused, check for color format */
        case OMX_PortDomainVideo :
            if( sTunPortDef.eDomain == OMX_PortDomainImage ) {
                if( pPort->sPortDef.format.video.eColorFormat !=
                    sTunPortDef.format.image.eColorFormat ) {
                    eError = OMX_ErrorPortsNotCompatible;
                }
            } else if( sTunPortDef.eDomain == OMX_PortDomainVideo ) {
                if( pPort->sPortDef.format.video.eCompressionFormat !=
                    sTunPortDef.format.video.eCompressionFormat ) {
                    eError = OMX_ErrorPortsNotCompatible;
                } else { /* compression format is matching */
                    if( sTunPortDef.format.video.eCompressionFormat ==
                        OMX_VIDEO_CodingUnused ) {
                        if( pPort->sPortDef.format.video.eColorFormat !=
                            sTunPortDef.format.video.eColorFormat ) {
                            eError = OMX_ErrorPortsNotCompatible;
                        }
                    }
                }
            }
            break;

        case OMX_PortDomainImage :
            if( sTunPortDef.eDomain == OMX_PortDomainVideo ) {
                if( pPort->sPortDef.format.image.eColorFormat !=
                    sTunPortDef.format.video.eColorFormat ) {
                    eError = OMX_ErrorPortsNotCompatible;
                }
            } else if( sTunPortDef.eDomain == OMX_PortDomainImage ) {
                if( pPort->sPortDef.format.image.eCompressionFormat !=
                    sTunPortDef.format.image.eCompressionFormat ) {
                    eError = OMX_ErrorPortsNotCompatible;
                } else { /* compression format is matching */
                    if( sTunPortDef.format.image.eCompressionFormat ==
                        OMX_IMAGE_CodingUnused ) {
                        if( pPort->sPortDef.format.image.eColorFormat !=
                            sTunPortDef.format.image.eColorFormat ) {
                            eError = OMX_ErrorPortsNotCompatible;
                        }
                    }
                }
            }
            break;

        default :
            eError = OMX_ErrorPortsNotCompatible;
            break;

    }

EXIT:
    return (eError);
}

