//==---MetaDataApiTestFixture.h - Fixture for MD API test -- C++ -----------=
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------===

#ifndef __MetadataAPITextFixture__
#define __MetadataAPITextFixture__

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <gtest/gtest.h>

#include "common_utils.h"

#include <string>

class MetadataTest : public ::testing::Test {
protected:
  MetadataTest() : m_LLVMContext(nullptr), m_pModule(nullptr) {}

  void SetUp() override {
    m_LLVMContext.reset(new llvm::LLVMContext());

    llvm::SMDiagnostic errDiagnostic;
    auto pModule = llvm::parseIRFile(get_exe_dir() + m_testFileName,
                                     errDiagnostic, *m_LLVMContext);

    if (!pModule) {
      errDiagnostic.print(m_testFileName.c_str(), llvm::errs());
      exit(1);
    }

    m_pModule.reset(pModule.release());
  }

  void TearDown() override {}

  llvm::Module *GetTestModule() { return m_pModule.get(); }

private:
  std::unique_ptr<llvm::LLVMContext> m_LLVMContext;
  std::unique_ptr<llvm::Module> m_pModule;

  std::string m_testFileName = "metadatatest.ll";
};

#endif
