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

File Name:  FactoryTest.cpp

\*****************************************************************************/


#include "BackendWrapper.h"
#include <gtest/gtest.h>


TEST_F(BackEndTests_FactoryMethods, FactoryInitialization)
{
    // get Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();
    ASSERT_TRUE(funcGetFactory);
}



TEST_F(BackEndTests_FactoryMethods, CompilerServiceCreation)
{
    // get Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();
    ASSERT_TRUE(funcGetFactory);
    // get Compilation service pointer
    ICLDevBackendCompileServicePtr   spCompileService(NULL);
    ASSERT_TRUE(spCompileService.get() == NULL);

    CompilationServiceOptions options;
    cl_dev_err_code ret;

    //-----------------------------------------------------------------
    // create valid set of options
    options.InitFromTestConfiguration(CPU_DEVICE, "auto", "", TRANSPOSE_SIZE_AUTO, false);
    EXPECT_FALSE(options.GetBooleanValue(CL_DEV_BACKEND_OPTION_USE_VTUNE, false));
    EXPECT_TRUE(STRING_EQ("",options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES, "")));
    EXPECT_TRUE(STRING_EQ("auto",options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "")));
    EXPECT_EQ(TRANSPOSE_SIZE_AUTO,options.GetIntValue(CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, TRANSPOSE_SIZE_UNSUPPORTED));
    // call GetCompilationService with valid parameters - should success
    ret = funcGetFactory->GetCompilationService(&options, spCompileService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);


    //-----------------------------------------------------------------
    // create another set of valid options
    std::string currCPU = Utils::CPUDetect::GetInstance()->GetCPUId().GetCPUName();
    options.InitFromTestConfiguration(CPU_DEVICE, currCPU, "", TRANSPOSE_SIZE_1, true);
    EXPECT_TRUE(options.GetBooleanValue(CL_DEV_BACKEND_OPTION_USE_VTUNE, false));
    EXPECT_TRUE(STRING_EQ("",options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES, "")));
    EXPECT_TRUE(STRING_EQ(currCPU,options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "")));
    EXPECT_EQ(TRANSPOSE_SIZE_1,options.GetIntValue(CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, TRANSPOSE_SIZE_UNSUPPORTED));
    // call GetCompilationService with valid parameters - should success
    ret = funcGetFactory->GetCompilationService(&options, spCompileService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);


    //-----------------------------------------------------------------
    // create another set of valid options - enabling special features
    const bool avx1Support = Utils::CPUDetect::GetInstance()->GetCPUId().HasAVX1();
    if(avx1Support){
        options.InitFromTestConfiguration(CPU_DEVICE, currCPU, "+avx", TRANSPOSE_SIZE_16, false);
        EXPECT_FALSE(options.GetBooleanValue(CL_DEV_BACKEND_OPTION_USE_VTUNE, false));
        EXPECT_TRUE(STRING_EQ("+avx",options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES, "")));
        EXPECT_TRUE(STRING_EQ(currCPU,options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "")));
        EXPECT_EQ(TRANSPOSE_SIZE_16,options.GetIntValue(CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, TRANSPOSE_SIZE_UNSUPPORTED));
        // call GetCompilationService with valid parameters - should success
        ret = funcGetFactory->GetCompilationService(&options, spCompileService.getOutPtr());
        EXPECT_EQ(CL_DEV_SUCCESS, ret);
    }
}



TEST_F(BackEndTests_FactoryMethods, CompilerServiceFailure)
{
    // get the Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();
    ASSERT_TRUE(funcGetFactory);
    // get Compilation service pointer
    ICLDevBackendCompileServicePtr   spCompileService(NULL);
    ASSERT_TRUE(spCompileService.get() == NULL);

    CompilationServiceOptions options;
    cl_dev_err_code ret;

    //-----------------------------------------------------------------
    // create invalid set of options - unsupported architecture
    options.InitFromTestConfiguration(CPU_DEVICE, ARCH_UNSUPPORTED, "", TRANSPOSE_SIZE_AUTO, false);
    EXPECT_FALSE(options.GetBooleanValue(CL_DEV_BACKEND_OPTION_USE_VTUNE, false));
    EXPECT_TRUE(STRING_EQ("",options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES, "")));
    EXPECT_EQ(TRANSPOSE_SIZE_AUTO,options.GetIntValue(CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE, TRANSPOSE_SIZE_UNSUPPORTED));
    EXPECT_TRUE(STRING_EQ(ARCH_UNSUPPORTED, options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "auto")));
    // call GetCompilationService with Options invalid - should fail
    ret = funcGetFactory->GetCompilationService(&options, spCompileService.getOutPtr());
    EXPECT_NE(CL_DEV_SUCCESS, ret);

    //-----------------------------------------------------------------
    // test invalid parameters to the actuall GetCompilationService call
    // call GetCompilationService with Output variable NULL - should fail with no crash
    ret = funcGetFactory->GetCompilationService(NULL, NULL);
    EXPECT_NE(CL_DEV_SUCCESS, ret);
}



TEST_F(BackEndTests_FactoryMethods, ExecutionServiceCreation)
{
    // get the Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();
    ASSERT_TRUE(funcGetFactory);
    // get Execution service pointer
    ICLDevBackendExecutionServicePtr spExecutionService(NULL);
    ASSERT_TRUE(spExecutionService.get() == NULL);

    ExecutionServiceOptions options;
    cl_dev_err_code ret;

    //-----------------------------------------------------------------
    // create valid set of options
    options.InitFromTestConfiguration(BW_CPU_DEVICE, "auto");
    EXPECT_TRUE(STRING_EQ("auto",options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "")));
    // call GetExecutionService with valid parameters - should success
    ret = funcGetFactory->GetExecutionService(&options, spExecutionService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);


    //-----------------------------------------------------------------
    // create another set of valid options
    std::string currCPU = Utils::CPUDetect::GetInstance()->GetCPUId().GetCPUName();
    options.InitFromTestConfiguration(CPU_DEVICE, currCPU);
    EXPECT_TRUE(STRING_EQ(currCPU,options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "")));
    // call GetExecutionService with valid parameters - should success
    ret = funcGetFactory->GetExecutionService(&options, spExecutionService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
}



TEST_F(BackEndTests_FactoryMethods, ExecutionServiceFailure)
{
    // get the Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();
    ASSERT_TRUE(funcGetFactory);
    // get Execution service pointer
    ICLDevBackendExecutionServicePtr spExecutionService(NULL);
    ASSERT_TRUE(spExecutionService.get() == NULL);

    ExecutionServiceOptions options;
    cl_dev_err_code ret;

    //-----------------------------------------------------------------
    // create invalid opthions parameters - unsupported architecture
    options.InitFromTestConfiguration(static_cast<DeviceMode>(UNSUPPORTED_DEVICE), ARCH_UNSUPPORTED);
    EXPECT_TRUE(STRING_EQ(ARCH_UNSUPPORTED, options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "auto")));
    // call GetExecutionService with Options invalid - should fail
    ret = funcGetFactory->GetExecutionService(&options, spExecutionService.getOutPtr());
    EXPECT_NE(CL_DEV_SUCCESS, ret);


    //-----------------------------------------------------------------
    // test invalid parameters to the actuall GetExecutionService call
    // call GetExecutionService with Output variable NULL - should fail
    ret = funcGetFactory->GetExecutionService(NULL, NULL);
    EXPECT_NE(CL_DEV_SUCCESS, ret);
}


TEST_F(BackEndTests_FactoryMethods, SerializationServiceFailure)
{
    // get the Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();
    ASSERT_TRUE(funcGetFactory);
    // get Serialization service pointer
    ICLDevBackendSerializationServicePtr spSerializationService(NULL);
    ASSERT_TRUE(spSerializationService.get() == NULL);
    // init the jit allocator and get instance
    JITAllocator::Init();
    JITAllocator* pJITAllocator = JITAllocator::GetInstance();
    ASSERT_TRUE(pJITAllocator);
    void* pJITAllocatorTemp;
    size_t size;
    SerializationServiceOptions options;
    cl_dev_err_code ret;

    //-----------------------------------------------------------------
    // create invalid set of options - unsupported architecture
    options.InitFromTestConfiguration(BW_CPU_DEVICE, ARCH_UNSUPPORTED, pJITAllocator);
    EXPECT_TRUE(STRING_EQ(ARCH_UNSUPPORTED, options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "auto")));
    pJITAllocatorTemp = NULL;
    size = 0;
    options.GetValue(CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR, &pJITAllocatorTemp, &size);
    EXPECT_EQ(pJITAllocator,pJITAllocatorTemp);
    // call GetSerializationService with Options invalid - should fail
    ret = funcGetFactory->GetSerializationService(&options, spSerializationService.getOutPtr());
    EXPECT_NE(CL_DEV_SUCCESS, ret);


    //-----------------------------------------------------------------
    // create another set of invalid options - CPU mode and not MIC mode
    options.InitFromTestConfiguration(BW_CPU_DEVICE,"auto", pJITAllocator);
    EXPECT_TRUE(STRING_EQ("auto",options.GetStringValue(CL_DEV_BACKEND_OPTION_SUBDEVICE, "")));
    pJITAllocatorTemp = NULL;
    size = 0;
    options.GetValue(CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR, &pJITAllocatorTemp, &size);
    EXPECT_EQ(pJITAllocator,pJITAllocatorTemp);
    // call GetSerializationService with Options invalid - should fail with no crash
    ret = funcGetFactory->GetSerializationService(&options, spSerializationService.getOutPtr());
    EXPECT_NE(CL_DEV_SUCCESS, ret);

    //-----------------------------------------------------------------
    // test invalid parameters to the actuall GetSerializationService
    // call GetSerializationService with NULL in output variable - should fail
    ret = funcGetFactory->GetSerializationService(NULL, NULL);
    EXPECT_NE(CL_DEV_SUCCESS, ret);
}
