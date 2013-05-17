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
/*
 *  ======== Server.h ========
 *  DSP Server module
 *
 *  APIs for accessing information from remote server.
 */

/**
 *  @file       ti/sdo/ce/Server.h
 *
 *  @brief      The Codec Engine Server Interface.  Provides the user an
 *              inteface to open and manipulate a Server which contains
 *              remote algorithms.
 */
/**
 *  @addtogroup   ti_sdo_ce_Server     Codec Engine Server Interface
 */
#ifndef ti_sdo_ce_Server_
#define ti_sdo_ce_Server_

#ifdef __cplusplus
extern "C" {
#endif


#include <ti/sdo/ce/ServerDefs.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/skel.h>

#include <stdio.h>  /* def of FILE * */

/** @ingroup    ti_sdo_ce_Server */
/*@{*/

/**
 * @brief       Name to pass to Diags_setMask() to enable logging for Server
 *              functions. For example,
 *                  Diags_setMask(Server_MODNAME"+EX1234567");
 *              turns on all Log statements in this module.
 *              Diags_setMask() must be called after initialization to take
 *              effect.
 */
#define Server_MODNAME "ti.sdo.ce.Server"

/**
 *  @brief      Maximum number of characters used in memory segment names.
 */
#define Server_MAXSEGNAMELENGTH 32

/**
 *  @brief      Server error code
 */
typedef enum Server_Status {
    Server_EOK       = 0,      /**< Success. */
    Server_ENOSERVER = 1,      /**< Engine has no server. */
    Server_ENOMEM    = 2,      /**< Unable to allocate memory. */
    Server_ERUNTIME  = 3,      /**< Internal runtime failure. */
    Server_EINVAL    = 4,      /**< Bad value passed to function. */
    Server_EWRONGSTATE =5,     /**< Server is not in the correct state to
                                *   execute the requested function. */
    Server_EINUSE    = 6,      /**< Server call did not succeed because a
                                *   because a required resource is in use. */
    Server_ENOTFOUND = 7,      /**< An entity was not found */
    Server_EFAIL     = 8,      /**< Unknown failure */
    Server_ENOTSUPPORTED = 9   /**< API not supported for given parameters. */
} Server_Status;

/*
 *  ======== Server_AlgDesc ========
 */
typedef struct Server_AlgDesc {
    /**
     *  @brief  The name of the algorithm. This is used by the application
     *          when instantiating an instance of the algorithm through one
     *          of the VISA APIs.
     */
    String      name;

    /**
     *  @brief  The address of the XDAIS alg function table.
     *
     *  @remarks
     *          All XDAIS algorithms must define an IALG_Fxns structure that
     *          contains implementations of the IALG methods.  This field
     *          is simply the address of this structure.
     */
    IALG_Fxns   *fxns;

    /**
     *  @brief  The address of the IDMA3_Fxns function table, if the algorithm
     *          uses DMA. If the algorithm does not use DMA, this field should
     *          set to NULL. Valid for local algorithm only.
     */
    Ptr         idmaFxns;

    /**
     *  @brief  If true, the algorithm will be instantiated on the
     *          "local" CPU.  Otherwise the server will create an
     *           instance of the algorithm.
     */
    Bool        isLocal;

    /**
     *  @brief  This id specifies which resource sharing group that this
     *          alg will be placed into.
     *
     *  @remarks
     *          This 'group' concept is used by the framework for sharing
     *          resources. Algorithms in the same group share resources, and
     *          therefore, must not run at the same time. If you assign the
     *          same groupId to multiple algorithms, these algorithms must
     *          not pre-empt eachother, or the shared resources may be
     *          corrupted.
     *          When server algorithms are configured statically in a .cfg
     *          file, if the @c groupId parameter for the algorithm has
     *          not been set, the configuration process assigns the @c groupId
     *          will be assigned automatically, based on the priority that
     *          the algorithm will run at.  However, when Server_addAlg() is
     *          used to dynamically add the algorithm to the server, you must
     *          ensure that the @c groupId is appropriately. If two algorithms
     *          will run at the same time, you must assign them different
     *          group Ids.
     *
     *          Algorithms in different groups do not share resources.
     *
     *  @sa http://processors.wiki.ti.com/index.php/Codec_Engine_GroupIds
     */
    Int         groupId;

    /**
     *  @brief  Address of the XDAIS alg IRES Interface function table.
     *
     *  @remarks
     *          All XDAIS algorithms that use an IRES resource must define an
     *          IRES_Fxns structure containing the pointers to functions
     *          implementatng the IRES interface.
     *          If the algorithm does not use an IRES resource this field
     *          should be set to NULL.
     */
    Ptr         iresFxns;

    /*
     *  Currently not used.
     *           Codec class configuration data for stub side, if any.  We
     *           generate this structure for both sides, although it is
     *           currently only used in the skeletons.
     */
    Void        *stubsCodecClassConfig;

    /**
     *  @brief  Codec class configuration data, if any.
     */
    Void        *codecClassConfig;

    /*
     *  Currently not used.
     *          Indicates the type of memory the alg's memory requests will
     *          be allocated from.
     *          The alg's memory will be allocated from cached memory, if
     *              memType = Engine_USECACHEDMEM_CACHED,
     *          from non-cached memory, if
     *              memType = Engine_USECACHEDMEM_NONCACHED,
     *          Otherwise, if
     *              memType = Engine_USECACHEDMEM_DEFAULT,
     *          memory allocations will be determined by the value of
     *          ti_sdo_ce_alg_Algorithm_useCache (cached, if TRUE, non-cached,
     *          if FALSE).
     *
     *  @sa  Engine_CachedMemType
     */
    Engine_CachedMemType memType;  /**< Memory type for alg's mem reqs. */

    /**
     *  @brief  A string idicating the type(s) of algorithm this is.
     *          This should be a ';' separated string of inherited types.
     *
     *  @remarks
     *          In most cases, @c types will just be set to the VISA type
     *          defined in the Codec Engine algorithm interface header
     *          file included by the algorithm, depending on the XDM interface
     *          the algorithm implements.
     *
     *          For example, if the algorithm implements the ISPHDEC1
     *          interface as defined by XDM, @c types should be set
     *          to
     *              @c SPHDEC1_VISATYPE
     *          (defined as "ti.sdo.ce.speech1.ISPHDEC1" in the header file
     *          ti/sdo/ce/speech1/sphdec1.h).
     *
     *          Another example to illustrate multiple typss specified in
     *          @c typss, if the algorithm implements the (made-up)
     *          interface, ti.sdo.ce.test.xvideo.IVIDE, which in turn
     *          implements the IVIDDEC interface, we could then set @c types
     *          to
     *              VIDDEC_VISATYPE";ti.sdo.ce.test.xvideo.IVIDE"
     *          or
     *              "ti.sdo.ce.test.xvideo.IVIDE;"VIDDEC_VISATYPE
     */
    String      types;

    /**
     *  @brief  A string idicating the name of the stub functions.  This is
     *  needed by remote apps that call Engine_initFromServer().
     */
    String      stubFxnsName;

    /**
     *  @brief  The skel functions needed to invoke the alg remotely
     */
    SKEL_Fxns   *skelFxns;

    /**
     *  @brief  The priority the alg will run at.
     */
    Int         priority;

    /**
     *  @brief  Algorithm stack size.
     */
    Int         stackSize;

    /*
     *  Currently not used.
     *          Memory heap for algorithm stack.
     */
    Int         stackSeg;
} Server_AlgDesc;

/**
 *  @brief      Information for a memory heap of a remote DSP server.
 *
 *  @remarks    Sizes are given in DSP data MAUs.
 *  @sa         Server_getMemStat().
 *
 */
typedef struct Server_MemStat {
    Char   name[Server_MAXSEGNAMELENGTH + 1]; /**< Name of memory heap. */
    Uint32 base;           /**< Base address of the memory segment. */
    Uint32 size;           /**< Original size of the memory segment. */
    Uint32 used;           /**< Number of bytes used. */
    Uint32 maxBlockLen;    /**< Length of the largest contiguous free block. */
} Server_MemStat;


/*
 *  ======== Server_addAlg ========
 */
/**
 *  @brief      Dynamically add an algorithm to a Server.
 *
 *  @param[in]  server          The handle of a server returned by
 *                              Engine_getServer().  Set to NULL when adding
 *                              a local algorithm to the server. In the
 *                              future, this handle will be used to
 *                              dynamically add algorithms to a remote server.
 *  @param[in]  location        String identifying the location of the
 *                              algorithm.  Often this is a file name, but for
 *                              systems without a file system, it may be a
 *                              system-specific string identifier. This may
 *                              be NULL if the algorithm is built into the
 *                              executable. Currently not supported - set to
 *                              NULL.
 *  @param[in]  pAlgDesc        Parameters describing the algorithm being
 *                              added. Before setting the fields of this
 *                              structure, it should first be initialized
 *                              with @c Server_initAlgDesc(), to set all
 *                              fields to default values.
 *
 *                              The user must set the following fields of
 *                              pAlgDesc:
 *                                  pAlgDesc->name
 *                                  pAlgDesc->fxns
 *                                  pAlgDesc->idmaFxns, if applicable
 *                                  pAlgDesc->iresFxns, if applicable
 *
 *                                  pAlgDesc->groupId
 *                                  pAlgDesc->priority
 *                                  pAlgDesc->stackSize
 *
 *                                  pAlgDesc->types
 *                                  pAlgDesc->stubFxnsName
 *                                  pAlgDesc->skelFxns
 *
 *                              Currently, adding only local algorithms is
 *                              supported, so the default value of TRUE can
 *                              be used for:
 *                                  pAlgDesc->isLocal
 *
 *  @pre        As with all Codec Server API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @remarks    If adding a local algorithm to a server that is built with
 *              BIOS, this function must be called after CERuntime_init()
 *              has been called, but before BIOS_start(). This is necessary
 *              to ensure that the algorithm will be visible to the remote
 *              app that loaded the server.
 *
 *  @remarks    If adding a remote algorithm to a remote server, the server
 *              handle for the opened Engine must be used.  The server handle
 *              is obtained by calling Engine_getServer() with the handle of
 *              the opened engine.
 *              In this case, the added algorithm will only be accessible to
 *              the caller of this function.
 *              Adding a remote algorithm is not yet supported.
 *
 *  @remarks    If there is an existing algorithm in the server already named
 *              @c name, an error will be returned.
 *
 *  @retval     Server_EOK       Success.
 *  @retval     Server_EINVAL    @c pAlgDesc or @c pAlgDesc->name is NULL.
 *  @retval     Server_EINUSE    The name of the alg in @c pAlgDesc->name is
 *                               already in use.
 *  @retval     Server_ENOTSUPPORTED  @c pAlgDesc->isLocal = FALSE is currently
 *                               not supported.
 *
 *  @par Example Usage:
 *  @code
 *      #include <ti/sdo/ce/Server.h>
 *
 *      Server_AlgDesc desc;
 *
 *      Server_initAlgDesc(&desc);
 *
 *      desc.groupId = 2;
 *      desc.isLocal = TRUE;
 *      desc.fxns = &UNIVERSALCOPY_TI_IUNIVERSALCOPY;
 *      desc.idmaFxns = NULL;
 *      desc.iresFxns = NULL;
 *      desc.priority = 2;
 *      desc.stackSize = 0x2000;
 *      desc.types = UNIVERSAL_VISATYPE;
 *      desc.stubFxnsName = "UNIVERSAL_STUBS";
 *      desc.skelFxns = &UNIVERSAL_SKEL;
 *
 *      status = Server_addAlg(NULL, NULL, &desc);
 *
 *  @endcode
 *
 *  @sa         Server_AlgDesc
 *  @sa         Server_initAlgDesc()
 *  @sa         Engine_getServer()
 */
extern Server_Status Server_addAlg(Server_Handle server, String location,
        Server_AlgDesc *pAlgDesc);


/*
 *  ======== Server_connectTrace ========
 */
/**
 *  @brief                      Connect to server for purposes of collecting
 *                              trace and/or LOG data.
 *
 *  @param[in]  server          Server handle obtained from Engine_getServer().
 *
 *  @param[in]  token           Address to store connection token.  This token
 *                              should be specified in the companion call to
 *                              Server_disconnectTrace().
 *
 *  @retval     Server_EOK       Success, trace token was acquired.
 *  @retval     Server_EINUSE    A connection for server trace is already
 *                               established.
 *  @retval     Server_ERUNTIME  An internal runtime error occurred.
 *
 *  @pre        @c server is non-NULL.
 *  @pre        @c token is non-NULL.
 *
 *  @sa         Server_disconnectTrace().
 *
 */
extern Server_Status Server_connectTrace(Server_Handle server, Int * token);

/*
 *  ======== Server_disconnectTrace ========
 */
/**
 *  @brief                      Disconnect from server when finished collecting
 *                              trace and/or LOG data.
 *
 *  @param[in]  server          Server handle obtained from Engine_getServer().
 *
 *  @param[in]  token           Connection token (as obtained from earlier,
 *                              companion call to Server_connectTrace()).
 *
 *  @retval     Server_EOK       Success.
 *  @retval     Server_ERUNTIME  An internal runtime error occurred.
 *
 *  @pre        @c server is non-NULL.
 *
 *  @sa         Server_connectTrace().
 *
 */
extern Server_Status Server_disconnectTrace(Server_Handle server, Int token);

/*
 *  ======== Server_fwriteTrace ========
 */
/**
 *  @brief              Write Server's trace buffer to specifed file stream
 *
 *  @param[in]  server  Server handle, obtained from Engine_getServer().
 *
 *  @param[in]  prefix  A string to prepend to each line output; this
 *                      allows one to easily identify trace from the
 *                      server from the application's trace, for
 *                      example.
 *  @param[in]  out     An open FILE stream used to output the
 *                      Server's trace characters.
 *
 *  @retval             Integer number of characters copied to the specified
 *                      FILE stream.
 *
 *  @pre             @c server is non-NULL.
 *  @pre             @c Corresponding engine is in the open state.
 *
 *  @post       In the event a negative value is returned,
 *              Engine_getLastError() will return the value:
 *                - #Engine_ERUNTIME    Either an internal runtime error
 *                                      occured or the underlying server
 *                                      error occured.
 */
extern Int Server_fwriteTrace(Server_Handle server, String prefix, FILE *out);

/*
 *  ======== Server_getCpuLoad ========
 */
/**
 *  @brief                      Get Server's CPU usage in percent.
 *
 *  @param[in]  server          Server handle, obtained from Engine_getServer().
 *
 *  @retval                     Integer between 0-100 indicating percentage
 *                              of time the Server is processing measured
 *                              over a period of approximately 1 second.  If
 *                              the load is unavailable, a negative value is
 *                              returned.
 *
 *  @pre        @c server is non-NULL.
 *
 *  @post       In the event a negative value is returned,
 *              Engine_getLastError() will return one of the following values:
 *                - #Engine_ERUNTIME    Either an internal runtime error
 *                                      occured or an underlying server
 *                                      error occured.
 *                - #Engine_EINVAL      The Server handle is not valid.
 *
 */
extern Int Server_getCpuLoad(Server_Handle server);

/*
 *  ======== Server_getMemStat ========
 */
/**
 *  @brief Get information on a memory heap segment of a remote DSP server.
 *
 *  @param[in]  server          Server handle obtained from Engine_getServer().
 *  @param[in]  segNum          The heap number of a segment on the DSP.
 *  @param[out] memStat         Structure to store memory segment information.
 *
 *  @retval     Server_EOK       Success.
 *  @retval     Server_ENOTFOUND @c segNum is out of range.
 *  @retval     Server_ERUNTIME  Internal runtime error occurred.
 *
 *  @pre        @c server is non-NULL.
 *  @pre        @c memStat is non-NULL.
 *
 *  @post       On success, memStat will contain information about the memory
 *              heap @c segNum on the DSP.
 *
 *  @sa         Server_getNumMemSegs().
 *
 *  @remarks    This API only returns statistics for BIOS HeapMem heaps
 *              that have been statically configured into the server.
 */
extern Server_Status Server_getMemStat(Server_Handle server, Int segNum,
        Server_MemStat *memStat);

/*
 *  ======== Server_getNumMemSegs ========
 */
/**
 *  @brief Get the number of memory heap segments of a remote DSP server.
 *
 *  @param[in]  server          Server handle obtained from Engine_getServer().
 *  @param[out] numSegs         The number of heap segments of the DSP server.
 *
 *  @retval     Server_EOK        Success.
 *  @retval     Server_ERUNTIME   Internal runtime error occurred.
 *  @retval     Server_ENOSERVER  Engine has no server.
 *
 *  @pre        @c server is non-NULL.
 *  @pre        @c numSegs is non-NULL.
 *
 *  @post       On success, numSegs will contain the number of memory heaps
 *              on the DSP.
 *
 *  @sa         Server_getMemStat().
 *
 *  @remarks    This API returns only the number of BIOS HeapMem heaps that
 *              have been statically configured into the server.
 */
extern Server_Status Server_getNumMemSegs(Server_Handle server, Int *numSegs);

/** @cond INTERNAL */
/*
 *  ======== Server_init ========
 */
extern Void Server_init(Void);
/** @endcond */

/*
 *  ======== Server_initAlgDesc ========
 */
/**
 *  @brief      Initialize an Server_AlgDesc structure with default values.
 *
 *  @param[in]  pAlgDesc  Location of Server_AlgDesc object to initialize.
 *                        The fields of pAlgDesc will be set to the following:
 *  @code
 *                        pAlgDesc->name = NULL;
 *                        pAlgDesc->uuid.data = 0;
 *                        pAlgDesc->fxns = NULL;
 *                        pAlgDesc->idmaFxns = NULL;
 *                        pAlgDesc->typeTab = NULL;
 *                        pAlgDesc->isLocal = TRUE;
 *                        pAlgDesc->groupId = 0;
 *                        pAlgDesc->iresFxns = NULL;
 *                        pAlgDesc->codecClassConfig = NULL;
 *                        pAlgDesc->priority = 1;
 *                        pAlgDesc->stackSize = 1024;
 *                        pAlgDesc->types = NULL;
 *                        pAlgDesc->stubFxnsName = NULL;
 *                        pAlgDesc->skelFxns = NULL;
 *
 *                        Unused fields below are initialized to the
 *                        following:
 *
 *                        pAlgDesc->rpcProtocolVersion = 0;
 *                        pAlgDesc->memType = Engine_USECACHEDMEM_DEFAULT;
 *                        pAlgDesc->stackSeg = 0;
 *
 *  @endcode
 *
 *  @sa         Server_addAlg()
 */
extern Void Server_initAlgDesc(Server_AlgDesc *pAlgDesc);

/*
 *  ======== Server_redefineHeap ========
 */
/**
 *  @brief Set the base address and size of a remote DSP server heap.
 *
 *  @remarks
 *  This API is used to move and/or resize a named heap of the remote DSP
 *  server.  The address passed to this API is a DSP address and the
 *  memory from @c base to @c base + @c size must be contiguous in physical
 *  memory. The size of the heap should be given in DSP MADUs (minimum
 *  addressable data units).
 *  The name of the heap can be at most #Server_MAXSEGNAMELENGTH
 *  characters long.
 *
 *  For example, in the case of DM644x, suppose that an application wants
 *  to allocate a block of memory on the GPP to be used by the DSP server
 *  for the memory segment named "DDRALGHEAP".  A block of physically
 *  contiguous memory could be obtained by Memory_alloc() and the
 *  corresponding DSP address obtained with
 *  Memory_getBufferPhysicalAddress().  This DSP address and the size
 *  of the block could then be passed to Server_redefineHeap().
 *  For example:
 *
 *  @code
 *      Server_redefineHeap(server, "DDRALGHEAP", base, size);
 *  @endcode
 *
 *  This function can only be called when there is no memory currently
 *  allocated in the heap (since the heap cannot be changed if it is being
 *  used).
 *
 *  @param[in]  server          Server handle obtained from Engine_getServer().
 *  @param[in]  name            Name of heap to be redefined.
 *  @param[in]  base            Base address for algorithm heap.
 *  @param[in]  size            Size (DSP MADUs) of new heap.
 *
 *  @retval     Server_EOK         Success.
 *  @retval     Server_ERUNTIME    Internal runtime error occurred.
 *  @retval     Server_ENOTFOUND   No heap with the specified name was found.
 *  @retval     Server_EINVAL      Changing to the new base and size would
 *                                 cause an overlap with another heap.
 *  @retval     Server_EINUSE      Memory is currently allocated in the heap.
 *
 *  @pre        @c server is non-NULL.
 *  @pre        @c base is a DSP address of a physically contiguous
 *              block of memory.
 *  @pre        @c base is aligned on an 8-byte boundary.
 *
 *  @post       On success, the server's algorithm heap base will have been
 *              set to @c base, and the size will have been set to @c size.
 *
 *  @sa         Server_restoreHeap().
 *
 *  @remarks    This API is not supported in Codec Engine 3.20.
 */
extern Server_Status Server_redefineHeap(Server_Handle server, String name,
        Uint32 base, Uint32 size);

#if 0
/*
 *  ======== Server_removeAlg ========
 */
/*
 *  Currently not implemented.
 *
 *  @brief      Dynamically remove an algorithm that was added to a Server
 *              with Server_addAlg().
 *
 *  @remarks    The same values of the parameters @c name and @c server that
 *              were passed to Server_addAlg() should be used here.  In
 *              particular, if @c name was used to add the alg, all handles to
 *              the engine named @c name must be closed before calling
 *              Server_removeAlg().
 *
 *
 *  @param[in]  name            The name of the engine or NULL, that was
 *                              passed to Server_addAlg(). Set to NULL for now.
 *                              Currently, only the default "local" engine is
 *                              supported.
 *  @param[in]  server          The handle to a server, previously acquired
 *                              by a call to Engine_getServer(), or NULL.
 *                              Set to NULL for now.  In the future, this will
 *                              be used to dynamically remove remote
 *                              algorithms from a server.
 *  @param[in]  algName         Name of the algorithm to remove.
 *
 *  @retval     Server_EOK          Success.
 *  @retval     Server_ENOTFOUND    The server's underlying engine, @c name,
 *                                  does not exist.
 *  @retval     Server_ENOTFOUND    @c algName could not be found in the
 *                                  server's underlying engine table.
 *  @retval     Server_EINUSE       The server's underlying engine, @c name,
 *                                  is still open.
 *
 *  @sa         Server_addAlg()
 */
extern Server_Status Server_removeAlg(String name, Server_Handle server,
        String algName);
#endif

/*
 *  ======== Server_restoreHeap ========
 */
/**
 *  @brief Set the base address and size of a remote DSP server heap
 *         back to their original values.
 *
 *  This function resets the base address and size of a named heap of
 *  the remote server, back to their values before the first call to
 *  Server_redefineHeap() was made. The name of the heap can be at most
 *  #Server_MAXSEGNAMELENGTH characters long, otherwise this function
 *  will return #Server_ENOTFOUND.
 *
 *  As with Server_redefineHeap(), this function can only be called when
 *  no memory is currently allocated from the heap (as the heap cannot be
 *  changed if it is being used).
 *
 *  @param[in]  server      Server handle obtained through Engine_getServer().
 *  @param[in]  name        Name of the heap to be restored.
 *
 *  @retval     Server_EOK         Success.
 *  @retval     Server_ERUNTIME    Internal runtime error occurred.
 *  @retval     Server_ENOTFOUND   No heap with the specified name was found.
 *  @retval     Server_EINVAL      Changing back to the original base and size
 *                                 would cause an overlap with another heap.
 *  @retval     Server_EINUSE      Memory is currently allocated in the heap.
 *
 *  @pre        @c server is non-NULL.
 *
 *  @post       On success, the server's algorithm heap base and size will
 *              have been reset to their original value.
 *
 *  @sa         Server_redefineHeap().
 *
 *  @remarks    This API is not supported in Codec Engine 3.20.
 */
extern Server_Status Server_restoreHeap(Server_Handle server, String name);

/*
 *  ======== Server_setTrace ========
 */
/**
 *  @brief              Set Server's trace mask
 *
 *  @param[in]  server  Server handle obtained through Engine_getServer().
 *  @param[in]  mask    Trace mask, e.g. "*=01234567"
 *
 *  @retval     Server_EOK         Success.
 *  @retval     Server_EINUSE      A connection for server trace is already
 *                                 established.
 *  @retval     Server_ENOSERVER   No server for the engine.
 *  @retval     Server_ERUNTIME    Internal runtime error occurred.
 *
 *  @pre        @c server is non-NULL.
 *  @pre        @c Corresponding engine is in the open state.
 *
 *  @remarks    This only sets the trace for a remote server.  To change
 *              the trace mask for the application-side of the framework,
 *              use Diags_setMask or Diags_setMaskMeta.
 *
 *  @sa         xdc.runtime.Diags
 */
extern Int Server_setTrace(Server_Handle server, String mask);

/*@}*/  /* ingroup */

#ifdef __cplusplus
}
#endif

#endif
/*
 *  @(#) ti.sdo.ce; 1, 0, 6,1; 8-14-2012 12:59:44; /db/atree/library/trees/ce/ce-u07/src/ xlibrary

 */

