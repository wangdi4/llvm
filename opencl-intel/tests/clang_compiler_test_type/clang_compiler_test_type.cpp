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

#include <CL/cl.h>
#include <gtest/gtest.h>
#include <clang_compiler.h>
#include <frontend_api.h>
#include <clang_device_info.h>

#include "ClangCompilerTestTypeFixture.h"

#include <string>
#include <fstream>
#include <cstring>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::FECompilerAPI;

TEST_F(ClangCompilerTestType, Test_PlainSpirvConversion)
// take a simple spirv file and make FE Compiler convert it to llvm::Module
{
    const char* build_options = "";

    FESPIRVProgramDescriptor spirvDesc = GetTestFESPIRVProgramDescriptor(build_options);

    int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);

    ASSERT_FALSE(err) << "Could not parse SPIR-V binary.\nThe log:" << m_binary_result->GetErrorLog() << "\n";

    auto pModuleOrError = ExtractModule(m_binary_result);

    ASSERT_TRUE(bool(pModuleOrError)) << "Module LLVM error: " << pModuleOrError.getError().value() << "\n"
                                      << "            message: " << pModuleOrError.getError().message() << "\n";
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
