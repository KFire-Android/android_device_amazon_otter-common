/*
*******************************************************************************
 * HDVICP2.0 Based H.264 HP Decoder
 *
 * "HDVICP2.0 Based H.264 HP Decoder" is a software module developed on TI's
 *  HDVICP2 based SOCs. This module is capable of decoding a compressed
 *  high/main/baseline profile H.264 bit-stream into a YUV 4:2:0 Raw video.
 *  Based on "ISO/IEC 14496-10".
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
*******************************************************************************
*/

/**
*******************************************************************************
 * @file ih264vdec.h
 *
 * @brief  H264VDEC Interface Header file.
 *
 *         This File contains the interface structures and Macro definition
 *         required for integrating the H264VDEC.
 *
 * @author: Pavan Shastry (pavanvs@ti.com)
 *
 * @version 0.1 (Jan 2008) : Base version borrowed from DSP RnD
 * @version 0.2 (Oct 2009) : Code commenting/cleanup.
 *                           [Keshav]
 * @version 0.3 (Sep 2010) : Added new structure IH264VDEC_SeiFramePacking
 *                            and also added the new element frame_packing
 *                            to the structure IH264VDEC_SeiMessages
 *                            [Ramkumar]
 * @version 0.4 (Sep 2010) : Added review comments and added one new default
 *                           dynemic parameter [Ashish  Singh ]
 * @version 0.5 (Nov 2010) : Added new elements in status and params for
 *                           debug trace [Resmi]
 * @version 0.6 (Nov 2010) : Changes for indicating gaps in frame_num to app
 *                           [Resmi]
 * @version 0.7 (Dec 2010) : Extending the create param structure to accomodate
 *                           configurability feature for the detection
 *                           of CABAC and IPCM alignment errors.
 *                           [Ramakrishna Adireddy]
 * @version 0.8 (Dec 2010) : Extending the array size for registered and
 *                           un-registered SEI and added a flag in each SEI
 *                           message to indicate the overflow.
 *                           [Ramakrishna Adireddy]
 * @version 0.9 (Jan 2011) : Added new common str for storing up the Data
 *                           elements required by SVC decoder
 *                           [Ashish]
 * @version 1.0(June 2011) : Added enableDualOutput param and enum for dualYuv
 *                           support.
 *                           [Vijay Kumar Yadav]
 * @version 1.1(June 2011) : Added parameter required for dualYuv EC.
 *                           [Vijay]
 * @version 1.2 (Oct 2011) : Gaps in frame number enhancements
 *                           [Ramakrishna Adireddy]
 * @version 1.3 (Oct 2011) : Added WaterMark parameters.
 *                           [Suresh Reddy]
 * @version 1.4 (July 2012): Added new create time param to support decoding
 *                           of specific frame types as requested by
 *                           application [Mahantesh]
******************************************************************************
*/
#ifndef IH264VDEC_
#define IH264VDEC_

#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividdec3.h>

#define IH264VDEC_MAX_LENGTH_PROCESS_LIST (24)

/**
 ******************************************************************************
 *  @struct IH264VDEC_Obj
 *
 *  @brief  Module’s object definition. This structure must be the first field
 *          of all H264VDEC instance objects
 *
 *  @param  fxns : Structure type defining all functions necessary to
 *                 implement the IH264VDEC interface.
 ******************************************************************************
*/
typedef struct IH264VDEC_Obj
{
  struct IH264VDEC_Fxns *fxns;
} IH264VDEC_Obj;

/**
 ******************************************************************************
 *  @struct IH264VDEC_Status
 *
 *  @brief  This structure defines parameters that describe the status of the
 *          H264 Decoder and any other implementation specific parameters.
 *          The status parameters are defined in the XDM data structure,
 *          IVIDDEC3_Status
 *
 *  @param  viddec3Status : XDM Base class status structure (see ividdec3.h)
 *
 *  @param  svcTargetLayerDID : SVC Spatial target layer ID,need to set if svc
 *                codec flag is on else default set to 0.
 *
 *  @param  svcTargetLayerQID : SVC Quality target layer ID,need to set if svc
 *                codec flag is on else default set to 0.
 *
 *
 *  @param  svcTargetLayerTID : SVC Temporal target layer ID,need to set if svc
 *                codec flag is on else default set to 0.
 *
 *  @param  debugTraceLevel:  This parameter reports the debug trace level
 *                            configured for the codec
 *
 *  @param  lastNFramesToLog:  This parameter reports the number of last N
 *                             pictures for which history has been maintained
 *
 *  @param  extMemoryDebugTraceAddr:  This parameter reports the external
 *                             memory address (as seen by M3) where debug trace
 *                             information is being dumped
 *
 *  @param  extMemoryDebugTraceSize:  This parameter reports the external
 *                             memory buffer size(in bytes) where debug trace
 *                             information is being dumped
 *
 *  @param  gapInFrameNum:  This parameter reports the gap in frame_num
 *                          observed in the current frame. It is overloaded
 *                          with details of two things.
 *                          LSB 1-bit [0:0]: Indicates whether SPS allows frame
 *                                           num gaps.
 *                          Rest 31-bits [31:1]: Indicates the gap observed
 *
 *  @param  spsMaxRefFrames:  This parameter reports max reference frames
 *                           that get used for decoding of a given stream.
 *                           If SPS is not parsed, this gives out the value
 *                           based on level & resolution set in create time
 *                           params. Otherwise, it gives out the parsed value
 *
 *  @param enableDualOutput  :  This Parameter tells whether Dual output is
 *                             enable or not. If enable then application needs
 *                             to provide two extra buffer (one for DualLuma
 *                             and another for DualChroma.
 *
 *  @param reserved[2]      :  Left reserve for future purpose
 *
 ******************************************************************************
*/
typedef struct IH264VDEC_Status
{
  IVIDDEC3_Status viddec3Status;
  XDAS_Int32  svcTargetLayerDID;
  XDAS_Int32  svcTargetLayerTID;
  XDAS_Int32  svcTargetLayerQID;
  XDAS_UInt32 debugTraceLevel;
  XDAS_UInt32 lastNFramesToLog;
  XDAS_UInt32 *extMemoryDebugTraceAddr;
  XDAS_UInt32 extMemoryDebugTraceSize;
  XDAS_UInt32 gapInFrameNum;
  XDAS_UInt32 spsMaxRefFrames;
  XDAS_UInt32 enableDualOutput;
  XDAS_UInt32 reserved[2];
} IH264VDEC_Status;

 /**
 ******************************************************************************
 *  @struct IH264VDEC_Params
 *
 *  @brief  This structure defines the creation parameters for all H264VDEC
 *          objects. This structure includes the xdm baseclass creation
 *          parameters and any other implementation specific parameters for
 *          H264 Decoder instance object.
 *
 *  @param  viddec3Params :  XDM Baselass create time parameters.
 *                              (see ividdec3.h)
 *  @param  dpbSizeInFrames:   Number of frames required by the DPB
 *                             (Decoded Picture Buffer). This is the DPB size
 *                             in number of frames. Also, See the enum
 *                             IH264VDEC_dpbNumFrames.
 *  @param  pConstantMemory :  This pointer points to the the memory area where
 *                             constants are located. Default value is NULL in
 *                             which case, codec puts the constants in a
 *                             default section.
 *                             It has to be in DDR addressable space by
 *                             vDMA. This is useful to allow relocatable
 *                             constants for the applications which doesn't use
 *                             M3 as host. Actual memory controller/allocator
 *                             is on another master processor.
 *                             If this is set to NULL then decoder assumes
 *                             that all constants are pointed by symbol
 *                             H264VDEC_TI_ConstData
 *
 *  @param  bitStreamFormat :  Input bit stream format. Input bits stream can
 *                             be IH264VDEC_NAL_UNIT_FORMAT or
 *                             IH264VDEC_BYTE_STREAM_FORMAT. See the enum
 *                             IH264VDEC_bitStreamFormat for details.
 *                             The decoder supports IH264VDEC_BYTE_STREAM_FORMAT
 *                             in both datasync mode and non datasync mode
 *                             (i.e inputDataMode = IVIDEO_ENTIREFRAME).
 *                             But the IH264VDEC_NAL_UNIT_FORMAT is supported
 *                             only in datasync mode, i.e only when
 *                             inputDataMode = IH264VDEC_NALUNIT_SLICEMODE.
 *
 *  @param  errConcealmentMode : If this is set to 1, it means that the YUV
 *                              buffer passed in current process call needs
 *                              concealment. If this is set to 0, it means that
 *                              the YUV buffer passed in current process call
 *                              does not concealment. Note that whether decoder
 *                              actually performed the concealment or not is
 *                              indicated by XDM_APPLIEDCONCEALMENT bit in
 *                              extended error field
 *
 *  @param  temporalDirModePred: Parameter to enable/disable temporal direct
 *                               Prediction mode. 0: Disable, 1:Enable
 *                               If this Parameter is disabled set to 0), and
 *                               if the B slice uses temporal direct mode,
 *                               then the codec returns error for that slice.
 *
 *  @param svcExtensionFlag  : If required to decode SVC streams , set flag
 *                             ON else default set to IH264SVCVDEC_EXT_FLAG
 *
 *  @param  svcTargetLayerDID : SVC Spatial target layer ID,need to read if svc
 *                codec flag (svcExtensionFlag)is on else default
 *                set to IH264SVCVDEC_TARGET_DEFAULT_DID.
 *
 *  @param  svcTargetLayerQID : SVC Quality target layer ID,need to read if svc
 *                codec flag  (svcExtensionFlag) is on else default
 *                set to IH264SVCVDEC_TARGET_DEFAULT_QID.
 *
 *
 *  @param  svcTargetLayerTID : SVC Temporal target layer ID,need to read if svc
 *                codec flag  (svcExtensionFlag)is on else default
 *                set to IH264SVCVDEC_TARGET_DEFAULT_TID.
 *
 *  @param presetLevelIdc   : Level to which decoder has to be configured by the
 *                            application
 *
 *  @param presetProfileIdc : Profile to which decoder has to be configured by
 *                            the application. Currently unused inside codec.
 *
 *  @param  detectCabacAlignErr: This parameter configures the cabac alignment
 *                               error detection
 *
 *  @param  detectIPCMAlignErr: This parameter configures the IPCM alignment
 *                              error detection
 *
 *  @param  debugTraceLevel:  This parameter configures the debug trace level
 *                            for the codec
 *
 *  @param  lastNFramesToLog:  This parameter configures the codec to maintain
 *                             a history of last N frames/pictures
 *
 *  @param enableDualOutput  :  This Parameter tells whether Dual output is
 *                             enable or not. If enable then application needs
 *                             to provide two extra buffer (one for DualLuma
 *                             and another for DualChroma.
 *
 *  @param processCallLevel : Flag to select field/frame level process call
 *
 *  @param enableWatermark  :  This Parameter tells whether watermark is
 *                             enabled or not.
 *
 *  @param decodeFrameType : Flag to decoder from application to request
 *                           decoding of only I & IDR or IP or all frame types.
 *                           Setting of IVIDDEC3_DynamicParams::frameSkipMode =
 *                           IVIDEO_SKIP_PB or IVIDEO_SKIP_B could have been
 *                           used for this purpose but it is defined at dynamic
 *                           level, whereas the intention of this parameter is
 *                           to have create time indication to codec for lesser
 *                           memory foot print request. Hence this new parameter
 *                           is defined.
 ******************************************************************************
*/
typedef struct IH264VDEC_Params
{
  IVIDDEC3_Params viddec3Params;
  XDAS_Int32      dpbSizeInFrames;
  XDAS_Int32      pConstantMemory;
  XDAS_Int32      bitStreamFormat;
  XDAS_UInt32     errConcealmentMode;
  XDAS_Int32      temporalDirModePred;
  XDAS_UInt32     svcExtensionFlag;
  XDAS_Int32      svcTargetLayerDID;
  XDAS_Int32      svcTargetLayerTID;
  XDAS_Int32      svcTargetLayerQID;
  XDAS_Int32      presetLevelIdc;
  XDAS_Int32      presetProfileIdc;
  XDAS_UInt32     detectCabacAlignErr;
  XDAS_UInt32     detectIPCMAlignErr;
  XDAS_UInt32     debugTraceLevel;
  XDAS_UInt32     lastNFramesToLog;
  XDAS_UInt32     enableDualOutput;
  XDAS_Int32      processCallLevel;
  XDAS_UInt32     enableWatermark;
  XDAS_UInt32     decodeFrameType;
} IH264VDEC_Params;
/**
 ******************************************************************************
 *  @struct IH264VDEC_DynamicParams
 *
 *  @brief  This structure defines the run-time parameters and any other
 *          implementation specific parameters for an H.264 instance object.
 *          The base run-time parameters are defined in the XDM data structure,
 *          IVIDDEC3_DynamicParams.
 *
 *  @param  viddec3DynamicParams : XDM Base class dynamic structure
 *                                 (see ividdec3.h)
 *
 *  @param  deblockFilterMode : indicates the mode of deblocking filter
 *                              (see enum IH264VDEC_deblockFilterMode)
 *
 *  @param  svcTargetLayerDID : SVC Spatial target layer ID,need to read if svc
 *                codec flag is on else default set to
 *                IH264SVCVDEC_TARGET_DEFAULT_DID.
 *
 *  @param  svcTargetLayerQID : SVC Quality target layer ID,need to read if svc
 *                codec flag is on else default set to
 *                IH264SVCVDEC_TARGET_DEFAULT_QID.
 *
 *
 *  @param  svcTargetLayerTID : SVC Temporal target layer ID,need to read if svc
 *                codec flag is on else default set to
 *                IH264SVCVDEC_TARGET_DEFAULT_TID.
 *
 *  @param  svcELayerDecode   : Flag to set for enhancement layer decode,
 *                              defaultvalue is IH264VDEC_DISABLE_ELAYERDECODE
 *
 *  @param reserved[3]      :  Left reserve for future purpose
 *
 ******************************************************************************
*/
typedef struct IH264VDEC_DynamicParams
{
  IVIDDEC3_DynamicParams viddec3DynamicParams;
  XDAS_Int32     deblockFilterMode;
  XDAS_Int32      svcTargetLayerDID;
  XDAS_Int32      svcTargetLayerTID;
  XDAS_Int32      svcTargetLayerQID;
  XDAS_Int32      svcELayerDecode;
  XDAS_Int32      reserved[3];
} IH264VDEC_DynamicParams;

/**
 ******************************************************************************
 *  @struct IH264VDEC_InArgs
 *
 *  @brief  This structure defines the run-time input arguments for an H264
 *          instance object (IH264VDEC::process)
 *
 *  @param  viddec3InArgs : XDM Base class InArgs structure  (see ividdec3.h)
 *
 *  @param  lateAcquireArg : XDM Base class InArgs structure  (see ividdec3.h)
 ******************************************************************************
*/
typedef struct IH264VDEC_InArgs
{
  IVIDDEC3_InArgs viddec3InArgs;
  XDAS_Int32 lateAcquireArg;
} IH264VDEC_InArgs;
/**
 ******************************************************************************
 *  @struct IH264VDEC_OutArgs
 *
 *  @brief  This structure defines the run time output arguments for
 *          IH264VDEC::process function
 *
 *  @param  viddec3OutArgs : XDM Base class OutArgs structure  (see ividdec3.h)
 *
 *  @param  decryptedKey : This variable contains watermark decrypted key.
 ******************************************************************************
*/
typedef struct IH264VDEC_OutArgs
{
  IVIDDEC3_OutArgs viddec3OutArgs;
  XDAS_UInt32 decryptedKey;
} IH264VDEC_OutArgs;

/*
 *  ======== IH264VDEC_Handle ========
 *  This handle is used to reference all H264VDEC instance objects
 */
typedef struct IH264VDEC_Obj *IH264VDEC_Handle;

/**
 ******************************************************************************
 *  @struct IH264VDEC_ProcessParams
 *
 *  @brief  This structure defines the container for holding the channel
 *          information.
 *
 *  @param  handle : Handle for the channel.
 *  @param  inBufs : Input Buffers for the Channel.
 *  @param  outBufs : Output Buffers for the Channel.
 *  @param  inArgs : Input Arguments for the Channel.
 *  @param  outArgs : Output Arguments for the Channel.
 ******************************************************************************
*/
typedef struct IH264VDEC_ProcessParams
{
  IH264VDEC_Handle handle;
  XDM2_BufDesc *inBufs;
  XDM2_BufDesc *outBufs;
  IVIDDEC3_InArgs *inArgs;
  IVIDDEC3_OutArgs *outArgs;
} IH264VDEC_ProcessParams;

/**
 ******************************************************************************
 *  @struct IH264VDEC_ProcessParamsList
 *
 *  @brief  This structure defines the container for holding the N channel
 *          information.
 *
 *  @param  numEntries : Number of channels in the given container.
 *  @param  processParams : Array holding the Process Parameters.
 ******************************************************************************
*/
typedef struct IH264VDEC_ProcessParamsList
{
  XDAS_Int32 numEntries ;
  IH264VDEC_ProcessParams processParams[IH264VDEC_MAX_LENGTH_PROCESS_LIST];
} IH264VDEC_ProcessParamsList ;

/**
 ******************************************************************************
 *  @struct IH264VDEC_Fxns
 *
 *  @brief  This structure contains pointers to all the XDAIS and XDM interface
 *          functions
 *
 *  @param  ividdec3 : This structure contains pointers to all the XDAIS and
 *                     XDM interface functions
 ******************************************************************************
*/
typedef struct IH264VDEC_Fxns
{
  IVIDDEC3_Fxns ividdec3;
  XDAS_Int32 (*processMulti) (IH264VDEC_ProcessParamsList *processList);
} IH264VDEC_Fxns;



/*
 *  ======== IH264VDEC_Cmd ========
 *  This structure defines the control commands for the IMP4VENC module.
 */
typedef IVIDDEC3_Cmd IH264VDEC_Cmd;

/*
 *  ======== IH264VDEC_PARAMS ========
 *  Default parameter values for H264VDEC instance objects
 */
extern const IH264VDEC_Params IH264VDEC_PARAMS;
/*
 *  ======== IH264VDEC_DYNAMICPARAMS ========
 *  Default dynamic parameter values for H264VDEC instance objects
 */
extern const IH264VDEC_DynamicParams IH264VDEC_TI_DYNAMICPARAMS;


/**
 ******************************************************************************
 *  @enum       IH264VDEC_deblockFilterMode
 *  @brief      This enum indicates the mode of deblocking filter
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DEBLOCK_DISABLE_NONE = 0,
  /**
  * Perform deblocking across all edges
  */
  IH264VDEC_DEBLOCK_DISABLE_ALL,
  /**
  * Disable deblocking across all edges
  */
  IH264VDEC_DEBLOCK_DISABLE_SLICE_EDGE,
  /**
  * Disable deblocking only at slice edges. Internal to slice,
  * edges are deblocked.
  */
  IH264VDEC_DEBLOCK_DEFAULT
  /**
  * Perform deblocking as controlled by disable_deblocking_filter_idc of
  * the bitstream
  */
} IH264VDEC_deblockFilterMode;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_temporalDirModePred
 *  @brief      This enum indicates whether or not to decode slices with
 *              temporal direct prediction
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DISABLE_TEMPORALDIRECT = 0,
  /**
  *  Do not decode slice with temporal direct
  */
  IH264VDEC_ENABLE_TEMPORALDIRECT
  /**
  * Decode slice with temporal direct
  */
} IH264VDEC_temporalDirModePred;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_detectCabacAlignErr
 *  @brief      This enum indicates whether or not to detect the cabac
 *              alignment errors in MB-decoding
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DISABLE_CABACALIGNERR_DETECTION = 0,
  /**
  *  Do not detect the CABAC alignment errors
  */
  IH264VDEC_ENABLE_CABACALIGNERR_DETECTION
  /**
  * Detect the CABAC alignment errors
  */
} IH264VDEC_detectCabacAlignErr;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_detectIPCMAlignErr
 *  @brief      This enum indicates whether or not to detect the IPCM
 *              alignment errors in MB-decoding
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DISABLE_IPCMALIGNERR_DETECTION = 0,
  /**
  *  Do not detect the IPCM alignment errors
  */
  IH264VDEC_ENABLE_IPCMALIGNERR_DETECTION
  /**
  * Detect the IPCM alignment errors
  */
} IH264VDEC_detectIPCMAlignErr;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_errConcealmentMode
 *  @brief      This enum indicates whether to apply error concealment or not
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_NO_CONCEALMENT = 0,
  /**
  *  do not apply error concealment
  */
  IH264VDEC_APPLY_CONCEALMENT
  /**
  * apply error concealment
  */
} IH264VDEC_errConcealmentMode;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_LevelId
 *
 *  @brief      This enum indicates preset level numbers for H.264 Decoder
 *
 *  The application should set the level in the create time parameters.
 ******************************************************************************
*/
typedef enum {
 IH264VDEC_LEVEL1 = 0,
  /** 0: Level 1
  */
 IH264VDEC_LEVEL1B,
  /** 1: Level 1.b
  */
 IH264VDEC_LEVEL11,
  /** 2: Level 1.1
  */
 IH264VDEC_LEVEL12,
  /** 3: Level 1.2
  */
 IH264VDEC_LEVEL13,
  /** 4: Level 1.3
  */
 IH264VDEC_LEVEL2,
  /** 5: Level 2
  */
 IH264VDEC_LEVEL21,
  /** 6: Level 2.1
  */
 IH264VDEC_LEVEL22,
  /** 7: Level 2.2
  */
 IH264VDEC_LEVEL3,
  /** 8: Level 3
  */
 IH264VDEC_LEVEL31,
  /** 9: Level 3.1
  */
 IH264VDEC_LEVEL32,
  /** 10: Level 3.2
  */
 IH264VDEC_LEVEL4,
  /** 11: Level 4
  */
 IH264VDEC_LEVEL41,
  /** 12: Level 4.1
  */
 IH264VDEC_LEVEL42,
  /** 13: Level 4.2
  */
 IH264VDEC_LEVEL5,
  /** 14: Level 5
  */
 IH264VDEC_LEVEL51,
  /** 15: Level 5.1
  */
IH264VDEC_MAXLEVELID = IH264VDEC_LEVEL51
  /** 15: Maximum Level ID that can be configured
  */
} IH264VDEC_LevelId;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_ProfileId
 *
 *  @brief      This enum indicates preset profiles for H.264 Decoder
 *
 *  The application should set the profile in the create time parameters.
 *  In the current implementation, the codec ignores this value.
 ******************************************************************************
*/
typedef enum {
 IH264VDEC_PROFILE_BASELINE = 0,
  /** 0: Baseline profile
  */
 IH264VDEC_PROFILE_MAIN,
  /** 1: Main profile
  */
 IH264VDEC_PROFILE_HIGH,
  /** 2: High profile
  */
 IH264VDEC_PROFILE_ANY
  /** 3: As decoded from the bitstream. This is needed to pass compliance.
  */
} IH264VDEC_ProfileId;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_debugTraceLevel
 *
 *  @brief      This enum indicates the debug trace levels for H.264 Decoder
 *
 *  The application should set this in the create time parameters.
 ******************************************************************************
*/
typedef enum {
 IH264VDEC_DEBUGTRACE_LEVEL0 = 0,
  /** 0: Debug Trace Level 0
  */
 IH264VDEC_DEBUGTRACE_LEVEL1,
  /** 1: Debug Trace Level 1
  */
 IH264VDEC_DEBUGTRACE_LEVEL2,
  /** 2: Debug Trace Level 2
  */
 IH264VDEC_DEBUGTRACE_LEVEL3
  /** 2: Debug Trace Level 3
  */
} IH264VDEC_debugTraceLevel;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_MetadataType
 *  @brief      This enum indicates Meta Data types for H.264 Decoder
 *
 *  The way to get meta data from decoder is via outBufs of the decoder during
 *  process call.
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_PARSED_SEI_DATA = XDM_CUSTOMENUMBASE,
  /** Write out Parsed SEI data
  *   By setting to this value(for IVIDDEC3_Params::metadataType[i])
  *   the codec can  can provide the parsed SEI metadata
  */

  IH264VDEC_ENCODED_SEI_DATA,
  /** Write out Encoded (compressed) SEI data
  *
  */

  IH264VDEC_PARSED_VUI_DATA,
  /** Write out Parsed VUI data
  *   By setting to this value(for IVIDDEC3_Params::metadataType[i])
  *   the codec can  can provide the parsed VUI metadata
  */
  IH264VDEC_ENCODED_VUI_DATA
  /** Write out Encoded (compressed) VUI data
  *
  */

} IH264VDEC_MetadataType;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_DataMode
 *  @brief      Describes the input slice format provided to decoder.
 *              This enumeration type is used by App to specify codec
 *              input slice format (NAL/Bytestream) type
 *
 ******************************************************************************
*/

typedef enum
{
  IH264VDEC_NALUNIT_MODE   = XDM_CUSTOMENUMBASE
  /** data in NAL stream format
  *
  */
} IH264VDEC_DataMode;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_bitStreamFormat
 *  @brief      Input bit stream format provided to decoder.
 *              This enumeration type is used by App to specify codec
 *              input bit stream format (NAL/Bytestream) type
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_BYTE_STREAM_FORMAT = 0,
  /** Input data is in Byte stream format (stream with start code).
  *
  */
  IH264VDEC_NAL_UNIT_FORMAT
  /** Input data is in NAL stream format (No start code)
  *
  */
} IH264VDEC_bitStreamFormat;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_mbErrStatus
 *  @brief      This enum indicates if a MB was in error or not
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_MB_NOERROR = 0,
  /**
  *  MB was non-erroneous
  */
  IH264VDEC_MB_ERROR = 1
  /**
  * MB was erroneous
  */
} IH264VDEC_mbErrStatus;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_ErrorBit
 *  @brief      H.264 Error Codes: Delaration of h264 decoder specific Error
 *              Codes.
 *  @details    Error status is communicated trough a 32 bit word. In this,
 *              Error Bits 8 to 15 are used to indicate the XDM error bits. See
 *              XDM_ErrorBit definition in xdm.h. Other bits in a 32 bit word
 *              can be used to signal any codec specific errors. The staructure
 *              below enumerates the H264 decoder specific error bits used.
 *              The algorithm can set multiple bits to 1 depending on the error
 *              condition
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_ERR_NOSLICE = 0,
    /**<
    * Bit 0
    *  1 - No error-free slice header detected in the frame
    *  0 - Ignore
    */
  IH264VDEC_ERR_SPS,
    /**<
    * Bit 1
    *  1 - Error in SPS parsing
    *  0 - Ignore
    */
  IH264VDEC_ERR_PPS,
    /**<
    * Bit 2
    *  1 -  Error during PPS parsing
    *  0 - Ignore
    */
  IH264VDEC_ERR_SLICEHDR,
    /**<
    * Bit 3
    *  1 - Error in slice header parsing
    *  0 - Ignore
    */
  IH264VDEC_ERR_MBDATA,
    /**<
    * Bit 4
    *  1 -  Error in MB data parsing
    *  0 - Ignore
    */
  IH264VDEC_ERR_UNAVAILABLESPS,
    /**<
    * Bit 5
    *  1 - SPS rferred in the header is not available.
    *  0 - Ignore
    */
  IH264VDEC_ERR_UNAVAILABLEPPS,
    /**<
    * Bit 6
    *  1 -  PPS rferred in the header is not available
    *  0 - Ignore
    */
  IH264VDEC_ERR_INVALIDPARAM_IGNORE,
    /**<
    * Bit 7
    *  1 -  Invalid Parameter
    *  0 - Ignore
    */
  IH264VDEC_ERR_UNSUPPFEATURE = 16,
      /**<
    * Bit 16
    *  1 -  Unsupported feature
    *  0 - Ignore
    */
  IH264VDEC_ERR_METADATA_BUFOVERFLOW,
      /**<
    * Bit 17
    *  1 - SEI Buffer overflow detected
    *  0 - Ignore
    */
  IH264VDEC_ERR_STREAM_END,
    /**<
    * Bit 18
    *  1 - End of stream reached
    *  0 - Ignore
    */
  IH264VDEC_ERR_NO_FREEBUF,
    /**<
    * Bit 19
    *  1 - No free buffers available for reference storing reference frame
    *  0 - Ignore
    */
  IH264VDEC_ERR_PICSIZECHANGE,
    /**<
    * Bit 20
    *  1 - Change in resolution detected
    *  0 - Ignore
    */
  IH264VDEC_ERR_UNSUPPRESOLUTION,
    /**<
    * Bit 21
    *  1 - Unsupported resolution by the decoder
    *  0 - Ignore
    */
  IH264VDEC_ERR_NUMREF_FRAMES,
    /**<
    * Bit 22
    *  1 - maxNumRefFrames parameter is not compliant to stream properties
    *     (does not comply to stream requirements).
    *  0 - Ignore
    */
  IH264VDEC_ERR_INVALID_MBOX_MESSAGE,
    /**<
    * Bit 23
    *  1 - Invalid (unexpected) mail box message received by M3 or IVAHD
    *  0 - Ignore
    */
  IH264VDEC_ERR_DATA_SYNC,
    /**<
    * Bit 24
    *  1 - In datasync enable mode, the input supplied is wrong
    *  0 - Ignore
    */
  IH264VDEC_ERR_MISSINGSLICE,
    /**<
    * Bit 25
    *  1 - Missing slice in a frame
    *  0 - Ignore
    */
  IH264VDEC_ERR_INPUT_DATASYNC_PARAMS,
    /**<
    * Bit 26
    *  1 - Input datasync enable mode, the input parameter is wrong
    *  0 - Ignore
    */
  IH264VDEC_ERR_HDVICP2_IMPROPER_STATE,
    /**<
    * Bit 27
    *  1 - IVAHD standby failed or couldn't turn-on/off the IP's clock
    *      or HDVICP reset failed.
    *  0 - Ignore
    */
  IH264VDEC_ERR_TEMPORAL_DIRECT_MODE,
    /**<
    * Bit 28
    *  1 - Temporal direct mode is present in the bits stream
    *      when disableTemporalDirect parameter (create time) is set.
    *  0 - Ignore
    */
  IH264VDEC_ERR_DISPLAYWIDTH,
    /**<
    * Bit 29
    *  1 - DisplayWidth is less than the Image width + Padded width.
    *  0 - Ignore
    */
  IH264VDEC_ERR_NOHEADER,
    /**<
    * Bit 30
    *  1 - Indicates that no SPS/PPS header is decoded in the current
    *      process call.
    * (or) It indicates that watermark SEI data is unavailable even though
    *      watermark parameter is enabled.
    *  0 - Ignore
    */
  IH264VDEC_ERR_GAPSINFRAMENUM
    /**<
    * Bit 31
    *  1 - Indicates that a gap is detected in frame_num for a stream with
    *      gaps_in_frame_num_value_allowed_flag 1 in SPS.
    *  0 - Ignore
    */
} IH264VDEC_ErrorBit;
/**
 ******************************************************************************
 *  @enum       IH264VDEC_svcExtension
 *  @brief      This enum indicates whether or not to support SVC extension
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DISABLE_SVCEXTENSION = 0,
  /**
  * Do not support SVC extension
  */
  IH264VDEC_ENABLE_SVCEXTENSION
  /**
  * Support SVC extension
  */
} IH264VDEC_svcExtension;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_eLayerDecodeMode
 *  @brief      This enum indicates whether or not to support decoding of
 *              enhancement layer
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DISABLE_ELAYERDECODE = 0,
  /**
  * Decode base layer only. Do not decode enhancement layer
  */
  IH264VDEC_ENABLE_ELAYERDECODE
  /**
  * Support decoding of enhancement layer
  */
} IH264VDEC_ELAYERDECODEMODE;


/**
 ******************************************************************************
 *  @enum       IH264VDEC_dependancyLayerIds
 *  @brief      This enum indicates the dependancy layer IDs.
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_TARGET_DID_DEFAULT = -1,
  /**
  * Default dependancy layer ID.
  */
  IH264VDEC_TARGET_DID_ZERO = 0,
  /**
  * Dependancy layer ID = 0
  */
  IH264VDEC_TARGET_DID_ONE,
  /**
  * Dependancy layer ID = 1
  */
  IH264VDEC_TARGET_DID_TWO,
  /**
  * Dependancy layer ID = 2
  */
  IH264VDEC_TARGET_DID_THREE,
  /**
  * Dependancy layer ID = 3
  */
  IH264VDEC_TARGET_DID_FOUR,
  /**
  * Dependancy layer ID = 4
  */
  IH264VDEC_TARGET_DID_FIVE,
  /**
  * Dependancy layer ID = 5
  */
  IH264VDEC_TARGET_DID_SIX,
  /**
  * Dependancy layer ID = 6
  */
  IH264VDEC_TARGET_DID_SEVEN,
  /**
  * Dependancy layer ID = 7
  */
  IH264VDEC_TARGET_DID_MAX =
  IH264VDEC_TARGET_DID_SEVEN
  /**
  * Dependancy layer max ID = 7
  */
  } IH264VDEC_dependancyLayerIds;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_temporalLayerIds
 *  @brief      This enum indicates the temporal layer IDs for svc.
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_TARGET_TID_DEFAULT = -1,
  /**
  * Default Temporal layer ID.
  */
  IH264VDEC_TARGET_TID_ZERO = 0,
  /**
  * Temporal layer ID = 0
  */
  IH264VDEC_TARGET_TID_ONE,
  /**
  * Temporal layer ID = 1
  */
  IH264VDEC_TARGET_TID_TWO,
  /**
  * Temporal layer ID = 2
  */
  IH264VDEC_TARGET_TID_THREE,
  /**
  * Temporal layer ID = 3
  */
  IH264VDEC_TARGET_TID_FOUR,
  /**
  * Temporal layer ID = 4
  */
  IH264VDEC_TARGET_TID_FIVE,
  /**
  * Temporal layer ID = 5
  */
  IH264VDEC_TARGET_TID_SIX,
  /**
  * Temporal layer ID = 6
  */
  IH264VDEC_TARGET_TID_SEVEN,
  /**
  * Temporal layer ID = 7
  */
  IH264VDEC_TARGET_TID_MAX =
  IH264VDEC_TARGET_TID_SEVEN
  /**
  * Temporal layer max ID = 7
  */
  } IH264VDEC_temporalLayerIds;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_qualityLayerIds
 *  @brief      This enum indicates the quality layer IDs for svc.
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_TARGET_QID_DEFAULT = -1,
  /**
  * Default Quality layer ID.
  */
  IH264VDEC_TARGET_QID_ZERO = 0,
  /**
  * Quality layer ID = 0
  */
  IH264VDEC_TARGET_QID_ONE,
  /**
  * Quality layer ID = 1
  */
  IH264VDEC_TARGET_QID_TWO,
  /**
  * Quality layer ID = 2
  */
  IH264VDEC_TARGET_QID_THREE,
  /**
  * Quality layer ID = 3
  */
  IH264VDEC_TARGET_QID_FOUR,
  /**
  * Quality layer ID = 4
  */
  IH264VDEC_TARGET_QID_FIVE,
  /**
  * Quality layer ID = 5
  */
  IH264VDEC_TARGET_QID_SIX,
  /**
  * Quality layer ID = 6
  */
  IH264VDEC_TARGET_QID_SEVEN,
  /**
  * Quality layer ID = 7
  */
  IH264VDEC_TARGET_QID_EIGHT,
  /**
  * Quality layer max ID = 8
  */
  IH264VDEC_TARGET_QID_NINE,
  /**
  * Quality layer max ID = 9
  */
  IH264VDEC_TARGET_QID_TEN,
  /**
  * Quality layer max ID = 10
  */
  IH264VDEC_TARGET_QID_ELEVEN,
  /**
  * Quality layer max ID = 11
  */
  IH264VDEC_TARGET_QID_TWELVE,
  /**
  * Quality layer max ID = 12
  */
  IH264VDEC_TARGET_QID_THIRTEEN,
  /**
  * Quality layer max ID = 13
  */
  IH264VDEC_TARGET_QID_FOURTEEN,
  /**
  * Quality layer max ID = 14
  */
  IH264VDEC_TARGET_QID_FIFTEEN,
  /**
  * Quality layer max ID = 15
  */
  IH264VDEC_TARGET_QID_MAX =
  IH264VDEC_TARGET_QID_FIFTEEN
  /**
  * Quality layer max ID = 15
  */
} IH264VDEC_qualityLayerIds;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_seiOverFlowFlag
 *  @brief      This enum indicates user data reg/unreg SEI overFlowFlag values
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_PAYLOAD_NO_OVERFLOW = 0,
  /**
  * Indicates there is no overflow occured in user data reg or unreg sei
  */
  IH264VDEC_PAYLOAD_OVERFLOW
  /**
  * Indicates there is a overflow in user data reg or unreg sei
  */
} IH264VDEC_seiOverFlowFlag;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_enableDualOutput
 *  @brief      This enum is used to enable/disable dual output feature
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DUALOUTPUT_DISABLE = 0,
  /**
  * Indicates that dual output is disabled
  */
  IH264VDEC_DUALOUTPUT_ENABLE
  /**
  * Indicates that dual output is enabled
  */
} IH264VDEC_enableDualOutput;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_processCallLevel
 *  @brief      This enum indicates whether process call is done at a field
 *              level or frame level
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_FIELDLEVELPROCESSCALL = 0,
  /**
  * Indicates that process call should be at field level
  */
  IH264VDEC_FRAMELEVELPROCESSCALL
  /**
  * Indicates that process call should be at frame level
  */
} IH264VDEC_processCallLevel;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_enableWaterMark
 *  @brief      This enum is used to enable/disable Watermark feature
 *
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_WATERMARK_DISABLE = 0,
  /**
  * Indicates that Watermark is disabled
  */
  IH264VDEC_WATERMARK_ENABLE
  /**
  * Indicates that Watermark is enabled
  */
} IH264VDEC_enableWaterMark;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_decodeFrameType
 *  @brief      This enum is used to request decoder to decode only I, IP or ALL
 *              frame types
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DECODE_ALL = 0,
  /**
  * Indicates that all type of frames decoding is enabled
  */
   IH264VDEC_DECODE_IP_ONLY = 1,
  /**
  * Indicates that only I/IDR and P frames decoding is enabled
  */
   IH264VDEC_DECODE_I_ONLY = 2
  /**
  * Indicates that only I/IDR frames decoding is enabled
  */
} IH264VDEC_decodeFrameType;

/**
 *  Macro definitions required for SEI support: HRD sequence parameter set
 */
#define IH264VDEC_MAXCPBCNT        32

/**
 *  Macro definitions required for SEI support: HRD sequence parameter set
 */
#define IH264VDEC_MAXUSERDATA_PAYLOAD 300

/**
 ******************************************************************************
 *  @struct IH264VDEC_HrdParams
 *
 *  @brief  This structure contains the HRD parameter elements.
 *
 *  @param  cpb_cnt_minus1 : Number of alternative CPB specifications in the
 *                           bit-stream
 *  @param  bit_rate_scale : Together with bit_rate_value[i], it specifies the
 *                           maximum input bit-rate for the ith CPB.
 *  @param  cpb_size_scale : Together with cpb_size_value[i], specifies the
 *                           maximum CPB size for the ith CPB.
 *  @param  bit_rate_value_minus1[IH264VDEC_MAXCPBCNT] :Maximum input bitrate
 *                                                     for the ith CPB
 *  @param  cpb_size_value_minus1[IH264VDEC_MAXCPBCNT] :Maximum CPB size for the
 *                                                     ith CPB
 *  @param  vbr_cbr_flag[IH264VDEC_MAXCPBCNT] :Specifies the ith CPB is operated
 *                          in Constant Bit-rate mode or variable bit-rate mode
 *  @param  initial_cpb_removal_delay_length_minus1 :Length in bits of
 *                                   initial_cpb_removal_length syntax element
 *  @param  cpb_removal_delay_length_minus1 :Length in bits of
 *                                      cpb_removal_delay_length syntax element
 *  @param  dpb_output_delay_length_minus1 :Length in bits of
 *                                       dpb_output_delay_length syntax element
 *  @param  time_offset_length : Length in bits of time_offset syntax element
 ******************************************************************************
*/
typedef struct IH264VDEC_HrdParams
{
  XDAS_UInt32  cpb_cnt_minus1;
  XDAS_UInt8   bit_rate_scale;
  XDAS_UInt8   cpb_size_scale;
  XDAS_UInt32  bit_rate_value_minus1[IH264VDEC_MAXCPBCNT];
  XDAS_UInt32  cpb_size_value_minus1[IH264VDEC_MAXCPBCNT];
  XDAS_UInt8   vbr_cbr_flag[IH264VDEC_MAXCPBCNT];
  XDAS_UInt8   initial_cpb_removal_delay_length_minus1;
  XDAS_UInt8   cpb_removal_delay_length_minus1;
  XDAS_UInt8   dpb_output_delay_length_minus1;
  XDAS_UInt8   time_offset_length;
} IH264VDEC_HrdParams;

/**
 ******************************************************************************
 *  @struct IH264VDEC_SVCVuiParams
 *
 *  @brief   This structure contains VUI  message syntax elements for scalable
 *           video stream
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call, c
 *                            contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *
 *  @param  svc_vui_ext_num_entries_minus1:(svc_vui_ext_num_entries_minus1 + 1)
 *                      specifies the number of information
 *                        entries that are present in the SVC
 *                      VUI parameters extension syntax
 *                        structure
 *  @param  svc_vui_ext_dependency_id:indicate the max value of DId for the
 *                    i-th subset of coded video sequences
 *  @param  svc_vui_ext_quality_id:indicate the max value of QId for the
 *                    i-th subset of coded video sequences
 *  @param  svc_vui_ext_temporal_id: indicate the max value of TId for the
 *                    i-th subset of coded video sequences
 *  @param  svc_vui_ext_timing_info_present_flag: Flag to tells that
 *                          svc_vui_ext_num_units_in_tick,
 *                          svc_vui_ext_time_scale,
 *                          svc_vui_ext_fixed_frame_rate_flag
 *                          are present for current coded
 *                          sequence or not.
 *  @param  svc_vui_ext_num_units_in_tick: specifies the value of
 *                                         num_units_in_tick
 *  @param  svc_vui_ext_time_scale: specifies the value of time_scale
 *  @param  svc_vui_ext_fixed_frame_rate_flag: specifies the value of
 *                         fixed_frame_rate_flag
 *  @param  svc_vui_ext_nal_hrd_parameters_present_flag:specifies the
 *                value of nal_hrd_parameters_present_flag
 *  @param  svc_vui_ext_vcl_hrd_parameters_present_flag: ] specifies the
 *                value of vcl_hrd_parameters_present_flag
 *  @param  svc_vui_ext_low_delay_hrd_flag: specifies the value
 *          of low_delay_hrd_flag
 *  @param  svc_vui_ext_pic_struct_present_flag: specifies the value
 *           of pic_struct_present_flag
 *
 ******************************************************************************
*/

typedef struct sIH264VDEC_SVCVuiParams
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt16 svc_vui_ext_num_entries_minus1;
  XDAS_UInt16 svc_vui_ext_dependency_id;
  XDAS_UInt16 svc_vui_ext_quality_id;
  XDAS_UInt16 svc_vui_ext_temporal_id;
  XDAS_UInt16 svc_vui_ext_timing_info_present_flag;
  XDAS_UInt32 svc_vui_ext_num_units_in_tick;
  XDAS_UInt32 svc_vui_ext_time_scale;
  XDAS_UInt16 svc_vui_ext_fixed_frame_rate_flag;
  XDAS_UInt16 svc_vui_ext_nal_hrd_parameters_present_flag;
  XDAS_UInt16 svc_vui_ext_vcl_hrd_parameters_present_flag;
  XDAS_UInt16 svc_vui_ext_low_delay_hrd_flag;
  XDAS_UInt16 svc_vui_ext_pic_struct_present_flag;
} IH264VDEC_SVCVuiParams;

/**
 ******************************************************************************
 *  @struct IH264VDEC_VuiParams
 *
 *  @brief  This structure contains the VUI Sequence Parameter elements.
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call, c
 *                            contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  aspect_ratio_info_present_flag :Indicates whether aspect ratio idc
 *                                          is present or not.
 *  @param  aspect_ratio_idc : Aspect ratio of Luma samples
 *  @param  sar_width : Horizontal size of sample aspect ratio
 *  @param  sar_height : Vertical size of sample aspect ratio
 *  @param  overscan_info_present_flag : Cropped decoded pictures are suitable
 *                                       for display or not.
 *  @param  overscan_appropriate_flag : Overscan_appropriate_flag
 *  @param  video_signal_type_present_flag : Flag indicates whether
 *          video_format, video_full_range_flag and colour_description_present_
 *          flag are present or not
 *  @param  video_format :Video format indexed by a table. For example,PAL/NTSC
 *  @param  video_full_range_flag : Black level, luma and chroma ranges. It
 *                                  should be used for BT.601 compliance
 *  @param  colour_description_present_flag:Indicates whether colour_primaries,
 *                transfer_characteristics and matrix_coefficients are present.
 *  @param  colour_primaries :Chromaticity co-ordinates of source primaries
 *  @param  transfer_characteristics :Opto-electronic transfer characteristics
 *          of the source picture
 *  @param  matrix_coefficients :Matrix coefficients for deriving Luma and
 *          chroma data from RGB components.
 *  @param  chroma_location_info_present_flag : Flag indicates whether
 *          chroma_sample_loc_type_top field and chroma_sample_loctype
 *          bottom_field are present.
 *  @param  chroma_sample_loc_type_top_field : Location of chroma_sample top
 *          field
 *  @param  chroma_sample_loc_type_bottom_field :Location of chroma_sample
 *          bottom field
 *  @param  timing_info_present_flag :Indicates whether num_units_in_tick,
 *          time_scale, and fixed_frame_rate_flag are present.
 *  @param  num_units_in_tick :Number of units of a clock that corresponds to 1
 *          increment of a clock tick counter
 *  @param  time_scale :Indicates actual increase in time for 1 increment of a
 *          clock tick counter
 *  @param  fixed_frame_rate_flag :Indicates how the temporal distance between
 *          HRD output times of any two output pictures is constrained
 *  @param  nal_hrd_parameters_present_flag :Indicates whether
 *          nal_hrd_parameters are present
 *  @param  nal_hrd_pars : NAL HRD Parameters
 *  @param  vcl_hrd_parameters_present_flag :Indicates whether
 *          vcl_hrd_parameters are present
 *  @param  vcl_hrd_pars : VCL HRD Parameters
 *  @param  low_delay_hrd_flag :HRD operational mode as in Annex C of the
 *          standard
 *  @param  pic_struct_present_flag :Indicates whether picture timing SEI
 *          messages are present
 *  @param  bitstream_restriction_flag :Indicates if the bit-stream restriction
 *          parameters are present
 *  @param  motion_vectors_over_pic_boundaries_flag :Specifies whether motion
 *          vectors can point to regions outside the picture boundaries
 *  @param  max_bytes_per_pic_denom :Maximum number of bytes not exceeded by
 *          the sum of sizes of all VCL NAL units of a single coded picture
 *  @param  max_bits_per_mb_denom :Maximum number of bits taken by any coded MB
 *  @param  log2_max_mv_length_vertical :Maximum value of any motion vector’s
 *          vertical component
 *  @param  log2_max_mv_length_horizontal :Maximum value of any motion vector’s
 *           horizontal component
 *  @param  max_dec_frame_reordering :
 *  @param  num_reorder_frames :Maximum number of frames that need to be
 *          re-ordered
 *  @param  max_dec_frame_buffering :Size of HRD decoded buffer (DPB) in terms
 *          of frame buffers
 *  @param  svcVuiParams :  struct instance of vui parameters for svc
 *
 ******************************************************************************
*/
typedef struct IH264VDEC_VuiParams
{
  XDAS_UInt32  parsed_flag;
  XDAS_UInt8   aspect_ratio_info_present_flag;
  XDAS_UInt32  aspect_ratio_idc;
  XDAS_UInt32  sar_width;
  XDAS_UInt32  sar_height;
  XDAS_UInt8   overscan_info_present_flag;
  XDAS_UInt8   overscan_appropriate_flag;
  XDAS_UInt8   video_signal_type_present_flag;
  XDAS_UInt8   video_format;
  XDAS_UInt8   video_full_range_flag;
  XDAS_UInt8   colour_description_present_flag;
  XDAS_UInt8   colour_primaries;
  XDAS_UInt8   transfer_characteristics;
  XDAS_UInt8   matrix_coefficients;
  XDAS_UInt8   chroma_location_info_present_flag;
  XDAS_UInt32  chroma_sample_loc_type_top_field;
  XDAS_UInt32  chroma_sample_loc_type_bottom_field;
  XDAS_UInt8   timing_info_present_flag;
  XDAS_UInt32  num_units_in_tick;
  XDAS_UInt32  time_scale;
  XDAS_UInt8   fixed_frame_rate_flag;
  XDAS_UInt8   nal_hrd_parameters_present_flag;
  IH264VDEC_HrdParams   nal_hrd_pars;
  XDAS_UInt8   vcl_hrd_parameters_present_flag;
  IH264VDEC_HrdParams   vcl_hrd_pars;
  XDAS_UInt8   low_delay_hrd_flag;
  XDAS_UInt8   pic_struct_present_flag;
  XDAS_UInt8   bitstream_restriction_flag;
  XDAS_UInt8   motion_vectors_over_pic_boundaries_flag;
  XDAS_UInt32  max_bytes_per_pic_denom;
  XDAS_UInt32  max_bits_per_mb_denom;
  XDAS_UInt32  log2_max_mv_length_vertical;
  XDAS_UInt32  log2_max_mv_length_horizontal;
  XDAS_UInt32  max_dec_frame_reordering;
  XDAS_UInt32  num_reorder_frames;
  XDAS_UInt32  max_dec_frame_buffering;
  IH264VDEC_SVCVuiParams svcVuiParams;
} IH264VDEC_VuiParams;

/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiUserDataRegITUT
 *
 *  @brief  This structure contains the user data SEI msg elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  num_payload_bytes :Specifies the size of the payload
 *  @param  itu_t_t35_country_code : A byte having a value specified as a
 *                          country code by ITU-T Recommendation T.35 Annex A
 *  @param  itu_t_t35_country_code_extension_byte :A byte having a value
 *          specified as a country code by ITU-T Recommendation T.35 Annex B
 *  @param  itu_t_t35_payload_byte[] : A byte containing data registered as
 *          specified by ITU-T Recommendation T.35.
 *  @param  dataOverflowFlag: This indicates if pay load data is more than the
 *                            array size i.e., IH264VDEC_MAXUSERDATA_PAYLOAD.
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiUserDataRegITUT
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 num_payload_bytes;
  XDAS_UInt8  itu_t_t35_country_code;
  XDAS_UInt8  itu_t_t35_country_code_extension_byte;
  XDAS_UInt8  itu_t_t35_payload_byte[IH264VDEC_MAXUSERDATA_PAYLOAD];
  XDAS_UInt8  dataOverflowFlag;
} IH264VDEC_SeiUserDataRegITUT;

/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiUserDataUnReg
 *
 *  @brief  This structure contains the user data SEI msg elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  num_payload_bytes :Specifies the size of the payload
 *  @param  uuid_iso_iec_11578 :Value specified as a UUID according to the
 *                              procedures of ISO/IEC 11578:1996 Annex A.
 *  @param  user_data_payload_byte :Byte containing data having syntax and
 *                                semantics as specified by the UUID generator.
 *  @param  dataOverflowFlag: This indicates if pay load data is more than the
 *                            array size i.e., IH264VDEC_MAXUSERDATA_PAYLOAD.
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiUserDataUnReg
{
  XDAS_UInt32  parsed_flag;
  XDAS_UInt32  num_payload_bytes;
  XDAS_UInt8   uuid_iso_iec_11578[16];
  XDAS_UInt8   user_data_payload_byte[IH264VDEC_MAXUSERDATA_PAYLOAD];
  XDAS_UInt8   dataOverflowFlag;
} IH264VDEC_SeiUserDataUnReg;


/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiBufferingPeriod
 *
 *  @brief   This structure contains the buffering period SEI msg elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  seq_parameter_set_id :Specifies the sequence parameter set that
 *                                contains the sequence HRD attributes
 *  @param  nal_cpb_removal_delay :Specifies the delay for the indexed NAL CPB
 *          between the time of arrival in the CPB of the first bit of the
 *          coded data associated with the access unit associated with the
 *          buffering period SEI message and the time of removal from the CPB
 *          of the coded data associated with the same access unit, for the
 *          first buffering period after HRD initialization.
 *  @param  nal_cpb_removal_delay_offset :Used for the indexed NAL CPB in
 *          combination with the cpb_removal_delay to specify the initial
 *          delivery time of coded access units to the CPB
 *  @param  vcl_cpb_removal_delay :Specifies the delay for the indexed VCL CPB
 *          between the time of arrival in the CPB of the first bit of the
 *          coded data associated with the access unit associated with the
 *          buffering period SEI message and the time of removal from the CPB
 *          of the coded data associated with the same access unit, for the
 *          first buffering period after HRD initialization.
 *  @param  vcl_cpb_removal_delay_offset :Used for the indexed VCL CPB in
 *          combination with the cpb_removal_delay to specify the initial
 *          delivery time of coded access units to the CPB
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiBufferingPeriod
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 seq_parameter_set_id;
  XDAS_UInt32 nal_cpb_removal_delay[IH264VDEC_MAXCPBCNT];
  XDAS_UInt32 nal_cpb_removal_delay_offset[IH264VDEC_MAXCPBCNT];
  XDAS_UInt32 vcl_cpb_removal_delay[IH264VDEC_MAXCPBCNT];
  XDAS_UInt32 vcl_cpb_removal_delay_offset[IH264VDEC_MAXCPBCNT];
}IH264VDEC_SeiBufferingPeriod;
/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiPanScanRect
 *
 *  @brief   This structure contains the pan scan rectangle SEI msg elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  pan_scan_rect_id :Specifies an identifying number that may be used
 *                            to identify the purpose of the pan-scan rectangle
 *  @param  pan_scan_rect_cancel_flag :Equal to 1 indicates that the SEI
 *                    message cancels the persistence of any previous pan-scan
 *                    rectangle SEI message in output order.
 *                    pan_scan_rect_cancel_flag equal to 0 indicates that
 *                    pan-scan rectangle information follows.
 *  @param  pan_scan_cnt_minus1 :Specifies the number of pan-scan rectangles
 *          that are present in the SEI message
 *  @param  pan_scan_rect_left_offset :Specifies as signed integer quantities
 *          in units of one-sixteenth sample spacing relative to the luma
 *          sampling grid, the location of the pan-scan rectangle
 *  @param  pan_scan_rect_right_offset :Specifies as signed integer quantities
 *          in units of one-sixteenth sample spacing relative to the luma
 *          sampling grid, the location of the pan-scan rectangle
 *  @param  pan_scan_rect_top_offset : Top offset
 *  @param  pan_scan_rect_bottom_offset : Bottom offset
 *  @param  pan_scan_rect_repetition_period :Specifies the persistence of the
 *          pan-scan rectangle SEI message and may specify a picture order
 *          count interval within which another pan-scan rectangle SEI message
 *          with the same value of pan_scan_rect_id or the end of the coded
 *          video sequence shall be present in the bit-stream
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiPanScanRect
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 pan_scan_rect_id;
  XDAS_UInt32  pan_scan_rect_cancel_flag;
  XDAS_UInt32 pan_scan_cnt_minus1;
  XDAS_Int32 pan_scan_rect_left_offset[3];
  XDAS_Int32 pan_scan_rect_right_offset[3];
  XDAS_Int32 pan_scan_rect_top_offset[3];
  XDAS_Int32 pan_scan_rect_bottom_offset[3];
  XDAS_UInt32 pan_scan_rect_repetition_period;
} IH264VDEC_SeiPanScanRect;

/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiProgRefineStart
 *
 *  @brief  This structure contains the progressive refinement start SEI msg
 *          elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  progressive_refinement_id :Specifies an identification number for
 *          the progressive refinement operation.
 *  @param  num_refinement_steps_minus1 :Specifies the number of reference
 *          frames in the tagged set of consecutive coded pictures
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiProgRefineStart
{
    XDAS_UInt32 parsed_flag;
    XDAS_UInt32 progressive_refinement_id;
    XDAS_UInt32 num_refinement_steps_minus1;
} IH264VDEC_SeiProgRefineStart;
/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiProgRefineEnd
 *
 *  @brief  TThis structure contains the progressive refinement end SEI msg
 *          elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  progressive_refinement_id :Specifies an identification number for
 *                    the progressive refinement operation.
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiProgRefineEnd
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 progressive_refinement_id;
} IH264VDEC_SeiProgRefineEnd;
/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiRecoveryPointInfo
 *
 *  @brief  This structure contains the sRecovery Point Info SEI msg elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  exact_match_flag :Indicates whether decoded pictures at and
 *          subsequent to the specified recovery point in output order derived
 *          by starting the decoding process at the access unit associated with
 *          the recovery point SEI message, will be an exact match to the
 *          pictures that would be produced by starting the decoding process
 *        at the location of a previous IDR access unit in the NAL unit stream.
 *  @param  recovery_frame_cnt :Specifies the recovery point of output pictures
 *          in output order
 *  @param  broken_link_flag :Indicates the presence or absence of a broken
 *                            link in the NAL unit stream
 *  @param  changing_slice_group_idc :Indicates whether decoded pictures are
 *          correct or approximately correct in content at and subsequent to
 *          the recovery point in output order when all macro-blocks of the
 *          primary coded pictures are decoded within the changing slice group
 *          period.
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiRecoveryPointInfo
{
  XDAS_UInt32  parsed_flag;
  XDAS_UInt32  recovery_frame_cnt;
  XDAS_UInt32  exact_match_flag;
  XDAS_UInt32  broken_link_flag;
  XDAS_UInt32  changing_slice_group_idc;
} IH264VDEC_SeiRecoveryPointInfo;

/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiPictureTiming
 *
 *  @brief  This structure contains the picture timing SEI msg elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param     NumClockTs :
 *  @param     cpb_removal_delay :Specifies how many clock ticks to wait after
 *               removal from the CPB of the access unit associated with the
 *               most recent buffering period SEI message before removing from
 *               the buffer the access unit data associated with the picture
 *               timing SEI message.
 *  @param     dpb_output_delay : Used to compute the DPB output time of the
 *               picture.
 *  @param     pic_struct : Indicates whether a picture  should be displayed as
 *               a frame or field
 *  @param     clock_time_stamp_flag[4]:1 - Indicates number of clock timestamp
 *                            syntax elements present and follow immediately
 *                            0 – Indicates associated clock timestamp syntax
 *                                elements not present
 *  @param     ct_type[4] : Indicates the scan type(interlaced or progressive)
 *                          of the source material
 *  @param     nuit_field_based_flag[4] : Used to calculate the clockTimestamp
 *  @param     counting_type[4] : Specifies the method of dropping values of
 *                                n_frames
 *  @param     full_timestamp_flag[4] : 1 - Specifies that the n_frames syntax
 *                                      element is followed by seconds_value,
 *                                      minutes_value, and hours_value.
 *                                      0 - Specifies that the n_frames syntax
 *                                      element is followed by seconds_flag
 *  @param     discontinuity_flag[4] : Indicates whether the difference between
 *               the current value of clockTimestamp and the value of
 *               clockTimestamp computed from the previous clockTimestamp in
 *               output order can be interpreted as the time difference between
 *               the times of origin or capture of the associated frames or
 *               fields.
 *  @param     cnt_dropped_flag[4] : Specifies the skipping of one or more
 *               values of n_frames using the counting method
 *  @param     n_frames[4] : Specifies the value of nFrames used to compute
 *                            clockTimestamp.
 *  @param     seconds_flag[4] : equal to 1 specifies that seconds_value and
 *                               minutes_flag are present when
 *                               full_timestamp_flag is equal to 0.
 *  @param     minutes_flag[4] : equal to 1 specifies that minutes_value and
 *                              hours_flag are present when full_timestamp_flag
 *                              is equal to 0 and seconds_flag is equal to 1.
 *  @param     hours_flag[4] :  equal to 1 specifies that hours_value is
 *                              present when full_timestamp_flag is equal to 0
 *                              and seconds_flag is equal to 1 and minutes_flag
 *                              is equal to 1.
 *  @param     seconds_value[4] : Specifies the value of sS used to compute
 *                                clockTimestamp.
 *  @param     minutes_value[4] : Specifies the value of mM used to compute
 *                                clockTimestamp.
 *  @param     hours_value[4] : Specifies the value of tOffset used to compute
 *                              clockTimestamp
 *  @param     time_offset[4] : Specifies the value of tOffset used to compute
 *                              clockTimestamp
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiPictureTiming
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 NumClockTs;
  XDAS_UInt32 cpb_removal_delay;
  XDAS_UInt32 dpb_output_delay;
  XDAS_UInt32 pic_struct;
  XDAS_UInt32 clock_time_stamp_flag[4];
  XDAS_UInt32 ct_type[4];
  XDAS_UInt32 nuit_field_based_flag[4];
  XDAS_UInt32 counting_type[4];
  XDAS_UInt32 full_timestamp_flag[4];
  XDAS_UInt32 discontinuity_flag[4];
  XDAS_UInt32 cnt_dropped_flag[4];
  XDAS_UInt32 n_frames[4];
  XDAS_UInt32 seconds_flag[4];
  XDAS_UInt32 minutes_flag[4];
  XDAS_UInt32 hours_flag[4];
  XDAS_UInt32 seconds_value[4];
  XDAS_UInt32 minutes_value[4];
  XDAS_UInt32 hours_value[4];
  XDAS_Int32 time_offset[4];
}IH264VDEC_SeiPictureTiming;
/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiFullFrameFreezeRep
 *
 *  @brief  This structure contains the full frmae freeze repetition SEI msg
 *          elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  full_frame_freeze_repetition_period :Specifies the persistence of
 *            the full-frame freeze SEI message
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiFullFrameFreezeRep
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 full_frame_freeze_repetition_period;
} IH264VDEC_SeiFullFrameFreezeRep;

/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiFullFrameFreezeRel
 *
 *  @brief   This structure contains frame freeze release SEI msg elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  payloadSize : Size of the frame_freeze_release payload
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiFullFrameFreezeRel
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 payloadSize;
} IH264VDEC_SeiFullFrameFreezeRel;

/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiStereoVideoInfo
 *
 *  @brief   This structure contains stereo video information SEI msg elements
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  field_views_flag :   1 - indicates that all pictures in the current
 *                  coded video sequence are fields
 *                0 - indicates that all pictures in the current
 *                  coded video sequence are frames.
 *  @param  top_field_is_left_view_flag :
 *                1 - top field is a left  view.
 *                0 - topfield is right view.
 *  @param  current_frame_is_left_view_flag :
 *                1 - current frame is left view.
 *                0 - current frame is right view.
 *  @param  next_frame_is_second_view_flag :
 *                1 - current picture and a next picture in
 *                  output order form a stereo video pair.
 *                0 - current picture and a previous picture in
 *                  output order form a stereo video pair.
 *  @param  left_view_self_contained_flag :
 *                1 - it will not use right view as a reference
 *                  picture for inter prediction
 *                0 - it may use right view as a reference
 *                  picture for inter prediction.
 *  @param  right_view_self_contained_flag :
 *                1 - it will not use left view as a reference
 *                  picture for inter prediction
 *                0 - it may use left view as a reference
 *                  picture for inter prediction.
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiStereoVideoInfo
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 field_views_flag;
  XDAS_UInt32 top_field_is_left_view_flag;
  XDAS_UInt32 current_frame_is_left_view_flag;
  XDAS_UInt32 next_frame_is_second_view_flag;
  XDAS_UInt32 left_view_self_contained_flag;
  XDAS_UInt32 right_view_self_contained_flag;
} IH264VDEC_SeiStereoVideoInfo;

/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiFramePacking
 *
 *  @brief  This structure contains frame packing arrangement SEI msg elements
 *
 *  @param  frame_packing_arrangement_id :
 *                contains an identifying number that may be used to identify
 *                the usage of the frame packing arrangement SEI message.
 *  @param  frame_packing_arrangement_cancel_flag :
 *                1 - equal to 1 indicates that the frame packing arrangement
 *                    SEI message cancels the persistence of any previous frame
 *                    packing arrangement SEI message in output order.
 *                0 - indicates that frame packing arrangement info follows
 *  @param  frame_packing_arrangement_type :
 *                indicates the type of packing arrangement of the frames
 *  @param  quincunx_sampling_flag :
 *                1 - indicates that each color component plane of each
 *                    constituent frame is quincunx sampled
 *                0 - indicates that each color component plane of each
 *                    constituent frame is not quincunx sampled
 *  @param  content_interpretation_type :
 *                1 - frame 0 being associated with the left view and frame 1
 *                    being associated with the right view
 *                2 - frame 0 being associated with the right view and frame 1
 *                    being associated with the left view
 *  @param  spatial_flipping_flag :
 *                1 - spatial flipping is enabled for any one of the frame
 *                    constituent, if frame_packing_arrangement_type is 3 or 4.
 *                0 - spatial flipping is disabled for any one of the frame
 *                    constituent, if frame_packing_arrangement_type is 3 or 4.
 *  @param  frame0_flipped_flag :
 *                1 - frame 0 is spatially flipped
 *                0 - frame 1 is spatially flipped
 *  @param  field_views_flag :
 *                1 - indicates that all pictures in the current coded video
 *                   sequence are coded as complementary field pairs.
 *                0 - indicates that all pictures in the current coded video
 *                   sequence are coded as frame.
 *  @param  current_frame_is_frame0_flag :
 *                1 - indicates that the current decoded frame is constituent
 *                    frame 0 and the next decoded frame in output order
 *                    is constituent frame 1.
 *                0 - indicates that the current decoded frame is constituent
 *                    frame 1 and the next decoded frame in output order
 *                    is constituent frame 0.
 *  @param  frame0_self_contained_flag :
 *                1 - indicates that the constituent frame 0 is dependent on
 *                    constituent frame 1 in decoding process
 *                0 - indicates that the constituent frame 0 may dependent on
 *                    constituent frame 1 in decoding process
 *  @param  frame1_self_contained_flag :
 *                1 - indicates that the constituent frame 1 is dependent on
 *                    constituent frame 0 in decoding process
 *                0 - indicates that the constituent frame 1 may dependent on
 *                    constituent frame 0 in decoding process
 *  @param  frame0_grid_position_x :
 *                specifies the horizontal location of the upper left
 *                sample of constituent frame 0 in the units of one
 *                sixteenth of the luma samples
 *  @param  frame0_grid_position_y :
 *                specifies the vertical location of the upper left
 *                sample of constituent frame 0 in the units of one
 *                sixteenth of the luma samples
 *  @param  frame1_grid_position_x :
 *                specifies the horizontal location of the upper left
 *                sample of constituent frame 1 in the units of one
 *                sixteenth of the luma samples
 *  @param  frame1_grid_position_y :
 *                specifies the vertical location of the upper left
 *                sample of constituent frame 1 in the units of one
 *                sixteenth of the luma samples
 *  @param  frame_packing_arrangement_reserved_byte :
 *                reserved for the future use.
 *  @param  frame_packing_arrangement_repetition_period :
 *                specifies the persistence of the frame packing arrangement
 *                SEI message and may specify a frame order count interval
 *                within which another frame packing arrangement SEI message
 *                with the same value of frame_packing_arrangement_id or the
 *                end of the coded video sequence shall be present in the
 *                bitstream.
 *  @param  frame_packing_arrangement_extension_flag :
 *                0 - indicates that no additional data follows within the
 *                    frame packing arrangement SEI message.
 *                1 - Reserved for the future use.
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiFramePacking
{
  XDAS_UInt32 parsed_flag;
  XDAS_UInt32 frame_packing_arrangement_id;
  XDAS_UInt32 frame_packing_arrangement_repetition_period;
  XDAS_UInt8  frame_packing_arrangement_cancel_flag;
  XDAS_UInt8  frame_packing_arrangement_type;
  XDAS_UInt8  quincunx_sampling_flag;
  XDAS_UInt8  content_interpretation_type;
  XDAS_UInt8  spatial_flipping_flag;
  XDAS_UInt8  frame0_flipped_flag;
  XDAS_UInt8  field_views_flag;
  XDAS_UInt8  current_frame_is_frame0_flag;
  XDAS_UInt8  frame0_self_contained_flag;
  XDAS_UInt8  frame1_self_contained_flag;
  XDAS_UInt8  frame0_grid_position_x;
  XDAS_UInt8  frame0_grid_position_y;
  XDAS_UInt8  frame1_grid_position_x;
  XDAS_UInt8  frame1_grid_position_y;
  XDAS_UInt8  frame_packing_arrangement_reserved_byte;
  XDAS_UInt8  frame_packing_arrangement_extension_flag;
} IH264VDEC_SeiFramePacking;


/**
 ******************************************************************************
 *  @struct IH264VDEC_SeiMessages
 *
 *  @brief   This structure contains all the supported SEI msg objects
 *
 *  @param  parsed_flag :1 - Indicates that in the current process call,
 *                           contents of the structure is updated
 *                       0 - Indicates contents of the structure is not updated
 *  @param  full_frame_freeze : Full-frame freeze SEI message
 *  @param  full_frame_freeze_release :Cancels the effect of any full-frame
 *             freeze SEI message sent with pictures that precede the current
 *             picture in the output order.
 *  @param  prog_refine_start :Specifies the beginning of a set of consecutive
 *             coded pictures that is labeled as the current picture followed
 *             by a sequence of one or more pictures of refinement of the
 *             quality of the current picture, rather than as a representation
 *             of a continually moving scene.
 *  @param  prog_refine_end : Specifies end of progressive refinement.
 *  @param  user_data_registered :Message contains user data registered as
 *            specified by ITU-T Recommendation T.35
 *  @param  user_data_unregistered :Message contains unregistered user data
 *            identified by a UUID
 *  @param  buffering_period_info :Message specifies the buffering period
 *  @param  pan_scan_rect :Message specifies the coordinates of a rectangle
 *            relative to the cropping rectangle of the sequence parameter set
 *  @param  recovery_pt_info :The recovery point SEI message assists a decoder
 *            in determining when the decoding process will produce acceptable
 *            pictures for display after the decoder initiates random access or
 *            after the encoder indicates a broken link in the sequence.
 *  @param  pic_timing :Specifies timing information regarding cpb delays, dpb
*              output delay, and so on.
 *  @param  stereo_video_info :stereo video information SEI message consist of
 *      pair of picture forming stereo view content.
 ******************************************************************************
*/
typedef struct IH264VDEC_SeiMessages
{
  XDAS_UInt32                  parsed_flag;
  IH264VDEC_SeiFullFrameFreezeRep full_frame_freeze;
  IH264VDEC_SeiFullFrameFreezeRel    full_frame_freeze_release;
  IH264VDEC_SeiProgRefineStart           prog_refine_start;
  IH264VDEC_SeiProgRefineEnd             prog_refine_end;
  IH264VDEC_SeiUserDataRegITUT    user_data_registered;
  IH264VDEC_SeiUserDataUnReg      user_data_unregistered;
  IH264VDEC_SeiBufferingPeriod             buffering_period_info;
  IH264VDEC_SeiPanScanRect               pan_scan_rect;
  IH264VDEC_SeiRecoveryPointInfo         recovery_pt_info;
  IH264VDEC_SeiPictureTiming               pic_timing;
  IH264VDEC_SeiStereoVideoInfo       stereo_video_info;
  IH264VDEC_SeiFramePacking       frame_packing;
} IH264VDEC_SeiMessages;

/**
 ******************************************************************************
 *  @struct IH264VDEC_MbxTraceData
 *
 *  @brief  This structure holds the elements that describe the attributes of
 *          traced data.
 *
 *  @param  userId  : Indicates whose data getting logged.
 *  @param  message : Indicates the message type that's been used at given
 *                    user ID.
 *  @param  rwMode  : Indicates mode in which the MBx is accessed.
 *  @param  rsvd    : Reserved byte to add any new attribute in future.
 *
 ******************************************************************************
*/
typedef struct IH264VDEC_MbxTraceData
{
  XDAS_UInt8 userId;
  XDAS_UInt8 message;
  XDAS_UInt8 rwMode;
  XDAS_UInt8 rsvd;
}IH264VDEC_MbxTraceData;

/**
 *  Macro to indicate the max number of samples
 *  that we capture in the log.
*/
#define NUM_MBX_TRACE_ELEMENTS  300

/**
 ******************************************************************************
 *  @struct IH264VDEC_ProfileInfo
 *
 *  @brief   This structure contains elements related to profiling information
 *           This Structure is used to get some codec internal cycles and not
 *           to be used for any system level profiling.
 *
 *  @param  hostPreIva : Cycles spent by M3 before giving control to IVAHD.
 *                       This gives M3 cycles alone that are spent before IVAHD
 *                       starts doing any thing for a given process call.
 *  @param  preMbLoop  : Cycles spent before entering MB Loop
 *  @param  inMbLoop   : Cycles in the MB Loop
 *  @param  postMbLoop : Cycles spent after the MB Loop execution
 *  @param  hostPostIva : Cycles spent after getting control back to Host from
 *                        IVAHD. This gives M3 cycles alone that are spent
 *                        after IVAHD gives control back to M3
 *  @param  ivahdTotalCycles : Total cycles on IVAHD
 *  @param  sliceTask  : per slice cycles spent; Array size set to the stream
 *                       "Combined_CABAC_07_HD_10.1.26l"
 *  @param  noOfSlices : Number of lices in the picture
 ******************************************************************************
*/
typedef struct IH264VDEC_ProfileInfo
{
  XDAS_UInt32 hostPreIva;
  XDAS_UInt32 preMbLoop;
  XDAS_UInt32 inMbLoop;
  XDAS_UInt32 postMbLoop;
  XDAS_UInt32 hostPostIva;
  XDAS_UInt32 ivahdTotalCycles;
  XDAS_UInt32 sliceTask[136];
  XDAS_UInt32 noOfSlices;
  IH264VDEC_MbxTraceData mbxTraceArray[NUM_MBX_TRACE_ELEMENTS];
  XDAS_UInt16 mbxtraceIdx;
} IH264VDEC_ProfileInfo;

/**
 ******************************************************************************
 *  @struct _sErrConcealStr
 *  @brief  This str holds up the required Info for implementing the SCV EC,
 *          this will get updated by H.264 decoder while decoding the  SVC
 *          Base/Target Layers
 *
 *  @param  CurrMbInfoBufPointer  :Base Address of the current decoded frame
 *                                   MB Info buffer
 *
 *  @param  CurrMbStatusBufPointer: Base Address of the current decoded frame
 *                                   MB staus buffer pointer
 *
 *  @param  currFrameY            : Base Address of the current decoded Luma
 *                                  frame buffer pointer (physical pointer)
 *
 *  @param  currFrameUV           : Base Address of the current decoded Chroma
 *                                  frame buffer pointer (physical pointer)
 *
 *  @param  refConclY             : Base Address of the ref decoded Luma
 *                                  frame buffer pointer (virtual pointer)
 *
 *  @param  refConclUV            : Base Address of the ref decoded Chroma
 *                                  frame buffer pointer (virtual pointer)
 *
 *  @param  TilerBaseAddress      : TBA vaule for the VDMA
 *
 *  @param  pSliceInfoFlags       : Flag to enable slice info
 *
 *  @param  ref_width             : Resultant Horizontal LUMA picture size
 *                                  after Pad size addition on both Left
 *                                  & Right sides. This gets used as
 *                                  stride during vDMA programming.
 *                                  In case of TILER,the stride is fixed,
 *                                  independant of Picture width, and
 *                                  only changes with TILER mode.
 *
 *  @param  ref_width_c           : Resultant Horizontal CHROMA picture size
 *                                  after Pad size addition on both Left &
 *                                  Right sides.
 *
 *
 *  @param  ref_frame_height      : In case of Interlaced streams,the picure
 *                                  store is different i.e., store each field
 *                                  by applying PAD on top & bottom lines.
 *                                  Hence the picture height will be Height
 *                                  plus four times the Pad size. This
 *                                  variable holds this resultant value.
 *
 *  @param  mb_width              : Picture width in terms of Macroblocks
 *
 *  @param  mb_height             : Picture height in terms of Macroblocks.
 *
 *  @param  image_width           : Image width of the decoded frame
 *
 *  @param  image_width           : Image height of the decoded frame
 *
 *  @param  frameType             : Frame type of the current frame.
 *
 *  @param  picaff_frame          : Flag to indicate whether current picture
 *                                  is of Frame type & referring to Field
 *                                  picture as reference.
 *
 *  @param  mb_aff_frame_flag     : Flag to indicate whether the current
 *                                  decoding picture is MBAFF type.
 *
 *  @param  field_pic_flag        : Flag to indicate whether the current
 *                                  decoding picture is field type.
 *
 *  @param  bottom_field_flag     : This parameter equal to 1 specifies that
 *                                  the slice is part of a coded bottom field.
 *                                  bottom_field_flag equalto 0 specifies
 *                                  that the picture is a coded top field.
 *
 *  @param  nonPairedFieldPic     : Flag to indicate Non paired field picture.
 *
 *  @param  prev_pic_bottom_field : this variable Indicates if the previous
 *                                  picture was a bottom field or not (a Flag)
 *
 *  @param  currFrameYDual        : Base Address of the current decoded Luma
 *                                  frame buffer pointer (physical pointer)
 *                                  for dual yuv output.
 *
 *  @param  currFrameUVDual       : Base Address of the current decoded Chroma
 *                                  frame buffer pointer (physical pointer)
 *                                  for dual yuv output.
 *
 *  @param  ref_widthDual         : Resultant Horizontal LUMA picture size
 *                                  after Pad size addition on both Left
 *                                  & Right sides. This gets used as
 *                                  stride during vDMA programming.
 *                                  In case of TILER,the stride is fixed,
 *                                  independant of Picture width, and
 *                                  only changes with TILER mode.
 *
 *  @param  ref_width_cDual       : Resultant Horizontal CHROMA picture size
 *                                  after Pad size addition on both Left &
 *                                  Right sides.
 *
 ******************************************************************************
*/

typedef struct _sErrConcealStr
{
  XDAS_Int32  ErrConcealmentEnable;
  XDAS_Int32  CurrMbInfoBufPointer;
  XDAS_Int32  CurrMbStatusBufPointer;
  XDAS_Int32  CurrMbInfoIresBufPointer;
  XDAS_Int32  currFrameY;
  XDAS_Int32  currFrameUV;
  XDAS_Int32  refConclY;
  XDAS_Int32  refConclUV;
  XDAS_UInt32 TilerBaseAddress;
  XDAS_UInt16 ref_width;
  XDAS_UInt16 ref_width_c;
  XDAS_UInt16 ref_frame_height;
  XDAS_UInt16 mb_width;
  XDAS_UInt16 mb_height;
  XDAS_UInt16 image_width;
  XDAS_UInt16 image_height;
  XDAS_UInt8  frameType;
  XDAS_UInt8  picaff_frame;
  XDAS_UInt8  mb_aff_frame_flag;
  XDAS_UInt8  field_pic_flag;
  XDAS_UInt8  bottom_field_flag;
  XDAS_UInt8  nonPairedFieldPic;
  XDAS_UInt8  prev_pic_bottom_field;
  XDAS_Int32  currFrameYDual;
  XDAS_Int32  currFrameUVDual;
  XDAS_UInt16 ref_widthDual;
  XDAS_UInt16 ref_width_cDual;
  XDAS_UInt16 rsvd[2];
}sErrConcealStr;

/**
 *  Size of sliceinfo flags - We have two slice info flag arrays in SL2, one
 *  for ECD3 and the other for MC3. ECD3 flag is one bit per MB. Since Maximum
 *  supported number of MBs in a frame for Low resolution is 128 x 128 = 16384,
 *  and for High resolution 256*256 = 65536. So we need 16384/8 = 2048 bytes
 *  for Low resolution and 65536/8 = 8192 for High resolution to store slice
 *  info flag array for ECD3. But for the MC3 array, we always make the next
 *  bit also as 1 to enable loading into ping and pong memories of MCBUF.
 *  So we need an extra bit for the MC3 array, to avoid buffer overflow when
 *  the last MB is a new slice. To keep the next SL2 buffer in 16-byte aligned
 *  position (some buffers need it) we round the size to next multiple of 16,
 *  i.e., 2064 and 8208 for Low and High resolutions respectively.
 *  As we are maintaining only one decoder image in M3, we define
 *  SLICEINFO_FLAGSIZE as 8208 (maximum among 2064 and 8208).
*/
#define SLICEINFO_FLAGSIZE_HIGHRES  8208
#define SLICEINFO_FLAGSIZE_LOWRES   2064

/**
 ******************************************************************************
 *  @struct _sErrConcealLayerStr
 *  @brief  This str holds up the required Info for implementing the SCV EC,
 *          this will get updated by H.264 decoder while decoding the  SVC
 *          Base/Target Layers
 *
 *  @param  svcEcStr              : structure instance of sSVCErrConcealStr
 *
 *  @param  pSliceInfoFlags       : Array to store the sliceInfo flag
 *
 *
 ******************************************************************************
*/
typedef struct _sErrConcealLayerStr
{
  sErrConcealStr sECStr;
  XDAS_UInt8 pSliceInfoFlags[SLICEINFO_FLAGSIZE_HIGHRES];
}sErrConcealLayerStr;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_dpbNumFrames
 *  @brief      This enum can be used to choose the DPB Size in number
 *              number of frames.
 *  @details
 ******************************************************************************
*/
typedef enum
{
  IH264VDEC_DPB_NUMFRAMES_AUTO = -1,
   /**<
    * Allow the decoder to choose the number of reference frames based on the
    * stream information.
    */
  IH264VDEC_DPB_NUMFRAMES_0 = 0,
    /**<
    * Number of frames required is 0
    */
  IH264VDEC_DPB_NUMFRAMES_1 = 1,
    /**<
    * Number of frames required is 1
    */
  IH264VDEC_DPB_NUMFRAMES_2 = 2,
    /**<
    * Number of frames required is 2
    */
  IH264VDEC_DPB_NUMFRAMES_3 = 3,
    /**<
    * Number of frames required is 3
    */
  IH264VDEC_DPB_NUMFRAMES_4 = 4,
      /**<
    * Number of frames required is 4
    */
  IH264VDEC_DPB_NUMFRAMES_5 = 5,
    /**<
    * Number of frames required is 5
    */
  IH264VDEC_DPB_NUMFRAMES_6 = 6,
    /**<
    * Number of frames required is 6
    */
  IH264VDEC_DPB_NUMFRAMES_7 = 7,
      /**<
    * Number of frames required is 7
    */
  IH264VDEC_DPB_NUMFRAMES_8 = 8,
    /**<
    * Number of frames required is 8
    */
  IH264VDEC_DPB_NUMFRAMES_9 = 9,
    /**<
    * Number of frames required is 9
    */
  IH264VDEC_DPB_NUMFRAMES_10 = 10,
    /**<
    * Number of frames required is 10
    */
  IH264VDEC_DPB_NUMFRAMES_11 = 11,
    /**<
    * Number of frames required is 11
    */
  IH264VDEC_DPB_NUMFRAMES_12 = 12,
    /**<
    * Number of frames required is 12
    */
  IH264VDEC_DPB_NUMFRAMES_13 = 13,
    /**<
    * Number of frames required is 13
    */
  IH264VDEC_DPB_NUMFRAMES_14 = 14,
    /**<
    * Number of frames required is 14
    */
  IH264VDEC_DPB_NUMFRAMES_15 = 15,
    /**<
    * Number of frames required is 15
    */
  IH264VDEC_DPB_NUMFRAMES_16 = 16,
    /**<
    * Number of frames required is 16
    */
  IH264VDEC_DPB_NUMFRAMES_DEFAULT = IH264VDEC_DPB_NUMFRAMES_AUTO
   /**<
    * Allow the decoder to choose the number of reference frames based on the
    * stream information.
    */
} IH264VDEC_dpbNumFrames;

/**
 ******************************************************************************
 *  @enum       IH264VDEC_SVCErrConcealMode
 *  @brief      Describes the error concealment mode ID for SVC codec
 *              This enumeration type is used by svc app to specify codec
 *              to concealment mode
 *
 ******************************************************************************
*/

typedef enum
{
  IH264VDEC_SETERRCONCEALMODE   = 15
  /** SVC error concealment mode ID
  *
  */
} IH264VDEC_SVCErrConcealMode;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_CommonInfo
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_CommonInfo
{
  XDAS_UInt32 codec_type:8;
  XDAS_UInt32 fmt_type:8;
  XDAS_UInt32 mb_ll_avail:1;
  XDAS_UInt32 mb_ul_avail:1;
  XDAS_UInt32 mb_uu_avail:1;
  XDAS_UInt32 mb_ur_avail:1;
  XDAS_UInt32 pic_bound_l:1;
  XDAS_UInt32 pic_bound_u:1;
  XDAS_UInt32 pic_bound_r:1;
  XDAS_UInt32 pic_bound_b:1;
  XDAS_UInt32 first_mb_flag:1;
  XDAS_UInt32 error_flag:1;
  XDAS_UInt32 zero:6;
  XDAS_UInt32 zeroes:16;
  XDAS_UInt32 mb_addr:16;

} IH264VDEC_TI_CommonInfo;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_MotionVector
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_MotionVector
{
  XDAS_Int16 x;
  XDAS_Int16 y;
} IH264VDEC_TI_MotionVector;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_CabacContext
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_CabacContext
{
  IH264VDEC_TI_MotionVector   mvd_l0[4];
  IH264VDEC_TI_MotionVector   mvd_l1[4];

} IH264VDEC_TI_CabacContext;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_TotalCoefLuma
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_TotalCoefLuma
{
  XDAS_UInt8 right[3];
  XDAS_UInt8 bottom_right;
  XDAS_UInt8 bottom[3];
  XDAS_UInt8 zero;
} IH264VDEC_TI_TotalCoefLuma;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_TotalCoefChroma
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_TotalCoefChroma
{
  XDAS_UInt8 right_cb;
  XDAS_UInt8 bottom_right_cb;
  XDAS_UInt8 bottom_cb;
  XDAS_UInt8 zero;
  XDAS_UInt8 right_cr;
  XDAS_UInt8 bottom_right_cr;
  XDAS_UInt8 bottom_cr;
  XDAS_UInt8 zero1;
} IH264VDEC_TI_TotalCoefChroma;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_CavlcContext
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_CavlcContext
{
  unsigned long long zeroes[2];
  IH264VDEC_TI_TotalCoefLuma    total_coef_luma;
  IH264VDEC_TI_TotalCoefChroma  total_coef_chroma;

} IH264VDEC_TI_CavlcContext;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_IntraPredMode
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_IntraPredMode
{
  XDAS_UInt32 ipred_mode0:4;
  XDAS_UInt32 ipred_mode1:4;
  XDAS_UInt32 ipred_mode2:4;
  XDAS_UInt32 ipred_mode3:4;
  XDAS_UInt32 ipred_mode4:4;
  XDAS_UInt32 ipred_mode5:4;
  XDAS_UInt32 ipred_mode6:4;
  XDAS_UInt32 ipred_mode7:4;
  XDAS_UInt32 ipred_mode8:4;
  XDAS_UInt32 ipred_mode9:4;
  XDAS_UInt32 ipred_mode10:4;
  XDAS_UInt32 ipred_mode11:4;
  XDAS_UInt32 ipred_mode12:4;
  XDAS_UInt32 ipred_mode13:4;
  XDAS_UInt32 ipred_mode14:4;
  XDAS_UInt32 ipred_mode15:4;

} IH264VDEC_TI_IntraPredMode;


/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_MbPredType
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_MbPredType
{
  XDAS_UInt32 mbskip:1;
  XDAS_UInt32 tr8x8:1;
  XDAS_UInt32 mb_field:1;
  XDAS_UInt32 cond_mbskip:1;
  XDAS_UInt32 c_ipred_mode:2;
  XDAS_UInt32 zero:1;
  XDAS_UInt32 end_of_slice:1;
  XDAS_UInt32 mb_y_mod2:1;
  XDAS_UInt32 zero1:7;
  XDAS_UInt32 refidx_equal_flag_l0:1;
  XDAS_UInt32 refidx_equal_flag_l1:1;
  XDAS_UInt32 mv_equal_flag_l0:1;
  XDAS_UInt32 mv_equal_flag_l1:1;
  XDAS_UInt32 zeroes:4;
  XDAS_UInt32 mb_type:8;
  XDAS_UInt8 sub_mb_type[4];

} IH264VDEC_TI_MbPredType;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_QpCbp
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_QpCbp
{
  XDAS_UInt32 cbp;
  XDAS_UInt8 qp_y;
  XDAS_UInt8 qp_cb;
  XDAS_UInt8 qp_cr;
  XDAS_UInt8 zero;
} IH264VDEC_TI_QpCbp;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_RefPicControl
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_RefPicControl
{
  XDAS_UInt8 refidx[4];
  XDAS_UInt8 refpicid[4];

} IH264VDEC_TI_RefPicControl;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_MvBidirectional16
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_MvBidirectional16
{
  IH264VDEC_TI_MotionVector   mv_forward[16];
  IH264VDEC_TI_MotionVector   mv_backward[16];
} IH264VDEC_TI_MvBidirectional16;


/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_MvBidirectional4
 *
 *  @brief
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_MvBidirectional4
{
  IH264VDEC_TI_MotionVector   mv_forward[4];
  IH264VDEC_TI_MotionVector   mv_backward[4];

} IH264VDEC_TI_MvBidirectional4;

/**
 ******************************************************************************
 *  @struct IH264VDEC_TI_MbInfo
 *
 *  @brief  This structure details the data format for MB information shared to
 *          application. This helps application to understand all fields
 *          the way codec uses MB info internally. This structure is of size
 *          208 Bytes.
 *
 *  @param  info : This elements gives details about the MB placement in the
 *                 frame.
 *
 *  @param  cabac: This field holds the context data for a CABAC coded MB
 *
 *  @param  cavlc: This field holds the context data for a CAVLC coded MB
 *
 *  @param  ipred_mode: This field holds information of intra prediction modes
 *                      at 4x4 level, for intra coded MB.
 *
 *  @param  mb_pred_type: This indicates prediction specific details for inter
 *                        coded MB
 *
 *  @param  qp_cbp: This gives coded & QP informations for both LUMA & CHROMA
 *                   components of a Macro Block.
 *
 *  @param  l0_ref_pic_control: Informs all details about reference indices
 *                              at 8x8 block level in L0 direction
 *
 *  @param  l1_ref_pic_control: Informs all details about reference indices
 *                              at 8x8 block level in L1 direction
 *
 *  @param  mv_forward: Lists all Motion vectors at 4x4 level in L0 direction
 *
 *  @param  bidirectional16: Lists all Motion vectors at 4x4 level in both
 *                           directions
 *
 *  @param  bidirectional4: Lists all Motion vectors at 8x8 level in both
 *                          directions
 *
 ******************************************************************************
*/
typedef struct _IH264VDEC_TI_MbInfo
{
  IH264VDEC_TI_CommonInfo info;

  union {
    IH264VDEC_TI_CabacContext  cabac;
    IH264VDEC_TI_CavlcContext  cavlc;
  } IH264VDEC_TI_context;

  IH264VDEC_TI_IntraPredMode  ipred_mode;
  IH264VDEC_TI_MbPredType     mb_pred_type;
  IH264VDEC_TI_QpCbp          qp_cbp;
  IH264VDEC_TI_RefPicControl  l0_ref_pic_control;
  IH264VDEC_TI_RefPicControl  l1_ref_pic_control;

  union {
    IH264VDEC_TI_MotionVector       mv_forward[16];
    IH264VDEC_TI_MvBidirectional16  bidirectional16;
    IH264VDEC_TI_MvBidirectional4   bidirectional4;
  } IH264VDEC_TI_motion_vecs;

} IH264VDEC_TI_MbInfo;

#endif             /* IH264VDEC_ */
