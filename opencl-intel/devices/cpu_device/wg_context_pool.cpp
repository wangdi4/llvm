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
  

WgContextPool::WgContextPool() : m_wgContextPool(NULL), m_wgContextWrapperPool(NULL), m_nextWorkerContext(0), m_maxNumWorkers(0)
{
}

void WgContextPool::Init(unsigned int maxNumWorkers)
{
    assert (NULL == m_wgContextPool && "Context pool already initialized");
    assert (NULL == m_wgContextWrapperPool && "Context pool already initialized");
    assert (0 < maxNumWorkers && "Zero threads requested");

    m_maxNumWorkers = maxNumWorkers;
    if (0 == m_maxNumWorkers) return;

    m_wgContextPool        = new WGContext[m_maxNumWorkers];
    m_wgContextWrapperPool = new WGContextWrapper[m_maxNumWorkers];
    assert (NULL != m_wgContextPool && "Memory allocation failed");
    assert (NULL != m_wgContextWrapperPool && "Memory allocation failed");

    for (long i = 0; i < m_maxNumWorkers; ++i)
    {
        m_wgContextWrapperPool[i].pContext = m_wgContextPool + i;
    }
}

void WgContextPool::Clear()
{
    if (NULL != m_wgContextWrapperPool)
    {
        for (long i = 0; i < m_maxNumWorkers; ++i)
        {
            m_wgContextWrapperPool[i].pContext = NULL;
        }
    }
    if (NULL != m_wgContextPool)
    {
        delete[] m_wgContextPool;
        m_wgContextPool = NULL;
    }
    m_maxNumWorkers = 0;
    m_nextWorkerContext = m_maxNumWorkers;
}

WGContextBase* WgContextPool::GetWGContext(bool bBelongsToMasterThread)
{
    static THREAD_LOCAL WGContextWrapper* t_pContext = NULL;

    if ((NULL != t_pContext) && (NULL != t_pContext->pContext))
    {
        return t_pContext->pContext;
    }
    
    
    if (bBelongsToMasterThread)
    {
        if (NULL == t_pContext)
        {
            t_pContext = new WGContextWrapper();
        }
        t_pContext->pContext = new WGContext();
    }
    else
    {
        unsigned int myIndex = m_nextWorkerContext++;
        assert(myIndex < m_maxNumWorkers);
        if (myIndex < m_maxNumWorkers)
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
    }
    if ((NULL != t_pContext) && (NULL != t_pContext->pContext))
    {
        t_pContext->pContext->Init();
        t_pContext->pContext->SetBelongsToMasterThread(bBelongsToMasterThread);
    }
    return t_pContext->pContext;
}
