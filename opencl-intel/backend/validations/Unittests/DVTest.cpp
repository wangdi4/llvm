// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#include "BufferContainerList.h"
#include "OCLBuilder.h"
#include "OCLKernelDataGenerator.h"
#include "cl_types.h"
#include "gtest_wrapper.h"
#include "opencl_clang.h"
#include "tinyxml_wrapper.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include <fstream>

using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL::ClangFE;
using namespace Validation;

enum TestSamplerType {
  NONE_FALSE_NEAREST =
      CLK_ADDRESS_NONE | CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST,
  CLAMP_FALSE_NEAREST =
      CLK_ADDRESS_CLAMP | CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST,
  CLAMPTOEDGE_FALSE_NEAREST = CLK_ADDRESS_CLAMP_TO_EDGE |
                              CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST,
  REPEAT_FALSE_NEAREST =
      CLK_ADDRESS_REPEAT | CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST,
  MIRRORED_FALSE_NEAREST = CLK_ADDRESS_MIRRORED_REPEAT |
                           CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST,
  NONE_TRUE_NEAREST =
      CLK_ADDRESS_NONE | CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_NEAREST,
  CLAMP_TRUE_NEAREST =
      CLK_ADDRESS_CLAMP | CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_NEAREST,
  CLAMPTOEDGE_TRUE_NEAREST = CLK_ADDRESS_CLAMP_TO_EDGE |
                             CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_NEAREST,
  REPEAT_TRUE_NEAREST =
      CLK_ADDRESS_REPEAT | CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_NEAREST,
  MIRRORED_TRUE_NEAREST = CLK_ADDRESS_MIRRORED_REPEAT |
                          CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_NEAREST,
  NONE_FALSE_LINEAR =
      CLK_ADDRESS_NONE | CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR,
  CLAMP_FALSE_LINEAR =
      CLK_ADDRESS_CLAMP | CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR,
  CLAMPTOEDGE_FALSE_LINEAR = CLK_ADDRESS_CLAMP_TO_EDGE |
                             CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR,
  REPEAT_FALSE_LINEAR =
      CLK_ADDRESS_REPEAT | CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR,
  MIRRORED_FALSE_LINEAR = CLK_ADDRESS_MIRRORED_REPEAT |
                          CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR,
  NONE_TRUE_LINEAR =
      CLK_ADDRESS_NONE | CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_LINEAR,
  CLAMP_TRUE_LINEAR =
      CLK_ADDRESS_CLAMP | CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_LINEAR,
  CLAMPTOEDGE_TRUE_LINEAR = CLK_ADDRESS_CLAMP_TO_EDGE |
                            CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_LINEAR,
  REPEAT_TRUE_LINEAR =
      CLK_ADDRESS_REPEAT | CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_LINEAR,
  MIRRORED_TRUE_LINEAR = CLK_ADDRESS_MIRRORED_REPEAT |
                         CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_LINEAR,
  SAMPLER_UNDEFINED = 0xFF
};

// according to googleTest's recomendations, DataVersion.cpp included
// in order to test static funtions
#include "DataVersion.cpp"

TEST(DataVersionTest, ConvertSample_v0_v1) {
  unsigned int sampler;

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_NONE |
                           CL_DEV_SAMPLER_FILTER_NEAREST);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)NONE_FALSE_NEAREST, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_NONE |
                           CL_DEV_SAMPLER_FILTER_LINEAR);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)NONE_FALSE_LINEAR, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
                           CL_DEV_SAMPLER_FILTER_NEAREST);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)CLAMP_FALSE_NEAREST, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
                           CL_DEV_SAMPLER_FILTER_LINEAR);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)CLAMP_FALSE_LINEAR, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
                           CL_DEV_SAMPLER_FILTER_NEAREST);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)CLAMPTOEDGE_FALSE_NEAREST, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
                           CL_DEV_SAMPLER_FILTER_LINEAR);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)CLAMPTOEDGE_FALSE_LINEAR, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_REPEAT |
                           CL_DEV_SAMPLER_FILTER_NEAREST);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)REPEAT_FALSE_NEAREST, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_REPEAT |
                           CL_DEV_SAMPLER_FILTER_LINEAR);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)REPEAT_FALSE_LINEAR, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT |
                           CL_DEV_SAMPLER_FILTER_NEAREST);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)MIRRORED_FALSE_NEAREST, sampler);

  sampler = (unsigned int)(CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT |
                           CL_DEV_SAMPLER_FILTER_LINEAR);
  convertSampler(&sampler);
  EXPECT_EQ((unsigned int)MIRRORED_FALSE_LINEAR, sampler);
}

static std::string buildLibName(const char *s) {
  std::stringstream ret;
#ifdef _WIN32
  ret << s << ".dll";
#else
  ret << "lib" << s << ".so";
#endif
  return ret.str();
}

static void testFindSamplers(std::string sourceIn,
                             std::vector<unsigned int> &samplerIndxs,
                             std::string kernelName) {
  const char *source = sourceIn.c_str();
  OCLBuilder &builder = OCLBuilder::Instance();

  IOCLFEBinaryResult *pBinary;

  try {
    pBinary = builder.withSource(source).createCompiler().build();
#if !defined(_WIN32)
    builder.close();
#endif
  } catch (Validation::Exception::OperationFailed ex) {
    std::cerr << ex.what();
    GTEST_FAIL();
  }

  const void *binaryBuff;
  size_t binaryBuffsize;

  binaryBuff = reinterpret_cast<char *>(const_cast<void *>(pBinary->GetIR()));
  binaryBuffsize = pBinary->GetIRSize();

  llvm::SMDiagnostic err;
  llvm::LLVMContext ctxt;
  // retrieving the name of the kernel
  llvm::StringRef inData((const char *)binaryBuff, binaryBuffsize);
  std::unique_ptr<llvm::MemoryBuffer> pBuffer =
      llvm::MemoryBuffer::getMemBuffer(inData, "", false);
  std::unique_ptr<llvm::Module> pModule(
      llvm::parseIR(pBuffer->getMemBufferRef(), err, ctxt));

  pBinary->Release();

  auto pKernel = pModule->getFunction(kernelName);
  assert(pKernel && "No function with given name found");
  assert(pKernel->getCallingConv() == llvm::CallingConv::SPIR_KERNEL &&
         "Given function isn't a kernel");

  // this function is tested
  samplerIndxs = FindSamplers(pKernel);
}

TEST(DataVersionTest, DISABLED_find_type_sampler_t) {
  // test case: no sampler_t types among the kernel's arguments
  std::vector<unsigned int> samplerIndxs;
  std::string kernelName("add");
  std::string source = std::string("__kernel void ") + kernelName +
                       std::string(" (int* a, int *b, __global int *c){"
                                   "int tid = get_global_id(0);"
                                   "c[tid] = b[tid];}");

  testFindSamplers(source, samplerIndxs, kernelName);
  EXPECT_EQ(samplerIndxs.size(), (unsigned int)0);

  // test case: one sampler_t type at position 0
  std::string source1 = std::string("__kernel void ") + kernelName +
                        std::string("(sampler_t a, int *b, __global int *c){"
                                    "int tid = get_global_id(0);"
                                    "c[tid] = b[tid];}");

  testFindSamplers(source1, samplerIndxs, kernelName);
  EXPECT_EQ(samplerIndxs.size(), (unsigned int)1);
  if (samplerIndxs.size() == (unsigned int)1) {
    EXPECT_EQ(samplerIndxs[0], (unsigned int)0);
  }

  // test case: two sampler_t types at positions 1 and 2
  std::string source2 = std::string("__kernel void ") + kernelName +
                        std::string("(int * a, sampler_t b, sampler_t c){"
                                    "int tid = get_global_id(0);"
                                    "a[tid] = 0;}");

  testFindSamplers(source2, samplerIndxs, kernelName);
  EXPECT_EQ(samplerIndxs.size(), (unsigned int)2);
  if (samplerIndxs.size() == (unsigned int)2) {
    EXPECT_EQ(samplerIndxs[0], (unsigned int)1);
    EXPECT_EQ(samplerIndxs[1], (unsigned int)2);
  }
}

TEST(DataVersionTest, DISABLED_ConvertData) {
  // the source code in this tests consists of 3 kernels: thirst and second has
  // samplers as parameters at positions 0 and 1 accordingly. the third kernel
  // has no samplers as parameters. The tested function ConvertData must convert
  // samplers for kernelName1 and kernelName2 and must not change data for
  // kernelName3

  std::string kernelName1 = "sample_kernel_1D_1";
  std::string kernelName2 = "sample_kernel_1D_2";
  std::string kernelName3 = "sample_kernel_1D_3";

  std::vector<std::string> vecKernelNames;

  vecKernelNames.push_back(kernelName1);
  vecKernelNames.push_back(kernelName2);
  vecKernelNames.push_back(kernelName3);

  std::string source1 = std::string("__kernel void ") + kernelName1 +
                        std::string("( int input0, int input1, int input2) {"
                                    " int res = input0 + input1 + input2; }");

  std::string source2 =
      std::string("__kernel void ") + kernelName2 +
      std::string(
          "( read_only image1d_t input, sampler_t imageSampler) {"
          " int tidX = get_global_id(0);"
          " int offset = tidX;"
          " float coord = (float)offset;"
          " float4 results = read_imagef( input, imageSampler, coord ); }");

  // second kernel's arguments placed in the other order comparing to the first
  // kernel
  std::string source3 =
      std::string("__kernel void ") + kernelName3 +
      std::string(
          "( sampler_t imageSampler, read_only image1d_t input) {"
          " int tidX = get_global_id(0);"
          " int offset = tidX;"
          " float coord = (float)offset;"
          " float4 results = read_imagef( input, imageSampler, coord ); }");

  std::string source = source1 + source2 + source3;

  std::vector<std::string> vecConfigs;

  std::string ConfigFile1 = "<?xml version=\"1.0\" ?>\
        <OCLKernelDataGeneratorConfig>\
            <Seed>10232</Seed>\
            <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"i32\">\
            </GeneratorConfiguration>\
                <Value>0</Value>\
            <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"i32\">\
            </GeneratorConfiguration>\
                <Value>0</Value>\
            <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"i32\">\
                <Value>0</Value>\
            </GeneratorConfiguration>\
        </OCLKernelDataGeneratorConfig>";

  std::string ConfigFile2 = "<?xml version=\"1.0\" ?>\
        <OCLKernelDataGeneratorConfig>\
            <Seed>10232</Seed>\
            <GeneratorConfiguration Name=\"ImageRandomGenerator\">\
                <ImageType>CL_MEM_OBJECT_IMAGE1D</ImageType>\
                <ChannelDataType>CL_UNORM_INT8</ChannelDataType>\
                <ChannelOrder>CL_sRGBA</ChannelOrder>\
                <Size>64</Size>\
            </GeneratorConfiguration>\
            <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"i32\">\
                <Value>0</Value>\
            </GeneratorConfiguration>\
        </OCLKernelDataGeneratorConfig>";

  std::string ConfigFile3 = "<?xml version=\"1.0\" ?>\
        <OCLKernelDataGeneratorConfig>\
            <Seed>10232</Seed>\
            <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"i32\">\
                <Value>0</Value>\
            </GeneratorConfiguration>\
            <GeneratorConfiguration Name=\"ImageRandomGenerator\">\
                <ImageType>CL_MEM_OBJECT_IMAGE1D</ImageType>\
                <ChannelDataType>CL_UNORM_INT8</ChannelDataType>\
                <ChannelOrder>CL_sRGBA</ChannelOrder>\
                <Size>64</Size>\
            </GeneratorConfiguration>\
        </OCLKernelDataGeneratorConfig>";

  vecConfigs.push_back(ConfigFile1);
  vecConfigs.push_back(ConfigFile2);
  vecConfigs.push_back(ConfigFile3);

  OCLBuilder &builder = OCLBuilder::Instance();

  IOCLFEBinaryResult *pBinary;

  try {
    pBinary = builder.withSource(source.c_str()).createCompiler().build();
#if !defined(_WIN32)
    builder.close();
#endif
  } catch (Validation::Exception::OperationFailed ex) {
    std::cerr << ex.what();
    GTEST_FAIL();
  }

  const void *binaryBuff;
  size_t binaryBuffsize;

  binaryBuff = reinterpret_cast<char *>(const_cast<void *>(pBinary->GetIR()));
  binaryBuffsize = pBinary->GetIRSize();

  llvm::SMDiagnostic err;
  llvm::LLVMContext ctxt;
  // retrieving the name of the kernel
  llvm::StringRef inData((const char *)binaryBuff, binaryBuffsize);
  std::unique_ptr<llvm::MemoryBuffer> pBuffer =
      llvm::MemoryBuffer::getMemBuffer(inData, "", false);
  std::unique_ptr<llvm::Module> pModule(
      llvm::parseIR(pBuffer->getMemBufferRef(), err, ctxt));
  pBinary->Release();

  assert(vecKernelNames.size() == vecConfigs.size() &&
         "Number of kernels differs than number of configs for them");
  for (unsigned j = 0, e = vecKernelNames.size(); j < e; ++j) {

    auto *F = pModule->getFunction(vecKernelNames[j]);
    assert(F && F->getCallingConv() == llvm::CallingConv::SPIR_KERNEL &&
           "No valid kernel found");

    BufferContainerList input;
    OpenCLKernelArgumentsParser parser;
    OCLKernelArgumentsList argDescriptions =
        parser.KernelArgumentsParser(vecKernelNames[j], pModule.get());

    std::fstream io_file;
    io_file.open("testConvertSampler.cfg", std::fstream::out);
    io_file << vecConfigs[j];
    io_file.close();

    TiXmlDocument config(std::string("testConvertSampler.cfg"));
    ASSERT_TRUE(config.LoadFile());
    OCLKernelDataGeneratorConfig cfg(&config);

    OCLKernelDataGenerator gen(argDescriptions, cfg);
    gen.Read(&input);

    IBufferContainerList *pContainer = &input;
    pContainer->SetDataVersion(0);

    const unsigned int arrSamplersLen = 10;

    unsigned int arrSamplersIn[arrSamplersLen] = {
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_NONE |
                       CL_DEV_SAMPLER_FILTER_NEAREST),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_NONE |
                       CL_DEV_SAMPLER_FILTER_LINEAR),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
                       CL_DEV_SAMPLER_FILTER_NEAREST),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
                       CL_DEV_SAMPLER_FILTER_LINEAR),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
                       CL_DEV_SAMPLER_FILTER_NEAREST),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
                       CL_DEV_SAMPLER_FILTER_LINEAR),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_REPEAT |
                       CL_DEV_SAMPLER_FILTER_NEAREST),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_REPEAT |
                       CL_DEV_SAMPLER_FILTER_LINEAR),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT |
                       CL_DEV_SAMPLER_FILTER_NEAREST),
        (unsigned int)(CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT |
                       CL_DEV_SAMPLER_FILTER_LINEAR)};

    unsigned int arrSamplersOut[arrSamplersLen] = {
        (unsigned int)NONE_FALSE_NEAREST,
        (unsigned int)NONE_FALSE_LINEAR,
        (unsigned int)CLAMP_FALSE_NEAREST,
        (unsigned int)CLAMP_FALSE_LINEAR,
        (unsigned int)CLAMPTOEDGE_FALSE_NEAREST,
        (unsigned int)CLAMPTOEDGE_FALSE_LINEAR,
        (unsigned int)REPEAT_FALSE_NEAREST,
        (unsigned int)REPEAT_FALSE_LINEAR,
        (unsigned int)MIRRORED_FALSE_NEAREST,
        (unsigned int)MIRRORED_FALSE_LINEAR};

    // for the first kernel in this test GetMemoryObject(j) int not a sampler,
    // it is just integer so it must not be changed by ConvertData, the number
    // of position is 2, for the second kernel the samlper has position number
    // 1, for the third kernel sampler has position number 0
    unsigned int *sampler = NULL;
    switch (j) {
    case 0:
      sampler = (uint32_t *)input.GetBufferContainer(0)
                    ->GetMemoryObject(2)
                    ->GetDataPtr();
      break;
    case 1:
      sampler = (uint32_t *)input.GetBufferContainer(0)
                    ->GetMemoryObject(1)
                    ->GetDataPtr();
      break;
    case 2:
      sampler = (uint32_t *)input.GetBufferContainer(0)
                    ->GetMemoryObject(0)
                    ->GetDataPtr();
      break;
    default:
      break;
    }

    if (sampler == NULL)
      GTEST_FAIL();

    for (unsigned int i = 0; i < arrSamplersLen; i++) {
      *sampler = arrSamplersIn[i];

      try {
        DataVersion::ConvertData(&input, F);
      } catch (Exception::InvalidArgument ex) {
        std::cerr << ex.what();
        GTEST_FAIL();
      }
      // for second and third kernels samplers must be converted, for the thist
      // one must not be converted
      if (j == 0)
        EXPECT_EQ(arrSamplersIn[i], *sampler);
      else
        EXPECT_EQ(arrSamplersOut[i], *sampler);
    }
  }
}
