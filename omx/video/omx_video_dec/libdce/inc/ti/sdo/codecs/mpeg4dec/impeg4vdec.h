/*
********************************************************************************
* HDVICP2.0 Based MPEG4 ASP Decoder
*
* "HDVICP2.0 Based MPEG4 ASP Decoder" is software module developed on TI's
*  HDVICP2 based SOCs. This module is capable of decode a 4:2:0 Raw
*  video stream of Advanced/Simple profile and also H.263 bit-stream.
*  Based on  ISO/IEC 14496-2:2003."
* Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
********************************************************************************
*/

/**
********************************************************************************
* @file <impeg4vdec.h>
*
* @brief This file provides definisions for the interface handles.
*
* @author: Ashish Singh   (ashish.singh@ti.com)
*
* @version 0.0 (June 2009) : Base version created [Ashish]
*
* @version 0.1 (Apr 2010) : Review Comments Added [Ananya]
*
* @version 0.2 (oct 2010) : cleared doxygen warning and fixed VOP non coded
*                           related bugs.
*
* @version 0.3 (Nov 2010) : Modified the error bit 20.
*
* @version 0.4 (Dec 2010) : Removed the sorenson support and compile time
*                           parameter _DEBUGTRACE. [Nitesh]
*
* @version 0.5 (May 2011) : Added one more create time parameter paddingMode
*                           to support the padding of non multiple of
*                           16 resolution clips, along with this removed some
*                           un-neccesary code from interface file [Ashish]

* @version 0.6 (July 2011): rename the error code to have alignment of the error
*                           codes related mpeg4 [Ashish]
*
* @version 0.7 (Sep  2011): rename the error codes and also introduced more
*                           error code as IMPEG4D_ERR_FRAME_DROPPED etc.
*                           allocated some reserve space  in all interface str
*                           for future uses. [Ashish]
*
* @version 0.8 (Oct  2011): renamed the enum macros to appropriate uses cases.
*                           removed unwanted enum from the inetrface file.
*                           added debug trace level enum to interface file
*                           [Ashish]
*
* @version 0.9 (Mar 2012) : Added create time params & enums to support Enhanced
*                           deblocking feature [Mahantesh]
*
* @version 1.0 [May 2012] : MB Info format structure added [Mahantesh]
*
*******************************************************************************
*/

/* -------------------- compilation control switches ---------------------- */

#ifndef _IMPEG4VDEC_H_
#define _IMPEG4VDEC_H_

/****************************************************************************
*   INCLUDE FILES
*****************************************************************************/
/* -------------------- system and platform files ------------------------- */
/* ------------------------- program files -------------------------------- */

#include <ti/xdais/xdas.h>
#include <ti/xdais/dm/ividdec3.h>
//#include <ti/sdo/fc/ires/hdvicp/ires_hdvicp2.h>
#include <ti/xdais/ialg.h>


/****************************************************************************
*   EXTERNAL REFERENCES NOTE : only use if not found in header file
*****************************************************************************/
/* ------------------------ data declarations ----------------------------- */
/* ----------------------- function prototypes ---------------------------- */

/****************************************************************************
*   PUBLIC DECLARATIONS Defined here, used elsewhere
*****************************************************************************/
/* ----------------------- data declarations ------------------------------ */
/**
*  Macro defined for Offset version length
*/
#define IMPEG4DEC_VERSION_LENGTH  53
/**
*******************************************************************************
*  @struct   IMPEG4VDEC_Obj
*
*  @brief   This structure must be the first field of all mp4VDEC instance
*           objects
*
*  @param  fxns
*          Handle to extented mpeg4 video decoder library interface functions
*
*  @note    None
********************************************************************************
*/
typedef struct IMPEG4VDEC_Obj
{
  struct IMPEG4VDEC_Fxns *fxns;
} IMPEG4VDEC_Obj;

/**
*******************************************************************************
*  @struct   IMPEG4VDEC_Handle
*
*  @brief  This handle is used to reference all mp4VDEC instance objects
*
*  @note    None
********************************************************************************
*/
typedef struct IMPEG4VDEC_Obj *IMPEG4VDEC_Handle;

/**
*******************************************************************************
*  @struct   IMPEG4VDEC_Status
*
*  @brief  Status structure defines the parameters that can be changed or
*          read during real-time operation of the alogrithm.
*
*  @param  viddec3Status
*          Handle to base class status struture which defines the all
*          run time parameters.
*
*  @param  lastNFramesToLog
*          element to set the number of frame traces required before the
*          current frame decoded
*
*  @param  extMemoryDebugTraceAddr
*          External memory address where codec dumps the debug trace logs
*
*  @param  extMemoryDebugTraceSize
*          size of the debug trace logs dump by codec in external memory
*
*  @param  reserved[3]
*          allocted reserve space for future uses
*
*  @note    None
********************************************************************************
*/
typedef struct IMPEG4VDEC_Status
{
  IVIDDEC3_Status  viddec3Status;

  XDAS_UInt32   debugTraceLevel;
  XDAS_UInt32   lastNFramesToLog;
  XDAS_UInt32  *extMemoryDebugTraceAddr;
  XDAS_UInt32   extMemoryDebugTraceSize;
  XDAS_UInt32   reserved[3];
} IMPEG4VDEC_Status;

/**
*******************************************************************************
*  @struct  IMPEG4VDEC_Cmd
*
*  @brief  The Cmd enumeration defines the control commands for the MPEG4
*          video decoder control method.
*
*  @note    None
********************************************************************************
*/
typedef IVIDDEC3_Cmd IMPEG4VDEC_Cmd;

/**
*******************************************************************************
*  @struct  IMPEG4VDEC_Params
*
*  @brief  This structure defines the creation parameters for all
*          mp4VDEC objects
*
*  @param  viddecParams
*          Defines the creation time parameters for
*          all IVIDDEC3 instance objects.
*
*  @param  outloopDeBlocking
*          Flag for Optional deBlock filter ON or Enhanced filtering ON
*
*  @param  errorConcealmentEnable
*          flag to set error concealment feature-set on or off
*
*  @param  sorensonSparkStream
*          Flag reserved for future usage for sorenson spark stream
*
*  @param  debugTraceLevel
*          element to set the debug trace level, codec will give trace
*          info base on tarce level
*
*  @param  lastNFramesToLog
*          element to set the number of frame traces required before the
*          current frame decoded
*
*  @param  paddingMode
*          Flag to set the padding type used by codec for non multiple
*          for 16 resolution clips
*
*  @param  enhancedDeBlockingQp
*          QP value to be used for filtering all edges
*
*  @param  reserved[2]
*          allocted reserve space for future uses
*
*  @note    None
********************************************************************************
*/
typedef struct IMPEG4VDEC_Params
{
  IVIDDEC3_Params    viddec3Params;
  XDAS_Int32         outloopDeBlocking;
  XDAS_Int32         errorConcealmentEnable;
  XDAS_Int32         sorensonSparkStream;
  XDAS_UInt32        debugTraceLevel;
  XDAS_UInt32        lastNFramesToLog;
  XDAS_UInt32        paddingMode;
  XDAS_UInt32        enhancedDeBlockingQp;
  XDAS_UInt32        reserved[2];
} IMPEG4VDEC_Params;

extern IMPEG4VDEC_Params IMPEG4VDEC_PARAMS;

/**
*******************************************************************************
*  @struct   IMPEG4VDEC_DynamicParams
*
*  @brief  This structure defines the run time parameters for all
*          mp4VDEC objects
*
*  @param  viddecDynamicParams
*          Defines the run time parameters for
*          all IVIDDEC3 instance objects.
*
*  @param  reserved[3]
*          allocted reserve space for future uses

*  @see    None
********************************************************************************
*/
typedef struct IMPEG4VDEC_DynamicParams
{
  IVIDDEC3_DynamicParams viddec3DynamicParams;
  XDAS_UInt32            reserved[3];
} IMPEG4VDEC_DynamicParams;

extern IMPEG4VDEC_DynamicParams IMPEG4VDEC_TI_DYNAMICPARAMS;
/**
*******************************************************************************
*  @struct   IMPEG4VDEC_InArgs
*
*  @brief  This structure defines the run time input arguments for all VIDDEC
*          objects.This structure may be extended by individual codec
*          implementation allowing customization with vendor specific
*          parameters.
*
*  @param  viddec3InArgs
*          Defines the input arguments for all IVIDDEC3 instance
*          process function.
*
*  @see    None
********************************************************************************
*/
typedef struct IMPEG4VDEC_InArgs
{
  IVIDDEC3_InArgs   viddec3InArgs;
}IMPEG4VDEC_InArgs;

/**
*******************************************************************************
*  @struct  IMPEG4VDEC_OutArgs
*
*  @brief  This structure defines the run time output arguments for VIDDEC
*          objects.This structure may be extended by individual codec
*          implementation allowing customization with vendor specific
*          parameters.
*
*  @param  viddec3OutArgs
*          Defines the output arguments for all IVIDDEC3 instance
*          process function.

*  @param  vopTimeIncrementResolution
*          VOP Time increamnet resolution info present in mpeg4 stream

*  @param  vopTimeIncrement
*          VOP Time increment info present in mpeg4 stream
*
*  @param  mp4ClosedGov
*          Flag to get to know info about closed_gov
*
*  @param  mp4BrokenLink
*          Flag to get to know info about mpeg4 broken link
*
*  @note   None
********************************************************************************
*/
typedef struct IMPEG4VDEC_OutArgs
{
  IVIDDEC3_OutArgs  viddec3OutArgs;
  XDAS_Int32        vopTimeIncrementResolution;
  XDAS_Int32        vopTimeIncrement;
  XDAS_Int32        mp4ClosedGov;
  XDAS_Int32        mp4BrokenLink;
}IMPEG4VDEC_OutArgs;

/**
*******************************************************************************
*  @enum   IMPEG4VDEC_ErrorBit
*
*  @brief      Mpeg4 Error Codes: Delaration of mpeg4 decoder specific Error
*              Codes.
*  @details    Error status is communicated through a 32 bit word. In this,
*              Error Bits 8 to 15 are used to indicate the XDM error bits. See
*              XDM_ErrorBit definition in xdm.h. Other bits in a 32 bit word
*              can be used to signal any codec specific errors. The staructure
*              below enumerates the mpeg4 decoder specific error bits used.
*              The algorithm can set multiple bits to 1 depending on the error
*              condition
*
********************************************************************************
*/
typedef enum
{
  IMPEG4D_ERR_VOS = 0,
  /**<
* Bit 0
*  1 - No Video Object Sequence detected in the frame
*  0 - Ignore
*/

  IMPEG4D_ERR_VO,
  /**<
* Bit 1
*  1 - Incorrect Video Object type
*  0 - Ignore
*/

  IMPEG4D_ERR_VOL,
  /**<
* Bit 2
*  1 - Error in Video Object Layer detected
*  0 - Ignore
*/

  IMPEG4D_ERR_GOV,
  /**<
* Bit 3
*  1 - Error in Group of Video parsing
*  0 - Ignore
*/

  IMPEG4D_ERR_VOP,
  /**<
* Bit 4
*  1 - Error in Video Object Plane parsing
*  0 - Ignore
*/

  IMPEG4D_ERR_SHORTHEADER,
  /**<
* Bit 5
*  1 - Error in short header parsing
*  0 - Ignore
*/

  IMPEG4D_ERR_GOB,
  /**<
* Bit 6
*  1 - Error in GOB parsing
*  0 - Ignore
*/

  IMPEG4D_ERR_VIDEOPACKET,
  /**<
* Bit 7
*  1 - Error in Video Packet parsing
*  0 - Ignore
*/

  IMPEG4D_ERR_MBDATA = 16 ,
  /**<
* Bit 16
*  1 -  Error in MB data parsing
*  0 - Ignore
*/

  IMPEG4D_ERR_INVALIDPARAM_IGNORE,
  /**<
* Bit 17
*  1 -  Invalid Parameter
*  0 - Ignore
*/

  IMPEG4D_ERR_UNSUPPFEATURE,
  /**<
* Bit 18
*  1 -  Unsupported feature
*  0 - Ignore
*/

  IMPEG4D_ERR_STREAM_END,
  /**<
* Bit 19
*  1 - End of stream reached
*  0 - Ignore
*/

  IMPEG4D_ERR_VALID_HEADER_NOT_FOUND,
  /**<
* Bit 20
*  1 - Vaild header not found.i.e (VOL/VOP not found)
*  0 - Ignore
*/

  IMPEG4D_ERR_UNSUPPRESOLUTION,
  /**<
* Bit 21
*  1 - Unsupported resolution by the decoder
*  0 - Ignore
*/

  IMPEG4D_ERR_BITSBUF_UNDERFLOW,
  /**<
* Bit 22
*  1 - The stream buffer has underflowed
*  0 - Ignore
*/

  IMPEG4D_ERR_INVALID_MBOX_MESSAGE,
  /**<
* Bit 23
*  1 - Invalid (unexpected) mail boX message recieved by IVAHD
*  0 - Ignore
*/
  IMPEG4D_ERR_NO_FRAME_FOR_FLUSH,
  /**<
* Bit 24
*  1 -  Codec does not have any frame for flushing out to application
*  0 - Ignore
*/
  IMPEG4D_ERR_VOP_NOT_CODED,
  /**<
* Bit 25
*  1 -  Given vop is not codec
*  0 - Ignore
*/
  IMPEG4D_ERR_START_CODE_NOT_PRESENT,
  /**<
* Bit 26
*  1 -  Start code for given stream is not present in case of Parse Header
*       Mode
*  0 - Ignore
*/
  IMPEG4D_ERR_VOP_TIME_INCREMENT_RES_ZERO,
  /**<
* Bit 27
*  1 - Unsupported time increment resolution by the decoder
*  0 - Ignore
*/
  IMPEG4D_ERR_PICSIZECHANGE,
  /**<
* Bit 28
*  1 - resolution gets change in between process call
*  0 - Ignore
*/
  IMPEG4D_ERR_UNSUPPORTED_H263_ANNEXS,
  /**<
* Bit 29
*  1 - Unsupported annexs of the H263
*  0 - Ignore
*/
  IMPEG4D_ERR_HDVICP2_IMPROPER_STATE,
  /**<
* Bit 30
*  1 - HDVCIP is not in correct state
*  0 - Ignore
*/
  IMPEG4D_ERR_IFRAME_DROPPED
  /**<
* Bit 31
*  1 - Current frame is lost,no frame is present for decode
*  0 - Ignore
*/

} IMPEG4VDEC_ErrorBit;

/**
******************************************************************************
*  @enum       IMPEGVDEC_MetadataType
*  @brief      This enum indicates Meta Data types for MPEG4 ASP Decoder
*
*  @remarks    The way to get meta data from decoder is via outBufs of the
*              decoder during  process call.
******************************************************************************
*/

typedef enum
{
  IMPEGVDEC_PARSED_MB_INFO_DATA = XDM_CUSTOMENUMBASE,
  IMPEGVDEC_PARSED_MB_ERROR_INFO_DATA,
  IMPEGVDEC_PARSED_ALFA_DATA
} IMPEGVDEC_MetadataType;

/**
*******************************************************************************
*  @enum    _IMPEG4MemDescription
*  @brief   MemDescription of  IVAHD MPEG4 ASP Decoder implementation by TI.
*
* @remarks  this enum explain the type of memory required by mpeg4 ASP decoder
*
*******************************************************************************
*/

typedef enum _IMPEG4MemDescription
{
  IMPEG4VDEC_OBJECT,
  /**
* this memory type is allocated by alg_alloc call & this is allocated
* for handle of decoder
*/
  IMPEG4VDEC_COLOC_PMBDATA,

  /**
* Number of MemTab required if no-Deblock and no-B colocated MB info
* required
*/
  IMPEG4VDEC_COLOC_BMBDATA ,
  /**
* this memory type is allocated by IRES & this is for storing the colocated
* MB data for B Frame
*/
  IMPEG4VDEC_DEBUG_TRACE_BUF,
  /**<
* Memtab for debug trace parameter storage
*/
  IMPEG4VDEC_DEBLOCK_OFF_TOTAL_MEMTABS,
  /**
* Number of MemTab required if Deblock is Off required
*/
  IMPEG4VDEC_LUMARECON_DATA_BUF0 = IMPEG4VDEC_DEBLOCK_OFF_TOTAL_MEMTABS,
  /**
* this memory type is allocated by IRES & this is for storing the decoded
* Luma data for Recon Frame used in case of Deblock Enable
*/
  IMPEG4VDEC_LUMARECON_DATA_BUF1,
  /**
* this memory type is allocated by IRES & this is for storing the decoded
* Luma data for Recon Frame used in case of Deblock Enable
*/
  IMPEG4VDEC_LUMARECON_DATA_BUF2,
  /**
* this memory type is allocated by IRES & this is for storing the decoded
* Luma data for Recon Frame used in case of Deblock Enable
*/
  IMPEG4VDEC_CHROMARECON_DATA_BUF0,
  /**
* this memory type is allocated by IRES & this is for storing the decoded
* Chroma data for Recon Frame used in case of Deblock Enable
*/
  IMPEG4VDEC_CHROMARECON_DATA_BUF1,
  /**
* this memory type is allocated by IRES & this is for storing the decoded
* Chroma data for Recon Frame used in case of Deblock Enable
*/
  IMPEG4VDEC_CHROMARECON_DATA_BUF2,
  /**
* this memory type is allocated by IRES & this is for storing the decoded
* Chroma data for Recon Frame used in case of Deblock Enable
*/

  IMPEG4VDEC_DEBLOCK_ON_TOTAL_MEMTABS,
  /**
* Number of memtab required if deblock is on and it will be  maximum
* resource required by codec
*/
  IMPEG4VDEC_MAX_MEMTABS = IMPEG4VDEC_DEBLOCK_ON_TOTAL_MEMTABS
  /**
* Maximum number of memtab required
*/
}IMEPG4VDEC_MemDescription;

/*
*  Number of 1D resource required by codec from IRES
*/
#define IMEPG4VDEC_TOTAL_1D_OBJECTS     0x4
/*
*  Number of 2D resource required by codec from IRES when filtering is off
*/
#define IMEPG4VDEC_MIN_2D_OBJECTS 0x0
/*
*  Number of 2D resource required by codec from IRES when filtering is on
*/
#define IMEPG4VDEC_MAX_2D_OBJECTS 0x6

/**
*******************************************************************************
*  @struct   IMPEG4VDEC_Fxns
*
*  @brief  This structure defines all of the operations on mp4VDEC objects.
*
*  @param  ividdec3
*          handle to the all function of the operations on IVIDDEC3 objects
*
*  @see    None
********************************************************************************
*/
typedef struct IMPEG4VDEC_Fxns
{
  IVIDDEC3_Fxns    ividdec3;
} IMPEG4VDEC_Fxns;

/**
 ******************************************************************************
 *  @enum       IMPEG4VDEC_FrameFlushState
 *  @brief      This enum indicates whether to frame needs to flush or not
 *
 ******************************************************************************
*/
typedef enum
{
  IMPEG4VDEC_FLUSH_DISABLE = 0,
  /**
    *  Flag to set the frame flush is disable
    */
  IMPEG4VDEC_FLUSH_ENABLE
  /**
    *  Flag to set the frame flush is Enable
    */
}IMPEG4VDEC_FrameFlushState;

/**
 ******************************************************************************
 *  @enum       IMPEG4VDEC_ColocatedBFrameMBinfoStoreMode
 *  @brief      This enum indicates whether to Application needs co-located
 *              MB data or not
 *
 ******************************************************************************
*/
typedef enum
{
  IMPEG4VDEC_STORE_COLOCATED_BMBINFO_DISABLE = 0,
  /**
    *  Flag to set the co-located MB info disable
    */
  IMPEG4VDEC_STORE_COLOCATED_BMBINFO_ENABLE
  /**
    *  Flag to set the co-located MB info enable
    */
}IMPEG4VDEC_ColocatedBFrameMBinfoStoreMode;

/**
 ******************************************************************************
 *  @enum       IMPEG4VDEC_OptionalDeBlkMode
 *  @brief      This enum indicates whether deblock need to be done or not
 *
 ******************************************************************************
*/
typedef enum
{
  IMPEG4VDEC_DEBLOCK_DISABLE = 0,
  /**
    *  Flag to set the de-block disable
    */
  IMPEG4VDEC_DEBLOCK_ENABLE,
  /**
    *  Flag to set the de-block enable
    */
  IMPEG4VDEC_ENHANCED_DEBLOCK_ENABLE
  /**
    *  Flag to set the de-block enable for 8x8 edges of all Macroblocks
    *  including its top & left edges.
    */
}IMPEG4VDEC_OptionalDeBlkMode;

/**
 ******************************************************************************
 *  @enum       IMPEG4VDEC_EnhancedDeblockQp
 *  @brief      This enum the MIN & MAX values that QP can take while filering
 *              for all edges is enabled
 *
 ******************************************************************************
*/
typedef enum
{
  IMPEG4VDEC_DEBLOCK_QP_MIN = 1,
  /**
    *  Indicates min QP value when fitering all edges is enabled
    */
  IMPEG4VDEC_DEBLOCK_QP_MAX = 31
  /**
    *  Indicates max QP value when fitering all edges is enabled
    */
}IMPEG4VDEC_EnhancedDeblockQp;
/**
 ******************************************************************************
 *  @enum       IMPEG4VDEC_ErrorConcealmentMode
 *  @brief      This enum indicates whether to apply error concealment or not
 *
 ******************************************************************************
*/
typedef enum
{
  IMPEG4VDEC_EC_DISABLE = 0,
  /**
    *  Flag to set error concealement disable
    */
  IMPEG4VDEC_EC_ENABLE
  /**
    *  Flag to error concealment enable
    */
}IMPEG4VDEC_ErrorConcealmentMode;

/**
 ******************************************************************************
 *  @enum   IMPEG4VDEC_PaddingModeForNonMultipleOf16Res
 *  @brief  These enumerations captures different methods of padding the Ref
 *          frame when dimension is non multiple of 16.
 *
 ******************************************************************************
*/
typedef enum
{
   /**
    * Method as suggested by DivX spec.
    */
  IMPEG4VDEC_DIVX_MODE_PADDING     = 0,
   /**
    * Method as suggested by MPEG4 spec.
    */
  IMPEG4VDEC_MPEG4_MODE_PADDING    = 1 ,
    /**
     * Default mode is DIVX suggested way.
    */
  IMPEG4VDEC_DEFAULT_MODE_PADDING = IMPEG4VDEC_DIVX_MODE_PADDING
} IMPEG4VDEC_PaddingModeForNonMultipleOf16Res;

/**
 ******************************************************************************
 *  @enum   IMPEG4VDEC_debugTraceLevel
 *  @brief  These enumerations captures different debug trace level supported by
 *          codec.
 *
 ******************************************************************************
*/
typedef enum
{
   IMPEG4VDEC_DEBUGTRACE_LEVEL0 = 0,
    /** 0: Debug Trace Level 0
    */
   IMPEG4VDEC_DEBUGTRACE_LEVEL1,
    /** 1: Debug Trace Level 1
    */
   IMPEG4VDEC_DEBUGTRACE_LEVEL2,
    /** 2: Debug Trace Level 2
    */
   IMPEG4VDEC_DEBUGTRACE_MAXLEVEL = IMPEG4VDEC_DEBUGTRACE_LEVEL2
    /** 2: Max level of debug trace
     */
}IMPEG4VDEC_DebugTraceLevel;

/**
 ******************************************************************************
 *  @enum   IMPEG4VDEC_FrameToLog
 *  @brief  These enumerations captures the max number of frame for which codec
 *          will dump debug trace.
 *
 ******************************************************************************
*/
typedef enum
{
   IMPEG4VDEC_MINNUM_OF_FRAME_LOGS = 0,
    /** 0: minimum number of frames for which debug trace would be
    *      dump.
    */
   IMPEG4VDEC_MAXNUM_OF_FRAME_LOGS = 10
    /** 10:max number of frames for which debug trace would be
    *      dump.
    */
}IMPEG4VDEC_FrameToLog;

/**
 ******************************************************************************
 *  @struct IMPEG4VDEC_TI_CommonInfo
 *
 *  @brief  This structure defines the common fields in MB info
 *
 ******************************************************************************
*/
typedef struct _IMPEG4VDEC_TI_CommonInfo
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

} IMPEG4VDEC_TI_CommonInfo;

/**
 ******************************************************************************
 *  @struct IMPEG4VDEC_TI_CodecSpecificWordSix
 *
 *  @brief  This structure defines codec specific fields in MB info
 *
 ******************************************************************************
*/
typedef struct _IMPEG4VDEC_TI_CodecSpecificWordSix
{
  XDAS_UInt32 pred_mode:3;
  XDAS_UInt32 zero7:9;
  XDAS_UInt32 pred_type:2;
  XDAS_UInt32 gob_frame_id:2;
  XDAS_UInt32 gob_number:5;
  XDAS_UInt32 gob_header_empty:1;
  XDAS_UInt32 forward_top_field_reference:1;
  XDAS_UInt32 forward_bottom_field_reference:1;
  XDAS_UInt32 backward_top_field_reference:1;
  XDAS_UInt32 backward_bottom_field_reference:1;
  XDAS_UInt32 header_extension_code:1;
  XDAS_UInt32 zero6:5;
  XDAS_UInt32 pattern_code:6;
  XDAS_UInt32 zero5:2;
  XDAS_UInt32 intra_dc_vlc_thr:3;
  XDAS_UInt32 zero4:5;
  XDAS_UInt32 not_coded:1;
  XDAS_UInt32 dct_type:1;
  XDAS_UInt32 ac_pred_flag:1;
  XDAS_UInt32 cond_skip_flag:1;
  XDAS_UInt32 use_intra_dc_vlc:1;
  XDAS_UInt32 end_of_texture:1;
  XDAS_UInt32 zero3:2;
  XDAS_UInt32 vop_fcode_forward:3;
  XDAS_UInt32 zero2:1;
  XDAS_UInt32 vop_fcode_backward:3;
  XDAS_UInt32 zero1:1;

} IMPEG4VDEC_TI_CodecSpecificWordSix;

/**
 ******************************************************************************
 *  @struct IMPEG4VDEC_TI_CodecSpecificWordSix
 *
 *  @brief  This structure defines codec specific fields in MB info
 *
 ******************************************************************************
*/
typedef struct _IMPEG4VDEC_TI_CodecSpecificWordSeven
{
  XDAS_UInt32 dc_scaler_luma:6;
  XDAS_UInt32 zero4:10;
  XDAS_UInt32 dc_scaler_chroma:5;
  XDAS_UInt32 zero3:11;
  XDAS_UInt32 quant_c:5;
  XDAS_UInt32 zero2:19;
  XDAS_UInt32 quantiser_scale:5;
  XDAS_UInt32 zero1:3;

} IMPEG4VDEC_TI_CodecSpecificWordSeven;

/**
 ******************************************************************************
 *  @struct IMPEG4VDEC_TI_MotionVector
 *
 *  @brief  This structure defines format of Motion Vectors as present in MBinfo
 *
 ******************************************************************************
*/
typedef struct _IMPEG4VDEC_TI_MotionVector
{
  XDAS_Int16 x;
  XDAS_Int16 y;
} IMPEG4VDEC_TI_MotionVector;

/**
 ******************************************************************************
 *  @struct IMPEG4VDEC_TI_MvBidirectional4
 *
 *  @brief  This structure defines Motion Vectors at 8x8 level in both
 *          directions
 *
 ******************************************************************************
*/
typedef struct _IMPEG4VDEC_TI_MvBidirectional4
{
  IMPEG4VDEC_TI_MotionVector   mv_forward[4];
  IMPEG4VDEC_TI_MotionVector   mv_backward[4];

} IMPEG4VDEC_TI_MvBidirectional4;

/**
 ******************************************************************************
 *  @struct IMPEG4VDEC_TI_MbInfo
 *
 *  @brief  This structure details the data format for MB information shared to
 *          application. This helps application to understand all fields
 *          the way codec uses MB info internally. This structure is of size
 *          112 Bytes.
 *
 *  @param  info : This elements gives details about the MB placement in the
 *                 frame.
 *
 *  @param  IQedDCY2: This field holds the Inverse Quantized DC for Y2 MB
 *
 *  @param  IQedDCY3: This field holds the Inverse Quantized DC for Y3 MB
 *
 *  @param  IQedDCY2: This field holds the Inverse Quantized DC for Cb MB
 *
 *  @param  IQedDCY3: This field holds the Inverse Quantized DC for Cr MB
 *
 *  @param  IQedDCY1: This field holds the Inverse Quantized DC for Y1 MB
 *
 *  @param  zeroes1[3]: This field is set to 0
 *
 *  @param  DP_DC_Y0: If data partition = 1, this field contains DC coefficient
 *                    values for Y0 MB
 *
 *  @param  DP_DC_Y1: If data partition = 1, this field contains DC coefficient
 *                    values for Y1 MB
 *
 *  @param  DP_DC_Y2: If data partition = 1, this field contains DC coefficient
 *                    values for Y2 MB
 *
 *  @param  DP_DC_Y3: If data partition = 1, this field contains DC coefficient
 *                    values for Y3 MB
 *
 *  @param  DP_DC_Cb: If data partition = 1, this field contains DC coefficient
 *                    values for Cb MB
 *
 *  @param  DP_DC_Cr: If data partition = 1, this field contains DC coefficient
 *                    values for Cr MB
 *
 *  @param  Reserved: Reserved field
 *
 *  @param  zeroes2[2]: This field is set to 0
 *
 *  @param  codecSpecificinfoWordSix: Codec specific fields
 *
 *  @param  codecSpecificinfoWordSeven: Codec specific fields
 *
 *  @param  zeroes3[4]: This field is set to 0
 *
 *  @param  mv_forward_backward: Lists all Motion vectors at 4x4 level in L0 &
 *                               L1 direction. First 4 MVs in L0 next 4 MVs in
 *                               L1 direction.
 *
 *  @param  bidirectional4: Lists all Motion vectors at 8x8 level in both
 *                          directions
 ******************************************************************************
*/
typedef struct _IMPEG4VDEC_TI_MbInfo
{
  IMPEG4VDEC_TI_CommonInfo info;

  XDAS_Int16 IQedDCY2;
  XDAS_Int16 IQedDCY3;
  XDAS_Int16 IQedDCCb;
  XDAS_Int16 IQedDCCr;

  XDAS_Int16 IQedDCY1;
  XDAS_Int16 zeroes1[3];

  XDAS_Int16 DP_DC_Y0;
  XDAS_Int16 DP_DC_Y1;
  XDAS_Int16 DP_DC_Y2;
  XDAS_Int16 DP_DC_Y3;

  XDAS_Int16 DP_DC_Cb;
  XDAS_Int16 DP_DC_Cr;
  XDAS_Int32 Reserved;

  XDAS_Int32 zeroes2[2];

  IMPEG4VDEC_TI_CodecSpecificWordSix codecSpecificinfoWordSix;

  IMPEG4VDEC_TI_CodecSpecificWordSeven codecSpecificinfoWordSeven;

  XDAS_Int32 zeroes3[4];

  union {
    IMPEG4VDEC_TI_MotionVector       mv_forward_backward[8];
    IMPEG4VDEC_TI_MvBidirectional4   bidirectional4;
  } IMPEG4VDEC_TI_motion_vecs;


} IMPEG4VDEC_TI_MbInfo;

#endif /*_IMPEG4VDEC_H_*/











