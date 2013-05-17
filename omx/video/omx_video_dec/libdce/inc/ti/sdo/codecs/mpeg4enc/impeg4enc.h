/* ======================================================================
 *   Copyright (C) 2010 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission
 *   of Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been
 *   supplied.
 * ====================================================================*/
/*
********************************************************************************
 * HDVICP2.0 Based MPEG4 SP Encoder
 *
 * "HDVICP2.0 Based MPEG4 SP Encoder" is software module developed on TI's
 *  HDVICP2 based SOCs. This module is capable of compressing a 4:2:0 Raw
 *  video into a simple profile bit-stream. Based on ISO/IEC 14496-2."
 *
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
********************************************************************************
*/
/**
********************************************************************************
 * @file <impeg4enc.h>
 *
 * @brief Interface header file for MPEG4 SP Encoder
 *
 * @author: Venugopala Krishna
 *
 * @version 0.0 (Feb 2009) : Initial version.
 *                           [Venugopala Krishna]
 * @version 0.1 (Apr 2009) : Updated version.
 *                           [Radhesh Bhat]
 *
 *******************************************************************************
*/

/* -------------------- compilation control switches -------------------------*/
#ifndef IMPEG4ENC_
#define IMPEG4ENC_

/**
 *  @defgroup   HDVICP2MPEG4 IMPEG4ENC_TI (V7M)
 *  @ingroup    m3
 *
 *              The IMPEG4ENC_TI interface enables encoding in MPEG-4 format
 *
 */

/** @ingroup    HDVICP2MPEG4 */
/*@{*/

/*-------------------------------------------*/
/* typecasting of control method commands    */
/*-------------------------------------------*/
#define IMPEG4ENC_GETSTATUS      XDM_GETSTATUS
#define IMPEG4ENC_SETPARAMS      XDM_SETPARAMS
#define IMPEG4ENC_RESET          XDM_RESET
#define IMPEG4ENC_FLUSH          XDM_FLUSH
#define IMPEG4ENC_SETDEFAULT     XDM_SETDEFAULT
#define IMPEG4ENC_GETBUFINFO     XDM_GETBUFINFO

/**
* MPEG4 Simple profile IDC
*/
#define MPEG4_SIMPLE_PROFILE_IDC               3

/*******************************************************************************
*                             INCLUDE FILES
*******************************************************************************/
/* -------------------- system and platform files ----------------------------*/

/*--------------------- program files ----------------------------------------*/
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc2.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*  PUBLIC DECLARATIONS Defined here, used elsewhere
*******************************************************************************/
/*---------------------- data declarations -----------------------------------*/

/*******************************************************************************
*  PRIVATE DECLARATIONS Defined here, used only here
*******************************************************************************/
/*---------------------- data declarations -----------------------------------*/
/*---------------------- function prototypes ---------------------------------*/


/**
 *******************************************************************************
 *  @enum       IMPEG4ENC_ErrorBit
 *  @brief      error informations of IVAHD MPEG4 encoder implementation by TI.
 *
 *  @remarks    When an internal error occurs, the algorithm will return
 *              an error return value (e.g. EFAIL, EUNSUPPORTED)
 *
 *  @remarks    The value of each enum is the bit which is set.
 *
 *  @remarks    Bits 8-15 are defined by XDM and hence not used by codec
 *              implementation. rest all bits are used.
 *              XDM defined error bits are also active.
 *
 *  @remarks    The algorithm can set multiple bits to 1 based on conditions.
 *              e.g. it will set bits #XDM_FATALERROR (fatal) and
 *              #XDM_UNSUPPORTEDPARAM (unsupported params) in case
 *              of unsupported run time parameters.
 *
 *******************************************************************************
*/
typedef enum {
    IMPEG4ENC_LEVEL_INCOMPLAINT_PARAMETER = 0,
    /**< Bit 0 - level incomplaint parameters.
    *   @remarks  This error is applicable when some parameters are set
    *             which are not meeting the limits set by MPEG4 standard
    */

    IMPEG4ENC_PROFILE_INCOMPLAINT_CONTENTTYPE = 1,
    /**< Bit 1 - Profile incomplaint content type.
    *   @remarks  This error is applicable when
    *             IVIDENC2_Params::inputContentType is not set as
    *             IVIDEO_PROGRESSIVE but IVIDENC2_Params::profile is set
    *             as IMPEG4_SIPPLE_PROFILE
    */

    IMPEG4ENC_IMPROPER_HDVICP2_STATE = 16,
    /**< Bit 16 - Device is not proper state to use.
    */

    IMPEG4ENC_WARNING_H263_PLUS_CUSTOM_SOURCE_FORMAT = 17,
    /**< Bit 17 - Indication that the input resolution given to codec
    *             is custom source format supported in H.263+ not the
    *             standard resolutions supported with H263 baseline or
    *             MPEG4 with short video header.
    */

    IMPEG4ENC_ERROR_BITSTREAM_MEMORY_INSUFFICIENT = 18,
    /**< Bit 18 - Indication that the buffer given to codec from
     *            getBuffer function is insufficient so that codec
     *            cannot continue encoding. It means that if return value
     *            from getBuffer function is -1, then this bit gets set
     *            by the codec. This is the situation where application
     *            might not be able to provide memory to codec.
    */

    IMPEG4ENC_IMPROPER_DATASYNC_SETTING = 19,
    /**< Bit 19 - data synch settings are not proper
    *   @remarks  This error is set when encoder is asked to operate
    *             at sub frame level but the call back function pointer
    *             is  NULL
    */

    IMPEG4ENC_UNSUPPORTED_VIDENC2PARAMS = 20,
    /**< Bit 20 - Invalid videnc2 parameters
    *   @remarks  This error is set when any parameter of struct
    *             IVIDENC2_Params is not in allowed range
    */

    IMPEG4ENC_UNSUPPORTED_RATECONTROLPARAMS = 21,
    /**< Bit 21 - Invalid rate control parameters
    *   @remarks  This error is set when any parameter of struct
    *             IMPEG4ENC_RateControlParams is not in allowed range
    */

    IMPEG4ENC_UNSUPPORTED_INTERCODINGPARAMS = 22,
    /**< Bit 22 - Invalid inter coding parameters
    *   @remarks  This error is set when any parameter of struct
    *             IMPEG4ENC_InterCodingParams is not in allowed range
    */

    IMPEG4ENC_UNSUPPORTED_INTRACODINGPARAMS = 23,
    /**< Bit 23 - Invalid Intra coding parameters
    *   @remarks  This error is set when any parameter of struct
    *             IMPEG4ENC_IntraCodingParams is not in allowed range
    */

    IMPEG4ENC_UNSUPPORTED_SLICECODINGPARAMS = 25,
    /**< Bit 25 - Invalid slice coding parameters
    *   @remarks  This error is set when any parameter of struct
    *             IMPEG4ENC_SliceControlParams is not in allowed range
    */

    IMPEG4ENC_UNSUPPORTED_MPEG4ENCPARAMS = 29,
    /**< Bit 29 - Invalid Create time extended parameters
    *   @remarks  This error is set when any parameter of struct
    *             IMPEG4ENC_CreateParams is not in allowed range
    */

    IMPEG4ENC_UNSUPPORTED_VIDENC2DYNAMICPARAMS = 30,
    /**< Bit 30 - Invalid base class dyanmic parameters during control
    *   @remarks  This error is set when any parameter of struct
    *             IVIDENC2_DynamicParams is not in allowed range
    */

    IMPEG4ENC_UNSUPPORTED_MPEG4ENCDYNAMICPARAMS = 31
                                                  /**< Bit 31 -Invalid extended class dyanmic parameters during control
                                                  *   @remarks  This error is set when any parameter of struct
                                                  *             IMPEG4ENC_DynamicParams (excluding embedded structures)
                                                  *             is not in allowed range
                                                  */

} IMPEG4ENC_ErrorBit;


/**
 *******************************************************************************
 *  @enum       IMPEG4ENC_Level
 *  @brief      Enum for MPEG-4 Simple profile levels
 *
 *  @remarks    allowed levels are 0, 0b, 1, 2, 3, 4a & 5
 *
 *******************************************************************************
*/
typedef enum {
    IMPEG4ENC_SP_LEVEL_0   = 0,  /**< MPEG4 Simple Profile Level 0 */

    IMPEG4ENC_SP_LEVEL_0B  = 9,  /**< MPEG4 Simple Profile Level 0b*/

    IMPEG4ENC_SP_LEVEL_1   = 1,  /**< MPEG4 Simple Profile Level 1 */

    IMPEG4ENC_SP_LEVEL_2   = 2,  /**< MPEG4 Simple Profile Level 2 */

    IMPEG4ENC_SP_LEVEL_3   = 3,  /**< MPEG4 Simple Profile Level 3 */

    IMPEG4ENC_SP_LEVEL_4A  = 4,  /**< MPEG4 Simple Profile Level 4a*/

    IMPEG4ENC_SP_LEVEL_5   = 5,  /**< MPEG4 Simple Profile Level 5 */

    IMPEG4ENC_SP_LEVEL_6   = 6   /**< MPEG4 Simple Profile Level 6 */

} IMPEG4ENC_Level;


/**
 *******************************************************************************
 *  @enum       IMPEG4ENC_H263Level
 *  @brief      Enum for H.263 base profile levels
 *
 *  @remarks    allowed levels are 10,20,30,40, 45, 50, 60 and 70
 *
 *******************************************************************************
*/
typedef enum {
    IMPEG4ENC_H263_LEVEL_10 = 10,   /**< H263 Baseline Profile Level 10 */

    IMPEG4ENC_H263_LEVEL_20 = 20,   /**< H263 Baseline Profile Level 20 */

    IMPEG4ENC_H263_LEVEL_30 = 30,   /**< H263 Baseline Profile Level 30 */

    IMPEG4ENC_H263_LEVEL_40 = 40,   /**< H263 Baseline Profile Level 40 */

    IMPEG4ENC_H263_LEVEL_45 = 45,   /**< H263 Baseline Profile Level 45 */

    IMPEG4ENC_H263_LEVEL_50 = 50,   /**< H263 Baseline Profile Level 50 */

    IMPEG4ENC_H263_LEVEL_60 = 60,   /**< H263 Baseline Profile Level 60 */

    IMPEG4ENC_H263_LEVEL_70 = 70    /**< H263 Baseline Profile Level 70 */
} IMPEG4ENC_H263Level;

/**
 *******************************************************************************
 *  @enum       IMPEG4ENC_PixelRange
 *  @brief      pixel/video range enum
 *
 *  @remarks
 *
 *******************************************************************************
*/

typedef enum {
    /**
     * Y varies from 16 to 235 and Cb/Cr varies from 16 to 240
     */
    IMPEG4ENC_PR_16_235  = 0,

    /**
     * Y/Cb/Cr varies from 0 to 255
     */
    IMPEG4ENC_PR_0_255   = 1,

    IMPEG4ENC_PR_DEFAULT = IMPEG4ENC_PR_0_255

} IMPEG4ENC_PixelRange;


/**
 *******************************************************************************
 *  @enum       IMPEG4ENC_SceneChangeAlgo
 *  @brief      Enum for enabling or disabling scene change detection algo
 *
 *  @remarks
 *
 *******************************************************************************
*/

typedef enum {
    /**
     * Disables the scene change detection algorithm
     */
    IMPEG4ENC_SCDA_DISABLE  = 0,

    /**
     * Enables the scene change detection algorithm
     */
    IMPEG4ENC_SCDA_ENABLE   = 1,

    IMPEG4ENC_SCDA_DEFAULT = IMPEG4ENC_SCDA_ENABLE

} IMPEG4ENC_SceneChangeAlgo;


/**
 *******************************************************************************
 *  @enum       IMPEG4ENC_RateControlParamsPreset
 *  @brief      These enumerations control the RateControl Params
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {
    IMPEG4_RATECONTROLPARAMS_DEFAULT     = 0, /**< Default RC params */

    IMPEG4_RATECONTROLPARAMS_USERDEFINED = 1, /**< User defined RC params*/

    /**
     * Keep the Rate Control params as existing.
     * This is useful because during control call if user don't want to chnage
     * the Rate Control Params
     */
    IMPEG4_RATECONTROLPARAMS_EXISTING    = 2,


    IMPEG4_RATECONTROLPARAMS_MAX

} IMPEG4ENC_RateControlParamsPreset;


/**
 *******************************************************************************
 *  @enum       IMPEG4ENC_RateControlAlgoPreset
 *  @brief      These enumerations control the RateControl Algorithm
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {
    /**
     * Fixed QP
     */
    IMPEG4_RATECONTROLALGO_NONE        = 0,

    /**
     * VBR Rate Control Algorithm
     */
    IMPEG4_RATECONTROLALGO_VBR         = 1,

    /**
     * CBR Rate Control Algorithm -- Low Delay
     */
    IMPEG4_RATECONTROLALGO_CBR         = 2,

    IMPEG4_RATECONTROLALGO_MAX

} IMPEG4ENC_RateControlAlgoPreset;


/**
 *******************************************************************************
 *  @enum   IMPEG4ENC_InterCodingPreset
 *  @brief  These enumerations control the type of inter coding
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {

    /**
     * Default Inter coding params
    */

    IMPEG4_INTERCODING_DEFAULT     = 0,

    /**
     * User defined inter coding params
    */
    IMPEG4_INTERCODING_USERDEFINED = 1,


    /**
     * Keep the inter coding params as existing
    */
    IMPEG4_INTERCODING_EXISTING    = 2,

    IMPEG4_INTERCODING_MAX

} IMPEG4ENC_InterCodingPreset;


/**
 *******************************************************************************
 *  @enum   IMPEG4ENC_InterBlockSize
 *  @brief  These enumerations control the block size of each MB in encoding
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {
    /**
     * 16x16 Block size
     */
    IMPEG4_BLOCKSIZE_16x16     = 0,

    /**
     * 8x8 Block size
     */
    IMPEG4_BLOCKSIZE_8x8       = 1,

    /**
      * Default block size
      */
    IMPEG4_BLOCKSIZE_DEFAULT   = IMPEG4_BLOCKSIZE_8x8,

    IMPEG4_BLOCKSIZE_MAX       = 2

} IMPEG4ENC_InterBlockSize;


/**
 *******************************************************************************
 *  @enum   IMPEG4ENC_IntraRefreshMethods
 *  @brief  Refresh method Type Identifier for MPEG4 Encoder
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {

    /**
     * Doesn't insert forcefully any intra macro blocks
    */
    IMPEG4_INTRAREFRESH_NONE       = 0,
    /**
      * Inserts intra macro blocks in a cyclic fashion
      * cyclic interval is equal to intraRefreshRate
     */
    IMPEG4_INTRAREFRESH_CYCLIC_MBS,

    /**
     * Inserts Intra Rows in a cyclic fashion
     * Number of Rows equal to intraRefreshRate
    */
    IMPEG4_INTRAREFRESH_CYCLIC_ROWS,

    /**
     *  Mandatory Intra Refresh -- evenly distributes number of INTRA MBs over
     *  frames.
    */
    IMPEG4_INTRAREFRESH_MANDATORY,

    /**
     * position of intra macro blocks is intelligently chosen by encoder, but the
     * number of forcely coded intra macro blocks in a frame is gaurnteed to be
     * equal to totalMbsInFrame/intraRefreshRate.
     * This method is not implemented currently.
     */
    IMPEG4_INTRAREFRESH_RDOPT_MBS

} IMPEG4ENC_IntraRefreshMethods;


/**
 *******************************************************************************
 *  @enum   IMPEG4ENC_IntraCodingPreset
 *  @brief  These enumerations control the type of intra coding
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {
    /**
     * Default intra coding params
     */
    IMPEG4_INTRACODING_DEFAULT     = 0,

    /**
     * User defined intra coding params
     */
    IMPEG4_INTRACODING_USERDEFINED = 1,
    IMPEG4_INTRACODING_MAX

} IMPEG4ENC_IntraCodingPreset;


/**
 *******************************************************************************
 *  @enum   IMPEG4ENC_SliceCodingPreset
 *  @brief  These enumerations control the type of slice coding
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {
    /**
     * Default slice coding params
     */
    IMPEG4_SLICECODING_DEFAULT     = 0,

    /**
     * User defined slicecoding params
     */
    IMPEG4_SLICECODING_USERDEFINED = 1,

    /**
     * Keep the slice coding params as existing
     * This is useful because during control call if user don't want to chnage
     * the sliceCodingParams
     */
    IMPEG4_SLICECODING_EXISTING    = 2,
    IMPEG4_SLICECODING_MAX

} IMPEG4ENC_SliceCodingPreset;


/**
 *******************************************************************************
 *  @enum   IMPEG4ENC_SliceMode
 *  @brief  These enumerations control the mode of slice coding
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {
    IMPEG4_SLICEMODE_NONE    = 0,

    /**
     * Default slice coding mode is MB based
     */
    IMPEG4_SLICEMODE_DEFAULT = IMPEG4_SLICEMODE_NONE,

    /**
     * Slices are controlled based upon number of Macroblocks
     */
    IMPEG4_SLICEMODE_MBUNIT  = 1,

    /**
     * Slices are controlled based upon number of bits consumed
     */
    IMPEG4_SLICEMODE_BITS   = 2,
    IMPEG4_SLICEMODE_MAX

} IMPEG4ENC_SliceMode;

/**
 *******************************************************************************
 *  @enum   IMPEG4ENC_nonMultiple16RefPadMethod
 *  @brief  These enumerations captures different methods of padding the Ref
 *          frame when dimension is non multiple of 16.
 *
 *  @remarks
 *
 *******************************************************************************
*/
typedef enum {
    /**
     * Method as suggested by DivX spec.
     */
    IMPEG4_PAD_METHOD_DIVX     = 0,

    /**
     * Method as suggested by MPEG4 spec.
     */
    IMPEG4_PAD_METHOD_MPEG4    = 1,

    /**
     * Default mode is MPEG4 suggested way.
    */
    IMPEG4_PAD_METHOD_DEFAULT = IMPEG4_PAD_METHOD_MPEG4,
    IMPEG4_PAD_METHOD_MAX

} IMPEG4ENC_nonMultiple16RefPadMethod;


/**

  @enum   IMPEG4ENC_AspectRatioIdc
  @brief  Defines aspect ratio IDs

*/
typedef enum {
    IMPEG4ENC_ASPECTRATIO_SQUARE = 1,   /**< 1:1 (square) aspect ratio */
    IMPEG4ENC_ASPECTRATIO_12_11,        /**<  12:11  aspect ratio      */
    IMPEG4ENC_ASPECTRATIO_10_11,        /**<  10:11  aspect ratio      */
    IMPEG4ENC_ASPECTRATIO_16_11,        /**<  16:11  aspect ratio      */
    IMPEG4ENC_ASPECTRATIO_40_33,        /**<  40:33  aspect ratio      */
    IMPEG4ENC_ASPECTRATIO_EXTENDED = 15 /**<  Extended aspect ratio    */

} IMPEG4ENC_AspectRatioIdc;

/**
 *******************************************************************************
 *  @struct  IMPEG4ENC_Cmd
 *  @brief   This structure defines the control commands for the IMP4VENC module
 *
 *******************************************************************************
*/
typedef IVIDENC2_Cmd IMPEG4ENC_Cmd;


/**
 *******************************************************************************
 *  @struct  IMPEG4ENC_Obj
 *  @brief   This structure must be the first field of all MPEG4ENC instance
 *           objects
 *
 *  @param   fxns - is a pointer to IMPEG4ENC_Fxns structure which includes the
 *           below function pointers
 *           IALG_Fxns
 *             Void algActivate(IALG_Handle handle)
 *             Int  algAlloc(const IALG_Params *params,
 *               struct IALG_Fxns **parentFxns, IALG_MemRec *memTab)
 *             Void algControl(IALG_Handle handle, IALG_Cmd cmd,
 *               IALG_Status *status)
 *             Void algDeactivate(IALG_Handle handle)
 *             Int algFree(IALG_Handle handle, IALG_MemRec *memTab)
 *             Int algInit(IALG_Handle handle, const IALG_MemRec *memTab,
 *               IALG_Handle parent, const IALG_Params *params)
 *             Void algMoved(IALG_Handle handle, const IALG_MemRec *memTab,
 *               IALG_Handle parent, const IALG_Params *params)
 *             Int algNumAlloc(Void)
 *
 *           XDAS_Int32 process(IVIDENC2_Handle handle, IVIDEO2_BufDesc *inBufs,
 *             XDM2_BufDesc *outBufs, IVIDENC2_InArgs *inArgs,
 *             IVIDENC2_OutArgs *outArgs)
 *
 *           XDAS_Int32 control(IVIDENC2_Handle handle, IVIDENC2_Cmd id,
 *             IVIDENC2_DynamicParams *params, IVIDENC2_Status *status)
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_Obj {
    struct IMPEG4ENC_Fxns *fxns;
} IMPEG4ENC_Obj;


/**
 *******************************************************************************
 *  @struct  IMPEG4ENC_Handle
 *  @brief   This handle is used to reference all MPEG4ENC instance objects
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_Obj *IMPEG4ENC_Handle;

/**
 *******************************************************************************
 *  @struct IMPEG4ENC_RateControlParams
 *  @brief  This structure contains all the parameters which controls Rate
 *          Control behavior
 *
 *  @param  rateControlParamsPreset  :
 *          if this is IMPEG4_RATECONTROLPARAMS_DEFAULT the algorithm loads the
 *          default rate control parameters.
 *          if this is IMPEG4_RATECONTROLPARAMS_USERDEFINED the algorithm loads
 *          the user defined rate control parameters. if user is not aware of
 *          the floowing parameters it should be set to this enumeration.
 *          if this is IMPEG4_RATECONTROLPARAMS_EXISTING the algorithm loads
 *          the default rate control parameters.
 *
 *  @param  rcAlgo  :
 *          if this is IMPEG4_RATECONTROLALGO_NONE the algorithm uses the fixed
 *          qpI/qpP depending on the Frame
 *          if this is IMPEG4_RATECONTROLALGO_PLR the algorithm uses the
 *          PLR algorithm for QP Selection and bitrate achievement
 *
 *  @param  qpI  :
 *          Initial Quantization Parameter for I frames. Valid Range is [1, 31].
 *          when rateControlPreset = IVIDEO_NONE, this quantization parameter is
 *          used by the I frame encoding
 *
 *  @param  qpP  :
 *          Initial Quantization Parameter for P frames. Valid Range is [1, 31]
 *          when rateControlPreset = IVIDEO_NONE, this quantization parameter is
 *          used by the P frame encoding
 *
 *  @param  seIntialQP  :
 *          when rcAlgo is anything other than IMPEG4_RATECONTROLALGO_NONE,
 *          frame encoding start with seIntialQP value.
 *          When the user does not have understanding of what to set, set to 0,
 *          so the Codec internally decides intelligently the initial QP to be
 *          used.
 *
 *  @param  qpMax  :
 *          Maximum Quantization Parameter. Range [1, 31]. Useful to control
 *          a minimum quality level
 *
 *  @param  qpMin  :
 *          Minimum Quantization Parameter. Range [1, 31]. Useful to control
 *          a maximum bit-rate level
 *
 *  @param  enablePerceptualQuantMode  :
 *          perceptual quantization is enabled or diasabled. It varies the Qp at
 *          MB level instead of row level to improve the perceptual quality of
 *          video.
 *          1 for enable
 *          0 for disable
 *
 *  @param  allowFrameSkip  :
 *          This enables the Frame Skip Feature looking at the VBV Fullness.
 *          This should be enabled when Low Delay Mode is enabled.
 *          2 for enabling early frame skip
 *          1 for enabling late frame skip
 *          0 for disabling frame skip
 *  @param  initialBufferLevel :
 *          Initial buffer level for VBV compliance. It informs that
 *          hypothetical decoder can start depending on the fullness of the
 *          VBV buffer. Default value is 0, where codec will internally
 *          calculate the value based on the RC algo type
 *  @param  vbvBufferSize :
 *          Virtual Buffer Verifier buffer size. This size controls the frame
 *          skip logic of the encoder. For low delay applications this size
 *          should be small. This size is in bits.
 *          Default value is 0, where codec will internally calculate the
 *          value based on the RC algo type.
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_RateControlParams {
    XDAS_Int32 rateControlParamsPreset;
    XDAS_Int32 rcAlgo;
    XDAS_Int32 qpI;
    XDAS_Int32 qpP;
    XDAS_Int32 seIntialQP;
    XDAS_Int32 qpMax;
    XDAS_Int32 qpMin;
    XDAS_Int32 enablePerceptualQuantMode;
    XDAS_Int32 allowFrameSkip;
    XDAS_Int32 initialBufferLevel;
    XDAS_Int32 vbvBufferSize;
    XDAS_Int32 qpMinIntra;
} IMPEG4ENC_RateControlParams;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_InterCodingParams
 *  @brief  This structure contains all the parameters which controls Inter MBs
 *          coding behavior
 *
 *  @param  interCodingPreset  :
 *          This Preset controls the USER_DEFINED vs DEFAULT mode. if User is
 *          not aware about following fields, it should be set as
 *          IMPEG4_INTERCODING_DEFAULT
 *
 *  @param  searchRangeHorP  :
 *          Horizontal Search Range for ME algo, range is [16, 144]
 *
 *  @param  searchRangeVerP  :
 *          Vertical Search Range for ME algo, range is [16, 32]
 *
 *  @param  globalOffsetME  :
 *          This variable is used to control ME search algorithm to improve
 *          video quality by doing ME around Temporal average MV.
 *          1 for Enable
 *          0 for Disable
 *
 *  @param  earlySkipThreshold  :
 *          Threshold to use for early skip determination
 *          The Inter SAD is compared against this Threshold for early skip
 *          selection
 *
 *  @param  enableThresholdingMethod  :
 *          Thresholding cost Method is used by CALC3 suppress expensive
 *          coefficients.Thresholding cost Method is used to set a block to be
 *          not_coded if the block has very few small amplitude coeffs.
 *
 *  @param  minBlockSizeP  : minimum block size for P frames.
 *          Refer IMPEG4ENC_InterBlockSize enumeration to see the valid values
 *          if this variable takes value of IMPEG4_BLOCKSIZE_8x8 a MB in P Frame
 *          can have 4 Motion Vectors one for each 8x8 MB to
 *          improve video quality (not necessarily). This mode is used only for
 *          MPEG-4. This Field is neglected or read as Disable for H263 encoding.
 *          Set
 *          IMPEG4_BLOCKSIZE_8x8 for 4MV
 *          else only 1MV
 *
 *  @param  enableRoundingControl  :
 *          When enabled reduces the IDCT drift
 *          1 for Enable
 *          0 for Disable
 *******************************************************************************
*/
typedef struct IMPEG4ENC_InterCodingParams {
    XDAS_Int32  interCodingPreset;
    XDAS_Int32  searchRangeHorP;
    XDAS_Int32  searchRangeVerP;
    XDAS_UInt32 globalOffsetME;
    XDAS_Int32  earlySkipThreshold;
    XDAS_Int32  enableThresholdingMethod;
    XDAS_UInt32 minBlockSizeP;
    XDAS_UInt32 enableRoundingControl;

} IMPEG4ENC_InterCodingParams;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_IntraCodingParams
 *  @brief  This structure contains all the parameters which controls Intra
 *          encoding
 *
 *  @param  intraCodingPreset  :
 *          This Preset controls the USER_DEFINED vs DEFAULT mode. if User is
 *          not aware about following fields, it should be set as
 *          IMPEG4_INTERCODING_DEFAULT
 *
 *  @param  intraRefreshMethod  :
 *          Intra Refresh methods, this can be any one of the
 *          IMPEG4ENC_IntraRefreshMethods enumeration.
 *          refer IMPEG4ENC_IntraRefreshMethods enumeration above.
 *
 *  @param  intraRefreshRate  :
 *          if intraRefreshMethod is IMPEG4_INTRAREFRESH_CYCLIC_MBS, this value
 *          represents madulo cyclic MBs value. MPEG4 Encoder encodes a
 *          macro block as Intra after every intraRefreshRate number of macro
 *          blocks.if intraRefreshMethod is IMPEG4_INTRAREFRESH_CYCLIC_ROWS,
 *          this value represents number if rows which are intra. MPEG4 Encoder
 *          encodes those many rows as intra every frame and the location of
 *          intra rows moves in cyclic fashion.
 *          This variable is ignored if intraRefreshMethod is
 *          IMPEG4_INTRAREFRESH_NONE.
 *
 *  @param  acpredEnable  :
 *          AC prediction
 *          0 for Disable
 *          1 for Enable
 *
 *  @param  insertGOVHdrBeforeIframe  :
 *          inserts GOV Header before I Frame if enabled
 *          0 for Disable
 *          1 for Enable
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_IntraCodingParams {
    XDAS_Int32  intraCodingPreset;
    XDAS_UInt32 intraRefreshMethod;
    XDAS_UInt32 intraRefreshRate;
    XDAS_UInt32 acpredEnable;
    XDAS_UInt32 insertGOVHdrBeforeIframe;
    XDAS_UInt32 enableDriftControl;

} IMPEG4ENC_IntraCodingParams;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_sliceCodingParams
 *  @brief  This structure contains all the parameters which controls Intra
 *          encoding
 *
 *  @param  sliceCodingPreset  :
 *          This Preset controls the USER_DEFINED vs DEFAULT mode. if User is
 *          not aware about following fields, it should be set as
 *          IMPEG4_INTERCODING_DEFAULT
 *
 *  @param  sliceMode  :
 *         This defines the control mechanism to split a picture in slices.
 *         It can be either MB based or bits based and takes the enum
 *         IMPEG4ENC_SliceMode
 *
 *  @param  sliceUnitSize  :
 *         The meaning of this parameter depends upon sliceMode.
 *         sliceMode == IMPEG4_SLICEMODE_MBUNIT then this
 *         parameter informs the number of Macroblocks in one slice
 *         sliceMode == IMPEG4_SLICEMODE_BITS then this
 *         parameter informs the number of bits in one slice in MPEG4
 *         jargon resyncIntervalInBits
 *         sliceMode == IMPEG4_SLICEMODE_NONE then this
 *         parameter is not respected
 *
 *  @param  gobInterval  :
 *          insert GOB header after every n GOBs
 *          This field is only used for H263
 *          gobInterval range is [0, Max GOB Number-1]
 *
 *  @param  useHec  :
 *          Use Header extension code [0-2]
 *          0 for Disable
 *          1 Include HEC for only First GobHeader
 *          2 Include HEC for for all GobHeader
 *
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_sliceCodingParams {
    XDAS_Int32  sliceCodingPreset;
    XDAS_Int32  sliceMode;
    XDAS_Int32  sliceUnitSize;
    XDAS_UInt32 gobInterval;
    XDAS_UInt32 useHec;

} IMPEG4ENC_sliceCodingParams;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_DynamicParams
 *  @brief  This structure defines the dynamic parameters for MPEG4ENC objects
 *
 *  @param  videnc2DynamicParams  :
 *          refer IVIDENC2_DynamicParams in ividenc2.h file
 *
 *  @param  rateControlParams  :
 *          refer IMPEG4ENC_RateControlParams structure above
 *
 *  @param  interCodingParams  :
 *          refer IMPEG4ENC_InterCodingParams structure above
 *
 *  @param  sliceCodingParams  :
 *          refer IMPEG4ENC_sliceCodingParams structure above
 *
 *  @param  aspectRatioIdc  :
 *          defines the value of pixel aspect ratio
 *          See Table 6-12 of standard spec for aspect ratio details
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_DynamicParams {
    IVIDENC2_DynamicParams      videnc2DynamicParams;
    IMPEG4ENC_RateControlParams rateControlParams;
    IMPEG4ENC_InterCodingParams interCodingParams;
    IMPEG4ENC_sliceCodingParams sliceCodingParams;
    XDAS_UInt32                 aspectRatioIdc;
}IMPEG4ENC_DynamicParams;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_DynamicParams
 *  @brief  Default dynamic parameter values for MPEG4ENC instance objects
 *          if user defined dynamic parameters are not given to the encoder then
 *          encoder uses this default dynamic parameters (which can be found in
 *          impeg4enc.c file) for encoding.
 *
 *******************************************************************************
*/
extern IMPEG4ENC_DynamicParams    MPEG4ENC_TI_DYNAMICPARAMS;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_Params
 *  @brief  This structure defines the creation parameters for MPEG4ENC objects
 *
 *  @param  videnc2Params  :
 *          Defines creation time parameters for all IVIDENC2 instance objects.
 *          refer IVIDENC2_Params structure in ividenc2.h file
 *
 *  @param  rateControlParams  :
 *          refer IMPEG4ENC_RateControlParams structure above
 *
 *  @param  interCodingParams  :
 *          refer IMPEG4ENC_InterCodingParams structure above
 *
 *  @param  intraCodingParams  :
 *          refer IMPEG4ENC_IntraCodingParams structure above
 *
 *  @param  sliceCodingParams  :
 *          refer IMPEG4ENC_sliceCodingParams structure above
 *
 *  @param  useDataPartitioning  :
 *          Controls data partitioning for MPEG4 Encoder.
 *          This mode is automatically disabled when short video header is
 *          enabled
 *          0 for Disable
 *          1 for Enable
 *
 *  @param  useRvlc  :
 *          Use Reversible Variable Length Coding.
 *          MPEG4 Encoder expects Data Partitioning to be enabled when RVLC is
 *          enabled or else it returns error.
 *          This mode is automatically disabled when short video header is
 *          enabled
 *          0 for Disable
 *          1 for Enable
 *
 *  @param  useShortVideoHeader  :
 *          short video header / h263 base line profile
 *          MPEG4 Encoder automatically disable the 4 MV, Data Partitioning,
 *          RVLC modes and reset resync interval in bits (disable H.241 flow)
 *          0 for Disable
 *          1 for Enable
 *
 *  @param  vopTimeIncrementResolution  :
 *          resolution of vop_time_increment bit-stream syntax element,
 *          number of ticks/sec
 *
 *  @param  nonMultiple16RefPadMethod  :
 *          Controls the way the padding is done for Ref Frame when Height is
 *          non-multiple of  16.
 *          Follows the enum IMPEG4ENC_nonMultiple16RefPadMethod
 *          IMPEG4_PAD_METHOD_DIVX  - VLC, DIVx way of padding
 *          IMPEG4_PAD_METHOD_MPEG4 - MPEG4 Standard specific way of padding
 *          default value is IMPEG4_PAD_METHOD_MPEG4.
 *
 * @param pixelRange :video_range=0 :Y from 16 to 235, Cb and Cr from 16 to 240;
 *                    video_range=1 : Y from 0 to 255,Cb and Cr from 0 to 255.
 *
 * @param enableSceneChangeAlgo : Parameter to enable or disable scene change
 *                                algorithm.
 * @param useVOS                : VOS header insertion, 0 = off, 1 = on
 * @param enableMONA            : enable MONA settings  0 = off, 1 = on
 * @param enableAnalyticinfo    : enable MV and SAD access to user
 *                                0 = off, 1 = on
 *
 * @param debugTraceLevel       : Indicates level of debug trace info to be
 *                                dumped.
 *                                Disabled if this value is zero.
 *
 * @param lastNFramesToLog      : Indicates no. of frames for which debug trace
 *                              info to be dumped. Valid only if debugTraceLevel
 *                   is non zero.
 *******************************************************************************
*/
typedef struct IMPEG4ENC_Params {
    IVIDENC2_Params             videnc2Params;
    IMPEG4ENC_RateControlParams rateControlParams;
    IMPEG4ENC_InterCodingParams interCodingParams;
    IMPEG4ENC_IntraCodingParams intraCodingParams;
    IMPEG4ENC_sliceCodingParams sliceCodingParams;

    XDAS_UInt32 useDataPartitioning;
    XDAS_UInt32 useRvlc;
    XDAS_UInt32 useShortVideoHeader;
    XDAS_UInt32 vopTimeIncrementResolution;
    XDAS_UInt32 nonMultiple16RefPadMethod;
    XDAS_UInt32 pixelRange;
    XDAS_UInt32 enableSceneChangeAlgo;
    XDAS_UInt32 useVOS;
    XDAS_UInt32 enableMONA;
    XDAS_Int32  enableAnalyticinfo;

    XDAS_UInt32 debugTraceLevel;
    XDAS_UInt32 lastNFramesToLog;

} IMPEG4ENC_Params;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_Params
 *  @brief  Default parameter values for MPEG4ENC instance objects
 *          if user defined parameters are not given to the encoder then
 *          encoder uses this default parameters (which can be found in
 *          impeg4enc.c file) for encoding.
 *
 *******************************************************************************
*/
extern IMPEG4ENC_Params    MPEG4ENC_TI_PARAMS;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_Status
 *  @brief  Status structure defines the parameters that can be changed or read
 *          during real-time operation of the alogrithm.
 *
 *  @param  videnc2Status  :
 *          Status of the MPEG4 encoder along with error information, if any.
 *          refer IVIDENC2_Status structure in ividenc2.h file
 *
 *  @param  rateControlParams  :
 *          refer IMPEG4ENC_RateControlParams structure above
 *
 *  @param  interCodingParams  :
 *          refer IMPEG4ENC_InterCodingParams structure above
 *
 *  @param  intraCodingParams  :
 *          refer IMPEG4ENC_IntraCodingParams structure above
 *
 *  @param  sliceCodingParams  :
 *          refer IMPEG4ENC_sliceCodingParams structure above
 *
 *  @param  useDataPartitioning  :
 *          Use data partitioning
 *          0 for Disable
 *          1 for Enable
 *
 *  @param  useRvlc  :
 *          Use Reversible Variable Length Coding
 *          0 for Disable
 *          1 for Enable
 *
 *  @param  useShortVideoHeader  :
 *          short video header / h263 base line profile
 *          0 for Disable
 *          1 for Enable
 *  @param  vopTimeIncrementResolution  :Resolution of vop_time_increment
 *                                       bit-stream syntax element,
 *                                       number of ticks/sec.
 *
 *  @param  nonMultiple16RefPadMethod  : Controls the way the padding is done
 *                                       for Ref Frame when Height is
 *                                       Non-multiple of 16.
 *  @param  pixelRange  : Pixel range to be put in header
 *                        See IMPEG4VENC_PixelRange enumeration for details.
 *
 *  @param  enableSceneChangeAlgo  :
 *          Scene change detection algorithm.
 *          0 for Disable
 *          1 for Enable
 * @param   useVOS                : VOS header insertion, 0 = off, 1 = on
 * @param   enableMONA            : enable MONA settings  0 = off, 1 = on
 * @param   enableAnalyticinfo    : enable MV and SAD access to user
 *                                  0 = off, 1 = on
 *
 * @param   debugTraceLevel        :
 *          0 Disable dumping debug data
 *           1-4 enable dumping debug data
 *
 * @param lastNFramesToLog : No. of frame for which debug trace info to be
 *                           dumped.
 *
 * @param   extMemoryDebugTraceAddr : External memory address where debug trace
 *                                    info is dunped
 *
 * @param   extMemoryDebugTraceSize : Size of the debug trace info in the
 *                                    external memory.
 *
 *******************************************************************************
*/
typedef struct {
    IVIDENC2_Status             videnc2Status;
    IMPEG4ENC_RateControlParams rateControlParams;
    IMPEG4ENC_InterCodingParams interCodingParams;
    IMPEG4ENC_IntraCodingParams intraCodingParams;
    IMPEG4ENC_sliceCodingParams sliceCodingParams;

    XDAS_UInt32 useDataPartitioning;
    XDAS_UInt32 useRvlc;
    XDAS_UInt32 useShortVideoHeader;
    XDAS_UInt32 vopTimeIncrementResolution;
    XDAS_UInt32 nonMultiple16RefPadMethod;
    XDAS_UInt32 pixelRange;
    XDAS_UInt32 enableSceneChangeAlgo;
    XDAS_UInt32 useVOS;
    XDAS_UInt32 enableMONA;
    XDAS_Int32  enableAnalyticinfo;

    XDAS_UInt32  debugTraceLevel;
    XDAS_UInt32  lastNFramesToLog;
    XDAS_UInt32 *extMemoryDebugTraceAddr;
    XDAS_UInt32  extMemoryDebugTraceSize;

} IMPEG4ENC_Status;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_InArgs
 *  @brief  This structure defines the runtime input arguments for
 *          IMPEG4ENC::process function
 *
 *  @param  videnc2InArgs  :
 *          Parameters common to video encoders
 *          refer IVIDENC2_InArgs structure in ividenc2.h file
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_InArgs {
    IVIDENC2_InArgs videnc2InArgs;
} IMPEG4ENC_InArgs;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_OutArgs
 *  @brief  This structure defines the run time output arguments for
 *          IMPEG4ENC::process function
 *
 *  @param  videnc2OutArgs  :
 *          output parameters from the IMPEG4ENC::process call
 *          refer IVIDENC2_OutArgs structure in ividenc2.h file
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_OutArgs {
    IVIDENC2_OutArgs videnc2OutArgs;
} IMPEG4ENC_OutArgs;


/**
 *******************************************************************************
 *  @struct IMPEG4ENC_Fxns
 *  @brief  This structure defines all of the operations on MPEG4ENC objects
 *
 *  @param  ividenc  :
 *          refer IVIDENC2_Fxns structure in ividenc2.h file
 *
 *******************************************************************************
*/
typedef struct IMPEG4ENC_Fxns {
    IVIDENC2_Fxns ividenc; /* IMPEG4ENC extends IVIDENC */
} IMPEG4ENC_Fxns;

/**
 ******************************************************************************
 *  @struct IMPEG4ENC_DataSyncDesc
 *  @brief  This structure is an extension of XDM_DataSyncDesc to provide add-
 *           itional information required for Mode B Packetization according to
 *           RFC2190.
 *
 *  @param mbAddr: this is a pointer to an array of FIrst MB Addresses in diff
 *                   packets.
 *  @param gobNumber: this is a pointer to an array of GOB Number of first Mb
 *                     in different packets
 *  @param quantScale: this is a pointer to an array of Quant values of first
 *                      Mb in different packets
 *  @param mv: this is a pointer to an array of MV of first MB in different
 *              packets. Note: the MV is in half-pel reolution as required in
 *              RFC2190.
 ******************************************************************************
*/
typedef struct IMPEG4ENC_DataSyncDesc {
    XDM_DataSyncDesc dataSyncDesc;
    XDAS_UInt16     *mbAddr;
    XDAS_UInt16     *gobNumber;
    XDAS_UInt16     *quantScale;
    XDAS_UInt32     *mv;
} IMPEG4ENC_DataSyncDesc;

#ifdef __cplusplus
}
#endif /* extern "C" */

/*@}*/ /* ingroup HDVICP2MPEG4 */

#endif /* IMPEG4ENC_ */

