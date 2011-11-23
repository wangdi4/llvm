#ifndef ICLDevBackendCompilationService_H
#define ICLDevBackendCompilationService_H

#include "ICLDevBackendProgram.h"
#include "cl_device_api.h"
#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ICLDevBackendOptions;
    
/**
 * This interface class is responsible for the compilation services which should be
 * loaded at the Host side and Device side, programs can be created and compiled 
 * through this interface;
 */
class ICLDevBackendCompilationService
{
public:
    /**
     * Creates program from the specified bytecode, it should verify the byte code before 
     * creation
     *
     * @param pByteCodeContainer is the byte code buffer
     * @param ppProgram will be modified to point to the generated program
     *
     * @returns in case of success CL_DEV_SUCCESS will be returned and ppProgram will be modified
     *  to point to the generated program; otherwise pProgram will point to NULL and will return:
     *  CL_DEV_INVALID_BINARY if the program bytecode is invalid
     *  CL_DEV_OUT_OF_MEMORY if there's no sufficient memory 
     *  CL_DEV_ERROR_FAIL in any other error
     */
    virtual cl_dev_err_code CreateProgram(
        const cl_prog_container_header* pByteCodeContainer, 
        ICLDevBackendProgram_** ppProgram) = 0;
		
    /**
     * Builds the program
     *
     * @param pProgram pointer to the program which will be passed to the builder
     *  (will be modified with the build output)
     * @param pOptions pointer to ICLDevBackendOptions that describes the build options to be 
     *  used for building the program
     *
     * @returns
     *	CL_DEV_BUILD_ERROR	    - in case their is a build errors
     *	CL_DEV_BUILD_WARNING	- in case their is warnings
     *	CL_DEV_SUCCESS	        - the build succeeded
     *	CL_DEV_INVALID_BUILD_OPTIONS  - if the build options specified by pOptions are invalid
     *	CL_DEV_OUT_OF_MEMORY          - if the there is a failure to allocate memory 
     *  CL_DEV_BUILD_ALREADY_COMPLETE - if the program has been already compiled
     */
	virtual cl_dev_err_code BuildProgram(
        ICLDevBackendProgram_* pProgram, 
        const ICLDevBackendOptions* pOptions ) = 0;
    

    /**
     * Dumps the content of the given code container 
     * using the options passed in pOptions parameter
     *
     * @param pCodeContainer Code container to dump
     * @param pOptions pointer to the options object which may contain the dump settings.
     *                 /see cl_dev_backend_dump_options
     *
     * @returns
     *	CL_DEV_SUCCESS	        - the build succeeded
     *	CL_DEV_INVALID_BUILD_OPTIONS  - if the build options specified by pOptions are invalid
     *	CL_DEV_OUT_OF_MEMORY          - if the there is a failure to allocate memory 
     */
    virtual cl_dev_err_code DumpCodeContainer(
        const ICLDevBackendCodeContainer* pCodeContainer,
        const ICLDevBackendOptions* pOptions ) const = 0;

    /**
     * Releases the Compilation Service
     */
    virtual void Release() = 0;

     /**
     * Prints the JIT code in assembly x86
     *
     * @param pCodeContainer Code container
     * @param dumpJIT The filename for dumping the JIT code
     * @param baseDirectory The base directory
     */
    virtual void DumpJITCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
            const std::string dumpJIT,
            const std::string baseDirectory) const = 0;
};

}}} // namespace

#endif // ICLDevBackendCompilationService_H
