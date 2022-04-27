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

#include "cl_synch_objects.h"

/////////////////////////////////////////////////////////////////////////////
// OclSpinMutex
/////////////////////////////////////////////////////////////////////////////
Intel::OpenCL::Utils::OclSpinMutex::OclSpinMutex()
{
    lMutex.exchange(0);
    threadId = INVALID_MUTEX_OWNER;
}
void Intel::OpenCL::Utils::OclSpinMutex::Lock()
{
    if (threadId == clMyThreadId())
    {
        lMutex++;
        return;
    }
    while (lMutex.test_and_set(0, 1))
    {
        // In order to improve the performance of spin-wait loops.
        InnerSpinloopImpl();
    }
    threadId = clMyThreadId();
}
void Intel::OpenCL::Utils::OclSpinMutex::Unlock()
{
    //Prevent a thread that doesn't own the mutex from unlocking it
    if (clMyThreadId() != threadId)
    {
        return;
    }
    if ( 1 == (long)lMutex )
    {
        threadId = INVALID_MUTEX_OWNER;
        lMutex.exchange(0);
        return;
    }
#ifdef _DEBUG    
    long val = lMutex--;
    assert(val != 0);
#else
  lMutex--;
#endif      
}

