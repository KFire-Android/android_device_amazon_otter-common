/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _D_OMX_BASE_H_
#define _D_OMX_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_TI_Index.h>
#include <OMX_TI_Common.h>

#define OMX_NOPORT 0xFFFFFFFE
#define OMX_BASE_MAXNAMELEN 32

/**================================================================*/
/**
 * @def Used by the Derived Components to extend the
 *           Base Component Private Structure
 *
 *  Usage Rules:
 *
 *  1)  Include this File
 *  2)  If you don't extend(inherit) start your structure with STRUCT
 *  3)  If you want to Extend(inherit) few elements,
 *      start your Structure with
 *      DERIVEDSTRUCT (structure_name , inherited _structure_name)
 *  4)  End your Structure with ENDSTRUCT(structure_name)
 *  5)  Define your Structure variables with a
 *      #define structure_name_FIELDS
 *      inherited_structure_name_FIELDS  inside your structure and
 *      always add a backslash at the end of the file (except last)
 */
/**==================================================================*/
#define STRUCT(a) typedef struct a a; \
    struct a {
#define DERIVEDSTRUCT(a, b) typedef struct a a; \
    struct a {
#define ENDSTRUCT(a) a##_FIELDS };


/** The OMX_BUFFERHEADERTYPE_FIELDS macro is used by derived components when
 *  defining their own buffer headers. The custom defined buffer headers will
 *  have this as their first field and then any other fields that may be
 *  required. Thus any standard OMX buffer header fields can be directly
 *  accessed from the custom defined header. The nMetaDataUsageBitField is a
 *  compulsory 4 byte integer that every custom header should have to indicate
 *  to the dio layer which of the meta data types it is using from the ones
 *  defined in pMetaDataFieldTypesArr at init time. However if the custom header
 *  is not being used to send any meta data then this field is not required. An
 *  example custom defined header could be:
 *   DERIVEDSTRUCT(OMX_ABC_CUSTOMHEADER, OMX_BASE_BUFFERHEADERTYPE)
 *       #define OMX_ABC_CUSTOMHEADER_FIELDS OMX_BASE_BUFFERHEADERTYPE_FIELDS \
 *       OMX_U32 nMetaDataUsageBitField;                                      \
 *       OMX_U32 nMetaData1;                                                  \
 *       OMX_U16 nMetaData2;                                                  \
 *       OMX_PTR pQuantData;                                                  \
 *       OMX_U32 nQuantDataSize;
 *   ENDSTRUCT(OMX_ABC_CUSTOMHEADER)
 */
STRUCT(OMX_BASE_BUFFERHEADERTYPE)
    #define OMX_BASE_BUFFERHEADERTYPE_FIELDS  \
    OMX_U32 nSize;                            \
    OMX_VERSIONTYPE nVersion;                 \
    OMX_U8 *pBuffer;                          \
    OMX_U32 nAllocLen;                        \
    OMX_U32 nFilledLen;                       \
    OMX_U32 nOffset;                          \
    OMX_PTR pAppPrivate;                      \
    OMX_PTR pPlatformPrivate;                 \
    OMX_PTR pInputPortPrivate;                \
    OMX_PTR pOutputPortPrivate;               \
    OMX_HANDLETYPE hMarkTargetComponent;      \
    OMX_PTR pMarkData;                        \
    OMX_U32 nTickCount;                       \
    OMX_U32 __padding1;                       \
    OMX_TICKS nTimeStamp;                     \
    OMX_U32 nFlags;                       \
    OMX_U32 nOutputPortIndex;                 \
    OMX_U32 nInputPortIndex;                  \
    OMX_U32 __padding2;
ENDSTRUCT(OMX_BASE_BUFFERHEADERTYPE)


/*===============================================================*/
/** The OMX_BASE_CONTEXT_TYPE          : Indicates active or passive context.
 *
 * @ param OMX_BASE_CONTEXT_PASSIVE    : In passive context the base component
 *                                       does not create a separate thread to
 *                                       handle asynchronous commands.
 *                                       Asynchronous commands are handled in
 *                                       the context of the caller in base.
 * @ param OMX_BASE_CONTEXT_ACTIVE     : In active context, the base component
 *                                       creates a separate thread to handle
 *                                       asynchronous commands.
 */
typedef enum OMX_BASE_CONTEXT_TYPE {
    OMX_BASE_CONTEXT_PASSIVE = 0x1,
    OMX_BASE_CONTEXT_ACTIVE  = 0x2
}OMX_BASE_CONTEXT_TYPE;



/*===============================================================*/
/** The OMX_BASE_DIO_CTRLCMD_TYPE            : This enumeration is used to
 *                                          specify the action in the
 *                                          OMX_BASE_DIO_Control Macro.
 *
 * @ param OMX_BASE_DIO_CTRLCMD_Flush        : Flush the buffers on this port.
 *                                          This command is used only by
 *                                          omx_base for now.
 * @ param OMX_BASE_DIO_CTRLCMD_Start        : Start buffer processing on this
 *                                          port. This command is used only by
 *                                          omx_base for now.
 * @ param OMX_BASE_DIO_CTRLCMD_Stop         : Stop buffer processing on this
 *                                          port. This command is used only by
 *                                          omx_base for now.
 * @ param OMX_BASE_DIO_CTRLCMD_GetCtrlAttribute : To get the attribute pending
 *                                          on this port. A pointer to structure
 *                                          of type
 *                                          OMX_BASE_ATTRIBUTE_DESCRIPTOR is
 *                                          passed as the parameter to the
 *                                          control command. This should be used
 *                                          if dio_dequeue returns
 *                                          OMX_WarningAttributePending
 *                                          indicating that an attribute buffer
 *                                          is pending.
 * @ param OMX_BASE_DIO_CTRLCMD_SetCtrlAttribute : To send some attribute on
 *                                          this port. A pointer to structure of
 *                                          type OMX_BASE_ATTRIBUTE_DESCRIPTOR
 *                                          is passed as the parameter to the
 *                                          control command.
 * @ param OMX_BASE_DIO_CTRLCMD_ExtnStart    : If some specific DIO
 *                                          implementation wants to have control
 *                                          commands specific for that DIO then
 *                                          these extended commands can be added
 *                                          after this.
 */
typedef enum OMX_BASE_DIO_CTRLCMD_TYPE {
    OMX_BASE_DIO_CTRLCMD_Flush            = 0x1,
    OMX_BASE_DIO_CTRLCMD_Start            = 0x2,
    OMX_BASE_DIO_CTRLCMD_Stop             = 0x3,
    OMX_BASE_DIO_CTRLCMD_GetCtrlAttribute = 0x4,
    OMX_BASE_DIO_CTRLCMD_SetCtrlAttribute = 0x5,

    OMX_BASE_DIO_CTRLCMD_ExtnStart        = 0xA
}OMX_BASE_DIO_CTRLCMD_TYPE;



/*===============================================================*/
/** The OMX_BASE_DIO_UTIL_TYPE                            : This enumeration is
 *                                                          used to specify the
 *                                                          action in
 *                                                          OMX_BASE_DIO_Util
 *
 * @ param OMX_BASE_DIO_UTIL_GetTotalBufferSize           : A pointer to the
 *                                     derived component defined buffer header
 *                                     is passed as the pParams parameter in the
 *                                     utility function. In the header,
 *                                     the bitfield, the buffer data size (as
 *                                     AllocLen) and the size of all the meta
 *                                     data is taken as input and returns the
 *                                     total buffer size (as AllocLen) that the
 *                                     derived component will set in its port
 *                                     definitions structure. The total buffer
 *                                     size will be much bigger as the meta data
 *                                     passed by the derived component in the
 *                                     dio header will have to be packed along
 *                                     with the data buffer according to omx
 *                                     protocol. The meta data types will be
 *                                     picked up from the meta data array in
 *                                     derived to base params.
 * @ param OMX_BASE_DIO_UTIL_UpdatePacketExtraDataPointer :
 */
typedef enum OMX_BASE_DIO_UTIL_TYPE {
    OMX_BASE_DIO_UTIL_GetTotalBufferSize                      = 0x1,
    OMX_BASE_DIO_UTIL_UpdatePacketExtraDataPointer            = 0x2,

    OMX_BASE_DIO_UTIL_ExtnStart                               = 0xA
}OMX_BASE_DIO_UTIL_TYPE;



/*===============================================================*/
/** OMX_BASE_DATA_ACCESS_MODE_TYPE  : Data mode.
 *
 *  @ param OMX_BASE_FRAME_MODE     : Frame based data
 *  @ param OMX_BASE_STREAM_MODE    : Stream based data
 */
/*===============================================================*/
typedef enum OMX_BASE_DATA_ACCESS_MODE_TYPE {
    OMX_BASE_DATA_ACCESS_MODE_FRAME           = 0x1,
    OMX_BASE_DATA_ACCESS_MODE_STREAM          = 0x2
}OMX_BASE_DATA_ACCESS_MODE_TYPE;



/*===============================================================*/
/** OMX_BASE_BUFFER_MEMORY_TYPE      : Buffer allocation type.
 *
 *  @ param OMX_BASE_BUFFER_MEMORY_DEFAULT  : Default allocation
 *  @ param OMX_BASE_BUFFER_MEMORY_1D_CONTIGUOUS : 1D contiguous
 *                                                 allocation (paged mode)
 *  @ param OMX_BASE_BUFFER_MEMORY_2D            : 2D buffer allocation
 *  @ param OMX_BASE_BUFFER_MEMORY_CUSTOM        : Custom buffer allocation
 *                                                 which will be specified by
 *                                                 derived component
 */
/*===============================================================*/
typedef enum OMX_BASE_BUFFER_MEMORY_TYPE {
    OMX_BASE_BUFFER_MEMORY_DEFAULT           = 0x1,
    OMX_BASE_BUFFER_MEMORY_1D_CONTIGUOUS     = 0x2,
    OMX_BASE_BUFFER_MEMORY_2D                = 0x3,

    OMX_BASE_BUFFER_MEMORY_CUSTOM            = 0xA,
    OMX_BASE_BUFFER_MEMORY_VIRTUAL           = 0xB, /*Used when buffer pointers come from the normal A9 virtual space */
    OMX_BASE_BUFFER_MEMORY_GRALLOC           = 0xC, /*Used when buffer pointers come from Gralloc allocations */
    OMX_BASE_BUFFER_MEMORY_ION               = 0xD, /*Used when buffer pointers come from ION allocations */
    OMX_BASE_BUFFER_MEMORY_ENC_METADATA      = 0xE, /*Used when buffer pointers come from Stagefright in camcorder usecase */
    OMX_BASE_BUFFER_MEMORY_DESC_VIRTUAL_2D   = 0xF  /*Virtual unpacked buffers passed via OMX_TI_BUFFERDESCRIPTOR_TYPE */
}OMX_BASE_BUFFER_MEMORY_TYPE;


/*===============================================================*/
/** OMX_BASE_BUF_ELEMENT_ACCESS_TYPE       : Access type for each buffer
 *                                           element. Currently only valid
 *                                           for 2D buffers.
 *
 *  @ param OMX_BASE_MEMORY_ACCESS_IGNORE  : Unspecified. To be used for 1D
 *                                           buffers presently.
 *  @ param OMX_BASE_MEMORY_ACCESS_8BIT    : 8 bit access type.
 *  @ param OMX_BASE_MEMORY_ACCESS_16BIT   : 16 bit access type.
 *  @ param OMX_BASE_MEMORY_ACCESS_32BIT   : 32 bit access type.
 */
/*===============================================================*/
typedef enum OMX_BASE_BUF_ELEMENT_ACCESS_TYPE {
    OMX_BASE_MEMORY_ACCESS_IGNORE   = 0x1,
    OMX_BASE_MEMORY_ACCESS_8BIT,
    OMX_BASE_MEMORY_ACCESS_16BIT,
    OMX_BASE_MEMORY_ACCESS_32BIT,
}OMX_BASE_BUF_ELEMENT_ACCESS_TYPE;



/*===============================================================*/
/** OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_TYPE : The preferences of the
 *                                          component as to who will allocate
 *                                          the buffers.
 *
 *  @ param OMX_BASE_ALLOCATION_DEFAULT   : Default allocation. The derived
 *                                          component does not care which port
 *                                          allocates buffers.
 *  @ param OMX_BASE_ALLOCATION_PREFERRED : Buffers should preferably be
 *                                          allocated by this port. However if
 *                                          client/tunneled port wants to
 *                                          allocate buffers then that it
 *                                          acceptable too.
 *  @ param OMX_BASE_ALLOCATION_MUST      : Buffers must be allocated by this
 *                                          port. If some one else tries to
 *                                          override allocator preferences then
 *                                          a warning will be thrown out.
 *  @ param OMX_BASE_ALLOCATION_NEVER     : Buffers must not be allocated by
 *                                          this port and must come from
 *                                          outside.
 */
/*===============================================================*/
typedef enum OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_TYPE {
    OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_DEFAULT        = 0x1,
    OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_PREFERRED      = 0x2,
    OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_MUST           = 0x3,
    OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_NEVER          = 0x4
}OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_TYPE;



/*===============================================================*/
/** OMX_BASE_EXTRA_DATA_TABLE    : This table contains details of the various
 *                                 extra data types. Derived components can
 *                                 update this table to add new extra data types
 *                                 Any new extra data types also need to be
 *                                 to be added in the OMX_EXTRADATATYPE
 *                                 enumeration after
 *                                 OMX_ExtraDataVendorStartUnused. The table is
 *                                 maintained in omx_base_table.h
 *
 *  @ param eExtraDataType       : This is the extra data type from the
 *                                 OMX_EXTRADATATYPE enumeration.
 *  @ param nSize                : This is the size of the extra data type in
 *                                 bytes. If extra data is of constant
 *                                 size (e.g. a 4 byte integer) then that size
 *                                 is added here. If the extra data consists of
 *                                 packets of data of variable size then the
 *                                 size will be 8 (in the header there will be
 *                                 4 bytes for the pointer to the data and 4
 *                                 bytes for the size of the size of the data)
 *  @ param bPacketData          : Whether the extra data consists of variable
 *                                 sized packets of data. If true then size
 *                                 should be set to 8 bytes.
 */
/*===============================================================*/
typedef struct OMX_BASE_EXTRA_DATA_TABLE {
    OMX_EXTRADATATYPE eExtraDataType;
    OMX_U32           nSize;
    OMX_BOOL          bPacketData;
}OMX_BASE_EXTRA_DATA_TABLE;



/*===============================================================*/
/** OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR : This structure is used to specify
 *                                   any attributes that might be sent/received
 *                                   at runtime during buffer transfer (such as
 *                                   inband control or some codec configuration)
 *
 *  @ param pAttributeData         : The pointer to the attribute data.
 *  @ param nSize                  : Size ofthe attribute data in bytes. While
 *                                   receiving data, size will be an in/out
 *                                   field. If size of the buffer is less than
 *                                   the size of the attribute then the size
 *                                   field is updated and returned.
 */
/*===============================================================*/
typedef struct OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR {
    OMX_PTR pAttributeData;
    OMX_U32 nSize;
}OMX_BASE_CONTROL_ATTRIBUTE_DESCRIPTOR;



/*===============================================================*/
/** OMX_BASE_OUTPUTPORTPRIVATE         : The pOutputPortPrivate field for all
 *                                       buffer headers will be set to this
 *                                       structure by the base component.
 *
 *  @ param pDerivedOutputPortPrivate  : This pointer is available for derived
 *                                       components to set any port private
 *                                       data if required.
 *  @ param nDupBufCount               : This is used internally by the base
 *                                       component to maintain reference count
 *                                       in dup cases. This is NOT supposed to
 *                                       be accessed by the derived component.
 *  @ param pBufCountMutex             : This mutex is used internally by the
 *                                       base to protect the dup count. This is
 *                                       NOT supposed to be accessed by the
 *                                       derived component.
 */
/*===============================================================*/
typedef struct OMX_BASE_OUTPUTPORTPRIVATE {
    OMX_PTR pDerivedOutputPortPrivate;
    OMX_U16 nDupBufCount;
    OMX_PTR pBufCountMutex;
}OMX_BASE_OUTPUTPORTPRIVATE;



/*===============================================================*/
/** OMX_BASE_2D_MEM_PARAMS     : These parameters will be used by base for
 *                               allocating 2D buffers.
 *
 *  @ param nHeight            : Height for the component buffer in pixels.
 *  @ param nWidth             : Width for the component buffer in pixels.
 *  @ param nStride            : Stride for the component buffer.
 *  @ param eAccessMode        : Access mode for the component
 *                               buffer (8bit, 16bit etc.).
 */
/*===============================================================*/
typedef struct OMX_BASE_2D_MEM_PARAMS {
    OMX_U32                          nHeight;
    OMX_U32                          nWidth;
    OMX_U32                          nStride;
    OMX_BASE_BUF_ELEMENT_ACCESS_TYPE eAccessMode;
}OMX_BASE_2D_MEM_PARAMS;



/*===============================================================*/
/** OMX_BASE_PORT_PROPERTIES   : Port properties.
 *
 *  @ param eBufAllocPref      : Buffer allocator preferences on this port.
 *                               DEFAULT setting is default (don't care).
 *  @ param bReadOnlyBuffers   : If true then buffers on this port will be
 *                               read only. Applicable only for o/p buffers.
 *                               DEFAULT setting is true.
 *  @ param nWaterMark         : Watermark level on each port. Derived
 *                               component will get data notification only when
 *                               watermark level is crossed on a port.
 *                               DEFAULT setting is 1.
 *  @ param hBufHeapPerPort    : Heap from which buffers for this port will be
 *                               allocated. Currently supported only for
 *                               default allocation type.
 *                               DEFAULT setting is null.
 *  @ param eDataAccessMode    : Whether frame mode or stream mode data.
 *                               DEFAULT setting is frame mode.
 *  @ param bDMAAccessedBuffer : If true then data buffers on this port will be
 *                               accessed only via DMA. Thus cache operations
 *                               such as invalidate, write back etc.
 *                               will not have to be performed during
 *                               buffer transfer.
 *                               DEFAULT setting is false.
 *  @ param eBufMemoryType     : Buffer memory type.
 *                               DEFAULT setting is default.
 *  @ param nNumComponentBuffers : Number of component buffers per buffer. For
 *                                 e.g. in NV12 format this would be two.
 *                                 DEFAULT setting is 1.
 *  @ param pBufParams           : Buffer parameters. Currently required only
 *                                 for 2D buffers so OMX_BASE_2D_MEM_PARAMS
 *                                 should be used. This is a pointer so that
 *                                 an array with number of elemets equal to
 *                                 number of component buffers can be created.
 *                                 For e.g. if there are 2 component buffers
 *                                 then this should be allocated to size
 *                                 (2 * OMX_BASE_2D_MEM_PARAMS). Then
 *                                 pBufParams[0] will have params of Buf0 and
 *                                 pBufParams[1] will have params of Buf1.
 *                                 DEFAULT setting is NULL.
 */
/*===============================================================*/
typedef struct OMX_BASE_PORT_PROPERTIES {
    OMX_BASE_BUFFER_ALLOCATION_PREFERENCE_TYPE eBufAllocPref;
    OMX_BOOL                                   bReadOnlyBuffers;
    OMX_U32                                    nWatermark;
    OMX_PTR                                    hBufHeapPerPort;
    OMX_BASE_DATA_ACCESS_MODE_TYPE             eDataAccessMode;
    OMX_BOOL                                   bDMAAccessedBuffer;
    OMX_U32                                    nNumMetaDataFields;
    OMX_EXTRADATATYPE                         *pMetaDataFieldTypesArr;
    OMX_BASE_BUFFER_MEMORY_TYPE                eBufMemoryType;
    OMX_U32                                    nNumComponentBuffers;
    OMX_PTR                                    pBufParams;
    OMX_U32                                    nTimeoutForDequeue;
}OMX_BASE_PORT_PROPERTIES;



/** OMX_BASE_PARAMSFROMDERIVED :
 *  This structure contains the params required to Initialize Base component
 *  @param cComponentName    :  Name of the Component
 *  @param nComponentVersion :  version of the component
 *  @param ctxType           :  type of context(Active/Passive)
 *  @param cTaskName         :  Task name
 *  @param nStackSize        :  stack size incase of Active context
 *  @param nPrioirty         :  task prioirty incase of Active context
 *  @param nGroupID          :  unique ID for all components in the same group
 *  @param nGroupPriority    :  priority associated with a group of components
 *  @param pAudioPortParams  :  Audio port parameters info
 *  @param pVideoPortParams  :  Video port parameters info
 *  @param pImagePortParams  :  Image port parameters info
 *  @param pOtherPortParams  :  other port parameters info
 *  @param nNumPorts         :  Total number of ports of the component
 *  @param nMinStartPortIndex:  Minimum start port index
 *  @param hDefaultHeap      :  Default heap to be used for all allocations
 *  @param bNotifyForAnyPort :  If set to true then derived component will get
 *                              data notifiation whenever watermark level is
 *                              reached on any port. If false then data
 *                              notification will be sent only if watermark is
 *                              reached for all ports.
 *                              DEFAULT setting is true.
 *  @param nNumMetaDataFields:  Number of meta data fields that will be used
 *                              by this component.
 *                              DEFAULT setting is 0.
 *  @ param MetaDataFieldTypesArr: Array of the meta field types. The fields
 *                                 in this array should be in the same order
 *                                 as in the extended buffer header.
 *                                 DEFAULT setting is null.
 *  @param fpCommandNotify   :  Base to Derived hook to notify commands
 *  @param fpDataNotify      :  Base to derived hook to notify data
 */
typedef struct OMX_BASE_PARAMSFROMDERIVED {
    OMX_STRING                  cComponentName;
    OMX_VERSIONTYPE             nComponentVersion;
    OMX_BASE_CONTEXT_TYPE       ctxType;
    OMX_U8                      cTaskName[OMX_BASE_MAXNAMELEN];
    OMX_U32                     nStackSize;
    OMX_U32                     nPrioirty;
    OMX_U32                     nGroupID;
    OMX_U32                     nGroupPriority;
    OMX_PORT_PARAM_TYPE        *pAudioPortParams;
    OMX_PORT_PARAM_TYPE        *pVideoPortParams;
    OMX_PORT_PARAM_TYPE        *pImagePortParams;
    OMX_PORT_PARAM_TYPE        *pOtherPortParams;
    OMX_U32                     nNumPorts;
    OMX_U32                     nMinStartPortIndex;
    OMX_PTR                     hDefaultHeap;
    OMX_BASE_PORT_PROPERTIES * *pPortProperties;
    OMX_BOOL                    bNotifyForAnyPort;
    OMX_ERRORTYPE (*fpCommandNotify)(OMX_HANDLETYPE hComponent,
                                     OMX_COMMANDTYPE Cmd,
                                     OMX_U32 nParam, OMX_PTR pCmdData);
    OMX_ERRORTYPE (*fpDataNotify)(OMX_HANDLETYPE hComponent);

}OMX_BASE_PARAMSFROMDERIVED;



/** OMX_BASE_PRIVATETYPE "
 *  This structure contains the elements required for a omx component
 *  @param hOMXBaseInt         : handle to access internals of Base
 *  @param pMark               : pointer to mark data
 *  @param tDerToBase          : derived to base parameters
 *  @param nNumPorts           : number of ports
 *  @param nMinStartPortIndex  : minimum start port index of component
 *  @param pPortdefs           : pointer to access the portdef params
 *  @param tCurState           : current state of a component
 *  @param tNewState           : new state of a component
 *  @param pMutex              : mutex to avoid race conditions
 *  @param bResourceSuspended   : Flag to indicate suspend action triggered or not
 *  @param bResourceInUse       : Flag to indicate pSemaphoreSuspendResume is aquired
 *  @param pMutexResourceSuspended  : mutex for bResourceSuspended
 *  @param pSemaphoreSuspendResume  : semaphore to hold the suspend action
 *  @param fpReturnEventNotify : Derived to base hook to notify incase
 *                               of any Event of interest occurs
 *
 *  @param fpDioGetCount       : hook to get count of number of buffers
 *                               param hComponent [in] - Handle of the component
 *                               param nPortIndex [in] - Index of the port.
 *                               param pCount [out]    - Count of the buffers
 *                                                       available.
 *
 *  @param fpDioDequeue        : hook to dequeue a buffer header from internal
 *                               queue for processing. The caller provides
 *                               a local buffer header which the DIO layer
 *                               can read for certain parameters such as
 *                               buffer size required. Rest of the parameters
 *                               in the available header are then copied to
 *                               the local header to be used by the caller.
 *                               NOTE: If OMX_WarningAttributPending is returned
 *                                     then the derived component is supposed
 *                                     to retreive the attribute first using
 *                                     Get Attribute control command. Dequeue
 *                                     will continue to return error until this
 *                                     is done.
 *                               param hComponent [in]      - Handle of the
 *                                                            component.
 *                               param nPortIndex [in]      - Index of the port.
 *                               param pBuffHeader [in out] - Buffer header.
 *
 *  @param fpDioSend           : hook to send the buffer. The caller will
 *                               provide a local buffer header which will be
 *                               copied and the copy will be sent across.
 *                               param hComponent [in]  - Handle of the
 *                                                        component.
 *                               param nPortIndex [in]  - Index of the port.
 *                               param pBuffHeader [in] - Buffer header to be
 *                                                        sent.
 *
 *  @param fpDioDup            : hook to duplicate a buffer header. Used when
 *                               the caller wants to hold on to a buffer and
 *                               share with others. The duplicate buffer
 *                               header will be a local header provided by
 *                               the caller. After returning, the caller
 *                               can send the original buffer and retain
 *                               the duplicate, to be canceled later.
 *                               param hComponent [in]         - Handle of the
 *                                                               component.
 *                               param nPortIndex [in]         - Index of the
 *                                                               port.
 *                               param pBuffHeader [in]        - Buffer header
 *                                                               to be
 *                                                               duplicated.
 *                               param pDupBuffHeader [in out] - Duplicate
 *                                                               buffer header.
 *
 *  @param fpDioCancel         : hook to cancel the buffer.
 *                               The next dequeue call will
 *                               return the same buffer.
 *                               param hComponent [in]  - Handle of the
 *                                                        component.
 *                               param nPortIndex [in]  - Index of the port.
 *                               param pBuffHeader [in] - Buffer header to be
 *                                                        canceled.
 *
 *  @param fpDioControl        : hook for control commands
 *                               param hComponent [in] - Handle of the component
 *                               param nPortIndex [in] - Index of the port.
 *                               param nCmdType [in]   - Control command.
 *                               param pParams         - Any control parameters.
 *
 */

STRUCT(OMX_BASE_PRIVATETYPE)
 #define OMX_BASE_PRIVATETYPE_FIELDS                                 \
    OMX_PTR hOMXBaseInt;                                                \
    OMX_MARKTYPE *pMark;                                                \
    OMX_BASE_PARAMSFROMDERIVED tDerToBase;                              \
    OMX_PARAM_PORTDEFINITIONTYPE * *pPortdefs;                           \
    OMX_TI_PARAM_BUFFERPREANNOUNCE * *pBufPreAnnouncePerPort;            \
    OMX_STATETYPE tCurState;                                            \
    OMX_STATETYPE tNewState;                                            \
    OMX_PTR pMutex;                                                     \
    OMX_BOOL bResourceSuspended;                                        \
    OMX_BOOL bResourceInUse;                                            \
    OMX_PTR pMutexResourceSuspended;                                    \
    OMX_PTR pSemaphoreSuspendResume;                                    \
    OMX_ERRORTYPE (*fpReturnEventNotify)(OMX_HANDLETYPE hComponent,     \
                                         OMX_EVENTTYPE eEvent,          \
                                         OMX_U32 nEventData1,           \
                                         OMX_U32 nEventData2,           \
                                         OMX_PTR pEventData);           \
    OMX_ERRORTYPE (*fpDioGetCount)(OMX_HANDLETYPE hComponent,          \
                                   OMX_U32 nPortIndex,                    \
                                   OMX_U32 *pCount);                      \
    OMX_ERRORTYPE (*fpDioDequeue)(OMX_HANDLETYPE hComponent,           \
                                  OMX_U32 nPortIndex,                     \
                                  OMX_PTR pBuffHeader);                   \
    OMX_ERRORTYPE (*fpDioSend)(OMX_HANDLETYPE hComponent,              \
                               OMX_U32 nPortIndex,                        \
                               OMX_PTR pBuffHeader);                      \
    OMX_ERRORTYPE (*fpDioDup)(OMX_HANDLETYPE hComponent,               \
                              OMX_U32 nPortIndex,                       \
                              OMX_PTR pBuffHeader,                      \
                              OMX_PTR pDupBuffHeader);                  \
    OMX_ERRORTYPE (*fpDioCancel)(OMX_HANDLETYPE hComponent,            \
                                 OMX_U32 nPortIndex,                    \
                                 OMX_PTR pBuffHeader);                  \
    OMX_ERRORTYPE (*fpDioControl)(OMX_HANDLETYPE hComponent,           \
                                  OMX_U32 nPortIndex,                       \
                                  OMX_BASE_DIO_CTRLCMD_TYPE nCmdType,       \
                                  OMX_PTR pParams);                         \
    OMX_ERRORTYPE (*fpDioUtil)(OMX_HANDLETYPE hComponent,              \
                               OMX_U32 nPortIndex,                       \
                               OMX_BASE_DIO_UTIL_TYPE nUtilType,         \
                               OMX_PTR pParams);
ENDSTRUCT(OMX_BASE_PRIVATETYPE)



/*----------         function prototypes      ------------------- */

/*===============================================================*/
/** @fn OMX_BASE_IsCmdPending  : This function returns true if there is any
 *                               command in the command pipe. This function is
 *                               supposed to be called by derived components
 *                               before every data process so that if there is
 *                               any command pending then the derived component
 *                               should return from data notify function
 *                               immediately without processing data so that
 *                               command call may be processed.
 *                               This function is defined in base_internal.c
 *
 *  @ param hComponent         : Component handle.
 */
/*===============================================================*/
OMX_BOOL OMX_BASE_IsCmdPending (OMX_HANDLETYPE hComponent);



/*===============================================================*/
/** @fn OMX_BASE_SetDefaultProperties    : Sets default values for the
 *                                         following properties in derived to
 *                                         base parameters.
 *                                         - pPortProperties
 *                                         - bNotifyForAnyPort
 *                                         - nNummetaDataTypes
 *                                         - pArrayOfmetaDataTypes
 *                                         DEFAULT setting for these paramaters
 *                                         are mentioned along with their
 *                                         definition
 *
 *  @ PreRequisites                      : The parameter nNumPorts needs to be
 *                                         set by the derived component before
 *                                         calling this function. Also all
 *                                         memory required will have to be
 *                                         allocated by the derived component
 *                                         before calling this function. This
 *                                         function won't allocate any memory
 *
 *  @ params hComponent                  : Component handle.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SetDefaultProperties(OMX_HANDLETYPE hComponent);



/*----------         Standard Functions      ------------------- */

/*The derived component can override any of the below base functions. Comments
  indicate which functions need to overridden necessarily and which can be
  safely handled by omx base*/

/*===============================================================*/
/** @fn OMX_BASE_ComponentInit :
 *      This method is the first call into a Base component by
 *      Derived componnet. Derived component should fill all
 *      create parameters of an Base Componet before this call.
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent);

/*===============================================================*/
/** @fn OMX_BASE_SetCallbacks :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_CALLBACKTYPE *pCallbacks,
                                    OMX_IN OMX_PTR pAppData);

/*===============================================================*/
/** @fn OMX_BASE_GetComponentVersion :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                           OMX_OUT OMX_STRING pComponentName,
                                           OMX_OUT OMX_VERSIONTYPE *pComponentVersion,
                                           OMX_OUT OMX_VERSIONTYPE *pSpecVersion,
                                           OMX_OUT OMX_UUIDTYPE *pComponentUUID);

/*===============================================================*/
/** @fn OMX_BASE_SendCommand :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_COMMANDTYPE Cmd,
                                   OMX_IN OMX_U32 nParam1,
                                   OMX_IN OMX_PTR pCmdData);

/*===============================================================*/
/** @fn OMX_BASE_GetParameter :
 *  This function supports mandatory port parameters of a
 *  standard component.Derived comp must override this and
 *  call this only for mandatory params of standard component.
 *
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nParamIndex,
                                    OMX_INOUT OMX_PTR pParamStruct);

/*===============================================================*/
/** @fn OMX_BASE_SetParameter :
 *  This function supports only mandatory port parameters of a
 *  standard component.Derived comp must override this and
 *  call this only for mandatory params of standard component
 *
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nIndex,
                                    OMX_IN OMX_PTR pParamStruct);

/*===============================================================*/
/** @fn OMX_BASE_GetConfig :
 *  This must be overridden by derived component to handle all
 *  configurations supported by component
 *
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                 OMX_IN OMX_INDEXTYPE nIndex,
                                 OMX_INOUT OMX_PTR pComponentConfigStructure);

/*===============================================================*/
/** @fn OMX_BASE_SetConfig :
 *  This must be overridden by derived component to handle all
 *  configurations supported by component. It should also be called for certain
 *  base configurations.
 *
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                 OMX_IN OMX_INDEXTYPE nIndex,
                                 OMX_IN OMX_PTR pComponentConfigStructure);

/*===============================================================*/
/** @fn OMX_BASE_GetExtensionIndex :
 *  This function should be overridden
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_STRING cParameterName,
                                         OMX_OUT OMX_INDEXTYPE *pIndexType);

/*===============================================================*/
/** @fn OMX_BASE_GetState :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_OUT OMX_STATETYPE *pState);

/*===============================================================*/
/** @fn OMX_BASE_ComponentTunnelRequest :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_ComponentTunnelRequest(OMX_IN OMX_HANDLETYPE hComp,
                                              OMX_IN OMX_U32 nPort,
                                              OMX_IN OMX_HANDLETYPE hTunneledComp,
                                              OMX_IN OMX_U32 nTunneledPort,
                                              OMX_INOUT OMX_TUNNELSETUPTYPE *pTunnelSetup);

/*===============================================================*/
/** @fn OMX_BASE_UseBuffer :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                 OMX_INOUT OMX_BUFFERHEADERTYPE * *ppBufferHdr,
                                 OMX_IN OMX_U32 nPortIndex,
                                 OMX_IN OMX_PTR pAppPrivate,
                                 OMX_IN OMX_U32 nSizeBytes,
                                 OMX_IN OMX_U8 *pBuffer);

/*===============================================================*/
/** @fn OMX_BASE_AllocateBuffer :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_INOUT OMX_BUFFERHEADERTYPE * *ppBuffer,
                                      OMX_IN OMX_U32 nPortIndex,
                                      OMX_IN OMX_PTR pAppPrivate,
                                      OMX_IN OMX_U32 nSizeBytes);

/*===============================================================*/
/** @fn OMX_BASE_FreeBuffer :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                  OMX_IN OMX_U32 nPortIndex,
                                  OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

/*===============================================================*/
/** @fn OMX_BASE_EmptyThisBuffer :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

/*===============================================================*/
/** @fn OMX_BASE_FillThisBuffer :
 *  This function need not be overriden by the derived comopnent.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

/*===============================================================*/
/** @fn OMX_BASE_ComponentDeinit : This function must be overridden and should
 *  be called from the derived implementation.
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_ComponentDeinit(OMX_IN OMX_HANDLETYPE hComponent);

/*===============================================================*/
/** @fn OMX_BASE_UseEGLImage : This must be overridden
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_UseEGLImage(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_INOUT OMX_BUFFERHEADERTYPE * *ppBufferHdr,
                                   OMX_IN OMX_U32 nPortIndex,
                                   OMX_IN OMX_PTR pAppPrivate,
                                   OMX_IN void *eglImage);

/*===============================================================*/
/** @fn OMX_BASE_ComponentRoleEnum : This must be overridden
 *  see omx_core.h
 */
/*===============================================================*/
OMX_ERRORTYPE OMX_BASE_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_OUT OMX_U8 *cRole,
                                         OMX_IN OMX_U32 nIndex);


/*******************************************************************
 * PRIVATE DECLARATIONS: defined here, used only here
 *******************************************************************/
/*----------          data declarations        ------------------- */
/*----------          function prototypes      ------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_BASE_H_ */

