// INTEL CONFIDENTIAL
//
// Copyright 2006-2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef ENHANCEMENT_TASK_GROUP_WITH_REFERENCE_H
#define ENHANCEMENT_TASK_GROUP_WITH_REFERENCE_H

#include <tbb/task_group.h>

/// Extend tbb::task_group with reserve/release_wait functions
class task_group_with_reference : public tbb::task_group {
public:
  virtual ~task_group_with_reference() noexcept(true) {
    // Method wait must be called before destroying a task_group, otherwise the
    // destructor throws an exception.
    wait();
  }
  void reserve_wait() { m_wait_ctx.reserve(); }
  void release_wait() { m_wait_ctx.release(); }
  unsigned ref_count() const {
    // From: https://github.com/oneapi-src/oneTBB
    // Commit: 112076d06c1fdab3df8612ee091fb639194fc6c4
    // Path: include/oneapi/tbb/detail/_task.h
    //
    // class wait_context {
    //     static constexpr std::uint64_t overflow_mask = ~((1LLU << 32) - 1);
    //     std::uint64_t m_version_and_traits{1};
    //     std::atomic<std::uint64_t> m_ref_count{};
    //     ... ...
    // };
    //
    // We have to do a nasty hack here because there is no public method to get
    // reference count from m_wait_ctx any more.
    // Fortunately, the ABI compatibility of wait_context is preserved so that
    // we can access the reference count from the 2nd filed of m_wait_ctx via
    // memory cast.
    static_assert(sizeof(std::uint64_t) == sizeof(std::atomic<std::uint64_t>),
                  "the sizes are expected to be equal");
    return unsigned(((const std::atomic<std::uint64_t> *)&m_wait_ctx)[1].load(
        std::memory_order_acquire));
  }
};

#endif // #ifndef ENHANCEMENT_TASK_GROUP_WITH_REFERENCE_H
