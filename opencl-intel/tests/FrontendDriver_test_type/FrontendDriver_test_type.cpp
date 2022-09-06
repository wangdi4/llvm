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

#include "FrontendDriver.h"
#include "FrontendDriverFixture.h"
#include "clang_device_info.h"
#include "common_clang.h"
#include "common_utils.h"
#include "frontend_api.h"
#include "gtest_wrapper.h"
#include <CL/cl.h>

#include "SPIRV/libSPIRV/spirv_internal.hpp"

#include "llvm/IR/InstIterator.h"
#include "llvm/Support/SwapByteOrder.h"

#include <string>
#include <fstream>
#include <cstring>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::FECompilerAPI;

TEST_F(ClangCompilerTestType, Test_PassingHeaders)
{
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

    std::vector<const char*> headers = {header1, header2};
    std::vector<const char*> header_names = {header1_name, header2_name};

    desc.pProgramSource = kernel;
    desc.uiNumInputHeaders = headers.size();
    desc.pInputHeaders = headers.data();
    desc.pszInputHeadersNames = header_names.data();
    desc.pszOptions = build_options;
    desc.bFpgaEmulator = false;

    int err = GetFECompiler()->CompileProgram(&desc, &m_binary_result);

    ASSERT_EQ(CL_SUCCESS, err) << "Headers have not been passed correctly." << std::endl
                               << "The log: " << std::endl
                               << m_binary_result->GetErrorLog() << std::endl;
}

TEST_F(ClangCompilerTestType, Test_FPAtomics)
{
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
    ASSERT_EQ(CL_SUCCESS, err) << "Pre-release header is not available." << std::endl
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

TEST_F(ClangCompilerTestType, Test_RejectInvalidCompileOption)
// pass an invalid option and see the parsing rejected
{
    std::string invalid_option = "-cl-kernel-arg-info--cl-opt-disable";

    FESPIRVProgramDescriptor spirvDesc = GetTestFESPIRVProgramDescriptor(invalid_option.c_str());

    int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_EQ(CL_INVALID_COMPILER_OPTIONS, err) << "Unexpected retcode in presence of invalid compile options.\n";
}

// The following constants are used to make SPIR-V 1.1 BC in place
// (little-endian byte order)
const std::uint32_t SPIRV10Version = 0x00010000;
const std::uint32_t SPIRV12Version = 0x00010200;
const std::uint32_t SPIRVOpCapability = 0x00020000 | spv::OpCapability;
const std::uint32_t SPIRVOpMemoryModel = 0x00030000 | spv::OpMemoryModel;

// test that a module with device agnostic capabilities is accepted by FE
TEST_F(ClangCompilerTestType, Test_AcceptCommonSpirvCapabilitiesLittleEndian) {
    // Hand made SPIR-V module
    std::uint32_t const spvBC[] = {
        // First 5 mandatory words
        spv::MagicNumber, SPIRV12Version, 0, 0, 0,
        // Common capabilities
        SPIRVOpCapability,  spv::CapabilityAddresses,
        SPIRVOpCapability,  spv::CapabilityLinkage,
        SPIRVOpCapability,  spv::CapabilityKernel,
        SPIRVOpCapability,  spv::CapabilityVector16,
        SPIRVOpCapability,  spv::CapabilityFloat16Buffer,
        SPIRVOpCapability,  spv::CapabilityInt64,
        SPIRVOpCapability,  spv::CapabilityPipes,
        SPIRVOpCapability,  spv::CapabilityGroups,
        SPIRVOpCapability,  spv::CapabilityDeviceEnqueue,
        SPIRVOpCapability,  spv::CapabilityLiteralSampler,
        SPIRVOpCapability,  spv::CapabilityInt16,
        SPIRVOpCapability,  spv::CapabilityGenericPointer,
        SPIRVOpCapability,  spv::CapabilityInt8,
        SPIRVOpCapability,  spv::CapabilityInt64Atomics,
        SPIRVOpCapability,  spv::CapabilityAsmINTEL,
        SPIRVOpCapability,  spv::CapabilityVariableLengthArrayINTEL,
        SPIRVOpCapability,  spv::internal::CapabilityJointMatrixINTEL,
        SPIRVOpCapability, spv::CapabilityLongConstantCompositeINTEL,
        SPIRVOpCapability, spv::internal::CapabilityBfloat16ConversionINTEL,
        SPIRVOpCapability, spv::internal::CapabilityGlobalVariableDecorationsINTEL,
        SPIRVOpCapability, spv::CapabilityGroupNonUniformBallot,

        // Memory model
        SPIRVOpMemoryModel, spv::AddressingModelPhysical32, spv::MemoryModelOpenCL
    };
    auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

    int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_EQ(CL_SUCCESS, err) << "Unexpected retcode for a valid SPIR-V module.\n";
}

// test that a module with SPIR-V 1.0 version is accepted by FE
TEST_F(ClangCompilerTestType, Test_AcceptSPIRV10) {
   // Hand made SPIR-V module
   std::uint32_t const spvBC[] = {
       // First 5 mandatory words
       spv::MagicNumber, SPIRV10Version, 0, 0, 0,
       // Common capabilities
       SPIRVOpCapability,  spv::CapabilityKernel,
       // Memory model
       SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL
   };
   auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

   int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
   ASSERT_EQ(CL_SUCCESS, err)
       << "ERROR: SPIR-V 1.0 was rejected by FE though is supported.\n";
}

// test that a moudle with SPIR-V 1.2 version is accepted by FE
TEST_F(ClangCompilerTestType, Test_AcceptSPIRV12) {
   // Hand made SPIR-V module
   std::uint32_t const spvBC[] = {
       // First 5 mandatory words
       spv::MagicNumber, SPIRV12Version, 0, 0, 0,
       // Common capabilities
       SPIRVOpCapability,  spv::CapabilityKernel,
       // Memory model
       SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL
   };
   auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

   int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
   ASSERT_EQ(CL_SUCCESS, err)
       << "ERROR: SPIR-V 1.2 was rejected by FE though is supported.\n";
}

// Enable the following subtest once the byte order bug is fixed in SPIR-V consumer
// https://jira01.devtools.intel.com/browse/CORC-1111
// https://github.com/KhronosGroup/SPIRV-LLVM/issues/132
// test that a module with device agnostic capabilities is accepted by FE
TEST_F(ClangCompilerTestType, DISABLED_Test_AcceptCommonSpirvCapabilitiesBigEndian) {
    // Hand made SPIR-V module
    std::uint32_t spvBC[] = {
        // First 5 mandatory words
        spv::MagicNumber, SPIRV12Version, 0, 0, 0,
        // Common capabilities
        SPIRVOpCapability,  spv::CapabilityAddresses,
        SPIRVOpCapability,  spv::CapabilityLinkage,
        SPIRVOpCapability,  spv::CapabilityKernel,
        SPIRVOpCapability,  spv::CapabilityVector16,
        SPIRVOpCapability,  spv::CapabilityFloat16Buffer,
        SPIRVOpCapability,  spv::CapabilityInt64,
        SPIRVOpCapability,  spv::CapabilityPipes,
        SPIRVOpCapability,  spv::CapabilityGroups,
        SPIRVOpCapability,  spv::CapabilityDeviceEnqueue,
        SPIRVOpCapability,  spv::CapabilityLiteralSampler,
        SPIRVOpCapability,  spv::CapabilityInt16,
        SPIRVOpCapability,  spv::CapabilityGenericPointer,
        SPIRVOpCapability,  spv::CapabilityInt8,
        SPIRVOpCapability,  spv::CapabilityInt64Atomics,
        SPIRVOpCapability,  spv::CapabilityAsmINTEL,
        SPIRVOpCapability,  spv::CapabilityVariableLengthArrayINTEL,
        SPIRVOpCapability,  spv::internal::CapabilityJointMatrixINTEL,
        SPIRVOpCapability, spv::CapabilityLongConstantCompositeINTEL,
        SPIRVOpCapability, spv::internal::CapabilityBfloat16ConversionINTEL,
        SPIRVOpCapability, spv::internal::CapabilityGlobalVariableDecorationsINTEL,
        SPIRVOpCapability, spv::CapabilityGroupNonUniformBallot,

        // Memory model
        SPIRVOpMemoryModel, spv::AddressingModelPhysical32, spv::MemoryModelOpenCL
    };
    // Swap byte order of SPIR-V BC
    for(auto & word : spvBC)
        word = llvm::ByteSwap_32(word);

    auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

    int err = GetFECompiler()->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_EQ(CL_SUCCESS, err) << "Unexpected retcode for a valid SPIR-V module.\n";
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
        << "Unexpected retcode for a SPIR-V module w\\o mandatory OpMemoryModel .\n";
}

// test that a module requiring fp64 and images is accepted by a device
// which supports it
TEST_F(ClangCompilerTestType, Test_SpirvWithFP64AndImages) {
    // Hand made SPIR-V module
    std::uint32_t const spvBC[] = {
        // First 5 mandatory words
        spv::MagicNumber, SPIRV12Version, 0, 0, 0,
        // Common capabilities
        SPIRVOpCapability, spv::CapabilitySampled1D,
        SPIRVOpCapability, spv::CapabilitySampledBuffer,
        SPIRVOpCapability, spv::CapabilityImageBasic,
        SPIRVOpCapability, spv::CapabilityImageReadWrite,
        SPIRVOpCapability, spv::CapabilityFloat64,
        // Memory model
        SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL
    };
    auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

    CLANG_DEV_INFO devInfo = {
        "",    // extensions
        true,  // images support
        true,  // fp64 support
        false, // source level profiling
        false  // fpga emu
    };
    std::unique_ptr<IOCLFECompiler> spFeCompiler;
    IOCLFECompiler * pFeCompiler = spFeCompiler.get();

    int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
    ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

    err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_EQ(CL_SUCCESS, err)
        << "Unexpected retcode for a device supporting images/fp64 .\n";
}

// test that a module requiring fp64 is rejected by a device which doesn't support fp64
TEST_F(ClangCompilerTestType, Test_SpirvDeviceWOFP64) {
    // Hand made SPIR-V module
    std::uint32_t const spvBC[] = {
        // First 5 mandatory words
        spv::MagicNumber, SPIRV12Version, 0, 0, 0,
        // Common capabilities
        SPIRVOpCapability, spv::CapabilityFloat64,
        // Memory model
        SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL
    };
    auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

    CLANG_DEV_INFO devInfo = {
        "",    // extensions
        true,  // images support
        false, // fp64 support
        false, // source level profiling
        false  // fpga emu
    };
    std::unique_ptr<IOCLFECompiler> spFeCompiler;
    IOCLFECompiler * pFeCompiler = spFeCompiler.get();

    int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
    ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

    err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
    ASSERT_EQ(CL_INVALID_PROGRAM, err)
        << "Unexpected retcode for a device w\\o fp64 support.\n";
}

// test that a module requiring images is rejected by a device which doesn't support images
TEST_F(ClangCompilerTestType, Test_SpirvDeviceWOImages) {
    CLANG_DEV_INFO devInfo = {
        "",    // extensions
        false, // images support
        true,  // fp64 support
        false, // source level profiling
        false  // fpga emu
    };
    std::unique_ptr<IOCLFECompiler> spFeCompiler;
    IOCLFECompiler * pFeCompiler = spFeCompiler.get();

    int err = CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
    ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

    std::uint32_t imageCapabilities[] = {spv::CapabilitySampled1D,
        spv::CapabilitySampledBuffer, spv::CapabilityImageBasic,
        spv::CapabilityImageReadWrite};
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

// test that a module with FPGA device agnostic capabilities is accepted by FE
TEST_F(ClangCompilerTestType, Test_AcceptCommonSpirvCapabilitiesOnFPGA) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {
      // First 5 mandatory words
      spv::MagicNumber, SPIRV12Version, 0, 0, 0,
      // Common capabilities
      SPIRVOpCapability, spv::CapabilityFPGAMemoryAttributesINTEL,
      SPIRVOpCapability, spv::CapabilityFPGALoopControlsINTEL,
      SPIRVOpCapability, spv::CapabilityFPGARegINTEL, SPIRVOpCapability,
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
      spv::internal::CapabilityFPGADSPControlINTEL, SPIRVOpCapability,
      spv::internal::CapabilityFPGAInvocationPipeliningAttributesINTEL,
      SPIRVOpCapability, spv::internal::CapabilityFPArithmeticFenceINTEL,
      SPIRVOpCapability, spv::CapabilityLongConstantCompositeINTEL,
      SPIRVOpCapability, spv::internal::CapabilityTaskSequenceINTEL, // INTEL
      SPIRVOpCapability, spv::internal::CapabilityBfloat16ConversionINTEL,
      SPIRVOpCapability, spv::internal::CapabilityGlobalVariableDecorationsINTEL,
      SPIRVOpCapability, spv::CapabilityGroupNonUniformBallot,

      // Memory model
      SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL

  };
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      false, // images support
      true,  // fp64 support
      false, // source level profiling
      true   // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err =
      CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
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
      SPIRVOpCapability, spv::CapabilityInt64Atomics, SPIRVOpCapability,
      spv::CapabilityAtomicFloat64AddEXT,
      // Memory model
      SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL

  };
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      false, // images support
      true,  // fp64 support
      false, // source level profiling
      true   // fpga emu
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err =
      CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
  ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

  err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_NE(CL_SUCCESS, err)
      << "Unexpected retcode for an invalid SPIR-V module.\n";
}

// test the capability is accepted on both CPU and FPGA device
TEST_F(ClangCompilerTestType, Test_AcceptCommonSpirvCapabilitiesOnCPUAndFPGA) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {
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
      spv::CapabilityInt8, SPIRVOpCapability,
      spv::CapabilitySubgroupShuffleINTEL, SPIRVOpCapability,
      spv::CapabilitySubgroupBufferBlockIOINTEL, SPIRVOpCapability,
      spv::CapabilitySubgroupImageBlockIOINTEL, SPIRVOpCapability,
      spv::CapabilitySubgroupDispatch, SPIRVOpCapability,
      spv::CapabilityFunctionPointersINTEL, SPIRVOpCapability,
      spv::CapabilityIndirectReferencesINTEL, SPIRVOpCapability,
      spv::CapabilityAsmINTEL, SPIRVOpCapability,
      spv::CapabilityVariableLengthArrayINTEL, SPIRVOpCapability,
      spv::internal::CapabilityVectorVariantsINTEL, SPIRVOpCapability,
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
      spv::internal::CapabilityRuntimeAlignedAttributeINTEL,
      SPIRVOpCapability, spv::CapabilityLongConstantCompositeINTEL,
      SPIRVOpCapability, spv::internal::CapabilityBfloat16ConversionINTEL,
      SPIRVOpCapability, spv::internal::CapabilityGlobalVariableDecorationsINTEL,
      SPIRVOpCapability, spv::CapabilityGroupNonUniformBallot,
      SPIRVOpCapability, spv::CapabilityGroupUniformArithmeticKHR,

      // Memory model
      SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL

  };
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  for (int i = 0; i < 2; i++) {
    CLANG_DEV_INFO devInfo = {
        "",    // extensions
        false, // images support
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

// test that a module with FPGA device agnostic capabilities is rejected by FE
// for CPU device
TEST_F(ClangCompilerTestType, Test_RejectCommonSpirvCapabilitiesOnCPU) {
  // Hand made SPIR-V module
  std::uint32_t const spvBC[] = {
      // First 5 mandatory words
      spv::MagicNumber, SPIRV12Version, 0, 0, 0,
      // Common capabilities
      SPIRVOpCapability, spv::CapabilityFPGAMemoryAttributesINTEL,
      SPIRVOpCapability, spv::CapabilityFPGALoopControlsINTEL,
      SPIRVOpCapability, spv::CapabilityFPGARegINTEL, SPIRVOpCapability,
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
      spv::internal::CapabilityFPGADSPControlINTEL, SPIRVOpCapability,
      spv::internal::CapabilityFPGAInvocationPipeliningAttributesINTEL,
      SPIRVOpCapability, spv::internal::CapabilityFPArithmeticFenceINTEL,
      SPIRVOpCapability, spv::internal::CapabilityTaskSequenceINTEL, // INTEL
      // Memory model
      SPIRVOpMemoryModel, spv::AddressingModelPhysical64, spv::MemoryModelOpenCL

  };
  auto spirvDesc = GetTestFESPIRVProgramDescriptor(spvBC);

  CLANG_DEV_INFO devInfo = {
      "",    // extensions
      false, // images support
      true,  // fp64 support
      false, // source level profiling
      false  // CPU
  };
  std::unique_ptr<IOCLFECompiler> spFeCompiler;
  IOCLFECompiler *pFeCompiler = spFeCompiler.get();

  int err =
      CreateFrontEndInstance(&devInfo, sizeof(devInfo), &pFeCompiler);
  ASSERT_EQ(0, err) << "Failed to create FE instance.\n";

  err = pFeCompiler->ParseSPIRV(&spirvDesc, &m_binary_result);
  ASSERT_NE(CL_SUCCESS, err)
      << "Unexpected retcode for an invalid SPIR-V module.\n";
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
