/////////////////////////////////////////////////////////////////////////
// opencl_mic_printf_ext.cpp:
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
#include "exceptions.h"
#include "opencl_printf_ext.h"
#include "cl_dev_backend_api.h"

#include <stdlib.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL;

int printFormatCommon(OutputAccumulator& output, const char* format, const char* args);

extern "C" LLVM_BACKEND_API int opencl_mic_printf(const char* format, char* args, void* pCallback, void* pHandle)
{
    char lastChar;
    StringOutputAccumulator out_counter(&lastChar, 1);
    // first run in order to know the required buffer size
    int retVal = printFormatCommon(out_counter, format, args);
    if (0 > retVal) return retVal;
    
    // need to allocate memory for the terminating '/0' as well, thus the '+1'
    char *buf = (char*)malloc(sizeof(char) * (out_counter.output_count())+1);
    if (nullptr == buf) return -1;

    StringOutputAccumulator output(buf, out_counter.output_count()+1);
    // print the output string
    retVal = printFormatCommon(output, format, args);
    if (0 > retVal) 
    {
        free(buf);
        return retVal;
    }

    if (nullptr !=  pCallback)
    {
        // send the output buffer to the runtime
        retVal = ((DeviceBackend::ICLDevBackendDeviceAgentCallback*)pCallback)->Print(buf, pHandle);
    }
    else
    {
        assert(false && "No Device Printer Found");
        retVal = -1;
    }

    // free the buffer
    free(buf);
    return retVal;
}



