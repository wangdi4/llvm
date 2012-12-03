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

#include <cassert>
#include <set>
#include <map>
#include "tbb/task_scheduler_observer.h"
#include "tbb/task_arena.h"
#include "cl_device_api.h"
#include "cl_synch_objects.h"
#include "task_executor.h"
#include "tbb_executor.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

class TBBTaskExecutor;
class ArenaHandler;
class in_order_executor_task;
class base_command_list;
template<typename T> class command_list;

using Intel::OpenCL::Utils::AtomicCounter;
using Intel::OpenCL::Utils::SharedPtr;

/**
 * This class is responsible for handling entering and leaving the arena of the fissionable device
 */
class DevArenaObserver : public tbb::task_scheduler_observer
{
public:

    /**
     * Constructor
     * @param arena         the device's task_arena
     * @param taskExecutor  a reference to the TBBTaskExecutor															
     */
    DevArenaObserver(tbb::task_arena& arena, TBBTaskExecutor& taskExecutor) : tbb::task_scheduler_observer(arena), m_taskExecutor(taskExecutor) { }

    /**
     * Set the ArenaHandler
     * @param pArenaHandler a pointer to the ArenaHandler
     */
    void SetArenaHandler(ArenaHandler* pArenaHandler)
    {
        m_pArenaHandler = pArenaHandler;
    }

    // overriden methods

    virtual void on_scheduler_entry(bool bIsWorker);

    virtual void on_scheduler_exit(bool bIsWorker);

    virtual bool on_scheduler_leaving();

private:

    TBBTaskExecutor& m_taskExecutor;
    ArenaHandler* m_pArenaHandler;

};

/**
 * This class represents a DevArenaObserver for sub-devices
 */
class SubdevArenaObserver : public DevArenaObserver
{
public:

    /**
     * Constructor
     * @param arena             the device's task_arena
     * @param taskExecutor      a reference to the TBBTaskExecutor															
     * @param pLegalCores       an array of the core IDs which it is legal for the worker threads to affinitize to
     * @param szNumlegalCores   size of pLegalCores array
     * @param observer          the IAffinityChangeObserver that observers changes in the affinity of worker threads																		
     */
    SubdevArenaObserver(tbb::task_arena& arena, TBBTaskExecutor& taskExecutor, const unsigned int* pLegalCores, size_t szNumlegalCores, IAffinityChangeObserver& observer);

    // overriden methods

    virtual void on_scheduler_entry(bool bIsWorker);

    virtual void on_scheduler_exit(bool bIsWorker);

private:

    Intel::OpenCL::Utils::OclMutex m_mutex;
    std::set<unsigned int> m_legalCores;
    std::map<int, unsigned int> m_slots2Cores;
    IAffinityChangeObserver& m_observer;    
};

/**
 * This class is responsible for handling the explicit arena and the per arena slot resources (work group context) for a single fissionable device.
 */
class ArenaHandler
{
public:
        
    /**
     * Destructor
     */
    virtual ~ArenaHandler();

    /**
     * @return a pointer to the work group context for the current worker thread
     */
    WGContextBase* GetWGContext();

    /**
     * Enqueue a functor on the arena.
     * @param F the type of the functor
     * @param f the functor object
     */
    template <class F>
    void Enqueue(F& f);

    /**
     * Execute a functor on the arena.
     * @param F the type of the functor
     * @param f the functor object
     */
    template<class F>
    void Execute(F& f) { m_arena.execute(f); }

    /**
     * Wait for all work in the arena is completed
     */
    void WaitUntilEmpty();

    /**
     * @param uiSlot slot number
     * @return a pointer to the WGContextBase in uiSlot
     */
    WGContextBase* GetWGContext(unsigned int uiSlot)
    {
        assert(uiSlot < m_wgContexts.size());
        return m_wgContexts[uiSlot];
    }

    /**
     * Set a WGContextBase in a certain slot number
     * @param uiSlot        the slot number
     * @param pWgContext    a pointer to the WGContextBase
     */
    void SetWGContext(unsigned int uiSlot, WGContextBase* pWgContext)
    {
        assert(uiSlot < m_wgContexts.size());
        m_wgContexts[uiSlot] = pWgContext;
    }

    /**
     * @return whether there are enqueued EnqueuedRunnable objects in the arena
     */
    bool AreEnqueuedTasks() const { return m_numEnqueuedTasks > 0; }    

    /**
     * @return the number of compute units of the sub-device of this ArenaHandler
     */
    unsigned int GetNumSubdevComputeUnits() const { return m_uiNumSubdevComputeUnits; }

    /**
     * Add a base_command_list that uses this ArenaHandler
     * @param pCmdList the base_command_list to add
     */
    void AddCommandList(const SharedPtr<base_command_list>& pCmdList);

    /**
     * Remove a base_command_list that uses this ArenaHandler
     * @param pCmdList the base_command_list to remove
     */
    void RemoveCommandList(const base_command_list* pCmdList);

protected:

    /**
     * Constructor
     * @param uiNumSubdevComputeUnits   number of computing units in the sub-device
     * @param uiNumTotalComputeUnits	number of total computing units in the device
     * @param taskExecutor              a reference to the TBBTaskExecutor															
     * @param devArenaHandler           the DevArenaObserver of this ArenaHandler									
     */
    ArenaHandler(unsigned int uiNumSubdevComputeUnits, unsigned int uiNumTotalComputeUnits, TBBTaskExecutor& taskExecutor);

    /**
     * Initialize this ArenaHandler with a DevArenaObserver (it should be called by sub-classes in their constructor body)
     * @param pArenaObserver the DevArenaObserver to initialize this ArenaHandler with
     */
    void Init(DevArenaObserver* pArenaObserver);    

    /**
     * The explicit arena of this device
     */
    tbb::task_arena m_arena;

private:

    std::vector<WGContextBase*> m_wgContexts; // Since each thread has its own slot in this vector, accessing it doesn't need synchronization.    
    TBBTaskExecutor& m_taskExecutor;
        
    DevArenaObserver* m_pArenaObserver;
    AtomicCounter m_numEnqueuedTasks;    
    const unsigned int m_uiNumSubdevComputeUnits;
    Intel::OpenCL::Utils::OclMutex m_mutex;
    std::set<SharedPtr<base_command_list> > m_cmdLists;

    // do not implement:
    ArenaHandler(const ArenaHandler&);
    ArenaHandler& operator=(const ArenaHandler&);

    // auxiliary class for Enqueue
    template<typename F>
    class EnqueuedFunctorWrapper
    {
    public:

        EnqueuedFunctorWrapper(ArenaHandler& arenaHandler, F& f) : m_arenaHandler(arenaHandler), m_functor(f) { }

        void operator()()
        {
            m_arenaHandler.m_numEnqueuedTasks--;
            m_functor();            
        }

    private:

        ArenaHandler& m_arenaHandler;
        F m_functor;
    };

};

/**
 * This class represents an ArenaHandler for root devices
 */
class RootDevArenaHandler : public ArenaHandler
{
public:

    /**
     * Constructor
     * @param uiNumSubdevComputeUnits   number of computing units in the sub-device
     * @param uiNumTotalComputeUnits	number of total computing units in the device
     * @param taskExecutor              a reference to the TBBTaskExecutor															
     */
    RootDevArenaHandler(unsigned int uiNumSubdevComputeUnits, unsigned int uiNumTotalComputeUnits, TBBTaskExecutor& taskExecutor) :
      ArenaHandler(uiNumSubdevComputeUnits, uiNumTotalComputeUnits, taskExecutor), m_devArenaObserver(m_arena, taskExecutor)
      {
          Init(&m_devArenaObserver);
      }

private:

    DevArenaObserver m_devArenaObserver;
    
};

/**
 * This class represents an ArenaHandler for sub-devices
 */ 
class SubdevArenaHandler : public ArenaHandler
{
public:

    /**
     * Constructor
     * @param uiNumSubdevComputeUnits   number of computing units in the sub-device
     * @param uiNumTotalComputeUnits	number of total computing units in the device
     * @param taskExecutor              a reference to the TBBTaskExecutor															
     * @param pLegalCores               an array of the core IDs which it is legal for the worker threads to affinitize to (size of the array is uiNumSubdevComputeUnits)
     * @param observer                  the IAffinityChangeObserver that observers changes in the affinity of worker threads																		
     */
    SubdevArenaHandler(unsigned int uiNumSubdevComputeUnits, unsigned int uiNumTotalComputeUnits, TBBTaskExecutor& taskExecutor, const unsigned int* pLegalCores, 
        IAffinityChangeObserver& observer);

    /**
     * Destructor
     */
    ~SubdevArenaHandler()
    {
        // stop observing before the observer is destroyed
        m_subdevArenaObserver.observe(false);
    }

    /**
     * @return the internal command list of this ArenaHandler
     */
    base_command_list& GetInternalCommandList() { return *m_pInternalCmdList; }

private:

    Intel::OpenCL::Utils::SharedPtr<base_command_list> m_pInternalCmdList;
    SubdevArenaObserver m_subdevArenaObserver;

};

template <class F>
void ArenaHandler::Enqueue(F& f)
{
    m_numEnqueuedTasks++;
    m_arena.enqueue(EnqueuedFunctorWrapper<F>(*this, f));
}

}}}
