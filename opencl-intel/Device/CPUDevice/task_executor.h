#pragma once

#include "OCL_RT.h"

#include "cl_synch_objects.h"

#include <list>

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class WGExecutor;

struct STaskDescriptor
{
	// Kernel execution information
	SKernelInfo		sKernelParam;
	// Working dimention information
	SWorkDim		sWorkingDim;
};

// Defenition of task complition notification function
typedef void*	TTaskHandle;
typedef void	TTaskNotifier(TTaskHandle hTask, STaskDescriptor* psTaskDescriptor, void* pData);
typedef void	TFunctionNotifier(TTaskHandle hTask, void* pParams, size_t size, void* pData);

enum	ETERetCode
{
	TE_SUCCESS				= 0,
	TE_ERROR				= 0x80000000,
	TE_ALREADY_INITILIZED,
	TE_OUT_OF_RANGE,
	TE_OUT_OF_MEMORY,
	TE_RESOURCE_EXHAUSTED
};

class TaskExecutor
{
public:
	TaskExecutor();
	virtual ~TaskExecutor();

	// Init task executor
	ETERetCode	Init(unsigned int uiNumThreads);

	// Wait for all task completion and then release Task Executor resourses
	void Close();

	// Execute a Task comprosed of kernels
	ETERetCode	ExecuteKernel(STaskDescriptor* pTaskDesc,
						TTaskNotifier* pfnNotify, void* pData,
						TTaskHandle* pDepList, unsigned int uiDepListCount,
						TTaskHandle* pTask);

	// Execute single Function
	ETERetCode	ExecuteFunction(const void* pfnFunction, void* pParams, size_t stSize,
						TFunctionNotifier* pfnNotify, void* pData,
						TTaskHandle* pDepList, unsigned int uiDepListCount,
						TTaskHandle* pTask);


	// Free previously allocated task reference (ExecuteTask/ExecuteFunction)
	ETERetCode FreeTaskHandle(TTaskHandle pTask);

protected:
	//  Private types defenition
	typedef	std::list<WGExecutor*>	TWGExecutorList;

	// Defines running task and function instances
	struct SRunningTask
	{
		unsigned int			uiTaskId;		// Task Identifier to be used for WG executor
		TaskExecutor*			pTE;			// A pointer to Task Executor object that executes the task
		STaskDescriptor*		psTaskDesc;		// A pointer to task descripor, is valid during task execution
		SWGinfo*				psWGInfo;		// A pointer to Working groups information as a number of working groups

		// Notification parameters
		TTaskNotifier*			pfnNotify;		// A pointer to notification function
		void*					pData;			// A pointer to Data to be passed upon notification
		void*					pScratch;		// A scratch data required by XNTask library
	};

	struct SRunningFunction
	{
		unsigned int			uiTaskId;		// Task Identifier to be used for WG executor
		TaskExecutor*			pTE;			// A pointer to Task Executor object that executes the task
		void*					pParams;		// A pointer to function parameters buffer
		size_t					stSize;			// Size of the parameter buffer
		// Notification parameters
		TFunctionNotifier*		pfnNotify;		// A pointer to notification function
		void*					pData;			// A pointer to Data to be passed upon notification
		void*					pScratch;		// A scratch data required by XNTask library
	};

	// Private variables
	unsigned int					m_uiHWThreads;
	unsigned int					m_uiNumWorkingThreads;

	OclMutex						m_muWGExecList;		// Mutex that guards 
	TWGExecutorList					m_lWGExecutors;		// A list of free WG executors
	volatile long					m_lLastTaskId;

	// Static functions
	static void TaskExecutionRoutine(void* _Task, unsigned int uiIndex, unsigned int uiSize);
	static void TaskCompleted(TTaskHandle hTask, void* _Task);
	static void	FunctionCompleted(TTaskHandle hTask, void* _Function);
};

}}}