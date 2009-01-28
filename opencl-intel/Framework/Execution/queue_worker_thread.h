/////////////////////////////////////////////////////////////////////////
// queue_worker_thread.h
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
#ifndef __QUEUE_WORKER_THREAD_H__
#define __QUEUE_WORKER_THREAD_H__

#include <cl_types.h>
#include <cl_thread.h>


namespace Intel { namespace OpenCL { namespace Framework {

    //Forward declarations
    class ICommandQueue;
    class Command;

    /************************************************************************
     * A worker thread is an abstract thread that works on a specific working job.
     * One job at a time.
     * The jobs are sorted in a WorkList. The thread continuously gets the next job
     * from the list. If the list is empty, the thread waits until the a job is
     * ready for working or the list is destroyed.
     * Alternatively, the thread can exit if it was marked to.
     * The real worker, a SliceEncoder, is expected to implemented the ProcessJob
     * method.
    /************************************************************************/ 
    class QueueWorkerThread: public Intel::OpenCL::Utils::OclThread
    {

    public:
	    QueueWorkerThread();
	    virtual ~QueueWorkerThread();
	    cl_err_code         Init(ICommandQueue* pCommandsQueue);                
        cl_err_code         CancelProcessing();

    private:
        int                 Run();                  // The actual thread running loop.
        Command*            GetNextCommand();
        
        // Private members
        ICommandQueue*      m_pCommandsQueue;       // The command queue object
    };

}}};    // Intel::OpenCL::Framework

#endif // __QUEUE_WORKER_THREAD_H__
