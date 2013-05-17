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
 *  ======== Engine.h ========
 */

/**
 *  @file       ti/sdo/ce/Engine.h
 *
 *  @brief      The Codec Engine Runtime.
 *
 *  @remarks    Provides the user an interface to
 *              open and manipulate an Engine which can instantiate
 *              and communicate with XDAIS algorithms that run either
 *              on the local CPU or on a "remote" Server.
 */
/**
 *  @addtogroup   CODECENGINE     Codec Engine Runtime
 */

#ifndef Engine_
#define Engine_

#ifdef __cplusplus
extern "C" {
#endif


#include <ti/sdo/ce/ServerDefs.h>
#include <ti/sdo/ce/ipc/Comm.h>
#include <ti/xdais/ialg.h>
#include <ti/sdo/ce/node/node.h>

#include <stddef.h> /* def of size_t */
#include <stdio.h>  /* def of FILE * */

/** @ingroup    CODECENGINE */
/*@{*/

/**
 * @brief       Name to pass to Diags_setMask() to enable logging for Engine
 *              functions.
 *
 * @par Example Usage:
 *              The following code turns on all Log statements in the Engine
 *              module.
 * @code
 *     Diags_setMask(Engine_MODNAME "+EX1234567");
 * @endcode
 *
 * @remarks     Using Diags_setMask() to enable Engine Logging must be called
 *              after CERuntime_init() (which creates and initializes the
 *              Engine trace mask) to have any effect.
 */
#define Engine_MODNAME "ti.sdo.ce.Engine"

/**
 *  @brief      Opaque handle to an engine.
 */
typedef struct Engine_Obj *Engine_Handle;

/**
 *  @brief      Engine error code
 */
typedef Int Engine_Error;

#define Engine_EOK          0   /**< Success. */
#define Engine_EEXIST       1   /**< Name does not exist. */
#define Engine_ENOMEM       2   /**< Unable to allocate memory. */
#define Engine_EDSPLOAD     3   /**< Unable to load the DSP. */
#define Engine_ENOCOMM      4   /**< Unable to create a comm connection to
                                 *   the DSP.
                                 */
#define Engine_ENOSERVER    5   /**< Unable to locate the server on the DSP. */
#define Engine_ECOMALLOC    6   /**< Unable to allocate communication buffer. */
#define Engine_ERUNTIME     7   /**< Internal engine runtime failure. */
#define Engine_ECODECCREATE 8   /**< Creation of the Codec failed. */
#define Engine_ECODECSTART  9   /**< Start of the Codec failed.  For codecs
                                 *   which are implemented as a thread, this
                                 *   implies that the codec thread of execution
                                 *   failed to start.
                                 */
#define Engine_EINVAL       10  /**< Bad paramater passed to method. */
#define Engine_EBADSERVER   11  /**< Incompatible server specified. */
#define Engine_ENOTAVAIL    12  /**< Service not available. */
#define Engine_EWRONGSTATE  13  /**< Call can not be made at this time. */
#define Engine_EINUSE       14  /**< Call can't be made at this time because
                                 *   a required name/resource is in use.
                                 */
#define Engine_ENOTFOUND    15  /**< Entity was not found. */
#define Engine_ETIMEOUT     16  /**< Timeout-based operation timed out. */

/** @cond INTERNAL */

/**
 *  @brief      Opaque handle to a node.
 */
typedef struct Engine_NodeObj *Engine_Node;

/**
 *  @brief      Special value for timeout parameter of Engine_callWait()
 */
#define Engine_FOREVER Comm_FOREVER

/** @endcond */

/**
 *  @brief      Attributes of an Engine
 *
 *  @sa         Engine_initAttrs().
 *  @sa         Engine_open().
 */
typedef struct Engine_Attrs {
    String procId;  /**< id of the processor that runs the server; only
                     *   needed in the case that there's more than one
                     *   processor that can provide the same server.
                     */
} Engine_Attrs;

/**
 *  @brief      Properties of an Engine algorithm
 *
 *  @sa         Engine_getAlgInfo()
 */
typedef struct Engine_AlgInfo {
    Int         algInfoSize;    /**< Size of this structure. */
    String      name;           /**< Name of algorithm. */
    String      *typeTab;       /**< Inheritance hierarchy. */
    Bool        isLocal;        /**< If TRUE, run locally. */
} Engine_AlgInfo;

/**
 *  @brief      Properties of an Engine algorithm.
 *
 *  @remarks    This structure is identical to Engine_AlgInfo except that the
 *              @c typeTab array of strings is replaced by a single string
 *              called @c types. The string, @c types, represents a ';'
 *              separated list of inheritance hierarchies of the algorithm,
 *              for example,
 *              "ti.sdo.ce.video.IVIDDEC;ti.sdo.ce.test.xvideo.IVIDE".
 *
 *  @sa         Engine_getAlgInfo2()
 */
typedef struct Engine_AlgInfo2 {
    Int         algInfoSize;    /**< Size of this structure. */
    String      name;           /**< Name of algorithm. */
    String      types;          /**< Inheritance hierarchy. */
    Bool        isLocal;        /**< If TRUE, run locally. */
} Engine_AlgInfo2;

/**
 *  @brief      Default engine attributes.
 *
 *  @deprecated Engine_ATTRS is no longer recommended. Please use
 *              Engine_initAttrs() instead.
 *
 *  @sa Engine_initAttrs()
 */
extern Engine_Attrs Engine_ATTRS;    /**< Default attrs. */

/** @cond INTERNAL */

typedef Int Engine_Ctrl;

#define Engine_CEXIT    0
#define Engine_MAXSEGNAMELENGTH 32

/** @endcond */

/**
 *  @brief      Engine Cacheable Memory types.
 *
 *  @enumWarning
 */
typedef enum Engine_CachedMemType {
    Engine_USECACHEDMEM_DEFAULT = -1,  /**< Use default cache setting */
    Engine_USECACHEDMEM_NONCACHED = 0, /**< Use non-cached memory */
    Engine_USECACHEDMEM_CACHED = 1     /**< Use cached memory */
} Engine_CachedMemType;


/*
 *  ======== Engine_AlgDesc ========
 */
/**
 *  @brief      Descriptor for an alg. This object can be passed to
 *              @c Engine_addAlg(), to dynamically add an alg to an engine.
 *
 *  @sa         Engine_initAlgDesc()
 *  @sa         Engine_addAlg()
 */
typedef struct Engine_AlgDesc {
    /**
     *  @brief  The name of the algorithm. This is used by the application
     *          when instantiating an instance of the algorithm through one
     *          of the VISA APIs.
     */
    String      name;

    NODE_Uuid   uuid;   /**< Id of alg if running on remote target. No need
                         *   to set this field.
                         */

    /**
     *  @brief  The address of the XDAIS alg function table.
     *          All XDAIS algorithms must define an IALG_Fxns structure that
     *          contains implementations of the IALG methods.  This field
     *          is simply the address of this structure.
     */
    IALG_Fxns   *fxns;

    /**
     *  @brief  The address of the IDMA3_Fxns function table, if the algorithm
     *          uses DMA. If the algorithm does not use DMA, this field should
     *          set to NULL.
     */
    Ptr         idmaFxns;

    String      *typeTab;       /**< inheritance hierarchy - Do not modify. */

    /**
     *  @brief  If true, the algorithm will be instantiated on the
     *          "local" CPU.  Otherwise the server will create an
     *           instance of the algorithm.
     */
    Bool        isLocal;

    /**
     *  @brief  This id specifies which resource sharing group that this
     *          alg will be placed into.  This 'group' concept
     *          is used by the framework to ensure algorithms in the
     *          same group don't pre-empt each other and corrupt the
     *          shared resources.
     *          This parameter will be ignored if @c isLocal is FALSE.
     */
    Int         groupId;

    Int         rpcProtocolVersion; /**< Protocol version. Do not modify */

    /**
     *  @brief  Address of the XDAIS alg IRES Interface function table
     *          All XDAIS algorithms that use an IRES resource must define an
     *          IRES_Fxns structure containing the pointers to functions
     *          implementatng the IRES interface.
     *          If the algorithm does not use an IRES resource this field
     *          should be set to NULL.
     */
    Ptr         iresFxns;

    /**
     *  @brief  Codec class configuration data, if any.
     */
    Void        *codecClassConfig;

    /**
     *  @brief  Indicates the type of memory the alg's memory requests will
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
} Engine_AlgDesc;


/**
 *  @brief      Name of function that a dynamically loaded codec must supply.
 */
#define Engine_GETALGDESCFXN  "GetEngineAlgDesc"

/*
 *  ======== Engine_DllAlgDesc ========
 *  An alg that will be dynamically loaded must have a descriptor of this type.
 */
/**
 *  @brief      Descriptor for a dynamically loaded alg. A dynamic library
 *              for a codec must export a function that fills in a structure
 *              of this type.
 *
 *  @sa         Engine_GetAlgDescFxn
 */
typedef struct Engine_DllAlgDesc {
    /**
     *  @brief  Pointer to codec's IALG_Fxns. This can not be NULL.
     */
    IALG_Fxns   *fxns;

    /**
     *  @brief  Pointer to codec's IDMA3_Fxns table. If the codec does not
     *          use DMA, this should be NULL.
     */
    Ptr         idmaFxns;

    /**
     *  @brief  Pointer to codec's IRES_Fxns function table. This should be
     *          NULL if the codec does not implement the IRES_Fxns.
     */
    Ptr         iresFxns;

    /**
     *  @brief  Inheritance hierarchy of codec. This is a ';' separated
     *          string that lists the interfaces inherited by the code. For
     *          example:
     *
     *              "ti.sdo.ce.speech1.ISPHDEC1"
     *
     *          or, in the case where a test IVIDE interface inherits IVIDDEC:
     *
     *              "ti.sdo.ce.video.IVIDDEC;ti.sdo.ce.test.xvideo.IVIDE"
     *
     */
    String      types;

    /**
     *  @brief  codec class config data, if any.
     *
     *  @todo   Figure out what this is.
     */
    Void        *codecClassConfig;
} Engine_DllAlgDesc;


/*
 *  ======== Engine_GetAlgDescFxn ========
 *  A dynamically loaded codec library must supply a function of this type to
 *  get properties of the library's algorithm.
 */
/**
 *  @brief      Prototype of function that must be supplied by a dynamic
 *              codec library to fill in a @c Engine_DllAlgDesc structure.
 *
 *  @remarks    This function will be called by @c Engine_addAlg() to fill
 *              in the dynamic codec's descriptor.
 *
 *  @sa         Engine_DllAlgDesc
 */
typedef Int (*Engine_GetAlgDescFxn)(Engine_DllAlgDesc *dllAlgDesc);


/*
 *  ======== Engine_Desc ========
 */
/**
 *  @brief      This structure is passed to @c Engine_add(), and contains
 *              parameters to specify an engine.
 *
 *  @sa         Engine_add()
 *  @sa         Engine_open()
 */
typedef struct Engine_Desc {
    String name;            /**< Name of the Engine
                             *
                             *   @remarks   This must not be NULL
                             */
    Engine_AlgDesc *algTab; /**< No longer used, set to NULL */
    String remoteName;      /**< Name of Server image, if applicable
                             *
                             *   @remarks   If this Engine has no remote
                             *              algorithms, this can be NULL.
                             *
                             *   @remarks   On SysLink-based systems, this
                             *              is the name of a file, and is
                             *              passed unchanged to
                             *              ProcMgr_load().
                             */
    String memMap;          /**< Name of a file containing the slave
                             *   memory map
                             *
                             *   @remarks   If this Engine has no remote
                             *              algorithms, this can be NULL.
                             *
                             *   @remarks   If the remote algorithms are
                             *              on a Server whos MMU is not
                             *              enabled, this can be NULL.
                             *
                             *   @remarks   The format of this file matches
                             *              the SysLink format described at
                  * http://processors.wiki.ti.com/index.php/SysLink_MMU_Support
                             *
                             *   @remarks   If useExtLoader is FALSE, this
                             *              field can be NULL.
                             */
    Bool useExtLoader;      /**< Indicates whether the Server containing
                             *   any remote algorithms will be loaded using
                             *   an external loader (e.g. SysLink's
                             *   slaveloader)
                             *
                             *   @remarks   If @c useExtLoader is TRUE,
                             *              Engine_open() will not load
                             *              the slave.
                             *
                             *   @remarks   If @c useExtLoader is FALSE,
                             *              Engine_open() will load the
                             *              Server with the file specified
                             *              by @c remoteName.
                             */
    Int numAlgs;            /**< No longer used, set to zero */
    Int heapId;             /**< No longer used, set to zero */
} Engine_Desc;


/** @cond INTERNAL */

/*
 *  ======== Engine_AlgCreateAttrs ========
 */
typedef struct Engine_AlgCreateAttrs {
    Bool            useExtHeap; /**< Use a single external heap for alg's
                                 *   memory requests if TRUE, otherwise attempt
                                 *   to honor the alg's algAlloc() function for
                                 *   memory heap assignments.
                                 */
    Int             priority;   /**< Alg instance priority (-1: use value from
                                 *   configuration). */
} Engine_AlgCreateAttrs;


/*
 *  ======== Engine_Config ========
 */
typedef struct Engine_Config {
    Engine_Desc *engineTab;
    String      localEngine;
} Engine_Config;

/*
 *  ======== Engine_MemStat ========
 *  This structure must match Server_MemStat.
 */
typedef struct Engine_MemStat {
    Char   name[Engine_MAXSEGNAMELENGTH + 1]; /* Name of memory segment */
    Uint32 base;           /* Base address of memory segment */
    Uint32 size;           /* Original size of the memory segment. */
    Uint32 used;           /* Number of bytes used. */
    Uint32 maxBlockLen;    /* Size of the largest contiguous free block. */
} Engine_MemStat;

/* Default alg create attributes */
extern Engine_AlgCreateAttrs Engine_ALGCREATEATTRS;

/*
 *  ======== Engine_config ========
 */
extern Engine_Config Engine_config;

/** @endcond */


/*
 *  ======== Engine_addStubFxns ========
 */
/**
 *  @brief      Register stub functions through which a remote algorithm
 *              can be called
 *
 *  @param[in]  fxnsName    The name of the stub function table (e.g.
 *                          "UNIVERSAL_STUBS")
 *  @param[in]  fxns        Address of stub function table
 *                          (e.g. &UNIVERSAL_STUBS)
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @remarks    This service is not necessary if you configure your Engine
 *              at build/config time using Engine.createFromServer().
 *              When using Engine.createFromServer(), the appropriate
 *              alg-specific stubs are added to the system automatically.
 *
 *  @remarks    When on an RTOS (e.g. SYS/BIOS), the stubs registered are
 *              available to all Engines in the system.  When on a HLOS
 *              (e.g. Linux, WinCE), the stubs registered are available to
 *              all Engines in the calling application's process.
 *
 *  @remarks    The symbol passed to the @c fxns argument can often be found
 *              in the class-specific VISA header file (e.g. UNIVERSAL_STUBS
 *              is declared in ti/sdo/ce/universal/universal.h).
 *
 *  @remarks    For example, to register "UNIVERSAL_STUBS" for use by an
 *              IUNIVERSAL-compliant algorithm at runtime, you can
 *              do the following:
 *  @code
 *      #include <ti/sdo/ce/Engine.h>
 *      #include <ti/sdo/ce/universal/universal.h>
 *
 *      Engine_register("UNIVERSAL_STUBS",
 *              (IALG_Fxns *)&UNIVERSAL_STUBS);
 *  @endcode
 *
 *  @retval     Engine_EOK     Success.
 *  @retval     Engine_ENOMEM  Memory allocation failed.
 *
 *  @sa         Engine_open()
 */
extern Engine_Error Engine_addStubFxns(String fxnsName, IALG_Fxns *fxns);

/*
 *  ======== Engine_add ========
 */
/**
 *  @brief Add an Engine to the database of engines that can be opened with
 *         Engine_open()
 *
 *  @param[in]  pDesc       The handle of an Engine Descriptor object.
 *                          Before setting the fields of pDesc, it must
 *                          first be initialized with @c Engine_initDesc().
 *
 *  @retval     Engine_EINVAL   Bad parameter passed, such as @c pDesc = NULL,
 *                              or @c pDesc->name = NULL.
 *  @retval     Engine_EINUSE   An engine with the name @c pDesc->name already
 *                              exists.
 *  @retval     Engine_ENOMEM   A memory allocation failed.
 *  @retval     Engine_EOK      Success.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @post       If the return value is Engine_EOK, Engine_open() can be
 *              called with @c pDesc->name.
 *
 *  @par Example Usage:
 *  @code
 *      #include <ti/sdo/ce/Engine.h>
 *
 *      Engine_Desc desc;
 *
 *      Engine_initDesc(&desc);
 *
 *      desc.name = "myEngine";
 *      desc.remoteName = "myServer.x64P";
 *      Engine_add(&desc);
 *  @endcode
 *
 *  @sa         Engine_remove()
 *  @sa         Engine_open()
 */
extern Engine_Error Engine_add(Engine_Desc *pDesc);

/*
 *  ======== Engine_addAlg ========
 */
/**
 *  @brief      Dynamically add an algorithm to an Engine.
 *
 *  @remarks    If the Engine has not been opened, the name of the Engine is
 *              used, otherwise, a handle to the opened Engine. If the Engine
 *              has been opened, the added alg will only be accessible to the
 *              caller of this function.  Either one or the other, but not
 *              both, of @c name and @c engine should be non-NULL.
 *
 *  @param[in]  name            The name of the engine.  @c name is
 *                              specified in the engine configuration, or
 *                              the name of an engine added with
 *                              @c Engine_add(). This can only be non-NULL if
 *                              the engine is not opened, otherwise, use
 *                              an engine handle and set @c name to NULL.
 *  @param[in]  engine          The handle of an engine returned by
 *                              Engine_open(). If @c engine is non-NULL, set
 *                              @c name to NULL.
 *  @param[in]  location        String identifying the location of the
 *                              algorithm.  Often this is a file name, but for
 *                              systems without a file system, it may be a
 *                              system-specific string identifier. This may
 *                              be NULL if the algorithm is built into the
 *                              executable.
 *  @param[in]  pAlgDesc        Parameters describing the algorithm being
 *                              added. Before setting the fields of this
 *                              structure, it should first be initialized
 *                              with @c Engine_initAlgDesc(), to set all
 *                              fields to default values.
 *                              If location is non-NULL (a dynamic library),
 *                              then the following fields of pAlgDesc must
 *                              be specified:
 *                                  pAlgDesc->name
 *                                  pAlgDesc->isLocal
 *                                  pAlgDesc->groupId
 *                              All other Engine_AlgDesc fields will be
 *                              from the dynamic library.
 *                              If location is NULL (not a dynamic library),
 *                              the user must set the following fields of
 *                              pAlgDesc:
 *                                  pAlgDesc->name
 *                                  pAlgDesc->fxns
 *                                  pAlgDesc->idmaFxns, if applicable
 *                                  pAlgDesc->iresFxns, if applicable
 *                                  pAlgDesc->isLocal
 *                                  pAlgDesc->groupId
 *                                  pAlgDesc->types
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle which is
 *              in the open state.
 *
 *  @remarks    If there is an existing algorithm in the engine already named
 *              @c name, an error will be returned.
 *
 *  @retval     Engine_EOK      Success.
 *  @retval     Engine_EINUSE   The Engine @c name is non-NULL, and the
 *                              Engine is open.
 *  @retval     Engine_EINVAL   @c pAlgDesc or @c pAlgDesc->name is NULL.
 *  @retval     Engine_EINVAL   Both @c name and @c engine are NULL. Ensure
 *                              one of these is non-NULL.
 *  @retval     Engine_EINVAL   Both @c name and @c engine are non-NULL.
 *                              Ensure that one of these is NULL.
 *  @retval     Engine_EEXIST   There is no engine with the given name.
 *  @retval     Engine_EINUSE   The name of the alg in @c pAlgDesc->name is
 *                              already in use.
 *  @par Example Usage:
 *  @code
 *      #include <ti/sdo/ce/Engine.h>
 *
 *      Engine_AlgDesc desc;
 *
 *      Engine_initAlgDesc(&desc);
 *
 *      desc.groupId = 2;
 *      desc.isLocal = TRUE;
 *      desc.fxns = &UNIVERSALCOPY_TI_IUNIVERSALCOPY;
 *      desc.idmaFxns = NULL;
 *      desc.iresFxns = NULL;
 *      desc.memType = Engine_USECACHEDMEM_DEFAULT;
 *      desc.types = UNIVERSAL_VISATYPE;
 *
 *      status = Engine_addAlg("myEngine", NULL, NULL, &desc);
 *
 *  @endcode
 *
 *  @sa         Engine_initAlgDesc()
 *  @sa         Engine_open()
 *  @sa         Engine_removeAlg()
 */
extern Engine_Error Engine_addAlg(String name, Engine_Handle engine,
        String location, Engine_AlgDesc *pAlgDesc);

/*
 *  ======== Engine_removeAlg ========
 */
/**
 *  @brief      Dynamically remove an algorithm that was added to an Engine
 *              with Engine_addAlg().
 *
 *  @remarks    The same values of the parameters @c name and @c engine that
 *              were passed to Engine_addAlg() should be used here.  In
 *              particular, if @c name was used to add the alg, all handles to
 *              the engine must be closed before calling Engine_removeAlg().
 *
 *
 *  @param[in]  name            The name of the engine or NULL, that was
 *                              passed to Engine_addAlg().
 *  @param[in]  engine          The handle to an engine, previously acquired
 *                              by a call to Engine_open(), or NULL, that was
 *                              used in Engine_addAlg().
 *  @param[in]  algName         Name of the algorithm to remove.
 *
 *  @retval     Engine_EOK          Success.
 *  @retval     Engine_EEXIST       The engine @c name does not exist.
 *  @retval     Engine_ENOTFOUND    @c algName could not be found in @c engine.
 *  @retval     Engine_EINUSE       The Engine @c name is still open.
 *
 *  @sa         Engine_open()
 *  @sa         Engine_addAlg()
 */
extern Engine_Error Engine_removeAlg(String name, Engine_Handle engine,
        String algName);

/** @cond INTERNAL */

/*
 *  ======== Engine_call ========
 */
extern Int Engine_call(Engine_Node node, Comm_Msg *msg);

/*
 *  ======== Engine_callAsync ========
 */
extern Int Engine_callAsync(Engine_Node node, Comm_Msg *msg);

/*
 *  ======== Engine_callWait ========
 */
extern Int Engine_callWait(Engine_Node node, Comm_Msg *msg, UInt timeout);

/*
 *  ======== Engine_ctrlNode ========
 */
extern Int Engine_ctrlNode(Engine_Node node, Comm_Msg *msg, Engine_Ctrl code);

/** @endcond */

/*
 *  ======== Engine_close ========
 */
/**
 *  @brief      Close an Engine
 *
 *  @param[in]  engine          The handle to an engine, previously acquired
 *                              by a call to Engine_open().
 *
 *  @pre        @c engine must not be referenced by any algorithm instance
 *              objects; i.e., you must first delete all algorithm instances
 *              associated with @c engine before closing it.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle which is
 *              in the open state.
 *
 *  @sa         Engine_open()
 */
extern Void Engine_close(Engine_Handle engine);

/** @cond INTERNAL */
/*
 *  ======== Engine_createNode ========
 */
/**
 *  @brief      Create a remote algorithm
 *
 *  @param[in]  engine          The handle to an engine, previously acquired
 *                              by a call to Engine_open().
 *  @param[in]  name            Name of the algorithm to create.
 *  @param[in]  msgSize         Size of the internal message required to
 *                              communicate with the remote algorithm.
 *  @param[in]  nodeAttrs       Creation parameters for the remote algorithm.
 *  @param[in]  attrs           Attributes used by the framework for creating
 *                              the remote algorithm.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle which is
 *              in the open state.
 *
 *  @remarks    Engine_createNode2() was added after Engine_createNode() to
 *              support more use cases.  Engine_createNode() is a wrapper
 *              around Engine_createNode2(), and is maintained for compatibility.
 *
 *  @retval     NULL            Failure
 *  @retval     non-NULL        A handle to the created remote algorithm.
 *
 *  @sa         Engine_createNode2()
 *  @sa         Engine_deleteNode()
 */
extern Engine_Node Engine_createNode(Engine_Handle engine, String name,
    size_t msgSize, IALG_Params *nodeAttrs, Engine_AlgCreateAttrs *attrs);


/*
 *  ======== Engine_createNode2 ========
 */
/**
 *  @brief      Create a remote algorithm
 *
 *  @param[in]  engine          The handle to an engine, previously acquired
 *                              by a call to Engine_open().
 *  @param[in]  name            Name of the algorithm to create.
 *  @param[in]  msgSize         Size of the internal message required to
 *                              communicate with the remote algorithm.
 *  @param[in]  nodeAttrs       Creation parameters for the remote algorithm.
 *  @param[in]  nodeAttrsSize   Size of @c nodeAttrs.
 *  @param[in]  attrs           Attributes used by the framework for creating
 *                              the remote algorithm.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle which is
 *              in the open state.
 *
 *  @remarks    Engine_createNode() is the preferred method to create remote
 *              algorithms.  However, some algorithm interfaces incorrectly
 *              fail to provide a size field of type "Int" as the first field
 *              in their creation parameters, which the XDAIS spec defines.
 *              This service allows the creation of remote algorithms where the
 *              size of the creation params is specified "some other way" than
 *              the XDAIS spec defines.
 *
 *  @retval     NULL            Failure
 *  @retval     non-NULL        A handle to the created remote algorithm.
 *
 *  @sa         Engine_createNode()
 *  @sa         Engine_deleteNode()
 */
extern Engine_Node Engine_createNode2(Engine_Handle engine, String name,
    size_t msgSize, IALG_Params *nodeAttrs, Int nodeAttrsSize,
    Engine_AlgCreateAttrs *attrs);


/*
 *  ======== Engine_deleteNode ========
 */
extern Void Engine_deleteNode(Engine_Node node);

/*
 *  ======== Engine_getAlgMemRecs ========
 */
/**
 *  @brief      Get the IALG_MemRecs used by an algorithm
 *
 *  @param[in]  node    Handle to an algorithm instance.
 *  @param[out] memTab  Location to store the IALG_MemRecs.
 *  @param[in]  size    Maximum number of IALG_MemRecs to put in memTab array.
 *  @param[out] numRecs Actual number of IALG_MemRecs copied into memTab array.
 *
 *  @retval     Engine_EOK       Success.
 *  @retval     Engine_ERUNTIME  Failure.
 *
 *  @sa         Engine_getAlgNumRecs()
 */
extern Engine_Error Engine_getAlgMemRecs(Engine_Node node, IALG_MemRec *memTab, Int size,
        Int *numRecs);

/*
 *  ======== Engine_getAlgNumRecs ========
 */
/**
 *  @brief      Get the number of IALG_MemRecs used by a remote algorithm
 *
 *  @param[in]  node    Handle to an algorithm instance.
 *  @param[out] numRecs Location to store the number of IALG_MemRecs used.
 *
 *  @retval     Engine_EOK       Success.
 *  @retval     Engine_ERUNTIME  Failure.
 *
 *  @sa         Engine_getAlgMemRecs()
 */
extern Engine_Error Engine_getAlgNumRecs(Engine_Node node, Int *numRecs);

/*
 *  ======== Engine_getConstName ========
 */
extern String Engine_getConstName(Engine_Handle engine, String name,
    String type);

/*
 *  ======== Engine_getFxns ========
 */
extern IALG_Fxns *Engine_getFxns(Engine_Handle svr, String name, String type,
        Bool *isLocal, Ptr *idmaFxns, Ptr *iresFxns, Int *groupId,
        Engine_CachedMemType *memType);

/*
 *  ======== Engine_getMemId ========
 */
extern Int Engine_getMemId(Engine_Handle engine);

/*
 *  ======== Engine_getLocalEngine ========
 */
extern Engine_Handle Engine_getLocalEngine(Void);

/*
 *  ======== Engine_getEngine ========
 */
extern Engine_Handle Engine_getEngine(Engine_Node node);

/*
 *  ======== Engine_getMemStat ========
 */
extern Engine_Error Engine_getMemStat(Server_Handle server, Int segNum,
    Engine_MemStat *stat);

/*
 *  ======== Engine_getNumMemSegs ========
 */
extern Engine_Error Engine_getNumMemSegs(Server_Handle server, Int *numSegs);

/*
 *  ======== Engine_getNumEngines ========
 */
extern Int Engine_getNumEngines();

/*
 *  ======== Engine_getProcId ========
 */
extern String Engine_getProcId(Engine_Handle engine);

/*
 *  ======== Engine_hasServer ========
 */
extern Bool Engine_hasServer(Engine_Handle engine);

/*
 *  ======== Engine_init ========
 */
extern Void Engine_init(Void);


/** @endcond */

/*
 *  ======== Engine_initAlgDesc ========
 */
/**
 *  @brief      Initialize an Engine_AlgDesc structure with default values.
 *
 *  @param[in]  pAlgDesc  Location of Engine_AlgDesc object to initialize.
 *                        The fields of pAlgDesc will be set to the following:
 *
 *                        pAlgDesc->name = NULL;
 *                        pAlgDesc->uuid.data = 0;
 *                        pAlgDesc->fxns = NULL;
 *                        pAlgDesc->idmaFxns = NULL;
 *                        pAlgDesc->typeTab = NULL;
 *                        pAlgDesc->isLocal = TRUE;
 *                        pAlgDesc->groupId = 0;
 *                        pAlgDesc->rpcProtocolVersion = 0;
 *                        pAlgDesc->iresFxns = NULL;
 *                        pAlgDesc->codecClassConfig = NULL;
 *                        pAlgDesc->memType = Engine_USECACHEDMEM_DEFAULT;
 *                        pAlgDesc->types = NULL;
 *
 *  @sa         Engine_addAlg()
 */
extern Void Engine_initAlgDesc(Engine_AlgDesc *pAlgDesc);

/*
 *  ======== Engine_initAttrs ========
 */
/**
 *  @brief      Initialize an Engine_Attrs structure with default values.
 *
 *  @param[in]  pAttrs Location of Engine_Attrs object to initialize.
 *
 *  @sa         Engine_open()
 */
extern Void Engine_initAttrs(Engine_Attrs *pAttrs);

/*
 *  ======== Engine_initDesc ========
 */
/**
 *  @brief      Initialize an Engine_Desc structure with default values.
 *
 *  @param[in]  pDesc  Location of Engine_Desc object to initialize.
 *                     The fields of pDesc will be set to the following:
 *
 *                        pDesc->name = NULL;
 *                        pDesc->remoteName = NULL;
 *                        pDesc->heapId = 0;
 *
 *  @sa         Engine_add()
 */
extern Void Engine_initDesc(Engine_Desc *pDesc);

/** @cond INTERNAL */

/*
 *  ======== Engine_getRemoteVisa ========
 */
extern UInt32 Engine_getRemoteVisa(Engine_Node node);

/*
 *  ======== Engine_getCodecClassConfig ========
 */
extern Ptr Engine_getCodecClassConfig(Engine_Handle engine, String name,
    String type);

/*
 *  ======== Engine_getNodeQueues ========
 */
extern Void Engine_getNodeQueues(Engine_Node node, Comm_Id *stdIn, Comm_Handle *stdOut);


/*
 *  ======== Engine_initFromServer ========
 */
extern Engine_Error Engine_initFromServer(Engine_Handle engine);


/*
 *  ======== Engine_redefineHeap ========
 */
extern Engine_Error Engine_redefineHeap(Server_Handle server, String name,
        Uint32 base, Uint32 size);

/*
 *  ======== Engine_releaseTraceToken ========
 */
extern Bool Engine_releaseTraceToken(Server_Handle server);

/*
 *  ======== Engine_requestTraceToken ========
 */
extern Engine_Error Engine_requestTraceToken(Server_Handle server);

/*
 *  ======== Engine_restoreHeap ========
 */
extern Engine_Error Engine_restoreHeap(Server_Handle server, String name);

/** @endcond */

/*
 *  ======== Engine_open ========
 */
/**
 *  @brief      Open an Engine
 *
 *  The handle returned may be used to create one or more instances of an
 *  algorithm contained in the specified Engine.
 *
 *  An Engine may be opened more than once; each open returns a unique
 *  handle that can be used to create algorithm instances or get status of the
 *  Engine.
 *
 *  Engine handles must not be concurrently accessed by multiple threads; each
 *  thread must either obtain its own handle (via Engine_open()) or explicitly
 *  serialize access to a shared handle.
 *
 *  @param[in]  name            The name of the engine to open.  @c name is
 *                              specified in the engine configuration.
 *  @param[in]  attrs           Attributes for the open engine.
 *  @param[out] ec              Optional output error code
 *
 *  @retval     NULL            An error has occurred.
 *  @retval     non-NULL        The handle to the opened engine.
 *
 *  @pre        @c name is a non-NULL string.
 *
 *  @pre        @c name is a valid, pre-configured name of an engine.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @post       If the return handle is NULL and @c ec is non-NULL, @c *ec
 *              is set to a non-zero value indicating the cause of the failure.
 *
 *  @post       If @c ec is non-NULL, the Engine_Error value is set to one of
 *              the following values:
 *                - #Engine_EOK         success
 *                - #Engine_EEXIST      name does not exist
 *                - #Engine_ENOMEM      can't allocate memory
 *                - #Engine_EDSPLOAD    can't load the DSP
 *                - #Engine_ENOCOMM     can't create a comm connection to DSP
 *                - #Engine_ENOSERVER   can't locate the server on the DSP
 *                - #Engine_ECOMALLOC   can't allocate communication buffer
 *
 *  @sa         Engine_close()
 *  @sa         Engine_add()
 *  @sa         Engine_remove()
 */
extern Engine_Handle Engine_open(String name, Engine_Attrs *attrs,
        Engine_Error *ec);

/*
 *  ======== Engine_fwriteTrace ========
 */
/**
 *  @brief              Write Server's trace buffer to specifed file stream
 *
 *  @param[in]  engine  The handle to the opened engine.
 *
 *  @param[in]  prefix  A string to prepend to each line output; this
 *                      allows one to easily identify trace from the
 *                      server from the application's trace, for
 *                      example.
 *  @param[in]  out     A open FILE stream used to output the
 *                      Server's trace characters.
 *
 *  @retval             Integer number of characters copied to the specified
 *                      FILE stream.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle and the engine
 *              is in the open state.
 *
 *  @post       In the event a negative value is returned,
 *              Engine_getLastError() will return one of the following values:
 *                - #Engine_ERUNTIME    Either an internal runtime error
 *                                      occured or the underlying server
 *                                      error occured.
 *                - #Engine_EINUSE      Server trace resource is already in use.
 */
extern Int Engine_fwriteTrace(Engine_Handle engine, String prefix, FILE *out);

/*
 *  ======== Engine_getAlgInfo ========
 */
/**
 *  @brief      Get details of an algorithm configured into an engine
 *
 *  @param[in]  name        The name of the engine.  @c name is
 *                          specified in the engine configuration.
 *  @param[out] algInfo     Structure to store algorithm details. The
 *                          @c algInfoSize field of this structure must
 *                          be set to @c sizeof(Engine_AlgInfo) by the
 *                          application.
 *  @param[out] index       The index of the algorithm to get the information.
 *
 *  @retval     Engine_EOK        Success.
 *  @retval     Engine_EEXIST     There is no engine with the given name.
 *  @retval     Engine_ENOTFOUND  @c index is greater than or equal to the
 *                                total number of algorithms configured for
 *                                the engine, or @c index < 0.
 *  @retval     Engine_EINVAL     The value of @c algInfoSize passed to this
 *                                function does not match the CE library's
 *                                @c sizeof(Engine_AlgInfo).
 *
 *  @pre        @c name is a non-NULL string.
 *
 *  @pre        @c algInfo is non-NULL.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @post       If @c name is a valid engine name and 0 <= @c index < the
 *              total number of algorithms configured for the engine, then
 *              @c algInfo will contain the information for the engine's
 *              ith (i = @c index) algorithm.
 *
 *  @sa         Engine_getNumAlgs()
 */
extern Engine_Error Engine_getAlgInfo(String name, Engine_AlgInfo *algInfo,
        Int index);

/*
 *  ======== Engine_getAlgInfo2 ========
 */
/**
 *  @brief      Get details of an algorithm.
 *
 *  @param[in]  name        The name of the engine.  @c name is
 *                          specified in the engine configuration. This
 *                          may be NULL if @c engine contains a valid
 *                          engine handle.
 *  @param[in]  engine      The handle of an engine returned by Engine_open().
 *                          If this is NULL, only information for a static
 *                          alg can be obtained.
 *  @param[out] algInfo2    Structure to store algorithm details. The
 *                          @c algInfoSize field of this structure must
 *                          be set to @c sizeof(Engine_AlgInfo2) by the
 *                          application.
 *  @param[out] index       The index of the algorithm to get the information.
 *
 *  @retval     Engine_EOK        Success.
 *  @retval     Engine_EEXIST     There is no engine with the given name.
 *  @retval     Engine_ENOTFOUND  @c index is greater than or equal to the
 *                                total number of algorithms configured for
 *                                the engine, or @c index < 0.
 *  @retval     Engine_EINVAL     The value of @c algInfoSize passed to this
 *                                function does not match the CE library's
 *                                @c sizeof(Engine_AlgInfo2).
 *
 *  @pre        @c name is a non-NULL string or @c engine is non-NULL.
 *
 *  @pre        @c algInfo2 is non-NULL.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @post       If @c name is a valid engine name and 0 <= @c index < the
 *              total number of algorithms configured for the engine, then
 *              @c algInfo2 will contain the information for the engine's
 *              ith (i = @c index) algorithm.
 *
 *  @remarks    This service supports algorithms statically configured into
 *              an Engine at build/config time, or algorithms that have been
 *              dynamically added to an opened engine.  If the Engine has not
 *              been opened yet, the name of the Engine is used to get the
 *              statically configured alg.  If the Engine has been opened, the
 *              Engine handle can be used to get either information for a
 *              statically configured alg, or a remote alg that was added when
 *              the server was queried during Engine_open().
 *
 *  @sa         Engine_getNumAlgs2().
 */
extern Engine_Error Engine_getAlgInfo2(String name, Engine_Handle engine,
        Engine_AlgInfo2 *algInfo2, Int index);


/** @cond INTERNAL */
/*
 *  ======== Engine_getCpuLoad ========
 */
/**
 *  @brief      Get Server's cpu usage in percent
 *
 *  @deprecated This service has been replaced by Server_getCpuLoad()
 *              to better indicate that this API is not intended for
 *              obtaining the current processor's CPU load, rather it
 *              obtains the CPU load of a remote Server.
 *
 *  @param[in]  engine  The handle to the opened engine.
 *
 *  @retval             integer between 0-100 indicating percentage
 *                      of time the Server is processing measured
 *                      over a period of approximately 1 second.  If
 *                      the load is unavailable, a negative value is
 *                      returned.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle and the engine
 *              is in the open state.
 *
 *  @post       In the event a negative value is returned,
 *              Engine_getLastError() will return one of the following values:
 *                - #Engine_ERUNTIME    Either an internal runtime error
 *                                      occured or the underlying server
 *                                      error occured.
 *                - #Engine_ENOTAVAIL   The CPU load can not be computed.
 *
 *  @sa         Server_getCpuLoad()
 */
extern Int Engine_getCpuLoad(Engine_Handle engine);

/** @endcond */


/*
 *  ======== Engine_getDesc ========
 */
/**
 *  @brief      Fill in an Engine_Desc structure with the values of the
 *              Engine descriptor for an Engine.
 *
 *  @param[in]  name            The name of the Engine.  @c name is
 *                              specified in the engine configuration or a
 *                              a name that was passed to @c Engine_add().
 *  @param[out] desc            The structure where the descriptor of the
 *                              Engine specified by @c name will be copied to.
 *
 *  @retval     Engine_EOK      Success.
 *  @retval     Engine_EEXIST   There is no engine with the given name.
 *
 *  @pre        @c name is a non-NULL string.
 *
 *  @pre        @c desc is non-NULL.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @post       If @c name is a valid engine name, then desc will contain
 *              the descriptor of the engine @c name.
 *
 *  @sa         Engine_setDesc().
 */
extern Engine_Error Engine_getDesc(String name, Engine_Desc *desc);

/*
 *  ======== Engine_getLastError ========
 */
/**
 *  @brief              Get error code of the last failed operation
 *
 *  @param[in]  engine  The handle to the opened engine.
 *
 *  @retval             error code (Engine_Error) of the last failed
 *                      engine operation.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle and the engine
 *              is in the open state.
 */
extern Engine_Error Engine_getLastError(Engine_Handle engine);

/*
 *  ======== Engine_getName ========
 */
/**
 *  @brief      Get the name of an opened engine
 *
 *  @param[in]  engine  The handle to the opened engine.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @retval     NULL            An error has occurred.
 *  @retval     non-NULL        The name of the opened engine.
 */
extern String Engine_getName(Engine_Handle engine);


/*
 *  ======== Engine_getNumAlgs ========
 */
/**
 *  @brief      Get the number of algorithms configured into an Engine
 *
 *  @param[in]  name            The name of the Engine.  @c name is
 *                              specified in the engine configuration.
 *  @param[out] numAlgs         The number of algorithms that are configured
 *                              in the given engine.
 *
 *  @retval     Engine_EOK      Success.
 *  @retval     Engine_EEXIST   There is no engine with the given name.
 *
 *  @pre        @c name is a non-NULL string.
 *
 *  @pre        @c numAlgs is non-NULL.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @post       If @c name is a valid engine name, then numAlgs will contain
 *              the number of algorithms configured for the given engine.
 *
 *  @sa         Engine_getAlgs().
 */
extern Engine_Error Engine_getNumAlgs(String name, Int *numAlgs);

/*
 *  ======== Engine_getNumAlgs2 ========
 */
/**
 *  @brief Get the number of algorithms statically configured into an engine
 *         or the total number of algorithms both statically configured and
 *         dynamically added through server information when the engine was
 *         opened.
 *
 *  @param[in]  name            The name of the engine.  @c name is
 *                              specified in the engine configuration. @c name
 *                              can be NULL, if @c engine is a valid
 *                              Engine_Handle.
 *  @param[in]  engine          The handle of an engine returned by
 *                              Engine_open(). If @c engine is NULL, @c name
 *                              must be non-NULL, and only the number of
 *                              statically configured algorithms will be
 *                              returned in @c numAlgs.
 *                              specified in the engine configuration.
 *  @param[out] numAlgs         The number of algorithms that are configured
 *                              in the given engine.
 *
 *  @retval     Engine_EOK      Success.
 *  @retval     Engine_EEXIST   There is no engine with the given name.
 *
 *  @pre        @c name is a non-NULL string or @c engine is non-NULL.
 *
 *  @pre        @c numAlgs is non-NULL.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @post       If @c name is a valid engine name, then numAlgs will contain
 *              the number of algorithms configured for the given engine.
 *
 *  @remarks    If the engine has a server, but was not configured with
 *              Engine.createFromServer() number of remote algorithms (if any)
 *              that were statically configured into the engine, will be
 *              counted twice: once for the static alg table, and once for
 *              the information queried from the server.
 *
 *  @sa         Engine_getAlgs().
 */
extern Engine_Error Engine_getNumAlgs2(String name, Engine_Handle engine,
        Int *numAlgs);

/*
 *  ======== Engine_getServer ========
 */
/**
 *  @brief              Get handle to an Engine's Server
 *
 *  This function returns the handle to an Engines server, that can be used
 *  with Server APIs to obtain information from and control the remote DSP
 *  server.
 *
 *  @param[in]  engine  The handle to the opened engine.
 *
 *  @retval             Handle to engine's server.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle and the engine
 *              is in the open state.
 */
extern Server_Handle Engine_getServer(Engine_Handle engine);


/*
 *  ======== Engine_getUsedMem ========
 */
/**
 *  @brief      Get Server's total memory usage
 *
 *  @deprecated This service has been replaced by Server_getMemStat()
 *              to better indicate that this API is not intended for
 *              obtaining the current processor's memory statistics,
 *              rather it obtains the memory statistics of a remote
 *              Server.  Also, Server_getMemStat() provides more granularity
 *              than Engine_getUsedMem().
 *
 *  @param[in]  engine  The handle to the opened engine.
 *
 *  @retval     Total amount of used memory (in MAUs).  If the amount is not
 *              available, 0 is returned and the reason can be retrieved via
 *              Engine_getLastError().
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle and the engine
 *              is in the open state.
 *
 *  @post       in the event that 0 is returned, Engine_getLastError() will
 *              return one of the following values:
 *                - #Engine_ERUNTIME    Either an internal runtime error
 *                                      occured or the underlying server
 *                                      error occured.
 *                - #Engine_ENOTAVAIL   The memory usage can not be computed.
 */
extern UInt32 Engine_getUsedMem(Engine_Handle engine);


/*
 *  ======== Engine_remove ========
 */
/**
 *  @brief      Remove an engine from the list of engines that can be opened
 *
 *  @param[in]  engineName      The name of the engine to be removed.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @pre        @c engineName is non-NULL.
 *
 *  @retval     Engine_EINUSE      The engine cannot be removed because
 *                                 an instance of it is still opened.
 *  @retval     Engine_EEXIST      No engine by this name exists.
 *  @retval     Engine_EOK         Success.
 *
 *  @sa         Engine_add()
 */
extern Engine_Error Engine_remove(String engineName);


/*
 *  ======== Engine_setDesc ========
 */
/**
 *  @brief      Set values for an Engine's descriptor.  This function should
 *              only be called when the Engine has not yet been opened.
 *
 *  @param[in]  name            The name of the Engine.  @c name is
 *                              specified in the engine configuration or a
 *                              a name that was passed to @c Engine_add().
 *  @param[in] desc             The structure where descriptor values for the
 *                              Engine specified by @c name will be copied
 *                              from.
 *
 *  @retval     Engine_EOK      Success.
 *  @retval     Engine_EEXIST   There is no engine with the given name.
 *  @retval     Engine_EINUSE   The Engine @c name has already been opened.
 *
 *  @pre        @c name is a non-NULL string.
 *
 *  @pre        @c desc is non-NULL.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @post       If @c name is a valid engine name, then the desscriptor for
 *              the engine @c name will have been updated with values from
 *              @c desc.
 *
 *  @remarks    Use @c Engine_getDesc() to fill in the descriptor, override
 *              the fields you want to change, and then pass the descriptor
 *              to Engine_setDesc().
 *              Only the following fields of the Engine_Desc are allowed to
 *              be modified:
 *                  @c memMap
 *                  @c useExtLoader
 *                  @c heapId
 *
 *  @sa         Engine_getDesc().
 */
extern Engine_Error Engine_setDesc(String name, Engine_Desc *desc);


/*
 *  ======== Engine_setTrace ========
 */
/**
 *  @brief      Set Server's trace mask
 *
 *  @param[in]  engine  The handle to the opened engine.
 *  @param[in]  mask    Trace mask, e.g. "*=01234567"
 *
 *  @retval     Engine_ENOSERVER   No server for this engine.
 *  @retval     Engine_EINUSE      Trace resource is already in use.
 *  @retval     Engine_ERUNTIME    Internal runtime error has occurred.
 *
 *  @pre        As with all Codec Engine API's, CERuntime_init() must have
 *              previously been called.
 *
 *  @pre        @c engine is a valid (non-NULL) engine handle and the engine
 *              is in the open state.
 *
 *  @remarks    This only sets the trace for a remote server.  To change
 *              the trace mask for the application-side of the framework,
 *              use Diags_setMask(), or Diags_setMaskMeta().
 *
 *  @sa         xdc.runtime.Diags
 */
extern Int Engine_setTrace(Engine_Handle engine, String mask);


/** @cond INTERNAL */

/*
 *  ======== Engine_getDesc ========
 *  Internal for testing.
 */
extern Engine_Desc *_Engine_getDesc(Int i);

/** @endcond */

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
/*
 *  @(#) ti.sdo.ce; 1, 0, 6,1; 8-14-2012 12:59:44; /db/atree/library/trees/ce/ce-u07/src/ xlibrary

 */

