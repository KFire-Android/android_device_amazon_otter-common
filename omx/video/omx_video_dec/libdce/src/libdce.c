/*
 * Copyright (c) 2011, Texas Instruments Incorporated
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <omap_dce.h>
#include <dce.h>
#include <MmRpc.h>
#include <xdc/std.h>
#include <ti/dce/dce_priv.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/video3/viddec3.h>
#include <ti/sdo/ce/video2/videnc2.h>
#include <errno.h>

#include <memplugin.h>
#include <utils/Log.h>

#define SERVER_NAME "dCE"

static int init(void);
static void deinit(void);

static int fd = -1;
static struct omap_device *dev;
static int ioctl_base;
#define CMD(name) (ioctl_base + DRM_OMAP_DCE_##name)

#define MEMPLUGIN_BUFFER_PARAMS_INIT(MEMPLUGIN_bufferinfo) do {\
        MEMPLUGIN_bufferinfo.eBuffer_type = DEFAULT;\
        MEMPLUGIN_bufferinfo.nHeight = 1;\
        MEMPLUGIN_bufferinfo.nWidth  = -1;\
        MEMPLUGIN_bufferinfo.bMap   = OMX_FALSE;\
        MEMPLUGIN_bufferinfo.eTiler_format = -1;\
} while(0)

#define LINUX_PAGE_SIZE 4096
static uint32_t nMemmgrClientDesc;
static void* pMemPluginHandle;
static MEMPLUGIN_BUFFER_PARAMS newbuffer_params;
static MEMPLUGIN_BUFFER_PROPERTIES newbuffer_prop;

uint32_t dce_debug = 3;

void dce_set_fd(int dce_fd) { fd = dce_fd; }
int dce_get_fd () { return fd; }

static int init(void);
static void deinit(void);
/* hande used for remote communication */
static MmRpc_Handle Mx_rpcIpu = NULL;

static int engines = 0;
static int codecs = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int count = 0;


/**
 * Allocate a memory block that can be passed as an argument to any of the
 * CE functions.
 */
void * dce_alloc(int sz)
{
	MEMPLUGIN_ERRORTYPE eMemError = MEMPLUGIN_ERROR_NONE;

	uint32_t nSize = (sz + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE - 1);
    		MEMPLUGIN_BUFFER_PARAMS_INIT(newbuffer_params);
		newbuffer_params.nWidth = nSize;
		newbuffer_params.bMap =TRUE;

		eMemError = MemPlugin_Alloc(pMemPluginHandle,nMemmgrClientDesc,&newbuffer_params,&newbuffer_prop);
		if(eMemError != MEMPLUGIN_ERROR_NONE)
		{
			ALOGE("Allocation failed %d",eMemError);
			return NULL;
		}

    return (newbuffer_prop.sBuffer_accessor.pBufferMappedAddress);
}

/**
 * Free a block allocated by dce_alloc()
 */
void dce_free(void *ptr)
{
    if (ptr) {
        
      MemPlugin_Free(pMemPluginHandle, nMemmgrClientDesc,&newbuffer_params,&newbuffer_prop);
    }
}

static int dce_exec(MmRpc_FxnCtx *msg, int32_t *rtnMsg)
{
    int _e = 0;
    pthread_mutex_lock(&mutex);

    _e = MmRpc_call(Mx_rpcIpu, msg, rtnMsg);
    if (_e < 0) {
        ERROR("failed MmRpc_call: %08x", _e);
        goto out;
    }

out:
    pthread_mutex_unlock(&mutex);
    return _e;
}

static UInt32 idx_Engine_open;
Engine_Handle Engine_open(String name, Engine_Attrs *attrs, Engine_Error *ec)
{
    int err;
    Engine_Handle ret = NULL;
    char msgbuf[512];
    char rplybuf[512];
    MmRpc_FxnCtx *fxnCtx = (MmRpc_FxnCtx *) msgbuf;
    int32_t *fxnRet = (int32_t *) rplybuf;

    String param_name = NULL;
    Engine_Error *param_ec = NULL;

    err = init();
    if (err < 0) {
        ERROR("fail: %08x", err);
        goto out;
    }
    DEBUG(">> name=%s, attrs=%p", name, attrs);

    param_name = dce_alloc(strlen(name) + 1);
    if (!param_name) {
        ERROR("dce_alloc fail");
        goto out;
    }

    param_ec = dce_alloc(sizeof(param_ec));
    if (!param_ec) {
        ERROR("dce_alloc fail");
        goto out;
    }

    /* marshall function arguments into the send buffer */
    strcpy(param_name, name);
    param_name[strlen(name)] = '\0';

    fxnCtx->fxn_id = idx_Engine_open;
    fxnCtx->num_params = 3;
    fxnCtx->params[0].type = MmRpc_ParamType_Ptr;
    fxnCtx->params[0].param.ptr.size = strlen(name) + 1;
    fxnCtx->params[0].param.ptr.addr = (size_t)param_name;
    fxnCtx->params[0].param.ptr.handle = NULL;
    fxnCtx->params[1].type = MmRpc_ParamType_Scalar;
    fxnCtx->params[1].param.scalar.size = sizeof(Uint32);
    fxnCtx->params[1].param.scalar.data = (size_t)NULL;
    fxnCtx->params[2].type = MmRpc_ParamType_Ptr;
    fxnCtx->params[2].param.ptr.size = sizeof(Engine_Error);
    fxnCtx->params[2].param.ptr.addr = (size_t)param_ec;
    fxnCtx->params[2].param.ptr.handle = NULL;
    fxnCtx->num_xlts = 0;
    fxnCtx->xltAry = NULL;

    err = dce_exec(fxnCtx, fxnRet);
    if (err < 0) {
        ERROR("fail err: %08x", err);
        deinit();
        goto out;
    } else if (fxnRet == NULL) {
        ERROR("fail: handle %08x", (uint32_t) fxnRet);
        deinit();
        goto out;
    }

    if (ec) {
        *ec = *param_ec;
    }

    ret = (Engine_Handle) fxnRet;

    if (ret) {
        engines++;
    }
    else {
        deinit();
        ERROR("fail: engine %08x", (unsigned int)ret);
        goto out;
    }

    DEBUG("<< engine=%p, ec=%d", ret, *param_ec);

out:
    if (param_name)
        dce_free(param_name);
    if (param_ec)
        dce_free(param_ec);
    return ret;
}

/*
 * Engine_close:
 */

static UInt32 idx_Engine_close;
Void Engine_close(Engine_Handle engine)
{
    int err;
    char msgbuf[512];
    char rplybuf[512];
    MmRpc_FxnCtx *fxnCtx = (MmRpc_FxnCtx *) msgbuf;
    int32_t *fxnRet = (int32_t *) rplybuf;

    DEBUG(">> engine=%p", engine);

    /* marshall function arguments into the send buffer */
    fxnCtx->fxn_id = idx_Engine_close;
    fxnCtx->num_params = 1;
    fxnCtx->params[0].type = MmRpc_ParamType_Scalar;
    fxnCtx->params[0].param.scalar.size = sizeof(Engine_Handle);
    fxnCtx->params[0].param.scalar.data = (size_t)engine;
    fxnCtx->num_xlts = 0;
    fxnCtx->xltAry = NULL;

    err = dce_exec(fxnCtx, fxnRet);
    if (err < 0) {
        ERROR("fail err: %08x", err);
        goto out;
    }

    engines--;

    DEBUG("<<");

out:
    deinit();
}

/*
 * VIDDEC3_create
 */

static UInt32 idx_VIDDEC3_create;
VIDDEC3_Handle VIDDEC3_create(Engine_Handle engine, String name,
        VIDDEC3_Params *params)
{
    int err;
    VIDDEC3_Handle ret=0;
    char msgbuf[512];
    char rplybuf[512];
    MmRpc_FxnCtx *fxnCtx = (MmRpc_FxnCtx *) msgbuf;
    int32_t *fxnRet = (int32_t *) rplybuf;
    String param_name = NULL;

    DEBUG(">> engine=%p, name=%s, params=%p", engine, name, params);

    param_name = dce_alloc(strlen(name) + 1);
    if (!param_name) {
        ERROR("dce_alloc fail");
        goto out;
    }

    /* marshall function arguments into the send buffer */
    strcpy(param_name, name);
    param_name[strlen(name)] = '\0';

    fxnCtx->fxn_id = idx_VIDDEC3_create;
    fxnCtx->num_params = 3;
    fxnCtx->params[0].type = MmRpc_ParamType_Scalar;
    fxnCtx->params[0].param.scalar.size = sizeof(Engine_Handle);
    fxnCtx->params[0].param.scalar.data = (size_t)engine;
    fxnCtx->params[1].type = MmRpc_ParamType_Ptr;
    fxnCtx->params[1].param.ptr.size = strlen(name) + 1;
    fxnCtx->params[1].param.ptr.addr = (size_t)param_name;
    fxnCtx->params[1].param.ptr.handle = (size_t)param_name;
    fxnCtx->params[2].type = MmRpc_ParamType_Ptr;
    fxnCtx->params[2].param.ptr.size = sizeof(VIDDEC3_Params);
    fxnCtx->params[2].param.ptr.addr = (size_t)params;
    fxnCtx->params[2].param.ptr.handle = (size_t)params;
    fxnCtx->num_xlts = 0;
    fxnCtx->xltAry = NULL;

    err = dce_exec(fxnCtx, fxnRet);
    if (err < 0) {
        ERROR("fail err: %08x", err);
        goto out;
    }

    ret = (VIDDEC3_Handle) fxnRet;

    if (ret) {
        codecs++;
    }

    DEBUG("<< codec=%p", ret);

out:
    return ret;
}

/*
 * VIDDEC3_control
 */

static UInt32 idx_VIDDEC3_control;
XDAS_Int32 VIDDEC3_control(VIDDEC3_Handle codec, VIDDEC3_Cmd id,
        VIDDEC3_DynamicParams *dynParams, VIDDEC3_Status *status)
{
    int err;
    XDAS_Int32 ret=0;
    char msgbuf[512];
    char rplybuf[512];
    MmRpc_FxnCtx *fxnCtx = (MmRpc_FxnCtx *) msgbuf;
    int32_t *fxnRet = (int32_t *) rplybuf;

    DEBUG(">> codec=%p, id=%d, dynParams=%p, status=%p",
            codec, id, dynParams, status);

    /* marshall function arguments into the send buffer */
    fxnCtx->fxn_id = idx_VIDDEC3_control;
    fxnCtx->num_params = 4;
    fxnCtx->params[0].type = MmRpc_ParamType_Scalar;
    fxnCtx->params[0].param.scalar.size = sizeof(VIDDEC3_Handle);
    fxnCtx->params[0].param.scalar.data = (size_t)codec;
    fxnCtx->params[1].type = MmRpc_ParamType_Scalar;
    fxnCtx->params[1].param.scalar.size = sizeof(VIDDEC3_Cmd);
    fxnCtx->params[1].param.scalar.data = (size_t)id;
    fxnCtx->params[2].type = MmRpc_ParamType_Ptr;
    fxnCtx->params[2].param.ptr.size = sizeof(VIDDEC3_DynamicParams);
    fxnCtx->params[2].param.ptr.addr = (size_t)dynParams;
    fxnCtx->params[2].param.ptr.handle = (size_t)dynParams;
    fxnCtx->params[3].type = MmRpc_ParamType_Ptr;
    fxnCtx->params[3].param.ptr.size = sizeof(VIDDEC3_Status);
    fxnCtx->params[3].param.ptr.addr = (size_t)status;
    fxnCtx->params[3].param.ptr.handle = (size_t)status;
    fxnCtx->num_xlts = 0;
    fxnCtx->xltAry = NULL;

    err = dce_exec(fxnCtx, fxnRet);
    if (err < 0) {
        ERROR("fail err: %08x", err);
        goto out;
    }

    ret = (XDAS_Int32) (*fxnRet);

    DEBUG("<< ret=%d", ret);

out:
    return ret;
}

/*
 * VIDDEC3_process
 */

static UInt32 idx_VIDDEC3_process;
XDAS_Int32 VIDDEC3_process(VIDDEC3_Handle codec,
        XDM2_BufDesc *inBufs, XDM2_BufDesc *outBufs,
        VIDDEC3_InArgs *inArgs, VIDDEC3_OutArgs *outArgs)
{
    int err, i;
    XDAS_Int32 ret=0;
    char msgbuf[512];
    char rplybuf[512];
    MmRpc_FxnCtx *fxnCtx = (MmRpc_FxnCtx *) msgbuf;
    int32_t *fxnRet = (int32_t *) rplybuf;

    DEBUG(">> codec=%p, inBufs=%p, outBufs=%p, inArgs=%p, outArgs=%p",
            codec, inBufs, outBufs, inArgs, outArgs);

    /* marshall function arguments into the send buffer */
    fxnCtx->fxn_id = idx_VIDDEC3_process;
    fxnCtx->num_params = 3;
    fxnCtx->params[0].type = MmRpc_ParamType_Scalar;
    fxnCtx->params[0].param.scalar.size = sizeof(VIDDEC3_Handle);
    fxnCtx->params[0].param.scalar.data = (size_t)codec;
    fxnCtx->params[1].type = MmRpc_ParamType_Ptr;
    fxnCtx->params[1].param.ptr.size = sizeof(VIDDEC3_InArgs);
    fxnCtx->params[1].param.ptr.addr = (size_t)inArgs;
    fxnCtx->params[1].param.ptr.handle = NULL;
    fxnCtx->params[2].type = MmRpc_ParamType_Ptr;
    fxnCtx->params[2].param.ptr.size = sizeof(VIDDEC3_OutArgs);
    fxnCtx->params[2].param.ptr.addr = (size_t)outArgs;
    fxnCtx->params[2].param.ptr.handle = NULL;
    fxnCtx->num_xlts = 0;
    //For inBuf - index = 0
    for (i = 0; i < inBufs->numBufs; i++) {
        fxnCtx->xltAry[fxnCtx->num_xlts].index = 0;
        fxnCtx->xltAry[fxnCtx->num_xlts].offset = MmRpc_OFFSET((Uint32) inBufs, (Uint32)&(inBufs->descs[i].buf));
        fxnCtx->xltAry[fxnCtx->num_xlts].base = (size_t)inBufs;
        fxnCtx->xltAry[fxnCtx->num_xlts].handle = NULL;
        fxnCtx->num_xlts++;
    }
    // For outBuf - index = 1
    for (i = 0; i < outBufs->numBufs; i++) {
        fxnCtx->xltAry[fxnCtx->num_xlts].index = 1;
        fxnCtx->xltAry[fxnCtx->num_xlts].offset = MmRpc_OFFSET((Uint32) outBufs, (Uint32)&(outBufs->descs[i].buf));
        fxnCtx->xltAry[fxnCtx->num_xlts].base = (size_t)outBufs;
        fxnCtx->xltAry[fxnCtx->num_xlts].handle = NULL;
        fxnCtx->num_xlts++;
        fxnCtx->xltAry = NULL;
    }

    err = dce_exec(fxnCtx, fxnRet);
    if (err < 0) {
        ERROR("fail: %08x", err);
        goto out;
    }

    ret = (XDAS_Int32) fxnRet;

    DEBUG("<< ret=%d", ret);

out:
    return ret;
}

/*
 * VIDDEC3_delete
 */

static UInt32 idx_VIDDEC3_delete;
Void VIDDEC3_delete(VIDDEC3_Handle codec)
{
    int err;
    char msgbuf[512];
    char rplybuf[512];
    MmRpc_FxnCtx *fxnCtx = (MmRpc_FxnCtx *) msgbuf;
    int32_t *fxnRet = (int32_t *) rplybuf;

    DEBUG(">> codec=%p", codec);

    fxnCtx->fxn_id = idx_VIDDEC3_delete;
    fxnCtx->num_params = 1;
    fxnCtx->params[0].type = MmRpc_ParamType_Scalar;
    fxnCtx->params[0].param.scalar.size = sizeof(VIDDEC3_Handle);
    fxnCtx->params[0].param.scalar.data = (size_t)codec;
    fxnCtx->num_xlts = 0;

    err = dce_exec(fxnCtx, fxnRet);
    if (err < 0) {
        ERROR("fail: %08x", err);
        goto out;
    }

    codecs--;

    DEBUG("<<");

out:
    return;
}

enum {
    FXN_IDX_ENGINE_OPEN = 0,
    FXN_IDX_ENGINE_CLOSE,
    FXN_IDX_VIDDEC3_CREATE,
    FXN_IDX_VIDDEC3_CONTROL,
    FXN_IDX_VIDDEC3_PROCESS,
    FXN_IDX_VIDDEC3_DELETE,
    FXN_IDX_MAX
};

/*
 * Startup/Shutdown
 */

static void dce_init(void)
{
    /* Local Function Registration for MmRpc server */
    idx_Engine_open = FXN_IDX_ENGINE_OPEN;
    idx_Engine_close = FXN_IDX_ENGINE_CLOSE;
    idx_VIDDEC3_create = FXN_IDX_VIDDEC3_CREATE;
    idx_VIDDEC3_control = FXN_IDX_VIDDEC3_CONTROL;
    idx_VIDDEC3_process = FXN_IDX_VIDDEC3_PROCESS;
    idx_VIDDEC3_delete = FXN_IDX_VIDDEC3_DELETE;

    DEBUG(SERVER_NAME " running");

if (!pMemPluginHandle) {
       int eMemError = MemPlugin_Init("MEMPLUGIN_ION",&pMemPluginHandle);
       if(eMemError != MEMPLUGIN_ERROR_NONE)
       {
               ALOGE("MEMPLUGIN configure step failed");
               return -1;
       }

       eMemError = MemPlugin_Open(pMemPluginHandle,&nMemmgrClientDesc);
       if(eMemError != MEMPLUGIN_ERROR_NONE)
       {
               ALOGE("Mem manager client creation failed!!!");
               return -1;
       }
}

}

static void dce_deinit(void)
{
    DEBUG("shutdown - dce_deinit");
}

#define DCE_DEVICE_NAME "rpmsg-dce0"

static int init(void)
{
    int err = 0;
    MmRpc_Params args;

    pthread_mutex_lock(&mutex);

    count++;
    if (count > 1) {
        ALOGE("dce_init() ->count= %08x", count);
        goto out;
    }

    /* create remote server insance */
    MmRpc_Params_init(&args);

//int MmRpc_create(const char *service, const MmRpc_Params *params,MmRpc_Handle *handlPtr)
    err = MmRpc_create("/dev/"DCE_DEVICE_NAME, &args, &Mx_rpcIpu);
    if (err < 0) {
        ERROR("MmRpc_create failed, cannot create rpmsg-dce0 device %d", errno);
        err = -1;
        count--;
        goto out;
    }
    else {
        err = 0;
    }

    DEBUG("open(/dev/" DCE_DEVICE_NAME ") -> %08x", Mx_rpcIpu);
    dce_init();
out:
    DEBUG("dce_init() -> %08x", err);
    pthread_mutex_unlock(&mutex);
    return err;
}

static void deinit(void)
{
    pthread_mutex_lock(&mutex);

    count--;

    if (count > 0) {
        goto out;
    }
    dce_deinit();
   // DEBUG("dce_deinit() MmRpcIpu -> %08x", MmRpcIpu);

    if (Mx_rpcIpu != NULL)
        MmRpc_delete(&Mx_rpcIpu);
    Mx_rpcIpu = NULL;
out:
    pthread_mutex_unlock(&mutex);
}
