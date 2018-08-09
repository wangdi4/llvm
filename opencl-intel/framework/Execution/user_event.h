// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
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

#pragma once

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
