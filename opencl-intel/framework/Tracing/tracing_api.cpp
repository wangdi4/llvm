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

#include "tracing_api.h"
#include "tracing_handle.h"
#include "tracing_notify.h"

#include "cl_framework.h"
#if defined (_WIN32)
#else
#include "cl_framework_alias_linux.h"
#endif

#include <algorithm>

namespace HostSideTracing {

// [XYZZ..Z] - { X     - enabled/disabled bit,
//               Y     - locked/unlocked bit,
//               ZZ..Z - client count bits }
std::atomic<uint32_t> tracingState(0);
std::vector<TracingHandle *> tracingHandle;
std::atomic<uint32_t> tracingCorrelationId(0);

bool addTracingClient() {
    uint32_t state = tracingState.load(std::memory_order_acquire);
    state = TRACING_SET_ENABLED_BIT(state);
    state = TRACING_UNSET_LOCKED_BIT(state);
    AtomicBackoff backoff;

    // Concurrent increment of tracing client counter
    while (!tracingState.compare_exchange_weak(state, state + 1,
                std::memory_order_release, std::memory_order_acquire)) {
        if (!TRACING_GET_ENABLED_BIT(state)) {// Tracing was disabled
            return false;
        } else if (TRACING_GET_LOCKED_BIT(state)) {
            // Tracing handle list is changing
            assert(TRACING_GET_CLIENT_COUNTER(state) == 0);
            state = TRACING_UNSET_LOCKED_BIT(state);
            backoff.pause();
        } else { // Another client tries to increment the counter
            backoff.pause();
        }
    }

    return true;
}

void removeTracingClient() {
    assert(TRACING_GET_ENABLED_BIT(tracingState.load(
        std::memory_order_acquire)));
    assert(!TRACING_GET_LOCKED_BIT(tracingState.load(
        std::memory_order_acquire)));
    assert(TRACING_GET_CLIENT_COUNTER(tracingState.load(
        std::memory_order_acquire)) > 0);
    tracingState.fetch_sub(1, std::memory_order_acq_rel);
}

static void LockTracingState() {
    uint32_t state = tracingState.load(std::memory_order_acquire);
    state = TRACING_ZERO_CLIENT_COUNTER(state);
    state = TRACING_UNSET_LOCKED_BIT(state);
    AtomicBackoff backoff;
    while (!tracingState.compare_exchange_weak(state,
                TRACING_SET_LOCKED_BIT(state),
                std::memory_order_release,
                std::memory_order_acquire)) {
        state = TRACING_ZERO_CLIENT_COUNTER(state);
        state = TRACING_UNSET_LOCKED_BIT(state);
        backoff.pause();
    }
    assert(TRACING_GET_LOCKED_BIT(tracingState.load(
        std::memory_order_acquire)));
    assert(TRACING_GET_CLIENT_COUNTER(tracingState.load(
        std::memory_order_acquire)) == 0);
}

static void UnlockTracingState() {
    assert(TRACING_GET_LOCKED_BIT(tracingState.load(
        std::memory_order_acquire)));
    assert(TRACING_GET_CLIENT_COUNTER(tracingState.load(
        std::memory_order_acquire)) == 0);
    tracingState.fetch_and(~TRACING_STATE_LOCKED_BIT,
        std::memory_order_acq_rel);
}

} // namespace HostSideTracing

using namespace HostSideTracing;

cl_int CL_API_CALL clCreateTracingHandleINTEL(cl_device_id device,
                                              cl_tracing_callback callback,
                                              void *userData,
                                              cl_tracing_handle *handle) {
    if (device == nullptr || callback == nullptr || handle == nullptr) {
        return CL_INVALID_VALUE;
    }

    try {
        *handle = new _cl_tracing_handle;
        if (*handle == nullptr) {
            return CL_OUT_OF_HOST_MEMORY;
        }
    } catch (...) {
        return CL_OUT_OF_HOST_MEMORY;
    }

    try {
        (*handle)->device = device;
        (*handle)->handle = new TracingHandle(callback, userData);
        if ((*handle)->handle == nullptr) {
            delete *handle;
            return CL_OUT_OF_HOST_MEMORY;
        }
    } catch (...) {
        delete *handle;
        return CL_OUT_OF_HOST_MEMORY;
    }

    return CL_SUCCESS;
}
SET_ALIAS(clCreateTracingHandleINTEL);
REGISTER_EXTENSION_FUNCTION(clCreateTracingHandleINTEL,
    clCreateTracingHandleINTEL);

cl_int CL_API_CALL clSetTracingPointINTEL(cl_tracing_handle handle,
                                          cl_function_id fid,
                                          cl_bool enable) {
    if (handle == nullptr) {
        return CL_INVALID_VALUE;
    }

    assert(handle->handle != nullptr);
    if (static_cast<uint32_t>(fid) >= CL_FUNCTION_COUNT) {
        return CL_INVALID_VALUE;
    }

    handle->handle->setTracingPoint(fid, enable);

    return CL_SUCCESS;
}
SET_ALIAS(clSetTracingPointINTEL);
REGISTER_EXTENSION_FUNCTION(clSetTracingPointINTEL, clSetTracingPointINTEL);

cl_int CL_API_CALL clDestroyTracingHandleINTEL(cl_tracing_handle handle) {
    if (handle == nullptr) {
        return CL_INVALID_VALUE;
    }

    assert(handle->handle != nullptr);
    delete handle->handle;
    delete handle;

    return CL_SUCCESS;
}
SET_ALIAS(clDestroyTracingHandleINTEL);
REGISTER_EXTENSION_FUNCTION(clDestroyTracingHandleINTEL,
    clDestroyTracingHandleINTEL);

cl_int CL_API_CALL clEnableTracingINTEL(cl_tracing_handle handle) {
    if (handle == nullptr) {
        return CL_INVALID_VALUE;
    }

    LockTracingState();

    assert(handle->handle != nullptr);
    for (TracingHandle* enabledHandle : tracingHandle) {
        if (enabledHandle == handle->handle) { // Handle is already enabled
            UnlockTracingState();
            return CL_INVALID_VALUE;
        }
    }

    if (tracingHandle.size() == TRACING_MAX_HANDLE_COUNT) {
        UnlockTracingState();
        return CL_OUT_OF_RESOURCES;
    }

    tracingHandle.push_back(handle->handle);
    if (tracingHandle.size() == 1) {
        tracingState.fetch_or(TRACING_STATE_ENABLED_BIT,
            std::memory_order_acq_rel);
    }

    UnlockTracingState();
    return CL_SUCCESS;
}
SET_ALIAS(clEnableTracingINTEL);
REGISTER_EXTENSION_FUNCTION(clEnableTracingINTEL, clEnableTracingINTEL);

cl_int CL_API_CALL clDisableTracingINTEL(cl_tracing_handle handle) {
    if (handle == nullptr) {
        return CL_INVALID_VALUE;
    }

    LockTracingState();

    assert(handle->handle != nullptr);
    for (size_t i = 0; i < tracingHandle.size(); ++i) {
        if (tracingHandle[i] == handle->handle) {
            if (tracingHandle.size() == 1) {
                tracingState.fetch_and(~TRACING_STATE_ENABLED_BIT,
                    std::memory_order_acq_rel);
                tracingHandle.clear();
            } else {
                tracingHandle[i] = tracingHandle.back();
                tracingHandle.pop_back();
            }
            UnlockTracingState();
            return CL_SUCCESS;
        }
    }

    // User-provided handle is not in the list of currently enabled handles
    UnlockTracingState();
    return CL_INVALID_VALUE;
}
SET_ALIAS(clDisableTracingINTEL);
REGISTER_EXTENSION_FUNCTION(clDisableTracingINTEL, clDisableTracingINTEL);

cl_int CL_API_CALL clGetTracingStateINTEL(cl_tracing_handle handle,
                                          cl_bool *enable) {
    if (handle == nullptr || enable == nullptr) {
        return CL_INVALID_VALUE;
    }

    LockTracingState();

    assert(handle->handle != nullptr);
    *enable = std::any_of(tracingHandle.begin(), tracingHandle.end(),
        [&handle](TracingHandle *h) { return h == handle->handle; }) ?
        CL_TRUE : CL_FALSE;

    UnlockTracingState();
    return CL_SUCCESS;
}
SET_ALIAS(clGetTracingStateINTEL);
REGISTER_EXTENSION_FUNCTION(clGetTracingStateINTEL, clGetTracingStateINTEL);
