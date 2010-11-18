/*
    Copyright 2005-2010 Intel Corporation.  All Rights Reserved.

    The source code contained or described herein and all documents related
    to the source code ("Material") are owned by Intel Corporation or its
    suppliers or licensors.  Title to the Material remains with Intel
    Corporation or its suppliers and licensors.  The Material is protected
    by worldwide copyright laws and treaty provisions.  No part of the
    Material may be used, copied, reproduced, modified, published, uploaded,
    posted, transmitted, distributed, or disclosed in any way without
    Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other
    intellectual property right is granted to or conferred upon you by
    disclosure or delivery of the Materials, either expressly, by
    implication, inducement, estoppel or otherwise.  Any license under such
    intellectual property rights must be express and approved by Intel in
    writing.
*/

#ifndef __TBB_machine_H
#error Do not include this file directly; include tbb_machine.h instead
#endif

#define NONET
#define NOD3D
#include "xtl.h"    
#include "ppcintrinsics.h"

#if _MSC_VER >= 1300
extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)
#define __TBB_release_consistency_helper() _ReadWriteBarrier()
#endif

inline void __TBB_rel_acq_fence() { __lwsync(); }

#define __TBB_WORDSIZE 4
#define __TBB_BIG_ENDIAN 1

//todo: define __TBB_DECL_FENCED_ATOMICS and define acquire/release primitives to maximize performance

typedef __int64 int64_t;  //required for definition of Store8/Load8 in atomic.h
typedef unsigned char uint8_t;  //same reason

inline __int32 __TBB_machine_cmpswp4(volatile void *ptr, __int32 value, __int32 comparand )
{                               
 __lwsync();
 __int32 result = InterlockedCompareExchange((volatile LONG*)ptr, value, comparand);
 __lwsync();
 return result;
}

inline __int64 __TBB_machine_cmpswp8(volatile void *ptr, __int64 value, __int64 comparand )
{
 __lwsync();
 __int64 result = InterlockedCompareExchange64((volatile LONG64*)ptr, value, comparand);
 __lwsync();
 return result;
}

#pragma optimize( "", off )
inline void __TBB_machine_pause (__int32 delay ) 
{
 for (__int32 i=0; i<delay; i++) {;};
}
#pragma optimize( "", on ) 


#define __TBB_CompareAndSwap4(P,V,C) __TBB_machine_cmpswp4(P,V,C)
#define __TBB_CompareAndSwap8(P,V,C) __TBB_machine_cmpswp8(P,V,C)
#define __TBB_CompareAndSwapW(P,V,C) __TBB_machine_cmpswp4(P,V,C)
#define __TBB_Yield()  Sleep(0)
#define __TBB_Pause(V) __TBB_machine_pause(V)
#define __TBB_fence_for_acquire() __lwsync()
#define __TBB_fence_for_release() __lwsync()

