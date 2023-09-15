// INTEL CONFIDENTIAL
//
// Copyright 2015 Intel Corporation.
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

#ifndef __MetadataAPITextFixture__
#define __MetadataAPITextFixture__

#include "common_utils.h"
#include "gtest_wrapper.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
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
