/* 
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/**
 *  @file       ti/xdais/ires_common.h
 *
 *  @brief      IRES Resource Protocol Definitions - IRES Resource
 */

#ifndef IRES_COMMON_
#define IRES_COMMON_

/** @ingroup    DSPIRES */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif

#include "ialg.h"


/**
 *  @brief      IRES_Status defines the standard error and success codes
 *              returned by IRES APIs.
 */
typedef enum IRES_Status {
    IRES_OK = 0,                /**< Success */
    IRES_EALG = 1,              /**< Error in algorithm IRES_Fxns functions */
    IRES_EEXISTS = 2,           /**< Error, already exists      */
    IRES_EFAIL = 3,             /**< Generic Error Message      */
    IRES_EINIT = 4,             /**< Error, already initialized */
    IRES_ENOINIT = 5,           /**< Error, not initialized     */
    IRES_ENOMEM = 6,            /**< Error, not enough memory   */
    IRES_ENORESOURCE = 7,       /**< Error, resource unavailable*/
    IRES_ENOTFOUND = 8          /**< Error, not found           */
} IRES_Status;

/**
 *  @brief      Protocol revision type.
 *              Used to ensure the given 'protocol' revision can be validated.
 */
typedef struct IRES_ProtocolRevision {
    unsigned int Major;
    unsigned int Source;
    unsigned int Radius;                /* Using unsigned int here to support
                                         * xdc/std.h as well as tistdtypes.h
                                         */
} IRES_ProtocolRevision;


/**
 *  @brief      Mode in which resources can be requested.
 */
typedef enum IRES_RequestMode {
    IRES_SCRATCH,               /**< Indicates that resource requested
                                 *   can be shared with other
                                 *   algorithms in the same group.
                                 */
    IRES_PERSISTENT,            /**< Indicates that resource requested
                                 *   for allocation exclusively for
                                 *   this algorithm.
                                 */
    IRES_LATEACQUIRE            /**< Indicates that resource requested
                                 *   will not be granted immediately, but
                                 *   afterwards. Specific IRES APIs will need
                                 *   to be called, to ensure that a particular
                                 *   algorithm is the only user of these
                                 *   resources.
                                 */
} IRES_RequestMode;


/**
 *  @brief      Abstract Protocol Arguments structure definition.
 *              Actual arguments passed by the algorithm to request a resource
 *              from a specific IRES Protocol will extend and supply the
 *              concrete definitions.
 */
typedef struct IRES_ProtocolArgs {

    Int32 size;                 /**< Size of this structure. */
    IRES_RequestMode mode;      /**< Resource request mode.
                                 *
                                 *  @sa IRES_RequestMode
                                 */
} IRES_ProtocolArgs;

/**
 *  @brief      Descriptor to Logical Resource.
 */
typedef struct IRES_ResourceDescriptor {

    /**
     *  String containing the package name to identify the resource.
     */
    String resourceName;

    /**
     *  @brief  Pointer to the Resource Protocol Arguments.
     *
     *  @remark The Resource Manager selects the appropriate resource protocol
     *          based on the supplied @c resourceName, and uses the protocol to
     *          construct the IRES Resource Handle.
     */
    struct IRES_ProtocolArgs  *protocolArgs;

    /**
     *  @brief  The revision of the IRES_ResourceProtocol Interface expected
     *          by the client algorithm.
     *
     *  @remark Depending on the resource manager implementation,
     *          the returned resource may/may not coincide with the version
     *          being requested. Resource manager will update this field with
     *          the version of the handle it returns (and supports).
     */
    IRES_ProtocolRevision *revision;

    /**
     *  The handle to the object representing the requested resource.
     *  The handle is initially set to 'null' by the requesting algorithm.
     *  The 'resource manager' allocates the resource and constructs the
     *  handle.
     */
    struct IRES_Obj * handle;

} IRES_ResourceDescriptor;

/**
 *  @brief      Abstract Resource Properties structure/pointer definition.
 *              Actual resource protocol will supply the concrete property
 *              definitions.
 *              The list of attributes for the actual resource will expose the
 *              relevant features that needs to be known to a client to use the
 *              resource, such as: resource register base addresses and offsets,
 *              critical register and memory region addresses, ...
 */
typedef struct IRES_Properties {
    Int32 size;                 /**< Size of this structure. */
} IRES_Properties;

/**
 *  @brief      IRES_Obj holds the private state associated with each
 *              logical resource.
 */
typedef struct IRES_Obj {

    /**
     * Indicates if the resource has been allocated as persistent.
     */
    Int32 persistent;

    /**
     *  Obtain the static properties associated with this resource
     *  This could include information like the register layer of the
     *  device etc.
     */
    Void (*getStaticProperties) (struct IRES_Obj * resourceHandle,
        IRES_Properties * resourceProperties);

} IRES_Obj;

/**
 *  @brief      Handle to "logical" resource
 */
typedef struct IRES_Obj *IRES_Handle;



#ifdef __cplusplus
}
#endif /* extern "C" */

/*@}*/

#endif  /* IRES_COMMON_ */
/*
 *  @(#) ti.xdais; 1, 2.0, 1,1; 6-19-2012 17:57:45; /db/wtree/library/trees/dais/dais-w06/src/ xlibrary

 */

