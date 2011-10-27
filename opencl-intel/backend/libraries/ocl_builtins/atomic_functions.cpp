// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  atomic_functions.cpp
///////////////////////////////////////////////////////////


#ifdef __cplusplus
extern "C" {
#endif

#include "cl_atomic_declaration.h"

#include <intrin.h>

typedef int intrin_type;

/**
 * Implementation NOTICE:
 *
 * This implementation assumes that local pool (workgroup threads) are executed by
 * only ONE phyiscal thread, this means that the operations in local memory will be 
 * invoked in a serial way, thus there's no need to synchronize the access to the 
 * local memory;
 * 
 * In the following implementation there's a helper functions there are two versions
 * of this:
 *  one for global memory (with _global postfix)
 *  one for local  memory (with _local  postfix)
 */

// global calls implementations
int __inline__ __attribute__((always_inline)) atomicInc_global(volatile int* p)
{
	return __sync_fetch_and_add(p, 1);
}

int __inline__ __attribute__((always_inline)) atomicDec_global(volatile int* p)
{
	return __sync_fetch_and_sub(p, 1);
}

int __inline__ __attribute__((always_inline)) atomicAdd_global(volatile int* p, int val)
{
	return __sync_fetch_and_add(p, val);
}

int __inline__ __attribute__((always_inline)) atomicSub_global(volatile int* p, int val)
{
	return __sync_fetch_and_sub(p, val);
}

__inline__ __attribute__((always_inline)) int _InterlockedCompareExchange_global(volatile int * const Destination, const int Comperand, const int Exchange)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

__inline__ __attribute__((always_inline)) int InterlockedExchange_global(volatile int * const Target, const int Value)
{
	_mm_mfence();
	return __sync_lock_test_and_set(Target, Value);
}

__inline__ __attribute__((always_inline)) int _InterlockedAnd_global(volatile int * const value, const int mask)
{
	return __sync_fetch_and_and(value, mask);
}

__inline__ __attribute__((always_inline)) int _InterlockedOr_global(volatile int * const value, const int mask)
{
	return __sync_fetch_and_or(value, mask);
}

__inline__ __attribute__((always_inline)) int _InterlockedXor_global(volatile int * const value, const int mask)
{
	return __sync_fetch_and_xor(value, mask);
}

// local calls implementations
int __inline__ __attribute__((always_inline)) atomicInc_local(volatile int* p)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *p;
    *p = oldValue + 1;
    return oldValue;
}

int __inline__ __attribute__((always_inline)) atomicDec_local(volatile int* p)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
	int oldValue = *p;
    *p = oldValue - 1;
    return oldValue;
}

int __inline__ __attribute__((always_inline)) atomicAdd_local(volatile int* p, int val)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
	int oldValue = *p;
    *p = oldValue + val;
    return oldValue;
}

int __inline__ __attribute__((always_inline)) atomicSub_local(volatile int* p, int val)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
	int oldValue = *p;
    *p = oldValue - val;
    return oldValue;
}

__inline__ __attribute__((always_inline)) int _InterlockedCompareExchange_local(volatile int * const Destination, const int Comperand, const int Exchange)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *Destination;
    if (oldValue == Comperand)
    {
        *Destination = Exchange;
    }
	return oldValue;
}

__inline__ __attribute__((always_inline)) int InterlockedExchange_local(volatile int * const Target, const int Value)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
	int oldValue = *Target;
    *Target = Value;
	return oldValue;
}

__inline__ __attribute__((always_inline)) int _InterlockedAnd_local(volatile int * const value, const int mask)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *value;
    *value = oldValue & mask;
    return oldValue;
}

__inline__ __attribute__((always_inline)) int _InterlockedOr_local(volatile int * const value, const int mask)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
	int oldValue = *value;
    *value = oldValue | mask;
    return oldValue;
}

__inline__ __attribute__((always_inline)) int _InterlockedXor_local(volatile int * const value, const int mask)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
	int oldValue = *value;
    *value = oldValue ^ mask;
    return oldValue;
}

// helper min, max implementations (not syncronized)
int __inline__ __attribute__((always_inline)) unsigned_min(unsigned int p0, unsigned int p1)
{
    return (p0 < p1) ? p0 : p1;
}

int __inline__ __attribute__((always_inline)) unsigned_max(unsigned int p0, unsigned int p1)
{
    return (p0 > p1) ? p0 : p1;
}

int __inline__ __attribute__((always_inline)) signed_min(int p0, int p1)
{
    return (p0 < p1) ? p0 : p1;
}

int __inline__ __attribute__((always_inline)) signed_max(int p0, int p1)
{
    return (p0 > p1) ? p0 : p1;
}

//functions signed
int  __attribute__((overloadable)) atom_inc(__global int *p)
{
	return atomicInc_global((int*)p);
}

int  __attribute__((overloadable)) atom_dec(__global int *p)
{
	return atomicDec_global((int*)p);
}

int  __attribute__((overloadable)) atom_add(__global int *p, int val)
{
	return atomicAdd_global((int*)p, val);
}

int  __attribute__((overloadable)) atom_sub(__global int *p, int val)
{
	return atomicSub_global((int*)p, val);
}

int  __attribute__((overloadable)) atom_xchg(__global int *p, int val)
{
	return ( InterlockedExchange_global((intrin_type*)p, val) );
}

int  __attribute__((overloadable)) atom_cmpxchg(__global int *p, int cmp, int val)
{
	return ( _InterlockedCompareExchange_global((intrin_type *)p, cmp, val) );
}

int  __attribute__((overloadable)) atom_max(__global int *p, int val)
{
	while (true)
	{
		int oldValue = *p;
		if (oldValue >= val) return oldValue;
		int retVal = _InterlockedCompareExchange_global((intrin_type *)p, oldValue, val);
		if (retVal == oldValue) return oldValue;
	}
}

int  __attribute__((overloadable)) atom_min(__global int *p, int val)
{
	while (true)
	{
		int oldValue = *p;
		if (oldValue <= val) return oldValue;
		int retVal = _InterlockedCompareExchange_global((intrin_type *)p, oldValue, val);
		if (retVal == oldValue) return oldValue;
	}
}

int  __attribute__((overloadable)) atom_and(__global int *p, int val)
{
	return ( _InterlockedAnd_global((intrin_type*)p, val) );
}

int  __attribute__((overloadable)) atom_or(__global int *p, int val)
{
	return ( _InterlockedOr_global((intrin_type*)p, val) );
}

int  __attribute__((overloadable)) atom_xor(__global int *p, int val)
{
	return ( _InterlockedXor_global((intrin_type*)p, val) );
}

//functions unsigned

unsigned int  __attribute__((overloadable)) atom_add(__global unsigned int *p, unsigned int val)
{
	return atomicAdd_global((int*)p, val);
}

unsigned int  __attribute__((overloadable)) atom_sub(__global unsigned int *p, unsigned int val)

{
	return atomicSub_global((int*)p, val);
}

unsigned int  __attribute__((overloadable)) atom_xchg(__global unsigned int *p, unsigned int val)
{
	return ( InterlockedExchange_global((intrin_type*)p, val) );
}

unsigned int  __attribute__((overloadable)) atom_inc(__global unsigned int *p)
{
	return atomicInc_global((int*)p);
}

unsigned int  __attribute__((overloadable)) atom_dec(__global unsigned int *p)
{
	return atomicDec_global((int*)p);
}

unsigned int  __attribute__((overloadable)) atom_cmpxchg(__global unsigned int *p, unsigned int cmp,unsigned int val)
{
	return ( _InterlockedCompareExchange_global((int *)p, cmp, val) );
}

unsigned int  __attribute__((overloadable)) atom_max(__global unsigned int *p, unsigned int val)
{
	while (true)
	{
		unsigned int oldValue = *p;
		if (oldValue >= val) return oldValue;
		unsigned int retVal = _InterlockedCompareExchange_global((intrin_type *)p, oldValue, val);
		if (retVal == oldValue) return oldValue;
	}
}
unsigned int  __attribute__((overloadable)) atom_min(__global unsigned int *p, unsigned int val)
{
	while (true)
	{
		unsigned int oldValue = *p;
		if (oldValue <= val) return oldValue;
		unsigned int retVal = _InterlockedCompareExchange_global((intrin_type *)p, oldValue, val);
		if (retVal == oldValue) return oldValue;
	}
}

unsigned int  __attribute__((overloadable)) atom_and(__global unsigned int *p, unsigned int val)
{
	return ( _InterlockedAnd_global((intrin_type*)p, val) );
}

unsigned int  __attribute__((overloadable)) atom_or(__global unsigned int *p, unsigned int val)
{
	return ( _InterlockedOr_global((intrin_type*)p, val) );
}

unsigned int  __attribute__((overloadable)) atom_xor(__global unsigned int *p, unsigned int val)
{
	return ( _InterlockedXor_global((intrin_type*)p, val) );
}

// local signed
int  __attribute__((overloadable)) atom_add(__local int *p, int val)
{
	return atomicAdd_local((int*)p, val);
}

int  __attribute__((overloadable)) atom_sub(__local int *p, int val)
{
	return atomicSub_local((int*)p, val);
}

int  __attribute__((overloadable)) atom_xchg(__local int *p, int val)
{
	return InterlockedExchange_local((intrin_type*)p, val);
}

int  __attribute__((overloadable)) atom_min(__local int *p, int val)
{
    int oldValue = *p;
    *p = signed_min(val, oldValue);
    return oldValue;
}

int  __attribute__((overloadable)) atom_max(__local int *p, int val)
{
    int oldValue = *p;
    *p = signed_max(val, oldValue);
    return oldValue;
}

int  __attribute__((overloadable)) atom_inc(__local int *p)
{
	return atomicInc_local((int*)p);
}

int  __attribute__((overloadable)) atom_dec(__local int *p)
{
	return atomicDec_local((int*)p);
}

int  __attribute__((overloadable)) atom_cmpxchg(__local int *p, int cmp, int val)
{
	return _InterlockedCompareExchange_local((intrin_type*)p, cmp, val);
}

int  __attribute__((overloadable)) atom_and(__local int *p, int val)
{
	return _InterlockedAnd_local((intrin_type*)p, val);
}

int  __attribute__((overloadable)) atom_or(__local int *p, int val)
{
	return _InterlockedOr_local((intrin_type*)p, val);
}

int  __attribute__((overloadable)) atom_xor(__local int *p, int val)
{
	return _InterlockedXor_local((intrin_type*)p, val);
}

// local unsigned
unsigned int  __attribute__((overloadable)) atom_add(__local unsigned int *p, unsigned int val)
{
	return atomicAdd_local((int*)p, val);
}

unsigned int  __attribute__((overloadable)) atom_sub(__local unsigned int *p, unsigned int val)
{
	return atomicSub_local((int*)p, val);
}

unsigned  __attribute__((overloadable)) atom_xchg(__local unsigned int *p, unsigned int val)
{
	return InterlockedExchange_local((intrin_type*)p, val);
}

unsigned int  __attribute__((overloadable)) atom_min(__local unsigned int *p, unsigned int val)
{
    unsigned int oldValue = *p;
    *p = unsigned_min(val, oldValue);
    return oldValue;
}

unsigned int  __attribute__((overloadable)) atom_max(__local unsigned int *p, unsigned int val)
{
    unsigned int oldValue = *p;
    *p = unsigned_max(val, oldValue);
    return oldValue;
}

unsigned int  __attribute__((overloadable)) atom_inc(__local unsigned int *p)
{
	return atomicInc_local((int*)p);
}

unsigned int  __attribute__((overloadable)) atom_dec(__local unsigned int *p)
{
	return atomicDec_local((int*)p);
}

unsigned int  __attribute__((overloadable)) atom_cmpxchg(__local unsigned int *p, unsigned int cmp, unsigned int val)
{
	return _InterlockedCompareExchange_local((intrin_type*)p, cmp, val);
}

unsigned int  __attribute__((overloadable)) atom_and(__local unsigned int *p, unsigned int val)
{
	return _InterlockedAnd_local((intrin_type*)p, val);
}

unsigned int  __attribute__((overloadable)) atom_or(__local unsigned int *p, unsigned val)
{
	return _InterlockedOr_local((intrin_type*)p, val);
}

unsigned int  __attribute__((overloadable)) atom_xor(__local unsigned int *p, unsigned val)
{
	return _InterlockedXor_local((intrin_type*)p, val);
}

//atomics (volotile)  signed
int  __attribute__((overloadable)) atomic_inc(__global volatile int *p)
{
	return atomicInc_global((int*)p);
}

int  __attribute__((overloadable)) atomic_dec(__global volatile int *p)
{
	return atomicDec_global((int*)p);
}

int  __attribute__((overloadable)) atomic_add(__global volatile int *p, int val)
{
	return atomicAdd_global((int*)p, val);
}

int  __attribute__((overloadable)) atomic_sub(__global volatile int *p, int val)
{
	return atomicSub_global((int*)p, val);
}

int  __attribute__((overloadable)) atomic_xchg(__global volatile int *p, int val)
{
	return ( InterlockedExchange_global((intrin_type*)p, val) );
}

float  __attribute__((overloadable)) atomic_xchg(__global volatile float *p, float val)
{
	int ret = ( InterlockedExchange_global((intrin_type*)p, *((int *)(&val))) );
	return as_float(ret);
}

int  __attribute__((overloadable)) atomic_cmpxchg(__global volatile int *p, int cmp, int val)
{
	return ( _InterlockedCompareExchange_global((intrin_type *)p, cmp, val) );
}

int  __attribute__((overloadable)) atomic_max(__global volatile int *p, int val)
{
	while (true)
	{
		int oldValue = *p;
		if (oldValue >= val) return oldValue;
		int retVal = _InterlockedCompareExchange_global((intrin_type *)p, oldValue, val);
		if (retVal == oldValue) return oldValue;
	}
}

int  __attribute__((overloadable)) atomic_min(__global volatile int *p, int val)
{
	while (true)
	{
		int oldValue = *p;
		if (oldValue <= val) return oldValue;
		int retVal = _InterlockedCompareExchange_global((intrin_type *)p, oldValue, val);
		if (retVal == oldValue) return oldValue;
	}
}

int  __attribute__((overloadable)) atomic_and(__global volatile int *p, int val)
{
	return ( _InterlockedAnd_global((intrin_type*)p, val) );
}

int  __attribute__((overloadable)) atomic_or(__global volatile int *p, int val)
{
	return ( _InterlockedOr_global((intrin_type*)p, val) );
}

int  __attribute__((overloadable)) atomic_xor(__global volatile int *p, int val)
{
	return ( _InterlockedXor_global((intrin_type*)p, val) );
}

//functions unsigned

unsigned int  __attribute__((overloadable)) atomic_add(__global volatile unsigned int *p, unsigned int val)
{
	return atomicAdd_global((int*)p, val);
}

unsigned int  __attribute__((overloadable)) atomic_sub(__global volatile unsigned int *p, unsigned int val)

{
	return atomicSub_global((int*)p, val);
}

unsigned int  __attribute__((overloadable)) atomic_xchg(__global volatile unsigned int *p, unsigned int val)
{
	return ( InterlockedExchange_global((intrin_type*)p, val) );
}

unsigned int  __attribute__((overloadable)) atomic_inc(__global volatile unsigned int *p)
{
	return atomicInc_global((int*)p);
}

unsigned int  __attribute__((overloadable)) atomic_dec(__global volatile unsigned int *p)
{
	return atomicDec_global((int*)p);
}

unsigned int  __attribute__((overloadable)) atomic_cmpxchg(__global volatile unsigned int *p, unsigned int cmp,unsigned int val)
{
	return ( _InterlockedCompareExchange_global((int *)p, cmp, val) );
}

unsigned int  __attribute__((overloadable)) atomic_max(__global volatile unsigned int *p, unsigned int val)
{
	while (true)
	{
		unsigned int oldValue = *p;
		if (oldValue >= val) return oldValue;
		unsigned int retVal = _InterlockedCompareExchange_global((intrin_type *)p, oldValue, val);
		if (retVal == oldValue) return oldValue;
	}
}
unsigned int  __attribute__((overloadable)) atomic_min(__global volatile unsigned int *p, unsigned int val)
{
	while (true)
	{
		unsigned int oldValue = *p;
		if (oldValue <= val) return oldValue;
		unsigned int retVal = _InterlockedCompareExchange_global((intrin_type *)p, oldValue, val);
		if (retVal == oldValue) return oldValue;
	}
}

unsigned int  __attribute__((overloadable)) atomic_and(__global volatile unsigned int *p, unsigned int val)
{
	return ( _InterlockedAnd_global((intrin_type*)p, val) );
}

unsigned int  __attribute__((overloadable)) atomic_or(__global volatile unsigned int *p, unsigned int val)
{
	return ( _InterlockedOr_global((intrin_type*)p, val) );
}

unsigned int  __attribute__((overloadable)) atomic_xor(__global volatile unsigned int *p, unsigned int val)
{
	return ( _InterlockedXor_global((intrin_type*)p, val) );
}

// local signed
int  __attribute__((overloadable)) atomic_add(__local volatile int *p, int val)
{
	return atomicAdd_local((int*)p, val);
}

int  __attribute__((overloadable)) atomic_sub(__local volatile int *p, int val)
{
	return atomicSub_local((int*)p, val);
}

int  __attribute__((overloadable)) atomic_xchg(__local volatile int *p, int val)
{
	return InterlockedExchange_local((intrin_type*)p, val);
}

float  __attribute__((overloadable)) atomic_xchg(__local volatile float *p, float val)
{
	int ret = ( InterlockedExchange_local((intrin_type*)p, *((int *)(&val))) );
	return as_float(ret);
}

int  __attribute__((overloadable)) atomic_min(__local volatile int *p, int val)
{
    int oldValue = *p;
    *p = signed_min(val, oldValue);
    return oldValue;
}

int  __attribute__((overloadable)) atomic_max(__local volatile int *p, int val)
{
    int oldValue = *p;
    *p = signed_max(val, oldValue);
    return oldValue;
}

int  __attribute__((overloadable)) atomic_inc(__local volatile int *p)
{
	return atomicInc_local((int*)p);
}

int  __attribute__((overloadable)) atomic_dec(__local volatile int *p)
{
	return atomicDec_local((int*)p);
}

int  __attribute__((overloadable)) atomic_cmpxchg(__local volatile int *p, int cmp, int val)
{
	return _InterlockedCompareExchange_local((intrin_type*)p, cmp, val);
}

int  __attribute__((overloadable)) atomic_and(__local volatile int *p, int val)
{
	return _InterlockedAnd_local((intrin_type*)p, val);
}

int  __attribute__((overloadable)) atomic_or(__local volatile int *p, int val)
{
	return _InterlockedOr_local((intrin_type*)p, val);
}

int  __attribute__((overloadable)) atomic_xor(__local volatile int *p, int val)
{
	return _InterlockedXor_local((intrin_type*)p, val);
}

// local unsigned
unsigned int  __attribute__((overloadable)) atomic_add(__local volatile unsigned int *p, unsigned int val)
{
	return atomicAdd_local((int*)p, val);
}

unsigned int  __attribute__((overloadable)) atomic_sub(__local volatile unsigned int *p, unsigned int val)
{
	return atomicSub_local((int*)p, val);
}

unsigned  __attribute__((overloadable)) atomic_xchg(__local volatile unsigned int *p, unsigned int val)
{
	return InterlockedExchange_local((intrin_type*)p, val);
}

unsigned int  __attribute__((overloadable)) atomic_min(__local volatile unsigned int *p, unsigned int val)
{
    unsigned int oldValue = *p;
    *p = unsigned_min(val, oldValue);
    return oldValue;
}

unsigned int  __attribute__((overloadable)) atomic_max(__local volatile unsigned int *p, unsigned int val)
{
    unsigned int oldValue = *p;
    *p = unsigned_max(val, oldValue);
    return oldValue;
}

unsigned int  __attribute__((overloadable)) atomic_inc(__local volatile unsigned int *p)
{
	return atomicInc_local((int*)p);
}

unsigned int  __attribute__((overloadable)) atomic_dec(__local volatile unsigned int *p)
{
	return atomicDec_local((int*)p);
}

unsigned int  __attribute__((overloadable)) atomic_cmpxchg(__local volatile unsigned int *p, unsigned int cmp, unsigned int val)
{
	return _InterlockedCompareExchange_local((intrin_type*)p, cmp, val);
}

unsigned int  __attribute__((overloadable)) atomic_and(__local volatile unsigned int *p, unsigned int val)
{
	return _InterlockedAnd_local((intrin_type*)p, val);
}

unsigned int  __attribute__((overloadable)) atomic_or(__local volatile unsigned int *p, unsigned val)
{
	return _InterlockedOr_local((intrin_type*)p, val);
}

unsigned int  __attribute__((overloadable)) atomic_xor(__local volatile unsigned int *p, unsigned val)
{
	return _InterlockedXor_local((intrin_type*)p, val);
}
#ifdef __cplusplus
}
#endif
