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

/*
*
* File omp_executor.h
*		Implements interface required for task execution on XNTask sub-system
*
*/
#pragma once

#ifdef DEVICE_NATIVE
#ifdef __OMP_EXECUTOR__

#include "task_executor.h"
#include <omp.h>

#include "cl_synch_objects.h"
#include "cl_dynamic_lib.h"
#include "cl_shared_ptr.h"
#include "cl_synch_objects.h"

using Intel::OpenCL::Utils::SharedPtr;
using Intel::OpenCL::Utils::AtomicPointer;
using Intel::OpenCL::Utils::OclMutex;

namespace Intel { namespace OpenCL { namespace TaskExecutor {


    class OMPTaskExecutor;
	 /**
     * a global flag indicating whether the program has called function exit
     */
	class OMPTEDevice : public ITEDevice
	{
	public:
		static Intel::OpenCL::Utils::SharedPtr<OMPTEDevice> Allocate(
											const RootDeviceCreationParam& device_desc, void* user_data, ITaskExecutorObserver* observer, 
											OMPTaskExecutor& taskExecutor, const Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>& parent = NULL )
		{
			return new OMPTEDevice(device_desc, user_data, observer, taskExecutor, parent );
		}

		virtual Intel::OpenCL::Utils::SharedPtr<ITEDevice> CreateSubDevice( unsigned int uiNumSubdevComputeUnits, void* user_handle = NULL ){return NULL; }

		virtual void ResetObserver(){m_observer = NULL;}
		
		virtual void ShutDown(){}
		
		virtual Intel::OpenCL::Utils::SharedPtr<ITaskList> CreateTaskList(const CommandListCreationParam& param );

		virtual bool Execute(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask );

		virtual te_wait_result WaitForCompletion(ITaskBase * pTask){return TE_WAIT_NOT_SUPPORTED; }

		virtual void WaitUntilEmpty(){return;}

		ITaskExecutorObserver* getObserver(){ return m_observer; }

		void* GetUserData() const { return m_userData; }

	protected:
		void*						m_userData;
		ITaskExecutorObserver*		m_observer;
		OMPTaskExecutor&			m_taskExecutor;
		
	private:
		OMPTEDevice(const RootDeviceCreationParam device_desc, void* user_data, ITaskExecutorObserver* observer, 
					OMPTaskExecutor& taskExecutor, const Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>& parent = NULL )
					:m_userData(user_data),m_observer(observer),m_taskExecutor(taskExecutor){}
		OMPTEDevice(OMPTEDevice&);
	};

	struct OMP_PerActiveThreadData
	{
		OMPTEDevice* device;
		void* user_tls;
		
		void reset()
		{
			device = NULL;
			user_tls = NULL;
		}
		
		OMP_PerActiveThreadData():device(NULL),user_tls(NULL){}
	};

	class OMP_TlsManager
	{
	public:
		OMP_TlsManager();
		
		~OMP_TlsManager();

		bool Init( unsigned int uiNumberOfThreads );
		
		void  UnregisterCurrentThread();

		OMP_PerActiveThreadData* RegisterCurrentThread();

		OMP_PerActiveThreadData* GetCurrentThreadDescriptor() const;
		
		OMP_PerActiveThreadData* RegisterAndGetCurrentThreadDescriptor();

	private:
		unsigned int						m_uiNumberOfStaticEntries;
		OMP_PerActiveThreadData*			m_ThreadDataArray;
		static bool                         m_object_exists;

		// do not implement
		OMP_TlsManager(const OMP_TlsManager& o);
		OMP_TlsManager& operator=( const OMP_TlsManager& o );
	};

	class OMPTaskExecutor : public ITaskExecutor
	{
	public:
		OMPTaskExecutor();
		
		virtual ~OMPTaskExecutor();
		
		int	Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData);
		
		void Finalize();
		
		Intel::OpenCL::Utils::SharedPtr<ITEDevice> CreateRootDevice( const RootDeviceCreationParam& device_desc = RootDeviceCreationParam(),  
			void* user_data = NULL, ITaskExecutorObserver* my_observer = NULL );

		ocl_gpa_data* GetGPAData() const;

		unsigned int GetMaxNumOfConcurrentThreads() const;

		virtual DeviceHandleStruct GetCurrentDevice() const;
		
		virtual bool IsMaster() const;
		
		virtual unsigned int GetPosition( unsigned int level = 0 ) const;

		OMP_TlsManager& GetTlsManager(){return m_TlsManager;}
	
	protected:
		void InitialOMP();

		OMP_TlsManager m_TlsManager;

	private:
		// do not implement
		OMPTaskExecutor(const OMPTaskExecutor&);
		OMPTaskExecutor& operator=(const OMPTaskExecutor&);
	};

	class omp_command_list : public ITaskList
	{
	public:
		PREPARE_SHARED_PTR(omp_command_list)

		~omp_command_list()
		{
			//TODO anything to delete?
			return;
		}

		static SharedPtr<omp_command_list> Allocate( OMPTaskExecutor& pOMPExec,
													const Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>& device, 
													const CommandListCreationParam* param = NULL )
		{
			return new omp_command_list(pOMPExec, device, param );
		}

		OMPTaskExecutor& GetTaskExecutor(){return m_pOMPExecutor;}
		
		Intel::OpenCL::Utils::SharedPtr<OMPTEDevice> GetDevice(){return m_pDevice;}
		
		unsigned int Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask)
		{
			return LaunchExecutorTask(true, &pTask);
		}

		te_wait_result WaitForCompletion(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTaskToWait) { return TE_WAIT_NOT_SUPPORTED; }

		bool Flush() { return true; }

		friend class omp_executor_task;

	private:
		virtual unsigned int LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>* pTask = NULL);

		omp_command_list(const omp_command_list& l);
		omp_command_list();
		omp_command_list(OMPTaskExecutor& pOMPExec, const Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>& device, const CommandListCreationParam* param = NULL) 
			: m_pOMPExecutor(pOMPExec), m_pDevice(device){}
	
		OMPTaskExecutor&										m_pOMPExecutor;
		Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>			m_pDevice;
	};

    class omp_executor_task
    {
    public:
        omp_executor_task(omp_command_list* list, const SharedPtr<ITaskBase>& pTask ):m_list(list), m_pTask( pTask ) {}

        void operator()();

    protected:
        omp_command_list*			m_list;
        SharedPtr<ITaskBase>        m_pTask;
    };

}}}
#endif // __OMP_EXECUTOR__
#endif // DEVICE_NATIVE