// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "OCLKernelDataGenerator.h"
#include "Buffer.h"
#include "BufferContainerList.h"
#include "FloatOperations.h"
#include "OpenCLKernelArgumentsParser.h"
#include "RandomUniformProvider.h"
#include "gtest_wrapper.h"
#include "tinyxml_wrapper.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <time.h>
#include <vector>

// #include "crtdbg.h"
//  static long _crt_test_leaks =  _CrtSetBreakAlloc(15313);
//  using namespace llvm;
using namespace Validation;

namespace {

std::string stringForParser(std::string input) {
  std::string output;
  int sizeInput = (int)input.size();
  int isVar = 0;
  for (int i = 0; i < sizeInput; i++) {
    if (input[i] == '%' && input.find(std::string("%opencl.image2d_t.0*"), i) ==
                               std::string::npos) {
      isVar = 1;
      continue;
    }
    if (!(isVar == 1) || input[i] == ',') // if input[i] is not letter
    {
      output += input[i];
    }
    if (input[i] == ',')
      isVar = 0;
  }
  return output;
}

void createModule(llvm::LLVMContext &Context, llvm::Module *&M,
                  std::string kernelName, std::string params) {
  std::string program = std::string("%opencl.image2d_t.0 = type opaque\n") +
                        std::string("define void @") + kernelName +
                        std::string("(") + params +
                        std::string(") { "
                                    "  ret void "
                                    "} "
                                    "!opencl.kernels = !{!0}"
                                    "!0 = !{void (") +
                        stringForParser(params) + std::string(")* @") +
                        kernelName + std::string("}");
  std::cerr << program << std::endl;

  llvm::SMDiagnostic Error;
  auto pM = parseAssemblyString(program, Error, Context);
  std::string errMsg;
  llvm::raw_string_ostream os(errMsg);
  Error.print("", os);
  EXPECT_TRUE(pM != nullptr) << os.str();
  M = pM.release();
}

TEST(OCLKernelDataGenerator, TestOfSeed) {
  uint64_t t = time(NULL);
  float F[1000], F2[1000];
  double D[1000], D2[1000];
  uint64_t I[1000];
  for (int i = 0; i < 1000; i++) {
    RandomUniformProvider a(t + i);
    F[i] = a.sample_f32();
    D[i] = a.sample_f64();
    F2[i] = a.sample_f32_binary();
    D2[i] = a.sample_f64_binary();
    I[i] = a.sample_u64();
  }
  for (int i = 0; i < 1000; i++) {
    RandomUniformProvider b(t + i);
    EXPECT_EQ(F[i], b.sample_f32());
    EXPECT_EQ(D[i], b.sample_f64());
    EXPECT_EQ(F2[i], b.sample_f32_binary());
    EXPECT_EQ(D2[i], b.sample_f64_binary());
    EXPECT_EQ(I[i], b.sample_u64());
  }
}
TEST(OCLKernelDataGenerator, TestOfSeedv2) {
  const uint64_t numOfIterations = 1000;
  for (uint64_t i = 0; i < numOfIterations; i++) {
    float f32_1, f32_2;
    double f64_1, f64_2;
    uint8_t u8_1, u8_2;
    uint16_t u16_1, u16_2;
    uint32_t u32_1, u32_2;
    uint64_t u64_1, u64_2;

    time_t t = time(NULL);

    RandomUniformProvider a1(t);
    RandomUniformProvider a2(t);
    for (uint64_t j = 0; j < numOfIterations; ++j) {
      f32_1 = a1.sample_f32();
      f32_2 = a2.sample_f32();
      EXPECT_EQ(f32_1, f32_2);

      f64_1 = a1.sample_f64();
      f64_2 = a2.sample_f64();
      EXPECT_EQ(f64_1, f64_2);

      u8_1 = a1.sample_u64();
      u8_2 = a2.sample_u64();
      EXPECT_EQ(u8_1, u8_2);

      u16_1 = a1.sample_u64();
      u16_2 = a2.sample_u64();
      EXPECT_EQ(u16_1, u16_2);

      u32_1 = a1.sample_u64();
      u32_2 = a2.sample_u64();
      EXPECT_EQ(u32_1, u32_2);

      u64_1 = a1.sample_u64();
      u64_2 = a2.sample_u64();
      EXPECT_EQ(u64_1, u64_2);
    }
  }
}
TEST(OCLKernelDataGenerator, TestOfFloatDouble) {
  uint64_t t = time(NULL);
  float F[1000], F2[1000];
  double D[1000], D2[1000];
  for (int i = 0; i < 1000; i++) {
    RandomUniformProvider a(t + i);
    F[i] = a.sample_f32();
    D[i] = a.sample_f64();
    F2[i] = a.sample_f32_binary();
    D2[i] = a.sample_f64_binary();
  }
  for (int i = 0; i < 1000; i++) {
    EXPECT_FALSE((Utils::IsNaN(D[i])) || (Utils::IsInf(D[i])) ||
                 (Utils::IsDenorm(D[i])));
    EXPECT_FALSE((Utils::IsNaN(D2[i])) || (Utils::IsInf(D2[i])) ||
                 (Utils::IsDenorm(D2[i])));
    EXPECT_FALSE((Utils::IsNaN(F[i])) || (Utils::IsInf(F[i])) ||
                 (Utils::IsDenorm(F[i])));
    EXPECT_FALSE((Utils::IsNaN(F2[i])) || (Utils::IsInf(F2[i])) ||
                 (Utils::IsDenorm(F2[i])));
  }
}

TEST(OCLKernelDataGenerator, TestOfRange) {
  for (int i = 0; i < 200; i++) {
    RandomUniformProvider a(time(NULL) + i);
    uint64_t tLong = a.sample_u64(100, 111);
    EXPECT_TRUE(tLong >= 100 && tLong <= 111);
    float tFloat = a.sample_f32(0, 111.111f);
    EXPECT_TRUE(tFloat >= 0 && tFloat <= 111.111);
    double tDouble = a.sample_f64(-5.11, 12.001);
    EXPECT_TRUE(tDouble >= -5.11 && tDouble <= 12.001);
  }
}

// test uniform distribution
TEST(OCLKernelDataGenerator, TestOfUniformDistribution) {
  std::vector<double> a(100000);
  // uint64_t t=time(NULL);
  for (int i = 0; i < 100000; i++) {
    RandomUniformProvider tmp(time(NULL) + i);
    a[i] = tmp.sample_f64(0.0, 10000.0);
  }
  std::sort(a.begin(), a.end());
  double diapason = 10000 / 10;
  int sizeOnDiapason = 100000 / 10;
  double currentDiapason = diapason;
  double x = 0;
  for (int i = 0, k = 0; i < 10; i++) {
    int count = 0;
    while (k < 100000 && a[k] < currentDiapason) {
      k++;
      count++;
    }
    x += double(count - sizeOnDiapason) * double(count - sizeOnDiapason) /
         double(sizeOnDiapason);
    currentDiapason += diapason;
  }
  EXPECT_TRUE(x < 12.242);
}
// args - float and double
TEST(OCLKernelDataGenerator, simpleTestOfOCLGenerator) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfsimpleArguments";
  createModule(Context, M, kernelName, "float %a, double %b");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig dgConfig;
  dgConfig.getConfigVector().resize(2);

  // create config for 0th argument
  BufferConstGeneratorConfig<float> *pConstF32 =
      static_cast<BufferConstGeneratorConfig<float> *>(
          GeneratorConfigFactory::create("BufferConstGeneratorf32"));
  pConstF32->SetFillValue(3.0f);
  dgConfig.getConfigVector()[0] = pConstF32;

  // create config for 1th argument
  BufferRandomGeneratorConfig<double> *pConstF64 =
      static_cast<BufferRandomGeneratorConfig<double> *>(
          GeneratorConfigFactory::create("BufferRandomGeneratorf64"));
  dgConfig.getConfigVector()[1] = pConstF64;

  // create generator
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  BufferContainerList bcl;
  // generate
  gen.Read(&bcl);

  IBufferContainer *bc = bcl.GetBufferContainer(0);
  IMemoryObject *buff = bc->GetMemoryObject(0);
  BufferAccessor<float> acc(*buff);
  float &t_float = acc.GetElem(0, 0);
  EXPECT_EQ(3.0f, t_float);
  BufferDesc buffDsc = GetBufferDescription(buff->GetMemoryObjectDesc());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TFLOAT, elemDesc.GetType());

  buff = bc->GetMemoryObject(1);
  buffDsc = GetBufferDescription(buff->GetMemoryObjectDesc());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TDOUBLE, elemDesc.GetType());
  delete M;
}
// args - flaot4, long16
TEST(OCLKernelDataGenerator, vectorTestOfOCLGenerator) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfVectorArguments";
  createModule(Context, M, kernelName, "<4 x float> %a,  <16 x i64> %b");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig dgConfig;
  dgConfig.getConfigVector().resize(2);

  // create config for 0th argument
  BufferConstGeneratorConfig<float> *pConstF32 =
      static_cast<BufferConstGeneratorConfig<float> *>(
          GeneratorConfigFactory::create("BufferConstGeneratorf32"));
  pConstF32->SetFillValue(5.0f);
  dgConfig.getConfigVector()[0] = pConstF32;

  // create config for 1th argument
  BufferConstGeneratorConfig<uint64_t> *pConstI64R =
      static_cast<BufferConstGeneratorConfig<uint64_t> *>(
          GeneratorConfigFactory::create("BufferConstGeneratori64"));
  pConstI64R->SetFillValue(uint64_t(123456));
  dgConfig.getConfigVector()[1] = pConstI64R;

  // create generator
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  BufferContainerList bcl;
  // generate
  gen.Read(&bcl);

  IBufferContainer *bc = bcl.GetBufferContainer(0);
  IMemoryObject *buff = bc->GetMemoryObject(0);
  BufferAccessor<float> acc(*buff);
  float *t_float = &(acc.GetElem(0, 0));
  for (int i = 0; i < 4; ++i)
    EXPECT_EQ(5.0f, *(t_float++));

  BufferDesc buffDsc = GetBufferDescription(buff->GetMemoryObjectDesc());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  EXPECT_EQ(uint32_t(4), elemDesc.GetNumberOfElements());
  EXPECT_EQ(TFLOAT, elemDesc.GetSubTypeDesc(0).GetType());

  uint64_t *t_int64;

  buff = bc->GetMemoryObject(1);
  BufferAccessor<uint64_t> acc1(*buff);
  t_int64 = &(acc1.GetElem(0, 0));
  for (int i = 0; i < 16; ++i)
    EXPECT_EQ(uint64_t(123456), *(t_int64++));

  buffDsc = GetBufferDescription(buff->GetMemoryObjectDesc());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  EXPECT_EQ(uint32_t(16), elemDesc.GetNumberOfElements());
  EXPECT_EQ(TLONG, elemDesc.GetSubTypeDesc(0).GetType());
  delete M;
}
// typedef struct { float4 a, struct {int a}  b} a
TEST(OCLKernelDataGenerator, structureTestOfOCLGenerator) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfstructureArguments";
  createModule(Context, M, kernelName, "{ < 4 x float > , { i32 } } %a ");

  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig dgConfig;
  dgConfig.getConfigVector().resize(1);

  // create config for 1th argument
  BufferStructureGeneratorConfig *pConstIS =
      static_cast<BufferStructureGeneratorConfig *>(
          GeneratorConfigFactory::create("BufferStructureGenerator"));
  dgConfig.getConfigVector()[0] = pConstIS;
  pConstIS->getConfigVector().resize(2);

  BufferConstGeneratorConfig<float> *constf32 =
      static_cast<BufferConstGeneratorConfig<float> *>(
          GeneratorConfigFactory::create("BufferConstGeneratorf32"));
  constf32->SetFillValue(1.0f);
  pConstIS->getConfigVector()[0] = constf32;

  BufferStructureGeneratorConfig *pRandBuff =
      static_cast<BufferStructureGeneratorConfig *>(
          GeneratorConfigFactory::create("BufferStructureGenerator"));
  pConstIS->getConfigVector()[1] = pRandBuff;

  pRandBuff->getConfigVector().resize(1);

  BufferConstGeneratorConfig<int32_t> *pConstI32 =
      static_cast<BufferConstGeneratorConfig<int32_t> *>(
          GeneratorConfigFactory::create("BufferConstGeneratori32"));
  pConstI32->SetFillValue(-15);
  pRandBuff->getConfigVector()[0] = pConstI32;

  // create generator
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  BufferContainerList bcl;
  // generate
  gen.Read(&bcl);

  IBufferContainer *bc = bcl.GetBufferContainer(0);
  IMemoryObject *buff = bc->GetMemoryObject(0);
  float *float_t = (float *)buff->GetDataPtr();
  uint32_t i;
  for (i = 0; i < 4; i++)
    EXPECT_EQ(1.0f, *(float_t++));

  EXPECT_EQ(int32_t(-15), *((int32_t *)float_t));

  BufferDesc buffDsc = GetBufferDescription(buff->GetMemoryObjectDesc());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());

  delete M;
}
// typedef struct { int[16] a} a
TEST(OCLKernelDataGenerator, arrayTestOfOCLGenerator) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfarrayArguments";
  createModule(Context, M, kernelName, "{ [16 x i32] } %a");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig dgConfig;
  dgConfig.getConfigVector().resize(1);

  BufferStructureGeneratorConfig *pRandBuff =
      static_cast<BufferStructureGeneratorConfig *>(
          GeneratorConfigFactory::create("BufferStructureGenerator"));
  dgConfig.getConfigVector()[0] = pRandBuff;

  BufferConstGeneratorConfig<uint32_t> *pConstI32 =
      static_cast<BufferConstGeneratorConfig<uint32_t> *>(
          GeneratorConfigFactory::create("BufferConstGeneratori32"));
  pConstI32->SetFillValue(uint32_t(765432));

  pRandBuff->getConfigVector().push_back(pConstI32);

  // create generator
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  BufferContainerList bcl;
  // generate
  gen.Read(&bcl);

  IBufferContainer *bc = bcl.GetBufferContainer(0);
  IMemoryObject *buff = bc->GetMemoryObject(0);

  uint32_t *p_uint32;

  p_uint32 = (uint32_t *)buff->GetDataPtr();

  for (uint32_t i = 0; i < 16; ++i)
    EXPECT_EQ(uint32_t(765432), *(p_uint32++));
  BufferDesc buffDsc = GetBufferDescription(buff->GetMemoryObjectDesc());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());

  delete M;
}
// double* a, long* b
TEST(OCLKernelDataGenerator, pointerTestOfOCLGenerator) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfpointerArguments";
  createModule(Context, M, kernelName, "double* %a, i64* %b");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig dgConfig;
  dgConfig.getConfigVector().resize(2);

  TypeDesc tyDesc = static_cast<BufferDesc *>(argDescriptions[0].get())
                        ->GetElementDescription();
  tyDesc.SetNumberOfElements(10);
  static_cast<BufferDesc *>(argDescriptions[0].get())->SetElementDecs(tyDesc);

  tyDesc = static_cast<BufferDesc *>(argDescriptions[1].get())
               ->GetElementDescription();
  tyDesc.SetNumberOfElements(40);
  static_cast<BufferDesc *>(argDescriptions[1].get())->SetElementDecs(tyDesc);

  BufferConstGeneratorConfig<double> *pConstF64 =
      static_cast<BufferConstGeneratorConfig<double> *>(
          GeneratorConfigFactory::create("BufferConstGeneratorf64"));
  dgConfig.getConfigVector()[0] = pConstF64;
  pConstF64->SetFillValue(2.0f);

  BufferConstGeneratorConfig<uint64_t> *pConstF641 =
      static_cast<BufferConstGeneratorConfig<uint64_t> *>(
          GeneratorConfigFactory::create("BufferConstGeneratori64"));
  dgConfig.getConfigVector()[1] = pConstF641;
  pConstF641->SetFillValue(345);

  // create generator
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  BufferContainerList bcl;
  // generate
  gen.Read(&bcl);

  IBufferContainer *bc = bcl.GetBufferContainer(0);
  IMemoryObject *buff = bc->GetMemoryObject(0);

  double *p_double;
  uint64_t *p_uint64;

  p_double = (double *)buff->GetDataPtr();

  for (uint32_t i = 0; i < 10; ++i)
    EXPECT_EQ(2.0f, *(p_double++));

  buff = bc->GetMemoryObject(1);
  p_uint64 = (uint64_t *)buff->GetDataPtr();
  for (uint32_t i = 0; i < 40; ++i)
    EXPECT_EQ(uint64_t(345), *(p_uint64++));

  BufferDesc buffDsc = GetBufferDescription(buff->GetMemoryObjectDesc());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TLONG, elemDesc.GetType());

  delete M;
}
// typedef struct { long a, float16 b} a
// a* b
TEST(OCLKernelDataGenerator, pointer_to_struct_TestOfOCLGenerator) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfpointerArguments";
  createModule(Context, M, kernelName, "{ i64, < 16 x float> }* %a ");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig dgConfig;
  dgConfig.getConfigVector().resize(1);

  TypeDesc tyDesc = static_cast<BufferDesc *>(argDescriptions[0].get())
                        ->GetElementDescription();
  tyDesc.SetNumberOfElements(10);
  static_cast<BufferDesc *>(argDescriptions[0].get())->SetElementDecs(tyDesc);

  BufferStructureGeneratorConfig *pConstF64 =
      static_cast<BufferStructureGeneratorConfig *>(
          GeneratorConfigFactory::create("BufferStructureGenerator"));
  dgConfig.getConfigVector()[0] = pConstF64;

  BufferConstGeneratorConfig<uint64_t> *pConstI64 =
      static_cast<BufferConstGeneratorConfig<uint64_t> *>(
          GeneratorConfigFactory::create("BufferConstGeneratori64"));
  pConstI64->SetFillValue(uint64_t(2));
  pConstF64->getConfigVector().push_back(pConstI64);

  BufferConstGeneratorConfig<float> *pConstF32 =
      static_cast<BufferConstGeneratorConfig<float> *>(
          GeneratorConfigFactory::create("BufferConstGeneratorf32"));
  pConstF32->SetFillValue(4.0f);
  pConstF64->getConfigVector().push_back(pConstF32);

  // create generator
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  BufferContainerList bcl;
  // generate
  gen.Read(&bcl);

  IBufferContainer *bc = bcl.GetBufferContainer(0);
  IMemoryObject *buff = bc->GetMemoryObject(0);

  uint64_t *p_uint64;
  float *p_float;
  void *_ptr = buff->GetDataPtr();
  p_float = (float *)_ptr;

  for (uint32_t i = 0; i < 10; ++i) {
    p_uint64 = (uint64_t *)p_float;
    EXPECT_EQ(uint64_t(2), *(p_uint64++));

    p_float = (float *)p_uint64;

    for (uint32_t j = 0; j < 16; ++j)
      EXPECT_EQ(4.0f, *(p_float++));
  }
  delete M;
}
// typedef struct { long a, float16[20] b} a
// a* b
TEST(OCLKernelDataGenerator, array_of_vectors_TestOfOCLGenerator) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfpointerArguments";
  createModule(Context, M, kernelName, "{ i64, [20 x < 16 x float>] }* %a ");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig dgConfig;
  dgConfig.getConfigVector().resize(1);

  TypeDesc tyDesc = static_cast<BufferDesc *>(argDescriptions[0].get())
                        ->GetElementDescription();
  tyDesc.SetNumberOfElements(10);
  static_cast<BufferDesc *>(argDescriptions[0].get())->SetElementDecs(tyDesc);

  BufferStructureGeneratorConfig *pConstF64 =
      static_cast<BufferStructureGeneratorConfig *>(
          GeneratorConfigFactory::create("BufferStructureGenerator"));
  dgConfig.getConfigVector()[0] = pConstF64;

  BufferConstGeneratorConfig<uint64_t> *pConstI64 =
      static_cast<BufferConstGeneratorConfig<uint64_t> *>(
          GeneratorConfigFactory::create("BufferConstGeneratori64"));
  pConstI64->SetFillValue(uint64_t(2));
  pConstF64->getConfigVector().push_back(pConstI64);

  BufferConstGeneratorConfig<float> *pConstF32 =
      static_cast<BufferConstGeneratorConfig<float> *>(
          GeneratorConfigFactory::create("BufferConstGeneratorf32"));
  pConstF32->SetFillValue(4.0f);
  pConstF64->getConfigVector().push_back(pConstF32);

  // create generator
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  BufferContainerList bcl;
  // generate
  gen.Read(&bcl);

  IBufferContainer *bc = bcl.GetBufferContainer(0);
  IMemoryObject *buff = bc->GetMemoryObject(0);

  uint64_t *p_uint64;
  float *p_float;
  void *_ptr = buff->GetDataPtr();
  p_float = (float *)_ptr;

  for (uint32_t i = 0; i < 10; ++i) {
    p_uint64 = (uint64_t *)p_float;
    EXPECT_EQ(uint64_t(2), *(p_uint64++));

    p_float = (float *)p_uint64;

    for (uint32_t i = 0; i < 320; ++i)
      EXPECT_EQ(4.0f, *(p_float++));
  }
  delete M;
}
// set and check config
TEST(OCLKernelDataGenerator, defaultConfig_Test_scalar) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfdefaultConfig";
  createModule(Context, M, kernelName,
               " float %a, double %b, i64 %c, i32 %d, i16 %e, i8 %f");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig *p_dgConfig =
      OCLKernelDataGeneratorConfig::defaultConfig(argDescriptions);
  OCLKernelDataGeneratorConfig &dgConfig = *p_dgConfig;

  AbstractGeneratorConfig *cfg = dgConfig.getConfigVector()[0];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<float>::getStaticName());

  cfg = dgConfig.getConfigVector()[1];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<double>::getStaticName());

  cfg = dgConfig.getConfigVector()[2];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int64_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[3];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int32_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[4];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int16_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[5];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int8_t>::getStaticName());
  // check config in constructor
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  UNUSED_ARGUMENT(gen);
  delete p_dgConfig;
}

// set and check config
TEST(OCLKernelDataGenerator, defaultConfig_Test_vector_and_scalar) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfdefaultConfig";
  createModule(Context, M, kernelName,
               "float %a, double %b, i64 %c, i32 %d, i16 %e, i8 %f, < 4 x "
               "float> %g, < 4 x double > %h, < 8 x i8 > %i, <3 x i16> %j, < "
               "16 x i32> %k, < 8 x i64> %l");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig *p_dgConfig =
      OCLKernelDataGeneratorConfig::defaultConfig(argDescriptions);
  OCLKernelDataGeneratorConfig &dgConfig = *p_dgConfig;

  AbstractGeneratorConfig *cfg = dgConfig.getConfigVector()[0];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<float>::getStaticName());

  cfg = dgConfig.getConfigVector()[1];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<double>::getStaticName());

  cfg = dgConfig.getConfigVector()[2];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int64_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[3];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int32_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[4];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int16_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[5];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int8_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[6];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<float>::getStaticName());

  cfg = dgConfig.getConfigVector()[7];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<double>::getStaticName());

  cfg = dgConfig.getConfigVector()[8];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int8_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[9];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int16_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[10];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int32_t>::getStaticName());

  cfg = dgConfig.getConfigVector()[11];
  EXPECT_EQ(cfg->getName(),
            BufferRandomGeneratorConfig<int64_t>::getStaticName());
  // check config in constructor
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  UNUSED_ARGUMENT(gen);

  delete p_dgConfig;
}
// set and check config
TEST(OCLKernelDataGenerator, defaultConfig_Test_simple_structs) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfdefaultConfig";
  createModule(Context, M, kernelName,
               "{float, double, {i64, i32, double}, i16, { <4 x i64>, {double, "
               "{ <3 x double>, {i32} } } } }");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig *p_dgConfig =
      OCLKernelDataGeneratorConfig::defaultConfig(argDescriptions);
  OCLKernelDataGeneratorConfig &dgConfig = *p_dgConfig;

  AbstractGeneratorConfig *_Buffcfg = dgConfig.getConfigVector()[0];
  EXPECT_STREQ(_Buffcfg->getName().c_str(),
               BufferStructureGeneratorConfig::getStaticName().c_str());

  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                   ->getConfigVector()[0]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<float>::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                   ->getConfigVector()[1]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<double>::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                   ->getConfigVector()[2]
                   ->getName()
                   .c_str(),
               BufferStructureGeneratorConfig::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(
                   static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                       ->getConfigVector()[2])
                   ->getConfigVector()[0]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<int64_t>::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(
                   static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                       ->getConfigVector()[2])
                   ->getConfigVector()[1]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<int32_t>::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(
                   static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                       ->getConfigVector()[2])
                   ->getConfigVector()[2]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<double>::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                   ->getConfigVector()[3]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<int16_t>::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                   ->getConfigVector()[4]
                   ->getName()
                   .c_str(),
               BufferStructureGeneratorConfig::getStaticName().c_str());

  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(
                   static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                       ->getConfigVector()[4])
                   ->getConfigVector()[0]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<int64_t>::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(
                   static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                       ->getConfigVector()[4])
                   ->getConfigVector()[1]
                   ->getName()
                   .c_str(),
               BufferStructureGeneratorConfig::getStaticName().c_str());
  // check config in constructor
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  UNUSED_ARGUMENT(gen);
  delete p_dgConfig;
}
// set and check config
TEST(OCLKernelDataGenerator, Test_simple_images) {
  llvm::LLVMContext Context;
  BufferContainerList input;
  llvm::Module *M = 0;

  std::string kernelName = "testSimpleImages";
  createModule(Context, M, kernelName, "%opencl.image2d_t.0* %input");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  std::string ConfigFile = "<?xml version=\"1.0\" ?>\
<OCLKernelDataGeneratorConfig>\
    <Seed>10232</Seed>\
    <GeneratorConfiguration Name=\"ImageRandomGenerator\">\
        <ImageType>CL_MEM_OBJECT_IMAGE2D</ImageType>\
        <ChannelDataType>CL_UNORM_INT8</ChannelDataType>\
        <ChannelOrder>CL_sRGBA</ChannelOrder>\
        <Size>128 64</Size>\
    </GeneratorConfiguration>\
</OCLKernelDataGeneratorConfig>";
  std::fstream io_file;
  io_file.open("testSimpleImages.cfg", std::fstream::out);
  io_file << ConfigFile;
  io_file.close();

  TiXmlDocument config(std::string("testSimpleImages.cfg"));
  ASSERT_TRUE(config.LoadFile());
  OCLKernelDataGeneratorConfig cfg(&config);

  OCLKernelDataGenerator gen(argDescriptions, cfg);
  gen.Read(&input);
  EXPECT_STREQ(
      Image::GetImageName().c_str(),
      input.GetBufferContainer(0)->GetMemoryObject(0)->GetName().c_str());
  const IMemoryObjectDesc *MemObjDesc =
      input.GetBufferContainer(0)->GetMemoryObject(0)->GetMemoryObjectDesc();
  EXPECT_STREQ(ImageDesc::GetImageDescName().c_str(),
               MemObjDesc->GetName().c_str());
  const ImageDesc *imDesc = static_cast<const ImageDesc *>(MemObjDesc);
  EXPECT_EQ(OpenCL_MEM_OBJECT_IMAGE2D, imDesc->GetImageType());
  EXPECT_EQ(OpenCL_UNORM_INT8, imDesc->GetImageChannelDataType());
  EXPECT_EQ(OpenCL_sRGBA, imDesc->GetImageChannelOrder());

  ImageSizeDesc sizeDesc = imDesc->GetSizesDesc();
  EXPECT_EQ((uint64_t)128, sizeDesc.width);
  EXPECT_EQ((uint64_t)64, sizeDesc.height);
}
// set and check config
TEST(OCLKernelDataGenerator, defaultConfig_Test_pointers) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfdefaultConfig";
  createModule(Context, M, kernelName, "{float, double}* %a, i64* %b");
  OpenCLKernelArgumentsParser parser;
  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);

  OCLKernelDataGeneratorConfig *p_dgConfig =
      OCLKernelDataGeneratorConfig::defaultConfig(argDescriptions);
  OCLKernelDataGeneratorConfig &dgConfig = *p_dgConfig;

  AbstractGeneratorConfig *_Buffcfg = dgConfig.getConfigVector()[0];
  EXPECT_STREQ(_Buffcfg->getName().c_str(),
               BufferStructureGeneratorConfig::getStaticName().c_str());

  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                   ->getConfigVector()[0]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<float>::getStaticName().c_str());
  EXPECT_STREQ(static_cast<BufferStructureGeneratorConfig *>(_Buffcfg)
                   ->getConfigVector()[1]
                   ->getName()
                   .c_str(),
               BufferRandomGeneratorConfig<double>::getStaticName().c_str());

  _Buffcfg = dgConfig.getConfigVector()[1];
  EXPECT_STREQ(_Buffcfg->getName().c_str(),
               BufferRandomGeneratorConfig<int64_t>::getStaticName().c_str());
  // check config in constructor
  OCLKernelDataGenerator gen(argDescriptions, dgConfig);
  UNUSED_ARGUMENT(gen);

  delete p_dgConfig;
}
// check if two different generators produce the same data with the same seed
TEST(OCLKernelDataGenerator, test_of_seed) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfdefaultConfig";
  createModule(Context, M, kernelName, "{float, i64}* %a, i64* %b");
  OpenCLKernelArgumentsParser parser;

  const size_t numOfWorkItems = 100;
  size_t dim[] = {numOfWorkItems};

  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);
  argDescriptions =
      OpenCLKernelArgumentsParser::KernelArgHeuristics(argDescriptions, dim, 1);

  OCLKernelDataGeneratorConfig *p_dgConfig =
      OCLKernelDataGeneratorConfig::defaultConfig(argDescriptions);
  OCLKernelDataGeneratorConfig &dgConfig = *p_dgConfig;

  for (uint32_t i = 0; i < 200; ++i) {
    dgConfig.setSeed(time(NULL));

    OCLKernelDataGenerator gen1(argDescriptions, dgConfig);
    OCLKernelDataGenerator gen2(argDescriptions, dgConfig);

    BufferContainerList bc1;
    BufferContainerList bc2;

    gen1.Read(&bc1);
    gen2.Read(&bc2);

    IBufferContainer *bc11 = bc1.GetBufferContainer(0);
    IMemoryObject *buff1 = bc11->GetMemoryObject(0);

    IBufferContainer *bc21 = bc2.GetBufferContainer(0);
    IMemoryObject *buff2 = bc21->GetMemoryObject(0);
    void *p1 = buff1->GetDataPtr();
    void *p2 = buff2->GetDataPtr();

    int res = memcmp(p1, p2, (sizeof(float) + sizeof(uint64_t)) * 100);

    EXPECT_EQ(res, 0);
  }
  delete p_dgConfig;
}

TEST(OCLKernelDataGenerator, XMLParsingTest) {
  std::string ConfigFile = "<?xml version=\"1.0\" ?>\
<OCLKernelDataGeneratorConfig>\
    <Seed>10232</Seed>\
    <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"f64\">\
        <Value>111.232335</Value>\
    </GeneratorConfiguration>\
    <GeneratorConfiguration Name=\"BufferRandomGenerator\" Type=\"i32\">\
        </GeneratorConfiguration>\
    <GeneratorConfiguration Name=\"BufferStructureGenerator\">\
        <SubGeneratorConfiguration Name=\"BufferRandomGenerator\" Type=\"i32\">\
            </SubGeneratorConfiguration>\
        <SubGeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"f32\">\
            <Value>111.5</Value>\
        </SubGeneratorConfiguration>\
        <SubGeneratorConfiguration Name=\"BufferStructureGenerator\">\
            <SubGeneratorConfiguration Name=\"BufferRandomGenerator\" Type=\"u64\">\
                </SubGeneratorConfiguration>\
            <SubGeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"u32\">\
                <Value>43544</Value>\
            </SubGeneratorConfiguration>\
        </SubGeneratorConfiguration>\
    </GeneratorConfiguration>\
    <GeneratorConfiguration Name=\"BufferRandomGenerator\" Type=\"i32\">\
        </GeneratorConfiguration>\
    <GeneratorConfiguration Name=\"ImageRandomGenerator\">\
        <ImageType>CL_MEM_OBJECT_IMAGE2D_ARRAY</ImageType>\
        <ChannelDataType>CL_UNORM_INT8</ChannelDataType>\
        <ChannelOrder>CL_RGBA</ChannelOrder>\
        <Size>1024 1024 0 5</Size>\
    </GeneratorConfiguration>\
</OCLKernelDataGeneratorConfig>";
  std::fstream io_file;
  io_file.open("DataGeneratorConfig.cfg", std::fstream::out);
  io_file << ConfigFile;
  io_file.close();

  TiXmlDocument config(std::string("DataGeneratorConfig.cfg"));
  ASSERT_TRUE(config.LoadFile());
  OCLKernelDataGeneratorConfig cfg(&config);

  EXPECT_STREQ(cfg.getConfigVector()[0]->getName().c_str(),
               BufferConstGeneratorConfig<double>::getStaticName().c_str());
  EXPECT_EQ(static_cast<BufferConstGeneratorConfig<double> *>(
                cfg.getConfigVector()[0])
                ->GetFillValue(),
            111.232335);

  EXPECT_STREQ(cfg.getConfigVector()[1]->getName().c_str(),
               BufferRandomGeneratorConfig<int32_t>::getStaticName().c_str());
  EXPECT_STREQ(cfg.getConfigVector()[2]->getName().c_str(),
               BufferStructureGeneratorConfig::getStaticName().c_str());

  BufferStructureGeneratorConfig *bufferConfig =
      static_cast<BufferStructureGeneratorConfig *>(cfg.getConfigVector()[2]);
  EXPECT_STREQ(bufferConfig->getConfigVector()[0]->getName().c_str(),
               BufferRandomGeneratorConfig<int32_t>::getStaticName().c_str());
  EXPECT_STREQ(bufferConfig->getConfigVector()[1]->getName().c_str(),
               BufferConstGeneratorConfig<float>::getStaticName().c_str());
  EXPECT_EQ(static_cast<BufferConstGeneratorConfig<float> *>(
                bufferConfig->getConfigVector()[1])
                ->GetFillValue(),
            111.5);

  EXPECT_STREQ(bufferConfig->getConfigVector()[2]->getName().c_str(),
               BufferStructureGeneratorConfig::getStaticName().c_str());

  bufferConfig = static_cast<BufferStructureGeneratorConfig *>(
      bufferConfig->getConfigVector()[2]);
  EXPECT_STREQ(bufferConfig->getConfigVector()[0]->getName().c_str(),
               BufferRandomGeneratorConfig<uint64_t>::getStaticName().c_str());
  EXPECT_STREQ(bufferConfig->getConfigVector()[1]->getName().c_str(),
               BufferConstGeneratorConfig<uint32_t>::getStaticName().c_str());
  EXPECT_EQ(static_cast<BufferConstGeneratorConfig<uint32_t> *>(
                bufferConfig->getConfigVector()[1])
                ->GetFillValue(),
            uint32_t(43544));

  ImageRandomGeneratorConfig *imageConfig =
      static_cast<ImageRandomGeneratorConfig *>(cfg.getConfigVector()[4]);
  EXPECT_STREQ(imageConfig->getName().c_str(),
               ImageRandomGeneratorConfig::getStaticName().c_str());
  ImageDesc *imdesc = imageConfig->getImageDescriptor();
  EXPECT_EQ(imdesc->GetImageChannelDataType(), OpenCL_UNORM_INT8);
  EXPECT_EQ(imdesc->GetImageChannelOrder(), OpenCL_RGBA);
  EXPECT_EQ(imdesc->GetImageType(), OpenCL_MEM_OBJECT_IMAGE2D_ARRAY);
  EXPECT_EQ(imdesc->GetSizesDesc().width, (uint64_t)1024);
  EXPECT_EQ(imdesc->GetSizesDesc().height, (uint64_t)1024);
  EXPECT_EQ(imdesc->GetSizesDesc().width, (uint64_t)1024);
  EXPECT_EQ(imdesc->GetSizesDesc().array_size, (uint64_t)5);
  EXPECT_EQ(imdesc->GetSizesDesc().row, (uint64_t)4096);
  EXPECT_EQ(imdesc->GetSizesDesc().slice, (uint64_t)4194304);
}
TEST(OCLKernelDataGenerator, XMLParsingTestMultiplySeed) {
  std::string ConfigFile = "<?xml version=\"1.0\" ?>\
<OCLKernelDataGeneratorConfig>\
    <Seed>10232</Seed>\
    <Seed>10222</Seed>\
    <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"f64\">\
        <Value>111.232335</Value>\
    </GeneratorConfiguration>\
    <GeneratorConfiguration Name=\"BufferRandomGenerator\" Type=\"i32\">\
        </GeneratorConfiguration>\
</OCLKernelDataGeneratorConfig>";
  std::fstream io_file;
  io_file.open("DataGeneratorConfig.cfg", std::fstream::out);
  io_file << ConfigFile;
  io_file.close();

  TiXmlDocument config(std::string("DataGeneratorConfig.cfg"));
  ASSERT_TRUE(config.LoadFile());
  TiXmlNode *Node = &config;
  EXPECT_THROW((OCLKernelDataGeneratorConfig(Node)), Exception::IOError);
}
TEST(OCLKernelDataGenerator, XMLParsingTestOCLKernelDataGeneratorConfig) {
  std::string ConfigFile = "<?xml version=\"1.0\" ?>\
<OCLKernelDataGeneratorConfig>\
    <Seed>10232</Seed>\
    <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"f64\">\
        <Value>111.232335</Value>\
    </GeneratorConfiguration>\
    <GeneratorConfiguration Name=\"BufferRandomGenerator\" Type=\"i32\">\
        </GeneratorConfiguration>\
</OCLKernelDataGeneratorConfig>\
<OCLKernelDataGeneratorConfig>\
    <Seed>10232</Seed>\
    <GeneratorConfiguration Name=\"BufferConstGenerator\" Type=\"f64\">\
        <Value>111.232335</Value>\
    </GeneratorConfiguration>\
    <GeneratorConfiguration Name=\"BufferRandomGenerator\" Type=\"i32\">\
        </GeneratorConfiguration>\
</OCLKernelDataGeneratorConfig>\
";
  std::fstream io_file;
  io_file.open("DataGeneratorConfig.cfg", std::fstream::out);
  io_file << ConfigFile;
  io_file.close();

  TiXmlDocument config(std::string("DataGeneratorConfig.cfg"));
  ASSERT_TRUE(config.LoadFile());
  TiXmlNode *Node = &config;
  EXPECT_THROW((OCLKernelDataGeneratorConfig(Node)), Exception::IOError);
}
} // namespace
