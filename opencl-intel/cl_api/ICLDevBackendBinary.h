#ifndef ICLDevBackendBinary_H
#define ICLDevBackendBinary_H

#include "cl_device_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

// Defines possible values for kernel argument types
typedef enum _cl_exec_mem_type
{
	CL_EXEC_LOCAL_MEMORY_TYPE = 0,	// this is a buffer used by the executable for local memory 
	CL_EXEC_PRIVATE_MEMORY_TYPE,	// this is a buffer used by the executable for private memory
	CL_EXEC_INTERNAL_MEMORY_TYPE	// this is a buffer used by the executable for internal uses (opaque)
} cl_exec_mem_type;

class ICLDevBackendExecutable_; //TEMPORARY
/**
 * This interface class represents an OCL binary with specified OCL work description
 */
class ICLDevBackendBinary_
{
public:
    virtual ~ICLDevBackendBinary_() {}

	// Returns the required number of memory buffers and their sizes 
	//	pBuffersSizes - an array of sizes of buffers
	//	pBufferCount - the number of buffers required for executing the kernels
	// Returns
	//		CL_DEV_BE_SUCCESS - the execution completed successfully
	//
	virtual cl_dev_err_code GetMemoryBuffersDescriptions(size_t* IN pBuffersSizes, size_t* INOUT pBufferCount ) const = 0;

	// Returns the actual number of Work Items handled by each executable instance
	virtual const size_t* GetWorkGroupSize() const = 0;


	// Create execution context that will be used by specific execution threads
	virtual cl_dev_err_code CreateExecutable(void* IN *pMemoryBuffers, 
		size_t IN stBufferCount, ICLDevBackendExecutable_* OUT *pContext) = 0;

    /**
     * Releases binary instance
     */
    virtual void Release() = 0;
};

}}} // namespace

#endif // ICLDevBackendBinary_H
