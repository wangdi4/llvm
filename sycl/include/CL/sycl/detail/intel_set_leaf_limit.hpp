//== intel_set_leaf_limit.hpp - Set limit of leafs for dependency tracking -==//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <CL/sycl/detail/common.hpp>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace detail {
__SYCL_EXPORT void setLeafLimit(size_t Limit);
} // namespace intel
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
