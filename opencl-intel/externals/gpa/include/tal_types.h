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

/** @file */

#ifndef TAL_TYPES
#define TAL_TYPES
#include "tal_platforms.h"


// When updating the below versions please update the change log below. 
#define TAL_API_VERSION_MAJOR 0x00B // Major API version incremented when API is changed in such a way as to cause in compatibility with previous 
                                    // version, any api change that will cause incompatibility between TAL and TalCapture. 
#define TAL_API_VERSION_MINOR 3 // Minor API version incremented whenever a new api is added. (includeing trace macros) 

#define TAL_FILE_VERSION_MAJOR ((TAL_UINT32)TAL_FILE_VERSION_7) // Major API version incremented when the file format changes structurally.in a way that would make it incompatible with previous version. 
                // Keep TAL_FILE_VERSION enum and TAL_DetermineFileVersion updated as you change file formats.
#define TAL_FILE_VERSION_MINOR 1 // Minor API version incremented whenever a new command is added. 

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
** API VERSION
** 0x00B,1 - Added private TAL_GetStringHandleEx ctl and api
** 0x00B,1 - Extended TAL_Trace to include flush function, and new overflow buffer support
** 0x00B,2 - Added TALX_ID support, Remove RegisterTransport, InjectTrace
** 0x00B,3 - addition of new dependency enum values
** FILE VERSION
** TAL_FILE_VERSION_6,2 - Added TAL_ANNOTE_TRACE_BUFFER_FLUSH_TIME and for trace header data TAL_ESCAPE_OPCODE_PROCESS_DATA 
** TAL_FILE_VERSION_7,0 - Change all first connect info to TAL_ESCAPE_OPCODE_PROCESS_DATA 
** TAL_FILE_VERSION_7,1 - Addition of new dependency enum values
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

typedef char TAL_BOOL;
#define TAL_TRUE  1
#define TAL_FALSE 0

typedef char TAL_INT8;
typedef unsigned char TAL_UINT8;
typedef TAL_UINT8 TAL_BYTE;

#if TAL_PLATFORM == TAL_PLATFORM_WINDOWS
typedef __int16 TAL_INT16;
typedef __int32 TAL_INT32;
typedef __int64 TAL_INT64;
typedef unsigned __int16 TAL_UINT16;
typedef unsigned __int32 TAL_UINT32;
typedef unsigned __int64 TAL_UINT64;
#elif TAL_PLATFORM == TAL_PLATFORM_LRB || TAL_PLATFORM == TAL_PLATFORM_NIX
#ifndef TAL_KERNEL
#include <stdint.h>
#else
#include <sys/stddef.h>
#include <sys/types.h>
#endif
typedef int16_t TAL_INT16;
typedef int32_t TAL_INT32;
typedef int64_t TAL_INT64;
typedef uint16_t TAL_UINT16;
typedef uint32_t TAL_UINT32;
typedef uint64_t TAL_UINT64;
#else 
    #error Types not defined for this platform
#endif

#ifndef TAL_DOXYGEN
# ifndef TAL_KERNEL
#  include <stdarg.h>
#  define TAL_VA_LIST va_list
#  define TAL_VA_START va_start
#  define TAL_VA_END va_end
#else
#  define TAL_VA_LIST __va_list
#  define TAL_VA_START __builtin_va_start
#  define TAL_VA_END __builtin_va_end
# endif
#endif // TAL_DOXYGEN

#define TAL_TLS  __declspec(thread)
#if TAL_COMPILER != TAL_COMPILER_GCC
#define TAL_UNUSED(sym) sym;
#else // TAL_COMPILER == TAL_COMPILER_GCC
#define TAL_UNUSED(sym) (void)sym;
#endif // TAL_COMPILER 

#define TAL_MAXHEADER_SIZE 256
//////////////////////////////////////////////////////////////////////////
/**
 ** \struct TAL_TRACE 
 ** Every TAL call that collects data requires a TAL_TRACE* handle as its first argument.
 ** For efficiency, each of these calls puts the data into a local buffer associated with the TAL_TRACE* handle. 
 ** 
 ** To get a handle to the <b>current thread's trace</b> handle, call TAL_GetThreadTrace().
 **
 ** <b>Never create these structures yourself!</b>
 **
 ** <b>You should treat this data structure as opaque.</b> Its internal member variables
 ** are subject to change at any time.
 ***********************************************************************/
#ifndef TAL_DOXYGEN
typedef struct _TAL_TRACE TAL_TRACE;
typedef void (*TAL_FlushFn)(TAL_TRACE* in_pTrace);	

struct _TAL_TRACE 
{
    // ATTENTION TAL DEVELOPERS: don't even THINK about deleting or rearranging these fields. It is OK to add fields
    // at the end of the struct.

    // most frequently used fields
    TAL_UINT32*           cur;    // where we are currently putting data into the buffer. The range [begin,cur] is unsent to the server.
    TAL_UINT32*           end;
    TAL_UINT32            captureLevel;
    TAL_UINT64            captureCategory;
    TAL_UINT32            hwPerfCounterHandles[4];

    // used just during flush...
    TAL_UINT64            pid;
    TAL_UINT64            tid;
    TAL_UINT32            headerSize;
    TAL_UINT32*           header;
    // more stuff used just during flush...
    struct _TAL_TRACE*    pNext;
    TAL_UINT32*           buffer; // pointer to the real trace buffer. In normal operation, begin will == buffer. However, in the case of a SendTracesEx(bFlushAll=true), the begin pointer on all traces will be updated to a different location than buffer.
    TAL_UINT32*           begin;  // the begin pointer is the location in buffer where we have un-flushed data. Usually, begin is == buffer. However, if we force a flush a buffer using TAL_SendTracesEx(true), begin will move independently of buffer.
    TAL_UINT32            flushLock;
    TAL_UINT64            bufferStartTime;
    TAL_UINT32*           overflow; // used by internal logic

    TAL_FlushFn			  pfnFlush;  // Flush Proc
};
#else
struct TAL_TRACE;
#endif

typedef struct _TAL_PROCESS TAL_PROCESS;
#ifndef TAL_DOXYGEN
typedef TAL_UINT64 (*TAL_GetProcessClockbaseFn)(void* fnData);
#endif

typedef TAL_UINT32 TAL_STRING_HANDLE;

typedef TAL_BYTE TAL_ID_NAMESPACE;

#ifndef TAL_DOXYGEN
typedef enum _TAL_CAPTURE_MODE 
{
    TAL_CAPTURE_MODE_STREAMING,
    TAL_CAPTURE_MODE_CONTINUOUS,
} TAL_CAPTURE_MODE;
#endif
//////////////////////////////////////////////////////////////////////////

typedef void (TAL_CALL *TAL_LogProc)(const char*);

#ifndef TAL_DOXYGEN
	typedef void (*TAL_DistillerFn)(TAL_TRACE* out_realtimeOutputStream, TAL_UINT32* in_pStart, TAL_UINT32 sizeInDwords);
#endif //TAL_DOXYGEN

#ifdef __cplusplus
	#define TAL_NULL (0)
#else //!__cplusplus
	#define TAL_NULL ((void*)0)
#endif // !__cplusplus
#endif // TAL_TYPES

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
