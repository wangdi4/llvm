/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) 2010, Intel Corporation. All rights reserved.
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
*LICENSED
** ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
** INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT PROVIDE ANY
*UPDATES,
** ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY WARRANTY OF
** MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY PARTICULAR PURPOSE, OR ANY
** OTHER WARRANTY.  Intel disclaims all liability, including liability for
** infringement of any proprietary rights, relating to use of the code. No
*license,
** express or implied, by estoppel or otherwise, to any intellectual property
** rights is granted herein.
**
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#if (!defined(TAL_OPCODES_H)) || defined(INHIBIT_MANAGED_DECL)
#define TAL_OPCODES_H
#include "../common/managed_decl.h"

#ifdef _MANAGED
#ifndef INHIBIT_MANAGED_DECL
#if defined(__TALTRACE_H__)
namespace GT {
namespace Tal {
#else  // !__TALTRACE_H__
namespace libTaskalyzer {
#endif // !__TALTRACE_H__
#endif // INHIBIT_MANAGED_DECL
#endif //_MANAGED

#ifndef TAL_DOXYGEN
BEGIN_ENUM_DECL(TAL_OPCODE){
    /* 0x3FF is max value */ /* keep this sync'd with taskalyzer_opcodes.cpp's
                                lut */
    TAL_OPCODE_NOP = 0x00,

    TAL_ANNOTE_PROCESSNAME = 0x01, // deprecated in favor of ANNOTE_PROCESSID.
    TAL_ANNOTE_TRACENAME = 0x02,   // deprecated in favor of ANNOTE_THREADID.
    TAL_ANNOTE_CLOCKPARAMS =
        0x03, // deprecated in favor of TAL_ANNOTE_CLOCKPARAMS_ID
    TAL_ANNOTE_MODULEINFO =
        0x04, // deprecated in favor of TAL_ANNOTE_MODULEINFO_ID
    TAL_ANNOTE_MASTERPROCESS =
        0x05, // deprecated in favor of TAL_ANNOTE_MASTERPROCESS_ID
    // 0x06 Unused (was once TAL_ANNOTE_PROPERTY but was never implemented)
    TAL_ANNOTE_VERSION = 0x07, TAL_ANNOTE_TASK_COLOR = 0x08,
    TAL_ANNOTE_NAMED_TASK_COLOR = 0x09, TAL_ANNOTE_STRING_HANDLE = 0x0A,
    TAL_ANNOTE_COUNTER_SAMPLE_TYPE = 0x0B,
    // 0x0C reserved for something important. Put new annotes down in 0x65
    // opcode space. 0x0D reserved for something important. Put new annotes down
    // in 0x65 opcode space. 0x0E reserved for something important. Put new
    // annotes down in 0x65 opcode space. 0x0F reserved for something important.
    // Put new annotes down in 0x65 opcode space.

    TAL_OPCODE_BEGIN_TASK = 0x10, TAL_OPCODE_BEGIN_NAMED_TASK = 0x11,
    TAL_OPCODE_SET_CURRENT_TASKID = 0x12, // deprecated
    TAL_OPCODE_BEGIN_NAMED_TASK_H = 0x13, TAL_OPCODE_BEGIN_TASK_WITH_ID = 0x14,
    TAL_OPCODE_BEGIN_NAMED_TASK_WITH_ID = 0x15,
    TAL_OPCODE_BEGIN_NAMED_TASK_H_WITH_ID = 0x16,

    // 0x17 - 0x1D: ITT API tasks
    TAL_OPCODE_ITT_TASK_BEGIN = 0x17, TAL_OPCODE_ITT_TASK_BEGIN_FN = 0x18,
    TAL_OPCODE_ITT_TASK_END = 0x19,
    // 0x1A Unused (was once TAL_OPCODE_ITT_TASK_END_OVERLAPPED which got
    // renamed and moved higher)
    TAL_OPCODE_ITT_TASK_BEGIN_EX = 0x1B, TAL_OPCODE_ITT_TASK_BEGIN_FN_EX = 0x1C,
    TAL_OPCODE_ITT_TASK_END_EX = 0x1D,

    TAL_OPCODE_END_TASK = 0x1E,

    TAL_OPCODE_ID_CREATED = 0x20,
    TAL_OPCODE_ADD_TASK_RELATION = 0x21, // deprecated, replaced by ADD_RELATION
                                         // and ADD_RELATION_TO_CURRENT
    TAL_OPCODE_ID_RETIRED = 0x22, TAL_OPCODE_TASK_STOLEN = 0x23,
    TAL_OPCODE_ADD_RELATION = 0x24, TAL_OPCODE_ADD_RELATION_TO_CURRENT = 0x25,
    TAL_OPCODE_ADD_RELATION_THIS = 0x26,

    TAL_OPCODE_ITT_RELATION_ADD_TO_CURRENT = 0x27,
    TAL_OPCODE_ITT_RELATION_ADD = 0x28,
    TAL_OPCODE_ITT_RELATION_ADD_TO_CURRENT_EX = 0x29,
    TAL_OPCODE_ITT_RELATION_ADD_EX = 0x2A,

    TAL_OPCODE_NAMED_MARKER = 0x30, TAL_OPCODE_NAMED_EVENT = 0x31,

    TAL_OPCODE_ITT_MARKER = 0x32, TAL_OPCODE_ITT_ID_CREATE = 0x33,
    TAL_OPCODE_ITT_ID_DESTROY = 0x34,

    TAL_OPCODE_ITT_ID_CREATE_EX = 0x35, TAL_OPCODE_ITT_ID_DESTROY_EX = 0x36,
    TAL_OPCODE_ITT_MARKER_EX = 0x37,

    // 0x38 thru 0x4F reserved for sw counters
    TAL_OPCODE_ADD_COUNTER = 0x38, TAL_OPCODE_SUB_COUNTER = 0x39,
    TAL_OPCODE_ADD_COUNTER_H = 0x3A, TAL_OPCODE_SUB_COUNTER_H = 0x3B,
    TAL_OPCODE_SAMPLE_COUNTER = 0x3C, TAL_OPCODE_SAMPLE_COUNTER_H = 0x3D,
    // 0x3E-0x43 inclusive reserved for 32b variants of above 6 opcodes

    // 0x44 - 0x45: ITT API V2, counters
    TAL_OPCODE_ITT_COUNTER_INC_V3 = 0x44,
    TAL_OPCODE_ITT_COUNTER_INC_DELTA_V3 = 0x45,

    // 0x50 thru 0x5F reserved for parameters
    TAL_OPCODE_PARAM_I32V = 0x50, TAL_OPCODE_PARAM_FV = 0x51,
    TAL_OPCODE_PARAM_S = 0x52, TAL_OPCODE_PARAM_I64V = 0x53,
    TAL_OPCODE_PARAM_I32V_H = 0x54, TAL_OPCODE_PARAM_FV_H = 0x55,
    TAL_OPCODE_PARAM_S_H = 0x56, TAL_OPCODE_PARAM_I64V_H = 0x57,

    // 0x58 - 0x5D: ITT API V2, metadata
    TAL_OPCODE_ITT_METADATA_ADD_I64 = 0x58,
    TAL_OPCODE_ITT_METADATA_ADD_I32 = 0x59,
    TAL_OPCODE_ITT_METADATA_ADD_I16 = 0x5A,
    TAL_OPCODE_ITT_METADATA_ADD_R = 0x5B, TAL_OPCODE_ITT_METADATA_ADD_S = 0x5C,
    TAL_OPCODE_ITT_METADATA_ADD_UDT = 0x5D,

    // 0x60 thru 8F for more annotations
    // 0x60-0x64 inclusive are currently unused.
    TAL_ANNOTATE_THREAD_NAME = 0x65, // deprecated in favor of TID_NAME
    TAL_ANNOTE_BUILD_VERSION = 0x66, TAL_ANNOTE_PID_NAME = 0x67,
    TAL_ANNOTE_TID_NAME = 0x68,
    // 0x69 reserved for TAL_ANNOTE_ADAPTERID_NAME
    // 0x6A reserved for TAL_ANNOTE_ADAPTERID // will eventually be used for uOS
    // tracing and hardware topology information...
    TAL_ANNOTE_PROCESSID = 0x6B, TAL_ANNOTE_THREADID = 0x6C,
    TAL_ANNOTE_CLOCKPARAMS_ID = 0x6D, TAL_ANNOTE_MODULEINFO_ID = 0x6E,
    TAL_ANNOTE_MASTERPROCESS_ID = 0x6F, TAL_ANNOTE_CLOCK_JITTER = 0x70,
    TAL_ANNOTE_TRACE_BUFFER_FLUSH_TIME = 0x71,
    TAL_ANNOTE_TRACE_BUFFER_FLUSH_TIME2 = 0x72,
    TAL_ANNOTE_PSEUDO_PROCESS = 0x73, TAL_ANNOTE_PLATFORM = 0x74,
    TAL_ANNOTE_SHADER = 0x75, TAL_ANNOTE_ITT_CLOCK_DOMAIN = 0x76,

    TAL_ANNOTE_TID_PARENT_PID =
        0x80, // the string handle id of the parent process of this tid

    // 0x90 thru 0xAF reserved for HW counter related datasets

    // 0xB0 - CF are reserved for scheduler and kernel-related opcodes

    // 0xD0 thru DF for virtual task opcodes
    TAL_OPCODE_BEGIN_NAMED_VIRTUAL_TASK_WITH_ID = 0xD0,
    TAL_OPCODE_BEGIN_NAMED_VIRTUAL_TASK_H_WITH_ID = 0xD1,
    TAL_OPCODE_END_VIRTUAL_TASK = 0xD2,

    // 0xD3 - 0xD6: ITT API V2, regions, frames, task_groups, events
    TAL_OPCODE_ITT_REGION_BEGIN = 0xD3, TAL_OPCODE_ITT_REGION_END = 0xD4,
    TAL_OPCODE_ITT_FRAME_BEGIN_V3 = 0xD5, TAL_OPCODE_ITT_FRAME_END_V3 = 0xD6,
    TAL_OPCODE_ITT_TASK_GROUP = 0xD7, TAL_OPCODE_ITT_EVENT_BEGIN = 0xD8,
    TAL_OPCODE_ITT_EVENT_END = 0xD9,

    // 0x0E0 - 0x300 are open opcode space... if you have to go above 300,
    // be really sure you've planned your opcode allocation in detail

    TAL_ANNOTE_ITT_DOMAIN = 0xE0, TAL_ANNOTE_ITT_STRING_HANDLE = 0xE1,
    TAL_ANNOTE_ITT_THREAD_SET_NAME = 0xE2,

    // 0xF0 - 0xFF for PMU counter, stack, and ETW context switch data
    TAL_OPCODE_PMU_COUNTERS = 0xF0, TAL_OPCODE_STACK_TRACEV = 0xF1,
    TAL_OPCODE_STACK_SYMBOL_NAME = 0xF2, TAL_OPCODE_CONTEXT_SWITCH_V1 = 0xF3,
    TAL_OPCODE_CONTEXT_SWITCH_V2 = 0xF4,

    // 0x100 - 0x102: ITT Custom Track Groups and Tracks
    TAL_OPCODE_ITT_CREATE_TRACK_GROUP = 0x100,
    TAL_OPCODE_ITT_CREATE_TRACK = 0x101, TAL_OPCODE_ITT_ANNOTE_TRACK = 0x102,

    // 0x110: TSC Trace Variance Detection
    TAL_OPCODE_GFX_ENTRY = 0x110,

    // 0x120 - 0x123: ITT queue tasks (formerly called overlapping tasks)
    TAL_OPCODE_ITT_QUEUE_TASK_BEGIN = 0x120,
    TAL_OPCODE_ITT_QUEUE_TASK_BEGIN_EX = 0x121,
    TAL_OPCODE_ITT_QUEUE_TASK_END = 0x122,
    TAL_OPCODE_ITT_QUEUE_TASK_END_EX = 0x123,

    // Allocate from 0x3FF down to 0x300 for file-format related opcodes
    TAL_OPCODE_ESCAPE =
        0x3FD, // all escape codes must be followed with a uint(4 bytes) that is
               // the size in bytes of the payload of this escape
    TAL_OPCODE_DELIMITER = 0x3FE} END_ENUM_DECL(TAL_OPCODE)
#endif // ndef TAL_DOXYGEN

/* TAL_OPCODEINFO::dasm_* contain control commands for the taskalyzer
disassembler.
* dasm_argA is an format string for the argA portion of
TAL_COMMAND(opcode,nDWords,argA,argB)
* dasm_argB is an format string for the argB portion of
TAL_COMMAND(opcode,nDWords,argA,argB)
* dasm_payload is a comma-delimited sequence of format strings for the command's
payload

* A disassembler command has the following format:
*    name:semantic
*
* Currently supported semantics are:
*    %s   String
*    %ts  Timestamp
*    %addr  Function pointer
*/
#ifndef TAL_DOXYGEN
    typedef struct _TAL_OPCODEINFO {
  TAL_OPCODE opcode;
  const char *name;
  const char *dasm_argA;
  const char *dasm_argB;
  const char *dasm_payload;
} TAL_OPCODEINFO;
#endif

#ifndef TAL_DOXYGEN
typedef struct _TAL_RELATIONINFO {
  TAL_RELATION relation;
  const char *name;
  const char *familiar_name;
  const char *familiar_name_plural;
} TAL_RELATIONINFO;
#endif

#ifdef _MANAGED
#ifndef INHIBIT_MANAGED_DECL
#if defined(__TALTRACE_H__)
} // namespace GT {
} // namespace Tal {
#else  // !__TALTRACE_H__
} //  namespace libTaskalyzer {
#endif // !__TALTRACE_H__
#endif // INHIBIT_MANAGED_DECL
#endif //_MANAGED

#if defined(_MANAGED)
#ifndef INHIBIT_MANAGED_DECL
#define INHIBIT_MANAGED_DECL
#include "public\tal_opcodes.h"
#undef INHIBIT_MANAGED_DECL
#endif // INHIBIT_MANAGED_DECL
#endif //_MANAGED
#undef __MANAGED_DECL_H__

#endif // TAL_OPCODES_H

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
