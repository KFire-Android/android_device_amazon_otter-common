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

#ifndef _OMX_VIDEO_DECODER_H_
#define _OMX_VIDEO_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <OMX_Types.h>
#include <omx_base.h>
#include <xdc/std.h>
#include <ti/xdais/xdas.h>
#include <ti/sdo/ce/video3/viddec3.h>
#include <timm_osal_interfaces.h>
#include <omx_base_utils.h>
#include <OMX_TI_Video.h>
#include <ti/sdo/codecs/h264dec/ih264vdec.h>

/** Input port Index for the OMX_MPEG4VD Comp */
#define OMX_VIDEODECODER_INPUT_PORT 0

/** Output port Index for the OMX_MPEG4VD Comp */
#define OMX_VIDEODECODER_OUTPUT_PORT 1

/** Number of ports for the OMX_MPEG4VD Comp */
#define  OMX_VIDEODECODER_NUM_OF_PORTS 2

/** Deafult portstartnumber of MPEG4vd comp */
#define  OMX_VIDEODECODER_DEFAULT_START_PORT_NUM 0

#define DEFAULT_COMPOENENT 0

/* ==========================================================================*/
/*
 * \struct BUFF_PARAMS
 * BUFF_PARAMS is an internal structure which gives the buffer parameters
 * of the buffer allocated to each port
 *
 */
/* ==========================================================================*/
typedef struct BUFF_PARAMS {
    OMX_U32 nBufferSize;              /*! Buffer size */
    OMX_U32 nPaddedWidth;             /*! Padded Width of the buffer */
    OMX_U32 nPaddedHeight;            /*! Padded Height */
    OMX_U32 nBufferCountMin;          /*! Min number of buffers required */
    OMX_U32 nBufferCountActual;       /*! Actual number of buffers */
    OMX_U32 n1DBufferAlignment;       /*! 1D Buffer Alignment value */
    OMX_U32 n2DBufferYAlignment;      /*! Y axis alignment value in 2Dbuffer */
    OMX_U32 n2DBufferXAlignment;      /*! X Axis alignment value in 2Dbuffer */
} BUFF_PARAMS;

/* ==========================================================================*/
/*
 * \struct BUFF_POINTERS
 * BUFF_POINTERS is an internal structure which contains the pointers to the
 * decoders interface xDM class objects (like InArgs, OutArgs). Note that this
 * can be either extended xDM params or base xDM params
 *
 */
/* ==========================================================================*/
typedef struct BUFF_POINTERS {
    OMX_PTR pDecStaticParams;        /*! Pointer to Decoder Static Params */
    OMX_PTR pDecDynParams;           /*! Pointer to Decoder Dynamic Params */
    OMX_PTR pDecStatus;              /*! Pointer to Decoder Status struct */
    OMX_PTR pDecInArgs;              /*! Pointer to Decoder InArgs */
    OMX_PTR pDecOutArgs;             /*! Pointer to Decoder OutArgs */
} BUFF_POINTERS;

/* ==========================================================================*/
/*
 * \struct BASECLASS_BUFF_POINTERS
 * BASECLASS_BUFF_POINTERS is same as BUFF_POINTERS except that this points
 * to only the baseclass xDM objects
 *
 */
/* ==========================================================================*/
typedef struct BASECLASS_BUFF_POINTERS {
    /*! Pointer to base-class Static Params */
    IVIDDEC3_Params *pDecStaticParams;
    /*! Pointer to base-class Dynamic Params */
    IVIDDEC3_DynamicParams *pDecDynParams;
    /*! Pointer to the base-class Decoder Status */
    IVIDDEC3_Status *pDecStatus;
    /*! Pointer to the base-class InArgs */
    IVIDDEC3_InArgs *pDecInArgs;
    /*! Pointer to the base-class OutArgs */
    IVIDDEC3_OutArgs *pDecOutArgs;
} BASECLASS_BUFF_POINTERS;

typedef struct OMX_METADATA_LOOKUP_TABLE {
    OMX_U32 inputID;
    OMX_PTR pMetadataBufferFrame1;
    OMX_PTR pMetadataBufferFrame2;
    OMX_U32 bEntryUsedFlag;
} OMX_METADATA_LOOKUP_TABLE;

typedef struct OMX_METADATA {
    OMX_PTR pMBinfo;
    OMX_PTR pSEIinfo;
    OMX_PTR pVUIinfo;
    OMX_U32 CodecExtenderError;
} OMX_METADATA;

typedef struct OMX_TI_MBINFO {
    OMX_PTR pMBinfo;
    OMX_U32 nSize;
} OMX_TI_MBINFO;

typedef struct OMX_TI_SEIINFO {
    OMX_PTR pSEIinfo;
    OMX_U32 nSize;
} OMX_TI_SEIINFO;

typedef struct OMX_TI_VUIINFO {
    OMX_PTR pVUIinfo;
    OMX_U32 nSize;
} OMX_TI_VUIINFO;

typedef struct OMX_DATASYNC_INBUF_LOOKUP_TABLE {
    OMX_BUFFERHEADERTYPE   *pInBufHeader;
    OMX_U32                bEntryUsedFlag;
} OMX_DATASYNC_INBUF_LOOKUP_TABLE;

#if (DUCATI_VIDEO_PERF_MEASUREMENT == 1)

DERIVEDSTRUCT(OMX_VIDEODECODER_PVTTYPE, OMX_BASE_PRIVATETYPE)
#define OMX_VIDEODECODER_PVTTYPE_FIELDS OMX_BASE_PRIVATETYPE_FIELDS \
    VIDDEC3_Handle pDecHandle;\
    Engine_Handle                     ce;\
    BASECLASS_BUFF_POINTERS           pBaseClassStructers; \
    BUFF_POINTERS                     pDecStructures; \
    XDM2_BufDesc                      tInBufDesc; \
    XDM2_BufDesc                      tOutBufDesc; \
    OMX_CONFIG_RECTTYPE               t2DBufferAllocParams[OMX_VIDEODECODER_NUM_OF_PORTS]; \
    OMX_VIDEO_PARAM_PORTFORMATTYPE    tVideoParams[OMX_VIDEODECODER_NUM_OF_PORTS];\
    void (*fpSet_StaticParams)(OMX_HANDLETYPE hComponent, void *params); \
    void (*fpSet_DynamicParams)(OMX_HANDLETYPE hComponent, void *dynamicparams); \
    void (*fpSet_Status)(OMX_HANDLETYPE hComponent, void *status); \
    void (*fpDeinit_Codec)(OMX_HANDLETYPE hComponent); \
    OMX_ERRORTYPE (*fpHandle_ExtendedError)(OMX_HANDLETYPE hComponent); \
    OMX_ERRORTYPE (*fpHandle_CodecGetStatus)(OMX_HANDLETYPE hComponent); \
    BUFF_PARAMS (*fpCalc_OubuffDetails)(OMX_HANDLETYPE hComponent, OMX_U32 width, OMX_U32 height); \
    String                                   strDecoderName; \
    String                                   strComponentName; \
    OMX_VIDEO_CODINGTYPE                     eCompressionFormat; \
    OMX_U32                                  nFrameCounter; \
    OMX_BOOL                                 bSyncFrameReady; \
    OMX_U32                                  CodecWorkaroundFlags; \
    OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR   *pAttrDesc; \
    OMX_U32                                  nOutbufInUseFlag; \
    OMX_U32                                  nOutPortReconfigRequired; \
    OMX_U32                                  nCodecRecreationRequired; \
    OMX_PARAM_COMPONENTROLETYPE              tComponentRole; \
    OMX_PTR                                  pCodecPrivate; \
    OMX_TI_VIDEO_RMADAPTIONPARAMS            tVideoCompRMParmas;\
    TIMM_OSAL_TRACEGRP                       nTraceGrp;\
    OMX_U32                                  mem_count_start;\
    OMX_U32                                  mem_size_start;\
    OMX_U32                                  mem_count_end;\
    OMX_U32                                  mem_size_end;\
    OMX_U32                                  parseHeaderFlag;\
    Memory_Stats                             stats; \
    OMX_U32                                  nFatalErrorGiven; \
    OMX_PTR                                  pMBinfo;\
    OMX_U32                                  nMBinfoSize;\
    OMX_CONFIG_RECTTYPE                      tCropDimension;         \
    OMX_TI_STREAMINTERLACEFORMAT             tIlaceFmtTypes; \
    OMX_BOOL                                 bUsePortReconfigForCrop;   \
    OMX_BOOL                                 bUsePortReconfigForPadding; \
    OMX_BOOL                                 bSupportDecodeOrderTimeStamp; \
    OMX_BOOL                                 bSupportSkipGreyOutputFrames; \
    uint64                                   decRecordSessionFrameCounter;\
    uint64                                   decRecordSessionProcessTimeCounter;\
    uint32                                   decRecordSessionAvgProcessTime;\
    uint32                                   decRecordSessionMinProcessTime;\
    uint32                                   decRecordSessionMaxProcessTime;\
    OMX_TI_PARAM_DECMETADATA                 tMetadata;\
    OMX_U8                                   nMBInfoIndex;\
    OMX_U8                                   nSEIInfoIndex;\
    OMX_U8                                   nVUIInfoIndex;\
    OMX_BOOL                                 bErrorConcealmentBuffer;\
    OMX_U32                                  nMBInfoStructSize;\
    OMX_U32                                  nMBInfoMaxSize; \
    OMX_TI_PARAM_USEBUFFERDESCRIPTOR         tUseBufDesc[OMX_VIDEODECODER_NUM_OF_PORTS];
ENDSTRUCT(OMX_VIDEODECODER_PVTTYPE)
ENDSTRUCT(OMX_VIDEODECODER_PVTTYPE)

#else

DERIVEDSTRUCT(OMX_VIDEODECODER_PVTTYPE, OMX_BASE_PRIVATETYPE)
#define OMX_VIDEODECODER_PVTTYPE_FIELDS OMX_BASE_PRIVATETYPE_FIELDS \
    VIDDEC3_Handle pDecHandle;\
    Engine_Handle                     ce;\
    BASECLASS_BUFF_POINTERS           pBaseClassStructers; \
    BUFF_POINTERS                     pDecStructures; \
    XDM2_BufDesc                      tInBufDesc; \
    XDM2_BufDesc                      tOutBufDesc; \
    OMX_CONFIG_RECTTYPE               t2DBufferAllocParams[OMX_VIDEODECODER_NUM_OF_PORTS]; \
    OMX_VIDEO_PARAM_PORTFORMATTYPE    tVideoParams[OMX_VIDEODECODER_NUM_OF_PORTS];\
    void (*fpSet_StaticParams)(OMX_HANDLETYPE hComponent, void *params); \
    void (*fpSet_DynamicParams)(OMX_HANDLETYPE hComponent, void *dynamicparams); \
    void (*fpSet_Status)(OMX_HANDLETYPE hComponent, void *status); \
    void (*fpDeinit_Codec)(OMX_HANDLETYPE hComponent); \
    OMX_ERRORTYPE (*fpHandle_ExtendedError)(OMX_HANDLETYPE hComponent); \
    OMX_ERRORTYPE (*fpHandle_CodecGetStatus)(OMX_HANDLETYPE hComponent); \
    BUFF_PARAMS (*fpCalc_OubuffDetails)(OMX_HANDLETYPE hComponent, OMX_U32 width, OMX_U32 height); \
    OMX_STRING                               strDecoderName; \
    OMX_STRING                               strComponentName; \
    OMX_VIDEO_CODINGTYPE                     eCompressionFormat; \
    OMX_U32                                  nFrameCounter; \
    OMX_BOOL                                 bSyncFrameReady;\
    OMX_U32                                  CodecWorkaroundFlags; \
    OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR   *pAttrDesc; \
    OMX_U32                                  nOutbufInUseFlag; \
    OMX_U32                                  nOutPortReconfigRequired; \
    OMX_U32                                  nCodecRecreationRequired; \
    OMX_PARAM_COMPONENTROLETYPE              tComponentRole; \
    OMX_PTR                                  pCodecPrivate; \
    TIMM_OSAL_TRACEGRP                       nTraceGrp;\
    OMX_U32                                  mem_count_start;\
    OMX_U32                                  mem_size_start;\
    OMX_U32                                  mem_count_end;\
    OMX_U32                                  mem_size_end;\
    OMX_U32                                  parseHeaderFlag;\
    OMX_U32                                  nFatalErrorGiven; \
    OMX_METADATA_LOOKUP_TABLE                tMetadata_table_entries[32];\
    OMX_U32                                  bIsFirstDataSyncCall;\
    XDAS_Int32                               nSliceSizes[4];\
    OMX_DATASYNC_INBUF_LOOKUP_TABLE          tDatasync_InBufHeader_table_entries[4];\
    OMX_U32                                  bIsFlushRequired;\
    OMX_PTR                                  pMBinfo;\
    OMX_U32                                  nMBinfoSize;\
    OMX_CONFIG_RECTTYPE                      tCropDimension; \
    OMX_CONFIG_SCALEFACTORTYPE               tScaleParams; \
    OMX_TI_STREAMINTERLACEFORMAT             tIlaceFmtTypes; \
    OMX_BOOL                                 bUsePortReconfigForCrop; \
    OMX_BOOL                                 bUsePortReconfigForPadding; \
    OMX_BOOL                                 bSupportDecodeOrderTimeStamp; \
    OMX_BOOL                                 bSupportSkipGreyOutputFrames; \
    TIMM_OSAL_PTR                            pTimeStampStoragePipe; \
    OMX_U32                                  bInputBufferCancelled;\
    OMX_U32                                  nFrameRateDivisor; \
    OMX_TI_PARAM_DECMETADATA                 tMetadata;\
    OMX_U8                                   nMBInfoIndex;\
    OMX_U8                                   nSEIInfoIndex;\
    OMX_U8                                   nVUIInfoIndex;\
    OMX_BOOL                                 bErrorConcealmentBuffer;\
    OMX_U32                                  nMBInfoStructSize;\
    OMX_U32                                  nMBInfoMaxSize; \
    OMX_TI_PARAM_USEBUFFERDESCRIPTOR         tUseBufDesc[OMX_VIDEODECODER_NUM_OF_PORTS];
ENDSTRUCT(OMX_VIDEODECODER_PVTTYPE)

#endif


DERIVEDSTRUCT(OMX_VIDEODECODER_CUSTOMBUFFERHEADER, OMX_BASE_BUFFERHEADERTYPE)
#define OMX_VIDEODECODER_CUSTOMBUFFERHEADER_FIELDS OMX_BASE_BUFFERHEADERTYPE_FIELDS \
    OMX_U32 nMetaDataUsageBitField;                           \
    OMX_TI_STEREODECINFO            tSEIinfo2004Frame1;                      \
    OMX_TI_STEREODECINFO            tSEIinfo2004Frame2;                        \
    OMX_TI_FRAMEPACKINGDECINFO      tSEIinfo2010Frame1;                  \
    OMX_TI_FRAMEPACKINGDECINFO      tSEIinfo2010Frame2;                  \
    OMX_TI_VIDEO_RANGEMAPPING       tRangeMapping;                        \
    OMX_TI_VIDEO_RESCALINGMATRIX    tRescalingMatrix;                  \
    OMX_U32                         nCodecExtendedErrorFrame1; \
    OMX_U32                         nCodecExtendedErrorFrame2; \
    OMX_TI_MBINFO                   tMbInfoFrame1; \
    OMX_TI_MBINFO                   tMbInfoFrame2; \
    OMX_TI_SEIINFO                  tSeiInfoFrame1; \
    OMX_TI_SEIINFO                  tSeiInfoFrame2; \
    OMX_TI_VUIINFO                  tVuiInfoFrame1; \
    OMX_TI_VUIINFO                  tVuiInfoFrame2;
ENDSTRUCT(OMX_VIDEODECODER_CUSTOMBUFFERHEADER)

/* external definition for the Video Params Init function */
extern void OMX_TI_Init_Video_decoder_params(OMX_HANDLETYPE hComponent,
                                             OMX_VIDEODECODER_PVTTYPE *pVideoDecoderComPvt);
extern OMX_ERRORTYPE OMX_TI_VideoDecoder_ComponentInit(OMX_HANDLETYPE hComponent);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_VIDEO_DECODER_H_ */

