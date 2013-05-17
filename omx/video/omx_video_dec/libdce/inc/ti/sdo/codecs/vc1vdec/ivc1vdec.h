/*
*******************************************************************************
* HDVICP2.0 Based VC-1 Decoder
*
* "HDVICP2.0 Based VC-1 Decoder" is a software module developed on TI's
* HDVICP2 based SOCs. This module is capable of decoding a compressed
* advanced/main/simple profile VC-1 bit-stream into a YUV 4:2:0 Raw video.
* Based on SMPTE-421M standard.
* Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
*******************************************************************************
*/
/**
*******************************************************************************
* @file  ivc1vdec.h
*
* @brief   This file contains the various defines for the implementation of
*          ividec3 interface for VC-1 decoder.
*
* @author: Krishnakanth Rapaka
*
* @version 0.0 (Feb 2008) : Base version created        [Krishnakanth Rapaka]
*
* @version 0.1 (May 2010) : Modified according to latest interface standard.
*                                                       [Dinesh Anand]
*
* @version 0.2 (Jun 2010) : Added review comments.      [Dinesh Anand]
*
* @version 0.6 (Dec 2011)  : Default structures are exposed in interface file.
*                            Added error codes for Ref picture Buffer.  [Naidu].
*
*******************************************************************************
*/
#ifndef IVC1VDEC_H
#define IVC1VDEC_H



/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <ti/xdais/xdas.h>
#include <ti/xdais/dm/ividdec3.h>
//#include <ti/sdo/fc/ires/hdvicp/ires_hdvicp2.h>

/*******************************************************************************
*                               MACROS
*******************************************************************************/

/*
 *  Some of the operations performed by the control call
 */
#define IVC1VDEC_GETSTATUS      XDM_GETSTATUS
#define IVC1VDEC_SETPARAMS      XDM_SETPARAMS
#define IVC1VDEC_RESET          XDM_RESET
#define IVC1VDEC_FLUSH          XDM_FLUSH
#define IVC1VDEC_SETDEFAULT     XDM_SETDEFAULT
#define IVC1VDEC_GETBUFINFO     XDM_GETBUFINFO

/*
 *  Macro defining the minimum version length of VC-1 decoder
 */
#define IVC1DEC_VERSION_LENGTH  (200)


/*******************************************************************************
*              PUBLIC DECLARATIONS Note: Defined here, used elsewhere
*******************************************************************************/

typedef enum {
    /* Error bit for unsupported VIDDEC3PARAMS                                  */
    IVC1DEC_ERR_UNSUPPORTED_VIDDEC3PARAMS = 0,
    /* Error bit for unsupported VIDDEC3 Dynamic PARAMS                         */
    IVC1DEC_ERR_UNSUPPORTED_VIDDEC3DYNAMICPARAMS,
    /* Error bit for unsupported VC1 VIDDEC3 Dynamic PARAMS                     */
    IVC1DEC_ERR_UNSUPPORTED_VC1DECDYNAMICPARAMS,
    /* Error bit for improper data sync setting                                 */
    IVC1DEC_ERR_IMPROPER_DATASYNC_SETTING,
    /* Error bit for no slice                                                   */
    IVC1DEC_ERR_NOSLICE,
    /* Error bit for slice header corruption                                    */
    IVC1DEC_ERR_SLICEHDR,
    /* Error bit for MB data corruption                                         */
    IVC1DEC_ERR_MBDATA,
    /* Error bit for unsupported VC1 feature                                    */
    IVC1DEC_ERR_UNSUPPFEATURE,
    /* Error bit for end of steam                                               */
    IVC1DEC_ERR_STREAM_END = 16,
    /* Error bit for unsupported resolution                                     */
    IVC1DEC_ERR_UNSUPPRESOLUTION,
    /* Error bit for IVAHD standby                                              */
    IVC1DEC_ERR_STANDBY,
    /* Error bit for invalid mailbox message                                    */
    IVC1DEC_ERR_INVALID_MBOX_MESSAGE,
    /* Error bit for sequence header corruption                                 */
    IVC1DEC_ERR_SEQHDR,
    /* Error bit for entry point header corruption                              */
    IVC1DEC_ERR_ENTRYHDR,
    /* Error bit for picture header corruption                                  */
    IVC1DEC_ERR_PICHDR,
  /* Error bit for Ref picture Buffer                                         */
  IVC1DEC_ERR_REF_PICTURE_BUFFER,
    /* Error bit if there is no sequence header start code                      */
    IVC1DEC_ERR_NOSEQUENCEHEADER,
    /* Error bit for invalid values of input/output buffer descriptors          */
    IVC1DEC_ERR_BUFDESC = 30,
    /* Error bit for picture size change. It will be set for a multiresolution  */
    /* Stream                                                                   */
    IVC1DEC_ERR_PICSIZECHANGE = 31

}Ivc1VDEC_ExtendedCommonErrorCodes;

/**
********************************************************************************
 *  @enum  IVC1VDEC_ERROR_STATUS
 *
 *  @brief    This enum defines the 128 codec specific error status bits
 *            in VC-1 decoder.
 *
 *  @note     None:
 *
********************************************************************************
*/
typedef enum {
    /* Error bit for invalid dynamic params structure size                      */
    VC1_DYNAMIC_PARAMS_SIZE_ERROR = 0,
    /* Error bit for decode header only mode                                    */
    VC1_DECODE_HEADER_ERROR,
    /* Error bit for invalid display width                                      */
    VC1_DISPLAY_WIDTH_ERROR,
    /* Error bit for invalid frame skip mode                                    */
    VC1_FRAME_SKIP_MODE_ERROR,
    /*Error bit for new frame flag                                              */
    VC1_NEW_FRAME_FLAG_ERROR,
    /* Error bit for datasync mode                                              */
    VC1_PUT_DATA_FXN_ERROR,
    /* Error bit for datasync mode                                              */
    VC1_PUT_DATA_HANDLE_ERROR,
    /* Error bit for datasync mode                                              */
    VC1_GET_DATA_FXN_ERROR,
    /* Error bit for datasync mode                                              */
    VC1_GET_DATA_HANDLE_ERROR,
    /* Error bit for datasync mode                                              */
    VC1_PUT_BUFFER_FXN_ERROR,
    /* Error bit for datasync mode                                              */
    VC1_PUT_BUFFER_HANDLE_ERROR,
    /* Error bit for late acquire argument                                      */
    VC1_LATE_ACQUIRE_ARG_ERROR,
    /* Error bit for inargs pointers                                            */
    VC1_NULL_INARGS_POINTER_ERROR,
    /* Error bit for inargs size                                                */
    VC1_INARGS_SIZE_ERROR,
    /* Error bit for invalid input bytes                                        */
    VC1_INVALID_INPUT_BYTES_ERROR,
    /* Error bit for invalid input bytes in flush mode                          */
    VC1_INVALID_INPUT_BYTES_IN_FLUSH_MODE_ERROR,
    /* Error bit for invalid input ID                                           */
    VC1_INVALID_INPUT_ID_ERROR,
    /* Error bit for NULL instance handle                                       */
    VC1_NULL_INSTANCE_HANDLE_ERROR,
    /* Error bit if decoder is not initialized                                  */
    VC1_DECODER_NOT_INITIALIZED_ERROR,
    /* Error bit for invalid input buffer descriptor                            */
    VC1_INVALID_INPUT_BUF_DESC_ERROR,
    /* Error bit for invalid input buffer pointer                               */
    VC1_INVALID_INPUT_BUFFER_POINTER_ERROR,
    /* Error bit for invalid input buffer size                                  */
    VC1_INVALID_INPUT_BUFFER_SIZE_ERROR,
    /* Error bit for invalid number of input buffer                             */
    VC1_INVALID_NUM_OF_INPUT_BUFFERS_ERROR,
    /* Error bit for excess number of input buffers                             */
    VC1_EXCESS_NUM_OF_INPUT_BUFFERS_ERROR,
    /* Error bit for invalid memory type of input buffer                        */
    VC1_INVALID_INPUT_BUFFER_MEMTYPE_ERROR,
    /* Error bit for invalid outargs pointer                                    */
    VC1_INVALID_OUTARGS_POINTER_ERROR,
    /* Error bit for invalid outargs size                                       */
    VC1_INVALID_OUTARGS_SIZE,
    /* Error bit for invalid output buffer descriptor pointer                   */
    VC1_INVALID_OUTPUT_BUF_DESC_POINTER_ERROR,
    /* Error bit for invalid output buffer descriptor                           */
    VC1_INVALID_OUTPUT_BUF_DESC_ERROR,
    /* Error bit for invalid output buffer                                      */
    VC1_INVALID_NUM_OF_OUTPUT_BUFFERS_ERROR,
    /* Error bit for invalid luma output buffer pointer                         */
    VC1_INVALID_OUTPUT_BUFFER0_POINTER_ERROR,
    /* Error bit for invalid luma output buffer size                            */
    VC1_INVALID_OUTPUT_BUFFER0_SIZE_ERROR,
    /* Error bit for invalid luma output buffer memory type                     */
    VC1_INVALID_OUTPUT_BUFFER0_MEMTYPE_ERROR,
    /* Error bit for invalid chroma output buffer pointer                       */
    VC1_INVALID_OUTPUT_BUFFER1_POINTER_ERROR,
    /* Error bit for invalid chroma output buffer size                          */
    VC1_INVALID_OUTPUT_BUFFER1_SIZE_ERROR,
    /* Error bit for invalid chroma output buffer memory type                   */
    VC1_INVALID_OUTPUT_BUFFER1_MEMTYPE_ERROR,
    /* Error bit for invalid output buffer2 pointer                             */
    VC1_INVALID_OUTPUT_BUFFER2_POINTER_ERROR,
    /* Error bit for invalid output buffer2 size                                */
    VC1_INVALID_OUTPUT_BUFFER2_SIZE_ERROR,
    /* Error bit for invalid output buffer2 memory type                         */
    VC1_INVALID_OUTPUT_BUFFER2_MEMTYPE_ERROR,
    /* Error bit for invalid buffre usage mode                                  */
    VC1_INVALID_BUFFER_USAGE_MODE,
    /* Error bit for invalid tiled width for output buffer0                     */
    VC1_INVALID_OUTPUT_BUFFER0_TILED_WIDTH_ERROR,
    /* Error bit for invalid tiled height for output buffer0                    */
    VC1_INVALID_OUTPUT_BUFFER0_TILED_HEIGHT_ERROR,
    /* Error bit for invalid tiled width for output buffer1                     */
    VC1_INVALID_OUTPUT_BUFFER1_TILED_WIDTH_ERROR,
    /* Error bit for invalid tiled height for output buffer1                    */
    VC1_INVALID_OUTPUT_BUFFER1_TILED_HEIGHT_ERROR,
    /* Error bit for invalid tiled width for output buffer2                     */
    VC1_INVALID_OUTPUT_BUFFER2_TILED_WIDTH_ERROR,
    /* Error bit for invalid tiled height for output buffer2                    */
    VC1_INVALID_OUTPUT_BUFFER2_TILED_HEIGHT_ERROR,
  /* Error bit for invalid ref picture buffer                                 */
  VC1_INVALID_REF_PICTURE_BUFFER,
    /* Error bit for invalid profile                                            */
    VC1_SEQ_HDR_INVALID_PROFILE = 64,
    /* Error bit for invalid invalid level bits                                 */
    VC1_SEQ_HDR_INVALID_LEVEL,
    /* Error bit for invalid color diff format                                  */
    VC1_SEQ_HDR_INVALID_COLORDIFF_FORMAT,
    /* Error bit for invalid max coded width                                    */
    VC1_SEQ_HDR_INVALID_MAX_CODED_WIDTH,
    /* Error bit for invalid max code height                                    */
    VC1_SEQ_HDR_INVALID_MAX_CODED_HEIGHT,
    /* Error bit for invalid reserved bits                                      */
    VC1_SEQ_HDR_INVALID_RESERVED,
    /* Error bit for invalid aspect ratio                                       */
    VC1_SEQ_HDR_INVALID_ASPECT_RATIO,
    /* Error bit for invalid frame rate numerator bits                          */
    VC1_SEQ_HDR_INVALID_FRAMERATENR,
    /* Error bit for invalid frame rate denominator bits                        */
    VC1_SEQ_HDR_INVALID_FRAMERATEDR,
    /* Error bit for invalid color prim bits                                    */
    VC1_SEQ_HDR_INVALID_COLOR_PRIM,
    /* Error bit for invalid transfer character bits                            */
    VC1_SEQ_HDR_INVALID_TRANSFER_CHAR,
    /* Error bit for invalid matrix coefficient bits                            */
    VC1_SEQ_HDR_INVALID_MATRIX_COEF,
    /* Error bit for invalid loop filter bits                                   */
    VC1_SEQ_HDR_INVALID_LOOPFILTER,
    /* Error bit for invalid FASTUVMC bits                                      */
    VC1_SEQ_HDR_INVALID_FASTUVMC,
    /* Error bit for invalid Extended MV bits                                   */
    VC1_SEQ_HDR_INVALID_EXTENDED_MV,
    /* Error bit for invalid DQUANT bits                                        */
    VC1_SEQ_HDR_INVALID_DQUANT,
    /* Error bit for invalid sync marker bits                                   */
    VC1_SEQ_HDR_INVALID_SYNCMARKER,
    /* Error bit for invalid rang reduction bits                                */
    VC1_SEQ_HDR_INVALID_RANGERED,
    /* Error bit for invalid max number of B frame bits                         */
    VC1_SEQ_HDR_INVALID_MAXBFRAMES,
    /* Error bit for invalid DQUANT in entry point header                       */
    VC1_ENTRY_PNT_HDR_INVALID_DQUANT,
    /* Error bit for invalid coded width                                        */
    VC1_ENTRY_PNT_HDR_INVALID_CODED_WIDTH,
    /* Error bit for invalid coded height                                       */
    VC1_ENTRY_PNT_HDR_INVALID_CODED_HEIGHT,
    /* Error bit for invalid PTYPE                                              */
    VC1_PIC_HDR_INVALID_PTYPE,
    /* Error bit for invalid PQINDEX                                            */
    VC1_PIC_HDR_INVALID_PQINDEX,
    /* Error bit for invalid MVRANGE                                            */
    VC1_PIC_HDR_INVALID_MVRANGE,
    /* Error bit for invalid RESPIC                                             */
    VC1_PIC_HDR_INVALID_RESPIC,
    /* Error bit for invalid FCM bits                                           */
    VC1_PIC_HDR_INVALID_FCM,
    /* Error bit for invalid RNDCTRL bits                                       */
    VC1_PIC_HDR_INVALID_RNDCTRL,
    /* Error bit for invalid MVMODE bits                                        */
    VC1_PIC_HDR_INVALID_MVMODE,
    /* Error bit for invalid DMVRANGE bits                                      */
    VC1_PIC_HDR_INVALID_DMVRANGE,
    /* Error bit for invalid BFRACTION bits                                     */
    VC1_PIC_HDR_INVALID_BFRACTION,
    /* Error bit for invalid REFDIST bits                                       */
    VC1_PIC_HDR_INVALID_REFDIST,
    /* Error bit for invalid number of MBs in a picture                         */
    VC1_ERR_MBNUMB,
    /* Error bit for invalid SCALERES bits                                      */
    VC1_ERR_SCALERES,
    /* Error bit for invalid ALTPQUANT bits                                     */
    VC1_ERR_ALTPQUANT,
    /* Error bit for invalid ABSPQ bits                                         */
    VC1_VOPDQUANT_INVALID_ABSPQ,
    /* Error bit for invalid slice address bits                                 */
    VC1_SLC_HDR_INVALID_SLICE_ADDR,
    /* Error bit for improper IVAHD reset                                       */
    VC1_IMPROPER_RESET,
    /* Error bit for improper standby                                           */
    VC1_IMPROPER_STANDBY,
    /* Error bit MB error                                                       */
    VC1_ECD_MB_ERROR,
    /* No Sequence header start code                                            */
    VC1_NO_SEQUENCE_STARTCODE

}IVC1VDEC_ERROR_STATUS;



/**

 ******************************************************************************

 *  @enum       IVC1VDEC_mbErrStatus

 *  @brief      This enum indicates if a MB was in error or not

 *

 ******************************************************************************

*/

typedef enum {
    /* MB was non-erroneous                                                     */
    IVC1VDEC_MB_NOERROR = 0,
    /* MB was erroneous                                                         */
    IVC1VDEC_MB_ERROR = 1

} IVC1VDEC_mbErrStatus;

/**
********************************************************************************
*  @struct  IVC1VDEC_TI_MbInfo
*
*  @brief   MB information structure that is written out by the IVA-HD hardware.
*
*  @note    None:
*
********************************************************************************
*/
typedef struct IVC1VDEC_TI_MbInfo {
    /* MB address                                                               */
    XDAS_UInt8 mb_addr;
    /* Error flag                                                               */
    XDAS_UInt8 error_flag;
    /* First MB flag                                                            */
    XDAS_UInt8 first_mb_flag;
    /* Picture bound                                                            */
    XDAS_UInt8 pic_bound_b;
    /* Upper picture bound                                                      */
    XDAS_UInt8 pic_bound_u;
    /* Right picture bound                                                      */
    XDAS_UInt8 pic_bound_r;
    /* Left picture bound                                                       */
    XDAS_UInt8 pic_bound_l;
    /* Availability of upper right MB                                           */
    XDAS_UInt8 mb_ur_avail;
    /* Availability of upper MB                                                 */
    XDAS_UInt8 mb_uu_avail;
    /* Availability of upper left MB                                            */
    XDAS_UInt8 mb_ul_avail;
    /* Availability of left MB                                                  */
    XDAS_UInt8 mb_ll_avail;
    /* Macroblock header format type                                            */
    XDAS_UInt8 fmt_type;
    /* Codec type                                                               */
    XDAS_UInt8 codec_type;
    /* Indicates DC values of each Y block in current MB                        */
    XDAS_UInt8 dc_coef_q_y[4];
    /* Indicates DC values of Cr block in current MB                            */
    XDAS_UInt8 dc_coef_q_cr;
    /* Indicates DC values of Cb block in current MB                            */
    XDAS_UInt8 dc_coef_q_cb;
    /* Block type of cr block                                                   */
    XDAS_UInt8 block_type_cr;
    /* Block type of cb block                                                   */
    XDAS_UInt8 block_type_cb;
    /* Block types of luma                                                      */
    XDAS_UInt8 block_type_y[4];
    /* In decoding, if the current macroblock is the last macroblock in a slice,*/
    /* ECD sets 1 to this field during executing the macroblock. Otherwise, ECD */
    /* sets 0 to this field                                                     */
    XDAS_UInt8 end_of_slice;
    /* 1 : allow skipping current MB if CBP = 0                                 */
    XDAS_UInt8 cond_skip_flag;
    /*  Skipped / non skipped MB                                                */
    XDAS_UInt8 skip;
    /* 1 indicates that overlap filtering is in use for the macroblock.         */
    XDAS_UInt8 overlap;
    /* 1 indicates that AC prediction is in use for the macroblock              */
    XDAS_UInt8 acpred;
    /* Denotes inter-prediction direction for the macroblock in B-picture       */
    XDAS_UInt8 b_picture_direction;
    /* Denotes the number of motion vectors.                                    */
    XDAS_UInt8 mv_mode;
    /* 1 indicates that the field transform is in use for the macroblock.       */
    XDAS_UInt8 fieldtx;
    /* 1 indicates that field inter-prediction is in use                        */
    XDAS_UInt8 mv_type;
    /* Equals the reference frame distance                                      */
    XDAS_UInt8 refdist;
    /* 1 indicates that macroblock quantizer-scale (MQUANT) overflows           */
    XDAS_UInt8 mquant_overflow;
    /* Equals the quantizer-scale for the macroblock                            */
    XDAS_UInt8 quant;
    /* 1 indicates that 0.5 shall be added to PQUANT in calculation of          */
    /* quantizer-scale. This field is valid for decoding only.                  */
    XDAS_UInt8 halfqp;
    /* Equals the DC coefficient step size which is derived from MQUANT in the  */
    /*   bit-stream                                                             */
    XDAS_UInt8 dc_step_size;
    /* Denotes the coded sub-block pattern for cr block                         */
    XDAS_UInt8 cbp_cr;
    /* Denotes the coded sub-block pattern for cb block                         */
    XDAS_UInt8 cbp_cb;
    /* Denotes the coded sub-block pattern for luma blocks                      */
    XDAS_UInt8 cbp_y[3];
    /* Denotes the backward reference field picture                             */
    XDAS_UInt8 mv_bw_ref_y[4];
    /* Denotes the forward reference field picture                              */
    XDAS_UInt8 mv_fw_ref_y[3];
    /* Unclipped forward motion vector for luma                                 */
    XDAS_UInt8 mv_fw_y[4][4];
    /* Unclipped backward motion vector for luma                                */
    XDAS_UInt8 mv_bw_y[1][1];
    /* Unclipped backward motion vector for chroma                              */
    XDAS_UInt8 mv_bw_c[2];
    /* Unclipped forward motion vector for chroma                               */
    XDAS_UInt8 mv_fw_c[2];
    /* Clipped forward motion vector for luma                                   */
    XDAS_UInt8 cmv_fw_y[4][4];
    /* Clipped backward motion vector for luma                                  */
    XDAS_UInt8 cmv_bw_y[4][4];
    /* Clipped forward motion vector for chroma                                 */
    XDAS_UInt8 cmv_fw_c[4][4];
    /* Clipped backward motion vector for chroma                                */
    XDAS_UInt8 cmv_bw_c[4][4];

}IVC1VDEC_TI_MbInfo;

/**
********************************************************************************
*  @struct  IVC1VDEC_Obj
*
*  @brief   Object defnition of the VC-1 decoder algorithm.This structure
*           must be the first field of all VC1VDEC instance objects.
*
*  @param   fxns:  Pointer to the structure defining all the ividdec3
*                    interface operations to be performed on the VC-1 decoder
*                    object.
********************************************************************************
*/
typedef struct IVC1VDEC_Obj {

    struct IVC1VDEC_Fxns *fxns;

} IVC1VDEC_Obj;

/**
********************************************************************************
*  @struct     IVC1VDEC_Handle
*
*  @brief       Handle to the VC-1 decoder instance object.
*
********************************************************************************
*/
typedef struct IVC1VDEC_Obj *IVC1VDEC_Handle;

/**
********************************************************************************
*  @struct  IVC1VDEC_Status
*
*  @brief   This structure defines parameters that describe the status of the
*           VC-1 Decoder and any other implementation specific parameters.
*           The status parameters are defined in the XDM data structure,
*           IVIDDEC3_Status
*
*  @param viddecStatus : XDM Base class status structure (see ividdec3.h)
*
*  @param eRes:          Extended class
********************************************************************************
*/
typedef struct IVC1VDEC_Status {
    IVIDDEC3_Status viddecStatus;
    /*-------------------------------------------------------------------------*/
    /* Extended Error Code0 returned by decoder                                */
    /*-------------------------------------------------------------------------*/
    XDAS_UInt32 extendedErrorCode0;
    /*-------------------------------------------------------------------------*/
    /* Extended Error Code1 returned by decoder                                */
    /*-------------------------------------------------------------------------*/
    XDAS_UInt32 extendedErrorCode1;
    /*-------------------------------------------------------------------------*/
    /* Extended Error Code2 returned by decoder                                */
    /*-------------------------------------------------------------------------*/
    XDAS_UInt32 extendedErrorCode2;
    /*-------------------------------------------------------------------------*/
    /* Extended Error Code3 returned by decoder                                */
    /*-------------------------------------------------------------------------*/
    XDAS_UInt32 extendedErrorCode3;
    /*-------------------------------------------------------------------------*/
    /* Debug trace level configured for the codec                              */
    /*-------------------------------------------------------------------------*/
    XDAS_UInt32 debugTraceLevel;
    /*-------------------------------------------------------------------------*/
    /* Number of frames for which history information is maintained by the     */
    /*  codec                                                                  */
    /*-------------------------------------------------------------------------*/
    XDAS_UInt32 lastNFramesToLog;
    /*-------------------------------------------------------------------------*/
    /* External memory address (as seen by M3) where debug trace information is*/
    /*  being dumped                                                           */
    /*-------------------------------------------------------------------------*/
    XDAS_UInt32 *extMemoryDebugTraceAddr;
    /*-------------------------------------------------------------------------*/
    /* External memory buffer size (in bytes) where debug trace information is */
    /*  being dumped                                                           */
    /*-------------------------------------------------------------------------*/
    XDAS_UInt32 extMemoryDebugTraceSize;
} IVC1VDEC_Status;


/**
********************************************************************************
*  @struct IVC1VDEC_Params
*
*  @brief  This structure defines the creation parameters for all VC-1 decoder
*          objects. This structure includes the xdm baseclass creation
*          parameters and any other implementation specific parameters for
*          VC-1 Decoder instance object.
*
*  @param viddec3Params:     XDM Baselass create time parameters.
*                              (see ividdec3.h)
*
********************************************************************************
*/
typedef struct IVC1VDEC_Params {
    IVIDDEC3_Params viddecParams;
    /*--------------------------------------------------------------------------*/
    /*Enable/Disable Error Concealment                                          */
    /* enumeration 'eFrameErrorConcealment' can be used to set this value       */
    /*--------------------------------------------------------------------------*/
    XDAS_Int32 errorConcealmentON;
    /*--------------------------------------------------------------------------*/
    /* Flag to indicate that whether the application is providing the frame     */
    /* layer data structure in case of simple & main profile                    */
    /*--------------------------------------------------------------------------*/
    XDAS_Int32 frameLayerDataPresentFlag;
    /*--------------------------------------------------------------------------*/
    /* This parameter configures the codec to dump a debug trace log            */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 debugTraceLevel;
    /*--------------------------------------------------------------------------*/
    /* This parameters configures the codec to maintain history of debug trace  */
    /* parameters for last N frames.                                            */
    /*--------------------------------------------------------------------------*/
    XDAS_UInt32 lastNFramesToLog;

} IVC1VDEC_Params;

/**
********************************************************************************
*  @struct IH264VDEC_DynamicParams
*
*  @brief  This structure defines the run-time parameters and any other
*          implementation specific parameters for an H.264 instance object.
*          The base run-time parameters are defined in the XDM data structure,
*          IVIDDEC3_DynamicParams.
*
*  @param  viddecDynamicParams : XDM Base class dynamic structure
*                                 (see ividdec3.h)
*
*  @param  outloopDeblocking :  Out loop deblocking flag to be enabled
*                                 for simple profile dynamically at frame level
*
*  @param  calc_sw_workaround:  Flag used for switching the CALC software
*                                 work around ON or OFF.
*
********************************************************************************
*/
typedef struct IVC1VDEC_DynamicParams {
    IVIDDEC3_DynamicParams viddecDynamicParams;
} IVC1VDEC_DynamicParams;


/**
********************************************************************************
*  @struct IVC1VDEC_InArgs
*
*  @brief  This structure defines the run-time input arguments for an VC-1
*          decoder process function.
*
*  @param viddec3InArgs : XDM Base class InArgs structure  (see ividdec3.h)
*
********************************************************************************
*/
typedef struct IVC1VDEC_InArgs {
    IVIDDEC3_InArgs viddecInArgs;
}IVC1VDEC_InArgs;


/**
********************************************************************************
*  @struct IVC1VDEC_OutArgs
*
*  @brief  This structure defines the run time output arguments for VC-1
*          decoder process function.
*
*  @param viddecOutArgs : XDM Base class OutArgs structure  (see ividdec3.h)
*
********************************************************************************
*/
typedef struct IVC1VDEC_OutArgs {
    IVIDDEC3_OutArgs viddecOutArgs;

} IVC1VDEC_OutArgs;

/**
********************************************************************************
*  @struct IVC1VDEC_Fxns
*
*  @brief  This structure contains pointers to all the XDAIS and XDM interface
*          functions
*
*  @param  ividdec :  This structure contains pointers to all the XDAIS and
*                     XDM interface functions
********************************************************************************
*/
typedef struct IVC1VDEC_Fxns {
    IVIDDEC3_Fxns ividdec;
} IVC1VDEC_Fxns;



/**
********************************************************************************
*  @fn     VC1VDEC_TI_decode(IVIDDEC3_Handle   handle,
*                             XDM2_BufDesc      *inptr,
*                             XDM2_BufDesc      *outptr,
*                             IVIDDEC3_InArgs   *inargs,
*                             IVIDDEC3_OutArgs  *outargs)
*
*  @brief  TI's(Texas Instrument) implementation of the process API defined
*          by XDM for the VC-1 Decoder.This process function is responsible
*          for the decode of a given frame.
*
* @param [in, out]handle :     Algorithm handle
*
* @param [in]     inptr :     Pointer to input buffer structure.
*
* @param [out]    outptr:     Pointer to output buffer structure.
*
* @param [in]     inargs :     Pointer to input arguments structure.
*
* @param [out]    outargs :    Pointer to output arguments structure.
*
* @return[out]    Error Code : IVIDDEC3_EFAIL if any error happened else
*                              IVIDDEC3_EOK for sucessful decoding of frame.
********************************************************************************
*/
XDAS_Int32 VC1VDEC_TI_decode(IVIDDEC3_Handle handle,
                             XDM2_BufDesc      *inptr,
                             XDM2_BufDesc      *outptr,
                             IVIDDEC3_InArgs   *inargs,
                             IVIDDEC3_OutArgs  *outargs);


/**
*******************************************************************************
*  @fn     VC1VDEC_TI_control (IVIDDEC3_Handle          handle,
*                              IVIDDEC3_Cmd             cmd,
*                              IVIDDEC3_DynamicParams   * params,
*                             IVIDDEC3_Status          * status)
*
*  @brief  Control API for the VC-1 decoder, to control various object
*          parameters.Some of the operations which can be performed by control
*          call are,
*          i)Get status of various elements defined in the IVC1dec_Status.
*         ii)Set the parameters in the structure IVC1dec_DyanmicParams
*        iii)Reset the algorithm.
*         iV)Set predefined defaults.
*          V)Flush buffers held by the system.
*
*  @param[in]   handle:    Pointer to algorithm instance object.
*
*  @param[in]   cmd:       Command specifying the operations to be performed.
*
*  @param[in]   params:    Pointer to the IVIDDEC3_DynamicParams struct.
*
*  @param[in]   status:    Pointer to the IVIDDEC1_Status struct.
*
*  @return                 Pass or Fail (IALG_EOK / IALG_EFAIL)
*
*******************************************************************************
*/
XDAS_Int32 VC1VDEC_TI_control (IVIDDEC3_Handle handle,
                               IVIDDEC3_Cmd cmd,
                               IVIDDEC3_DynamicParams   *params,
                               IVIDDEC3_Status          *status);


/**
******************************************************************************
 *  ======== IVC1VDEC_Params ========
 *  Default Create parameter values for VC1VDEC instance objects
********************************************************************************
*/
extern const IVC1VDEC_Params     VC1VDEC_TI_PARAMS;
/**
******************************************************************************
 *  ======== IVC1VDEC_DynamicParams ========
 *  Default DynamicParams values for VC1VDEC instance objects
********************************************************************************
*/
extern const IVC1VDEC_DynamicParams VC1VDEC_TI_DYNAMICPARAMS;
/*******************************************************************************
*            EXTERNAL REFERENCE Note: use only if not found in header file
*******************************************************************************/


extern IVC1VDEC_Fxns    VC1VDEC_TI_IVC1VDEC;

extern IVIDDEC3_Fxns    VC1VDEC_TI_IVIDDECFUNCTIONS;



#endif  /* IVC1VDEC_ */

