/////////////////////////////////////////////////////////////////////////
// opencl_cpu_printf_ext.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2010 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel?s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel?s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "ExecutionContext.h"
#include "ICLDevBackendServiceFactory.h"
#include "exceptions.h"
#include "opencl_printf_ext.h"
#include "llvm/Support/MutexGuard.h"
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
    llvm::MutexGuard locked(m_lock);
    StreamOutputAccumulator output(stdout);
    return printFormatCommon(output, format, args);
}

