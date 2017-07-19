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

#pragma once

#include <stack>
#include "cl_synch_objects.h"

using Intel::OpenCL::Utils::OclAutoMutex;

namespace Intel { namespace OpenCL { namespace Utils {

/**
 * This class is responsible for managing a pool of objects that can quickly be allocated and freed.
 * @param ElemType type of the objects managed by the pool
 */
template<typename ElemType>
class ObjectPool
{
public:

    /**
     * Destructor
     */
    ~ObjectPool() { Clear(); }

    /**
     * Allocate an object from the pool. If the pool is not empty, the complexity of this operation is O(1).
     * @return a pointer to the allocated object
     */
    ElemType* Malloc();

    /**
     * Free an object back to the pool. The complexity of this operation is O(1).
     * @param pObj the object to be freed
     */
    void Free(ElemType* pObj);

    /**
     * Clear and delete all the objects in the pool
     */
    void Clear();

private:

    std::stack<ElemType*> m_stack;
    Intel::OpenCL::Utils::OclSpinMutex m_mutex;

};

template<typename ElemType>
void ObjectPool<ElemType>::Clear()
{
    while (!m_stack.empty())
    {
        delete m_stack.top();
        m_stack.pop();
    }
}

template<typename ElemType>
ElemType* ObjectPool<ElemType>::Malloc()
{
    m_mutex.Lock();
    if (m_stack.empty())
    {
        m_mutex.Unlock();   // let the heavy new operator be outside the critical section
        return new ElemType();
    }
    else
    {
        ElemType* const pElem = m_stack.top();
        assert(nullptr != pElem);
        m_stack.pop();
        m_mutex.Unlock();
        return pElem;
    }
}

template<typename ElemType>
void ObjectPool<ElemType>::Free(ElemType* pObj)
{
    assert(nullptr != pObj);
    OclAutoMutex autoMutex(&m_mutex);
    m_stack.push(pObj);
}

}}}
