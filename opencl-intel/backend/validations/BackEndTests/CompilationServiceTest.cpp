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

File Name:  CompilationServiceTest.cpp 

\*****************************************************************************/

#include "BackendWrapper.h"
#include <gtest/gtest.h>


using namespace llvm;

TEST_F(BackEndTests_CompilationService, CreateProgramSuccess)
{
    // get Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();;
    ASSERT_TRUE(funcGetFactory);
    // create a Compilation service
    ICLDevBackendCompileServicePtr   spCompileService(NULL);
    ASSERT_TRUE(spCompileService.get() == NULL);
    cl_dev_err_code ret = funcGetFactory->GetCompilationService(NULL, spCompileService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    ICLDevBackendCompilationService* pCompileService = spCompileService.get();
    ASSERT_TRUE(pCompileService);

    //-----------------------------------------------------------------
    // load pre created bitcode buffer in correct format - with kernels
    std::auto_ptr<BackendWrapper> pBackendWrapper(new BackendWrapper());
    ASSERT_TRUE(pBackendWrapper.get());
    const cl_prog_container_header* pHeader = pBackendWrapper->CreateProgramContainer(FILE_BC_WITH_KERNELS);
    ASSERT_TRUE(pHeader);
    // call CreateProgram with valid parameters - should success
    ICLDevBackendProgram_* pProgram = NULL;
    ret = pCompileService->CreateProgram(pHeader, &pProgram);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
}



TEST_F(BackEndTests_CompilationService, CreateProgramNoKernels)
{
    // get Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();;
    ASSERT_TRUE(funcGetFactory);
    // create a Compilation service
    ICLDevBackendCompileServicePtr   spCompileService(NULL);
    ASSERT_TRUE(spCompileService.get() == NULL);
    cl_dev_err_code ret = funcGetFactory->GetCompilationService(NULL, spCompileService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    ICLDevBackendCompilationService* pCompileService = spCompileService.get();
    ASSERT_TRUE(pCompileService);

    //-----------------------------------------------------------------
    // load pre created bitcode buffer in correct format - with no kernels
    std::auto_ptr<BackendWrapper> pBackendWrapper(new BackendWrapper());
    ASSERT_TRUE(pBackendWrapper.get());
    const cl_prog_container_header* pHeader = pBackendWrapper->CreateProgramContainer(FILE_BC_NO_KERNELS);
    ASSERT_TRUE(pHeader);
    // call CreateProgram with valid parameters - should success
    ICLDevBackendProgram_* pProgram = NULL;
    ret = pCompileService->CreateProgram(pHeader, &pProgram);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
}



TEST_F(BackEndTests_CompilationService, CreateProgramFailure)
{
    // get Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();;
    ASSERT_TRUE(funcGetFactory);
    // create a Compilation service
    ICLDevBackendCompileServicePtr   spCompileService(NULL);
    ASSERT_TRUE(spCompileService.get() == NULL);
    cl_dev_err_code ret = funcGetFactory->GetCompilationService(NULL, spCompileService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    ICLDevBackendCompilationService* pCompileService = spCompileService.get();
    ASSERT_TRUE(pCompileService);
    
    //-----------------------------------------------------------------
    // call the CreateProgram with invalid parameters - NULL in pByteCodeContainer
    ICLDevBackendProgram_* pProgram = NULL;
    // invalid parameters - should fail with no crash
    ret = pCompileService->CreateProgram(NULL, &pProgram);
    EXPECT_NE(CL_DEV_SUCCESS, ret);


    //-----------------------------------------------------------------
    // call the CreateProgram with invalid parameters - NULL in ppProgram
    // load pre created bitcode buffer in correct format
    std::auto_ptr<BackendWrapper> pBackendWrapper(new BackendWrapper());
    ASSERT_TRUE(pBackendWrapper.get());
    const cl_prog_container_header* pHeader = pBackendWrapper.get()->CreateProgramContainer(FILE_BC_WITH_KERNELS);
    ASSERT_TRUE(pHeader);
    // invalid parameters - should fail with no crash
    ret = pCompileService->CreateProgram(pHeader, NULL);
    EXPECT_NE(CL_DEV_SUCCESS, ret);
}



TEST_F(BackEndTests_CompilationService, BuildProgramSuccess)
{
    // get Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();;
    ASSERT_TRUE(funcGetFactory);
    // create a Compilation service
    ICLDevBackendCompileServicePtr   spCompileService(NULL);
    ASSERT_TRUE(spCompileService.get() == NULL);
    cl_dev_err_code ret = funcGetFactory->GetCompilationService(NULL, spCompileService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    ICLDevBackendCompilationService* pCompileService = spCompileService.get();
    ASSERT_TRUE(pCompileService);
    // create valid ProgramBuilderBuild options
    ProgramBuilderBuildOptions buildOptions;
    buildOptions.InitFromTestConfiguration("auto");
    EXPECT_TRUE(STRING_EQ("auto",buildOptions.GetStringValue(CL_DEV_BACKEND_OPTION_CPU_ARCH, "")));

    //-----------------------------------------------------------------
    // create & build program with valid parameters - with kernels - valid options
    std::auto_ptr<BackendWrapper> pBackendWrapper(new BackendWrapper());
    ASSERT_TRUE(pBackendWrapper.get());
    const cl_prog_container_header* pHeader = pBackendWrapper->CreateProgramContainer(FILE_BC_WITH_KERNELS);
    ASSERT_TRUE(pHeader);
    ICLDevBackendProgram_* pProgram = NULL;
    ret = pCompileService->CreateProgram(pHeader, &pProgram);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    // call BuildProgram with valid parameters - should success
    ret = pCompileService->BuildProgram(pProgram, &buildOptions);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    pCompileService->ReleaseProgram(pProgram);

    

    //-----------------------------------------------------------------
    // create & build program with valid parameters - with no kernels - valid options
    std::auto_ptr<BackendWrapper> pBackendWrapper2(new BackendWrapper());
    ASSERT_TRUE(pBackendWrapper2.get());
    const cl_prog_container_header* pHeader2 = pBackendWrapper2->CreateProgramContainer(FILE_BC_NO_KERNELS);
    ASSERT_TRUE(pHeader2);
    ICLDevBackendProgram_* pProgram2 = NULL;
    ret = pCompileService->CreateProgram(pHeader2, &pProgram2);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    // call BuildProgram with valid parameters - should success
    ret = pCompileService->BuildProgram(pProgram2, &buildOptions);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    pCompileService->ReleaseProgram(pProgram2);



    //-----------------------------------------------------------------
    // create & build program with valid parameters - with kernels - NULL options
    std::auto_ptr<BackendWrapper> pBackendWrapper3(new BackendWrapper());
    ASSERT_TRUE(pBackendWrapper3.get());
    const cl_prog_container_header* pHeader3 = pBackendWrapper3->CreateProgramContainer(FILE_BC_WITH_KERNELS);
    ASSERT_TRUE(pHeader3);
    ICLDevBackendProgram_* pProgram3 = NULL;
    ret = pCompileService->CreateProgram(pHeader3, &pProgram3);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    // call BuildProgram with valid parameters - should success
    ret = pCompileService->BuildProgram(pProgram3, NULL);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    pCompileService->ReleaseProgram(pProgram3);



    //-----------------------------------------------------------------
    // create & build program with valid parameters - with no kernels - NULL options
    std::auto_ptr<BackendWrapper> pBackendWrapper4(new BackendWrapper());
    ASSERT_TRUE(pBackendWrapper4.get());
    const cl_prog_container_header* pHeader4 = pBackendWrapper4->CreateProgramContainer(FILE_BC_NO_KERNELS);
    ASSERT_TRUE(pHeader4);
    ICLDevBackendProgram_* pProgram4 = NULL;
    ret = pCompileService->CreateProgram(pHeader4, &pProgram4);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    // call BuildProgram with valid parameters - should success
    ret = pCompileService->BuildProgram(pProgram4,NULL);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    pCompileService->ReleaseProgram(pProgram4);
}



TEST_F(BackEndTests_CompilationService, BuildProgramFailure)
{
    // get Backend service factory
    ICLDevBackendServiceFactory* funcGetFactory = BackendWrapper::GetInstance().GetBackendServiceFactory();;
    ASSERT_TRUE(funcGetFactory);
    // create a Compilation service
    ICLDevBackendCompileServicePtr   spCompileService(NULL);
    ASSERT_TRUE(spCompileService.get() == NULL);
    cl_dev_err_code ret = funcGetFactory->GetCompilationService(NULL, spCompileService.getOutPtr());
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    ICLDevBackendCompilationService* pCompileService = spCompileService.get();
    ASSERT_TRUE(pCompileService);

    //-----------------------------------------------------------------
    // call BuildProgram with invalid parameters - invalid pProgram
    // create program with invalid bitcode
    std::auto_ptr<BackendWrapper> pBackendWrapper(new BackendWrapper());
    ASSERT_TRUE(pBackendWrapper.get());
    const cl_prog_container_header* pHeader = pBackendWrapper->CreateProgramContainer(FILE_BC_NOISE);
    ASSERT_TRUE(pHeader);
    ICLDevBackendProgram_* pProgram = NULL;
    ret = pCompileService->CreateProgram(pHeader, &pProgram);
    EXPECT_EQ(CL_DEV_SUCCESS, ret);
    // call BuildProgram with invalid bitcode - should fail
    ret = pCompileService->BuildProgram(pProgram, NULL);
    EXPECT_NE(CL_DEV_SUCCESS, ret);


    //-----------------------------------------------------------------
    // test invalid parameters to the actuall BuildProgram call - NULL in pProgram
    // invalid parameters - should fail with no crash
    ret = pCompileService->BuildProgram(NULL, NULL);
    EXPECT_NE(CL_DEV_SUCCESS, ret);
}
