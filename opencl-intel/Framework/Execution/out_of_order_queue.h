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
//  out_of_order_queue.h
//  Implementation of the Class OutOfOrderQueue
//  Created on:      23-Dec-2008 3:23:06 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OUT_OF_ORDER_QUEUE_H__)
#define __OUT_OF_ORDER_QUEUE_H__

#include <cl_types.h>
#include "in_order_queue.h"

namespace Intel { namespace OpenCL { namespace Framework {
    class QueueEvent;

    /************************************************************************
     * OutOfOrderQueue is a ICommandQueue that implementes the OutOfOrder queue policy
     * as it defined in the openCL spec.
     * If the application creates queue in Out-Of-Order mode, the queue is attaced
     * with this object.
     * 
     * In the current implementation the InOrderQueue acts as OutOfOrder Queue that
     * attaches each new command as dependent on the previous one, and by doing so the
     * InOrder policy is compelled. For this reason, the out of order queue uses the
     * InOrder implementation and only change the action on AddCommand and StableList
     * and that in order to monitor Barrier and WaitOnEvents commands.
     * 
    /************************************************************************/ 
    class OutOfOrderQueue : public InOrderQueue
    {

    public:
	    OutOfOrderQueue();
	    virtual ~OutOfOrderQueue();

        cl_err_code AddCommand(Command* command);


    private:
        Command*    m_pLastBarrierCmd;      // The last Barrier/WaitOnEvents command. 
                                            // Since each one is registered on the previous we can always replace them and keep the last one
                                            // If command is not NULL, all commands are register on this one.
        bool        StableLists();          //Override stable list to clean m_pLastBarrierCmd too
        bool        RegisterAsBarrier(QueueEvent* pEvent);

    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OUT_OF_ORDER_QUEUE_H__)
