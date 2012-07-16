// dllmain.cpp : Defines the entry point for the DLL application.
#include <cassert>
#include <libgen.h>

#include "stdafx.h"
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

// explicitly instantiate some SharedPtr classes:
template class SharedPtr<GenericMemObject>;
template class SharedPtr<GenericMemObject const>;
template class SharedPtr<MemoryObject>;
template class SharedPtr<Context>;
template class SharedPtr<GenericMemObject::DataCopyEvent>;
template class SharedPtr<IOclCommandQueueBase>;
template class SharedPtr<FissionableDevice>;
template class SharedPtr<QueueEvent>;
template class SharedPtr<QueueEvent const>;
template class SharedPtr<OclEvent>;
template class SharedPtr<OCLObject<_cl_device_id_int, _cl_platform_id_int> >;
template class SharedPtr<OCLObject<_cl_event_int, _cl_context_int> >;
template class SharedPtr<FissionableDevice const>;

void dll_init(void)
{
#if _DEBUG  // this is needed to initialize allocated objects DB, which is maintained in only in debug
    InitSharedPtrs();
#endif
}

void dll_fini(void)
{
	// release the framework proxy object 
	Intel::OpenCL::Framework::FrameworkProxy::Destroy();
#if _DEBUG
    FiniSharedPts();
#endif
}
