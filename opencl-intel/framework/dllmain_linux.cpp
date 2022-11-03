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

#include "Context.h"
#include "Device.h"
#include "GenericMemObj.h"
#include "MemoryObject.h"
#include "cl_secure_string_linux.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "command_queue.h"
#include "framework_proxy.h"
#include "ocl_event.h"
#include "queue_event.h"

__attribute__((constructor)) static void dll_init(void);
// As far as possible let dll_fini be called last
__attribute__((destructor(100))) static void dll_fini(void);

using namespace Intel::OpenCL::Framework;

// explicitly instantiate some SharedPtrBase classes:
template class Intel::OpenCL::Utils::SharedPtrBase<GenericMemObject>;
template class Intel::OpenCL::Utils::SharedPtrBase<GenericMemObject const>;
template class Intel::OpenCL::Utils::SharedPtrBase<MemoryObject>;
template class Intel::OpenCL::Utils::SharedPtrBase<Context>;
template class Intel::OpenCL::Utils::SharedPtrBase<
    GenericMemObject::DataCopyEvent>;
template class Intel::OpenCL::Utils::SharedPtrBase<IOclCommandQueueBase>;
template class Intel::OpenCL::Utils::SharedPtrBase<FissionableDevice>;
template class Intel::OpenCL::Utils::SharedPtrBase<QueueEvent>;
template class Intel::OpenCL::Utils::SharedPtrBase<QueueEvent const>;
template class Intel::OpenCL::Utils::SharedPtrBase<OclEvent>;
template class Intel::OpenCL::Utils::SharedPtrBase<
    OCLObject<_cl_device_id_int, _cl_platform_id_int>>;
template class Intel::OpenCL::Utils::SharedPtrBase<
    OCLObject<_cl_event_int, _cl_context_int>>;
template class Intel::OpenCL::Utils::SharedPtrBase<FissionableDevice const>;
template class Intel::OpenCL::Utils::SharedPtrBase<IEventObserver>;

void dll_init(void) {
#ifdef _DEBUG // this is needed to initialize allocated objects DB, which is
              // maintained in only in debug
  InitSharedPtrs();
#endif
}

void dll_fini(void) {
  Intel::OpenCL::Utils::FrameworkUserLogger::Destroy();
  Intel::OpenCL::Framework::MemoryObjectFactory::Destroy();
  // release the framework proxy object
  Intel::OpenCL::Framework::FrameworkProxy::Destroy();
#ifdef _DEBUG
  FiniSharedPts();
#endif
}
