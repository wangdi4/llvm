/*****************************************************************************\

Copyright 2000 - 2008 Intel Corporation All Rights Reserved.

    The source code contained or described herein and all documents related to
    the source code ("Material") are owned by Intel Corporation or its suppliers
    or licensors. Title to the Material remains with Intel Corporation or its
    suppliers and licensors. The Material contains trade secrets and proprietary
    and confidential information of Intel or its suppliers and licensors. The
    Material is protected by worldwide copyright and trade secret laws and
    treaty provisions. No part of the Material may be used, copied, reproduced,
    modified, published, uploaded, posted, transmitted, distributed, or
    disclosed in any way without Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other intellectual
    property right is granted to or conferred upon you by disclosure or delivery
    of the Materials, either expressly, by implication, inducement, estoppel or
    otherwise. Any license under such intellectual property rights must be
    express and approved by Intel in writing.

File Name: os_inc.h

Abstract: 

Notes:THIS IS A LINUX SPECIFIC FILE

\*****************************************************************************/
#pragma once

#if 0
//The project will pick the LINUX OS INCLUDE FILE
#ifdef TC_TESTER
#include <sys/time.h>
#endif
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <list>

#include <inttypes.h>
#include <malloc.h>
#include <dlfcn.h>
#include <assert.h>

//Move these includes out of here...
#ifdef ANDROID
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <CL/cl_gl.h>
#include <CL/cl.h>
#include <pthread.h>
#include <unistd.h>

#include "performance_counters.h"

using namespace std;


#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define API
#define OSAPI
#define CCPPAPI
#define CLAPI_EXPORT __attribute__((visibility("default")))

#define INFINITE        ~0
#define WAIT_OBJECT_0    0
#define WAIT_FAILED      0
#define MAX_PATH       260

#define TEXT(quote) quote
#define CREATE_EVENT_INITIAL_SET 0
#define STANDARD_RIGHTS_ALL 0
#define EVENT_MODIFY_STATE 0
#define INSTR_OA_COUNTERS_NUM_GEN7_5 (sizeof(sInstrOACounters_Gen7_5) / sizeof(DWORD))

#define VER_NT_WORKSTATION 0x0000001


#define MAX_KMD_SSH_HEAP_BLOCK_SIZE    ( 16 * KB ) // This is SSH size that is carved out of the command buffer.

typedef unsigned char UCHAR;

typedef uint64_t   QWORD;      //  64-bits,    8-bytes
typedef uint32_t   DWORD;      //  32-bits,    4-bytes
typedef uint16_t   WORD;       //  16-bits,    2-bytes
typedef uint8_t    BYTE;       //   8-bits,    1-byte

typedef uint64_t   UINT64;     //  64-bits,    8-bytes
typedef uint64_t   ULONG64;    //  64-bits,    8-bytes
typedef uint32_t   UINT;       //  32-bits,    4-bytes
typedef uint32_t   UINT32;     //  32-bits,    4-bytes
typedef uint32_t   ULONG;      //  FIXME: USC forces bad typedef. Problems can surface on linux64 platform.
typedef uint16_t   UINT16;     //  16-bits,    2-bytes

typedef int64_t    INT64;      //  64-bits,    8-bytes
typedef int64_t    LONG64;     //  64-bits,    8-bytes
typedef int32_t    INT;        //  32-bits,    4-bytes
typedef int32_t    LONG;       //  32-bits,    4-bytes
typedef int64_t    LONGLONG;   //  64-bits,    8-bytes
typedef int64_t*   PLONGLONG;  //  pointer to 64-bits,    8-bytes

typedef int64_t    LONGLONG;

typedef int32_t    BOOL;
typedef uint8_t    BOOLEAN;    //  1-bit
typedef uintptr_t  UINT_PTR;   //  Size the same as pointer

typedef void            VOID;
typedef void *          PVOID;
typedef void *          LPVOID;
typedef uintptr_t       HANDLE;
typedef uintptr_t       HDC;       // HDC
typedef uintptr_t       HWND;      // HWND
typedef uintptr_t       HGLRC;     // HGLRC
typedef uintptr_t       HINSTANCE; // HINSTANCE
typedef uintptr_t       HMODULE;   // HMODULE
typedef char            CHAR;
typedef CHAR *          LPSTR;
typedef unsigned char   UCHAR;
typedef void*           OS_HMODULE; 
typedef OS_HMODULE      OS_HINSTANCE;

#if defined __x86_64__
    typedef int64_t INT_PTR;
#else
    typedef int32_t INT_PTR;
#endif

#ifndef ulong
    typedef unsigned long ulong;
#endif

typedef INT_PTR (*PROC)();
typedef INT_PTR (*FARPROC)();

typedef int errno_t;
//Linux use cdecl as default one
#define __stdcall __attribute__((__stdcall__))
#define __cdecl __attribute__((__cdecl__))


//Linux OS structures and methods

namespace OCLRT
{
    //Data Types:
    typedef struct LINUX_CRITICAL_SECTION
    {
        pthread_mutex_t lock;
        BOOL initialized;
    } OS_CRITICAL_SECTION;                  // CRITICAL_SECTION

    typedef void*           OS_HANDLE;      // D3DKMT_HANDLE

    static const HANDLE     HNULL     = 0;
    static const OS_HANDLE  OS_HNULL  = 0;
    static const OS_HMODULE OS_HMNULL = NULL;


    #pragma pack(push, 1)

    // Memory Management...

    typedef struct _tagPatchLocationList
    {
        uint32_t    AllocationIndex;
        union
        {
            struct
            {
                uint32_t SlotId   : 24;
                uint32_t Reserved :  8;
            };
            uint32_t Value;
        };
        uint32_t    DriverId;
        uint32_t    AllocationOffset;
        uint32_t    PatchOffset;
        uint32_t    SplitOffset;
    } OS_PATCHLOCATIONLIST, *POS_PATCHLOCATIONLIST;

    typedef struct _tagAllocationList
    {
        OS_HANDLE hAllocation;
        union
        {
            struct
            {
                uint32_t WriteOperation      :  1;
                uint32_t DoNotRetireInstance :  1;
                uint32_t Reserved            : 30;
            };
            uint32_t Value;
        };
    } OS_ALLOCATIONLIST, *POS_ALLOCATIONLIST;


    typedef struct tagOCLRTPatchLocationListInfo
    {
        cl_uint AllocationIndex;
        cl_uint SlotID;
        cl_uint AllocationOffset;
        cl_uint PatchOffset;
        cl_uint SplitOffset;
        cl_uint DriverID;
    } OCLRT_PATCH_LOCATION_LIST_INFO, *POCLRT_PATCH_LOCATION_LIST_INFO;

    typedef struct _tagOSCmdBufferInfo
    {
        OS_HANDLE               hContext;               // Handle of the command buffer.

        void *                  pCommandBuffer;
        uint32_t                CommandBufferSize;

        POS_ALLOCATIONLIST      pAllocationList;
        uint32_t                AllocationListSize;

        // Command buffer management
        void *                  pCurrentPosition;
        uint32_t                PrevReqSize;

        //For KMD use...
        POS_PATCHLOCATIONLIST   pPatchLocationList;
        uint32_t                PatchLocationListSize;

        //SVM SUPPORT
        bool                    IsPageFaultSupported;

        //Linux command buffer support
        uint32_t                RenderContextID;
        uint32_t                HeapID;
        ulong                   CommandID;              // Current command ID
        ulong                   LastCompletedCmdID;     // ID of the last command completed by GPU

        // DRM late patch support.
        OS_HANDLE               SSHBO;                  // Current SSH tied to the current command buffer.
        OS_HANDLE               SSHHandle;              // Updated to current SSH only if SSH is used.
        uint32_t                SSHPatchOffset;
        uint32_t                SSHAllocationOffset;
    } OS_CMDBUF_INFO, *POS_CMDBUF_INFO;


    typedef struct _tagRenderFlags
    {
        uint32_t ResizeCommandBuffer     :  1;
        uint32_t ResizeAllocationList    :  1;
        uint32_t ResizePatchLocationList :  1;
        uint32_t NullRendering           :  1;
        uint32_t PresentRedirected       :  1;
        uint32_t RenderKm                :  1;
        uint32_t Reserved                : 26;
    } OS_RENDER_FLAGS, *POS_RENDER_FLAGS;

    typedef struct _tagRenderCB
    {
        OS_HANDLE               hContext;
        uint32_t                CommandOffset;
        uint32_t                CommandLength;
        uint32_t                AllocationCount;
        uint32_t                PatchLocationCount;
        void *                  pCommandBuffer;

        void *                  pNewCommandBuffer;
        uint32_t                NewCommandBufferSize;
        POS_ALLOCATIONLIST      pNewAllocationList;
        uint32_t                NewAllocationListSize;
        POS_PATCHLOCATIONLIST   pNewPatchLocationList;
        uint32_t                NewPatchLocationListSize;

        OS_RENDER_FLAGS         Flags;
        void *                  pPrivateDriverData;
        uint32_t                PrivateDriverDataSize;

        //Linux command buffer support
        POS_CMDBUF_INFO         pCmdBufInfo;
    } OS_RENDER, *POS_RENDER;

    typedef struct _tagEscapeData
    {
        uint64_t GPUTimeStamp;
        uint64_t CPUTimeinNS;
    } OS_ESCAPE_TIMESTAMP_DATA, *POS_ESCAPE_TIMESTAMP_DATA;


    #pragma pack(pop)

} // namespace


#ifndef TC_TESTER
class OSSyncObject
{
public:
    virtual ~OSSyncObject() {};
    virtual void Signal() = 0;
    virtual void Wait() = 0;
};

struct atomic_flag {
    bool flag;
};

class OSRenderingEvent : public OSSyncObject
{
public:    
    ~OSRenderingEvent() {};
    void Signal();
    void Wait();
};

class OSEvent : public OSSyncObject
{
public:
    OSEvent();
    ~OSEvent();
    void Signal();
    void Wait();
private:
    pthread_mutex_t m_condMutex;
    pthread_cond_t m_condVar;

    volatile atomic_flag m_flag;
};


class OSThread : public OSSyncObject
{
public:
    OSThread();
    ~OSThread();
    int Initialize( void *security,
                    uint32_t stack_size,
                    unsigned ( * start_address )( void * ),
                    void *arglist,
                    uint32_t initflag,
                    uint32_t *thrdaddr );
    void Signal();
    void Wait();
private:
    pthread_attr_t m_attr;
    pthread_t m_tid;
};
#endif// TC_TESTER

// MACROS - OS SPECIFIC

/*****************************************************************************\
MACRO: OCLRT_ALLOCATED
\*****************************************************************************/
#ifndef OCLRT_ALLOCATED
#define OCLRT_ALLOCATED( ptr, type )
#endif

/*****************************************************************************\
MACRO: OCLRT_FREED
\*****************************************************************************/
#ifndef OCLRT_FREED
#define OCLRT_FREED( ptr, type )
#endif


/*****************************************************************************\
MACRO: OCLRT_ALIGNED_MALLOC
\*****************************************************************************/
#ifndef OCLRT_ALIGNED_MALLOC
#if 0
#define OCLRT_ALIGNED_MALLOC( ptr, size, alignement )                           \
{                                                                               \
    ptr = _aligned_malloc( size, alignement );                                  \
    if( ptr != NULL )                                                           \
    {                                                                           \
        if( OCLRT::g_OCLRTDebugVariables.EnableMemoryObjectTracker == TRUE )    \
        {                                                                       \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Pointer:0x%x\n", ( INT32 ) ptr );    \
            OCLRT_DPF( OCLDBG_CRITICAL, "  File:%s\n", __FILE__ );              \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Function:%s\n", __FUNCTION__ );      \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Line:%d\n", __LINE__ );              \
            OCLRT::Allocated( ptr );                                            \
        }                                                                       \
    }                                                                           \
}
#else
#define OCLRT_ALIGNED_MALLOC( ptr , size, alignment )                           \
{                                                                               \
    ptr = memalign( alignment, size );                                          \
}
#endif
#endif

/*****************************************************************************\
MACRO: OCLRT_FREE
\*****************************************************************************/
#ifndef OCLRT_FREE
#ifdef _DEBUG
#define OCLRT_FREE( ptr )                                                       \
        {                                                                       \
            if( ptr != NULL )                                                   \
            {                                                                   \
                OCLRT_FREED( ptr, 0 )                                           \
                free( ptr );                                                    \
                ptr = NULL;                                                     \
            }                                                                   \
        }
#else
#define OCLRT_FREE( ptr )                                                       \
        {                                                                       \
            free( ptr );                                                        \
            ptr = NULL;                                                         \
        }
#endif
#endif

/*****************************************************************************\
MACRO: OCLRT_ALIGNED_FREE
\*****************************************************************************/
#ifndef OCLRT_ALIGNED_FREE
#if 0
#define OCLRT_ALIGNED_FREE( ptr )                                               \
{                                                                               \
    if( ptr != NULL )                                                           \
    {                                                                           \
        if ( OCLRT::g_OCLRTDebugVariables.EnableMemoryObjectTracker == TRUE )   \
        {                                                                       \
            OCLRT::Freed( ptr );                                                \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Pointer:0x%x\n", ( INT32 ) ptr );    \
            OCLRT_DPF( OCLDBG_CRITICAL, "  File:%s\n", __FILE__ );              \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Function:%s\n", __FUNCTION__ );      \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Line:%d\n", __LINE__ );              \
        }                                                                       \
        aligned_free( ptr );                                                   \
        ptr = NULL;                                                             \
    }                                                                           \
}
#else
#define OCLRT_ALIGNED_FREE( ptr )                                               \
{                                                                               \
    aligned_free( ptr );                                                        \
    ptr = NULL;                                                                 \
}
#endif
#endif


/*****************************************************************************\
MACRO: OCLRT_NEW
\*****************************************************************************/
#ifndef OCLRT_NEW
#if 0
#define OCLRT_NEW( ptr, size )                                                 \
{                                                                              \
    ptr = new( std::nothrow ) size;                                            \
    if ( ptr != NULL )                                                         \
    {                                                                          \
        if ( OCLRT::g_OCLRTDebugVariables.EnableMemoryObjectTracker == TRUE )  \
        {                                                                      \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Pointer:0x%x\n", ( INT32 ) ptr );   \
            OCLRT_DPF( OCLDBG_CRITICAL, "  File:%s\n", __FILE__ );             \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Function:%s\n", __FUNCTION__ );     \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Line:%d\n", __LINE__ );             \
            OCLRT::Allocated( ptr );                                           \
        }                                                                      \
    }                                                                          \
}
#else
#define OCLRT_NEW( ptr, size )                                                  \
{                                                                               \
    ptr = new( std::nothrow ) size;                                             \
}   
#endif
#endif

#ifndef OCLRT_DELETE
#if 0
#define OCLRT_DELETE( ptr )                                                     \
{                                                                               \
    if( ptr != NULL )                                                           \
    {                                                                           \
        if ( OCLRT::g_OCLRTDebugVariables.EnableMemoryObjectTracker == TRUE )   \
        {                                                                       \
            OCLRT::Freed( ptr );                                                \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Pointer:0x%x\n", ( INT32 ) ptr );    \
            OCLRT_DPF( OCLDBG_CRITICAL, "  File:%s\n", __FILE__ );              \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Function:%s\n", __FUNCTION__ );      \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Line:%d\n", __LINE__ );              \
        }                                                                       \
        delete ( ptr );                                                         \
        ptr = NULL;                                                             \
    }                                                                           \
}
#else
#define OCLRT_DELETE( ptr )                                                     \
{                                                                               \
    if( ptr != NULL )                                                           \
    {                                                                           \
        delete ( ptr );                                                         \
        ptr = NULL;                                                             \
    }                                                                           \
}   
#endif
#endif

/*****************************************************************************\
MACRO: OCLRT_DELETE_ARRAY
\*****************************************************************************/
#ifndef OCLRT_DELETE_ARRAY
#if 0
#define OCLRT_DELETE_ARRAY( ptr )                                               \
{                                                                               \
    if( ptr != NULL )                                                           \
    {                                                                           \
        if ( OCLRT::g_OCLRTDebugVariables.EnableMemoryObjectTracker == TRUE )   \
        {                                                                       \
            OCLRT::Freed( ptr );                                                \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Pointer:0x%x\n", ( INT32 ) ptr );    \
            OCLRT_DPF( OCLDBG_CRITICAL, "  File:%s\n", __FILE__ );              \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Function:%s\n", __FUNCTION__ );      \
            OCLRT_DPF( OCLDBG_CRITICAL, "  Line:%d\n", __LINE__ );              \
        }                                                                       \
        delete[] ( ptr );                                                       \
        ptr = NULL;                                                             \
    }                                                                           \
}
#else
#define OCLRT_DELETE_ARRAY( ptr )                                               \
{                                                                               \
    if( ptr != NULL )                                                           \
    {                                                                           \
        delete[] ( ptr );                                                       \
        ptr = NULL;                                                             \
    }                                                                           \
}
#endif
#endif  

/*****************************************************************************\
MACRO: OCLRT_DPF
\*****************************************************************************/
#ifndef OCLRT_DPF
#define OCLRT_DPF OCLRT::OCLRTDebugMessage
#endif // OCLRT_DPF

/*****************************************************************************\
MACRO: OCLRT_DPF_FUNCTION_ENTRY
\*****************************************************************************/
#ifndef OCLRT_DPF_FUNCTION_ENTRY
#define OCLRT_DPF_FUNCTION_ENTRY \
    OCLRT_DPF( OCLDBG_FUNCTION_ENTRY, "Enter Function: %s\n", __FUNCTION__ );
#endif // OCLRT_DPF_FUNCTION_ENTRY

/*****************************************************************************\
MACRO: OCLRT_DPF_FUNCTION_EXIT
\*****************************************************************************/
#ifndef OCLRT_DPF_FUNCTION_EXIT
#if 0
#define OCLRT_DPF_FUNCTION_EXIT( x ) \
    OCLRT_DPF( OCLDBG_FUNCTION_EXIT, "Exit Function: %s (returned 0x%8.8x)\n", __FUNCTION__, x );
#else
#define OCLRT_DPF_FUNCTION_EXIT( x ) ( void )0
#endif // _DEBUG
#endif // OCLRT_DPF_FUNCTION_EXIT

/*****************************************************************************\
MACRO: OCLRT_DPF_FUNCTION_INPUT
\*****************************************************************************/
#ifndef OCLRT_DPF_FUNCTION_INPUT
#if 0
#define OCLRT_DPF_FUNCTION_INPUT( x ) \
    OCLRT_DPF( OCLDBG_FUNCTION_INPUT, "  Input: " #x " = 0x%08x\n", x );
#else
#define OCLRT_DPF_FUNCTION_INPUT( x ) ( void )0
#endif // _DEBUG
#endif // OCLRT_DPF_FUNCTION_INPUT

/*****************************************************************************\
MACRO: OCLRT_DPF_FUNCTION_INPUT_STRING
\*****************************************************************************/
#ifndef OCLRT_DPF_FUNCTION_INPUT_STRING
#if 0
#define OCLRT_DPF_FUNCTION_INPUT_STRING( string )                    \
    OCLRT_DPF( OCLDBG_FUNCTION_INPUT, "  Input: " #string " = \n" ); \
    OCLRT::PrintInputStrings( 1, &string, NULL );
#else
#define OCLRT_DPF_FUNCTION_INPUT_STRING( x ) ( void )0
#endif // _DEBUG
#endif // OCLRT_DPF_FUNCTION_INPUT_STRING

/*****************************************************************************\
MACRO: OCLRT_DPF_FUNCTION_INPUT_STRINGS
\*****************************************************************************/
#ifndef OCLRT_DPF_FUNCTION_INPUT_STRINGS
#if 0
#define OCLRT_DPF_FUNCTION_INPUT_STRINGS( count, strings, lengths )   \
    OCLRT_DPF( OCLDBG_FUNCTION_INPUT, "  Input: " #strings " = \n" ); \
    OCLRT::PrintInputStrings( count, strings, lengths );
#else
#define OCLRT_DPF_FUNCTION_INPUT_STRINGS( count, strings, lengths ) ( void )0
#endif // _DEBUG
#endif // OCLRT_DPF_FUNCTION_INPUT_STRINGS

/*****************************************************************************\
MACRO: OCLRT_DEBUG_BREAK
\*****************************************************************************/
#ifndef OCLRT_DEBUG_BREAK
#if 0
#define OCLRT_DEBUG_BREAK                        \
    if( OCLRT::g_OCLRTDebugVariables.EnableAsserts ) \
{                                            \
    __debugbreak();                          \
}                                         
#else
#define OCLRT_DEBUG_BREAK ( void )0
#endif // _DEBUG
#endif // OCLRT_DEBUG_BREAK

/*****************************************************************************\
MACRO: OCL_ASSERT
\*****************************************************************************/
#ifndef OCL_ASSERT
#if 0
#define OCL_ASSERT( expr )                                             \
    if( !( expr ) )                                                    \
{                                                                  \
    OCLRT_DPF( OCLDBG_CRITICAL, "ASSERTION FAILURE:\n" );          \
    OCLRT_DPF( OCLDBG_CRITICAL, "  File:%s\n", __FILE__ );         \
    OCLRT_DPF( OCLDBG_CRITICAL, "  Function:%s\n", __FUNCTION__ ); \
    OCLRT_DPF( OCLDBG_CRITICAL, "  Line:%d\n", __LINE__ );         \
    OCLRT_DEBUG_BREAK;                                             \
}
#else
#define OCL_ASSERT     assert
#endif // _DEBUG
#endif // OCL_ASSERT

/*****************************************************************************\
MACRO: OCL_HEAP_CHECK
\*****************************************************************************/
#ifndef OCL_HEAP_CHECK
#if 0
#define OCL_HEAP_CHECK( ptr )                                                   \
    if( ( ptr ) && ( OCLRT::g_OCLRTDebugVariables.EnableHeapCheck ) )           \
    {                                                                           \
        OCLRT_DPF( OCLDBG_CRITICAL, "POINTER CHECK:0x%x\t", ptr );              \
        switch( _heapchk() )                                                    \
        {                                                                       \
            case _HEAPOK:                                                       \
            OCLRT_DPF( OCLDBG_CRITICAL, ">> NO ERROR - HEAP IS OK:\n" );        \
            break;                                                              \
            case _HEAPEMPTY:                                                    \
            OCLRT_DPF( OCLDBG_CRITICAL, ">> NO ERROR - HEAP IS EMPRY:\n" );     \
            break;                                                              \
            case _HEAPBADBEGIN:                                                 \
            OCLRT_DPF( OCLDBG_CRITICAL, ">> ERROR - BAD START OF HEAP:\n" );    \
            break;                                                              \
            case _HEAPBADNODE:                                                  \
            OCLRT_DPF( OCLDBG_CRITICAL, ">> ERROR - BAD NODE IN HEAP:\n" );     \
            break;                                                              \
        }                                                                       \
        OCLRT_DPF( OCLDBG_CRITICAL, "  File:%s\t", __FILE__ );                  \
        OCLRT_DPF( OCLDBG_CRITICAL, "  Function:%s\t", __FUNCTION__ );          \
        OCLRT_DPF( OCLDBG_CRITICAL, "  Line:%d\n", __LINE__ );                  \
    }
#endif // _DEBUG
#endif // OCL_HEAP_CHECK

/*****************************************************************************\
MACRO: OCLRT_INCREMENT_COUNTER
\*****************************************************************************/
#ifndef OCLRT_INCREMENT_COUNTER
#define OCLRT_INCREMENT_COUNTER( x )                            \
{                                                               \
    if( OCLRT::g_OCLRTDebugVariables.EnableCounters == TRUE )   \
{                                                               \
    x++;                                                        \
}                                                               \
}
#endif

/*****************************************************************************\
MACRO: OCLRT_RETURN
\*****************************************************************************/
#ifndef OCLRT_RETURN
#define OCLRT_RETURN( x )           \
{                                   \
    OCLRT_DPF_FUNCTION_EXIT( x );   \
    return x;                       \
}
#endif

//OS specific
BOOL OSInitializeCriticalSectionAndSpinCount( OCLRT::OS_CRITICAL_SECTION *pOSCriticalSection, DWORD Value );
void OSInitializeCriticalSection( OCLRT::OS_CRITICAL_SECTION *pOSCriticalSection );
void OSEnterCriticalSection( OCLRT::OS_CRITICAL_SECTION *pOSCriticalSection );
void OSLeaveCriticalSection( OCLRT::OS_CRITICAL_SECTION *pOSCriticalSection );
void OSDeleteCriticalSection( OCLRT::OS_CRITICAL_SECTION *pOSCriticalSection );
void OSDebugBreak( void );
void OSSleep( DWORD Value );

template <typename T>
inline T OSInterlockedIncrement( T volatile *addend, T value = 1 )
{
    return __sync_add_and_fetch( addend, value );
}

#define OSInterlockedIncrement64  OSInterlockedIncrement

template <typename T>
inline T OSInterlockedDecrement( T volatile *addend, T value = 1 )
{
    return __sync_sub_and_fetch( addend, value );
}

#define OSInterlockedDecrement64 OSInterlockedDecrement

template <typename T>
inline T OSInterlockedCompareExchange( T volatile *dest, T exchange, T compared )
{
    return __sync_val_compare_and_swap( dest, compared, exchange );
}

DWORD OSWaitForSingleObject( HANDLE Handle, DWORD Time );

uintptr_t OSBeginThread( void * security, uint32_t stack_size, unsigned ( API * start_address )( void * ), void * arglist,  uint32_t initflag, uint32_t * thrdaddr );
void OSEndThread( uint32_t ThreadExitCode );

HANDLE OSCreateEvent( void * pAttributes, BOOL ManualReset, BOOL InitialState, const void *pName );
HANDLE OSCreateEventEx( void *pAttributes, const void *pName, DWORD flags, DWORD access );

BOOL OSSetEvent( HANDLE Handle );
BOOL OSCloseHandle( HANDLE Handle );

DWORD OSGetEnvironmentVariable(const char* lpName, char* lpBuffer, DWORD nSize);
DWORD OSGetModuleFileName(HMODULE hModule, char* lpFilename, DWORD nSize);

// Time/counters related routines , 
#ifdef TC_TESTER
BOOL QueryPerformanceFrequency( LARGE_INTEGER *lpFrequency );
BOOL QueryPerformanceCounter( LARGE_INTEGER *lpPerformanceCount );
#else // TC_Tester does not need GL related stuff
//OpenGL functions
PROC OGLGetProcAddress( const char *pString );
uintptr_t OGLGetCurrentContext( void );
int32_t OGLMakeCurrent( uintptr_t Handle, uintptr_t Context );
int32_t OGLDeleteContext( uintptr_t Context );
uintptr_t OGLCreateContext( uintptr_t Handle );
uintptr_t OGLCreateBackupContext( uintptr_t Handle ,uintptr_t Context );
uintptr_t OGLGetCurrentDC( void );
#endif //TC_TESTER

//Library Load functions and definitions
static const char name_libfcl[] = "libigdfcl.so";
static const char name_libbcl[] = "libiCBE.so";

OS_HMODULE OSLoadLibrary( const char *name );
BOOL OSFreeLibrary( OS_HMODULE lib );
FARPROC OSGetProcAddress(OS_HMODULE lib, const char *fnName );
// C library functions...

PVOID OSSecureZeroMemory( PVOID pAddress, size_t SizeInBytes );

errno_t strcpy_s( char *, size_t, const char * );
errno_t strncpy_s( char *, size_t, const char *, size_t );
size_t strnlen_s( const char *, size_t );

#define strtok_s strtok_r

errno_t strcat_s( char* strDestination, size_t numberOfElements, const char* strSource );
errno_t fopen_s( FILE** pFile, const char* filename, const char *mode );

errno_t memcpy_s( void *, size_t, const void *, size_t );
errno_t memmove_s( void *, size_t, const void *, size_t );
int sprintf_s( char *buffer, unsigned int szBuffer, const char *format, ...);
int _vscprintf( const char *format, va_list va );

#define vprintf_s vprintf
#define vfprintf_s vfprintf
#define vsprintf_s vsnprintf
#define aligned_free free

#define printf_s printf
#define fprintf_s fprintf

void aligned_allocate( void *ptr, size_t size_in_bytes, size_t alignment );
void aligned_free( void *ptr );

//OpenGL API names
#define AcquireSharedBufferINTEL                "ioclAcquireSharedBufferINTEL"
#define ReleaseSharedBufferINTEL                "ioclReleaseSharedBufferINTEL" 
#define AcquireSharedTextureINTEL               "ioclAcquireSharedTextureINTEL"
#define ReleaseSharedTextureINTEL               "ioclReleaseSharedTextureINTEL" 
#define AcquireSharedRenderBufferINTEL          "ioclAcquireSharedRenderBufferINTEL" 
#define ReleaseSharedRenderBufferINTEL          "ioclReleaseSharedRenderBufferINTEL"
#define SetSharedOCLContextStateINTEL           "ioclSetSharedOCLContextStateINTEL" 
                                                       
#ifdef __cplusplus
extern "C" {
#endif

CLAPI_EXPORT CL_API_ENTRY void * CL_API_CALL
clGetExtensionFunctionAddress(const char * func_name );

CLAPI_EXPORT CL_API_ENTRY cl_int CL_API_CALL
clIcdGetPlatformIDsKHR( cl_uint          num_entries,
                        cl_platform_id   *platforms,
                        cl_uint          *num_platforms );
#ifdef __cplusplus
}
#endif
#endif