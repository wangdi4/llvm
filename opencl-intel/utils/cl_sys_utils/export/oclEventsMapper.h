/////////////////////////////////////////////////////////////////////////
// oclEventsMapper.h
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2014 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel's prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel's suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////
#pragma once

#include <CL/cl.h>
#include "cl_synch_objects.h"

namespace Intel { namespace OpenCL { namespace Utils {

    // This class helps to manage mapping between pairs of events.
    // It can be used to manage wrappers for events.
    // 
    class EventsMapper
    {
    public:
        virtual void addEventPair(cl_event userEvent, cl_event notifierEvent) = 0;
        virtual cl_event getUserEvent(cl_event notifierEvent) = 0;
        virtual cl_event getNotifierEvent(cl_event userEvent) = 0;
        virtual void delEvent(cl_event userEvent) = 0;
    };

    // The class below helps the notifier collection to wrap
    // each OCL write/modify event (e.g. clEnqueueWriteBuffer),
    // with a custom event.
    //
    class NotifierEventsMapper : public EventsMapper
    {
    public:
        virtual void addEventPair(cl_event userEvent, cl_event notifierEvent);
        virtual cl_event getUserEvent(cl_event notifierEvent);
        virtual cl_event getNotifierEvent(cl_event userEvent);
        virtual void delEvent(cl_event userEvent);
    private:
        OclMutex m_lock;

        typedef cl_event UserEvent;
        typedef cl_event NotifierEvent;
        typedef map<UserEvent, NotifierEvent> EventsMap;

        EventsMap m_eventsMap;
    };

}}} // Intel::OpenCL::Utils
