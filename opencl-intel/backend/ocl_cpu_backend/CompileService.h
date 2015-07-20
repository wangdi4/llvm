/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CompileService.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"
#include "ProgramBuilder.h"
#include "IAbstractBackendFactory.h"
#include "llvm/Support/Mutex.h"

#ifdef OCL_DEV_BACKEND_PLUGINS
#include "plugin_manager.h"
#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {
class CompileService: public ICLDevBackendCompilationService
{
public:
    CompileService();
    virtual ~CompileService() { }

    cl_dev_err_code CheckProgramBinary( const void* pBinary,
                                        size_t uiBinarySize ) const = 0;

    /**
     * Creates program from the specified bytecode, it should verify the byte code before
     * creation
     *
     * @param pByteCodeContainer is the byte code buffer
     * @param ppProgram will be modified to point to the generated program
     *
     * @returns in case of success CL_DEV_SUCCESS will be returned and pProgram will point to
     *  the generated program; otherwise pProgram will point to NULL and will return:
     *  CL_DEV_INVALID_BINARY if the program bytecode is invalid
     *  CL_DEV_OUT_OF_MEMORY if there's no sufficient memory
     *  CL_DEV_ERROR_FAIL in any other error
     */
    cl_dev_err_code CreateProgram( const void* pBinary,
                                   size_t uiBinarySize,
                                   ICLDevBackendProgram_** ppProgram);

    /**
     * Releases Program instance
     */
    void ReleaseProgram( ICLDevBackendProgram_* pProgram) const;

    /**
     * Builds the program
     *
     * @param pProgram pointer to the program which will be passed to the builder
     *  (will be modified with the build output)
     * @param pOptions pointer to a string that describes the build options to be used for building
     *  the program
     *
     * @returns
     *  CL_DEV_BUILD_ERROR      - in case their is a build errors
     *  CL_DEV_BUILD_WARNING    - in case their is warnings
     *  CL_DEV_SUCCESS          - the build succeeded
     *  CL_DEV_INVALID_BUILD_OPTIONS  - if the build options specified by pOptions are invalid
     *  CL_DEV_OUT_OF_MEMORY          - if the there is a failure to allocate memory
     *  CL_DEV_BUILD_ALREADY_COMPLETE - if the program has been already compiled
     */
    cl_dev_err_code BuildProgram( ICLDevBackendProgram_* pProgram,
                                  const ICLDevBackendOptions* pOptions );

    /**
     * Dumps the content of the given code container
     * using the options passed in pOptions parameter
     *
     * @param pCodeContainer Code container to dump
     * @param pOptions pointer to the options object which may contain the dump settings.
     *                 /see cl_dev_backend_dump_options
     *
     * @returns
     *  CL_DEV_SUCCESS          - the build succeeded
     *  CL_DEV_INVALID_BUILD_OPTIONS  - if the build options specified by pOptions are invalid
     *  CL_DEV_OUT_OF_MEMORY          - if the there is a failure to allocate memory
     */
    cl_dev_err_code DumpCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                                       const ICLDevBackendOptions* pOptions ) const;

    /**
     * Releases the Compilation Service
     */
    void Release();

     /**
     * Prints the JIT code in assembly x86
     *
     * @param pCodeContainer Code container
     * @param filename Destination file name to store the dump to
     */
    cl_dev_err_code DumpJITCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                                          const std::string& filename) const;

protected:
    virtual const ProgramBuilder* GetProgramBuilder() const = 0;
    virtual ProgramBuilder* GetProgramBuilder() = 0;

    // pointer to the Backend Factory, not owned by this class
    IAbstractBackendFactory* m_backendFactory;
    // temporary solution for MT build problem on OCL SDK
    mutable llvm::sys::Mutex       m_buildLock;
    #ifdef OCL_DEV_BACKEND_PLUGINS
    mutable Intel::OpenCL::PluginManager   m_pluginManager;
    #endif
};
}}}
