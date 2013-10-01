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
#include "cl_thread.h"

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

		virtual Intel::OpenCL::Utils::SharedPtr<ITEDevice> CreateSubDevice( unsigned int uiNumSubdevComputeUnits, void* user_handle = NULL ) { return NULL; };

		virtual void ResetObserver() { m_observer = NULL; };

		virtual void SetObserver(ITaskExecutorObserver* pObserver) { m_observer = pObserver; };

		virtual int GetConcurrency() const { return omp_in_parallel() ? omp_get_num_threads() : omp_get_max_threads(); };
		
		virtual void ShutDown() {};

		virtual void AttachMasterThread(void* user_tls) {};

		virtual void DetachMasterThread() {};
		
		virtual Intel::OpenCL::Utils::SharedPtr<ITaskList> CreateTaskList(const CommandListCreationParam& param );

		virtual Intel::OpenCL::Utils::SharedPtr<ITaskList> AllocateDefaultQueue(bool bIsProfilingEnabled) { return NULL; };

		virtual void ReleaseDefaultQueue() {};

		virtual queue_t GetDefaultQueue() { return NULL; };

		ITaskExecutorObserver* getObserver() { return m_observer; };

		void* GetUserData() const { return m_userData; };

	protected:
		void*						m_userData;
		ITaskExecutorObserver*		m_observer;
		OMPTaskExecutor&			m_taskExecutor;
		
	private:
		OMPTEDevice(const RootDeviceCreationParam device_desc, void* user_data, ITaskExecutorObserver* observer, 
					OMPTaskExecutor& taskExecutor, const Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>& parent = NULL );

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
		
		OMP_PerActiveThreadData() : device(NULL), user_tls(NULL) {}
	};

	class OMP_TlsManager
	{
	public:
		OMP_TlsManager();
		
		virtual ~OMP_TlsManager();

		bool Init( unsigned int uiNumberOfThreads );

		OMP_PerActiveThreadData* RegisterCurrentThread() const;

		OMP_PerActiveThreadData* GetCurrentThreadDescriptor() const;

		OMP_PerActiveThreadData* GetCurrentThreadDescriptor(ITaskExecutorObserver* pObserver) const;

	private:

		static THREAD_LOCAL OMP_PerActiveThreadData*    m_CurrentThreadGlobalID;

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
		
		virtual ~OMPTaskExecutor() {};
		
		int	Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData);
		
		void Finalize();
		
		Intel::OpenCL::Utils::SharedPtr<ITEDevice> CreateRootDevice( const RootDeviceCreationParam& device_desc = RootDeviceCreationParam(),  
			void* user_data = NULL, ITaskExecutorObserver* my_observer = NULL );

		virtual Intel::OpenCL::Utils::SharedPtr<ITaskGroup> CreateTaskGroup(const Intel::OpenCL::Utils::SharedPtr<ITEDevice>& device) { return NULL; };

		ocl_gpa_data* GetGPAData() const { return m_pGPAData; };

		unsigned int GetMaxNumOfConcurrentThreads() const;

		virtual DeviceHandleStruct GetCurrentDevice() const;
		
		virtual bool IsMaster() const { return (omp_get_thread_num() == 0) ? true : false; };
		
		virtual unsigned int GetPosition( unsigned int level = 0 ) const { return (0 == level) ? omp_get_thread_num() : TE_UNKNOWN; };

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

		static SharedPtr<omp_command_list> Allocate( OMPTaskExecutor& pOMPExec,
													const Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>& device, 
													const CommandListCreationParam* param = NULL )
		{
			return new omp_command_list(pOMPExec, device, param );
		}

		OMPTaskExecutor& GetTaskExecutor() { return m_pOMPExecutor; };
		
		Intel::OpenCL::Utils::SharedPtr<ITEDevice> GetDevice() { return m_pDevice; };
		Intel::OpenCL::Utils::ConstSharedPtr<ITEDevice> GetDevice() const { return (const ITEDevice*)m_pDevice.GetPtr(); };
		
		unsigned int Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask);

		// WaitForCompletion is not supported in immediate queue
		te_wait_result WaitForCompletion(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTaskToWait) { return TE_WAIT_NOT_SUPPORTED; };

		bool Flush() { return true; }

		virtual void Cancel() { /*TODO Implement Cancel for Shutdown support. */ };

		virtual void Launch(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask) {};

		virtual bool DoesSupportDeviceSideCommandEnqueue() const { return false; };

		virtual bool IsProfilingEnabled() const { return true; };

		virtual bool IsDefaultQueue() const { return false; };

		virtual Intel::OpenCL::Utils::SharedPtr<ITaskGroup> GetNDRangeChildrenTaskGroup() { return NULL; };

	private:

		omp_command_list(const omp_command_list& l);
		omp_command_list();
		omp_command_list(OMPTaskExecutor& pOMPExec, const Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>& device, const CommandListCreationParam* param = NULL); 
	
		OMPTaskExecutor&										m_pOMPExecutor;
		Intel::OpenCL::Utils::SharedPtr<OMPTEDevice>			m_pDevice;
	};

}}}
#endif // __OMP_EXECUTOR__
#endif // DEVICE_NATIVE