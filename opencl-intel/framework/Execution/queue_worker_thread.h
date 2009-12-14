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
#include <logger.h>


namespace Intel { namespace OpenCL { namespace Framework {

    //Forward declarations
    class ICommandQueue;
    class Command;

    /************************************************************************
     * A worker thread is hold a thread that work on a specific command.
     * One Command at a time.
     * The command are available in a CommandQueue. The thread continuously gets 
     * the next command from the queue. If the queue is empty, the thread waits until 
     * the a command is ready for executing or the list has finished.
     * Alternatively, the thread can exit if it was marked to.
     * the thread is responsible to clean and delete it self.
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

        // Logger client for logging operations. DEBUGGING
		DECLARE_LOGGER_CLIENT;

    };

}}};    // Intel::OpenCL::Framework

#endif // __QUEUE_WORKER_THREAD_H__
