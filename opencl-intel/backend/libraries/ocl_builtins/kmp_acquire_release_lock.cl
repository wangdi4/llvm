// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#define KMP_LOCK_FREE 0
#define KMP_LOCK_BUSY 1

void __builtin_IB_kmp_acquire_lock(__global int *lock) {
  volatile atomic_uint *lck = (volatile atomic_uint *)lock;
  uint expected = KMP_LOCK_FREE;
  while (atomic_load_explicit(lck, memory_order_relaxed) != KMP_LOCK_FREE ||
         !atomic_compare_exchange_strong_explicit(lck, &expected, KMP_LOCK_BUSY,
                                                  memory_order_acquire,
                                                  memory_order_relaxed)) {
    expected = KMP_LOCK_FREE;
  }
}

void __builtin_IB_kmp_release_lock(__global int *lock) {
  volatile atomic_uint *lck = (volatile atomic_uint *)lock;
  atomic_store_explicit(lck, KMP_LOCK_FREE, memory_order_release);
}

#undef KMP_LOCK_FREE
#undef KMP_LOCK_BUSY
