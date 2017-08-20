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

// means config.h

#ifdef DEVICE_NATIVE
#ifdef __OMP_EXECUTOR__

#include "omp_executor.h"

//OMP parameters
//#define OMP_MULTI_ITERATIONS_PER_THREAD 

#include <cassert>
#ifdef WIN32
#include <stdafx.h>
#include <Windows.h>
#endif
#include <cl_sys_defines.h>
#include <cl_sys_info.h>
#include "cl_shared_ptr.hpp"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::TaskExecutor;

#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

bool OMP_TlsManager::m_object_exists = false;
THREAD_LOCAL OMP_PerActiveThreadData* Intel::OpenCL::TaskExecutor::OMP_TlsManager::m_CurrentThreadGlobalID = nullptr;

namespace Intel { namespace OpenCL { namespace TaskExecutor {

void RegisterReleaseSchedulerForMasterThread();

// global OMP objects
unsigned int gOMPWorker_threads = 0;
volatile bool gOMPIsExiting = false;


static void parallel_execute(	omp_command_list& cmdList,
								const size_t       dims[],          // MAX_WORK_DIM size
								unsigned int       dims_size,
								ITaskSet&          task )
{
	
	
	OMPTaskExecutor& te = cmdList.GetTaskExecutor();
	ITaskExecutorObserver* pObserver = (dynamic_cast< OMPTEDevice* >(cmdList.GetDevice().GetPtr()))->getObserver();
#ifdef OMP_MULTI_ITERATIONS_PER_THREAD
	//implementation of parralel for iterations as minimum between 1)#work groups and 2)#working threads
	int totalIterations = 1;
	for (int i = 0; i < dims_size; i++)
	{
		totalIterations *= dims[i];
	}

	int workingThreads = te.GetMaxNumOfConcurrentThreads();
	int perWorkerBase = totalIterations / workingThreads;
	int workersWithExtraIndex = totalIterations % workingThreads;
	int workers = min(totalIterations, workingThreads);

	switch ( dims_size )
    {
        case 1:
			#pragma omp parallel
			{
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor(pObserver);
				assert(tls);
				assert(tls->user_tls);
				void* user_local = task.AttachToThread(tls->user_tls, 1, nullptr, nullptr);
				#pragma omp for 
				for (unsigned int i = 0; i < workers; i++)
				{
					unsigned int firstIteration = i * (perWorkerBase + 1);
					unsigned int lastIteration = firstIteration + perWorkerBase;
					if (i >= workersWithExtraIndex) //current worker doesn't have 1 extra work group
					{
						firstIteration -= i - workersWithExtraIndex; //fix first iteration due to lower indices workers without extra work group
						lastIteration = firstIteration + perWorkerBase - 1; //now worker has one work group less to execute (no extra)
					}
					
					for (int j = firstIteration; j <= lastIteration; j++) //execute all worker work groups
					{
						task.ExecuteIteration(j, 0, 0, user_local); 
					}
				}
				task.DetachFromThread(user_local);
			}
            break;

		case 2:
			#pragma omp parallel
			{
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor(pObserver);
				assert(tls);
				assert(tls->user_tls);
				void* user_local = task.AttachToThread(tls->user_tls, 1, nullptr, nullptr);
				#pragma omp for 
				for (unsigned int i = 0; i < workers; i++)
				{
					unsigned int firstIteration = i * (perWorkerBase + 1);
					unsigned int lastIteration = firstIteration + perWorkerBase;
					if (i >= workersWithExtraIndex)
					{
						firstIteration -= i - workersWithExtraIndex;
						lastIteration = firstIteration + perWorkerBase - 1;
					}

					//transform iteration coordinates from 1d to 2d
					unsigned int x1 = firstIteration % dims[0];
					unsigned int y1 = firstIteration / dims[0];

					for (int j = firstIteration; j <= lastIteration; j++)
					{
						x1++;
						if (x1 == dims[0]) 
						{
							y1++;
							x1 = 0;
						}
						assert(y1 < dims[1]);
						task.ExecuteIteration(x1, y1, 0, user_local); 
					}
				}
				task.DetachFromThread(user_local);
			}
            break;

		case 3:
			#pragma omp parallel
			{
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor(pObserver);
				assert(tls);
				assert(tls->user_tls);
				void* user_local = task.AttachToThread(tls->user_tls, 1, nullptr, nullptr);
				#pragma omp for 
				for (unsigned int i = 0; i < workers; i++)
				{
					unsigned int firstIteration = i * (perWorkerBase + 1);
					unsigned int lastIteration = firstIteration + perWorkerBase;
					if (i >= workersWithExtraIndex)
					{
						firstIteration -= i - workersWithExtraIndex;
						lastIteration = firstIteration + perWorkerBase - 1;
					}
					
					//transform iteration coordinates from 1d to 3d
					unsigned int x1 = firstIteration % dims[0];
					unsigned int y1 = (firstIteration % dims[0]*dims[1]) / dims[0];
					unsigned int z1 = firstIteration / (dims[0]*dims[1]);

					for (int j = firstIteration; j <= lastIteration; j++)
					{
						x1++;
						if (x1 == dims[0]) 
						{
							x1 = 0;
							y1++;
							if (y1 == dims[1])
							{
								y1 = 0;
								z1++;
							}
						}
						assert(z1 < dims[2]);
						task.ExecuteIteration(x1, y1, z1, user_local); 
					}
				}
				task.DetachFromThread(user_local);
			}
            break;
        default:
            assert( (dims_size != 0) && (dims_size <= MAX_WORK_DIM) );
			break;
    }
#else //OMP_MULTI_ITERATIONS_PER_THREAD not defined
	//implementation of parralel for iterations as #work groups
	switch ( dims_size )
    {
        case 1:
			#pragma omp parallel
			{
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor(pObserver);
				assert(tls);
				assert(tls->user_tls);
				void* user_local = task.AttachToThread(tls->user_tls, 1, nullptr, nullptr);
				#pragma omp for 
				for (unsigned int i = 0; i < dims[0]; i++)
				{
					unsigned int x = i;
					task.ExecuteIteration(x, 0, 0, user_local); 
				}
				task.DetachFromThread(user_local);
			}
            break;

        case 2:
			#pragma omp parallel 
			{
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor(pObserver);
				assert(tls);
				assert(tls->user_tls);
				void* user_local = task.AttachToThread(tls->user_tls, 1, nullptr, nullptr);
				#pragma omp for 
				for (unsigned int i=0; i<dims[0]*dims[1]; i++)
				{
					unsigned int x = i % dims[0];
					unsigned int y = i / dims[0];
					task.ExecuteIteration(x, y, 0, user_local); 
				}
				task.DetachFromThread(user_local);
			}
            break;

        case 3:
			#pragma omp parallel 
			{
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor(pObserver);
				assert(tls);
				assert(tls->user_tls);
				void* user_local = task.AttachToThread(tls->user_tls, 1, nullptr, nullptr);
				#pragma omp for 
				for (unsigned int i=0; i<dims[0]*dims[1]*dims[2]; i++)
				{
					unsigned int x = i % dims[0];
					unsigned int y = (i % dims[0]*dims[1]) / dims[0];
					unsigned int z = i / (dims[0]*dims[1]);
					task.ExecuteIteration(x, y, z, user_local); 
				}
				task.DetachFromThread(user_local);
			}
            break;
        default:
            assert( (dims_size != 0) && (dims_size <= MAX_WORK_DIM) );
			break;
    }
#endif
}

static bool execute_command(const SharedPtr<ITaskBase>& pCmd, omp_command_list& cmdList)
{
	bool runNextCommand = true;

	if ( pCmd->IsTaskSet() )
	{
		ITaskSet* pTask = static_cast<ITaskSet*>(pCmd.GetPtr());
		size_t dim[MAX_WORK_DIM];
		unsigned int dimCount;
		int res = pTask->Init(dim, dimCount);
		assert(res==0 && "Init Failed");
		if (res != 0)
		{
			pTask->Finish(FINISH_INIT_FAILED);
			return false;
		}

        // fork execution
		parallel_execute( cmdList, dim, dimCount, *pTask );
        // join execution

		runNextCommand = pTask->Finish(FINISH_COMPLETED);
	}
	else
	{
        ITask* pTask = static_cast<ITask*>(pCmd.GetPtr());
		runNextCommand = pTask->Execute();
	}

	runNextCommand &= !pCmd->CompleteAndCheckSyncPoint();
	return runNextCommand;
}

static void Terminate()
{
    gOMPIsExiting = true;
}

/////////////// TaskExecutor //////////////////////

OMPTaskExecutor::OMPTaskExecutor()
{
}

int	OMPTaskExecutor::Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData)
{
	if ( 0 != gOMPWorker_threads )
	{
		assert(0 && "OMPExecutor already initialized");
		return gOMPWorker_threads;
	}

	m_pGPAData = pGPAData;

	gOMPWorker_threads = uiNumThreads;
	
	if (uiNumThreads == TE_AUTO_THREADS)
	{
		gOMPWorker_threads = GetNumberOfProcessors();
	}
	
	InitialOMP();

	m_TlsManager.Init(gOMPWorker_threads);

	return gOMPWorker_threads;
}

void OMPTaskExecutor::Finalize() 
{ 
	gOMPWorker_threads = 0; 
}

SharedPtr<ITEDevice>
OMPTaskExecutor::CreateRootDevice( const RootDeviceCreationParam& device_desc, void* user_data, ITaskExecutorObserver* my_observer)
{
	assert(gOMPWorker_threads && "OMP executor should be initialized first");

    RootDeviceCreationParam device( device_desc );

    if (TE_AUTO_THREADS == device.uiThreadsPerLevel[0])
    {
        device.uiThreadsPerLevel[0] = gOMPWorker_threads;
    }

    // check params
    if (1 != device.uiNumOfLevels)
    {
		assert(0 && "Currently Only one level device can create with OMP");
        return nullptr;
    }
	
    if ((device.uiThreadsPerLevel[0] == 0) || (device.uiThreadsPerLevel[0] > gOMPWorker_threads))
    {
        assert(0 && "Too many threads requested - above maximum configured" );
        return nullptr;
    }

    // Create root device
    SharedPtr<OMPTEDevice> root = OMPTEDevice::Allocate( device, user_data, my_observer, *this );

	return root;
}

unsigned int OMPTaskExecutor::GetMaxNumOfConcurrentThreads() const 
{ 
	return gOMPWorker_threads; 
}

ITaskExecutor::DeviceHandleStruct OMPTaskExecutor::GetCurrentDevice() const
{
	OMP_PerActiveThreadData* tls = m_TlsManager.GetCurrentThreadDescriptor();
    
    if (nullptr == tls)
    {
        return DeviceHandleStruct();
    }
    else
    {
		return DeviceHandleStruct( tls->device, (nullptr != tls->device) ? tls->device->GetUserData() : nullptr );
	}
}

void OMPTaskExecutor::InitialOMP()
{
	assert(gOMPWorker_threads > 0 && "gOMPWorker_threads must be greater than 0");
	char numThreadsStr[sizeof(int) * 8 + 1] = {0};
	int n = sprintf(numThreadsStr, "%d", gOMPWorker_threads);
	assert(n > 0 && "sprintf(numThreadsStr, %d, gOMPWorker_threads) failed");
    setenv("KMP_NUM_THREADS", numThreadsStr, 1);
}


/////////////// TlsManager //////////////////////

OMP_TlsManager::OMP_TlsManager() : m_uiNumberOfStaticEntries(0), m_ThreadDataArray(nullptr)
{
}

OMP_TlsManager::~OMP_TlsManager()
{
	m_object_exists = false;
	if (nullptr != m_ThreadDataArray)
	{
		delete[] m_ThreadDataArray;
	}
}

bool OMP_TlsManager::Init( unsigned int uiNumberOfThreads )
{
	assert( false == m_object_exists );

	if (m_object_exists)
	{
		return false;
	}

	m_object_exists = true;
	m_uiNumberOfStaticEntries = uiNumberOfThreads;

	if (uiNumberOfThreads > 0)
	{
		m_ThreadDataArray = new OMP_PerActiveThreadData[uiNumberOfThreads];
	}
			
	return true;
}

OMP_PerActiveThreadData* OMP_TlsManager::RegisterCurrentThread() const
{
	assert( m_object_exists );
	OMP_PerActiveThreadData* tThreadData = nullptr;
	if (!m_object_exists)
	{
		return nullptr;
	}

	unsigned int threadId = omp_get_thread_num();
	if (threadId >= m_uiNumberOfStaticEntries)
	{
		tThreadData = new OMP_PerActiveThreadData;
		if (nullptr == tThreadData)
		{
			return nullptr;
		}
	}
	else
	{
		tThreadData = &(m_ThreadDataArray[threadId]);
	}

	m_CurrentThreadGlobalID = tThreadData;

	return tThreadData;
}

OMP_PerActiveThreadData* OMP_TlsManager::GetCurrentThreadDescriptor() const
{
	return (m_CurrentThreadGlobalID) ? m_CurrentThreadGlobalID : RegisterCurrentThread();
}

OMP_PerActiveThreadData* OMP_TlsManager::GetCurrentThreadDescriptor(ITaskExecutorObserver* pObserver) const
{
	assert(pObserver);
	if (nullptr == m_CurrentThreadGlobalID)
	{
		RegisterCurrentThread();
		m_CurrentThreadGlobalID->user_tls = pObserver->OnThreadEntry();
	}
	else if (nullptr == m_CurrentThreadGlobalID->user_tls)
	{
		m_CurrentThreadGlobalID->user_tls = pObserver->OnThreadEntry();
	}

	return m_CurrentThreadGlobalID; 
}


/////////////// TEDevice //////////////////////

OMPTEDevice::OMPTEDevice(const RootDeviceCreationParam device_desc, void* user_data, ITaskExecutorObserver* observer, OMPTaskExecutor& taskExecutor, const Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>& parent)
: m_userData(user_data),m_observer(observer),m_taskExecutor(taskExecutor)
{
}

SharedPtr<ITaskList> OMPTEDevice::CreateTaskList(const CommandListCreationParam& param )
{
	SharedPtr<ITaskList> pList = nullptr;
	// Support only TE_CMD_LIST_IMMEDIATE (which is the in order queue of MIC device)
	assert(TE_CMD_LIST_IMMEDIATE == param.cmdListType && "OMPTEDevice::CreateTaskList support only TE_CMD_LIST_IMMEDIATE (which is the in order queue of MIC device)");
	pList = omp_command_list::Allocate(m_taskExecutor, this, &param);
	assert(pList && "omp_command_list::Allocate failed");
	return pList;
}

omp_command_list::omp_command_list(OMPTaskExecutor& pOMPExec, const SharedPtr<OMPTEDevice>& device, const CommandListCreationParam* param) : m_pOMPExecutor(pOMPExec), m_pDevice(device)
{
}

unsigned int omp_command_list::Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask)
{
    execute_command(pTask, *this);
	return 0;
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
#endif //__OMP_EXECUTOR__
#endif //DEVICE_NATIVE
