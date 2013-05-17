/*
 * Copyright (c) 2012-2013, Texas Instruments Incorporated
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
 */

/** ============================================================================
 *  @file       MmRpc.h
 *
 *  @brief      Multi-Media derived Remote Procedure Call
 *
 *  ============================================================================
 */

#ifndef ti_ipc_mm_MmRpc__include
#define ti_ipc_mm_MmRpc__include

/* add includes here */
#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 *  @brief  Operation is successful
 */
#define MmRpc_S_SUCCESS (0)

/*!
 *  @brief  Operation failed
 */
#define MmRpc_E_FAIL (-1)

/*!
 *  @brief  Invalid parameter type
 */
#define MmRpc_E_INVALIDPARAM (-2)

/*!
 *  @brief  Size of parameter array in function context structure
 */
#define MmRpc_MAX_PARAMS (10)

/*!
 *  @brief  Maximum size of translation array in function context structure
 */
#define MmRpc_MAX_TRANSLATIONS (1024)

/*!
 *  @brief  Macro for computing offset to a field of a structure.
 *
 *          struct foobar {
 *              int a;
 *              int *p;
 *          };
 *
 *          struct foobar *sp = ...;
 *          offset = MmRpc_OFFSET(sp, &sp->p);
 *          struct foobar st = ...;
 *          offset = MmRpc_OFFSET(&st, &st.p);
 */
#define MmRpc_OFFSET(base, field) ((unsigned int)(field)-(unsigned int)(base))

/*!
 *  @brief      MmRpc_Handle type
 */
typedef struct MmRpc_Object *MmRpc_Handle;

/*!
 *  @brief      MmRpc_ParamType enum
 */
typedef enum {
    MmRpc_ParamType_Scalar = 1, /*!< pass by value */
    MmRpc_ParamType_Ptr,        /*!< data pointer */
    MmRpc_ParamType_Elem        /*!< array element */
} MmRpc_ParamType;

/*!
 *  @brief      MmRpc_Param type
 */
typedef struct {
    MmRpc_ParamType     type;   /*!< parameter type */

    union {
        struct {
            size_t      size;   /*!< size of the data */
            size_t      data;   /*!< data (pass by value)*/
        } scalar;

        struct {
            size_t      size;   /*!< size of the data referenced */
            size_t      addr;   /*!< pointer value */
            size_t      handle; /*!< memory allocator handle */
        } ptr;

#if 0 /* TBD */
        struct {
            size_t      size;   /*!< size of the array element */
            size_t      offset; /*!< offset to current array element */
            size_t      base;   /*!< base address of array */
            size_t      handle; /*!< memory allocator handle */
        } elem;
#endif
    } param;
} MmRpc_Param;

typedef struct {
    uint32_t    index;  /*!< parameter index to base pointer */
    ptrdiff_t   offset; /*!< offset from the base address to pointer */
    size_t      base;   /*!< user virtual address */
    size_t      handle; /*!< memory allocator handle */
} MmRpc_Xlt;

/*!
 *  @brief      Function call context structure
 */
typedef struct {
    uint32_t    fxn_id;         /*!< function id to call */
    uint32_t    num_params;     /*!< number of parameters in params array */
    MmRpc_Param params[MmRpc_MAX_PARAMS];
                                /*!< the array of parameters */
    uint32_t    num_xlts;       /*!< number of translations in xltAry */
    MmRpc_Xlt * xltAry;         /*!< array of translations */
} MmRpc_FxnCtx;

/*!
 *  @brief      Instance create parameters
 */
typedef struct {
    int reserved;
} MmRpc_Params;

/*!
 *  @brief      Invoke a remote procedure call
 *
 */
int MmRpc_call(MmRpc_Handle handle, MmRpc_FxnCtx *ctx, int32_t *ret);

/*!
 *  @brief      Create an MmRpc instance
 *
 */
int MmRpc_create(const char *service, const MmRpc_Params *params,
        MmRpc_Handle *handlPtr);

/*!
 *  @brief      Delete an MmRpc instance
 *
 */
int MmRpc_delete(MmRpc_Handle *handlePtr);

/*!
 *  @brief      Initialize the instance create parameter structure
 *
 */
void MmRpc_Params_init(MmRpc_Params *params);



#if defined(__cplusplus)
}
#endif
#endif /* ti_ipc_mm_MmRpc__include */
