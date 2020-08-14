//==- intel_set_leaf_limit.cpp - Set limit of leafs for dependency tracking ==//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include <CL/sycl/detail/intel_set_leaf_limit.hpp>
#include <detail/scheduler/scheduler.hpp>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace detail {
void setLeafLimit(size_t Limit) {
  cl::sycl::detail::Scheduler::getInstance().setLeafLimit(Limit);
}
} // namespace intel
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
