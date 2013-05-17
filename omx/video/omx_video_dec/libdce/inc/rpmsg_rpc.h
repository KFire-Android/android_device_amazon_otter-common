/*
 * Remote Processor Procedure Call Driver
 *
 * Copyright(c) 2012-2013 Texas Instruments. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name Texas Instruments nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _RPMSG_RPC_H_
#define _RPMSG_RPC_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stddef.h>

#define __packed __attribute__ ((packed))

#define RPPC_IOC_MAGIC		'r'
#define RPPC_IOC_CREATE		_IOW(RPPC_IOC_MAGIC, 1, struct rppc_create_instance)
/* TODO: these may not be needed */
#define RPPC_IOC_BUFREGISTER    _IOW(RPPC_IOC_MAGIC, 2, int)
#define RPPC_IOC_BUFUNREGISTER  _IOW(RPPC_IOC_MAGIC, 3, int)
#define RPPC_IOC_MAXNR		(4)

#define RPPC_MAX_PARAMETERS	(10)
#define RPPC_MAX_TRANSLATIONS	(1024)

enum rppc_param_type {
	RPPC_PARAM_TYPE_UNKNOWN = 0,
	/* An atomic data type, 1 byte to architecture limit sized bytes */
	RPPC_PARAM_TYPE_ATOMIC,
	/*
	 * A pointer to shared memory. The reserved field must contain
	 * the handle to the memory
	 */
	RPPC_PARAM_TYPE_PTR,
	/*
	 * (Unsupported) A structure type. Will be architecure width
	 * aligned in memory.
	 */
	RPPC_PARAM_TYPE_STRUCT,
};

struct rppc_param_translation {
	/* The parameter index which indicates which is the base pointer */
	uint32_t  index;
	/* The offset from the base address to the pointer to translate */
	ptrdiff_t offset;
	/*
	 * The base user virtual address of the pointer to translate
	 * (used to calculate translated pointer offset).
	 */
	size_t base;
	/* reserved field */
	size_t reserved;
};

struct rppc_param {
	uint32_t type;		/* rppc_param_type */
	size_t size;		/* The size of the data */
	size_t data;		/* Either the pointer to the data or
				the data itself */
	size_t base;		/* If a pointer is in data, this is the base
				pointer (if data has an offset from base). */
	size_t reserved;	/* Shared Memory Handle (used only with ptrs) */
};

struct rppc_function {
	/* The function to call */
	uint32_t fxn_id;
	/* The number of parameters in the array. */
	uint32_t num_params;
	/* The array of parameters */
	struct rppc_param params[RPPC_MAX_PARAMETERS];
	/* The number of translations needed in the offsets array */
	uint32_t num_translations;
	/*
	 * An indeterminate length array of offsets within
	 * payload_data to pointers which need translation
	 */
	struct rppc_param_translation translations[0];
};

struct rppc_function_return {
	uint32_t fxn_id;
	uint32_t status;
};

#define RPPC_DESC_EXEC_SYNC	(0x0100)
#define RPPC_DESC_TYPE_MASK	(0x0F00)

/* TODO: Remove the relative offset */
/** The remote functions are offset by one relative to the client */
#define RPPC_SET_FXN_IDX(idx)	(((idx) + 1) | 0x80000000)

/** The remote functions are offset by one relative to the client */
#define RPPC_FXN_MASK(idx)	(((idx) - 1) & 0x7FFFFFFF)

/* TODO: remove or mask unneeded fields for RFC */
/** This is actually a frankensteined structure of RCM */
struct rppc_packet {
	uint16_t desc;		/* RcmClient_Packet.desc */
	uint16_t msg_id;	/* RcmClient_Packet.msgId */
	uint32_t flags;		/* RcmClient_Message.jobId & poolId */
	uint32_t fxn_id;	/* RcmClient_Message.fxnIdx */
	int32_t  result;	/* RcmClient_Message.result */
	uint32_t data_size;	/* RcmClient_Message.data_size */
	uint8_t  data[0];	/* RcmClient_Message.data pointer */
} __packed;


//#ifdef __KERNEL__

#define RPPC_MAX_NUM_FUNCS		(1024)
#define RPPC_MAX_CHANNEL_NAMELEN	(64)
#define RPPC_MAX_FUNC_NAMELEN		(64)
#define RPPC_MAX_NUM_PARAMS		(10)
#define RPPC_MAX_INST_NAMELEN		(48)

/* Added below definition for use with CREATE ioctl in QNX */
struct rppc_create_instance {
	char name[RPPC_MAX_CHANNEL_NAMELEN];
};

/** The parameter direction as relative to the function it describes */
enum rppc_param_direction {
	RPPC_PARAMDIR_IN = 0,
	RPPC_PARAMDIR_OUT,
	RPPC_PARAMDIR_BI,
	RPPC_PARAMDIR_MAX
};

enum rppc_param_datatype {
	RPPC_PARAM_VOID = 0,
	RPPC_PARAM_S08,
	RPPC_PARAM_U08,
	RPPC_PARAM_S16,
	RPPC_PARAM_U16,
	RPPC_PARAM_S32,
	RPPC_PARAM_U32,
	RPPC_PARAM_S64,
	RPPC_PARAM_U64,
	RPPC_PARAM_ATOMIC_MAX,

	RPPC_PARAM_MASK = 0x7F,
	RPPC_PARAM_PTR = 0x80, /**< Logically OR'd with lower type to make a
				    pointer to the correct type */
	RPPC_PARAM_MAX
};

#define RPPC_PTR_TYPE(type)	(type | RPPC_PARAM_PTR)
#define RPPC_IS_PTR(type)	(type & RPPC_PARAM_PTR)
#define RPPC_IS_ATOMIC(type)	((type > RPPC_PARAM_VOID) && \
					(type < RPPC_PARAM_ATOMIC_MAX))

//#endif /* __KERNEL__ */

#endif /* _RPMSG_RPC_H_ */
