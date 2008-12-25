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

///////////////////////////////////////////////////////////
//  in_order_queue.h
//  Implementation of the Class InOrderQueue
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(EA_BA312865_45A0_4eea_B930_C28BB9E19102__INCLUDED_)
#define EA_BA312865_45A0_4eea_B930_C28BB9E19102__INCLUDED_

#include <cl_framework.h>
#include "command_queue.h"

namespace Intel { namespace OpenCL { namespace Framework {

    //Forward declrations
    class Command;

    class InOrderQueue : public ICommandQueue
    {

    public:
	    InOrderQueue();
	    virtual ~InOrderQueue();

	    void        PushBack(Command* command);
	    Command*    GetNextCommand();
        cl_err_code AddCommand(Command* command);

    private:
        Command** m_commands;     // The actual list of command. TODO: Make a list object...

    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(EA_BA312865_45A0_4eea_B930_C28BB9E19102__INCLUDED_)
