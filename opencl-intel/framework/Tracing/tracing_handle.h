// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#pragma once

#include "tracing_types.h"

#include <bitset>
#include <stdint.h>
#include <assert.h>

namespace HostSideTracing {

struct TracingHandle {
  public:
    TracingHandle(cl_tracing_callback callback, void *userData) :
        callback(callback), userData(userData) {}

    void call(cl_function_id fid, cl_callback_data *callbackData) {
        callback(fid, callbackData, userData);
    }

    void setTracingPoint(cl_function_id fid, bool enable) {
        assert(static_cast<uint32_t>(fid) < CL_FUNCTION_COUNT);
        mask[static_cast<uint32_t>(fid)] = enable;
    }

    bool getTracingPoint(cl_function_id fid) const {
        assert(static_cast<uint32_t>(fid) < CL_FUNCTION_COUNT);
        return mask[static_cast<uint32_t>(fid)];
    }

  private:
    cl_tracing_callback callback;
    void *userData;
    std::bitset<CL_FUNCTION_COUNT> mask;
};

} // namespace HostSideTracing

struct _cl_tracing_handle {
    cl_device_id device;
    HostSideTracing::TracingHandle *handle;
};
