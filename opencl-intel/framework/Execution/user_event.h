// Copyright (c) 2008-2012 Intel Corporation
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
//  user_event.h
//  Implementation of the Class UserEvent
//  Created on:      04-Jul-2010 
//  Original author: Singer, Doron
///////////////////////////////////////////////////////////

#include <cl_types.h>
#include <cl_object.h>
#include "queue_event.h"

namespace Intel { namespace OpenCL { namespace Framework {

    class UserEvent : public OclEvent
    {
    public:
        PREPARE_SHARED_PTR(UserEvent)

        static SharedPtr<UserEvent> Allocate(_cl_context_int* context)
        {
            return SharedPtr<UserEvent>(new UserEvent(context));
        }

        // OCLObject implementation
        cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize, void * paramValue, size_t * szParamValueSizeRet) const;

        void        SetComplete(cl_int returnCode);

        void        NotifyInvisible();

    protected:
        
        UserEvent( _cl_context_int* context );

        virtual ~UserEvent();        

        // A UserEvent object cannot be copied
        UserEvent(const UserEvent&);           // copy constructor
        UserEvent& operator=(const UserEvent&);// assignment operator
    };

}}}    // Intel::OpenCL::Framework
