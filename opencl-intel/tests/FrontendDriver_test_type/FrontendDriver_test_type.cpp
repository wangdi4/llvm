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

#include "CL/cl.h"
#include "FrontendDriver.h"
#include "FrontendDriverFixture.h"
#include "clang_device_info.h"
#include "common_utils.h"
#include "frontend_api.h"
#include "gtest_wrapper.h"
#include "opencl_clang.h"

#include "SPIRV/libSPIRV/spirv_internal.hpp"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/SwapByteOrder.h"

#include <cstring>
#include <fstream>
#include <string>

using namespace llvm;
using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::FECompilerAPI;

TEST_F(ClangCompilerTestType, Test_LinkFail) {
  const char program1[] = R"(
    int foo1() {
        return 10;
    }
  )";
  const char program2[] = R"(
    int foo1() {
        return 20;
    }
  )";
  FECompileProgramDescriptor desc1;
  desc1.pProgramSource = program1;
  desc1.uiNumInputHeaders = 0;
  desc1.pInputHeaders = nullptr;
  desc1.pszInputHeadersNames = nullptr;
  desc1.pszOptions = "";
  desc1.bFpgaEmulator = false;

  FECompileProgramDescriptor desc2;
  desc2.pProgramSource = program2;
  desc2.uiNumInputHeaders = 0;
  desc2.pInputHeaders = nullptr;
  desc2.pszInputHeadersNames = nullptr;
  desc2.pszOptions = "";
  desc2.bFpgaEmulator = false;

  IOCLFEBinaryResult *binary_result1;
  int err = GetFECompiler()->CompileProgram(&desc1, &binary_result1);
  ASSERT_EQ(CL_SUCCESS, err) << "Failed to compiler program1.\n";

  IOCLFEBinaryResult *binary_result2;
  err = GetFECompiler()->CompileProgram(&desc2, &binary_result2);
  ASSERT_EQ(CL_SUCCESS, err) << "Failed to compiler program2.\n";

  const void *binary_container[] = {binary_result1->GetIR(),
                                    binary_result2->GetIR()};
  const size_t binary_size[] = {binary_result1->GetIRSize(),
                                binary_result2->GetIRSize()};
  FELinkProgramsDescriptor link_desc;
  link_desc.pBinaryContainers = binary_container;
  link_desc.uiNumBinaries = 2;
  link_desc.puiBinariesSizes = binary_size;
  link_desc.pszOptions = "";
  link_desc.pKernelNames = nullptr;

  IOCLFEBinaryResult *binary_result3;
  err = GetFECompiler()->LinkPrograms(&link_desc, &binary_result3);
  ASSERT_EQ(CL_LINK_PROGRAM_FAILURE, err)
      << "The program should fail to link.\n";

  std::string log(binary_result3->GetErrorLog());
  ASSERT_NE(log.find("symbol multiply defined"), std::string::npos)
      << "link error is not expected.\n";
}

TEST_F(ClangCompilerTestType, Test_PassingHeaders) {
  FECompileProgramDescriptor desc;

  const char header1[] = "#define HEADER_1\n";
  const char header2[] = "#define HEADER_2\n";

  const char header1_name[] = "header1.h";
  const char header2_name[] = "header2.h";

  const char kernel[] = "#include \"header1.h\"\n\
                           #include \"header2.h\"\n\
                           void __kernel kern() {\n\
                             #ifndef HEADER_1\n\
                             #error\n\
                             #endif\n\
                             #ifndef HEADER_2\n\
                             #error\n\
                             #endif\n\
                          }";

  const char build_options[] = "";

  std::vector<const char *> headers = {header1, header2};
  std::vector<const char *> header_names = {header1_name, header2_name};

  desc.pProgramSource = kernel;
  desc.uiNumInputHeaders = headers.size();
  desc.pInputHeaders = headers.data();
  desc.pszInputHeadersNames = header_names.data();
  desc.pszOptions = build_options;
  desc.bFpgaEmulator = false;

  int err = GetFECompiler()->CompileProgram(&desc, &m_binary_result);

  ASSERT_EQ(CL_SUCCESS, err)
      << "Headers have not been passed correctly." << std::endl
      << "The log: " << std::endl
      << m_binary_result->GetErrorLog() << std::endl;
}

TEST_F(ClangCompilerTestType, Test_FPAtomics) {
  const char kernel[] = "void __kernel kern(__global float* p, float v) {\
                             atomic_max((__global volatile float*)p, v);\
                           }";
  const char build_options[] = "-D float_atomics_enable";

  FECompileProgramDescriptor desc;
  desc.pProgramSource = kernel;
  desc.uiNumInputHeaders = 0;
  desc.pInputHeaders = nullptr;
  desc.pszInputHeadersNames = nullptr;
  desc.pszOptions = build_options;
  desc.bFpgaEmulator = false;

  int err = GetFECompiler()->CompileProgram(&desc, &m_binary_result);
  ASSERT_EQ(CL_SUCCESS, err)
      << "Pre-release header is not available." << std::endl
      << "The log: " << std::endl
      << m_binary_result->GetErrorLog() << std::endl;
}

TEST_F(ClangCompilerTestType, Test_OptNone) {
  const char kernel[] = "void __attribute__((optnone, noinline)) kern() {\n\
                          }";
  const char build_options[] = "";

  FECompileProgramDescriptor desc;
  desc.pProgramSource = kernel;
  desc.uiNumInputHeaders = 0;
  desc.pInputHeaders = nullptr;
  desc.pszInputHeadersNames = nullptr;
  desc.pszOptions = build_options;
  desc.bFpgaEmulator = false;

  int err = GetFECompiler()->CompileProgram(&desc, &m_binary_result);

  ASSERT_EQ(CL_SUCCESS, err) << "Optnone attribute is supported." << std::endl
                             << "The log: " << std::endl
                             << m_binary_result->GetErrorLog() << std::endl;
}

TEST_F(ClangCompilerTestType, Test_CTSSPIR12) {
  std::string FileName =
      get_exe_dir() +
      "opencl.cts.spir.test_atomic_fn.atomic_and_local_int.bc64";
  std::vector<unsigned char> SpirBinary;
  ASSERT_NO_FATAL_FAILURE(readBinary(FileName, SpirBinary));
  FESPIRProgramDescriptor SpirDesc{SpirBinary.data(),
                                   (unsigned)SpirBinary.size()};

  int Err = GetFECompiler()->MaterializeSPIR(&SpirDesc, &m_binary_result);
  ASSERT_EQ(Err, 0) << "Failed to parse SPIR 1.2 binary:\n"
                    << m_binary_result->GetErrorLog() << "\n";

  auto ModuleOrError = ExtractModule(m_binary_result);
  ASSERT_TRUE(bool(ModuleOrError))
      << ModuleOrError.getError().message() << "\n";

  // Check that opencl.kernels metadata is dropped.
  Module &M = **ModuleOrError;
  ASSERT_EQ(M.getNamedMetadata("opencl.kernels"), nullptr);

  // Check that TBAA metadata are all dropped.
  for (auto &F : **ModuleOrError)
    for (auto &I : instructions(&F))
      ASSERT_EQ(I.getMetadata(LLVMContext::MD_tbaa), nullptr);
}

// Check image call can be processed properly without errors on opaque
// pointer mode.
TEST_F(ClangCompilerTestType, Test_ChangeImageCall) {
  std::string FileName =
      get_exe_dir() + "opencl.cts.spir.get_image_info_1D.bc64";
  std::vector<unsigned char> SpirBinary;
  ASSERT_NO_FATAL_FAILURE(readBinary(FileName, SpirBinary));
  FESPIRProgramDescriptor SpirDesc{SpirBinary.data(),
                                   (unsigned)SpirBinary.size()};

  int Err = GetFECompiler()->MaterializeSPIR(&SpirDesc, &m_binary_result);
  ASSERT_EQ(Err, 0) << "Failed to parse SPIR 1.2 binary:\n"
                    << m_binary_result->GetErrorLog() << "\n";
}

TEST_F(ClangCompilerTestType, Test_PlainSpirvConversion)
// take a simple spirv file and make FE Compiler convert it to Module
{
  const char *build_options = "";

  FESPIRVProgramDescriptor spirvDesc =
      GetTestFESPIRVProgramDescriptor(build_options);

  int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);

  ASSERT_FALSE(err) << "Could not parse SPIR-V binary.\nThe log:"
                    << m_binary_result->GetErrorLog() << "\n";

  auto pModuleOrError = ExtractModule(m_binary_result);

  ASSERT_TRUE(bool(pModuleOrError))
      << "Module LLVM error: " << pModuleOrError.getError().value() << "\n"
      << "            message: " << pModuleOrError.getError().message() << "\n";
}

TEST_F(ClangCompilerTestType, Test_RejectInvalidCompileOption)
// pass an invalid option and see the parsing rejected
{
  std::string invalid_option = "-cl-kernel-arg-info--cl-opt-disable";

  FESPIRVProgramDescriptor spirvDesc =
      GetTestFESPIRVProgramDescriptor(invalid_option.c_str());

  int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_INVALID_COMPILER_OPTIONS, err)
      << "Unexpected retcode in presence of invalid compile options.\n";
}

// The following constants are used to make SPIR-V 1.1 BC in place
// (little-endian byte order)
const std::uint32_t SPIRV10Version = 0x00010000;
const std::uint32_t SPIRV12Version = 0x00010200;
const std::uint32_t SPIRVOpCapability = 0x00020000 | spv::OpCapability;
const std::uint32_t SPIRVOpMemoryModel = 0x00030000 | spv::OpMemoryModel;

static std::uint32_t const spvBCWithCommonSpirvCapabilities[] = {
    // First 5 mandatory words
    spv::MagicNumber, SPIRV12Version, 0, 0, 0,
    // Common capabilities
    SPIRVOpCapability, spv::CapabilityAddresses, SPIRVOpCapability,
    spv::CapabilityLinkage, SPIRVOpCapability, spv::CapabilityKernel,
    SPIRVOpCapability, spv::CapabilityVector16, SPIRVOpCapability,
    spv::CapabilityFloat16Buffer, SPIRVOpCapability, spv::CapabilityInt64,
    SPIRVOpCapability, spv::CapabilityPipes, SPIRVOpCapability,
    spv::CapabilityPipeStorage, SPIRVOpCapability, spv::CapabilityGroups,
    SPIRVOpCapability, spv::CapabilityDeviceEnqueue, SPIRVOpCapability,
    spv::CapabilityLiteralSampler, SPIRVOpCapability, spv::CapabilityInt16,
    SPIRVOpCapability, spv::CapabilityGenericPointer, SPIRVOpCapability,
    spv::CapabilityInt8, SPIRVOpCapability, spv::CapabilitySubgroupShuffleINTEL,
    SPIRVOpCapability, spv::CapabilitySubgroupBufferBlockIOINTEL,
    SPIRVOpCapability, spv::CapabilitySubgroupImageBlockIOINTEL,
    SPIRVOpCapability, spv::CapabilitySubgroupDispatch, SPIRVOpCapability,
    spv::CapabilityFunctionPointersINTEL, SPIRVOpCapability,
    spv::CapabilityIndirectReferencesINTEL, SPIRVOpCapability,
    spv::CapabilityAsmINTEL, SPIRVOpCapability,
    spv::CapabilityVariableLengthArrayINTEL, SPIRVOpCapability,
    spv::internal::CapabilityVectorVariantsINTEL, SPIRVOpCapability, // INTEL
    spv::CapabilityExpectAssumeKHR, SPIRVOpCapability,
    spv::CapabilityVectorAnyINTEL, SPIRVOpCapability,
    spv::CapabilityUnstructuredLoopControlsINTEL, SPIRVOpCapability,
    spv::CapabilityArbitraryPrecisionIntegersINTEL, SPIRVOpCapability,
    spv::CapabilityFPFastMathModeINTEL, SPIRVOpCapability,
    spv::CapabilityAtomicFloat32AddEXT, SPIRVOpCapability,
    spv::CapabilityAtomicFloat16MinMaxEXT, SPIRVOpCapability,
    spv::CapabilityAtomicFloat32MinMaxEXT, SPIRVOpCapability,
    spv::CapabilityAtomicFloat64MinMaxEXT, SPIRVOpCapability,
    spv::internal::CapabilityOptNoneINTEL, SPIRVOpCapability,
    spv::CapabilityMemoryAccessAliasingINTEL, SPIRVOpCapability,
    spv::internal::CapabilityTokenTypeINTEL, SPIRVOpCapability,
    spv::CapabilityDebugInfoModuleINTEL, SPIRVOpCapability,
    spv::CapabilityRuntimeAlignedAttributeINTEL, SPIRVOpCapability,
    spv::CapabilityLongConstantCompositeINTEL, SPIRVOpCapability,
    spv::internal::CapabilityBfloat16ConversionINTEL, SPIRVOpCapability,
    spv::internal::CapabilityGlobalVariableDecorationsINTEL, SPIRVOpCapability,
    spv::CapabilityGroupNonUniformArithmetic, SPIRVOpCapability,
    spv::CapabilityGroupNonUniformBallot, SPIRVOpCapability,
    spv::CapabilityGroupNonUniformShuffle, SPIRVOpCapability,
    spv::CapabilityGroupNonUniformShuffleRelative, SPIRVOpCapability,
    spv::CapabilityGroupUniformArithmeticKHR, SPIRVOpCapability,
    spv::internal::CapabilityMaskedGatherScatterINTEL, SPIRVOpCapability,
    spv::CapabilityAtomicFloat64AddEXT, SPIRVOpCapability,
    spv::internal::CapabilityJointMatrixINTEL, SPIRVOpCapability,
    spv::internal::CapabilityJointMatrixWIInstructionsINTEL, SPIRVOpCapability,
    spv::internal::CapabilityJointMatrixTF32ComponentTypeINTEL,
    SPIRVOpCapability,
    spv::internal::CapabilityJointMatrixBF16ComponentTypeINTEL,
    SPIRVOpCapability, spv::internal::CapabilityTensorFloat32RoundingINTEL,
    SPIRVOpCapability, spv::CapabilityFPMaxErrorINTEL,

    // Memory model
    SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL};

static std::uint32_t const spvBCWithFPGASpirvCapabilities[] = {
    // First 5 mandatory words
    spv::MagicNumber, SPIRV12Version, 0, 0, 0,
    // Common capabilities
    SPIRVOpCapability, spv::CapabilityFPGAMemoryAttributesINTEL,
    SPIRVOpCapability, spv::CapabilityFPGALoopControlsINTEL, SPIRVOpCapability,
    spv::CapabilityFPGARegINTEL, SPIRVOpCapability,
    spv::CapabilityBlockingPipesINTEL, SPIRVOpCapability,
    spv::CapabilityKernelAttributesINTEL, SPIRVOpCapability,
    spv::CapabilityFPGAKernelAttributesINTEL, SPIRVOpCapability,
    spv::CapabilityArbitraryPrecisionFixedPointINTEL, SPIRVOpCapability,
    spv::CapabilityArbitraryPrecisionFloatingPointINTEL, SPIRVOpCapability,
    spv::CapabilityFPGAMemoryAccessesINTEL, SPIRVOpCapability,
    spv::CapabilityIOPipesINTEL, SPIRVOpCapability,
    spv::CapabilityUSMStorageClassesINTEL, SPIRVOpCapability,
    spv::CapabilityFPGABufferLocationINTEL, SPIRVOpCapability,
    spv::CapabilityFPGAClusterAttributesINTEL, SPIRVOpCapability,
    spv::CapabilityLoopFuseINTEL, SPIRVOpCapability,
    spv::CapabilityFPGADSPControlINTEL, SPIRVOpCapability,
    spv::CapabilityFPGAInvocationPipeliningAttributesINTEL, SPIRVOpCapability,
    spv::CapabilityFPGAKernelAttributesv2INTEL, SPIRVOpCapability,
    spv::CapabilityFPGALatencyControlINTEL, SPIRVOpCapability,
    spv::internal::CapabilityFPArithmeticFenceINTEL, SPIRVOpCapability,
    spv::internal::CapabilityTaskSequenceINTEL, // INTEL
    SPIRVOpCapability, spv::CapabilityFPGAArgumentInterfacesINTEL,

    // Memory model
    SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL};

// test that a module with device agnostic capabilities is accepted by FE
TEST_F(ClangCompilerTestType, Test_AcceptCommonSpirvCapabilitiesLittleEndian) {
  auto spirvDesc =
      GetTestFESPIRVProgramDescriptor(spvBCWithCommonSpirvCapabilities);

  int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_SUCCESS, err)
      << "Unexpected retcode for a valid SPIR-V module.\n";
}

// test that a module with SPIR-V 1.0 version is accepted by FE
TEST_F(ClangCompilerTestType, Test_AcceptSPIRV10) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {// First 5 mandatory words
                                 spv::MagicNumber, SPIRV10Version, 0, 0, 0,
                                 // Common capabilities
                                 SPIRVOpCapability, spv::CapabilityKernel,
                                 // Memory model
                                 SPIRVOpMemoryModel,
                                 spv::AddressingModelPhysical64,
                                 spv::MemoryModelOpenCL};
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_SUCCESS, err)
      << "ERROR: SPIR-V 1.0 was rejected by FE though is supported.\n";
}

// test that a moudle with SPIR-V 1.2 version is accepted by FE
TEST_F(ClangCompilerTestType, Test_AcceptSPIRV12) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {// First 5 mandatory words
                                 spv::MagicNumber, SPIRV12Version, 0, 0, 0,
                                 // Common capabilities
                                 SPIRVOpCapability, spv::CapabilityKernel,
                                 // Memory model
                                 SPIRVOpMemoryModel,
                                 spv::AddressingModelPhysical64,
                                 spv::MemoryModelOpenCL};
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_SUCCESS, err)
      << "ERROR: SPIR-V 1.2 was rejected by FE though is supported.\n";
}

// Enable the following subtest once the byte order bug is fixed in SPIR-V
// consumer
// https://github.com/KhronosGroup/SPIRV-LLVM/issues/132
// test that a module with device agnostic capabilities is accepted by FE
TEST_F(ClangCompilerTestType,
       DISABLED_Test_AcceptCommonSpirvCapabilitiesBigEndian) {
  constexpr size_t len = sizeof(spvBCWithCommonSpirvCapabilities) /
                         sizeof(spvBCWithCommonSpirvCapabilities[0]);
  std::uint32_t spvBC[len];
  std::copy(std::begin(spvBCWithCommonSpirvCapabilities),
            std::end(spvBCWithCommonSpirvCapabilities), std::begin(spvBC));
  // Swap byte order of SPIR-V BC
  for (auto &word : spvBC)
    word = byteswap(word);

  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_SUCCESS, err)
      << "Unexpected retcode for a valid SPIR-V module.\n";
}

// test that a module w\o mandatory memory model instruction is rejected
TEST_F(ClangCompilerTestType, Test_NoSpirvMemoryModel) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {
      // First 5 mandatory words
      spv::MagicNumber, SPIRV12Version, 0, 0, 0,
      // Common capabilities
      SPIRVOpCapability, spv::CapabilityAddresses,
      // No mandatory memory model
  };
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_INVALID_PROGRAM, err)
      << "Unexpected retcode for a SPIR-V module w\\o mandatory OpMemoryModel "
         ".\n";
}

// test that a module requiring fp64 and images is accepted by a device
// which supports it
TEST_F(ClangCompilerTestType, Test_SpirvWithFP64AndImages) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {
      // First 5 mandatory words
      spv::MagicNumber, SPIRV12Version, 0, 0, 0,
      // Common capabilities
      SPIRVOpCapability, spv::CapabilitySampled1D, SPIRVOpCapability,
      spv::CapabilitySampledBuffer, SPIRVOpCapability,
      spv::CapabilityImageBasic, SPIRVOpCapability,
      spv::CapabilityImageReadWrite, SPIRVOpCapability, spv::CapabilityFloat64,
      // Memory model
      SPIRVOpMemoryModel, spv::AddressingModelPhysical64,
      spv::MemoryModelOpenCL};
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      "",    // features
      true,  // images support
      false, // fp16 support
      true,  // fp64 support
      false, // source level profiling
      false  // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
  ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

  err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_SUCCESS, err)
      << "Unexpected retcode for a device supporting images/fp64 .\n";
}

// test that a module requiring fp64 is rejected by a device which doesn't
// support fp64
TEST_F(ClangCompilerTestType, Test_SpirvDeviceWOFP64) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {// First 5 mandatory words
                                 spv::MagicNumber, SPIRV12Version, 0, 0, 0,
                                 // Common capabilities
                                 SPIRVOpCapability, spv::CapabilityFloat64,
                                 // Memory model
                                 SPIRVOpMemoryModel,
                                 spv::AddressingModelPhysical64,
                                 spv::MemoryModelOpenCL};
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      "",    // features
      true,  // images support
      false, // fp16 support
      false, // fp64 support
      false, // source level profiling
      false  // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
  ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

  err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_INVALID_PROGRAM, err)
      << "Unexpected retcode for a device w\\o fp64 support.\n";
}

// test that a module requiring images is rejected by a device which doesn't
// support images
TEST_F(ClangCompilerTestType, Test_SpirvDeviceWOImages) {
  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      "",    // features
      false, // images support
      false, // fp16 support
      true,  // fp64 support
      false, // source level profiling
      false  // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
  ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

  std::uint32_t imageCapabilities[] = {
      spv::CapabilitySampled1D, spv::CapabilitySampledBuffer,
      spv::CapabilityImageBasic, spv::CapabilityImageReadWrite};
  for (auto IC : imageCapabilities) {
    // Hand made SPIR-V module
    std::uint32_t const spvBC[] = {// First 5 mandatory words
                                   spv::MagicNumber, SPIRV12Version, 0, 0, 0,
                                   // Image capability
                                   SPIRVOpCapability, IC,
                                   // Memory model
                                   SPIRVOpMemoryModel,
                                   spv::AddressingModelPhysical64,
                                   spv::MemoryModelOpenCL};
    auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

    err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_EQ(CL_INVALID_PROGRAM, err)
        << "Unexpected retcode for a device w\\o image support.\n";
  }
}

// test that a module requiring fp16  is accepted by a device
// which supports it and is rejected by a device which doesn't
// support fp16
TEST_F(ClangCompilerTestType, Test_SpirvWithFP16) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {// First 5 mandatory words
                                 spv::MagicNumber, SPIRV12Version, 0, 0, 0,
                                 // Common capabilities
                                 SPIRVOpCapability, spv::CapabilityFloat16,
                                 // Memory model
                                 SPIRVOpMemoryModel,
                                 spv::AddressingModelPhysical64,
                                 spv::MemoryModelOpenCL};
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  CLANG_DEV_INFO devInfo1 = {
      "",    // extensions
      "",    // features
      false, // images support
      false, // fp16 support
      false, // fp64 support
      false, // source level profiling
      false  // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler1;
  IOCLFECompiler *pFeCompiler1 = spFeCompiler1.get();

  int err1 = CreateFrontEndInstance(&devInfo1, sizeof(devInfo1), &pFeCompiler1);
  ASSERT_EQ(0, err1) << "Failed to create FE instance.\n";

  err1 = pFeCompiler1->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_INVALID_PROGRAM, err1)
      << "Unexpected retcode for a device w\\o fp16 support.\n";

  CLANG_DEV_INFO devInfo2 = {
      "",    // extensions
      "",    // features
      false, // images support
      true,  // fp16 support
      false, // fp64 support
      false, // source level profiling
      false  // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler2;
  IOCLFECompiler *pFeCompiler2 = spFeCompiler2.get();

  int err2 = CreateFrontEndInstance(&devInfo2, sizeof(devInfo2), &pFeCompiler2);
  ASSERT_EQ(0, err2) << "Failed to create FE instance.\n";

  err2 = pFeCompiler2->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_SUCCESS, err2)
      << "Unexpected retcode for a device with fp16 support.\n";
}

TEST_F(ClangCompilerTestType, Test_SPIRV_BIsRepresentation) {
  // Without -cl-std=CL2.0 option, the SPIR-V will be converted to IR with
  // OpenCL 1.2 builtins. The atomic_load_explicit builtin is not defined in
  // OpenCL 1.2 spec, and it's converted to atomic_add builtin as workaround.
  const char *default_build_options = "";
  FESPIRVProgramDescriptor spirvDesc =
      GetTestFESPIRVProgramDescriptor(default_build_options);
  int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_FALSE(err) << "Cannot parse SPIR-V binary.\nThe log:"
                    << m_binary_result->GetErrorLog() << "\n";
  const char *IRBuffer =
      reinterpret_cast<const char *>(m_binary_result->GetIR());
  const int IRSize = m_binary_result->GetIRSize();
  std::string IRString(IRBuffer, IRSize);
  ASSERT_TRUE(IRString.find("_Z10atomic_addPU3AS4Vii") != std::string::npos)
      << "Cannot find expected atomic builtins" << std::endl;
  auto pModuleOrError = ExtractModule(m_binary_result);
  ASSERT_TRUE(bool(pModuleOrError))
      << "Module LLVM error: " << pModuleOrError.getError().value() << "\n"
      << "            message: " << pModuleOrError.getError().message() << "\n";

  // With -cl-std=CL2.0 option, the SPIR-V will be converted to IR with OpenCL
  // 2.0 builtins. The atomic_load_explicit builtin is defined in OpenCL 2.0
  // spec, and it's converted to the same builtin.
  const char *cl20_build_options = "-cl-std=CL2.0";
  spirvDesc = GetTestFESPIRVProgramDescriptor(cl20_build_options);
  err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_FALSE(err) << "Could not parse SPIR-V binary.\nThe log:"
                    << m_binary_result->GetErrorLog() << "\n";
  const char *CL20IRBuffer =
      reinterpret_cast<const char *>(m_binary_result->GetIR());
  const int CL20IRSize = m_binary_result->GetIRSize();
  std::string CL20IRString(CL20IRBuffer, CL20IRSize);
  ASSERT_TRUE(CL20IRString.find(
                  "_Z20atomic_load_explicitPU3AS4VU7_Atomici12memory_order") !=
              std::string::npos)
      << "Cannot find correct atomic builtins" << std::endl;

  pModuleOrError = ExtractModule(m_binary_result);
  ASSERT_TRUE(bool(pModuleOrError))
      << "Module LLVM error: " << pModuleOrError.getError().value() << "\n"
      << "            message: " << pModuleOrError.getError().message() << "\n";
}

// Test that a module with FPGA device agnostic capabilities is accepted by FE.
TEST_F(ClangCompilerTestType, Test_AcceptFPGASpirvCapabilitiesOnFPGA) {
  auto spirvDesc =
      GetTestFESPIRVProgramDescriptor(spvBCWithFPGASpirvCapabilities);

  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      "",    // features
      false, // images support
      false, // fp16 support
      true,  // fp64 support
      false, // source level profiling
      true   // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
  ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

  err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_EQ(CL_SUCCESS, err)
      << "Unexpected retcode for a valid SPIR-V module.\n";
}

// test that a module with FPGA device agnostic capabilities is rejected by FE
TEST_F(ClangCompilerTestType, Test_RejectCommonSpirvCapabilitiesOnFPGA) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {
      // First 5 mandatory words
      spv::MagicNumber, SPIRV12Version, 0, 0, 0,
      // Common capabilities
      SPIRVOpCapability, spv::CapabilityInt64Atomics,
      // Memory model
      SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL

  };
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      "",    // features
      false, // images support
      false, // fp16 support
      true,  // fp64 support
      false, // source level profiling
      true   // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
  ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

  err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_NE(CL_SUCCESS, err)
      << "Unexpected retcode for an invalid SPIR-V module.\n";
}

// test the capability is accepted on both CPU and FPGA device
TEST_F(ClangCompilerTestType, Test_AcceptCommonSpirvCapabilitiesOnCPUAndFPGA) {
  auto spirvDesc =
      GetTestFESPIRVProgramDescriptor(spvBCWithCommonSpirvCapabilities);

  for (int i = 0; i < 2; i++) {
    CLANG_DEV_INFO devInfo = {
        "",    // extensions
        "",    // features
        false, // images support
        false, // fp16 support
        true,  // fp64 support
        false, // source level profiling
        true   // true: fpga emu; false: CPU
    };
    if (i == 1)
      devInfo.bIsFPGAEmu = false;
    std::unique_ptr<IOCLFECompiler> spFeCompiler;
    IOCLFECompiler *pFeCompiler = spFeCompiler.get();

    int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
    ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

    err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_EQ(CL_SUCCESS, err)
        << "Unexpected retcode for a valid SPIR-V module.\n";
  }
}

// Test that a module with FPGA device agnostic capabilities is rejected by FE
// for CPU device.
TEST_F(ClangCompilerTestType, Test_RejectFPGASpirvCapabilitiesOnCPU) {
  // This needs to be consistent with the function
  // ClangFECompilerParseSPIRVTask::isSPIRVSupported
  std::vector<std::string> SpvModuleName = {
      "FPGAMemoryAttributes",
      "FPGALoopControls",
      "FPGAReg",
      "BlockingPipes",
      "KernelAttributes",
      "FPGAKernelAttributes",
      "ArbitraryPrecisionFixedPoint",
      "ArbitraryPrecisionFloatingPoint",
      "FPGAMemoryAccesses",
      "IOPipes",
      "USMStorageClasses",
      "FPGABufferLocation",
      "FPGAClusterAttributes",
      "LoopFuse",
      "FPGADSPControl",
      "FPGAInvocationPipeliningAttributes",
      "FPGAArgumentInterfaces",
      "FPGAKernelAttributesv2",
      "FPArithmeticFence",
      "TaskSequence"};

  auto spirvDesc =
      GetTestFESPIRVProgramDescriptor(spvBCWithFPGASpirvCapabilities);

  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      "",    // features
      false, // images support
      false, // fp16 support
      true,  // fp64 support
      false, // source level profiling
      false  // CPU
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
  ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

  err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_NE(CL_SUCCESS, err)
      << "Unexpected retcode for an invalid SPIR-V module.\n";

  std::string ErrLog(m_binary_result->GetErrorLog());
  ASSERT_TRUE(std::any_of(SpvModuleName.begin(), SpvModuleName.end(),
                          [&](const std::string &str) {
                            return ErrLog.find(str) != std::string::npos;
                          }))
      << "Unsupported capability name not found in error log\n";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
