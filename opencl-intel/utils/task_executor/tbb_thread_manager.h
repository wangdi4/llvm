// Copyright (c) 2006-2013 Intel Corporation
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

#include "cl_thread.h"
#include "cl_synch_objects.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

class TEDevice;

template <class Data>
class TBB_ThreadManager;

template <class Data>
struct TBB_ThreadDescriptor
{
public:
    Data    m_data;

    friend class TBB_ThreadManager<Data>;
};

//
// Manage per-thread data for all threads as they enter any TEDevice.
// Implements simple single-linked free list with partial static preallocation for performance
//
template <class Data>
class TBB_ThreadManager
{
public:
    TBB_ThreadManager();
    ~TBB_ThreadManager();

    // return false on error
    bool Init( unsigned int uiNumberOfThreadsToOptimize );

    // allocate new thread entry
    Data* RegisterCurrentThread();
    void  UnregisterCurrentThread();

    static Data* GetCurrentThreadDescriptor()
    {
        TBB_ThreadDescriptor<Data>* cached = m_CurrentThreadGlobalID;
        return (NULL != cached) ? &(cached->m_data) : NULL; 
    }

    // register thread of still integistered
    Data* RegisterAndGetCurrentThreadDescriptor() 
    {
        Data* d = GetCurrentThreadDescriptor();
        return (NULL != d) ? d : RegisterCurrentThread(); 
    }

private:

    TBB_ThreadDescriptor<Data>*         m_DescriptorsArray;

    static THREAD_LOCAL TBB_ThreadDescriptor<Data>*    m_CurrentThreadGlobalID;

    unsigned int                        m_uiNumberOfStaticEntries;
    Intel::OpenCL::Utils::AtomicCounter m_nextFreeEntry;
    volatile bool                       m_bOverflowed;
    static bool                         m_object_exists;

    // do not implement
    TBB_ThreadManager(const TBB_ThreadManager& o);
    TBB_ThreadManager& operator=( const TBB_ThreadManager& o );
};

#include "tbb_thread_manager.hpp"

}}}
