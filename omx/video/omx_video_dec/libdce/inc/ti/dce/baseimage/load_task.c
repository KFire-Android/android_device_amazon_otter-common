/*
 * Monitor load and trace any change.
 * Author: Vincent Stehlé <v-stehle@ti.com>, copied from ping_tasks.c
 *
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

#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/smp/Load.h>
#include <ti/sysbios/gates/GateAll.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "profile.h"

/*
 * Time to sleep between load reporting attempts, in ticks.
 * On TI platforms, 1 tick == 1 ms.
 */
#define SLEEP_TICKS 1000

/*
 * Load "precision". We compute our loads in percent, multiplied by this
 * factor.
 */
#define PRECISION 100

/*
 * Load reporting "threshold". When the new load is within previous reported
 * load +- this value, we do not report it.
 */
#define THRESHOLD (1 * PRECISION)

/* State variables for one load. */
struct load {
    unsigned long prev_time,    /* Previous time the load was sampled (32k or CTM time). */
        prev_val;               /* Previously sampled load value (32k or CTM ticks). */
    unsigned prev_load;         /* Previously reported load (in percent * PRECISION). */
};

/* Initialize load structure. */
static void init_load(struct load *state)
{
    memset(state, 0, sizeof(struct load));
}

/* Compute a load, in percent * PRECISION. We nicely handle the case where the
 * load is negative or superior to 100%, in which case we return 0. This
 * happens when the profiler is reset between two measurements (e.g. player
 * exit & relaunched). Also, we handle div by zero, just in case. We update the
 * previous values, too. No statics allowed here. */
static unsigned compute_load(struct load *state, unsigned long time, unsigned long val)
{
    unsigned load;
    unsigned long delta_time = time - state->prev_time;
    long delta = (long)val - (long)state->prev_val;

    if (delta < 0 || delta > delta_time || !delta_time)
        load = 0;

    else
        load = 100 * PRECISION * delta / delta_time;

    state->prev_val = val;
    state->prev_time = time;
    return load;
}

/* Compute IVA load from KPI profiler, in percent * PRECISION. TODO: Better
 * integration into the KPI profiler. No need to protect the load "reading" as
 * we atomically read a single value. IVA time is measured with the 32k timer. */
static unsigned compute_iva_load(struct load *iva_state)
{
    extern unsigned long get_ivahd_t_tot(void);
    extern unsigned long get_32k(void);
    unsigned long ivahd_t_tot = get_ivahd_t_tot(), time = get_32k();
    return compute_load(iva_state, time, ivahd_t_tot);
}

/* Compute core load from KPI profiler, in percent * PRECISION. TODO: Better
 * integration into the KPI profiler. We protect the load "reading" for sanity
 * of the value. Core load is measured with the CTM timer (one per core). No
 * static allowed in this one, as we are called once per core. */
static unsigned compute_core_load(struct load *core_state, unsigned core, GateAll_Handle gate)
{
    extern unsigned long get_time_core(int core);
    extern unsigned long get_core_total(unsigned core);
    unsigned long time = get_time_core(core), core_total;
    IArg key;

    /* CRITICAL SECTION */
    key = GateAll_enter(gate);
    core_total = get_core_total(core);
    GateAll_leave(gate, key);

    return compute_load(core_state, time, core_total);
}

/* Trace a load if delta is above threshold. We update the previous value if
 * reported. We await the load in percent * PRECISION. */
static void trace_load_if_above(struct load *state, unsigned load, const char *name)
{
    unsigned delta;

    /* Trace if changed and delta above threshold. */
    delta = abs((int)load - (int)state->prev_load);

    if (delta > THRESHOLD) {
        unsigned integ = load / PRECISION, frac = load - integ * PRECISION;
        System_printf("loadTask: %s load = %u.%02u%%\n", name, integ, frac);
        state->prev_load = load;
    }
}

/* Monitor load and trace any change. */
static Void loadTaskFxn(UArg arg0, UArg arg1)
{
    GateAll_Handle gate;
    struct load bios_state, iva_state, core0_state, core1_state;

    /* Suppress warnings. */
    (void)arg0;
    (void)arg1;

    /* Init load state structures. */
    init_load(&bios_state);
    init_load(&iva_state);
    init_load(&core0_state);
    init_load(&core1_state);

    /* Prepare our Gate. */
    gate = GateAll_create(NULL, NULL);

    System_printf(
        "loadTask: started\n"
        "  SLEEP_TICKS: %u\n"
        "  Load_hwiEnabled: %d\n"
        "  Load_swiEnabled: %d\n"
        "  Load_taskEnabled: %d\n"
        "  Load_updateInIdle: %d\n"
        "  Load_windowInMs: %u\n"
        ,
        SLEEP_TICKS,
        Load_hwiEnabled,
        Load_swiEnabled,
        Load_taskEnabled,
        Load_updateInIdle,
        Load_windowInMs
    );

    /* Infinite loop to trace loads. */
    for (;;) {
        unsigned bios_load, iva_load, core0_load, core1_load;

        /* Get BIOS load and trace if delta above threshold.  Note: we "waste"
         * the state structure a bit as we do not use the time fields, and we
         * "waste" the PRECISION, too, but that gives us regularity in the
         * code. */
        bios_load = (unsigned)Load_getCPULoad() * PRECISION;
        trace_load_if_above(&bios_state, bios_load, "BIOS");

        /* Get IVA load, cores loads. "Gating" is done inside each core load
         * computation, and preemption can happen between them. We trace the
         * loads if delta is above threshold. */
        iva_load = compute_iva_load(&iva_state);
        trace_load_if_above(&iva_state, iva_load, "IVA");

        core0_load = compute_core_load(&core0_state, 0, gate);
        trace_load_if_above(&core0_state, core0_load, "core0");

        core1_load = compute_core_load(&core1_state, 1, gate);
        trace_load_if_above(&core1_state, core1_load, "core1");

        /* Delay. */
        Task_sleep(SLEEP_TICKS);
    }
}

void start_load_task(void)
{
    Task_Params params;

    /* Monitor load and trace any change. */
    Task_Params_init(&params);
    params.instance->name = "loadtsk";
    params.priority = 1;

    if(!Task_create(loadTaskFxn, &params, NULL))
        System_printf("Could not create load task!\n");
}
