// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include <cassert>
#include <libgen.h>

#include "framework_proxy.h"
#include "cl_secure_string_linux.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "cl_shared_ptr.hpp"
#include "GenericMemObj.h"
#include "MemoryObject.h"
#include "Context.h"
#include "command_queue.h"
#include "Device.h"
#include "queue_event.h"
#include "ocl_event.h"

#pragma comment(lib, "libcl_sys_utils.a")
#pragma comment(lib, "libcl_logger.so")
#pragma comment(lib, "libtask_executor.so")

__attribute__ ((constructor)) static void dll_init(void);
__attribute__ ((destructor)) static void dll_fini(void);

using namespace Intel::OpenCL::Framework;

// explicitly instantiate some SharedPtrBase classes:
template class SharedPtrBase<GenericMemObject>;
template class SharedPtrBase<GenericMemObject const>;
template class SharedPtrBase<MemoryObject>;
template class SharedPtrBase<Context>;
template class SharedPtrBase<GenericMemObject::DataCopyEvent>;
template class SharedPtrBase<IOclCommandQueueBase>;
template class SharedPtrBase<FissionableDevice>;
template class SharedPtrBase<QueueEvent>;
template class SharedPtrBase<QueueEvent const>;
template class SharedPtrBase<OclEvent>;
template class SharedPtrBase<OCLObject<_cl_device_id_int, _cl_platform_id_int> >;
template class SharedPtrBase<OCLObject<_cl_event_int, _cl_context_int> >;
template class SharedPtrBase<FissionableDevice const>;
template class SharedPtrBase<IEventObserver>;

void dll_init(void)
{
#ifdef _DEBUG  // this is needed to initialize allocated objects DB, which is maintained in only in debug
    InitSharedPtrs();
#endif
}

void dll_fini(void)
{
	// release the framework proxy object 
	Intel::OpenCL::Framework::FrameworkProxy::Destroy();
#ifdef _DEBUG
    FiniSharedPts();
#endif
}
