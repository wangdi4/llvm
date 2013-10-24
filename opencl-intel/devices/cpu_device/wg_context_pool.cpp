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

#include <cl_thread.h>
#include "wg_context_pool.h"

using namespace Intel::OpenCL::CPUDevice;
  

WgContextPool::WgContextPool() : m_wgContextPool(NULL), m_wgContextWrapperPool(NULL), m_nextWorkerContext(0), m_maxNumThreads(0)
{
}

void WgContextPool::Init(unsigned int maxNumThreads, unsigned int maxNumMasters)
{
    assert (NULL == m_wgContextPool && "Context pool already initialized");
    assert (NULL == m_wgContextWrapperPool && "Context pool already initialized");
    assert (0 < maxNumThreads && "Zero threads requested");
    assert (1 == maxNumMasters && "Current implementation supports only one master thread at a time");
    if (maxNumThreads == maxNumMasters)
    {
        //Always allow for at least one worker
        maxNumThreads++; 
    }

    m_maxNumThreads = maxNumThreads;
    if (0 == m_maxNumThreads) return;

    m_wgContextPool        = new WGContext[m_maxNumThreads];
    m_wgContextWrapperPool = new WGContextWrapper[m_maxNumThreads];
    assert (NULL != m_wgContextPool && "Memory allocation failed");
    assert (NULL != m_wgContextWrapperPool && "Memory allocation failed");

    for (unsigned int i = maxNumMasters; i < m_maxNumThreads; ++i)
    {
        m_wgContextWrapperPool[i].pContext = m_wgContextPool + i;
    }

    // The first pool is for a master thread
    m_wgContextPool->SetBelongsToMasterThread(true);
    m_wgContextPool->Init();
    m_nextWorkerContext = maxNumMasters;
}

void WgContextPool::Clear()
{
    if (NULL != m_wgContextWrapperPool)
    {
        for (unsigned int i = 0; i < m_maxNumThreads; ++i)
        {
            m_wgContextWrapperPool[i].pContext = NULL;
        }
    }
    if (NULL != m_wgContextPool)
    {
        delete[] m_wgContextPool;
        m_wgContextPool = NULL;
    }
    m_maxNumThreads = 0;
    m_nextWorkerContext = m_maxNumThreads;
}

WGContextBase* WgContextPool::GetWGContext(bool bBelongsToMasterThread)
{
    static THREAD_LOCAL WGContextWrapper* t_pContext = NULL;

    if (bBelongsToMasterThread)
    {
        // The first pool is for a master thread
        return m_wgContextPool;
    }

    if ((NULL != t_pContext) && (NULL != t_pContext->pContext))
    {
        return t_pContext->pContext;
    }
    
    
    unsigned int myIndex = m_nextWorkerContext++;
    assert(myIndex < m_maxNumThreads && "Worker overflow - received too many workers");
    if (myIndex < m_maxNumThreads)
    {
        t_pContext = m_wgContextWrapperPool + myIndex;
    }
    else
    {
        if (NULL == t_pContext)
        {
            t_pContext = new WGContextWrapper();
        }
        t_pContext->pContext = new WGContext();
        // Note: we should never get here, unless more worker contexts were allocated than the maximum defined by the device
        // Better to leak the worker context above than crash
    }

    if ((NULL != t_pContext) && (NULL != t_pContext->pContext))
    {
        t_pContext->pContext->Init();
        t_pContext->pContext->SetBelongsToMasterThread(bBelongsToMasterThread);
        return t_pContext->pContext;
    }
    else
    {
        return NULL;
    }
}
