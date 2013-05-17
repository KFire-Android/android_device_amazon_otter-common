/* ====================================================================
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
 * ==================================================================== */
/*
*   @file  profile.h
*   This file contains the function prototypes of the instrumentation
*   and the flags for defining the functionality
*
*   @path \WTSD_DucatiMMSW\platform\utils\
*
*  @rev 1.0
*/
#ifndef _PROFILE_H
#define _PROFILE_H

#ifdef BUILD_PSI_KPI

#define PSI_KPI                 /* add the instrumentation (cpu load and allowing tracing) */

#endif /*BUILD_PSI_KPI*/


#if 0
#include "omx_types.h"
#include "omx_core.h"
#include "omx_component.h"
#endif

#define IVA_DETAILS             /* for tracing IVA events */
//#define OMX_DETAILS             /* for tracing OMX events */
#define PHYSICAL_BUFFER         /* print physical buffer address for OMX events */
#define CPU_LOAD_DETAILS        /* for tracing CPU IDLE % each frame */
//#define CINIT_ENABLE_DUCATI_LOAD  /* measure starts when codec is created */
//#define OMX_ENABLE_DUCATI_LOAD  /* measure starts when OMX component is created */

#define USE_CTM_TIMER           /* CTM vs 32K for measuring CPU load */
#define INST_COST               /* measure the instrumentation cost during the test */
//#define COST_AFTER            /* allows estimating the instrumentation cost after completion */

/* Instrumentation Flags control */
typedef enum {
    KPI_END_SUMMARY = (1 << 0),  /* print IVA and Ducati/Benelli summary at end of use case */
    KPI_IVA_DETAILS = (1 << 1),  /* print IVA trace during the use case */
//    KPI_OMX_DETAILS = (1 << 2),  /* print OMX trace during the use case */
    KPI_CPU_DETAILS = (1 << 3)   /* print Idle trace during the use case */
} KPI_inst_type;

/* Instrumentation internal Status */
typedef enum {
    KPI_INST_RUN  = (1 << 0),    /* instrumentation running */
    KPI_CPU_LOAD  = (1 << 1),    /* ducati/benelly load on */
    KPI_IVA_LOAD  = (1 << 2),    /* IVA load on */
    KPI_IVA_TRACE = (1 << 3),    /* IVA trace is active */
//    KPI_OMX_TRACE = (1 << 4),    /* OMX trace is active */
    KPI_CPU_TRACE = (1 << 5),    /* CPU trace is active */
    KPI_IVA_USED  = (1 << 6)     /* IVA used in the test */
} KPI_inst_status;

/* function protypes */
extern void kpi_set_chipset_id  (uint32_t chipset_id);
extern void kpi_instInit        (void);
extern void kpi_instDeinit      (void);
extern void kpi_before_codec    (void);
extern void kpi_after_codec     (void);

#if 0
extern void kpi_omx_comp_init   (OMX_HANDLETYPE hComponent);
extern void kpi_omx_comp_deinit (OMX_HANDLETYPE hComponent);
extern void kpi_omx_comp_FTB    (OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffer);
extern void kpi_omx_comp_ETB    (OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffer);
extern void kpi_omx_comp_FBD    (OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffer);
extern void kpi_omx_comp_EBD    (OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffer);
#endif

/* use disableCoreInts in SMP: faster and appropriate for us, otherwise is disable (non SMP) */
#ifndef BUILD_FOR_SMP
#define  Hwi_disableCoreInts(   ) Hwi_disable(   )
#define  Hwi_restoreCoreInts( x ) Hwi_restore( x )
#endif//BUILD_FOR_SMP

#endif /*_PROFILE_H*/

