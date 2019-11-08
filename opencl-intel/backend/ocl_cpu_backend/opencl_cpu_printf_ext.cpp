// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#define NOMINMAX

#include "ExecutionContext.h"
#include "ICLDevBackendServiceFactory.h"
#include "exceptions.h"
#include "opencl_printf_ext.h"
#include "llvm/Support/Mutex.h"
#include <stdio.h>

using namespace std;
using namespace Intel::OpenCL;

int printFormatCommon(OutputAccumulator& output, const char* format, const char* args);

// Used to ensure that only one thread executes opencl_printf simultaneously,
// to avoid intermingling of output from different threads.
//
static llvm::sys::Mutex m_lock;

extern "C" LLVM_BACKEND_API int opencl_printf(const char* format, char* args, void* pCallback, void* pHandle)
{
    std::lock_guard<llvm::sys::Mutex> locked(m_lock);
    StreamOutputAccumulator output(stdout);
    return printFormatCommon(output, format, args);
}

