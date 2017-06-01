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

#include "ClangCompilerTestTypeFixture.h"

#include <algorithm>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::FECompilerAPI;

void ClangCompilerTestType::SetUp()
{
    GetTestSPIRVProgram(m_spirv_program_binary);

    std::vector<char> pszDeviceInfoVec;
    pszDeviceInfoVec.resize(sizeof(CLANG_DEV_INFO), 0);
    // stub with zeros
    memset(&pszDeviceInfoVec[0], 0, sizeof(CLANG_DEV_INFO));
    // fix up CLANG_DEV_INFO.sExtensionStrings
    CLANG_DEV_INFO* pszDeviceInfo = reinterpret_cast<CLANG_DEV_INFO*>(&pszDeviceInfoVec[0]);
    pszDeviceInfo->sExtensionStrings = "";

    int failure = CreateFrontEndInstance(&pszDeviceInfo[0], pszDeviceInfoVec.size(), &m_fe_compiler, nullptr);
    if (failure)
    {
        printf("Error while instantiating CreateFrontEndInstance: error #%d", failure);
        exit(1);
    }

    m_llvm_context = new llvm::LLVMContext();
}

void ClangCompilerTestType::TearDown() {
    if ( nullptr != m_fe_compiler )
    {
        m_fe_compiler->Release();
        m_fe_compiler = nullptr;
    }

    delete m_llvm_context;

    if (nullptr != m_binary_result)
    {
        m_binary_result->Release();
        m_binary_result = nullptr;
    }
}

void ClangCompilerTestType::GetTestSPIRVProgram(std::vector<char>& spirv)
{
    std::fstream spirv_file(SPIRV_TEST_FILE, std::fstream::in | std::fstream::binary);
    if (!spirv_file.is_open())
    {
        printf("Error while opening test spirv file: %s\n", SPIRV_TEST_FILE);
        exit(1);
    }

    std::copy(std::istreambuf_iterator<char>(spirv_file),
        std::istreambuf_iterator<char>(),
        std::back_inserter(spirv));
}

FESPIRVProgramDescriptor ClangCompilerTestType::GetTestFESPIRVProgramDescriptor(
    const char* build_options)
{
    FESPIRVProgramDescriptor spirvDesc;

    spirvDesc.pSPIRVContainer = &GetSpirvBinaryContainer()[0];
    spirvDesc.uiSPIRVContainerSize = GetSpirvBinaryContainer().size();
    spirvDesc.pszOptions = build_options;

    return spirvDesc;
}

llvm::ErrorOr<std::unique_ptr<llvm::Module>>
ClangCompilerTestType::ExtractModule(IOCLFEBinaryResult* pResult)
{
    llvm::StringRef bitCodeStr((const char*)pResult->GetIR(),
                               pResult->GetIRSize());
    std::unique_ptr<llvm::MemoryBuffer> pMemBuffer =
      llvm::MemoryBuffer::getMemBuffer(bitCodeStr, "", false);
    return llvm::expectedToErrorOrAndEmitErrors(*GetLLVMContext(),
      llvm::parseBitcodeFile(pMemBuffer->getMemBufferRef(), *GetLLVMContext()));
}
