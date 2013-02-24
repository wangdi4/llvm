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
THREAD_LOCAL TBB_ThreadDescriptor< Data >* Intel::OpenCL::TaskExecutor::TBB_ThreadManager< Data >::m_CurrentThreadGlobalID= NULL;         \
template <>                                                                                                                               \
bool  Intel::OpenCL::TaskExecutor::TBB_ThreadManager< Data >::m_object_exists = false                                                     \


template <class Data>
TBB_ThreadManager<Data>::TBB_ThreadManager() :
    m_FreeListHead(NULL), m_DescriptorsArray(NULL), m_uiNumberOfStaticEntries(0)
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

        if (NULL == m_DescriptorsArray)
        {
            m_object_exists = false;
            m_uiNumberOfStaticEntries = 0;
            return false;
        }

        for (unsigned int i = 0; i < m_uiNumberOfStaticEntries-1; ++i)
        {
            m_DescriptorsArray[i].next_free_entry = &(m_DescriptorsArray[i+1]);
        }

        m_FreeListHead = &(m_DescriptorsArray[0]);
    }

    return true;
}

template <class Data>
TBB_ThreadManager<Data>::~TBB_ThreadManager() 
{
    m_object_exists = false;

    // find overflowed entries in the free list and delete them
    TBB_ThreadDescriptor<Data>* next = m_FreeListHead;
    while (NULL != next)
    {
        TBB_ThreadDescriptor<Data>* tmp = next;
        next = next->next_free_entry;

        if ((NULL == m_DescriptorsArray) || 
            (tmp < &(m_DescriptorsArray[0])) || (&(m_DescriptorsArray[m_uiNumberOfStaticEntries-1]) < tmp))
        {
            // this is an overflow entry
            delete tmp;
        }
    }

    if (NULL != m_DescriptorsArray)
    {
        delete [] m_DescriptorsArray;
    }
}

template <class Data>
Data* TBB_ThreadManager<Data>::RegisterCurrentThread()
{
    assert( m_object_exists );
    if (!m_object_exists)
    {
        return NULL;
    }

    Intel::OpenCL::Utils::OclAutoMutex lock( &m_lock );

    if (NULL == m_FreeListHead)
    {
        // number of threads overflow - allocate for extra
        m_CurrentThreadGlobalID         = new TBB_ThreadDescriptor<Data>;
        if (NULL == m_CurrentThreadGlobalID)
        {
            return NULL;
        }
        m_CurrentThreadGlobalID->m_data.thread_attach();
        return &(m_CurrentThreadGlobalID->m_data);
    }

    m_CurrentThreadGlobalID = m_FreeListHead;
    m_FreeListHead          = m_FreeListHead->next_free_entry;
    m_CurrentThreadGlobalID->next_free_entry = NULL;
    m_CurrentThreadGlobalID->m_data.thread_attach();

    return &(m_CurrentThreadGlobalID->m_data);
}

template <class Data>
void TBB_ThreadManager<Data>::UnregisterCurrentThread()
{
    assert( m_object_exists );
    if (!m_object_exists)
    {
        return;
    }

    if (NULL == m_CurrentThreadGlobalID)
    {
        return;
    }

    // add overflowed entries to the free list also - to be used later if required 

    Intel::OpenCL::Utils::OclAutoMutex lock( &m_lock );

    // insert into free list
    m_CurrentThreadGlobalID->next_free_entry = m_FreeListHead;
    m_FreeListHead                           = m_CurrentThreadGlobalID;
    m_CurrentThreadGlobalID                  = NULL;
}

