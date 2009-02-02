// Copyright (c) 2008-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once
///////////////////////////////////////////////////////////
//  in_order_queue.h
//  Implementation of the Class InOrderQueue
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OCL_IN_ORDER_QUEUE_H__)
#define __OCL_IN_ORDER_QUEUE_H__

#include <cl_types.h>
#include <logger.h>
#include <cl_synch_objects.h>
#include "command_queue.h"
#include <list>

using namespace std;
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

    //Forward declrations
    class Command;

    /************************************************************************
     * InOrderQueue is a ICommandQueue that implementes the InOrder queue policy
     * as it defined in the openCL spec.
     * If the application creates queue in In-Order mode, the queue is attaced
     * with this object.
     * 
     * The implemented policy for each GetNextCommand is to query the front command
     * in the queue and to identify if the command is ready for execution (Green).
     * If the front command has already processed (black mode), the queue will pop it
     * out and delete it.
     * If it does the command is popped out and returned. Otherwise, the function is
     * blocked until the queue is signaled. After signal, it queries again.
     * The nature if the In order command is that foreach AddCommand, the command
     * is entered into the back of the queue, only after it registered as dependent 
     * on the previous command in the back.
     * 
    /************************************************************************/ 
    class InOrderQueue : public ICommandQueue
    {

    public:
	    InOrderQueue();
	    virtual ~InOrderQueue();

        cl_err_code Init()                  { return CL_SUCCESS; };  // Nothing to do in InOrderQueue, list is initialized on creation 
        Command*    GetNextCommand();
        cl_err_code AddCommand(Command* command);
	    cl_err_code PushFront(Command* command);
	    void        Signal();
        bool        IsEmpty() const;
        cl_uint     Size() const;
        cl_err_code Release();

    private:
        list<Command*>  m_waitingCmdsList;      // Commands that are not yet ready to be processed.
        list<Command*>  m_readyCmdsList;        // "Green" commands, commands that can be flushed to the device. currently no more than 1
        list<Command*>  m_deviceCmdsList;       // All commands that are already flushed to the device.

        bool            m_bCleanUp;             // If true, A cleanup process is executed and all functions that changes the list should return
        OclCondition*   m_pCond;                // Condition variable that is used to block calls to GetNextCommand.
        OclMutex*       m_pListLockerMutex;     // Mutex for acces to the commandsList

        // Logger client for logging operations. DEBUGGING
        Intel::OpenCL::Utils::LoggerClient* m_pLoggerClient; 

        // Private functions
        void StableLists();

        // A queue cannot be copied
        InOrderQueue(const InOrderQueue&);           // copy constructor
        InOrderQueue& operator=(const InOrderQueue&);// assignment operator
    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_IN_ORDER_QUEUE_H__)
