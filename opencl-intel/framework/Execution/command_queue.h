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
//  command_queue.h
//  Implementation of the Class ICommandQueue
//  Created on:      23-Dec-2008 3:23:01 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OCL_COMMAND_QUEUE_H__)
#define __OCL_COMMAND_QUEUE_H__

#include <cl_types.h>
#include "event_color_change_observer.h"

namespace Intel { namespace OpenCL { namespace Framework {

    //Forward declration
    class Command;

    /**
     * 
     * 
     */
    class ICommandQueue 
    {

    public:
        virtual cl_err_code Init() =0;
        virtual Command*    GetNextCommand() =0;
        virtual cl_err_code AddCommand(Command* command)  =0;
        virtual cl_err_code Flush( bool bBlocking )       =0;
        virtual void        Signal()    =0;
        virtual bool        IsEmpty()   =0;
        virtual cl_uint     Size()      =0;
        virtual cl_err_code Release()   =0;
        virtual cl_err_code NotifyEventColorChange( 
            const QueueEvent*       cpEvent, 
            QueueEventStateColor    prevColor,  
            QueueEventStateColor    newColor )  =0;

        virtual ~ICommandQueue(){}; // Virtual D'tor        
    };
}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_COMMAND_QUEUE_H__)



