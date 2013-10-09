/*
    Copyright 2005-2013 Intel Corporation.  All Rights Reserved.

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

#if !defined(__TBB_machine_H) || defined(__TBB_machine_gcc_itsx_H)
#error Do not #include this internal file directly; use public TBB headers instead.
#endif

#define __TBB_machine_gcc_itsx_H

#define __TBB_OP_XACQUIRE 0xF2
#define __TBB_OP_XRELEASE 0xF3
#define __TBB_OP_LOCK     0xF0

#define __TBB_STRINGIZE_INTERNAL(arg) #arg
#define __TBB_STRINGIZE(arg) __TBB_STRINGIZE_INTERNAL(arg)

#ifdef __TBB_x86_64
#define __TBB_r_out "=r"
#else
#define __TBB_r_out "=q"
#endif

inline static uint8_t __TBB_machine_try_lock_elided( volatile uint8_t* lk )
{
    uint8_t value = 1;
    __asm__ volatile (".byte " __TBB_STRINGIZE(__TBB_OP_XACQUIRE)"; lock; xchgb %0, %1;"
                      : __TBB_r_out(value), "=m"(*lk)  : "0"(value), "m"(*lk) : "memory" );
    return uint8_t(value^1);
}

inline static void __TBB_machine_try_lock_elided_cancel()
{
    // 'pause' instruction aborts HLE/RTM transactions
    __asm__ volatile ("pause\n" : : : "memory" );
}

inline static void __TBB_machine_unlock_elided( volatile uint8_t* lk )
{
    __asm__ volatile (".byte " __TBB_STRINGIZE(__TBB_OP_XRELEASE)"; movb $0, %0" 
                      : "=m"(*lk) : "m"(*lk) : "memory" );
}
