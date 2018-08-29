// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "MetadataAPITestFixture.h"

#include "MetadataAPI.h"
#include "MetadataStatsAPI.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <utility>

using namespace Intel;
using namespace MetadataAPI;

TEST_F(MetadataTest, Test_RecursiveCallMetadata) {
  auto pModule = GetTestModule();

  for (auto &F : *pModule) {
    FunctionMetadataAPI mdApi(&F);

    mdApi.RecursiveCall.set(true);
  }

  for (auto &F : *pModule) {
    FunctionMetadataAPI mdApi(&F);

    EXPECT_EQ(true, mdApi.RecursiveCall.get());
  }
}

TEST_F(MetadataTest, Test_GetRecursiveCallMetadataFromNoMetadataFunction) {
  auto pModule = GetTestModule();

  FunctionMetadataAPI mdApi(pModule->getFunction("metatest_plain_func"));

  EXPECT_FALSE(mdApi.RecursiveCall.get());
}

/// KernelArgNamespaces

TEST_F(MetadataTest, Test_RetrieveKernelArgAddressSpacesFromNotAKernel) {
  auto pModule = GetTestModule();

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_plain_func"));

  EXPECT_FALSE(kernelMDApi.ArgAddressSpaceList.hasValue());
}

TEST_F(MetadataTest, Test_RetrieveKernelArgNamespacesThroughIndex) {
  auto pModule = GetTestModule();

  const std::vector<int32_t> expected = {0, 1, 1};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  EXPECT_TRUE(kernelMDApi.ArgAddressSpaceList.hasValue());

  for (size_t i = 0; i < kernelMDApi.ArgAddressSpaceList.size(); i++) {
    EXPECT_EQ(expected[i], kernelMDApi.ArgAddressSpaceList.getItem(i));
  }
}

TEST_F(MetadataTest, Test_RetrieveKernelArgNamespacesThroughForRangeLoop) {
  auto pModule = GetTestModule();

  const std::vector<int32_t> expected = {0, 1, 1};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  EXPECT_TRUE(kernelMDApi.ArgAddressSpaceList.hasValue());

  EXPECT_EQ(expected.size(), kernelMDApi.ArgAddressSpaceList.size());

  auto expected_it = expected.begin();
  for (auto addr_space : kernelMDApi.ArgAddressSpaceList) {
    EXPECT_EQ(addr_space, *expected_it);
    expected_it++;
  }
}

/// kernel_arg_name

TEST_F(MetadataTest, Test_RetrieveKernelArgNameFromNotAKernel) {
  auto pModule = GetTestModule();

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_plain_func"));

  EXPECT_FALSE(kernelMDApi.ArgNameList.hasValue());
}

TEST_F(MetadataTest, Test_RetrieveKernelArgNameThroughIndex) {
  auto pModule = GetTestModule();

  const std::vector<llvm::StringRef> expected = {"argFloat", "argIntBuffer",
                                             "argImg"};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto KernelArgNameAPI = kernelMDApi.ArgNameList;

  EXPECT_TRUE(KernelArgNameAPI.hasValue());

  for (size_t i = 0; i < KernelArgNameAPI.size(); i++) {
    EXPECT_TRUE(expected[i] == KernelArgNameAPI.getItem(i));
  }
}

TEST_F(MetadataTest, Test_RetrieveKernelArgNameThroughForRangeLoop) {
  auto pModule = GetTestModule();

  const std::vector<llvm::StringRef> expected = {"argFloat", "argIntBuffer",
                                             "argImg"};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto KernelArgNameAPI = kernelMDApi.ArgNameList;

  EXPECT_TRUE(KernelArgNameAPI.hasValue());

  EXPECT_EQ(expected.size(), KernelArgNameAPI.size());

  auto expected_it = expected.begin();
  for (auto addr_space : KernelArgNameAPI) {
    EXPECT_TRUE(addr_space == *expected_it);
    expected_it++;
  }
}

/// kernel_arg_access_qual

TEST_F(MetadataTest, Test_RetrieveKernelAccessQualFromNotAKernel) {
  auto pModule = GetTestModule();

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_plain_func"));

  EXPECT_FALSE(kernelMDApi.ArgAccessQualifierList.hasValue());
}

TEST_F(MetadataTest, Test_RetrieveKernelAccessQual) {
  auto pModule = GetTestModule();

  const std::vector<llvm::StringRef> expected = {"none", "none", "read_only"};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto KernelArgAPI = kernelMDApi.ArgAccessQualifierList;

  EXPECT_TRUE(KernelArgAPI.hasValue());

  auto actual = KernelArgAPI.getList();

  EXPECT_EQ(expected.size(), actual.size());

  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_TRUE(expected[i] == actual[i]);
  }
}

/// kernel_arg_type

TEST_F(MetadataTest, Test_RetrieveKernelArgType) {
  auto pModule = GetTestModule();

  const std::vector<llvm::StringRef> expected = {"float", "int*", "image2d_t"};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto KernelArgAPI = kernelMDApi.ArgTypeList;

  EXPECT_TRUE(KernelArgAPI.hasValue());

  auto actual = KernelArgAPI.getList();

  EXPECT_EQ(expected.size(), actual.size());

  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_TRUE(expected[i] == actual[i]);
  }
}

/// kernel_arg_base_type

TEST_F(MetadataTest, Test_RetrieveKernelArgBaseType) {
  auto pModule = GetTestModule();

  const std::vector<llvm::StringRef> expected = {"float", "int*", "image2d_t"};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto KernelArgAPI = kernelMDApi.ArgBaseTypeList;

  EXPECT_TRUE(KernelArgAPI.hasValue());

  auto actual = KernelArgAPI.getList();

  EXPECT_EQ(expected.size(), actual.size());

  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_TRUE(expected[i] == actual[i]);
  }
}

/// kernel_arg_type_qual

TEST_F(MetadataTest, Test_RetrieveKernelArgTypeQual) {
  auto pModule = GetTestModule();

  const std::vector<llvm::StringRef> expected = {"", "", ""};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto KernelArgAPI = kernelMDApi.ArgTypeQualifierList;

  EXPECT_TRUE(KernelArgAPI.hasValue());

  auto actual = KernelArgAPI.getList();

  EXPECT_EQ(expected.size(), actual.size());

  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_TRUE(expected[i] == actual[i]);
  }
}

TEST_F(MetadataTest, Test_UnsetKernelArgTypeQual) {
  auto pModule = GetTestModule();

  {
    KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

    auto KernelArgAPI = kernelMDApi.ArgTypeQualifierList;

    EXPECT_TRUE(KernelArgAPI.hasValue());

    KernelArgAPI.unset();
  }
  {
    KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

    auto KernelArgAPI = kernelMDApi.ArgTypeQualifierList;

    EXPECT_FALSE(KernelArgAPI.hasValue());
  }
}

/// work_group_size_hint

TEST_F(MetadataTest, Test_RetrieveWorkGroupSizeHint) {
  auto pModule = GetTestModule();

  const std::vector<int32_t> expected = {8, 16, 32};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto WorkGroupSizeHint = kernelMDApi.WorkGroupSizeHint;

  EXPECT_TRUE(WorkGroupSizeHint.hasValue());

  EXPECT_EQ(expected[0], WorkGroupSizeHint.getXDim());
  EXPECT_EQ(expected[1], WorkGroupSizeHint.getYDim());
  EXPECT_EQ(expected[2], WorkGroupSizeHint.getZDim());
}

/// reqd_work_group_size

TEST_F(MetadataTest, Test_RetrieveReqdWorkGroupSize) {
  auto pModule = GetTestModule();

  const std::vector<int32_t> expected = {1, 2, 4};

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto ReqdWorkGroupSize = kernelMDApi.ReqdWorkGroupSize;

  EXPECT_TRUE(ReqdWorkGroupSize.hasValue());

  EXPECT_EQ(expected[0], ReqdWorkGroupSize.getXDim());
  EXPECT_EQ(expected[1], ReqdWorkGroupSize.getYDim());
  EXPECT_EQ(expected[2], ReqdWorkGroupSize.getZDim());
}

/// vec_type_hint

TEST_F(MetadataTest, Test_RetrieveVecTypeHintFromNotAKernel) {
  auto pModule = GetTestModule();

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_plain_func"));

  EXPECT_FALSE(kernelMDApi.VecTypeHint.hasValue());
}

TEST_F(MetadataTest, Test_RetrieveVecTypeHint) {
  auto pModule = GetTestModule();

  const auto expectedTy = llvm::Type::getFloatTy(pModule->getContext());

  const int32_t expectedSignnes = 0;

  KernelMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  auto VecTypeHint = kernelMDApi.VecTypeHint;

  EXPECT_TRUE(VecTypeHint.hasValue());

  auto actualTy = VecTypeHint.getType();
  auto actualSignness = VecTypeHint.getSign();

  EXPECT_EQ(expectedTy, actualTy);
  EXPECT_EQ(expectedSignnes, actualSignness);
}

/// KernelList

TEST_F(MetadataTest, Test_KernelList) {
  auto pModule = GetTestModule();

  auto kernels = KernelList(pModule);

  // should not contain a plain function
  bool containsPlainFunc =
      std::find_if(kernels.begin(), kernels.end(), [](llvm::Function *F) {
        return F->getName() == "metatest_plain_func";
      }) != kernels.end();
  EXPECT_FALSE(containsPlainFunc);

  // should contain a kernel
  bool containsKernel =
      std::find_if(kernels.begin(), kernels.end(), [](llvm::Function *F) {
        return F->getName() == "metatest_kernel";
      }) != kernels.end();
  EXPECT_TRUE(containsKernel);
}

TEST_F(MetadataTest, Test_KernelListInternalContainer) {
  auto pModule = GetTestModule();

  auto kernels = KernelList(pModule);

  bool containsPlainFunc =
      std::find_if(kernels.begin(), kernels.end(), [](llvm::Function *F) {
        return F->getName() == "metatest_plain_func";
      }) != kernels.end();
  EXPECT_FALSE(containsPlainFunc);

  // should contain a kernel
  bool containsKernel =
      std::find_if(kernels.begin(), kernels.end(), [](llvm::Function *F) {
        return F->getName() == "metatest_kernel";
      }) != kernels.end();
  EXPECT_TRUE(containsKernel);
}

TEST_F(MetadataTest, Test_SetGetVectorizerWidth) {
  auto pModule = GetTestModule();

  const int32_t expected = 16;

  KernelInternalMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  {
    auto VectorizedWidth = kernelMDApi.VectorizedWidth;

    EXPECT_FALSE(VectorizedWidth.hasValue());

    VectorizedWidth.set(expected);
  }

  {
    auto VectorizedWidth = kernelMDApi.VectorizedWidth;

    EXPECT_TRUE(VectorizedWidth.hasValue());

    EXPECT_EQ(expected, VectorizedWidth.get());
  }
}

TEST_F(MetadataTest, Test_SetGetVectorizedKernel) {
  auto pModule = GetTestModule();

  auto kernel = pModule->getFunction("metatest_kernel");

  llvm::ValueToValueMapTy VMap;
  auto vectorized_kernel = llvm::CloneFunction(kernel, VMap);
  vectorized_kernel->setName("vectorized_kernel");

  KernelInternalMetadataAPI kernelMDApi(pModule->getFunction("metatest_kernel"));

  {
    auto VectorizedKernel = kernelMDApi.VectorizedKernel;

    EXPECT_FALSE(VectorizedKernel.hasValue());

    VectorizedKernel.set(vectorized_kernel);
  }

  {
    auto VectorizedKernel = kernelMDApi.VectorizedKernel;

    EXPECT_TRUE(VectorizedKernel.hasValue());

    EXPECT_EQ(vectorized_kernel, VectorizedKernel.get());
  }
}

TEST_F(MetadataTest, Test_ModuleSpirVersion) {
  auto pModule = GetTestModule();

  const std::vector<int32_t> expected = {1, 2};

  ModuleMetadataAPI MDApi(pModule);

  auto SpirVersionList = MDApi.SpirVersionList;

  EXPECT_TRUE(SpirVersionList.hasValue());

  bool passed = expected == std::vector<int32_t>{SpirVersionList.getItem(0),
                                                 SpirVersionList.getItem(1)};

  EXPECT_TRUE(passed);
}

TEST_F(MetadataTest, Test_ModuleGlobalVariableTotalSize) {
  auto pModule = GetTestModule();

  const int32_t expected = 64;

  {
    ModuleInternalMetadataAPI MDApi(pModule);

    auto GlobalVariableTotalSize = MDApi.GlobalVariableTotalSize;

    EXPECT_FALSE(GlobalVariableTotalSize.hasValue());

    GlobalVariableTotalSize.set(expected);
  }

  {
    ModuleInternalMetadataAPI MDApi(pModule);

    auto GlobalVariableTotalSize = MDApi.GlobalVariableTotalSize;

    EXPECT_TRUE(GlobalVariableTotalSize.hasValue());

    EXPECT_EQ(expected, GlobalVariableTotalSize.get());
  }
}

// save the list to Metadata.

TEST_F(MetadataTest, Test_SaveKernelArgNamesMetadata) {
  auto pModule = GetTestModule();

  const KernelMetadataAPI::ArgNameListTy::vector_type
    expected = {"foo", "bar", "baz"};

  {
    auto pKernel = pModule->getFunction("metatest_kernel");
    KernelMetadataAPI kernelMDApi(pKernel);

    auto ArgNameList = kernelMDApi.ArgNameList;

    EXPECT_TRUE(ArgNameList.hasValue());

    ArgNameList.set(expected);
  }

  {
    auto pKernel = pModule->getFunction("metatest_kernel");
    KernelMetadataAPI kernelMDApi(pKernel);

    auto ArgNameList = kernelMDApi.ArgNameList;

    EXPECT_TRUE(ArgNameList.hasValue());

    auto actual = ArgNameList.getList();

    EXPECT_EQ(expected.size(), actual.size());

    for (size_t i = 0; i < actual.size(); i++) {
      EXPECT_TRUE(expected[i] == actual[i]);
    }
  }
}

TEST_F(MetadataTest, Test_ReadSaveThenReadMetadata)
{
  // check that dirty flag in cache implementation works fine

  auto pModule = GetTestModule();

  const KernelMetadataAPI::ArgNameListTy::vector_type expected =
  { "foo", "bar", "baz" };

  auto pKernel = pModule->getFunction("metatest_kernel");
  KernelMetadataAPI kernelMDApi(pKernel);
  auto ArgNameList = kernelMDApi.ArgNameList;

  // read
  EXPECT_TRUE(ArgNameList.hasValue());

  // set
  ArgNameList.set(expected);

  // read
  EXPECT_TRUE(ArgNameList.hasValue());

  auto actual = ArgNameList.getList();

  EXPECT_EQ(expected.size(), actual.size());

  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_TRUE(expected[i] == actual[i]);
  }
}

TEST_F(MetadataTest, Test_SaveModuleVersionListMetadata) {
  auto pModule = GetTestModule();

  const ModuleMetadataAPI::OpenCLVersionListTy::vector_type
    expected = {2, 0};

  {
    ModuleMetadataAPI ModuleMetadataAPI(pModule);

    auto OpenCLVersionList = ModuleMetadataAPI.OpenCLVersionList;

    EXPECT_TRUE(OpenCLVersionList.hasValue());

    OpenCLVersionList.set(expected);
  }

  {
    ModuleMetadataAPI ModuleMetadataAPI(pModule);

    auto OpenCLVersionList = ModuleMetadataAPI.OpenCLVersionList;

    EXPECT_TRUE(OpenCLVersionList.hasValue());

    auto actual = OpenCLVersionList.getList();

    EXPECT_EQ(expected.size(), actual.size());

    for (size_t i = 0; i < actual.size(); i++) {
      EXPECT_EQ(expected[i], actual[i]);
    }
  }
}

TEST_F(MetadataTest, Test_AttachFunctionStatistics) {
  auto pModule = GetTestModule();
  auto pKernel = pModule->getFunction("metatest_kernel");

  FunctionStatMetadataAPI::StatListTy expectedList = {
      {"stat1", 4, "desc1"}, {"stat2", 2, "desc2"}};

  FunctionStatMetadataAPI::set(*pKernel, expectedList);

  FunctionStatMetadataAPI::StatListTy actualList;
  for (const auto &stat : FunctionStatMetadataAPI(*pKernel)) {
    actualList.push_back(stat);
  }

  EXPECT_EQ(expectedList.size(), actualList.size());

  auto sortPred = [](const FunctionStatMetadataAPI::StatTy &a,
                     const FunctionStatMetadataAPI::StatTy &b) {
    return a.Name < b.Name;
  };

  std::sort(expectedList.begin(), expectedList.end(), sortPred);
  std::sort(actualList.begin(), actualList.end(), sortPred);

  for (size_t i = 0; i < actualList.size(); i++) {
    EXPECT_EQ(expectedList[i].Name, actualList[i].Name);
    EXPECT_EQ(expectedList[i].Value, actualList[i].Value);
    EXPECT_EQ(expectedList[i].Description, actualList[i].Description);
  }
}

TEST_F(MetadataTest, Test_RemoveFunctionStatistics) {
  auto pModule = GetTestModule();
  auto &kernel = *pModule->getFunction("metatest_kernel");

  FunctionStatMetadataAPI::StatListTy statList = {{"stat1", 4, "desc1"},
                                                      {"stat2", 2, "desc2"}};

  FunctionStatMetadataAPI::set(kernel, statList);
  FunctionStatMetadataAPI::remove(kernel);

  FunctionStatMetadataAPI::StatListTy actualList;
  {
    for (const auto &stat : FunctionStatMetadataAPI(kernel)) {
      actualList.push_back(stat);
    }
  }

  EXPECT_TRUE(actualList.empty());
}

TEST_F(MetadataTest, Test_CopyFunctionStatistics) {
  auto pModule = GetTestModule();
  auto &kernel = *pModule->getFunction("metatest_kernel");
  auto &func = *pModule->getFunction("metatest_plain_func");

  FunctionStatMetadataAPI::StatListTy expectedList = {
      {"stat1", 4, "desc1"}, {"stat2", 2, "desc2"}};

  { FunctionStatMetadataAPI::set(kernel, expectedList); }
  { FunctionStatMetadataAPI::copy(kernel, func); }

  FunctionStatMetadataAPI::StatListTy actualList;
  {
    for (const auto &stat : FunctionStatMetadataAPI(func)) {
      actualList.push_back(stat);
    }
  }

  EXPECT_EQ(expectedList.size(), actualList.size());

  auto sortPred = [](const FunctionStatMetadataAPI::StatTy &a,
                     const FunctionStatMetadataAPI::StatTy &b) {
    return a.Name < b.Name;
  };

  std::sort(expectedList.begin(), expectedList.end(), sortPred);
  std::sort(actualList.begin(), actualList.end(), sortPred);

  for (size_t i = 0; i < actualList.size(); i++) {
    EXPECT_EQ(expectedList[i].Name, actualList[i].Name);
    EXPECT_EQ(expectedList[i].Value, actualList[i].Value);
    EXPECT_EQ(expectedList[i].Description, actualList[i].Description);
  }
}

TEST_F(MetadataTest, Test_MoveFunctionStatistics) {

  auto pModule = GetTestModule();
  auto &kernel = *pModule->getFunction("metatest_kernel");
  auto &func = *pModule->getFunction("metatest_plain_func");

  FunctionStatMetadataAPI::StatListTy expectedList = {
      {"stat1", 4, "desc1"}, {"stat2", 2, "desc2"}};

  { FunctionStatMetadataAPI::set(kernel, expectedList); }
  { FunctionStatMetadataAPI::move(kernel, func); }

  // check that kernel does not have any statistics attached
  EXPECT_TRUE(FunctionStatMetadataAPI(kernel).empty());

  // check that func has the same exact statistics kernel had before
  {
    FunctionStatMetadataAPI::StatListTy actualList;
    {
      for (const auto &stat : FunctionStatMetadataAPI(func)) {
        actualList.push_back(stat);
      }
    }

    EXPECT_EQ(expectedList.size(), actualList.size());

    auto sortPred = [](const FunctionStatMetadataAPI::StatTy &a,
                       const FunctionStatMetadataAPI::StatTy &b) {
      return a.Name < b.Name;
    };

    std::sort(expectedList.begin(), expectedList.end(), sortPred);
    std::sort(actualList.begin(), actualList.end(), sortPred);

    for (size_t i = 0; i < actualList.size(); i++) {
      EXPECT_EQ(expectedList[i].Name, actualList[i].Name);
      EXPECT_EQ(expectedList[i].Value, actualList[i].Value);
      EXPECT_EQ(expectedList[i].Description, actualList[i].Description);
    }
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
