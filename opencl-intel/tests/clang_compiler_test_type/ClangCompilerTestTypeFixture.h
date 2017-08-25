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

#ifndef __ClangCompilerTestTypeFixture__
#define __ClangCompilerTestTypeFixture__

#include <CL/cl.h>
#include <gtest/gtest.h>
#include <clang_compiler.h>
#include <frontend_api.h>
#include <clang_device_info.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Bitcode/BitcodeReader.h>

#include <cstdint>

#define SPIR_OPTIONS_METADATA     "opencl.compiler.options"
#define SPIR_EXT_OPTIONS_METADATA "opencl.compiler.ext.options"

/**********************************************************************************************
* Description:
* Represents a test fixture for unit-testing clang_compiler in FE Compiler.
* By gtest design it is being instantiated with SetUp() call in the beginning of a test
* and destroyed with TearDown() call at the end of the test.
**********************************************************************************************/
class ClangCompilerTestType : public ::testing::Test {
protected:
    ClangCompilerTestType()
        : m_binary_result(nullptr), m_fe_compiler(nullptr), m_llvm_context(nullptr) {}

    virtual void SetUp();
    virtual void TearDown();

    std::vector<char>& GetSpirvBinaryContainer()
        { return m_spirv_program_binary; }
    Intel::OpenCL::FECompilerAPI::IOCLFECompiler* GetFECompiler()
        { return m_fe_compiler; }
    llvm::LLVMContext* GetLLVMContext()
        { return m_llvm_context; }

    // Reads a test SPIR-V program to a vector.
    void GetTestSPIRVProgram(std::vector<char>&);
    // Returns a test FESPIRVProgramDescriptor structure - an argument for ParseSPIRV.
    Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor GetTestFESPIRVProgramDescriptor(
        const char* build_options);
    // Returns a test FESPIRVProgramDescriptor structure - an argument for ParseSPIRV.
    template<typename T, std::size_t N>
    Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor GetTestFESPIRVProgramDescriptor(
        T (&bin) [N]) {
        using Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor;
        FESPIRVProgramDescriptor spirvDesc;

        spirvDesc.pSPIRVContainer = reinterpret_cast<char const*>(bin);
        spirvDesc.uiSPIRVContainerSize = N * sizeof(T);
        spirvDesc.pszOptions = "";

        return spirvDesc;
    }
    // Parses FE Compiler return structure and decodes llvm::Module.
    llvm::ErrorOr<std::unique_ptr<llvm::Module>> ExtractModule(Intel::OpenCL::ClangFE::IOCLFEBinaryResult* pResult);
protected:
    Intel::OpenCL::ClangFE::IOCLFEBinaryResult* m_binary_result;
private:
    std::vector<char> m_spirv_program_binary;
    Intel::OpenCL::FECompilerAPI::IOCLFECompiler* m_fe_compiler;
    llvm::LLVMContext* m_llvm_context;
};

#if defined (_WIN32)
#define DLL_IMPORT _declspec(dllimport)
#else
#define DLL_IMPORT
#endif

extern "C" DLL_IMPORT int CreateFrontEndInstance(
    const void* pDeviceInfo,
    size_t devInfoSize,
    Intel::OpenCL::FECompilerAPI::IOCLFECompiler* *pFECompiler,
    Intel::OpenCL::Utils::FrameworkUserLogger* pUserLogger);

#endif
