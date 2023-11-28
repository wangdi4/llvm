// Copyright (c) 2015 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly.

#include "FrontendDriverFixture.h"
#include "common_utils.h"
#include "opencl_clang.h"
#include "llvm/Support/MemoryBuffer.h"
#include <algorithm>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::FECompilerAPI;

void ClangCompilerTestType::SetUp() {
  ASSERT_NO_FATAL_FAILURE(GetTestSPIRVProgram(m_spirv_program_binary));

  CLANG_DEV_INFO DevInfo;
  DevInfo.sExtensionStrings = "";
  DevInfo.sOpenCLCFeatureStrings = "";
  int failure =
      CreateFrontEndInstance(&DevInfo, sizeof(DevInfo), &m_fe_compiler);
  ASSERT_EQ(failure, CL_SUCCESS) << "CreateFrontEndInstance failed";
  m_llvm_context = new llvm::LLVMContext();
}

void ClangCompilerTestType::TearDown() {
  if (m_fe_compiler)
    m_fe_compiler->Release();

  if (m_llvm_context)
    delete m_llvm_context;

  if (m_binary_result)
    m_binary_result->Release();
}

void ClangCompilerTestType::GetTestSPIRVProgram(std::vector<char> &spirv) {
  std::fstream spirv_file(get_exe_dir() + SPIRV_TEST_FILE,
                          std::fstream::in | std::fstream::binary);
  ASSERT_TRUE(spirv_file.is_open())
      << "Failed to open test spirv file: " << SPIRV_TEST_FILE;

  std::copy(std::istreambuf_iterator<char>(spirv_file),
            std::istreambuf_iterator<char>(), std::back_inserter(spirv));
}

FESPIRVProgramDescriptor ClangCompilerTestType::GetTestFESPIRVProgramDescriptor(
    const char *build_options) {
  FESPIRVProgramDescriptor spirvDesc;

  spirvDesc.pSPIRVContainer = &GetSpirvBinaryContainer()[0];
  spirvDesc.uiSPIRVContainerSize = GetSpirvBinaryContainer().size();
  spirvDesc.pszOptions = build_options;
  spirvDesc.uiSpecConstCount = 0;
  spirvDesc.puiSpecConstIds = nullptr;
  spirvDesc.puiSpecConstValues = nullptr;

  return spirvDesc;
}

llvm::ErrorOr<std::unique_ptr<llvm::Module>>
ClangCompilerTestType::ExtractModule(IOCLFEBinaryResult *pResult) {
  llvm::StringRef bitCodeStr((const char *)pResult->GetIR(),
                             pResult->GetIRSize());
  std::unique_ptr<llvm::MemoryBuffer> pMemBuffer =
      llvm::MemoryBuffer::getMemBuffer(bitCodeStr, "", false);
  return llvm::expectedToErrorOrAndEmitErrors(
      *GetLLVMContext(),
      llvm::parseBitcodeFile(pMemBuffer->getMemBufferRef(), *GetLLVMContext()));
}
