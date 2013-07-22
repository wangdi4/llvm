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
#define OMP_SCHED "guided"
#define KMP_AFFINITY "granularity=fine,scatter"
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

#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

bool Intel::OpenCL::TaskExecutor::OMP_TlsManager::m_object_exists = false;

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
			#pragma omp parallel for 
			for (unsigned int i = 0; i < workers; i++)
			{
				unsigned int firstIteration = i * (perWorkerBase + 1);
				unsigned int lastIteration = firstIteration + perWorkerBase;
				if (i >= workersWithExtraIndex) //current worker doesn't have 1 extra work group
				{
					firstIteration -= i - workersWithExtraIndex; //fix first iteration due to lower indices workers without extra work group
					lastIteration = firstIteration + perWorkerBase - 1; //now worker has one work group less to execute (no extra)
				}
				int numIterations = lastIteration - firstIteration + 1;

				size_t GID1[1] = {firstIteration};
				size_t GID2[1] = {lastIteration};

				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor();
				if (NULL == tls->user_tls)
				{
					tls->user_tls = cmdList.GetDevice()->getObserver()->OnThreadEntry();
				}
				void* user_local = task.AttachToThread(tls->user_tls, numIterations, GID1, GID2);
				for (int j = firstIteration; j <= lastIteration; j++) //execute all worker work groups
				{
					task.ExecuteIteration(j, 0, 0, user_local); 
				}
			}
			break;
		
		case 2:
			#pragma omp parallel for 
			for (unsigned int i = 0; i < workers; i++)
			{
				unsigned int firstIteration = i * (perWorkerBase + 1);
				unsigned int lastIteration = firstIteration + perWorkerBase;
				if (i >= workersWithExtraIndex)
				{
					firstIteration -= i - workersWithExtraIndex;
					lastIteration = firstIteration + perWorkerBase - 1;
				}
				int numIterations = lastIteration - firstIteration + 1;

				//transform iteration coordinates from 1d to 2d
				unsigned int x1 = firstIteration % dims[0];
				unsigned int y1 = firstIteration / dims[0];

				unsigned int x2 = lastIteration % dims[0];
				unsigned int y2 = lastIteration / dims[0];

				size_t GID1[2] = {x1,y1};
				size_t GID2[2] = {x2,y2};

				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor();
				if (NULL == tls->user_tls)
				{
					tls->user_tls = cmdList.GetDevice()->getObserver()->OnThreadEntry();
				}
				void* user_local = task.AttachToThread(tls->user_tls, numIterations, GID1, GID2);
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
			break;

		case 3:
			#pragma omp parallel for 
			for (unsigned int i = 0; i < workers; i++)
			{
				unsigned int firstIteration = i * (perWorkerBase + 1);
				unsigned int lastIteration = firstIteration + perWorkerBase;
				if (i >= workersWithExtraIndex)
				{
					firstIteration -= i - workersWithExtraIndex;
					lastIteration = firstIteration + perWorkerBase - 1;
				}
				int numIterations = lastIteration - firstIteration + 1;
				
				//transform iteration coordinates from 1d to 3d
				unsigned int x1 = firstIteration % dims[0];
				unsigned int y1 = (firstIteration % dims[0]*dims[1]) / dims[0];
				unsigned int z1 = firstIteration / (dims[0]*dims[1]);

				unsigned int x2 = lastIteration % dims[0];
				unsigned int y2 = (lastIteration % dims[0]*dims[1]) / dims[0];
				unsigned int z2 = lastIteration / (dims[0]*dims[1]);

				size_t GID1[3] = {x1,y1,z1};
				size_t GID2[3] = {x2,y2,z2};

				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor();
				if (NULL == tls->user_tls)
				{
					tls->user_tls = cmdList.GetDevice()->getObserver()->OnThreadEntry();
				}
				void* user_local = task.AttachToThread(tls->user_tls, numIterations, GID1, GID2);
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
			#pragma omp parallel for 
            for (unsigned int i=0; i<dims[0]; i++)
			{
				unsigned int x = i;
				size_t GID1[1] = {x};
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor();
				if (NULL == tls->user_tls)
				{
					tls->user_tls = cmdList.GetDevice()->getObserver()->OnThreadEntry();
				}
				void* user_local = task.AttachToThread(tls->user_tls, 1, GID1, GID1);
				task.ExecuteIteration(x, 0, 0, user_local); 
			}
            break;

        case 2:
			#pragma omp parallel for 
            for (unsigned int i=0; i<dims[0]*dims[1]; i++)
			{
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor();
				if (NULL == tls->user_tls)
				{
					tls->user_tls = cmdList.GetDevice()->getObserver()->OnThreadEntry();
				}
				unsigned int x = i % dims[0];
				unsigned int y = i / dims[0];
				size_t GID2[2] = {x,y};
				void* user_local = task.AttachToThread(tls->user_tls, 1, GID2, GID2);
				task.ExecuteIteration(x, y, 0, user_local); 
			}
            break;

        case 3:
			#pragma omp parallel for 
			for (unsigned int i=0; i<dims[0]*dims[1]*dims[2]; i++)
			{
				OMP_PerActiveThreadData* tls = te.GetTlsManager().GetCurrentThreadDescriptor();
				if (NULL == tls->user_tls)
				{
					tls->user_tls = cmdList.GetDevice()->getObserver()->OnThreadEntry();
				}
				unsigned int x = i % dims[0];
				unsigned int y = (i % dims[0]*dims[1]) / dims[0];
				unsigned int z = i / (dims[0]*dims[1]);
				size_t GID3[3] = {x,y,z};
				void* user_local = task.AttachToThread(tls->user_tls, 1, GID3, GID3);
				task.ExecuteIteration(x, y, z, user_local); 
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
    ITaskBase* cmd = pCmd.GetPtr();

	if ( cmd->IsTaskSet() )
	{
		ITaskSet* pTask = static_cast<ITaskSet*>(cmd);
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
        ITask* pCmd = static_cast<ITask*>(cmd);
		runNextCommand = pCmd->Execute();
	}

	runNextCommand &= !cmd->CompleteAndCheckSyncPoint();
	return runNextCommand;
}

static void Terminate()
{
    gOMPIsExiting = true;
}


void omp_executor_task::operator()()
{    
	assert(m_list);
    assert(m_pTask);

    execute_command(m_pTask, *m_list);
}


/////////////// TaskExecutor //////////////////////

OMPTaskExecutor::OMPTaskExecutor()
{
}

OMPTaskExecutor::~OMPTaskExecutor()
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
		gOMPWorker_threads = Intel::OpenCL::Utils::GetNumberOfProcessors();
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
        return NULL;
    }
	
    if ((device.uiThreadsPerLevel[0] == 0) || (device.uiThreadsPerLevel[0] > gOMPWorker_threads))
    {
        assert( false && "Too many threads requested - above maximum configured" );
        return NULL;
    }

    // Create root device
    SharedPtr<OMPTEDevice> root = OMPTEDevice::Allocate( device, user_data, my_observer, *this );

	return root;
}

unsigned int OMPTaskExecutor::GetMaxNumOfConcurrentThreads() const
{
	return gOMPWorker_threads;
}

ocl_gpa_data* OMPTaskExecutor::GetGPAData() const
{
	return m_pGPAData;
}

ITaskExecutor::DeviceHandleStruct OMPTaskExecutor::GetCurrentDevice() const
{
	OMP_PerActiveThreadData* tls = m_TlsManager.GetCurrentThreadDescriptor();
    
    if (NULL == tls)
    {
        return DeviceHandleStruct();
    }
    else
    {
		return DeviceHandleStruct( tls->device, (NULL != tls->device) ? tls->device->GetUserData() : NULL );
	}
}

bool OMPTaskExecutor::IsMaster() const
{
	return (omp_get_thread_num() == 0) ? true : false;
}

unsigned int OMPTaskExecutor::GetPosition( unsigned int level ) const
{
	if (level != 0)
	{
		return TE_UNKNOWN;
	}

	return omp_get_thread_num();
}

void OMPTaskExecutor::InitialOMP()
{
	setenv("OMP_SCHEDULE", OMP_SCHED, 1);
	 setenv("KMP_AFFINITY",KMP_AFFINITY, 1);
    omp_set_num_threads(gOMPWorker_threads);
}


/////////////// TlsManager //////////////////////

OMP_TlsManager::OMP_TlsManager(): m_uiNumberOfStaticEntries(0),m_ThreadDataArray(NULL){}

OMP_TlsManager::~OMP_TlsManager()
{
	m_object_exists = false;
	if (NULL != m_ThreadDataArray)
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

OMP_PerActiveThreadData* OMP_TlsManager::RegisterCurrentThread()
{
	assert( m_object_exists );
	if (!m_object_exists)
	{
		return NULL;
	}

	unsigned int threadId = omp_get_thread_num();
	if (threadId >= m_uiNumberOfStaticEntries)
	{
		return NULL;
	}
	m_ThreadDataArray[threadId].reset();
	return &(m_ThreadDataArray[threadId]);
}

void  OMP_TlsManager::UnregisterCurrentThread()
{
	assert( m_object_exists );
    if (!m_object_exists)
    {
        return;
    }
	unsigned int threadId = omp_get_thread_num();
	if (threadId >= m_uiNumberOfStaticEntries)
	{
		return;
	}
	m_ThreadDataArray[threadId].reset();
}

OMP_PerActiveThreadData* OMP_TlsManager::GetCurrentThreadDescriptor() const
{
	unsigned int threadId = omp_get_thread_num();
	if (threadId >= m_uiNumberOfStaticEntries)
	{
		return NULL;
	}
	return &(m_ThreadDataArray[threadId]);
}

OMP_PerActiveThreadData* OMP_TlsManager::RegisterAndGetCurrentThreadDescriptor()
{
	OMP_PerActiveThreadData* d = GetCurrentThreadDescriptor();
	return (NULL != d) ? d : RegisterCurrentThread();
}


/////////////// TEDevice //////////////////////

Intel::OpenCL::Utils::SharedPtr<ITaskList> OMPTEDevice::CreateTaskList(const CommandListCreationParam& param )
{
	SharedPtr<ITaskList> pList = NULL;
	pList = omp_command_list::Allocate(m_taskExecutor, this, &param);
	// TODO check whether list type fits
	/* 
    switch ( param.cmdListType )
    {
        case TE_CMD_LIST_IMMEDIATE:
            pList = omp_command_list::Allocate(m_taskExecutor, this, &param);
            break;

        default:
            assert( false && "Trying to create OMPTaskExecutor Command list which is not immediate");
    }
	*/
	return pList;
}

bool OMPTEDevice::Execute(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask )
{
	return false; //TODO local task list?
}

unsigned int omp_command_list::LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>* pTask )
{
	assert(NULL != pTask);
	assert(true == blocking);

	omp_executor_task functor(this, *pTask);
	functor();
	return 0;
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
#endif //__OMP_EXECUTOR__
#endif //DEVICE_NATIVE