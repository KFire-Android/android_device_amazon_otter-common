/*
********************************************************************************
* HDVICP2.0 Based JPEG Decoder
*
* "HDVICP2.0 Based JPEG Decoder" is software module developed on
* TI's HDVICP2 based SOCs. This module is capable of generating a raw image
* by de-compressing/decoding a jpeg bit-stream based on ISO/IEC IS 10918-1.
* Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
********************************************************************************
*/

/**
********************************************************************************
* @file      ijpegvdec.h
*
* @brief     This file provides definisions for the interface handles.
*
* @author    Odanaka
*
* @version 0.0 (Dec 2008) : Created the initial version.
*
* @version 0.1 (Dec 2009)  : Added extended dynamic paramters[Chetan]
*
* @version 0.2 (Feb 2010)  : Coding Guidelines[Chetan]
*
* @version 0.3 (July 2010)  : Added Error Robustness Error Codes[Chetan]
*
* @version 0.4 (Sept 2010)  : Added Error Robustness Error Codes for Data Sync
*                             [Chetan]
*
* @version 0.5 (Nov 2010) : Added support for Slice level decoding[Chetan]
*
* @version 0.6 (Feb 2011) : Added error codes for unsupported features [Chetan]
*
* @version 0.7 (Sep 2011) : Exposed default static/dynamic params [Naidu]
*
* @version 0.8 (Sep 2011) : Added error codes for un-supported resultions
*                           [Naidu]
*
*******************************************************************************
*/

/*----------------------compilation control switches -------------------------*/
#ifndef _IJPEGVDEC_
#define _IJPEGVDEC_

/*******************************************************************************
*   INCLUDE FILES
*******************************************************************************/
/* -------------------- system and platform files ----------------------------*/
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividdec3.h>
#include <ti/xdais/ires.h>
//#include <ti/sdo/fc/ires/hdvicp/ires_hdvicp2.h>

/*----------------------program files ----------------------------------------*/

/*******************************************************************************
*   PUBLIC DECLARATIONS Defined here, used elsewhere
*******************************************************************************/
/*-----------------------data declarations -----------------------------------*/


/**
*******************************************************************************
*  @enum       ethumbnailMode
*
*  @brief      Enumerator which defines all the different  types of thumbnail
*              supported.
*
*  @remarks    IJPEGVDEC_THUMBNAIL_JFIF        :  Decode and output thumbnail
*                                       available with JFIF marker
*
*  @remarks    IJPEGVDEC_THUMBNAIL_EXIF        :  Decode and output thumbnail
*                                       available with EXIF marker
*
*  @remarks    IJPEGVDEC_THUMBNAIL_DOWNSAMPLE  :  Decode the image and
*                                downsample it toprovide it as thumbnail output
*
*******************************************************************************
*/
typedef enum {
    IJPEGVDEC_THUMBNAIL_JFIF = 1,
    IJPEGVDEC_THUMBNAIL_EXIF = 2,
    IJPEGVDEC_THUMBNAIL_DOWNSAMPLE = 3
}ethumbnailMode;

/**
*******************************************************************************
*  @enum       edownSamplingFactor
*
*  @brief      Enumerator which defines the factor with which the downsampling
*              needs to be carried out. This applies to both Horizontal
*              and vertical dimentions
*
*  @remarks    IJPEGVDEC_NODOWNSAMPLE        :  No Down Sample.
*
*  @remarks    IJPEGVDEC_DOWNSAMPLEBY2       :  Downscale by 2
*
*  @remarks    IJPEGVDEC_DOWNSAMPLEBY4       :  Downscale by 4
*
*
*******************************************************************************
*/
typedef enum {
    IJPEGVDEC_NODOWNSAMPLE  = 1,
    IJPEGVDEC_DOWNSAMPLEBY2 = 2,
    IJPEGVDEC_DOWNSAMPLEBY4 = 4
} edownSamplingFactor;

/**
*******************************************************************************
*  @enum       eFrameErrorConcealment
*
*  @brief      Enumerator which defines values to ENABLE or DISABLE
*              Error Concealment
*
*  @remarks    IJPEGVDEC_EC_DISABLE      :  Disable Error concealment
*
*  @remarks    IJPEGVDEC_EC_ENABLE       :  Enable Error Concealment
*
*
*******************************************************************************
*/
typedef enum {
    IJPEGVDEC_EC_DISABLE = 0,
    IJPEGVDEC_EC_ENABLE
}eFrameErrorConcealment_jpegvdec;

/**
*******************************************************************************
*  @struct IJPEGVDEC_Obj
*
*  @brief  This structure must be the first field of all jpgVDEC instance
*          objects
*
*  @param  fxns  : Handle to extented jpeg video decoder library interface
*                  functions
*
*******************************************************************************
*/
typedef struct IJPEGVDEC_Obj {
    struct IJPEGVDEC_Fxns *fxns;
} IJPEGVDEC_Obj;

/**
*******************************************************************************
*  @struct IJPEGVDEC_Handle
*
*  @brief  This handle is used to reference all jpgVDEC instance objects
*
*******************************************************************************
*/
typedef struct IJPEGVDEC_Obj *IJPEGVDEC_Handle;

/**
*******************************************************************************
*  @struct IJPEGVDEC_Status
*
*  @brief  Status structure defines the parameters that can be changed or
*          read during real-time operation of the alogrithm.
*
*  @param  viddecStatus  :  Handle to base class status struture which defines
*                           the all  run time parameters.
*
*  @param extendedErrorCode0 : Extended Error Code0 returned by decoder
*
*  @param extendedErrorCode1 : Extended Error Code1 returned by decoder
*
*  @param extendedErrorCode2 : Extended Error Code2 returned by decoder
*
*  @param extendedErrorCode3 : Extended Error Code3 returned by decoder
*
*  @param debugTraceLevel    : DebugTrace level being used by decoder
*
*  @param lastNFramesToLog   : Number of frames of debug data decoder is
*                              dumping trace buffer
*
*  @param extMemoryDebugTraceAddr : Trace buffer base address in external memory
*
*  @param extMemoryDebugTraceSize : Size of Trace buffer in external memory
*
*******************************************************************************
*/
typedef struct IJPEGVDEC_Status {
    /*--------------------------------------------------------------------------*/
    /*Base Class                                                                */
    /*--------------------------------------------------------------------------*/
    IVIDDEC3_Status viddecStatus;

    /*--------------------------------------------------------------------------*/
    /*Extended Error Code0 returned by decoder                                  */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 extendedErrorCode0;

    /*--------------------------------------------------------------------------*/
    /*Extended Error Code1 returned by decoder                                  */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 extendedErrorCode1;

    /*--------------------------------------------------------------------------*/
    /*Extended Error Code2 returned by decoder                                  */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 extendedErrorCode2;

    /*--------------------------------------------------------------------------*/
    /*Extended Error Code3 returned by decoder                                  */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 extendedErrorCode3;

    /*--------------------------------------------------------------------------*/
    /*DebugTrace level being used by decoder                                    */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 debugTraceLevel;

    /*--------------------------------------------------------------------------*/
    /*Number of frames of debug data decoder is dumping trace buffer            */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 lastNFramesToLog;

    /*--------------------------------------------------------------------------*/
    /* Trace buffer base address in external memory                             */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 *extMemoryDebugTraceAddr;

    /*--------------------------------------------------------------------------*/
    /* Size of Trace buffer in external memory                                  */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 extMemoryDebugTraceSize;

} IJPEGVDEC_Status;

/**
*******************************************************************************
*  @struct IJPEGVDEC_Cmd
*
*  @brief  The Cmd enumeration defines the control commands for the JPEG
*                video decoder control method.
*
*******************************************************************************
*/
typedef IVIDDEC3_Cmd IJPEGVDEC_Cmd;

/**
*******************************************************************************
*  @struct IJPEGVDEC_Params
*
*  @brief  This structure defines the creation parameters for all
*          jpgVDEC objects
*
*  @param  viddecParams  :  Defines the creation time parameters for
*                           all IVIDDEC3 instance objects.
*
*  @param  ErrorConcealmentON  :  Enable/Disable Error Concealment
*
*  @param  debugTraceLevel  :  Enable/Disable Error Concealment
*
*  @param  lastNFramesToLog  :  Number of frames to log history for debugTrace
*
*  @param  sliceSwitchON     :  ENABLE/DISABLE Slice Switching  feature.
*
*  @param  numSwitchPerFrame     :  Number of Switches in a Frame.
*
*  @param  numRestartMarkerPerSwitch  :  Number of Restart Marker(slices) to
*                                  decode in one process call or in one switch.
*
*******************************************************************************
*/
typedef struct IJPEGVDEC_Params {
    /*--------------------------------------------------------------------------*/
    /*Base Class                                                                */
    /*--------------------------------------------------------------------------*/
    IVIDDEC3_Params viddecParams;

    /*--------------------------------------------------------------------------*/
    /*Enable/Disable Error Concealment                                          */
    /* enumeration 'eFrameErrorConcealment' can be used to set this value       */
    /*--------------------------------------------------------------------------*/
    XDAS_Int32 ErrorConcealmentON;

    /*--------------------------------------------------------------------------*/
    /*Debug trace Level                                                         */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 debugTraceLevel;

    /*--------------------------------------------------------------------------*/
    /*Number of frames to log history for debugTrace                            */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 lastNFramesToLog;

    /*--------------------------------------------------------------------------*/
    /* ENABLE/DISABLE Slice Switching  feature.                                 */
    /* enumeration 'eSliceSwitch' can be used to set this value                 */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 sliceSwitchON;

    /*--------------------------------------------------------------------------*/
    /* Number of Switches in a Frame. This is valid only when sliceSwitchON is  */
    /* enabled , when disabled , its dont care.                                 */
    /* Application tells how many switches should happen in a frame , codec has */
    /* to decide how to handle each process call ( how many slices) and it has  */
    /* process "numSwitchInFrame " process calls only                           */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 numSwitchPerFrame;

    /*--------------------------------------------------------------------------*/
    /* Application tells a number meaning codec has to decode this much slices  */
    /* or restart markers and come out of process call. Here each switch can    */
    /*have N number of restart marker ( N should be greater than or equal to 1).*/
    /* Once codec process call is done , application will give again the next   */
    /* number and application will handle till all the slices in the frame have */
    /* been decoded.                                                            */
    /* This paramter is valid only when "sliceSwitchON"  is enabled             */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 numRestartMarkerPerSwitch;

} IJPEGVDEC_Params;

/**
*******************************************************************************
*  @struct IJPEGVDEC_DynamicParams
*
*  @brief  This structure defines the run time parameters for all
*          jpgVDEC objects
*
*  @param  viddecDynamicParams  :  Defines the run time parameters for
*                                  all IVIDDEC3 instance objects.
*
*  @param  decodeThumbnail  :  Decode the ThumNail and provide the output for
*                              display.
*
*  @param  thumbnailMode  :  Specifies which thumbnail to decode. If none of
*                            the markers (JFIF and EXIF) have thumbnail image,
*                            use IJPEGVDEC_THUMBNAIL_DOWNSAMPLE. Refer to
*                            thumbnailMode enumeration
*
*  @param  downsamplingFactor  : If decodeThumbnail is enabled and
*                                thumbnailMode is set to
*                                IJPEGVDEC_THUMBNAIL_DOWNSAMPLE,
*                                downsamplingFactor is used as scaling factor
*                                for ThumNail output
*                                If decodeThumbnail is disabled, this parameter
*                                is used as scaling factor for display buffer
*                                output. Refer to  edownSamplingFactor
*
*  @param  streamingCompliant  : If an Input Image is Non-Interleaved ,
*                                Application has to set this params to "0"
*                                (DISABLE) , if it is Interleaved , value will
*                                be "1" (ENABLE). This Paramater along with
*                                ForceChromaFormat determines whether we
*                                have to give Planar Buffers from GetBufinfo.
*
*******************************************************************************
*/
typedef struct IJPEGVDEC_DynamicParams {
    IVIDDEC3_DynamicParams viddecDynamicParams;

    XDAS_Int32 decodeThumbnail;

    XDAS_Int32 thumbnailMode;

    XDAS_Int32 downsamplingFactor;

    XDAS_Int32 streamingCompliant;

} IJPEGVDEC_DynamicParams;

/**
*******************************************************************************
*  @struct IJPEGVDEC_InArgs
*
*  @brief  This structure defines the run time input arguments for all VIDDEC
*          objects.
*
*  @param  viddecInArgs       :  Defines the input arguments for all
*                                IVIDDEC3 instance process function.
*
*******************************************************************************
*/
typedef struct IJPEGVDEC_InArgs {
    IVIDDEC3_InArgs viddecInArgs;
}IJPEGVDEC_InArgs;

/**
*******************************************************************************
*  @struct IJPEGVDEC_OutArgs
*
*  @brief  This structure defines the run time output arguments for VIDDEC
*          objects.This structure may be extended by individual codec
*                implementation allowing customization with vendor specific
*          parameters.
*
*  @param  viddecOutArgs      :  Defines the output arguments for all
*                                IVIDDEC3 instance process function.
*
*******************************************************************************
*/
typedef struct IJPEGVDEC_OutArgs {
    IVIDDEC3_OutArgs viddecOutArgs;
    XDAS_UInt32      IsGrayFlag;
}IJPEGVDEC_OutArgs;


/**
*******************************************************************************
*  @struct IJPEGVDEC_Fxns
*
*  @brief  This structure defines all of the operations on jpgVDEC objects.
*
*  @param  ividdec            :  handle to the all function of the operations
*                                on IVIDDEC3 objects
*
*******************************************************************************
*/
typedef struct IJPEGVDEC_Fxns {
    IVIDDEC3_Fxns ividdec;
} IJPEGVDEC_Fxns;

typedef enum {
    IJPEGDEC_ERR_UNSUPPORTED_VIDDEC3PARAMS = 0,
    IJPEGDEC_ERR_UNSUPPORTED_VIDDEC3DYNAMICPARAMS,
    IJPEGDEC_ERR_UNSUPPORTED_JPEGDECDYNAMICPARAMS,
    IJPEGDEC_ERR_NOSLICE,
    IJPEGDEC_ERR_MBDATA,
    IJPEGDEC_ERR_STANDBY,
    IJPEGDEC_ERR_INVALID_MBOX_MESSAGE,
    IJPEGDEC_ERR_HDVICP_RESET,
    IJPEGDEC_ERR_HDVICP_WAIT_NOT_CLEAN_EXIT = 16,
    IJPEGDEC_ERR_FRAME_HDR,
    IJPEGDEC_ERR_SCAN_HDR,
    IJPEGDEC_ERR_HUFF_TBL_HDR,
    IJPEGDEC_ERR_QUANT_TBL_HDR,
    IJPEGDEC_ERR_OUTCHROMAFORMAT,
    IJPEGDEC_ERR_UNSUPPORTED_MARKER,
    IJPEGDEC_ERR_THUMBNAIL,
    IJPEGDEC_ERR_IRES_HANDLE,
    IJPEGDEC_ERR_DYNAMIC_PARAMS_HANDLE,

    /* Error Codes of Data Sync */
    IJPEGDEC_ERR_DATASYNC,
    IJPEGDEC_ERR_DOWNSAMPLE_INPUT_FORMAT,
    IJPEGDEC_ERR_NOT_SUPPORTED_FEATURE,
    IJPEGDEC_ERR_NOT_SUPPORTED_RESOLUTION

}IJPEGDEC_ExtendedErrorCodes;

typedef enum {
    JPEG_DECODE_THUMBNAIL_ERROR = 0,
    JPEG_DYNAMIC_PARAMS_HANDLE_ERROR,
    JPEG_THUMBNAIL_MODE_ERROR,
    JPEG_DOWNSAMPLING_FACTOR_ERROR,
    JPEG_STREAMING_COMPLIANT_ERROR,
    JPEG_NON_INTERLEAVED_STREAMING_COMPLIANT_ERROR,
    JPEG_DECODE_HEADER_ERROR,
    JPEG_DISPLAY_WIDTH_ERROR,
    JPEG_DYNAMIC_PARAMS_SIZE_ERROR,
    JPEG_NULL_INSTANCE_HANDLE_ERROR,
    JPEG_NULL_INARGS_POINTER_ERROR,
    JPEG_NULL_OUTARGS_POINTER_ERROR,
    JPEG_NULL_INPUT_BUF_DESC_ERROR,
    JPEG_NULL_OUTPUT_BUF_DESC_ERROR,
    JPEG_INVALID_INARGS_SIZE,
    JPEG_INVALID_OUTARGS_SIZE,
    JPEG_NULL_INPUT_BUFFER_POINTER_ERROR,
    JPEG_NULL_OUTPUT_BUF_DESC_POINTER_ERROR,
    JPEG_INVALID_NUM_OF_INPUT_BUFFERS_ERROR,
    JPEG_INVALID_INPUT_BYTES_ERROR,
    JPEG_INVALID_INPUT_BUFFER_MEMORY_TYPE_ERROR,
    JPEG_INVALID_NUM_OF_OUTPUT_BUFFERS_ERROR,
    JPEG_NULL_OUTPUT_BUFFER_POINTER0_ERROR,
    JPEG_INVALID_OUTPUT_BUFFER0_SIZE_ERROR,
    JPEG_INVALID_OUTPUT_BUFFER0_MEMTYPE_ERROR,
    JPEG_NULL_OUTPUT_BUFFER_POINTER1_ERROR,
    JPEG_INVALID_OUTPUT_BUFFER1_SIZE_ERROR,
    JPEG_INVALID_OUTPUT_BUFFER1_MEMTYPE_ERROR,
    JPEG_NULL_OUTPUT_BUFFER_POINTER2_ERROR,
    JPEG_INVALID_OUTPUT_BUFFER2_SIZE_ERROR,
    JPEG_INVALID_OUTPUT_BUFFER2_MEMTYPE_ERROR,
    JPEG_INVALID_INPUT_ID_ERROR,
    JPEG_NUM_VDMA_DESC_EXCEEDS_ERROR,
    JPEG_INVALID_SOI_MARKER_ERROR,
    JPEG_INVALID_MARKER_SEG_LENGTH_ERROR,
    JPEG_NON_STANDARD_MARKER_CODE_ERROR,
    JPEG_INVALID_QUANT_TABLE_TYPE_ERROR,
    JPEG_QUANT_TABLE_BYTES_READ_ERROR,
    JPEG_INVALID_HUFFMAN_TABLE_TYPE_ERROR,
    JPEG_HUFFMAN_CODE_LENGTH_SIZE_EXCEED_ERROR,
    JPEG_HUFFMAN_TABLE_MARKER_SEG_SIZE_ERROR,
    JPEG_HUFFMAN_TABLE_BYTES_READ_ERROR,
    JPEG_INVALID_SAMPLE_PRECISION_ERROR,
    JPEG_INVALID_NUM_COMPONENTS_ERROR,
    JPEG_FRAME_HDR_BYTES_READ_ERROR,
    JPEG_NOT_SUPPORTED_FORMAT_ERROR,
    JPEG_ARITHMETIC_DECODING_NOT_SUPPORTED_MARKER_ERROR,
    JPEG_PROG_DECODING_NOT_SUPPORTED_MARKER_ERROR,
    JPEG_LOSSLESS_DECODING_NOT_SUPPORTED_MARKER_ERROR,
    JPEG_DIFFERENTIAL_DECODING_NOT_SUPPORTED_MARKER_ERROR,
    JPEG_JFIF_THUMBNAIL_IDENTIFIER_ERROR,
    JPEG_JFIF_THUMBNAIL_BYTES_READ_ERROR,
    JPEG_JFIF_EXTN_NO_SOI_ERROR,
    JPEG_JFIF_NOT_SUPPORTED_FEATURE_ERROR,
    JPEG_FORCECHROMA_OUTPUTCHROMA_FORMAT_MISMATCH_ERROR,
    JPEG_INVALID_VERT_SCAN_FREQ_ERROR,
    JPEG_INVALID_HORI_SCAN_FREQ_ERROR,
    JPEG_INVALID_QUANT_DEST_SELECTOR_ERROR,
    JPEG_DC_ENTROPY_CODING_DEST_ERROR,
    JPEG_AC_ENTROPY_CODING_DEST_ERROR,
    JPEG_ECD_VLD_OUT_OF_TABLE_ERROR,
    JPEG_ECD_RESTART_INTERVAL_ERROR,
    JPEG_ECD_BLOCK_COEFF_NUM_ERROR,
    JPEG_GET_DATA_SYNC_NULL_FUNC_POINTER_ERROR,
    JPEG_PUT_DATA_SYNC_NULL_FUNC_POINTER_ERROR,
    JPEG_HDVICP_ACQUIRE_AND_CONFIGURE_ERROR,
    JPEG_NULL_ALGORITHM_HANDLE_ERROR,
    JPEG_GETVERSION_NULL_BUF_POINTER_ERROR,
    JPEG_IRES_RESOURCE_DESC_ERROR,
    JPEG_IRES_RESOURCE_DESC_HANDLE_ERROR,
    JPEG_NULL_STATUS_DATA_BUF,
    JPEG_EXCEED_BYTES_CONSUMED_ERROR,

    /* Extended Error Codes for Data Sync */
    JPEG_INPUT_DATASYNC_NUMBLOCKS_ERROR,
    JPEG_INPUT_DATASYNC_BUFF_POINTER_ERROR,
    JPEG_INPUT_DATASYNC_BLOCKSIZE_ERROR,
    JPEG_INPUT_DATASYNC_NOT_VALID,

    JPEG_OUTPUT_DATASYNC_NUMBLOCKS_ERROR,

    JPEG_SLICE_LEVEL_INPUT_NO_RST_MARKER_ERROR,
    JPEG_DOWNSAMPLING_IN_NON_TILED_ERROR,
    JPEG_DOWNSAMPLING_NOT_SUPPORTED_FORMAT_ERROR,
    JPEG_DOWNSAMPLING_NOT_SUPPORTED_FEATURE_ERROR,
    JPEG_THUMBNAIL_NOT_SUPPORTED_FEATURE_ERROR,

    /* Extended Error Codes for Unsupported Resolution */
    JPEG_NOT_SUPPORTED_WIDTH_ERROR,
    JPEG_NOT_SUPPORTED_HEIGHT_ERROR

}IjpegVDEC_ErrorStatus;
/*
 *  ======== IJPEGVDEC_Params   ========
 *  Default parameter values for JPEGVDEC instance objects
 */
extern const IJPEGVDEC_Params    JPEGVDEC_TI_Static_Params;
/*
 *  ======== IJPEGVDEC_DynamicParams ========
 *  Default dynamic parameter values for JPEGVDEC instance objects
 */
extern const IJPEGVDEC_DynamicParams    JPEGVDEC_TI_DynamicParams;
/* ------------------------------ macros ------------------------------------ */

/****************************************************************
*   PRIVATE DECLARATIONS Defined here, used only here
****************************************************************/
/*--------data declarations -----------------------------------*/

/**
* Extending control method commands
*/
/**
* The Macro indicates the Get Status Command Enum for the Control API
*/
#define IJPEGVDEC_GETSTATUS             XDM_GETSTATUS

/**
* The Macro indicates the SET PARAMS Command Enum for the Control API
*/
#define IJPEGVDEC_SETPARAMS             XDM_SETPARAMS

/**
* The Macro indicates the XDM RESET Command Enum for the Control API
*/
#define IJPEGVDEC_RESET                 XDM_RESET

/**
* The Macro indicates the XDM FLUSH Command Enum for the Control API
*/
#define IJPEGVDEC_FLUSH                 XDM_FLUSH

/**
* The Macro indicates the SET DEFUALT Command Enum for the Control API
*/
#define IJPEGVDEC_SETDEFAULT            XDM_SETDEFAULT

/**
* The Macro indicates the Get BUFINFO Command Enum for the Control API
*/
#define IJPEGVDEC_GETBUFINFO            XDM_GETBUFINFO

/**
* The Macro indicates the Get VERSION Command Enum for the Control API
*/
#define IJPEGVDEC_GETVERSION            XDM_GETVERSION

/**
* The Macro indicates the Get CONTEXTINFO Command Enum for the Control API
*/
#define IJPEGVDEC_GETCONTEXTINFO        XDM_GETCONTEXTINFO

/**
* The Macro indicates the Get DYNAMICPARAM DEFAULT Command Enum for the
* Control API
*/
#define IJPEGVDEC_GETDYNPARAMSDEFAULT   XDM_GETDYNPARAMSDEFAULT

#endif  /* _IJPEGVDEC_ */

