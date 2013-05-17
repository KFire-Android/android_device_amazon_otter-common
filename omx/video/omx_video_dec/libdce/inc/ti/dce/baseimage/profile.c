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
*   @file  profile.c
*   This file contains kpi psi instrumentation code for cpu load as well
*   as tracing codec + OMX events
*
*   @path \WTSD_DucatiMMSW\platform\utils\
*
*  @rev 1.0
*/

// Hacks for integration into DCE.
#include <stdio.h>
#include <xdc/runtime/System.h>
#define BUILD_FOR_SMP
#define TIMM_OSAL_TRACEGRP_SYSTEM 0
#define TIMM_OSAL_ErrorExt PSI_TracePrintf
typedef int TIMM_OSAL_TRACEGRP;
typedef char TIMM_OSAL_CHAR;
typedef unsigned char OMX_U8;
typedef unsigned long OMX_U32;

/* PSI_KPI profiler */
#include "profile.h"

#if 0
#include "timm_osal_trace.h"
#include "timm_osal_mutex.h"
#endif

#include <string.h>
#include <xdc/std.h>
#include <xdc/runtime/Error.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Swi.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>
#ifdef  BUILD_FOR_SMP
#include <ti/sysbios/hal/Core.h>
#endif//BUILD_FOR_SMP
#include <ti/sysbios/family/arm/ducati/TimestampProvider.h>
#include <ti/pm/IpcPower.h>
//#include <WTSD_DucatiMMSW/framework/tools_library/inc/tools_time.h>


/* private function prototypes */
void kpi_IVA_profiler_init  (void);
void kpi_CPU_profiler_init  (void);
void kpi_IVA_profiler_print (void);
void kpi_CPU_profiler_print (void);
void psi_kpi_task_test(Task_Handle prev, Task_Handle next);


/***************************************************************
 * psi_tsk_info
 * -------------------------------------------------------------
 * Structure maintaining task data for Duacti CPU load
 *
 ***************************************************************/
#define KPI_CONTEXT_TASK  0
#define KPI_CONTEXT_SWI   1
#define KPI_CONTEXT_HWI   2

typedef struct {
  void          *handle;      /* Pointer to task handle, used to identify task */
  char           name[50];    /* Task name */
  unsigned long  total_time;  /* Total time spent in the task context */
  unsigned long  nb_switch;   /* Number of times the context get activated */
} psi_context_info;


/***************************************************************
 * psi_bios_kpi
 * -------------------------------------------------------------
 * Structure maintaining data for Ducati CPU load
 *
 ***************************************************************/
#define CTX_DIRECT_NUM   256
#define KPI_MAX_NB_TASKS 64
#define KPI_MAX_NB_SWI   16
#define KPI_MAX_NB_HWI   16
#define MAX_NB_IT 32

typedef struct {
  unsigned long    prev_t32;                      /* Store T32K value of previous task switch */
  unsigned long    total_time;                    /* Total processing time */
  unsigned long    kpi_time;                      /* Total instrumentation processing time */
  unsigned long    kpi_count;                     /* Number of time instrumentation run */
  unsigned long    nb_switch;                     /* Number of context switches */

  psi_context_info tasks[KPI_MAX_NB_TASKS];       /* Tasks info DB */
  unsigned long    nb_tasks;                      /* Number of tasks in task info */
  psi_context_info hwi[KPI_MAX_NB_SWI];           /* HWI info DB */
  unsigned long    nb_hwi;                        /* Number of HWI in HWI info */
  psi_context_info swi[KPI_MAX_NB_HWI];           /* SWI info DB */
  unsigned long    nb_swi;                        /* Number of SWI in SWI info */

  psi_context_info* context_cache[CTX_DIRECT_NUM];/* pointers to tasks info DB */

  void*            context_stack[MAX_NB_IT];      /* to keep handles because of swi hwi */
  void**           pt_context_stack;              /* stack pointer */

  unsigned long   *ptr_idle_total;                /* to access to idle task total directly */
  unsigned long    idle_prev;                     /* total ticks of idle task previously */
  unsigned long    idle_time_prev;                /* time of idle_prev measure */

  unsigned long    trace_time;                    /* time spent tracing */
} psi_bios_kpi;


/***************************************************************
 * ivahd_kpi
 * -------------------------------------------------------------
 * Structure maintaining data for IVA-HD load and fps
 *
 ***************************************************************/
typedef struct {
  unsigned long ivahd_t_tot;         /* IVA-HD tot processing time per frame */
  unsigned long ivahd_t_max;         /* IVA-HD max processing time per frame */
  unsigned long ivahd_t_min;         /* IVA-HD min processing time per frame */
  unsigned long ivahd_t_max_frame;
  unsigned long ivahd_t_min_frame;

  unsigned long omx_nb_frames;       /* Number of frames      */

  unsigned long before_time;         /* T32K before codec execution */
  unsigned long after_time;          /* T32K after codec execution */

  unsigned long t32k_start;          /* T32K value at the beginning of video decode */
  unsigned long t32k_end;            /* T32K value at the end of video decode */
} psi_iva_kpi;


/***************************************************************
 * Globals
 ***************************************************************/
unsigned long kpi_control = 0;       /* instrumentation control (set with omapconf) */
unsigned long kpi_status  = 0;       /* instrumentation status variables */

psi_iva_kpi   iva_kpi;               /* IVA data base */
psi_bios_kpi  bios_kpi[2];           /* CPU data base (2 cores) */


/***************************************************************
 * Functions
 ***************************************************************/

/* Hacks for integration into the DCE image load reporting. */
unsigned long get_ivahd_t_tot(void)
{
    return iva_kpi.ivahd_t_tot;
}

/* You'd better protect calls to those with a GateAll_enter/leave, if you want
 * sane values reported, as the sum computation is not atomic. */
unsigned long get_core_total(unsigned core)
{
    unsigned long total = 0;
    psi_bios_kpi *core_kpi = &bios_kpi[core];
    int i;

    for (i = 0; i < core_kpi->nb_tasks; i++)
        total += core_kpi->tasks[i].total_time;

    /* Subtract idle task time from total. */
    total -= *(core_kpi->ptr_idle_total);

    for (i = 0; i < core_kpi->nb_swi; i++)
        total += core_kpi->swi[i].total_time;

    for (i = 0; i < core_kpi->nb_hwi; i++)
        total += core_kpi->hwi[i].total_time;

    return total;
}

/*
 * Functions to determine omap type.
 * (Default to OMAP4)
 */
static uint32_t kpi_chipset_id = 0;

static int omap_class(void)
{
	return (kpi_chipset_id >> 12) & 0xf;
}

static int is_omap5(void)
{
	return (omap_class() == 5);
}

/*
 * Set chipset id.
 * Call this ASAP.
 */
void kpi_set_chipset_id(uint32_t chipset_id)
{
	kpi_chipset_id = chipset_id;
	System_printf("kpi chipset: 0x%x\n", kpi_chipset_id);
}

/***************************************************************
 * get_32k
 * -------------------------------------------------------------
 * Function used to get 32k timer value
 *
 * @params: none
 *
 * @return: T32k value
 *
 ***************************************************************/
unsigned long get_32k(void)
{
	switch (omap_class()) {
	case 4:
		return *((volatile OMX_U32 *)0xAA304010);

	case 5:
		return *((volatile OMX_U32 *)0xAAE04030);

	default:
		System_printf("get_32k: unknown chipset 0x%x!\n", kpi_chipset_id);
		return 0;
	}
}

/***************************************************************
 * get_time
 * -------------------------------------------------------------
 * Left here for test
 *
 * @params: none
 *
 * @return: CTM timer value / 4096 (~100KHz)
 *          or 32K value (slower to read)
 *
 ***************************************************************/
inline unsigned long get_time( void )
#ifdef USE_CTM_TIMER
{
  Types_Timestamp64 tTicks;

  TimestampProvider_get64( &tTicks );
  return ( (unsigned long) (tTicks.lo >> 12) + (unsigned long) (tTicks.hi << 20) );
}
#else/*USE_CTM_TIMER*/
{
  return get_32k();
}
#endif/*USE_CTM_TIMER*/

/***************************************************************
 * get_time_core
 * -------------------------------------------------------------
 * Left here for test
 *
 * @params: int core
 *
 * @return: CTM timer value / 4096 (~100KHz)
 *          or 32K value (slower to read)
 *
 * This function is equivalent to get_time().
 * it can be called when interrupts have been masked
 * and when current core Id is known already (therefore faster)
 ***************************************************************/
/*inline*/ unsigned long get_time_core( int core )
#ifdef USE_CTM_TIMER
{
  Types_Timestamp64 tTicks;

    tTicks.hi = *(volatile unsigned long *) (0x40000400 + 0x180 + 0xc + (core<<3));
    tTicks.lo = *(volatile unsigned long *) (0x40000400 + 0x180 + 0x8 + (core<<3));

  return ( (unsigned long) (tTicks.lo >> 12) + (unsigned long) (tTicks.hi << 20) );
}
#else /*USE_CTM_TIMER*/
{
  return get_32k();
}
#endif/*USE_CTM_TIMER*/

/***************************************************************
 * set_WKUPAON
 * -------------------------------------------------------------
 * Function used to get fast access to the 32k timer value
 *
 * @params: int force_restore (1 or 0)
 *
 * @return: none
 *
 * This function prevents sleep mode to the L4 WKUPAON_CM
 * register, to avoid large stalls when reading the 32k timer
 ***************************************************************/
void set_WKUPAON(int force_restore)
{
	if (is_omap5()) {
		static unsigned long clktrctrl_01=0;
		unsigned long reg = *((volatile OMX_U32 *)0xAAE07800);

		/* Force nosleep mode or restore original configuration */
		if( force_restore == 1 ) {
			clktrctrl_01 = reg & 0x3;         /* save clktrctrl */
			reg &= 0xfffffffc;
			//reg |= 0x2;                     /* force SW_WAKEUP */
			reg |= 0;                         /* force NO_SLEEP */
		} else {
			reg &= 0xfffffffc;
			reg |= clktrctrl_01;              /* restore bits 01 */
			clktrctrl_01 = 0;                 /* restore done */
		}
		*(OMX_U32 *)0xAAE07800 = reg;
	}
}

/***************************************************************
 * PSI_TracePrintf
 * -------------------------------------------------------------
 * Function used to get 32k timer value
 *
 * @params: TIMM_OSAL_TRACEGRP eTraceGrp, TIMM_OSAL_CHAR *pcFormat, ...
 *
 * @return: none
 *
 ***************************************************************/
void PSI_TracePrintf(TIMM_OSAL_TRACEGRP eTraceGrp, TIMM_OSAL_CHAR *pcFormat, ...)
{
//  static TIMM_OSAL_PTR MyMutex = NULL;
  unsigned long tstart = get_time();
  unsigned long key = Task_disable();
  unsigned long CoreId = Core_getId();

//  if(!MyMutex) TIMM_OSAL_MutexCreate( &MyMutex );
//  TIMM_OSAL_MutexObtain(MyMutex, TIMM_OSAL_SUSPEND);

  va_list varArgs;
  va_start(varArgs, pcFormat);
  System_vprintf(pcFormat, varArgs);
// TIMM_OSAL_TracePrintfV(eTraceGrp, pcFormat, varArgs);
  va_end(varArgs);

//  TIMM_OSAL_MutexRelease(MyMutex);
  Task_restore(key);

  /* count overhead due to tracing when running (phase 1)*/
  if( kpi_status & KPI_INST_RUN ) bios_kpi[CoreId].trace_time += get_time()-tstart;

}


/***************************************************************
 * kpi_instInit
 * -------------------------------------------------------------
 * Function used to do the aquisition of the instrumentation
 * setting. Then initialising the DBs and variables
 *
 * @return: none
 *
 ***************************************************************/
void kpi_instInit(void)
{
  /* don't change setup when already active */
  if( kpi_status & KPI_INST_RUN )
    return;


  /* read control from the memory */

  /* Note:
   * kpi_control |= KPI_END_SUMMARY;     bit0 : Activate the IVA and CPU load measure and print summary
   * kpi_control |= KPI_IVA_DETAILS;     bit1 : Activate the IVA trace
   * kpi_control |= KPI_OMX_DETAILS;     bit2 : Activate the OMX trace
   * kpi_control |= KPI_CPU_DETAILS;     bit3 : Activate the CPU Idle trace
   */

  /* reset kpi_status */
  kpi_status = 0;

  /* nothing to do if no instrumentation required */
  if( !kpi_control )
    return;

  /* force clktrctrl to no sleep mode (fast 32k access) */
  //set_WKUPAON( 1 );
#ifdef  USE_CTM_TIMER
  IpcPower_wakeLock();
#endif/*USE_CTM_TIMER*/

  /* IVA load setup */
  if( kpi_control & (KPI_END_SUMMARY | KPI_IVA_DETAILS) )
  {
    /* Initialize the IVA data base */
    kpi_IVA_profiler_init();
    kpi_status |= KPI_IVA_LOAD;
    if ( kpi_control & KPI_IVA_DETAILS )
      kpi_status |= KPI_IVA_TRACE;
  }

  /* CPU load setup */
  if( kpi_control & (KPI_END_SUMMARY | KPI_CPU_DETAILS) )
  {
    /* Initialize the CPU data base */
    kpi_CPU_profiler_init();
    kpi_status |= KPI_CPU_LOAD;
    if ( kpi_control & KPI_CPU_DETAILS )
      kpi_status |= KPI_CPU_TRACE;
  }

#if 0
  /* OMX trace setup */
  if( kpi_control & KPI_OMX_DETAILS )
  {
    kpi_status |= KPI_OMX_TRACE;
  }
#endif

  /* Mark as running */
  kpi_status |= KPI_INST_RUN;

}

/***************************************************************
 * kpi_instDeinit
 * -------------------------------------------------------------
 * Function used stop the instrumentation
 * Then initialising the summary if required
 *
 * @params: none
 *
 * @return: none
 *
 ***************************************************************/
void kpi_instDeinit( void )
{
  unsigned long cpu_summary=0;
  unsigned long iva_summary=0;

  /* noting to do when not running */
  if( !(kpi_status & KPI_INST_RUN) )
    return;

  if( kpi_control & KPI_END_SUMMARY )
  {
    cpu_summary = kpi_status & KPI_CPU_LOAD;
    iva_summary = kpi_status & KPI_IVA_LOAD;
  }

  /* now stop everything since measure is completed */
  kpi_status = 0;

  /* Print the summarys */
  PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "\n<KPI> Profiler Deinit %-8lu\n", get_32k() );
  if (iva_summary) kpi_IVA_profiler_print();
  if (cpu_summary) kpi_CPU_profiler_print();

  /* restore clktrctrl register */
  //set_WKUPAON( 0 );
#ifdef  USE_CTM_TIMER
  IpcPower_wakeUnlock();
#endif/*USE_CTM_TIMER*/

}


/***************************/
/* IVA fps and IVA-HD load */
/***************************/

#ifdef  CPU_LOAD_DETAILS
/***************************************************************
 * psi_cpu_load_details
 * -------------------------------------------------------------
 * Function to be called once a frame
 *
 * @params: none
 *
 * @return: none
 *
 ***************************************************************/
void psi_cpu_load_details(void)
{
  unsigned long time_curr = get_time();
  unsigned long CoreId, time_delta, idle_delta, idle;
  psi_bios_kpi *core_kpi;
  char trace[2][50];

  /* calculate delta_time and delta_idle */
  for( CoreId = 0; CoreId < 2; CoreId++)
  {
    core_kpi = &bios_kpi[CoreId];
    sprintf( trace[CoreId], "");
    idle_delta= *(core_kpi->ptr_idle_total) - core_kpi->idle_prev;
    time_delta = time_curr - core_kpi->idle_time_prev;
    /* calculate idle value only if it was updated (idle task switch happend at least once!) */
    if( idle_delta ) {
      core_kpi->idle_time_prev = time_curr;
      core_kpi->idle_prev = *(core_kpi->ptr_idle_total);
      if( time_delta ) {
        /* calculate Idle %time */
        idle = ((idle_delta * 100) + time_delta/2) / time_delta;
        sprintf( trace[CoreId], "Idle.%lu : %lu%%", CoreId, idle );
      }
    }
  }

  PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "%s   %s\n", trace[0], trace[1]);
}
#endif//CPU_LOAD_DETAILS


/***************************************************************
 * kpi_IVA_profiler_init
 * -------------------------------------------------------------
 * Function to be called at start of processing.
 *
 * @params: none
 *
 * @return: none
 *
 ***************************************************************/
void kpi_IVA_profiler_init(void)
{

  PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "<KPI> IVA Profiler Init %-8lu\n", get_32k() );

  iva_kpi.ivahd_t_tot       = 0;         /* IVA-HD tot processing time per frame */
  iva_kpi.ivahd_t_max       = 0;         /* IVA-HD max processing time per frame */
  iva_kpi.ivahd_t_min       =~0;         /* IVA-HD min processing time per frame */
  iva_kpi.ivahd_t_max_frame = 0;
  iva_kpi.ivahd_t_min_frame = 0;

  iva_kpi.omx_nb_frames     = 0;         /* Number of frames      */

  iva_kpi.before_time       = 0;
  iva_kpi.after_time        = 0;

  iva_kpi.t32k_start        = 0;
  iva_kpi.t32k_end          = 0;

}

/***************************************************************
 * kpi_before_codec
 * -------------------------------------------------------------
 * Function to be called before codec execution.
 *
 * @return: none
 *
 ***************************************************************/
void kpi_before_codec(void)
{
  unsigned long start, prev, delta;

#ifndef OMX_ENABLE_DUCATI_LOAD
  /* Started 1st time */
  if( !(kpi_status & KPI_INST_RUN) )
    kpi_instInit();
#endif//OMX_ENABLE_DUCATI_LOAD

  if( kpi_status & KPI_IVA_LOAD )
  {
    /* read the 32k timer */
    start = get_32k();

    prev = iva_kpi.before_time;
    iva_kpi.before_time = start;

    /* record very 1st time codec is used */
    if ( !iva_kpi.t32k_start ) iva_kpi.t32k_start = iva_kpi.before_time;

    /* calculate delta frame time in ticks */
    delta = start - prev;
    if(!iva_kpi.omx_nb_frames)  delta = 0;

#ifdef  IVA_DETAILS
    if( kpi_status & KPI_IVA_TRACE )
    {
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "BEG %-7s %-4u %-8lu %d\n",
       "IVA", iva_kpi.omx_nb_frames + 1, start, (delta * 100000) / 32768);
    }
#endif/*IVA_DETAILS*/

  }

#ifdef  CPU_LOAD_DETAILS
  if( kpi_status & KPI_CPU_TRACE )
  {
    kpi_status |= KPI_IVA_USED; /* mark IVA actually used */
    psi_cpu_load_details();
  }
#endif//CPU_LOAD_DETAILS

}

/***************************************************************
 * kpi_after_codec
 * -------------------------------------------------------------
 * Function to be called at the end of processing.
 *
 * @params: none
 *
 * @return: none
 *
 ***************************************************************/
void kpi_after_codec(void)
{
  unsigned long processing_time;

  if( kpi_status & KPI_IVA_LOAD )
  {
    /* Read 32k timer */
    iva_kpi.after_time = get_32k();
    iva_kpi.t32k_end   = iva_kpi.after_time;

    /* Process IVA-HD working time */
    processing_time = iva_kpi.after_time - iva_kpi.before_time;

#ifdef  IVA_DETAILS
    if( kpi_status & KPI_IVA_TRACE )
    {
      /* transform 32KHz ticks into ms */
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "END %-7s %-4u %-8lu %d\n",
          "IVA", iva_kpi.omx_nb_frames + 1, iva_kpi.after_time, (processing_time * 100000) / 32768);
    }
#endif/*IVA_DETAILS*/

    /* Total for Average */
    iva_kpi.ivahd_t_tot = (iva_kpi.ivahd_t_tot + processing_time);

    /* Max */
    if (processing_time > iva_kpi.ivahd_t_max)
    {
      iva_kpi.ivahd_t_max       = processing_time;
      iva_kpi.ivahd_t_max_frame = iva_kpi.omx_nb_frames + 1;
    }

    /* Min */
    if (processing_time < iva_kpi.ivahd_t_min)
    {
      iva_kpi.ivahd_t_min       = processing_time;
      iva_kpi.ivahd_t_min_frame = iva_kpi.omx_nb_frames + 1;
    }

    iva_kpi.omx_nb_frames++;

  }

}

/***************************************************************
 * kpi_IVA_profiler_print
 * -------------------------------------------------------------
 * Function to be called after codec execution.
 *
 * It is printing all results. syslink_trace_daemon must be enabled
 *
 * @params: none
 *
 * @return: none
 *
 ***************************************************************/
void kpi_IVA_profiler_print(void)
{
  unsigned long total_time, fps_x100, fps, frtick_x10, Ivatick_x10, Iva_pct;

  /* Calculate the total time */
  total_time = iva_kpi.t32k_end - iva_kpi.t32k_start;

  if (total_time) {

     /* Calculate the frame period and the framerate */
     if( iva_kpi.omx_nb_frames ) {
        frtick_x10 = total_time*10 / iva_kpi.omx_nb_frames;
        fps_x100 = 32768 * 1000 / frtick_x10;
        fps = fps_x100 / 100;
     }

     /* Calculate the IVA load */
     if( iva_kpi.omx_nb_frames ) {
        Ivatick_x10 = (iva_kpi.ivahd_t_tot*10 / iva_kpi.omx_nb_frames);
        Iva_pct = Ivatick_x10 * 100 / frtick_x10;
     }

     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "----------------------------------\n");
     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "Ducati stats:\n");
     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "-------------\n");
     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "      IVA  1st beg: %lu\n", iva_kpi.t32k_start);
     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "      IVA last end: %lu\n", iva_kpi.t32k_end);
     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "        Total time: %lu\n", total_time);
     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, " Number of samples: %lu\n", iva_kpi.omx_nb_frames);
     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, " Period of samples: %lu\n", frtick_x10/10);
     PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "               fps: %2d.%02d\n", fps, fps_x100-(fps*100) );

     /* stat existing only if frames were processed */
     if( iva_kpi.omx_nb_frames ) {
       PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "\n");
       PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "----------------------------------\n");
       PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "IVA-HD processing time:\n");
       PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "-----------------------\n");
       PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "       IVA average: %d\n", iva_kpi.ivahd_t_tot / iva_kpi.omx_nb_frames);
       PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "           IVA max: %d frame: %d\n", iva_kpi.ivahd_t_max, iva_kpi.ivahd_t_max_frame);
       PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "           IVA min: %d frame: %d\n", iva_kpi.ivahd_t_min, iva_kpi.ivahd_t_min_frame);
       PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "           IVA use: %d %%\n\n", Iva_pct);
    }
  }

}


#if 0
/***************************/
/* OMX FillThisBuffer      */
/*     EmptyThisBuffer     */
/* end FillBufferDone      */
/*     EmptyBufferDone     */
/***************************/

/***************************************************************
 * psi_omx_component
 * -------------------------------------------------------------
 * Structure maintenaing data for Ducati OMX traces
 *
 ***************************************************************/
typedef struct {
	OMX_HANDLETYPE hComponent;
	OMX_U32 count_ftb;
	OMX_U32 count_fbd;
	OMX_U32 count_etb;
	OMX_U32 count_ebd;
	char    name[50];
} psi_omx_component;

/* we trace up to MAX_OMX_COMP components */
#define MAX_OMX_COMP 8

/***************************************************************
 * kpi_omx_monitor
 * -------------------------------------------------------------
 * Contains up to 8 components data
 *
 ***************************************************************/
psi_omx_component kpi_omx_monitor[MAX_OMX_COMP]; /* we trace up to MAX_OMX_COMP components */
OMX_U32 kpi_omx_monitor_cnt=0;                   /* no component yet */


/***************************************************************
 * kpi_omx_comp_init
 * -------------------------------------------------------------
 * setup monitor data
 *
 * @params: OMX_HANDLETYPE hComponent
 *
 * @return: none
 *
 ***************************************************************/
void kpi_omx_comp_init( OMX_HANDLETYPE hComponent )
{

  OMX_VERSIONTYPE nVersionComp;
  OMX_VERSIONTYPE nVersionSpec;
  OMX_UUIDTYPE    compUUID;
  char compName[OMX_MAX_STRINGNAME_SIZE];
  char* p;
  OMX_U32 omx_cnt;

#ifdef  OMX_ENABLE_DUCATI_LOAD
  /* Started 1st time */
  if( !(kpi_status & KPI_INST_RUN) )
    kpi_instInit();
#endif//OMX_ENABLE_DUCATI_LOAD

  if( kpi_status & KPI_OMX_TRACE )
  {

    /* First init: clear kpi_omx_monitor components */
    if( kpi_omx_monitor_cnt == 0) {
      for (omx_cnt=0; omx_cnt < MAX_OMX_COMP; omx_cnt++) {
         /*clear handler registery */
         kpi_omx_monitor[omx_cnt].hComponent = 0;
      }
    }

    /* identify the 1st component free in the table (maybe one was removed previously) */
    for( omx_cnt=0; omx_cnt < MAX_OMX_COMP;  omx_cnt++ ) {
       if( kpi_omx_monitor[omx_cnt].hComponent == 0 ) break;
    }

    if( omx_cnt >= MAX_OMX_COMP ) return;

    /* current comp num and update */
    kpi_omx_monitor_cnt++;

    /* register the component handle */
    kpi_omx_monitor[omx_cnt].hComponent = hComponent;

    /* reset event counts */
    kpi_omx_monitor[omx_cnt].count_ftb = 0;
    kpi_omx_monitor[omx_cnt].count_fbd = 0;
    kpi_omx_monitor[omx_cnt].count_etb = 0;
    kpi_omx_monitor[omx_cnt].count_ebd = 0;

    /* register the component name */
    ((OMX_COMPONENTTYPE*) hComponent)->GetComponentVersion(hComponent, compName, &nVersionComp, &nVersionSpec, &compUUID);
    /* get the end of the string compName... */
    p = compName + strlen( compName ) - 1;
    if (strlen( compName ) >= 40) p = compName + 40 - 1;          // Fix for very long names
    while( (*p != '.' ) && (p != compName) ) p--;                 // Find last "."
    strncpy( kpi_omx_monitor[omx_cnt].name, p + 1, 6 );           // keep 6 last chars after "."
    *(kpi_omx_monitor[omx_cnt].name + 6) = '\0';                  // complete the chain of char
    sprintf(kpi_omx_monitor[omx_cnt].name, "%s%d", kpi_omx_monitor[omx_cnt].name, omx_cnt ); // Add index to the name
    /* trace component init */
    PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "<KPI> OMX %-8s Init %-8lu \n", kpi_omx_monitor[omx_cnt].name, get_32k() );

  }

}

/***************************************************************
 * kpi_omx_comp_deinit
 * -------------------------------------------------------------
 * deinit monitor data
 *
 * @params: OMX_HANDLETYPE hComponent
 *
 * @return: none
 *
 ***************************************************************/
void kpi_omx_comp_deinit( OMX_HANDLETYPE hComponent )
{

  OMX_U32 omx_cnt;

  if( kpi_omx_monitor_cnt > 0)
  {
    /* identify the component from the registery */
    for( omx_cnt=0; omx_cnt < MAX_OMX_COMP;  omx_cnt++ ) {
      if( kpi_omx_monitor[omx_cnt].hComponent == hComponent ) break;
    }

    /* trace component deinit */
    PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "<KPI> OMX %-7s Deinit %-8lu\n", kpi_omx_monitor[omx_cnt].name, get_32k() );

    /* unregister the component */
    kpi_omx_monitor[omx_cnt].hComponent = 0;

    kpi_omx_monitor_cnt--;
  }

  /* stop the instrumentation */
  if( kpi_omx_monitor_cnt == 0) kpi_instDeinit();

}

/***************************************************************
 * kpi_omx_comp_FTB
 * -------------------------------------------------------------
 * Trace FillThisBuffer() event
 *
 * @params: OMX_HANDLETYPE hComponent
 *          OMX_BUFFERHEADERTYPE* pBuffer
 *
 * @return: none
 *
 ***************************************************************/
void kpi_omx_comp_FTB( OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE* pBuffer )
{
#ifdef  OMX_DETAILS
  OMX_U32 omx_cnt;

  if( kpi_omx_monitor_cnt == 0) return;

  /* identify the component from the registery */
  for( omx_cnt=0; omx_cnt < MAX_OMX_COMP;  omx_cnt++ ) {
    if( kpi_omx_monitor[omx_cnt].hComponent == hComponent ) break;
  }

  /* Update counts and trace the event */
  if( omx_cnt < MAX_OMX_COMP ) {
    /* trace the event */
    PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "FTB %-7s %-4u %-8lu x%-8x\n",
#ifdef PHYSICAL_BUFFER
    kpi_omx_monitor[omx_cnt].name, ++kpi_omx_monitor[omx_cnt].count_ftb, get_32k(), pBuffer->pBuffer );
#else
    kpi_omx_monitor[omx_cnt].name, ++kpi_omx_monitor[omx_cnt].count_ftb, get_32k(), pBuffer );
#endif
  }
#endif//OMX_DETAILS

}

/***************************************************************
 * kpi_omx_comp_FBD
 * -------------------------------------------------------------
 * Trace FillBufferDone() event
 *
 * @params: OMX_HANDLETYPE hComponent
 *          OMX_BUFFERHEADERTYPE* pBuffer
 *
 * @return: none
 *
 ***************************************************************/
void kpi_omx_comp_FBD( OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE* pBuffer )
{
#ifdef  OMX_DETAILS
  OMX_U32 omx_cnt;
  unsigned long time;

#ifdef  CPU_LOAD_DETAILS
  if( (kpi_status & KPI_CPU_TRACE) && !(kpi_status & KPI_IVA_USED) )
  {
    psi_cpu_load_details();   /* trace the Idle here unless already traced at IVA BEG time */
  }
#endif//CPU_LOAD_DETAILS

  if( kpi_omx_monitor_cnt == 0) return;

  time = get_32k();

  /* identify the component from the registery */
  for( omx_cnt=0; omx_cnt < MAX_OMX_COMP;  omx_cnt++ ) {
    if( kpi_omx_monitor[omx_cnt].hComponent == hComponent ) break;
  }

  /* Update counts and trace the event */
  if( omx_cnt < MAX_OMX_COMP ) {
    /* trace the event */
    PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "FBD %-7s %-4u %-8lu x%-8x\n",
#ifdef PHYSICAL_BUFFER
      kpi_omx_monitor[omx_cnt].name, ++kpi_omx_monitor[omx_cnt].count_fbd, time, pBuffer->pBuffer );
#else
      kpi_omx_monitor[omx_cnt].name, ++kpi_omx_monitor[omx_cnt].count_fbd, time, pBuffer );
#endif
  }
#endif//OMX_DETAILS

}

/***************************************************************
 * kpi_omx_comp_ETB
 * -------------------------------------------------------------
 * Trace EmptyThisBuffer() event
 *
 * @params: OMX_HANDLETYPE  hComponent
 *          OMX_BUFFERHEADERTYPE* pBuffer
 *
 * @return: none
 *
 ***************************************************************/
void kpi_omx_comp_ETB( OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE* pBuffer )
{
#ifdef  OMX_DETAILS
  OMX_U32 omx_cnt;

  if( kpi_omx_monitor_cnt == 0) return;

  /* identify the component from the registery */
  for( omx_cnt=0; omx_cnt < MAX_OMX_COMP;  omx_cnt++ ) {
    if( kpi_omx_monitor[omx_cnt].hComponent == hComponent ) break;
  }

  /* Update counts and trace the event */
  if( omx_cnt < MAX_OMX_COMP ) {
    /* trace the event */
    PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "ETB %-7s %-4u %-8lu x%-8x\n",
#ifdef PHYSICAL_BUFFER
      kpi_omx_monitor[omx_cnt].name, ++kpi_omx_monitor[omx_cnt].count_etb, get_32k(), pBuffer->pBuffer );
#else
      kpi_omx_monitor[omx_cnt].name, ++kpi_omx_monitor[omx_cnt].count_etb, get_32k(), pBuffer );
#endif
  }
#endif//OMX_DETAILS

}

/***************************************************************
 * kpi_omx_comp_EBD
 * -------------------------------------------------------------
 * Trace EmptyBufferDone() event
 *
 * @params: OMX_HANDLETYPE hComponent
 *          OMX_BUFFERHEADERTYPE* pBuffer
 *
 * @return: none
 *
 ***************************************************************/
void kpi_omx_comp_EBD( OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE* pBuffer )
{
#ifdef  OMX_DETAILS
  OMX_U32 omx_cnt;

  if( kpi_omx_monitor_cnt == 0) return;

  /* identify the component from the registery */
  for( omx_cnt=0; omx_cnt < MAX_OMX_COMP;  omx_cnt++ ) {
    if( kpi_omx_monitor[omx_cnt].hComponent == hComponent ) break;
  }

  /* Update counts and trace the event */
  if( omx_cnt < MAX_OMX_COMP ) {
    /* trace the event */
    PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "EBD %-7s %-4u %-8lu x%-8x\n",
#ifdef PHYSICAL_BUFFER
      kpi_omx_monitor[omx_cnt].name, ++kpi_omx_monitor[omx_cnt].count_ebd, get_32k(), pBuffer->pBuffer );
#else
      kpi_omx_monitor[omx_cnt].name, ++kpi_omx_monitor[omx_cnt].count_ebd, get_32k(), pBuffer );
#endif
  }
#endif//OMX_DETAILS

}
#endif


/***************************************************************
 * kpi_load_pct_x_100
 * -------------------------------------------------------------
 * calculate percentage with 2 digit such as 78.34 %
 *
 * @params: unsigned long load, total
 *
 * @return: unsigned long result
 *
 ***************************************************************/
unsigned long kpi_load_pct_x_100(unsigned long load, unsigned long total)
{
  unsigned long mult = 100 * 100;

  while (total > (32768*8)) {
    total /= 2;
    mult /= 2;
  }

  /* load = 100 * 100 * load / total */
  load = ((load * mult) + total/2) / total;
  return (load);
}

/***************************************************************
 * kpi_CPU_profiler_init
 * -------------------------------------------------------------
 * Initialize profiler data
 *
 * @params: none
 *
 * @return: none
 *
 ***************************************************************/
void kpi_CPU_profiler_init(void)
{
  unsigned int CoreId, i;

  PSI_TracePrintf( TIMM_OSAL_TRACEGRP_SYSTEM, "<KPI> CPU Profiler Init %-8lu\n", get_32k() );

  for (CoreId = 0; CoreId < 2; CoreId++)
  {
    psi_bios_kpi *core_kpi = &bios_kpi[CoreId];

    core_kpi->total_time       = 0;
    core_kpi->kpi_time         = 0;
    core_kpi->kpi_count        = 0;
    core_kpi->nb_switch        = 0;

    core_kpi->nb_tasks         = 0;
    core_kpi->nb_hwi           = 0;
    core_kpi->nb_swi           = 0;

    core_kpi->tasks[0].handle  = 0;                             /* for startup set task[0].handle to 0 */

    /* reserve last task slot in DB to others task that wouldn't fit in the DB */
    core_kpi->tasks[KPI_MAX_NB_TASKS-1].handle = (void*) 0x11000011;
    core_kpi->tasks[KPI_MAX_NB_TASKS-1].total_time = 0;
    core_kpi->tasks[KPI_MAX_NB_TASKS-1].nb_switch  = 0;
    strcpy(core_kpi->tasks[KPI_MAX_NB_TASKS-1].name, "Other tasks");
    /* reserve last swi slot in DB to others task that wouldn't fit in the DB */
    core_kpi->swi[KPI_MAX_NB_SWI-1].handle = (void*) 0x22000022;
    core_kpi->swi[KPI_MAX_NB_SWI-1].total_time = 0;
    core_kpi->swi[KPI_MAX_NB_SWI-1].nb_switch  = 0;
    strcpy(core_kpi->swi[KPI_MAX_NB_SWI-1].name, "Other swis");
    /* reserve last hwi slot in DB to others task that wouldn't fit in the DB */
    core_kpi->hwi[KPI_MAX_NB_HWI-1].handle = (void*) 0x33000033;
    core_kpi->hwi[KPI_MAX_NB_HWI-1].total_time = 0;
    core_kpi->hwi[KPI_MAX_NB_HWI-1].nb_switch  = 0;
    strcpy(core_kpi->hwi[KPI_MAX_NB_HWI-1].name, "Other hwis");

    /* clear the context pointers table */
    for (i=0; i < CTX_DIRECT_NUM; i++) {
     core_kpi->context_cache[i]= &(core_kpi->tasks[0]);	        /* point to the Data Base */
    }

    core_kpi->context_stack[0] = &(core_kpi->tasks[0]);         /* point to 1st context element */
    core_kpi->pt_context_stack = &(core_kpi->context_stack[0]); /* stack beginning */

    core_kpi->ptr_idle_total   = &(core_kpi->idle_prev);        /* will point later on to idle_total_ticks */
    core_kpi->idle_prev        = 0;
    core_kpi->idle_time_prev   = 0;

    core_kpi->trace_time       = 0;

  }

  /* set current time into prev_t32 in each data based */
  bios_kpi[0].prev_t32 = get_time();
  bios_kpi[1].prev_t32 = bios_kpi[0].prev_t32;

}

/***************************************************************
 * kpi_CPU_profiler_print
 * -------------------------------------------------------------
 * Print profiler data
 *
 * @params: none
 *
 * @return: none
 *    if (prev != NULL)

 ***************************************************************/
void kpi_CPU_profiler_print(void)
{
  unsigned long load, ld00, Idle, CoreId, i;
  psi_bios_kpi *core_kpi;

#ifdef  COST_AFTER
  unsigned long instx1024;
  /* calculate instrumentation cost a posteriori */
  {
    Task_Handle test;
    unsigned long key = Task_disable();
    core_kpi = &bios_kpi[ Core_getId() ];
    test = core_kpi->tasks[0].handle;
    instx1024 = get_time();
    for(i = 0; i < 1024; i++) psi_kpi_task_test( test, test );
    instx1024 = get_time()-instx1024;
    Task_restore(key);
  }
#endif//COST_AFTER

  /* Print the results for each core */
  for (CoreId = 0; CoreId < 2; CoreId++ ) {
    core_kpi = &bios_kpi[CoreId];

    /* Reconstruct global counts */
    for (i=0; i < core_kpi->nb_tasks; i++){
      core_kpi->nb_switch += core_kpi->tasks[i].nb_switch;     /* nb_switch  (nb interrupts + task switchs) */
      core_kpi->kpi_count += core_kpi->tasks[i].nb_switch;     /* kpi_count (nunber of times the intrumentation run) */
      core_kpi->total_time+= core_kpi->tasks[i].total_time;    /* total_time (all times measured) */
    }
    for (i=0; i < core_kpi->nb_swi; i++) {
      core_kpi->nb_switch += core_kpi->swi[i].nb_switch;
      core_kpi->kpi_count += core_kpi->swi[i].nb_switch * 2;   /* 2 runs per interrupts */
      core_kpi->total_time+= core_kpi->swi[i].total_time;
    }
    for (i=0; i < core_kpi->nb_hwi; i++) {
      core_kpi->nb_switch += core_kpi->hwi[i].nb_switch;
      core_kpi->kpi_count += core_kpi->hwi[i].nb_switch * 2;   /* 2 runs per interrupts */
      core_kpi->total_time+= core_kpi->hwi[i].total_time;
    }
    /* add cost of measured if stored as a separate task */
    core_kpi->total_time += core_kpi->kpi_time;

    if ( core_kpi->total_time ) {
      /* Print global stats */
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "----------------------------------\n");
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "Core %d :\n", CoreId);
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "--------\n");
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "  type  #: handle   ticks    counts  ( %%cpu )   instance-name\n");
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "----------------------------------\n");
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "  Total test        %-8lu %-7ld\n", core_kpi->total_time, core_kpi->nb_switch);
      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "----------------------------------\n");

      if( core_kpi->kpi_time )
      {
        ld00 = kpi_load_pct_x_100(core_kpi->kpi_time, core_kpi->total_time);/* ex. 7833 -> 78.33 % */
        load = ld00/100;                                                  /* 78.33 % -> 78 */
        ld00 = ld00 - load*100;                                           /* 78.33 % -> 33 */
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "  Measure: Overhead %-8lu ----    (%2d.%02d %%)  CPU load-inst-cost\n",
          core_kpi->kpi_time, load, ld00);
      }

      for (i=0; i < core_kpi->nb_tasks; i++)
      {
        ld00 = kpi_load_pct_x_100(core_kpi->tasks[i].total_time, core_kpi->total_time);
        load = ld00/100;                                                /* 78.33 % -> 78 */
        ld00 = ld00 - load*100;                                         /* 78.33 % -> 33 */
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "  task %2d: %-8lx %-8lu %-7ld (%2d.%02d %%)  %s\n",
          i, core_kpi->tasks[i].handle, core_kpi->tasks[i].total_time, core_kpi->tasks[i].nb_switch, load, ld00, core_kpi->tasks[i].name);
      }

      for (i=0; i < core_kpi->nb_swi; i++)
      {
        ld00 = kpi_load_pct_x_100(core_kpi->swi[i].total_time, core_kpi->total_time);
        load = ld00/100;                                                /* 78.33 % -> 78 */
        ld00 = ld00 - load*100;                                         /* 78.33 % -> 33 */
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "   swi %2d: %-8lx %-8lu %-7ld (%2d.%02d %%)  %s\n",
          i, core_kpi->swi[i].handle, core_kpi->swi[i].total_time, core_kpi->swi[i].nb_switch, load, ld00, core_kpi->swi[i].name);
      }

      for (i=0; i < core_kpi->nb_hwi; i++)
      {
        ld00 = kpi_load_pct_x_100(core_kpi->hwi[i].total_time, core_kpi->total_time);
        load = ld00/100;                                                /* 78.33 % -> 78 */
        ld00 = ld00 - load*100;                                         /* 78.33 % -> 33 */
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "   hwi %2d: %-8lx %-8lu %-7ld (%2d.%02d %%)  %s\n",
          i, core_kpi->hwi[i].handle, core_kpi->hwi[i].total_time, core_kpi->hwi[i].nb_switch, load, ld00, core_kpi->hwi[i].name);
      }

      PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "----------------------------------\n");

#ifdef  COST_AFTER
      if( !core_kpi->kpi_time )
      {
        /* calculate the cost in the instrumentation */
        core_kpi->kpi_time = (core_kpi->kpi_count * instx1024) / 1024;
        ld00 = kpi_load_pct_x_100(core_kpi->kpi_time, core_kpi->total_time);/* ex. 7833 -> 78.33 % */
        load = ld00/100;                                                  /* 78.33 % -> 78 */
        ld00 = ld00 - load*100;                                           /* 78.33 % -> 33 */
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "  CPU load measure overhead  %2d.%02d %%\n", load, ld00);
      }
#endif//COST_AFTER

      if( core_kpi->trace_time )
      {
        ld00 = kpi_load_pct_x_100(core_kpi->trace_time, core_kpi->total_time);
        load = ld00/100;                                                /* 78.33 % -> 78 */
        ld00 = ld00 - load*100;                                         /* 78.33 % -> 33 */
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "  Real time traces overhead  %2d.%02d %%\n", load, ld00);
      }

      /* restore to Idle Inst cost + print cost if idle exists */
      if( *(core_kpi->ptr_idle_total) )
      {
        Idle = *(core_kpi->ptr_idle_total) + core_kpi->kpi_time + core_kpi->trace_time;
        ld00 = kpi_load_pct_x_100(Idle, core_kpi->total_time);
        load = ld00/100;                                                /* 78.33 % -> 78 */
        ld00 = ld00 - load*100;                                         /* 78.33 % -> 33 */
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "----------------------------------\n");
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "  Idle task compensated  =>  %2d.%02d %% \n", load, ld00);
        PSI_TracePrintf(TIMM_OSAL_TRACEGRP_SYSTEM, "----------------------------------\n\n");
      }

    }
  }

}


/***************************************************************
 * psi_kpi_search_create_context_task
 * -------------------------------------------------------------
 * Search for a context entry to context DB.
 * Create a new on if not existing
 *
 * @params: void *task, unsigned int CoreId
 *
 * @return: psi_context_info *db_ptr
 *
 ***************************************************************/
psi_context_info *psi_kpi_search_create_context_task(void *task, psi_bios_kpi *core_kpi)
{
  int i, nb_items, strLenght;
  String taskName;
  psi_context_info *db_ptr;

  /* KPI_CONTEXT_TASK */
  nb_items = core_kpi->nb_tasks;
  db_ptr   = core_kpi->tasks;

  /* Search handle existing in task table */
  for (i=0; i < nb_items; i++) {
    if (task == db_ptr[i].handle) break;
  }

  /* handle found in our DB */
  if( i < nb_items ) {
    return (&db_ptr[i]);
  }

  /* check for space left in the DB. When last-1 reached use last for others tasks together */
  if (nb_items >= KPI_MAX_NB_TASKS-1) {
    core_kpi->nb_tasks = KPI_MAX_NB_TASKS;
    return (&db_ptr[KPI_MAX_NB_TASKS-1]);
  }

  /* add the new task */
  core_kpi->nb_tasks = nb_items+1;

  /* get the task name Make sure task name fits into the item.name */
  taskName = Task_Handle_name((Task_Handle)task);
  strLenght = strlen( taskName );
  if (strLenght > 40 ) strLenght = 40;

  /* For Idle direct access */
  if( strstr(taskName, "Idle") )
    core_kpi->ptr_idle_total = &(db_ptr[i].total_time);

  /* create the new entry */
  db_ptr[i].handle     = task;
  db_ptr[i].total_time = 0;
  db_ptr[i].nb_switch  = 0;
  strncpy(db_ptr[i].name, taskName, strLenght);
  *(db_ptr[i].name + strLenght) = '\0';                  // complete the chain of char

  return &(db_ptr[i]);
}

/***************************************************************
 * psi_kpi_search_create_context_swi
 * -------------------------------------------------------------
 * Search for a context entry to context DB.
 * Create a new on if not existing
 *
 * @params: void *task, unsigned int CoreId
 *
 * @return: psi_context_info *db_ptr
 *
 ***************************************************************/
psi_context_info *psi_kpi_search_create_context_swi(void *task, psi_bios_kpi *core_kpi)
{
  int i, nb_items, strLenght;
  String taskName;
  psi_context_info *db_ptr;

  /* KPI_CONTEXT_SWI */
  nb_items = core_kpi->nb_swi;
  db_ptr   = core_kpi->swi;

  /* Search handle existing in task table */
  for (i=0; i < nb_items; i++) {
    if (task == db_ptr[i].handle) break;
  }

  /* handle found in our DB */
  if( i < nb_items ) {
    return (&db_ptr[i]);
  }

  /* check for space left in the DB. When last-1 reached use last for others swi together */
  if (nb_items >= KPI_MAX_NB_SWI-1) {
    core_kpi->nb_swi = KPI_MAX_NB_SWI;
    return (&db_ptr[KPI_MAX_NB_SWI-1]);
  }

  /* add the new swi */
  core_kpi->nb_swi = nb_items+1;

  /* get the task name Make sure task name fits into the item.name */
  taskName = Swi_Handle_name((Swi_Handle)task);
  strLenght = strlen( taskName );
  if (strLenght > 40 ) strLenght = 40;

  /* create the new entry */
  db_ptr[i].handle     = task;
  db_ptr[i].total_time = 0;
  db_ptr[i].nb_switch  = 0;
  strncpy(db_ptr[i].name, taskName, strLenght);
  *(db_ptr[i].name + strLenght) = '\0';                  // complete the chain of char

  return &(db_ptr[i]);
}

/***************************************************************
 * psi_kpi_search_create_context_hwi
 * -------------------------------------------------------------
 * Search for a context entry to context DB.
 * Create a new on if not existing
 *
 * @params: void *task, unsigned int CoreId
 *
 * @return: psi_context_info *db_ptr
 *
 ***************************************************************/
psi_context_info *psi_kpi_search_create_context_hwi(void *task, psi_bios_kpi *core_kpi)
{
  int i, nb_items, strLenght;
  String taskName;
  psi_context_info *db_ptr;

  /* KPI_CONTEXT_HWI */
  nb_items = core_kpi->nb_hwi;
  db_ptr   = core_kpi->hwi;

  /* Search handle existing in task table */
  for (i=0; i < nb_items; i++) {
    if (task == db_ptr[i].handle) break;
  }

  /* handle found in our DB */
  if( i < nb_items ) {
    return (&db_ptr[i]);
  }

  /* check for space left in the DB. When last-1 reached use last for others hwi together */
  if (nb_items >= KPI_MAX_NB_HWI-1) {
    core_kpi->nb_hwi = KPI_MAX_NB_HWI;
    return (&db_ptr[KPI_MAX_NB_HWI-1]);
  }

  /* add the new hwi */
  core_kpi->nb_hwi = nb_items+1;

  /* get the task name Make sure task name fits into the item.name */
  taskName = Hwi_Handle_name((Hwi_Handle)task);
  strLenght = strlen( taskName );
  if (strLenght > 40 ) strLenght = 40;

  /* create the new entry */
  db_ptr[i].handle     = task;
  db_ptr[i].total_time = 0;
  db_ptr[i].nb_switch  = 0;
  strncpy(db_ptr[i].name, taskName, strLenght);
  *(db_ptr[i].name + strLenght) = '\0';                  // complete the chain of char

  return &(db_ptr[i]);
}

/***************************************************************
 * psi_kpi_task_switch
 * -------------------------------------------------------------
 * Task switch hook:
 *  - identify new tasks
 *  - accumulate task load
 *  - process total execution time
 *
 * @params: Task_Handle prev, Task_Handle next
 *
 * @return: none
 *
 ***************************************************************/
void psi_kpi_task_switch(Task_Handle prev, Task_Handle next)
{
  if ( kpi_status & KPI_CPU_LOAD )
  {
    unsigned long key    = Hwi_disable();
    unsigned long CoreId = Core_getId();
    unsigned long tick   = get_time_core(CoreId);
    psi_bios_kpi *core_kpi = &bios_kpi[CoreId];
    psi_context_info *context_next, *context;
    unsigned long cachePtr;

    /* Ending context */
    context = (psi_context_info *) *core_kpi->pt_context_stack;

    /* Starting context */
    cachePtr= ((((long) next)>>4) ^ (((long) next)>>12) ) & 0xff; /* build a simple 8 bits address for trying cache DB search */
    context_next = core_kpi->context_cache[ cachePtr ];
    if (context_next->handle != next) {                           /* if NOT pointing to our handle make exhaustive seartch */
      context_next = psi_kpi_search_create_context_task(next, core_kpi);
      core_kpi->context_cache[ cachePtr ] = context_next;         /* update direct table (this may evict one already here) */
    }

    /* Maintain stack for interrupts: save context starting */
    *core_kpi->pt_context_stack = (void*) context_next;

    /* update tasks stats */
    context->total_time  += tick - core_kpi->prev_t32;            /* count the time spend in the ending task */
    context->nb_switch   ++;                                      /* count the switch */

#ifdef  INST_COST
    /* will start processing the task now */
    {
      unsigned long tick2 = get_time_core(CoreId);
      core_kpi->kpi_time += tick2 - tick;                         /* update kpi_time (instrumentation cost) */
      core_kpi->prev_t32  = tick2;                                /* store tick: time when task actually starts */
    }
#else //INST_COST
    core_kpi->prev_t32    = tick;                                 /* store tick: time when task actually starts */
#endif//INST_COST

    Hwi_restore( key );
  }
}

/***************************************************************
 * psi_kpi_swi_begin
 * -------------------------------------------------------------
 * SWI begin hook: memorizes SWI start time
 *
 * @params: Swi_Handle swi
 *
 * @return: none
 *
 ***************************************************************/
void psi_kpi_swi_begin(Swi_Handle swi)
{
  if ( kpi_status & KPI_CPU_LOAD )
  {
    unsigned long key    = Hwi_disable();
    unsigned long CoreId = Core_getId();
    unsigned long tick   = get_time_core(CoreId);
    psi_bios_kpi *core_kpi = &bios_kpi[CoreId];
    psi_context_info *context_next, *context;
    unsigned long cachePtr;

    /* Ending context */
    context = (psi_context_info *) *core_kpi->pt_context_stack++; /* Going from a TASK or SWI to new SWI */

    /* Starting context */
    cachePtr= ((((long) swi)>>4) ^ (((long) swi)>>12) ) & 0xff;   /* build a simple 8 bits address for trying cache DB search */
    context_next = core_kpi->context_cache[ cachePtr ];
    if (context_next->handle != swi) {                            /* if NOT pointing to our handle make exhaustive seartch */
      context_next = psi_kpi_search_create_context_swi(swi, core_kpi);
      core_kpi->context_cache[ cachePtr ] = context_next;         /* update direct table (this may evict one already here) */
    }

    /* Maintain stack for interrupts: save context starting */
    *core_kpi->pt_context_stack = (void*) context_next;

    /* update tasks stats */
    context->total_time  += tick - core_kpi->prev_t32;            /* count the time spend in the ending task */

#ifdef  INST_COST
    /* will start processing the task now */
    {
      unsigned long tick2 = get_time_core(CoreId);
      core_kpi->kpi_time += tick2 - tick;                         /* update kpi_time (instrumentation cost) */
      core_kpi->prev_t32  = tick2;                                /* store tick: time when task actually starts */
    }
#else //INST_COST
    core_kpi->prev_t32    = tick;                                 /* store tick: time when task actually starts */
#endif//INST_COST

    Hwi_restore( key );
  }
}

/***************************************************************
 * psi_kpi_swi_end
 * -------------------------------------------------------------
 * SWI end hook: accumulates SWI execution time
 *
 * @params: Swi_Handle swi
 *
 * @return: none
 *
 ***************************************************************/
void psi_kpi_swi_end(Swi_Handle swi)
{
  if ( kpi_status & KPI_CPU_LOAD )
  {
    unsigned long key    = Hwi_disable();
    unsigned long CoreId = Core_getId();
    unsigned long tick   = get_time_core(CoreId);
    psi_bios_kpi *core_kpi = &bios_kpi[CoreId];
    psi_context_info *context;

    /* Ending context */
    context = (psi_context_info *) *core_kpi->pt_context_stack--; /* Going back to interrupted TASK or SWI */

    /* update tasks stats */
    context->total_time  += tick - core_kpi->prev_t32;            /* count the time spend in the ending task */
    context->nb_switch   ++;                                      /* count the switch */

#ifdef  INST_COST
    /* will start processing the task now */
    {
      unsigned long tick2 = get_time_core(CoreId);
      core_kpi->kpi_time += tick2 - tick;                         /* update kpi_time (instrumentation cost) */
      core_kpi->prev_t32  = tick2;                                /* store tick: time when task actually starts */
    }
#else //INST_COST
    core_kpi->prev_t32    = tick;                                 /* store tick: time when task actually starts */
#endif//INST_COST

    Hwi_restore( key );
  }
}

/***************************************************************
 * psi_kpi_hwi_begin
 * -------------------------------------------------------------
 * HWI begin hook: memorizes HWI start time
 *
 * @params: Hwi_Handle hwi
 *
 * @return: none
 *
 ***************************************************************/
void psi_kpi_hwi_begin(Hwi_Handle hwi)
{
  if ( kpi_status & KPI_CPU_LOAD )
  {
    unsigned long key    = Hwi_disable();
    unsigned long CoreId = Core_getId();
    unsigned long tick   = get_time_core(CoreId);
    psi_bios_kpi *core_kpi = &bios_kpi[CoreId];
    psi_context_info *context_next, *context;
    unsigned long cachePtr;

    /* Ending context */
    context = (psi_context_info *) *core_kpi->pt_context_stack++; /* Going from previous TASK or SWI to a HWI */

    /* Starting context */
    cachePtr= ((((long) hwi)>>4) ^ (((long) hwi)>>12) ) & 0xff;   /* build a simple 8 bits address for trying cache DB search */
    context_next = core_kpi->context_cache[ cachePtr ];
    if (context_next->handle != hwi) {                            /* if NOT pointing to our handle make exhaustive seartch */
      context_next = psi_kpi_search_create_context_hwi(hwi, core_kpi);
      core_kpi->context_cache[ cachePtr ] = context_next;         /* update direct table (this may evict one already here) */
    }

    /* Maintain stack for interrupts: save context starting */
    *core_kpi->pt_context_stack = (void*) context_next;

    /* update tasks stats */
    context->total_time  += tick - core_kpi->prev_t32;            /* count the time spend in the ending task */

#ifdef  INST_COST
    /* will start processing the task now */
    {
      unsigned long tick2 = get_time_core(CoreId);
      core_kpi->kpi_time += tick2 - tick;                         /* update kpi_time (instrumentation cost) */
      core_kpi->prev_t32  = tick2;                                /* store tick: time when task actually starts */
    }
#else //INST_COST
    core_kpi->prev_t32    = tick;                                 /* store tick: time when task actually starts */
#endif//INST_COST

    Hwi_restore( key );
  }
}

/***************************************************************
 * psi_kpi_hwi_end
 * -------------------------------------------------------------
 * HWI end hook: accumulates HWI execution time
 *
 * @params: Hwi_Handle hwi
 *
 * @return: none
 *
 ***************************************************************/
void psi_kpi_hwi_end(Hwi_Handle hwi)
{
  if ( kpi_status & KPI_CPU_LOAD )
  {
    unsigned long key    = Hwi_disable();
    unsigned long CoreId = Core_getId();
    unsigned long tick   = get_time_core(CoreId);
    psi_bios_kpi *core_kpi = &bios_kpi[CoreId];
    psi_context_info *context;

    /* Ending context */
    context = (psi_context_info *) *core_kpi->pt_context_stack--; /* Going back to interrupted TASK or SWI or HWI */

    /* update tasks stats */
    context->total_time  += tick - core_kpi->prev_t32;            /* count the time spend in the ending task */
    context->nb_switch   ++;                                      /* count the switch */

#ifdef  INST_COST
    /* will start processing the task now */
    {
      unsigned long tick2 = get_time_core(CoreId);
      core_kpi->kpi_time += tick2 - tick;                         /* update kpi_time (instrumentation cost) */
      core_kpi->prev_t32  = tick2;                                /* store tick: time when task actually starts */
    }
#else //INST_COST
    core_kpi->prev_t32    = tick;                                 /* store tick: time when task actually starts */
#endif//INST_COST

    Hwi_restore( key );
  }
}

/***************************************************************
 * psi_kpi_task_test
 * -------------------------------------------------------------
 * Task switch hook:
 *  - identify new tasks
 *  - used for measuring execution time of the instrumentation
 *
 * @params: Task_Handle prev, Task_Handle next
 *
 * @return: none
 *
 ***************************************************************/
void psi_kpi_task_test(Task_Handle prev, Task_Handle next)
{
    unsigned long key    = Hwi_disable();
    unsigned long CoreId = Core_getId();
    unsigned long tick   = get_time_core(CoreId);
    psi_bios_kpi *core_kpi = &bios_kpi[CoreId];
    psi_context_info *context_next;
    unsigned long cachePtr;

    /* Starting context */
    cachePtr= ((((long) next)>>4) ^ (((long) next)>>12) ) & 0xff; /* build a simple 8 bits address for trying cache DB search */
    context_next = core_kpi->context_cache[ cachePtr ];
    if (context_next->handle != next) {                           /* if NOT pointing to our handle make exhaustive seartch */
      context_next = psi_kpi_search_create_context_task(next, core_kpi);
      core_kpi->context_cache[ cachePtr ] = context_next;         /* update direct table (this may evict one already here) */
    }

    /* Maintain stack for interrupts: save context starting */
    *core_kpi->pt_context_stack = (void*) context_next;

    core_kpi->prev_t32    = tick;                                 /* store tick: time when task actually starts */

    Hwi_restore( key );
}



