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

#include <cl_shared_ptr.h>
#include <task_executor.h>

#include <malloc.h>

#include "native_program_service.h"

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

//
// TaskHandlerBase - state of specific task handled by QueueOnDevice
//
class TaskHandlerBase : virtual public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:
    PREPARE_SHARED_PTR(TaskHandlerBase);

    TaskHandlerBase(
        uint32_t lockBufferCount, void** pLockBuffers
#ifdef ENABLE_MIC_TRACER
        , size_t* pLockBufferSizes
#endif
        );

    virtual ~TaskHandlerBase() {};

    // Creates a copy of the task
    virtual TaskHandlerBase*     Duplicate() const = 0;

    // Retrieve a pointer to ITaskBase
    virtual Intel::OpenCL::TaskExecutor::ITaskBase* GetAsITaskBase() = 0;
    virtual TaskHandlerBase*                        GetAsTaskHandlerBase() { return this;}

    // Called before enqueue into the queue
    virtual bool PrepareTask() = 0;

    bool FiniTask();

    void setTaskError( cl_dev_err_code errorCode )
    {
        if ( CL_DEV_SUCCEEDED(m_errorCode) )
        {
            m_errorCode = errorCode;
        }
    }
    cl_dev_err_code getTaskError() const { return m_errorCode; };

#ifdef ENABLE_MIC_TRACER
    virtual const dispatcher_data& getDispatcherData() const = 0;
    CommandTracer& commandTracer() { return m_commandTracer; }
#endif

protected:
    TaskHandlerBase(const TaskHandlerBase& o);

    uint32_t              m_bufferCount;
    void**                m_bufferPointers;
#ifdef ENABLE_MIC_TRACER
    size_t*               m_bufferSizes;
#endif
    cl_dev_err_code       m_errorCode;

    bool                  m_bDuplicated;

#ifdef ENABLE_MIC_TRACER
    // Command tracer
    CommandTracer         m_commandTracer;
#endif

};

template<class Command, typename dispatch_data_type > class TaskHandler : public TaskHandlerBase
{
public:
    //PREPARE_SHARED_PTR(TaskHandler<typename Command >)

    TaskHandler(
        //const QueueOnDevice* pQueue,
        uint32_t lockBufferCount,
        void** pLockBuffers,
#ifdef ENABLE_MIC_TRACER
        size_t* pLockBufferSizes,
#endif
        dispatch_data_type* pDispatcherData,
        size_t uiDispatchSize
        ) :
        TaskHandlerBase(/*pQueue,*/ lockBufferCount, pLockBuffers
#ifdef ENABLE_MIC_TRACER
        , pLockBufferSizes
#endif
        ),
        m_dispatcherData(pDispatcherData),
        m_uiDispatchSize(uiDispatchSize)
    {
    }

    virtual ~TaskHandler()
    {
        if ( m_bDuplicated )
        {
            free(m_dispatcherData);
            m_dispatcherData = NULL;
        }
    }

    virtual TaskHandlerBase* Duplicate() const
    {
        // TODO: convert to tbb::scalable_allocator and allocate memory for whole object at a time
        TaskHandler* pNewHandler = new Command( this->GetAsCommandTypeConst() );
        if ( NULL == pNewHandler )
        {
            assert (0 && "Task duplication failed" );
            return NULL;
        }
        // Copy buffers and kernel parameters
        return pNewHandler;
    }

    virtual const Command&       GetAsCommandTypeConst() const = 0;


#ifdef ENABLE_MIC_TRACER
    virtual const dispatcher_data& getDispatcherData() const
    {
        return m_dispatcherData;
    }
#endif

protected:
    // The received dispatcher_data
    dispatch_data_type* m_dispatcherData;
    size_t              m_uiDispatchSize;

    TaskHandler(const TaskHandler& o) : TaskHandlerBase(o)
    {
        m_uiDispatchSize = o.m_uiDispatchSize;
        m_dispatcherData = (dispatch_data_type *)memalign(8, o.m_uiDispatchSize);
        memcpy(m_dispatcherData, o.m_dispatcherData, m_uiDispatchSize);
    }

private:
    // operator assigne is not allowed
    TaskHandler& operator= (const TaskHandler& o);
};

}}}
