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
  void reserve_wait() { m_wait_ctx.reserve(); }
  void release_wait() { m_wait_ctx.release(); }
  unsigned ref_count() const { return m_wait_ctx.reference_count(); }
};

#endif // #ifndef ENHANCEMENT_TASK_GROUP_WITH_REFERENCE_H
