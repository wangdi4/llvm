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

TEST_F(ClangCompilerTestType, Test_ValidSpirvProgramCompileOptions)
// parse a spir-v program and check output llvm::Module for build options metadata
{
    std::vector<std::string> expected_options_vector;
    expected_options_vector.push_back("-cl-kernel-arg-info");
    expected_options_vector.push_back("-cl-opt-disable");

    std::string build_options;

    for (auto s : expected_options_vector)
        build_options += s + " ";

    FESPIRVProgramDescriptor spirvDesc = GetTestFESPIRVProgramDescriptor(build_options.c_str());

    int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_FALSE(err) << "Could not parse SPIR-V binary.\nThe log:" << m_binary_result->GetErrorLog() << "\n";

    auto pModuleOrError = ExtractModule(m_binary_result);

    ASSERT_TRUE(bool(pModuleOrError)) << "Module LLVM error: " << pModuleOrError.getError().value() << "\n"
                                      << "            message: " << pModuleOrError.getError().message() << "\n";

    llvm::Module* pModule = pModuleOrError.get();

    llvm::NamedMDNode *OCLCompOptsMD = pModule->getNamedMetadata(SPIR_OPTIONS_METADATA);
    ASSERT_TRUE(OCLCompOptsMD->getNumOperands() == 1) <<
        "Module does not contain \"" << SPIR_OPTIONS_METADATA << "\" metadata.\n";

    llvm::MDNode* option_node_list = OCLCompOptsMD->getOperand(0);
    ASSERT_TRUE(option_node_list) << "\"" << SPIR_OPTIONS_METADATA << "\" does not contain a required node.\n";

    ASSERT_EQ(expected_options_vector.size(), option_node_list->getNumOperands())
        << "Number of options in \"" << SPIR_OPTIONS_METADATA << "\" is unexpected.\n";

    std::vector<std::string> actual_options_vector;
    for(unsigned i = 0; i < option_node_list->getNumOperands(); ++i)
    {
        llvm::MDString* option_name = llvm::dyn_cast<llvm::MDString>(option_node_list->getOperand(i));

        ASSERT_TRUE(option_name) << "\"" << SPIR_OPTIONS_METADATA << "\" contains smth besides the MDStrings.\n";

        auto option_found = std::string(option_name->getString());

        actual_options_vector.push_back(option_found);
    }

    for(auto expected_option : expected_options_vector)
    {
        auto option_found_iter = std::find(actual_options_vector.begin(), actual_options_vector.end(), expected_option);

        ASSERT_FALSE(option_found_iter == actual_options_vector.end())
            << "Expected option not found in \"" << SPIR_OPTIONS_METADATA << "\" metadata.\n";
    }
}

TEST_F(ClangCompilerTestType, Test_RejectInvalidCompileOption)
// pass an invalid option and see the parsing rejected
{
    std::string invalid_option = "-cl-kernel-arg-info--cl-opt-disable";

    FESPIRVProgramDescriptor spirvDesc = GetTestFESPIRVProgramDescriptor(invalid_option.c_str());

    int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_EQ(CL_INVALID_COMPILER_OPTIONS, err) << "Unexpected retcode in presence of invalid compile options.\n";
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
