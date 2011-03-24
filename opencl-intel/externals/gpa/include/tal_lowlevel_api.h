/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) Intel Corporation (2010).  All rights reserved.
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS LICENSED
** ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
** INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT PROVIDE ANY UPDATES,
** ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY WARRANTY OF
** MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY PARTICULAR PURPOSE, OR ANY
** OTHER WARRANTY.  Intel disclaims all liability, including liability for
** infringement of any proprietary rights, relating to use of the code. No license,
** express or implied, by estoppel or otherwise, to any intellectual property
** rights is granted herein.
**
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef TAL_LOWLEVEL_H
#define TAL_LOWLEVEL_H
#ifndef TAL_DOXYGEN
#ifdef TAL_DISABLE

#define TAL_ProcessCreate(adapter_id, pid, name, clockFreq, fn, fnData) (0)
#define TAL_ProcessAddModule(process, module)
#define TAL_AllocTraceBuffer(process, tid) (0)
#define TAL_FreeTraceBuffer(trace)

#else // TAL_DISABLE

#include "tal_types.h"
#include "tal_ctl.h"

#ifdef __cplusplus
extern "C" {
#endif //  __cplusplus
/** \ingroup Misc
 **/
TAL_INLINE TAL_PROCESS*	TAL_ProcessCreate(TAL_UINT32 adapter_id, TAL_UINT64 pid, const char* name, TAL_UINT64 clockFreq, TAL_GetProcessClockbaseFn fn, void* fnData)
{
    TAL_PROCESS* process = TAL_NULL;
    TAL_UINT32 resultSize = sizeof(process);
    TALCTL_PROCESS_CREATE_PARAMS params;
    params.adapter_id = adapter_id;
    params.pid = pid;
    params.name = name;
    params.clockFreq = clockFreq;
    params.getClockBaseFunc = fn;
    params.getClockBaseFuncData = fnData;
    p__talctl(TALCTL_PROCESS_CREATE, &params, sizeof(params), &process, &resultSize);
    return process;
}


TAL_INLINE TAL_TRACE*      TAL_AllocTraceBuffer(TAL_PROCESS* process, TAL_UINT64 tid)
{
    TAL_TRACE* trace = TAL_NULL;
    TAL_UINT32 resultSize = sizeof(trace);
    TALCTL_ALLOC_TRACE_BUFFER_PARAMS params;
    params.process = process;
    params.tid = tid;
    p__talctl(TALCTL_ALLOC_TRACE_BUFFER, &params, sizeof(params), &trace, &resultSize);
    return trace;
}

TAL_INLINE void            TAL_FreeTraceBuffer(TAL_TRACE* trace)
{
    p__talctl(TALCTL_FREE_TRACE_BUFFER, &trace, sizeof(trace), TAL_NULL, TAL_NULL);
}

#ifdef __cplusplus
}
#endif //  __cplusplus

#endif // TAL_DISABLE

#endif // ndef TAL_DOXYGEN
#endif

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
