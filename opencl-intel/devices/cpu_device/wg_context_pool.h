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

#include <cl_synch_objects.h>
#include "cl_device_api.h"
#include "cl_object_pool.h"
#include "wg_context.h"

namespace Intel { namespace OpenCL { namespace CPUDevice {

struct WGContextWrapper
{
    WGContext* pContext;
};

/**
 * This class implements IWGContextPool for CPU device agent
 */
class WgContextPool : public IWGContextPool
{
public:

     WgContextPool();
     	
     virtual ~WgContextPool() {}
     
    /**
     * Init a context pool with enough contexts for maxNumWorkers workers
     */
     void Init(unsigned int maxNumThreads, unsigned int maxNumMasters);

    /**
     * Clear and delete all WG contexts
     */
    void Clear();

    // overriden methods:

    virtual WGContextBase* GetWGContext(bool bBelongsToMasterThread);    

    virtual void ReleaseWorkerWGContext(WGContextBase* pWgContext) {}

private:

    WGContext*                          m_wgContextPool;
    WGContextWrapper*                   m_wgContextWrapperPool;
    Intel::OpenCL::Utils::AtomicCounter m_nextWorkerContext;
    unsigned int                        m_maxNumThreads;

};

}}}
