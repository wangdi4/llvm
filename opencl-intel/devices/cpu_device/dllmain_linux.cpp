// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

///////////////////////////////////////////////////////////
//  dllmain.cpp
///////////////////////////////////////////////////////////

#include "cl_sys_info.h"
#include "cpu_device.h"

#include <libgen.h>
#include <pthread.h>

using namespace Intel::OpenCL::CPUDevice;

static void __attribute__((constructor)) dll_init(void);
static void __attribute__((destructor)) dll_fini(void);

pthread_key_t thkMasterContext;

static void thread_cleanup_callback(void * /*_NULL*/) {}

void dll_init(void) {
  thkMasterContext = 0;
  pthread_key_create(&thkMasterContext, thread_cleanup_callback);
}

void dll_fini(void) {
  if (thkMasterContext) {
    pthread_key_delete(thkMasterContext);
  }
}
