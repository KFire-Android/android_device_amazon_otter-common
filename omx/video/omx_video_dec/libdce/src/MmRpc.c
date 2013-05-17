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

/*
 *  ======== MmRpc.c ========
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <stdint.h> /* should be in linux/rpmsg_rpc.h */
#include <stddef.h> /* should be in linux/rpmsg_rpc.h */

#include <utils/Log.h>


#if defined(KERNEL_INSTALL_DIR)

#ifdef linux
#define _linux_ linux
#undef linux
#endif
#define linux_include(kd,m) <kd/include/linux/m.h>
#include linux_include(KERNEL_INSTALL_DIR,rpmsg_rpc)
#ifdef _linux_
#define linux _linux
#undef _linux_
#endif

#define RPPC_MAX_INST_NAMELEN (48) /* in kernel part of rpmsg_rpc.h */

/* this should be in rpmsg_rpc.h, currently in rpmsg_rpc_internal.h */
struct rppc_create_instance {
    char name[RPPC_MAX_INST_NAMELEN];
};

#elif defined(SYSLINK_BUILDOS_QNX)

#include <ti/ipc/rpmsg_rpc.h>

#else
//#error Unsupported Operating System
#include <rpmsg_rpc.h>
#define STUB   //till linux mmrpc is supported, stub all the calls
#endif

#include "MmRpc.h"


/*
 *  ======== MmRpc_Object ========
 */
typedef struct {
    int                         fd;         /* device file descriptor */
    struct rppc_create_instance connect;    /* connection object */
} MmRpc_Object;

/*
 *  ======== MmRpc_Params_init ========
 */
void MmRpc_Params_init(MmRpc_Params *params)
{
    params->reserved = 0;
}

/*
 *  ======== MmRpc_create ========
 */
int MmRpc_create(const char *service, const MmRpc_Params *params,
        MmRpc_Handle *handlePtr)
{
    int             status = MmRpc_S_SUCCESS;
    MmRpc_Object *  obj;
    char            cbuf[RPPC_MAX_INST_NAMELEN+16];
#ifdef STUB
return status;
#endif

    /* allocate the instance object */
    obj = (MmRpc_Object *)calloc(1, sizeof(MmRpc_Object));

    if (obj == NULL) {
        status = MmRpc_E_FAIL;
        goto leave;
    }

    /* open the driver */
    sprintf(cbuf, "/dev/%s", service);
    obj->fd = open(cbuf, O_RDWR);

    if (obj->fd < 0) {
        printf("MmRpc_create: Error: open failed\n");
        status = MmRpc_E_FAIL;
        goto leave;
    }

    strncpy(obj->connect.name, service, (RPPC_MAX_INST_NAMELEN - 1));
    obj->connect.name[RPPC_MAX_INST_NAMELEN - 1] = '\0';

    /* create a server instance, rebind its address to this file descriptor */
   // status = ioctl(obj->fd, RPPC_IOC_CREATE, &obj->connect);

    if (status < 0) {
        printf("MmRpc_create: Error: connect failed\n");
        status = MmRpc_E_FAIL;
        goto leave;
    }

leave:
    if (status < 0) {
        if ((obj != NULL) && (obj->fd >= 0)) {
            close(obj->fd);
        }
        if (obj != NULL) {
            free(obj);
        }
        *handlePtr = NULL;
    }
    else {
        *handlePtr = (MmRpc_Handle)obj;
    }

    return(status);
}

/*
 *  ======== MmRpc_delete ========
 */
int MmRpc_delete(MmRpc_Handle *handlePtr)
{
    int status = MmRpc_S_SUCCESS;
    MmRpc_Object *obj;
#ifdef STUB
return status;
#endif
    obj = (MmRpc_Object *)(*handlePtr);

    /* close the device */
    if ((obj != NULL) && (obj->fd >= 0)) {
        close(obj->fd);
    }

    /* free the instance object */
    free((void *)(*handlePtr));
    *handlePtr = NULL;

    return(status);
}

/*
 *  ======== MmRpc_call ========
 */
int MmRpc_call(MmRpc_Handle handle, MmRpc_FxnCtx *ctx, int32_t *ret)
{
    int status = MmRpc_S_SUCCESS;
    MmRpc_Object *obj = (MmRpc_Object *)handle;
    struct rppc_function *rpfxn;
    struct rppc_function_return reply_msg;
    MmRpc_Param *param;
    void *msg;
    int len;
    int i;
#ifdef STUB
   *ret = status;
return status;
#endif

    /* Combine function parameters and translation array into one contiguous
     * message. TODO, modify driver to accept two separate buffers in order
     * to eliminate this step. */
    len = sizeof(struct rppc_function) +
                (ctx->num_xlts * sizeof(struct rppc_param_translation));
    msg = (void *)calloc(len, sizeof(char));

    if (msg == NULL) {
        ALOGE("MmRpc_call: Error: msg alloc failed\n");
        status = MmRpc_E_FAIL;
        goto leave;
    }

    /* copy function parameters into message */
    rpfxn = (struct rppc_function *)msg;
    rpfxn->fxn_id = ctx->fxn_id;
    rpfxn->num_params = ctx->num_params;

    for (i = 0; i < ctx->num_params; i++) {
        param = &ctx->params[i];

        switch (param->type) {
            case MmRpc_ParamType_Scalar:
                rpfxn->params[i].type = RPPC_PARAM_TYPE_ATOMIC;
                rpfxn->params[i].size = param->param.scalar.size;
                rpfxn->params[i].data = param->param.scalar.data;
                rpfxn->params[i].base = 0;
                rpfxn->params[i].reserved = 0;
                break;

            case MmRpc_ParamType_Ptr:
                rpfxn->params[i].type = RPPC_PARAM_TYPE_PTR;
                rpfxn->params[i].size = param->param.ptr.size;
                rpfxn->params[i].data = param->param.ptr.addr;
                rpfxn->params[i].base = param->param.ptr.addr;
                rpfxn->params[i].reserved = param->param.ptr.handle;
                break;

#if 0 /* TBD */
            case MmRpc_ParamType_Elem:
                rpfxn->params[i].type = RPPC_PARAM_TYPE_PTR;
                rpfxn->params[i].size = param->param.elem.size;
                rpfxn->params[i].data = param->param.elem.offset;
                rpfxn->params[i].base = param->param.elem.base;
                rpfxn->params[i].reserved = param->param.elem.handle;
                break;
#endif
            default:
                ALOGE("MmRpc_call: Error: invalid parameter type\n");
                status = MmRpc_E_INVALIDPARAM;
                goto leave;
                break;
        }
    }

    /* copy offset array into message */
    rpfxn->num_translations = ctx->num_xlts;

    for (i = 0; i < ctx->num_xlts; i++) {
        rpfxn->translations[i].index    = ctx->xltAry[i].index;
        rpfxn->translations[i].offset   = ctx->xltAry[i].offset;
        rpfxn->translations[i].base     = ctx->xltAry[i].base;
        rpfxn->translations[i].reserved = ctx->xltAry[i].handle;
    }

    /* send message for remote execution */
    status = write(obj->fd, msg, len);

    if (status < 0) {
        ALOGE("MmRpc_call: Error: write failed\n");
        status = MmRpc_E_FAIL;
        goto leave;
    }

    /* wait for return status from remote service */
    status = read(obj->fd, &reply_msg, sizeof(struct rppc_function_return));

    if (status < 0) {
        ALOGE("MmRpc_call: Error: read failed\n");
        status = MmRpc_E_FAIL;
        goto leave;
    }
    else if (status != sizeof(struct rppc_function_return)) {
        ALOGE("MmRpc_call: Error: reply bytes=%d, expected %d\n",
                status, sizeof(struct rppc_function_return));
        status = MmRpc_E_FAIL;
        goto leave;
    }
    else {
        status = MmRpc_S_SUCCESS;
    }

    *ret = (int32_t)reply_msg.status;

leave:
    if (msg != NULL) {
        free(msg);
    }

    return(status);
}
