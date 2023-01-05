#include "gtest_wrapper.h"

#include <string>

#include "BackendWrapper.h"
#include "KernelProperties.h"
#include "common_utils.h"

#if defined _M_X64 || defined __x86_64__
#define BC_FILE "reqd_num_sub_groups_64.bc"
#else
#define BC_FILE "reqd_num_sub_groups_32.bc"
#endif

// The source bitcode consists one kernel with __attribute__
// (required_num_sub_groups(5)). The tested function GetRequiredNumSubGroups()
// must return value of required number of subgroups defined in attribute (i.e.
// 5).

TEST_F(DISABLED_BackEndTests_KernelSubGroupInfo, SubGroupInfoSuccess) {
  ICLDevBackendServiceFactory *funcGetFactory =
      BackendWrapper::GetInstance().GetBackendServiceFactory();
  ASSERT_TRUE(funcGetFactory);

  ICLDevBackendCompileServicePtr spCompileService(nullptr);
  cl_dev_err_code ret = funcGetFactory->GetCompilationService(
      nullptr, spCompileService.getOutPtr());
  EXPECT_EQ(CL_DEV_SUCCESS, ret);
  ICLDevBackendCompilationService *pCompileService = spCompileService.get();
  ASSERT_TRUE(pCompileService);

  ProgramBuilderBuildOptions buildOptions;
  buildOptions.InitFromTestConfiguration("auto");
  EXPECT_TRUE(STRING_EQ("auto", buildOptions.GetStringValue(
                                    CL_DEV_BACKEND_OPTION_SUBDEVICE, "")));

  // load pre created bitcode buffer
  std::unique_ptr<BackendWrapper> pBackendWrapper(new BackendWrapper());
  ASSERT_TRUE(pBackendWrapper.get());
  std::vector<char> program;
  pBackendWrapper->CreateProgramContainer(get_exe_dir() + BC_FILE, program);
  ASSERT_TRUE(program.size() > 0);

  ICLDevBackendProgram_ *pProgram = nullptr;
  ret =
      pCompileService->CreateProgram(program.data(), program.size(), &pProgram);
  EXPECT_EQ(CL_DEV_SUCCESS, ret);

  ret = pCompileService->BuildProgram(pProgram, &buildOptions, "");
  EXPECT_EQ(CL_DEV_SUCCESS, ret);
  EXPECT_EQ(pProgram->GetKernelsCount(), 1);

  const ICLDevBackendKernel_ *pIKernel;
  pProgram->GetKernel(0, &pIKernel);
  ASSERT_FALSE(pIKernel == nullptr);

  const ICLDevBackendKernelProporties *pKernelProps =
      pIKernel->GetKernelProporties();

  size_t kernelValue = 0;
  const size_t requiredNumSG = 5;

  kernelValue = pKernelProps->GetRequiredNumSubGroups();
  EXPECT_EQ(requiredNumSG, kernelValue);
}
