#ifndef ICLDevBackendExecutable_H
#define ICLDevBackendExecutable_H

#include "cl_device_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * This interface class responsible of code execution of the specified OCL work description
 */
class ICLDevBackendExecutable_
{
public:
    virtual ~ICLDevBackendExecutable_() {}

    /**
     * Prepares the current thread for execution (should be called before execution)
     *
     * @returns CL_DEV_SUCCESS in case of success; CL_DEV_ERROR_FAIL otherwise
     */
    //virtual cl_dev_err_code PrepareThreadContext() = 0;

    /**
     * Executes the specified work group
     *
     * @param workGroupID is OCL_MAX_DIM array which specifes the work group
     *  to execute
     *
     * @returns CL_DEV_SUCCESS in case of success; CL_DEV_ERROR_FAIL otherwise
     */
    //virtual cl_dev_err_code ExecuteWholeWorkGroup(
    //    const size_t* workGroupID) = 0;



		// Prepares current thread for the executable execution
		// For example set the required FP flags
		virtual cl_dev_err_code PrepareThread() = 0;
		
		// Restores Thread state as it was before the execution
		virtual cl_dev_err_code RestoreThreadState() = 0;

		// Executes the context on a specific thread
		// Input
		//  pGroupId - a 3 dimension array which containing the group id to be executed
		//  pLocalOffset - a 3 dimension array which containing the local offset in each dimension where to start execution
		//  pItemsToProcess - a 3 dimension array which contains the number of work items to process in each dimension 
		// Returns
		//		CL_DEV_SUCCESS - the execution completed successfully
		//		CL_DEV_ERROR_FAIL - the execution failed
		//
		virtual cl_dev_err_code Execute(const size_t* IN pGroupId,
			const size_t* IN pLocalOffset, 
			const size_t* IN pItemsToProcess) = 0;

		// Releases the context object
		virtual void	Release() = 0;




    /**
     * Restores the current thread state (should be called after execution)
     *
     * @returns CL_DEV_SUCCESS in case of success; CL_DEV_ERROR_FAIL otherwise
     */
    //virtual cl_dev_err_code RestoreThreadContext() = 0;

    /**
     * Releases Executable instance
     */
    //virtual void ReleaseExecutable() = 0;
};

}}} // namespace

#endif // ICLDevBackendExecutable_H
