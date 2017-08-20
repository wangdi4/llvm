// Copyright (c) 2006-2012 Intel Corporation
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

#define INSTANTIATE_THREAD_MANAGER( Data )                                                                                                \
template <>                                                                                                                               \
THREAD_LOCAL TBB_ThreadDescriptor< Data >* Intel::OpenCL::TaskExecutor::TBB_ThreadManager< Data >::m_CurrentThreadGlobalID= nullptr;         \
template <>                                                                                                                               \
bool  Intel::OpenCL::TaskExecutor::TBB_ThreadManager< Data >::m_object_exists = false                                                     \


template <class Data>
TBB_ThreadManager<Data>::TBB_ThreadManager() :
    m_DescriptorsArray(nullptr), m_uiNumberOfStaticEntries(0), m_nextFreeEntry(0), m_bOverflowed(false)
{
}

template <class Data>
bool
TBB_ThreadManager<Data>::Init( unsigned int uiNumberOfThreads )
{
    assert( false == m_object_exists );

    if (m_object_exists)
    {
        return false;
    }

    m_object_exists = true;
    m_uiNumberOfStaticEntries = uiNumberOfThreads;

    // allocate data
    if (m_uiNumberOfStaticEntries > 0)
    {
        m_DescriptorsArray = new TBB_ThreadDescriptor<Data>[m_uiNumberOfStaticEntries];

        if (nullptr == m_DescriptorsArray)
        {
            m_object_exists = false;
            m_uiNumberOfStaticEntries = 0;
            return false;
        }
    }

    return true;
}

template <class Data>
TBB_ThreadManager<Data>::~TBB_ThreadManager() 
{
    m_object_exists = false;
    // Intentionally leak everything. The objects are small. 
    // If you want to be a hero, you can:
    // 0. Track "overflow" entries 
    // 1. Invalidate the thread descriptors held by the threads
    // 2. Delete[] the descriptor array
    // 3. Iterate over the overflow entries and delete them
}

template <class Data>
Data* TBB_ThreadManager<Data>::RegisterCurrentThread()
{
    assert( m_object_exists );
    if (!m_object_exists)
    {
        return nullptr;
    }

    unsigned int myEntry = m_uiNumberOfStaticEntries;
    if (!m_bOverflowed)
    {
        myEntry = (unsigned int)m_nextFreeEntry++;
        if (myEntry >= m_uiNumberOfStaticEntries)
        {
            m_bOverflowed = true;
        }
    }
    // m_bOverflowed is a sticky flag indicating entries need to be allocated on demand
    // The point is to prevent write accesses to a shared location and save atomic operations
    // Once we've overflowed, we don't use the atomic counter anymore, nor do we write to the bool itself


    if (myEntry < m_uiNumberOfStaticEntries)
    {
        m_CurrentThreadGlobalID = m_DescriptorsArray + myEntry;
    }
    else
    {
        // Overflow, allocate on demand
        // These are not tracked anywhere so they leak, but we don't have to synchronize anything so hurray
        m_CurrentThreadGlobalID = new TBB_ThreadDescriptor<Data>;
        if (nullptr == m_CurrentThreadGlobalID)
        {
            return nullptr;
        }
    }

    m_CurrentThreadGlobalID->m_data.thread_attach();
    return &(m_CurrentThreadGlobalID->m_data);
}

template <class Data>
void TBB_ThreadManager<Data>::UnregisterCurrentThread()
{
    // Intentional leak to avoid allocate/deallocate whenever threads join/leave an arena.
}

