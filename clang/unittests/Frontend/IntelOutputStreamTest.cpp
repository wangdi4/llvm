//===- unittests/Frontend/IntelOutputStreamTest.cpp --- FrontendAction tests --===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// Unit tests to test output stream setup for compiler instance.
//
//===----------------------------------------------------------------------===//

#include "clang/CodeGen/BackendUtil.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/FrontendTool/Utils.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace clang;
using namespace clang::frontend;

namespace {

TEST(FrontendOutputTests, TestOutputStream) {
  auto Invocation = std::make_shared<CompilerInvocation>();
  Invocation->getPreprocessorOpts().addRemappedFile(
      "test.cc",
      MemoryBuffer::getMemBuffer("").release());
  Invocation->getFrontendOpts().Inputs.push_back(
      FrontendInputFile("test.cc", InputKind::CXX));
  Invocation->getFrontendOpts().ProgramAction = EmitBC;
  Invocation->getTargetOpts().Triple = "i386-unknown-linux-gnu";
  CompilerInstance Compiler;

  SmallVector<char, 256> IRBuffer;
  std::unique_ptr<raw_pwrite_stream>
      IRStream(new raw_svector_ostream(IRBuffer));

  Compiler.SetOutputStream(std::move(IRStream));
  Compiler.setInvocation(std::move(Invocation));
  Compiler.createDiagnostics();

  bool Success = ExecuteCompilerInvocation(&Compiler);
  EXPECT_TRUE(Success);
  EXPECT_TRUE(!IRBuffer.empty());
  EXPECT_TRUE(StringRef(IRBuffer.data()).startswith("BC"));
}
}
