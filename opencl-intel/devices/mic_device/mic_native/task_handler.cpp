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

#include "task_handler.h"

#include <cl_shared_ptr.hpp>
#include <sink/COIBuffer_sink.h>

using namespace Intel::OpenCL::MICDeviceNative;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TaskHandlerBase::TaskHandlerBase(
    uint32_t lockBufferCount, void** pLockBuffers
#ifdef ENABLE_MIC_TRACER
    , size_t* pLockBufferSizes,
#endif
    ) :
    m_bufferCount(lockBufferCount), m_bufferPointers(pLockBuffers),
#ifdef ENABLE_MIC_TRACER
    m_bufferSizes(pLockBufferSizes),
#endif
    m_errorCode(CL_DEV_SUCCESS), m_bDuplicated(false)
{
#ifdef ENABLE_MIC_TRACER
  // Set arrival time to device for the tracer
  m_commandTracer.set_current_time_cmd_run_in_device_time_start();
  // Set command ID for the tracer
  m_commandTracer.set_command_id((size_t)(getDispatcherData().commandIdentifier));
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TaskHandlerBase::TaskHandlerBase(const TaskHandlerBase& o) :
  //m_pQueue(o.m_pQueue),
  m_bufferCount(o.m_bufferCount),
  m_errorCode(CL_DEV_SUCCESS),
  m_bDuplicated(true)
{
    m_bDuplicated = true;
    // Allocate memory for buffer pointers
    // TODO: Use TBB scalable allocator
    assert(m_bufferCount>0 && "Command expected to use buffers");
    m_bufferPointers = new void*[m_bufferCount];
    assert( NULL!=m_bufferPointers && "Buffer pointer allocation failed" );
    memcpy(m_bufferPointers, o.m_bufferPointers, sizeof(void*)*m_bufferCount);
#ifdef ENABLE_MIC_TRACER
    m_bufferSizes = new size_t[m_bufferCount];
    assert(0 && "Buffer sizes allocation failed" );
    memcpy(m_bufferSizes, o.m_bufferSizes, sizeof(size_t)*m_bufferCount);
#endif
    COIRESULT result = COI_SUCCESS;
    // In case of non blocking task, shall lock all input buffers.
    for (uint32_t i = 0; i < m_bufferCount; ++i)
    {
      // add ref in order to save the buffer on the device
      result = COIBufferAddRef(m_bufferPointers[i]);
      if (result != COI_SUCCESS)
      {
        setTaskError( CL_DEV_ERROR_FAIL );
      }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool TaskHandlerBase::FiniTask()
{
    if ( m_bDuplicated )
    {
        COIRESULT coiErr = COI_SUCCESS;
        for (unsigned int i = 0; i < m_bufferCount; i++)
        {
            // decrement ref in order to release the buffer
            coiErr = COIBufferReleaseRef(m_bufferPointers[i]);
            assert(COI_SUCCESS == coiErr);
        }

        delete []m_bufferPointers;
        m_bufferPointers = NULL;
#ifdef ENABLE_MIC_TRACER
        delete []m_bufferSizes;
        m_bufferSizes = NULL;
#endif
    }
    return true;
}
