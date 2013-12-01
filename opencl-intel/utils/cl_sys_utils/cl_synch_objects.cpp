/////////////////////////////////////////////////////////////////////////
// cl_synch_objects.cpp
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

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

