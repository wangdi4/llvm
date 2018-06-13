/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BackendWrapper.h

\*****************************************************************************/
#ifndef BACKEND_WRAPPER_H
#define BACKEND_WRAPPER_H

/** @brief Common classes for the backend tests.
*
*/

#include <string>
#include <vector>

#include <gtest/gtest.h>                // Test framework

#include "BE_DynamicLib.h"
#include "BWOptions.h" // the implemented Options classes
#include "CPUDetect.h"
#include "Exception.h"
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "mem_utils.h"
#include "plugin_manager.h"

using namespace Intel::OpenCL::DeviceBackend;

#if defined(_WIN32)
/// @brief the backend dll file name
#if defined(_M_X64)
#define CPUBACKEND_DLL_NAME             "OclCpuBackEnd64.dll"
#else
#define CPUBACKEND_DLL_NAME             "OclCpuBackEnd32.dll"
#endif
/// @brief the plugin dll file name, in our case its the SamplePlugin
#define PLUGIN_DLL_NAME                 "OCLSamplePlugin.dll"
#else
/// @brief the backend dll file name
#define CPUBACKEND_DLL_NAME             "libOclCpuBackEnd.so"
/// @brief the plugin dll file name, in our case its the SamplePlugin
#define PLUGIN_DLL_NAME                 "libOCLSamplePlugin.so"
#endif

/// @brief bitcode filenames, used in CompilationService tests
#if defined _M_X64 || defined __x86_64__
#define FILE_BC_WITH_KERNELS            "bitcodeWithKernels_64.bc"
#define FILE_BC_NO_KERNELS              "bitcodeNoKernels_64.bc"
#else
#define FILE_BC_WITH_KERNELS            "bitcodeWithKernels_32.bc"
#define FILE_BC_NO_KERNELS              "bitcodeNoKernels_32.bc"
#endif
#define FILE_BC_NOISE                   "bitcodeNoise.bc"

/// @brief options parameters, used in FactoryMethods tests
#define TRANSPOSE_SIZE_UNSUPPORTED      -1
#define ARCH_UNSUPPORTED                "arch"
#define CPU_FEATURES_UNSUPPORTED        "cpu"
#define UNSUPPORTED_DEVICE              -1

/// @brief devices supported by the backend
#define BW_CPU_DEVICE  CPU_DEVICE

/// @brief the plugin dll path environment variable
#define PLUGIN_ENVIRONMENT_VAR          "OCLBACKEND_PLUGINS"

#define STRING_EQ(str1, str2)           ((std::string)str1 == (std::string)(str2))

/// @brief auto pointers for the backend services
typedef Validation::auto_ptr_ex<ICLDevBackendCompilationService, Validation::ReleaseDP<ICLDevBackendCompilationService> >      ICLDevBackendCompileServicePtr;
typedef Validation::auto_ptr_ex<ICLDevBackendExecutionService, Validation::ReleaseDP<ICLDevBackendExecutionService> >          ICLDevBackendExecutionServicePtr;
typedef Validation::auto_ptr_ex<ICLDevBackendSerializationService, Validation::ReleaseDP<ICLDevBackendSerializationService> >  ICLDevBackendSerializationServicePtr;

/// @brief the plugin exported function
typedef bool (*PLUGIN_EXPORT_F)();

/** @brief BackendWrapper: This class hides the internals of loading and calling OCL CPU Backend.
*        +base class for FactoryMethds, CompilationsService & Plugins test cases classes
*/
class BackendWrapper : public ::testing::Test
{
public:
    BackendWrapper();
    ~BackendWrapper();

    /// @brief Init(): creating instance, loading the dll and (decided by caller) initiating the backend
    static bool Initiate(bool callBackendInit = true);
    /// @brief GetInstance(): getting the BackendWrapper instance
    static BackendWrapper& GetInstance();
    /// @brief Terminate(): terminating the BackendWrapper instance
    static bool Terminate();

    /// @brief LoadDll(): loading the backend dll and getting the exported functions
    ///        + Will be called by the Init() method
    bool LoadDll();

    /// @brief the exported functions from the backend dll
    cl_dev_err_code Init(const ICLDevBackendOptions* pBackendOptions);
    ICLDevBackendServiceFactory* GetBackendServiceFactory();
    void Release();

    /// @brief CreateProgramContainer: create a valid program container
    void CreateProgramContainer(const std::string& programFile, std::vector<char>& buffer);

protected:
    /** @brief SetUp(), TearDown(): called by the gtest, shared for FactoryMethods tests
    *        and CompilationService tests, responsible for initiating the BackendWrapper
    *        class and terminating it
    *        overload them if necessary (in PluginTest for example)
    */
    virtual void SetUp();
    virtual void TearDown();

    /// @brief the backend dll and the exported functions
    Intel::OpenCL::DeviceBackend::Utils::BE_DynamicLib m_dll;
    BACKEND_INIT_FUNCPTR       m_funcInit;
    BACKEND_TERMINATE_FUNCPTR  m_funcTerminate;
    BACKEND_GETFACTORY_FUNCPTR m_funcGetFactory;

    static BackendWrapper* s_instance;

    /// @brief true only if the backend m_funcInit is called successfully
    bool backendInitiated;

    void TestBody(){}
};//BackendWrapper

/** @brief classes that inherit from BackendWrapper
*   this gives the option to overload SetUp and TearDown
*   useful for Plugin Tests for example
*/

/// @brief dummy inheritance, only for the output of gtest
class BackEndTests_FactoryMethods : public BackendWrapper {};

/// @brief dummy inheritance, only for the output of gtest
class BackEndTests_CompilationService : public BackendWrapper {};

/// @brief dummy inheritance, only for the output of gtest
class BackEndTests_KernelSubGroupInfo : public BackendWrapper {};

/// @brief inheritance is needed for overloading SetUp()
class BackEndTests_Plugins : public BackendWrapper
{
public:
    /// @brief creating the event for testing the plugin flag (see SamplePlugin)
    ///        in our case the event is CreateProgram
    void CreateTestEvent();

    void CreateTestEvent(Intel::OpenCL::PluginManager* pManager);

protected:
    virtual void SetUp();

    Intel::OpenCL::DeviceBackend::Utils::BE_DynamicLib m_dll;
    /// @brief the plugin exported function (see SamplePlugin)
    PLUGIN_EXPORT_F getFlag;
};

class OCLProgramMock: public ICLDevBackendProgram_
{
public:
    ~OCLProgramMock(){}

    unsigned long long int GetProgramID() const { return 0; }

    const char* GetBuildLog() const { return ""; }

    virtual const ICLDevBackendCodeContainer* GetProgramCodeContainer() const { return NULL; }

    virtual const ICLDevBackendCodeContainer* GetProgramIRCodeContainer() const { return NULL; }

    virtual cl_dev_err_code GetKernelByName(
        const char* pKernelName,
        const ICLDevBackendKernel_** ppKernel) const { return CL_DEV_ERROR_FAIL; }

    virtual int GetNonBlockKernelsCount() const { return 0; }

    virtual int GetKernelsCount() const { return 0; }

    virtual cl_dev_err_code	GetKernel(
        int kernelIndex,
        const ICLDevBackendKernel_** pKernel) const { return CL_DEV_ERROR_FAIL; }

    virtual const ICLDevBackendProgramJITCodeProperties* GetProgramJITCodeProperties() const { return NULL; }

    virtual size_t GetGlobalVariableTotalSize() const { return 0; }
};

#endif // BACKEND_WRAPPER_H
