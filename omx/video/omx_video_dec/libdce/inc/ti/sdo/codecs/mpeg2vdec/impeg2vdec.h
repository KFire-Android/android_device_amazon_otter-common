/*
********************************************************************************
* HDVICP2.0 Based MPEG-2 MP Decoder
*
* "HDVICP2.0 Based MPEG-2 MP Decoder" is software module developed on TI's
* HDVICP2 based SOCs. This module is capable of generating a raw 4:2:0 video
* data by de-compressing/decoding a main/simple profile bit-stream based on
* ISO/IEC 13818-2.
* Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
********************************************************************************
*/

/**
********************************************************************************
* @file      impeg2vdec.h
*
* @brief     This file provides definisions for the interface(API) parameters.
*
* @author    Prashanth
*
* @version 0.0 (July 2008) : Created  [Prashanth]
* @version 0.1 (Dec 2009)  : Added extended dynamic paramters[Deepa]
* @version 0.2 (Feb 2010)  : Coding Guidelines[Deepa]
* @version 0.3 (July 2010) : Error robustness added.
*                            [Deepa Nagendra].
* @version 0.4 (Aug 2010)  : Debug trace implementation.
*                            [Deepa Nagendra].
* @version 0.5 (Aug 2010)  : Error Concealment support.
*                            [Deepa Nagendra].
* @version 0.6 (Dec 2011)  : Default structures are exposed in interface file
*                            and macros are differenciated with codec name.
*                            [Naidu].
* @version 0.6 (July 2012)  : Added extended error codes for errors incase
*                            referenceframe is not available while decoding
*                            after flush and SEEK [Naidu].
*
********************************************************************************
*/
/* ---------------------- compilation control switches ---------------------- */

#ifndef _IMPEG2VDEC_
#define _IMPEG2VDEC_


/*******************************************************************************
*   INCLUDE FILES
*******************************************************************************/
/* ---------------------- system and platform files ------------------------- */
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividdec3.h>
#include <ti/xdais/ires.h>
//#include <ti/sdo/fc/ires/hdvicp/ires_hdvicp2.h>
/* --------------------------- program files -------------------------------- */

/*******************************************************************************
*   PUBLIC DECLARATIONS Defined here, used elsewhere
*******************************************************************************/
/*-----------------------data declarations -----------------------------------*/


/*******************************************************************************
*   PRIVATE DECLARATIONS Defined here, used only here
*******************************************************************************/
/*-----------------------data declarations -----------------------------------*/

/*-----------------------function prototypes ---------------------------------*/

/**
********************************************************************************
*  @struct     IMPEG2VDEC_Obj
*
*  @brief      This structure must be the first field of all Mpeg2vdec instance
*              objects
*
*  @param      fxns  :  Handle to extented mpeg2 video decoder library
*                       interface functions
*
********************************************************************************
*/
typedef struct IMPEG2VDEC_Obj {
    struct IMPEG2VDEC_Fxns *fxns;
} IMPEG2VDEC_Obj;

/**
********************************************************************************
*  @struct     IMPEG2VDEC_Handle
*
*  @brief       This handle is used to reference all Mpeg2vdec instance objects
*
********************************************************************************
*/
typedef struct IMPEG2VDEC_Obj *IMPEG2VDEC_Handle;

/**
********************************************************************************
*  @struct     IMPEG2VDEC_Status
*
*  @brief      Status structure defines the parameters that can be changed or
*              read during real-time operation of the alogrithm.
*
*  @param      viddecStatus  :  Handle to base class status struture which
*                               defines the all  run time parameters.
********************************************************************************
*/
typedef struct IMPEG2VDEC_Status {
    IVIDDEC3_Status viddecStatus;
    /* Extended Error Code0 returned by decoder  */
    XDAS_UInt32 extendedErrorCode0;
    /* Extended Error Code1 returned by decoder  */
    XDAS_UInt32 extendedErrorCode1;
    /* Extended Error Code2 returned by decoder  */
    XDAS_UInt32 extendedErrorCode2;
    /* Extended Error Code3 returned by decoder  */
    XDAS_UInt32 extendedErrorCode3;

    XDAS_UInt32 debugTraceLevel;

    XDAS_UInt32 lastNFramesToLog;

    XDAS_UInt32 *extMemoryDebugTraceAddr;

    XDAS_UInt32 extMemoryDebugTraceSize;

} IMPEG2VDEC_Status;

/**
*  @brief    The Cmd enumeration defines the control commands for the MPEG2
*            video decoder control method.
*/
typedef IVIDDEC3_Cmd IMPEG2VDEC_Cmd;

/**
********************************************************************************
*  @struct     IMPEG2VDEC_Params
*
*  @brief      This structure defines the creation parameters for all
*              mpeg2VDEC objects
*
*  @param      viddecParams  :  Defines the creation time parameters for
*                               all IVIDDEC1 instance objects.
*
********************************************************************************
*/
typedef struct IMPEG2VDEC_Params {

    IVIDDEC3_Params viddecParams;
    XDAS_Int32      ErrorConcealmentON;
    XDAS_Int32      outloopDeBlocking;

    /*--------------------------------------------------------------------------*/
    /*Debug trace Level                                                         */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 debugTraceLevel;

    /*--------------------------------------------------------------------------*/
    /*History of last N frames                                                  */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 lastNFramesToLog;

} IMPEG2VDEC_Params;

/**
********************************************************************************
*  @struct     IMPEG2VDEC_DynamicParams
*
*  @brief      This structure defines the run time parameters for all
*              Mpeg2vdec objects
*
*  @param      viddecDynamicParams  :  Defines the run time parameters for
*                                      all IVIDDEC3 instance objects.
*
********************************************************************************
*/
typedef struct IMPEG2VDEC_DynamicParams {
    /*Base class Parameters*/
    IVIDDEC3_DynamicParams viddecDynamicParams;

    /*--------------------------------------------------------------------------*/
    /* gotoNextIFrame : If enabled, during process call skips decoding of all   */
    /*                  non I frames. Enable seekFrameEnd to get the bytes      */
    /*                   consumed for each non I frame                          */
    /*--------------------------------------------------------------------------*/
    XDAS_Int32 gotoNextIFrame;
    /*--------------------------------------------------------------------------*/
    /* skipBFrame  : If enabled, skips decoding of all B frames.                */
    /*               Enable seekFrameEnd to get the bytes consumed for the      */
    /*               B-frames                                                   */
    /*--------------------------------------------------------------------------*/
    XDAS_Int32 skipBFrame;
    /*--------------------------------------------------------------------------*/
    /* skipCurrFrame  : If enabled, skips decoding of current frame             */
    /*               Enable seekFrameEnd to get the bytes consumed for the      */
    /*               Current frame                                              */
    /*--------------------------------------------------------------------------*/
    XDAS_Int32 skipCurrFrame;
    /*--------------------------------------------------------------------------*/
    /* seekFrameEnd  : If enabled along with any SkipFrame option, prcoess call */
    /*                 returns the bytesconsumed for the frame w/o decoding the */
    /*                 frame                                                    */
    /*--------------------------------------------------------------------------*/
    XDAS_Int32 seekFrameEnd;

} IMPEG2VDEC_DynamicParams;

/**
********************************************************************************
*  @struct     IMPEG2VDEC_InArgs
*
*  @brief      This structure defines the runtime input arguments for all VIDDEC
*              objects.
*
*  @param      viddecInArgs  :   Defines the input arguments for all IVIDDEC3
*                                instance process function.
*
********************************************************************************
*/
typedef struct IMPEG2VDEC_InArgs {

    IVIDDEC3_InArgs viddecInArgs;

}IMPEG2VDEC_InArgs;

/**
********************************************************************************
*  @struct     IMPEG2VDEC_OutArgs
*
*  @brief      This structure defines the run time output arguments for VIDDEC
*              objects.
*
*  @param      viddecOutArgs  :   Defines the output arguments for all IVIDDEC3
*                                 instance  process function.
*
********************************************************************************
*/
typedef struct IMPEG2VDEC_OutArgs {
    IVIDDEC3_OutArgs viddecOutArgs;
}IMPEG2VDEC_OutArgs;

/**
********************************************************************************
*  @struct     IMPEG2VDEC_Fxns
*
*  @brief     This structure defines all of the operations on Mpeg2vdec objects.
*
*  @param      ividdec  :  handle to the all function of the operations on
*                          IVIDDEC3 objects
*
********************************************************************************
*/
typedef struct IMPEG2VDEC_Fxns {
    IVIDDEC3_Fxns ividdec;
} IMPEG2VDEC_Fxns;

/**
 * Error concealment can be enabled or disabled through create time parameter.
*/
typedef enum {
    IMPEG2VDEC_EC_DISABLE = 0,
    IMPEG2VDEC_EC_ENABLE
}eFrameErrorConcealment;


/**
 *  Flag to deblock enable
 */
#define IMPEG2VDEC_DEBLOCK_ENABLE    (1)

/**
 *  Flag to deblock disable
 */
#define IMPEG2VDEC_DEBLOCK_DISABLE   (0)

/**
* Number of MemTab required if Deblock is Off
*/
#define IMPEG2VDEC_NUM_MEMTABS_DEBLOCK_OFF (0)

/**
* Number of memtab required if deblock is on and it will be  maximum
* resource required by codec
*/
#define IMPEG2VDEC_NUM_MEMTABS_DEBLOCK_ON (6)

/**
 * The error codes correspond to the 32-bit extended error parameter passed
 * through outargs and get sttus. The error have been categorised to the below
 * 32 groups and the respective bit is set on error occurrence.
 */
typedef enum {
    IMPEG2VDEC_ERR_UNSUPPORTED_VIDDEC3PARAMS = 0,
    IMPEG2VDEC_ERR_UNSUPPORTED_VIDDEC3DYNAMICPARAMS,
    IMPEG2VDEC_ERR_UNSUPPORTED_MPEG2DECDYNAMICPARAMS,
    IMPEG2VDEC_ERR_IMPROPER_DATASYNC_SETTING,

    IMPEG2VDEC_ERR_NOSLICE,
    IMPEG2VDEC_ERR_SLICEHDR,
    IMPEG2VDEC_ERR_MBDATA,
    IMPEG2VDEC_ERR_UNSUPPFEATURE,

    IMPEG2VDEC_ERR_STREAM_END = 16,
    IMPEG2VDEC_ERR_UNSUPPRESOLUTION,
    IMPEG2VDEC_ERR_STANDBY,
    IMPEG2VDEC_ERR_INVALID_MBOX_MESSAGE,

    IMPEG2VDEC_ERR_HDVICP_RESET,
    IMPEG2VDEC_ERR_HDVICP_WAIT_NOT_CLEAN_EXIT,
    IMPEG2VDEC_ERR_SEQHDR,
    IMPEG2VDEC_ERR_GOP_PICHDR,

    IMPEG2VDEC_ERR_SEQLVL_EXTN,
    IMPEG2VDEC_ERR_PICLVL_EXTN,
    IMPEG2VDEC_ERR_TRICK_MODE,
    IMPEG2VDEC_ERR_PICSIZECHANGE,

    IMPEG2VDEC_ERR_SEMANTIC,
    IMPEG2VDEC_ERR_DECODE_EXIT,
    IMPEG2VDEC_ERR_IRES_RESHANDLE,
    IMPEG2VDEC_ERR_IRES_RESDESC

}IMPEG2VDEC_ExtendedErrorCodes;

/**
 * The enum corresponds to the 4 32-bit words used to pass the error codes to
 * the application in the extended parameters of status stucture through the
 * getstatus command. Each bit is set for an error which falls under one of
 * the groups in the outargs 32 bvits.
 */
typedef enum {
    MPEG2_ECD_ILLEGAL_EOM=0,
    MPEG2_ECD_ILLEGAL_EOB,
    MPEG2_ECD_ILLEGAL_MP1_ESCAPE_LVL,
    MPEG2_ECD_ILLEGAL_MP2_ESCAPE_LVL,

    MPEG2_ECD_ILLEGAL_MARKER_CONCEAL,
    MPEG2_ECD_ILLEGAL_MBTYPE_D_PIC,
    MPEG2_ECD_ILLEGAL_DCT_COEFF,
    MPEG2_ECD_ILLEGAL_CBP,

    MPEG2_ECD_ILLEGAL_MOTION_CODE,
    MPEG2_ECD_ILLEGAL_MB_TYPE,
    MPEG2_ECD_ILLEGAL_MB_ADDR_INCR,
    MPEG2_ECD_ILLEGAL_EOS,

    MPEG2_ECD_ILLEGAL_QUANT_SCALE_CODE,
    MPEG2_ECD_ILLEGAL_SLICE_START_POS,
    MPEG2_ECD_ILLEGAL_START_CODE_SEARCH,
    MPEG2_ECD_ILLEGAL_DC_COEFF_OVFL,

    MPEG2_DYNAMIC_PARAMS_HANDLE_ERROR,
    MPEG2_STATUS_HANDLE_ERROR,
    MPEG2_DYNAMIC_PARAMS_SIZE_ERROR,
    MPEG2_STATUS_SIZE_ERROR,

    MPEG2_DECODE_HEADER_ERROR,
    MPEG2_DISPLAY_WIDTH_ERROR,
    MPEG2_FRAME_SKIP_MODE_ERROR,
    MPEG2_NEW_FRAME_FLAG_ERROR,

    MPEG2_GOTO_IFRAME_ERROR,
    MPEG2_SKIP_BFRAME_ERROR,
    MPEG2_SKIP_CURRENTFRAME_ERROR,
    MPEG2_SEEK_FRAMEEND_ERROR,

    MPEG2_NULL_STATUS_DATA_BUF,
    MPEG2_INSUFFICIENT_STATUS_DATA_BUF,
    MPEG2_NULL_INARGS_POINTER_ERROR,
    MPEG2_INARGS_SIZE_ERROR,

    MPEG2_INVALID_INPUT_BYTES_ERROR,
    MPEG2_INVALID_INPUT_ID_ERROR,
    MPEG2_DECODER_NOT_INITIALIZED_ERROR,
    MPEG2_NULL_INPUT_BUF_DESC_ERROR,

    MPEG2_NULL_INPUT_BUFFER_POINTER_ERROR,
    MPEG2_INVALID_INPUT_BUFFER_SIZE_ERROR,
    MPEG2_INVALID_NUM_OF_INPUT_BUFFERS_ERROR,
    MPEG2_EXCESS_NUM_OF_INPUT_BUFFERS_ERROR,

    MPEG2_INVALID_INPUT_BUFFER_MEMTYPE_ERROR,
    MPEG2_NULL_OUTARGS_POINTER_ERROR,
    MPEG2_INVALID_OUTARGS_SIZE,
    MPEG2_NULL_OUTPUT_BUF_DESC_POINTER_ERROR,

    MPEG2_NULL_OUTPUT_BUF_DESC_ERROR,
    MPEG2_INVALID_OUTPUT_BUFFER0_POINTER_ERROR,
    MPEG2_INVALID_OUTPUT_BUFFER0_SIZE_ERROR,
    MPEG2_INVALID_NUM_OF_OUTPUT_BUFFERS_ERROR,

    MPEG2_INVALID_OUTPUT_BUFFER0_MEMTYPE_ERROR,
    MPEG2_INVALID_OUTPUT_BUFFER0_ALIGNMENT_ERROR,
    MPEG2_INVALID_OUTPUT_BUFFER1_POINTER_ERROR,
    MPEG2_INVALID_OUTPUT_BUFFER1_SIZE_ERROR,

    MPEG2_INVALID_OUTPUT_BUFFER1_MEMTYPE_ERROR,
    MPEG2_INVALID_OUTPUT_BUFFER1_ALIGNMENT_ERROR,
    MPEG2_INVALID_OUTPUT_BUFFER2_POINTER_ERROR,
    MPEG2_INVALID_OUTPUT_BUFFER2_SIZE_ERROR,

    MPEG2_INVALID_OUTPUT_BUFFER2_MEMTYPE_ERROR,
    MPEG2_INVALID_BUFFER_USAGE_MODE,
    MPEG2_SEQ_HDR_INVALID_FRAME_WIDTH,
    MPEG2_SEQ_HDR_INVALID_FRAME_HEIGHT,

    MPEG2_SEQ_HDR_INVALID_ASPECT_RATIO,
    MPEG2_SEQ_HDR_INVALID_FRAME_RATE_CODE,
    MPEG2_INVALID_INTRA_QUANT_MAT,
    MPEG2_INVALID_NON_INTRA_QUANT_MAT,

    MPEG2_SEQ_HDR_INVALID_INTRA_ESCAPE_BIT,
    MPEG2_SEQ_HDR_INVALID_PROFILE,
    MPEG2_SEQ_HDR_INVALID_LEVEL,
    MPEG2_SEQ_HDR_INVALID_RESOLUTION_FORLVL,

    MPEG2_SEQ_HDR_INVALID_CHROMA_FORMAT,
    MPEG2_SEQ_HDR_INVALID_LOW_DELAY,
    MPEG2_SEQ_DSP_INVALID_VIDEO_FORMAT,
    MPEG2_SEQ_DSP_INVALID_COLOUR_PRIM,

    MPEG2_SEQ_DSP_INVALID_TRF_CHARS,
    MPEG2_SEQ_DSP_INVALID_MAT_COEFFS,
    MPEG2_GOP_HDR_INVALID_DROP_FLAG,
    MPEG2_GOP_HDR_INVALID_HOUR,

    MPEG2_GOP_HDR_INVALID_MIN,
    MPEG2_GOP_HDR_INVALID_SEC,
    MPEG2_GOP_HDR_INVALID_TIME_CODE_PICTURES,
    MPEG2_GOP_HDR_INVALID_BROKEN_LINK,

    MPEG2_PIC_HDR_INVALID_TEMP_REF,
    MPEG2_PIC_HDR_INVALID_PIC_TYPE,
    MPEG2_PIC_HDR_INVALID_VBV_DELAY,
    MPEG1_PIC_HDR_INVALID_FWD_FCODE,

    MPEG1_PIC_HDR_INVALID_BWD_FCODE,
    MPEG2_PIC_HDR_INVALID_FCODE,
    MPEG2_PIC_HDR_INVALID_PIC_STRUCTURE,
    MPEG2_PIC_HDR_INVALID_FIELD_COMB,

    MPEG2_PIC_HDR_INVALID_TFF,
    MPEG2_PIC_HDR_INVALID_FPFD,
    MPEG2_PIC_HDR_INVALID_RFF,
    MPEG2_PIC_HDR_INVALID_PROG_FLAG,

    MPEG2_QUANT_EXT_INVALID_LOAD_CHROMA_INTRA_FLAG,
    MPEG2_QUANT_EXT_INVALID_LOAD_CHROMA_NON_INTRA_FLAG,
    MPEG2_INVALID_EXTN_CODE,
    MPEG2_SEQ_HDR_MISSING,

    MPEG2_NO_PICTURE_ENCODED_ERROR,
    MPEG2_SEQ_EXT_MISSING,
    MPEG2_PIC_CODING_EXT_MISSING,
    MPEG2_SEQ_DISP_EXT_MISSING,

    MPEG2_GOP_FIRST_FRAME_NOT_I,
    MPEG2_SCALABILITY_NOT_SUPPORTED,
    MPEG2_END_OF_SEQ_DETECTED,
    MPEG2_PIC_HDR_RFF_FRAME_RATE_MISMATCH,

    MPEG2_PIC_HDR_INVALID_DC_PRECISION,
    MPEG2_INVALID_FRAME_RATE,
    MPEG2_INVALID_BIT_RATE,
    MPEG2_FRAME_SKIPPED,

    MPEG2_REF_FRAME_SKIPPED,
    MPEG2_NO_REF_TO_FLUSH,
    MPEG2_EXCESS_INPUT_BYTES,
    MPEG2_ALL_MBS_NOT_DECODED,

    MPEG2_NO_REF_PFRAME,
    MPEG2_NO_REF_BFRAME

}Impeg2VDEC_ErrorStatus;

/*
 *  ======== IMPEG2VDEC_PARAMS ========
 *  Default parameter values for MPEG2VDEC instance objects
 */
extern const IMPEG2VDEC_Params    MPEG2VDEC_TI_Static_Params;
/*
 *  ======== IMPEG2VDEC_IVDEC_DYNAMICPARAMS ========
 *  Default dynamic parameter values for MPEG2VDEC instance objects
 */
extern const IMPEG2VDEC_DynamicParams    MPEG2VDEC_TI_DynamicParams;
/* ------------------------ function prototypes ----------------------------- */
/*******************************************************************************
*   PRIVATE DECLARATIONS Defined here, used only here
*******************************************************************************/
/* ------------------------ data declarations ------------------------------- */
/* ----------------------- function prototypes ------------------------------ */

/* ------------------------------ macros ------------------------------------ */
/*----------------------------------------------------------------------------*/
/* Extending control method commands                                          */
/*----------------------------------------------------------------------------*/
#define IMPEG2VDEC_GETSTATUS             XDM_GETSTATUS
#define IMPEG2VDEC_SETPARAMS             XDM_SETPARAMS
#define IMPEG2VDEC_RESET                 XDM_RESET
#define IMPEG2VDEC_FLUSH                 XDM_FLUSH
#define IMPEG2VDEC_SETDEFAULT            XDM_SETDEFAULT
#define IMPEG2VDEC_GETBUFINFO            XDM_GETBUFINFO
#define IMPEG2VDEC_GETVERSION            XDM_GETVERSION
#define IMPEG2VDEC_GETCONTEXTINFO        XDM_GETCONTEXTINFO
#define IMPEG2VDEC_GETDYNPARAMSDEFAULT   XDM_GETDYNPARAMSDEFAULT

#endif  /* __IMPEG2VDEC__ */

