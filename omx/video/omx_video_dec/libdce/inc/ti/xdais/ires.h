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
 *  @file       ti/xdais/ires.h
 *
 *  @brief      IRES Interface Definitions - Allows algorithms to
 *              request and receive handles representing private logical
 *              resources.
 */

/**
 *  @defgroup   DSPIRES IRES
 *
 *              The IRES interface enables algorithms to request and
 *              receive handles representing private logical resources.
 */
#ifndef IRES_
#define IRES_

/** @ingroup    DSPIRES */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif

#include "ires_common.h"

typedef enum IRES_YieldResourceType {
    IRES_ALL = 0,                       /**< Yielding all resources held by the
                                         *   algorithm.
                                         */
    IRES_SCRATCHMEMORY,                 /**< Yielding all scratch memory
                                         * resources held by the algorithm.
                                         */
    IRES_SCRATCHDMA,                    /**< Yielding all IDMA3 resources held
                                         *   by the algorithm.
                                         */
    IRES_RESOURCEHANDLE                 /**< Yielding IRES resource handle only.
                                         */

} IRES_YieldResourceType;

typedef Void (*IRES_ContextSaveFxn)(IALG_Handle algHandle, Void *contextArgs);
typedef Void (*IRES_ContextRestoreFxn)
        (IALG_Handle algHandle, Void *contextArgs);

/**
 *  @brief      Specifies the algorithm specific handles
 *              and context save & restore function pointers and arguments
 *              that the framework will call during a context switch.
 */
typedef struct IRES_YieldContext {

    /**
     *  Handle of the algorithm instance issuing the "yield".
     */
    IALG_Handle                algHandle;

    /**
     *  Optional resource handle that may be used to yield a particular
     *  IRES resource.
     *  May be NULL when some other resource or ALL resources associated
     *  with an algorithm are to be yielded.
     */
    IRES_Handle                resourceHandle;

    /**
     *  The 'contextSave' function is implemented by the 'yielding' algorithm
     *  instance and must be called by the framework whenever the 'yield'
     *  results  in a context switch.
     *  Any 'scratch' IRES, IALG or IDMA3 resource owned by the algorithm must
     *  be considered in volatile state during a 'yield' call, and must be
     *  re-initialized and activated following any context switch.
     *  The 'contextSave' function must save any volatile state information in
     *  its persistent memory that it needs in the subsequent 'contextRestore'
     *  function call to re-initialize and activate its  scratch resources.
     */
    IRES_ContextSaveFxn        contextSave;

    /**
     *  The 'contextRestore' function is implemented by the 'yielding' algorithm
     *  instance and must be called by the framework following a context switch
     *  which occured as a result of the 'yield', prior to returning from the
     *  framework supplied 'yield-function'.
     *
     *  Any 'scratch' IRES, IALG or IDMA3 resource owned by the algorithm must
     *  be considered in volatile state during a 'yield' call, and must be
     *  re-initialized and activated following any context swith.
     *  The 'contextRestore' function uses the saved context information in
     *  the preceeding 'contextSave' call to re-initialize and activate its
     *  scratch resources.
     */
    IRES_ContextRestoreFxn      contextRestore;

    /**
     *  The arguments that must be passed when calling the 'contextSave()'
     *  and 'contextRestore()' functions.
     *  The algorithm instance calling the 'yield function' optionally
     *  supplies any 'contextArgs' that it wants the framework to passed to the
     *  'contextSave()' and  'contextRestore()' functions.
     */
    Void                        *contextArgs;

} IRES_YieldContext;


typedef IRES_YieldContext *IRES_YieldContextHandle;
typedef Void *IRES_YieldArgs;

/**
 *  @brief      The 'yield function' pointer and 'yield arguments' are supplied
 *              to an algorithm during its IRES interface 'initialization'.
 *              The algorithm optionally calls the yield function during its
 *              'process' stages for co-operative multi-tasking and pre-emption
 *              by a higher priority algorithm.
 *
 *              An operating system level context switch may occur during the
 *              yield call. Whenever a context switch occurs, all
 *              scratch resources owned by the yielding algorithm must be
 *              context saved prior to the context switch and restored and
 *              initialized prior to 'resumption' by the framework/application.
 *              The 'yielding' algorithm must supply the IRES_YieldContext and
 *              the 'yield arguments' it received during IRES initialization.
 *              The framework calls the context save & restore functions
 *              provided by the yielding algorithm's IRES_YieldContext.
 */
typedef Void (*IRES_YieldFxn)(IRES_YieldResourceType resourceType,
    IRES_YieldContextHandle algYieldContext, IRES_YieldArgs yieldArgs);

/**
 *  @brief      These fxns are used to query/grant the resources requested
 *              by the algorithm at initialization time, and to change these
 *              resources at runtime. All these fxns are implemented by the
 *              algorithm, and called by the client of the algorithm.
 */
typedef struct IRES_Fxns {

    /**
     *  @brief  Unique pointer that identifies the module implementing this
     *          interface.
     */
    Void *implementationId;

    /**
     *  @brief  Query function to obtain the list of IRES resources requested
     *          by the algorithm instance.
     */
    IRES_Status (*getResourceDescriptors)(IALG_Handle handle,
        IRES_ResourceDescriptor *resourceDescriptors);

    /**
     *  @brief  Query function to obtain the number of IRES resources
     *          requested by the algorithm instance, which is also the number
     *          of resource descriptors that must be passed to the
     *          getResourceDescriptors() function.
     *
     */
    Int32 (*numResourceDescriptors)(IALG_Handle handle);

    /**
     *  @brief  Assignment function to grant the algorithm instance the list
     *          of IRES resources it requested. The algorithm can initialize
     *          internal instance memory with resource information, but may not
     *          use or access the resource state until the resource is
     *          activated via the activateResource call.
     *
     *  @remark Version of the resource handle being granted is updated in
     *          the resourceDescriptor, and may be different from the one
     *          requested.
     */
    IRES_Status (*initResources)(IALG_Handle handle,
            IRES_ResourceDescriptor * resourceDescriptor,
            IRES_YieldFxn yieldFxn, IRES_YieldArgs yieldArgs);

    /**
     *  @brief  Re-assignment function to grant the algorithm instance
     *          a list of "modified" IRES resources.
     *          The algorithm may choose to not support the re-assignment and
     *          indicate this by returning failure.
     *          In case of success the algorithm updates its internal state
     *          to reflect the new resource information, but may not
     *          use or access the resource state until the resource is
     *          activated via the activateResource call.
     */
    IRES_Status (*reinitResources)(IALG_Handle handle,
            IRES_ResourceDescriptor * resourceDescriptor,
            IRES_YieldFxn yieldFxn, IRES_YieldArgs yieldArgs);

    /**
     *  @brief  Deinitialization function to revoke back the resources
     *          that have been granted to the algorithm instance.
     */
    IRES_Status (*deinitResources)(IALG_Handle handle,
            IRES_ResourceDescriptor * resourceDescriptor);

    /**
     *  @brief  Resource Activation call to grant the algorithm instance
     *          exclusive access to the potentially shared resource.
     *          Algorithm can now access, initialize and or restore from a
     *          previous context saved during deactivation to use the resource.
     *          Each resource must be activated individually to give the
     *          framework optimization opportunities by tracking and deciding
     *          which resources truely require activation.
     */
    IRES_Status (*activateResource)(IALG_Handle handle,
            IRES_Handle resourceHandle);

    /**
     *  @brief  Resource Activation call to grant the algorithm instance
     *          exclusive access to all resources it acquired via IRES.
     *          Algorithm can now access, initialize and or restore from a
     *          previous context saved during deactivation to use the resources.
     */
    IRES_Status (*activateAllResources)(IALG_Handle handle);

    /**
     *  @brief  Resource Deactivation call to revoke the algorithm instance's
     *          exclusive access to the potentially shared resource.
     *          Algorithm must save any context that is needed to restore the
     *          state during the next resource activation call.
     */
    IRES_Status (*deactivateResource)(IALG_Handle handle,
            IRES_Handle resourceHandle);

    /**
     *  @brief  Resource Deactivation call to revoke the algorithm instance's
     *          exclusive access to ALL shared resources.
     *          Algorithm must save any context that is needed to restore the
     *          state during the next resource activation call.
     */
     IRES_Status (*deactivateAllResources)(IALG_Handle handle);

} IRES_Fxns;

/*@}*/


#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  /* IRES_ */
/*
 *  @(#) ti.xdais; 1, 2.0, 1,1; 6-19-2012 17:57:45; /db/wtree/library/trees/dais/dais-w06/src/ xlibrary

 */

