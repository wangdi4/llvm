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
#ifndef __TALCTL_H__
#define __TALCTL_H__
#ifndef TAL_DOXYGEN // this entire file is invisible to Doxygen

//-----------------------------------------------------------
// INSTRUCTIONS FOR ADDING A NEW TALCTL CODE:
//-----------------------------------------------------------
// 1. Define the new code in the TALCTL_CODE enum.
// 2. Define any new code-specific structs below.
// 3. Determine whether any default behavior is required. If so, add it to
// __Default_talctl().
// 4. Add an entry to talCtlArgInitTable[] in Capture.cpp.
// 5. Add an entry to the talctl() implementation in Capture.cpp.
// 6. Be sure to bump  TAL_API_VERSION_MAJOR.

// ** DON'T EVEN THINK ABOUT REMOVING A TALCTL CODE OR CHANGING ANY OF THE
// VALUES!! **

typedef enum {
  TALCTL_SEND_TRACES_EX = 0x00,
  TALCTL_GET_TIMESTAMP_FREQ = 0x01,
  TALCTL_GET_PROCESSOR_FREQ = 0x02,
  TALCTL_IS_CAPTURING = 0x03,
  TALCTL_SET_LOG_LEVEL = 0x04,
  TALCTL_SET_LOG_CATEGORY = 0x05,
  TALCTL_PROCESS_CREATE = 0x06,
  TALCTL_ALLOC_TRACE_BUFFER = 0x07,
  TALCTL_FREE_TRACE_BUFFER = 0x08,
  TALCTL_DEPRECATED_1 = 0x09,
  TALCTL_DEPRECATED_2 = 0x0A,
  TALCTL_REGISTER_COLLECTOR = 0x0B,
  TALCTL_ADD_STRING_TO_POOL = 0x0C,
  TALCTL_SET_COUNTER_SAMPLE_TYPE = 0x0D,
  TALCTL_SET_TASK_COLOR = 0x0E,
  TALCTL_SET_NAMED_TASK_COLOR = 0x0F,
  TALCTL_SET_THREAD_NAME = 0x10,
  TALCTL_SET_TRACE_NAME = 0x11,
  TALCTL_DESCRIBE_CATEGORY_MASK = 0x12,
  TALCTL_SET_NETSIM = 0x13,
  TALCTL_REGISTER_ID_NAMESPACE = 0x14,
  TALCTL_SET_LOG_FUNCTION = 0x15,
  TALCTL_UNREGISTER_COLLECTOR = 0x16,
  TALCTL_PERFORMANCE_TEST = 0x17,
  TALCTL_GET_LRB_SO_PATH = 0x18,
  TALCTL_FORCE_REF =
      0x19, // Don't even THINK about filling the gap between 0x19 and 0x20!!
  TALCTL_NOP = 0x20,
  TALCTL_GET_STRING_HANDLE = 0x21,
  TALCTL_GET_THREAD_TRACE_EX = 0x22,
  TALCTL_GET_CURRENT_PROCESS_HANDLE = 0x23,
  TALCTL_SEND_DATA_EX = 0x24,
  TALCTL_REGISTER_DISTILLER = 0x25,
  TALCTL_UNREGISTER_DISTILLER = 0x26,
  TALCTL_GET_STRING_HANDLE_EX = 0x27,
  TALCTL_MALLOC = 0X28,
  TALCTL_FREE = 0x29,
  TALCTL_GET_NEXT_TALX_ID = 0X2A,
  TALCTL_GET_PUSHERS = 0X2B,
  TALCTL_SET_STRING_HANDLE = 0X2C,
  TALCTL_TEST_TRIGGER_CONDITIONS = 0x2D,
  TALCTL_SHUTDOWN = 0X2E,
  TALCTL_SET_CLOCK_BASE = 0x2F,

  TALCTL_CODE_MAX
} TALCTL_CODE;

// TALCTL code-specific structures

// NOTE: It is possible to modify these structs in subsequent releases, but
// fields must only be added to the end of the struct, and the talctl()
// implementation must be written to support previous versions of the structs.
// The length supplied in the inputSize param identifies which version of the
// struct is being provided. When a struct is redefined, the previous definition
// should be renamed to <old_name>_V0, or something like that, so that talctl()
// can cast to the correct definition for the given length.

#pragma pack(push, 1)

typedef struct {
  TAL_PROCESS *process;
  TAL_UINT64 tid;
} TALCTL_ALLOC_TRACE_BUFFER_PARAMS;

typedef struct {
  TAL_UINT32 adapter_id;
  TAL_UINT64 pid;
  const char *name;
  TAL_UINT64 clockFreq;
  TAL_UINT64 clockBase;
  TAL_GetProcessClockbaseFn getClockBaseFunc;
  void *getClockBaseFuncData;
} TALCTL_PROCESS_CREATE_PARAMS;

typedef struct {
  const char *type;
  const char *param;
} TALCTL_REGISTER_COLLECTOR_PARAMS;

typedef TALCTL_REGISTER_COLLECTOR_PARAMS TALCTL_UNREGISTER_COLLECTOR_PARAMS;

typedef struct {
  const char *name;
  TAL_DistillerFn getDistillerFunc;
} TALCTL_REGISTER_DISTILLER_PARAMS;

typedef struct {
  const char *name;
} TALCTL_UNREGISTER_DISTILLER_PARAMS;

typedef struct {
  const char *str;
  TAL_COUNTER_SAMPLE_TYPE type;
} TALCTL_SET_COUNTER_SAMPLE_TYPE_PARAMS;

typedef struct {
  void (*fn)(void);
  TAL_UINT8 red;
  TAL_UINT8 green;
  TAL_UINT8 blue;
} TALCTL_SET_TASK_COLOR_PARAMS;

typedef struct {
  const char *name;
  TAL_UINT8 red;
  TAL_UINT8 green;
  TAL_UINT8 blue;
} TALCTL_SET_NAMED_TASK_COLOR_PARAMS;

typedef struct {
  TAL_TRACE *handle;
  const char *name;
} TALCTL_SET_TRACE_NAME_PARAMS;

typedef struct {
  TAL_UINT64 mask;
  const char *tag;
  const char *description;
} TALCTL_DESCRIBE_CATEGORY_MASK_PARAMS;

typedef struct {
  TAL_ID_NAMESPACE ns;
  const char *ns_name;
} TALCTL_REGISTER_ID_NAMESPACE_PARAMS;

typedef struct {
  const char *path;
} TALCTL_GET_LRB_SO_PATH_PARAMS;

typedef struct {
  TAL_PROCESS *process;
  TAL_UINT64 tid;
} TALCTL_GET_THREAD_TRACE_EX_PARAMS;

typedef struct {
  TAL_PROCESS *process;
  const char *string;
} TALCTL_GET_STRING_HANDLE_EX_PARAMS;

typedef struct {
  TAL_PROCESS *process;
  TAL_STRING_HANDLE id;
  const char *string;
} TALCTL_SET_STRING_HANDLE_PARAMS;

typedef struct {
  TAL_STRING_HANDLE nameHandle;
  int value;
} TALCTL_TEST_TRIGGER_CONDITIONS_PARAMS;

#pragma pack(pop)

#if !defined(TAL_MIN)
#define TAL_MIN(a, b) (a < b ? a : b)
#endif

#define talCtlInput(input, size, arg)                                          \
  memcpy(&arg, input, TAL_MIN(inputSize, sizeof(arg)))
#define talCtlOutput(arg, output, outputSize)                                  \
  memcpy(output, &arg, TAL_MIN(*outputSize, sizeof(arg)));                     \
  *outputSize = TAL_MIN(*outputSize, sizeof(arg));

// NOTE: This solves the problem for the existing set of APIs with their current
// argument sizes. If any of the arguments are extended in the future, this
// structure will need to be modified then.
struct TalCtlApiSpec {
  TAL_UINT32 inputSize;
  TAL_UINT32 outputSize;
  const char *friendlyName;
};

// Required TAL Capture entry points.
typedef TAL_BOOL TAL_CALL TALCTL_FUNC(TALCTL_CODE code, const void *input,
                                      TAL_UINT32 inputSize, void *output,
                                      TAL_UINT32 *outputSize);
typedef TAL_BOOL TAL_CALL TALVALIDATETALVERSION_FUNC(const TAL_VERSION *);
typedef TAL_TRACE *TAL_CALL TALGETTHREADTRACE_FUNC(void);
typedef void TAL_CALL TALFLUSH_FUNC(TAL_TRACE *);
// DJH TODO: Resolve whether these entry points can remain as part of the TAL
// api, or should we find another
//           that doesn't expose them (TALCTL codes?)    UNRESOLVED DEC-2010
typedef void TAL_CALL TALCAPTURESTACKBACKTRACE_FUNC(TAL_TRACE *, TAL_UINT32,
                                                    TAL_UINT64);
typedef void TAL_CALL TALREADPMUCOUNTERS_FUNC(TAL_TRACE *);

// implementations from tal.lib
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
extern TALCTL_FUNC *p__talctl;
extern TALVALIDATETALVERSION_FUNC *p__talValidateTalVersion;
extern TALGETTHREADTRACE_FUNC *p__TAL_GetThreadTrace;
extern TALFLUSH_FUNC *p__TAL_Flush;
// DJH TODO: Resolve whether these entry points can remain as part of the TAL
// api, or should we find another
//           that doesn't expose them (TALCTL codes?)    UNRESOLVED DEC-2010
extern TALCAPTURESTACKBACKTRACE_FUNC *p__TAL_CaptureStackBackTrace;
extern TALREADPMUCOUNTERS_FUNC *p__TAL_ReadPMUCounters;

#ifdef __cplusplus
}
#endif // __cplusplus

struct TalCtlExports {
  TALCTL_FUNC *pTalCtl;
  TALVALIDATETALVERSION_FUNC *pTalValidateTalVersion;
  TALGETTHREADTRACE_FUNC *pTalGetThreadTrace;
  TALFLUSH_FUNC *pTalFlush;
};

#endif // TAL_DOXYGEN
#endif // __TALCTL_H__
/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
