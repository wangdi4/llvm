// INTEL CONFIDENTIAL
//
// Copyright 2012-2023 Intel Corporation.
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

#include "GenericMemObj.h"
#include "cl_disable_sys_dialog.h"
#include "cl_shutdown.h"
#include "framework_proxy.h"

#include "llvm/Support/ManagedStatic.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;
using namespace llvm;

static int dll_init(void) {
#if defined(_WIN32) && !defined(INTEL_PRODUCT_RELEASE) && !defined(_DEBUG)
  DisableSystemDialogsOnCrash();
#endif
#ifdef _DEBUG // this is needed to initialize allocated objects DB, which is
              // maintained in only in debug
  InitSharedPtrs();
#endif
  return 0;
}

static int InitOCL = dll_init();

void dll_fini(void) {
  UpdateShutdownMode(ExitStarted);
  MemoryObjectFactory::Destroy();
  // release the framework proxy object
  FrameworkProxy::Destroy();
  llvm_shutdown();
#ifdef _DEBUG
  FiniSharedPts();
#endif
  UpdateShutdownMode(ExitDone);
}
