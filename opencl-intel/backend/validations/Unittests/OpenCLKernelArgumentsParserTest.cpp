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

#include "OpenCLKernelArgumentsParser.h"
#include "Buffer.h"
#include "gtest_wrapper.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

using namespace llvm;
using namespace Validation;

namespace {

std::string stringForParser(std::string input) {
  std::string output;
  int sizeInput = input.size();
  int isVar = 0;
  for (int i = 0; i < sizeInput; i++) {
    if (input[i] == '%') {
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

void createModule(LLVMContext &Context, Module *&M, std::string kernelName,
                  std::string params) {
  std::string program = std::string("define void @") + kernelName +
                        std::string("(") + params +
                        std::string(") { "
                                    "  ret void "
                                    "} "
                                    "!opencl.kernels = !{!0}"
                                    "!0 = !{void (") +
                        stringForParser(params) + std::string(")* @") +
                        kernelName + std::string("}");
  std::cerr << program << std::endl;

  SMDiagnostic Error;
  auto pM = parseAssemblyString(program, Error, Context);
  std::string errMsg;
  raw_string_ostream os(errMsg);
  Error.print("", os);
  EXPECT_TRUE(pM != nullptr) << os.str();
  M = pM.release();
}

TEST(OpenCLKernelArgumentsParser, FloatVectorArguments) {

  LLVMContext Context;
  Module *M = 0;

  std::string kernelName = "testOfFloatVectorArguments";
  createModule(Context, M, kernelName, "<4 x float> %a, double %b");
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions =
      parser->KernelArgumentsParser(kernelName, M);
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(uint64_t(4), elemDesc.GetNumberOfElements());
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TFLOAT, elemDesc.GetType());

  it++;
  buffDsc = GetBufferDescription(it->get());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TDOUBLE, elemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, IntVectorPointerArguments) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfIntVectorPointerArguments";
  createModule(Context, M, kernelName, "<4 x i32>* %a, i64 %b");
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions =
      parser->KernelArgumentsParser(kernelName, M);
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(uint64_t(4), elemDesc.GetNumberOfElements());
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TINT, elemDesc.GetType());

  it++;
  buffDsc = GetBufferDescription(it->get());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TLONG, elemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, IntArguments) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfIntArguments";
  createModule(Context, M, kernelName, "i30 %a");
  IOpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions;
  try {
    argDescriptions = parser->KernelArgumentsParser(kernelName, M);
  } catch (Exception::ParserBadTypeException) {
    EXPECT_TRUE(true);
    return;
  }
  EXPECT_TRUE(false);

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, PointToArguments) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfFloatVectorArguments";
  createModule(Context, M, kernelName, "float* %a, i16* %b");
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions =
      parser->KernelArgumentsParser(kernelName, M);
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TFLOAT, elemDesc.GetType());

  it++;
  buffDsc = GetBufferDescription(it->get());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TSHORT, elemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, StructOfIntFloatDouble) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfStruct";
  createModule(Context, M, kernelName,
               "{ i32, float, double } %a, float %d"); // programObjectStruct);
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions;
  try {
    argDescriptions = parser->KernelArgumentsParser(kernelName, M);
  } catch (Exception::ParserBadTypeException) {
    EXPECT_TRUE(false);
    return;
  }
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  TypeDesc subElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TINT, subElemDesc.GetType());

  subElemDesc = elemDesc.GetSubTypeDesc(1);
  EXPECT_EQ(TFLOAT, subElemDesc.GetType());
  subElemDesc = elemDesc.GetSubTypeDesc(2);
  EXPECT_EQ(TDOUBLE, subElemDesc.GetType());

  it++;
  buffDsc = GetBufferDescription(it->get());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TFLOAT, elemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, VectorInStruct) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfVectorInStruct";
  createModule(Context, M, kernelName, "{<100 x i64>} %a");
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions =
      parser->KernelArgumentsParser(kernelName, M);
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(uint64_t(100), elemDesc.GetNumberOfElements());
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TLONG, elemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, ArrayInStruct) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfArrayInStruct";
  createModule(Context, M, kernelName, "{[5 x i16]} %a, {[10 x <1 x i64>]} %b");
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions =
      parser->KernelArgumentsParser(kernelName, M);
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(uint64_t(5), elemDesc.GetNumberOfElements());
  EXPECT_EQ(TARRAY, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TSHORT, elemDesc.GetType());

  it++;
  buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(uint64_t(10), elemDesc.GetNumberOfElements());
  EXPECT_EQ(TARRAY, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(uint64_t(1), elemDesc.GetNumberOfElements());
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TLONG, elemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, TestOfStruct) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfStruct";
  createModule(Context, M, kernelName,
               "{ i64, float, double, <3 x i32>, <4 x float>, [10 x <8 x "
               "double>] } %a, i8 %b, float %c, {{{i32, float, <5 x i16>*}}} "
               "%d"); // programObjectStruct);
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions;
  try {
    argDescriptions = parser->KernelArgumentsParser(kernelName, M);
  } catch (Exception::ParserBadTypeException) {
    EXPECT_TRUE(false);
    return;
  }
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc =
      GetBufferDescription(it->get()); //{ i32, float, double, <3 x i32>, <4 x
                                       // float>, [10 x <8 x double>] }
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  EXPECT_EQ(uint64_t(6), elemDesc.GetNumOfSubTypes());
  TypeDesc subElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TLONG, subElemDesc.GetType());
  subElemDesc = elemDesc.GetSubTypeDesc(1);
  EXPECT_EQ(TFLOAT, subElemDesc.GetType());
  subElemDesc = elemDesc.GetSubTypeDesc(2);
  EXPECT_EQ(TDOUBLE, subElemDesc.GetType());
  subElemDesc = elemDesc.GetSubTypeDesc(3);
  EXPECT_EQ(TVECTOR, subElemDesc.GetType());
  EXPECT_EQ(uint64_t(3), subElemDesc.GetNumberOfElements());
  subElemDesc = subElemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TINT, subElemDesc.GetType());
  subElemDesc = elemDesc.GetSubTypeDesc(4);
  EXPECT_EQ(TVECTOR, subElemDesc.GetType());
  EXPECT_EQ(uint64_t(4), subElemDesc.GetNumberOfElements());
  subElemDesc = subElemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TFLOAT, subElemDesc.GetType());
  subElemDesc = elemDesc.GetSubTypeDesc(5);
  EXPECT_EQ(TARRAY, subElemDesc.GetType());
  EXPECT_EQ(uint64_t(10), subElemDesc.GetNumberOfElements());
  subElemDesc = subElemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TVECTOR, subElemDesc.GetType());
  EXPECT_EQ(uint64_t(8), subElemDesc.GetNumberOfElements());
  subElemDesc = subElemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TDOUBLE, subElemDesc.GetType());

  it++;
  buffDsc = GetBufferDescription(it->get()); // i8
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TCHAR, elemDesc.GetType());
  it++;
  buffDsc = GetBufferDescription(it->get()); // float
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TFLOAT, elemDesc.GetType());
  it++;
  buffDsc = GetBufferDescription(it->get()); //{{{i32, float, <5 x i16>*}}}
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  subElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TINT, subElemDesc.GetType());
  subElemDesc = elemDesc.GetSubTypeDesc(1);
  EXPECT_EQ(TFLOAT, subElemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(2);
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  EXPECT_EQ(uint64_t(5), elemDesc.GetNumberOfElements());
  subElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TSHORT, subElemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, TestOfVector) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfVector";
  createModule(Context, M, kernelName,
               "<16 x float> %a, <13 x i64> %b, <1 x double> %c");
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions =
      parser->KernelArgumentsParser(kernelName, M);
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  EXPECT_EQ(uint64_t(16), elemDesc.GetNumberOfElements());
  TypeDesc SubElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TFLOAT, SubElemDesc.GetType());

  it++;
  buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  EXPECT_EQ(uint64_t(13), elemDesc.GetNumberOfElements());
  SubElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TLONG, SubElemDesc.GetType());
  it++;
  buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  EXPECT_EQ(uint64_t(1), elemDesc.GetNumberOfElements());
  SubElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TDOUBLE, SubElemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, TestOfPointer) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "testOfPointer";
  createModule(Context, M, kernelName,
               "<10000 x i64>* %a, i16* %b, float* %c, double* %d, {i64}* %e");
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions =
      parser->KernelArgumentsParser(kernelName, M);
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TVECTOR, elemDesc.GetType());
  EXPECT_EQ(uint64_t(10000), elemDesc.GetNumberOfElements());
  TypeDesc SubElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TLONG, SubElemDesc.GetType());

  it++;
  buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  SubElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TSHORT, SubElemDesc.GetType());
  it++;
  buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  SubElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TFLOAT, SubElemDesc.GetType());
  it++;
  buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  SubElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TDOUBLE, SubElemDesc.GetType());
  it++;
  buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  SubElemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TSTRUCT, SubElemDesc.GetType());
  SubElemDesc = SubElemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TLONG, SubElemDesc.GetType());

  delete parser;
  delete M;
}

TEST(OpenCLKernelArgumentsParser, TestForPointerAndStruct) {
  LLVMContext Context;
  Module *M = 0;
  std::string kernelName = "TestForPointerAndStruct";
  createModule(Context, M, kernelName,
               "{{{{{{float*}*}*}*}*}*} %a, {{{{{{i32}*}*}*}*}*}* %b");
  OpenCLKernelArgumentsParser *parser = new OpenCLKernelArgumentsParser();
  Validation::OCLKernelArgumentsList argDescriptions =
      parser->KernelArgumentsParser(kernelName, M);
  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  TypeDesc elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  for (int i = 0; i < 5; i++) {
    elemDesc = elemDesc.GetSubTypeDesc(0);
    EXPECT_EQ(TPOINTER, elemDesc.GetType());
    elemDesc = elemDesc.GetSubTypeDesc(0);
    EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  }
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TFLOAT, elemDesc.GetType());
  it++;
  buffDsc = GetBufferDescription(it->get());
  EXPECT_EQ(BufferDesc::GetBufferDescName(), buffDsc.GetName());
  elemDesc = buffDsc.GetElementDescription();
  EXPECT_EQ(TPOINTER, elemDesc.GetType());
  for (int i = 0; i < 5; i++) {
    elemDesc = elemDesc.GetSubTypeDesc(0);
    EXPECT_EQ(TSTRUCT, elemDesc.GetType());
    elemDesc = elemDesc.GetSubTypeDesc(0);
    EXPECT_EQ(TPOINTER, elemDesc.GetType());
  }
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TSTRUCT, elemDesc.GetType());
  elemDesc = elemDesc.GetSubTypeDesc(0);
  EXPECT_EQ(TINT, elemDesc.GetType());

  delete parser;
  delete M;
}

static void CheckPointers(const TypeDesc Ty) {
  switch (Ty.GetType()) {
  case TPOINTER: {
    EXPECT_NE(Ty.GetNumberOfElements(), uint64_t(0));
    CheckPointers(Ty.GetSubTypeDesc(0));
    break;
  }
  case TVECTOR:
  case TARRAY: {
    CheckPointers(Ty.GetSubTypeDesc(0));
    break;
  }
  case TSTRUCT: {
    uint64_t n_elems = Ty.GetNumOfSubTypes();
    for (uint64_t i = 0; i < n_elems; ++i) {
      CheckPointers(Ty.GetSubTypeDesc(i));
    }
    break;
  }
  default:
    break;
  }
}

TEST(OpenCLKernelArgumentsParser, TestOfHeuristics) {
  llvm::LLVMContext Context;
  llvm::Module *M = 0;

  std::string kernelName = "testOfdefaultConfig";
  createModule(Context, M, kernelName,
               "{float*, i64*,{{{{i64*}*}*}, double*}*}* %a, i64* %b");
  OpenCLKernelArgumentsParser parser;

  const size_t numOfWorkItems = 100;
  size_t dim[] = {numOfWorkItems};

  OCLKernelArgumentsList argDescriptions =
      parser.KernelArgumentsParser(kernelName, M);
  argDescriptions =
      OpenCLKernelArgumentsParser::KernelArgHeuristics(argDescriptions, dim, 1);

  Validation::OCLKernelArgumentsList::iterator it = argDescriptions.begin();
  BufferDesc buffDsc;
  for (; it != argDescriptions.end(); ++it) {
    buffDsc = GetBufferDescription(it->get());
    CheckPointers(buffDsc.GetElementDescription());
  }
  delete M;
}

} // anonymous namespace
