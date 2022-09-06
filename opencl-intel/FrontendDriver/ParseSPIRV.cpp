// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "clang_device_info.h"
#include "common_clang.h" //IOCLFEBinaryResult
#include "FrontendResultImpl.h"
#include "ParseSPIRV.h"
#include "SPIRMaterializer.h"
#include "SPIRVMaterializer.h"

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <LLVMSPIRVLib.h> // llvm::ReadSPIRV
#include <LLVMSPIRVOpts.h> // SPIRV::TranslatorOpts
#include <llvm/Support/SwapByteOrder.h>
#include "SPIRV/libSPIRV/spirv_internal.hpp" // spv::MagicNumber, spv::Version

#include <memory>
#include <string>
#include <sstream>

using namespace std;

// See Table 1. "First Words of Physical Layout" of SPIR-V specification
enum { MagicNumberIdx = 0, SPIRVVersionIdx = 1, FirstOpIdx = 5 };

ClangFECompilerParseSPIRVTask::ClangFECompilerParseSPIRVTask(
    Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *pProgDesc,
    Intel::OpenCL::ClangFE::CLANG_DEV_INFO const &sDeviceInfo)
    : m_pProgDesc(pProgDesc), m_sDeviceInfo(sDeviceInfo) {

  uint32_t const *spirvBC =
      reinterpret_cast<std::uint32_t const *>(pProgDesc->pSPIRVContainer);
  m_littleEndian = spirvBC[MagicNumberIdx] == spv::MagicNumber;
}

std::uint32_t ClangFECompilerParseSPIRVTask::getSPIRVWord(
    std::uint32_t const *wordPtr) const {
  return m_littleEndian ? *wordPtr : llvm::ByteSwap_32(*wordPtr);
}

bool ClangFECompilerParseSPIRVTask::isSPIRV(const void *pBinary,
                                            const size_t BinarySize)
{
  if (!pBinary || BinarySize < sizeof (std::uint32_t))
    return false;
  auto Magic = *static_cast<const std::uint32_t*>(pBinary);
  // Also try with other endianness. See the tip in SPIR-V spec s3.1
  return spv::MagicNumber == Magic ||
         spv::MagicNumber == llvm::ByteSwap_32(Magic);
}

bool ClangFECompilerParseSPIRVTask::readSPIRVHeader(std::string &error) {
  std::uint32_t const *spirvBC =
      reinterpret_cast<std::uint32_t const *>(m_pProgDesc->pSPIRVContainer);
  // Pointer behind the last word in the SPIR-V BC
  std::uint32_t const *const spirvEnd =
      spirvBC + m_pProgDesc->uiSPIRVContainerSize / 4;

  if (m_pProgDesc->uiSPIRVContainerSize == 0) {
    error = "SPIR-V module is empty";
    return false;
  }
  if (spirvEnd <= spirvBC + FirstOpIdx) {
    error = "SPIR-V module has no instructions";
    return false;
  }
  std::stringstream errStr;
  // Check SPIR-V magic number is sane
  if (!isSPIRV(m_pProgDesc->pSPIRVContainer,
               m_pProgDesc->uiSPIRVContainerSize)) {
    errStr << "Invalid magic number: " << std::hex << spirvBC[MagicNumberIdx];
    error = errStr.str();
    return false;
  }

  m_version = getSPIRVWord(spirvBC + SPIRVVersionIdx);

  // Look for OpCapability instructions and check the declared capabilites
  // are supported by CPU device.
  std::uint32_t const *currentWord = spirvBC + FirstOpIdx;
  while (currentWord < spirvEnd) {
    std::uint32_t const word = getSPIRVWord(currentWord);
    // Word Count is the high-order 16 bits of word 0 of the instruction.
    std::uint16_t wordCount = word >> 16;
    // Opcode is the low-order 16 bits of word 0 of the instruction.
    std::uint16_t opCode = word & 0x0000ffff;
    if (spv::OpCapability == opCode) {
      m_capabilities.push_back(getSPIRVWord(currentWord + 1));
    } else if (spv::OpMemoryModel == opCode) {
      m_memoryModel = getSPIRVWord(currentWord + 2);
    } else if (spv::OpSource == opCode) {
      m_sourceLanguage = getSPIRVWord(currentWord + 1);
      // No need to go further
      return true;
    } else if (spv::OpName == opCode || spv::OpMemberName == opCode ||
               spv::OpDecorate == opCode || spv::OpMemberDecorate == opCode ||
               spv::OpFunction == opCode) {
      // Since OpSource is optional instruction, as well as most of other ones,
      // let's check for a bunch of other opcodes in order to stop reading
      // SPIR-V file further
      return true;
    }
    // Go for the next SPIR-V op.
    currentWord = currentWord + wordCount;
  }

  return true;
}

bool ClangFECompilerParseSPIRVTask::isSPIRVSupported(std::string &error) const {
  std::stringstream errStr;
  // Require SPIR-V version 1.1.
  // We do not fully support 1.1, yet want to use some of the features.
  if (m_version > spv::Version) {
    errStr << "Version required by the module (" << m_version
           << ") is higher than supported version (" << spv::Version << ')';
    error = errStr.str();
    return false;
  }

  for (const auto &capability : m_capabilities) {
    switch (capability) {
    default:
      errStr << "SPIRV module requires unsupported capability " << capability;
      error = errStr.str();
      return false;
    case spv::CapabilityImageBasic:
    case spv::CapabilitySampledBuffer:
    case spv::CapabilitySampled1D:
    case spv::CapabilityImageReadWrite:
      if (!m_sDeviceInfo.bImageSupport) {
        error = "SPIRV module requires image capabilities,"
          " but device doesn't support it";
        return false;
      }
      break;

    case spv::CapabilityFloat64:
      if (!m_sDeviceInfo.bDoubleSupport) {
        error = "SPIRV module requires fp64 data type,"
          " but device doesn't support it";
        return false;
      }
      break;

      // The following capabilities are common for all OpenCL 2.1 capable
      // devices
    case spv::CapabilityAddresses:
    case spv::CapabilityLinkage:
    case spv::CapabilityKernel:
    case spv::CapabilityVector16:
    case spv::CapabilityFloat16Buffer:
    case spv::CapabilityInt64:
    case spv::CapabilityPipes:
    case spv::CapabilityPipeStorage:
    case spv::CapabilityGroups:
    case spv::CapabilityDeviceEnqueue:
    case spv::CapabilityLiteralSampler:
    case spv::CapabilityInt16:
    case spv::CapabilityGenericPointer:
    case spv::CapabilityInt8:
    case spv::CapabilitySubgroupShuffleINTEL:
    case spv::CapabilitySubgroupBufferBlockIOINTEL:
    case spv::CapabilitySubgroupImageBlockIOINTEL:
    case spv::CapabilitySubgroupDispatch:
      // Function pointers support
    case spv::CapabilityFunctionPointersINTEL:
    case spv::CapabilityIndirectReferencesINTEL:
    case spv::CapabilityAsmINTEL:
    case spv::CapabilityVariableLengthArrayINTEL:
    case spv::internal::CapabilityVectorVariantsINTEL:
      // SPV_KHR_expect_assume
    case spv::CapabilityExpectAssumeKHR:
    case spv::CapabilityVectorAnyINTEL:
      // Unstructured loop control
    case spv::CapabilityUnstructuredLoopControlsINTEL:
      // Arbitrary Precision Integers
    case spv::CapabilityArbitraryPrecisionIntegersINTEL:
      // SPV_INTEL_fp_fast_math_mode extension
    case spv::CapabilityFPFastMathModeINTEL:
      // SPV_EXT_shader_atomic_float_add extension
    case spv::CapabilityAtomicFloat32AddEXT:
    case spv::CapabilityAtomicFloat16MinMaxEXT:
    case spv::CapabilityAtomicFloat32MinMaxEXT:
    case spv::CapabilityAtomicFloat64MinMaxEXT:
      // SPV_KHR_uniform_group_instructions extension
    case spv::CapabilityGroupUniformArithmeticKHR:
      // optnone attribute support
    case spv::internal::CapabilityOptNoneINTEL:
      // SPV_INTEL_memory_access_aliasing
    case spv::CapabilityMemoryAccessAliasingINTEL:
      // SPV_INTEL_token_type
    case spv::internal::CapabilityTokenTypeINTEL:
    case spv::CapabilityDebugInfoModuleINTEL:
      // SPV_INTEL_matrix
    case spv::internal::CapabilityJointMatrixINTEL:
      // SPV_INTEL_runtime_aligned
    case spv::internal::CapabilityRuntimeAlignedAttributeINTEL:
    case spv::CapabilityLongConstantCompositeINTEL:
      // SPV_INTEL_bf16_conversion
    case spv::internal::CapabilityBfloat16ConversionINTEL:
      // SPV_INTEL_global_variable_decoration
    case spv::internal::CapabilityGlobalVariableDecorationsINTEL:
    case spv::CapabilityGroupNonUniformBallot:
      break;
    case spv::CapabilityInt64Atomics:
    case spv::CapabilityAtomicFloat64AddEXT:
      if (m_sDeviceInfo.bIsFPGAEmu) {
        error = "64bit atomics are not supported on FPGA emulator.";
        return false;
      }
      break;
      // Intel FPGA capabilities
    case spv::CapabilityFPGAMemoryAttributesINTEL:
    case spv::CapabilityFPGALoopControlsINTEL:
    case spv::CapabilityFPGARegINTEL:
    case spv::CapabilityBlockingPipesINTEL:
    case spv::CapabilityKernelAttributesINTEL:
    case spv::CapabilityFPGAKernelAttributesINTEL:
    case spv::CapabilityArbitraryPrecisionFixedPointINTEL:
    case spv::CapabilityArbitraryPrecisionFloatingPointINTEL:
    case spv::CapabilityFPGAMemoryAccessesINTEL:
    case spv::CapabilityIOPipesINTEL:
    case spv::CapabilityUSMStorageClassesINTEL:
    case spv::CapabilityFPGABufferLocationINTEL:
    case spv::CapabilityFPGAClusterAttributesINTEL:
    case spv::CapabilityLoopFuseINTEL:
    case spv::internal::CapabilityFPGADSPControlINTEL:
    case spv::internal::CapabilityFPGAInvocationPipeliningAttributesINTEL:
    case spv::internal::CapabilityFPArithmeticFenceINTEL:
    case spv::internal::CapabilityTaskSequenceINTEL: // INTEL
      if (!m_sDeviceInfo.bIsFPGAEmu) {
        errStr << capability << " is only supported on FPGA emulator";
        error = errStr.str();
        return false;
      }
      break;
    }
  }

  if (spv::MemoryModelOpenCL != m_memoryModel) {
    error = "Memory model declared in SPIRV module is not "
            "OpenCL(the only supported memory model)";
    return false;
  }

  return true;
}

/// \brief: Implements conversion from a SPIR-V binary (incapsulated in
/// FESPIRVProgramDescriptor) to an llvm::Module, converts build options to
/// LLVM metadata according to SPIR specification.
int ClangFECompilerParseSPIRVTask::ParseSPIRV(
    IOCLFEBinaryResult **pBinaryResult) {
  std::unique_ptr<OCLFEBinaryResult> pResult(new OCLFEBinaryResult());

  // verify build options
  unsigned long uiUnrecognizedOptionsSize = strlen(m_pProgDesc->pszOptions) + 1;
  std::unique_ptr<char[]> szUnrecognizedOptions(
      new char[uiUnrecognizedOptionsSize]);
  szUnrecognizedOptions.get()[uiUnrecognizedOptionsSize - 1] = '\0';

  std::stringstream errorMessage;
  if (!::CheckCompileOptions(m_pProgDesc->pszOptions,
                             szUnrecognizedOptions.get(),
                             uiUnrecognizedOptionsSize)) {
    errorMessage << "Unrecognized build options: ";
    errorMessage << szUnrecognizedOptions.get();
    errorMessage << "\n";
    pResult->setLog(errorMessage.str());

    if (pBinaryResult) {
      *pBinaryResult = pResult.release();
    }

    return CL_INVALID_COMPILER_OPTIONS;
  }

  // verify SPIR-V module is supported
  std::string errorMsg;
  if (!readSPIRVHeader(errorMsg)) {
    if (pBinaryResult) {
      errorMessage << "Unsupported SPIR-V module\n" << errorMsg << '\n';
      pResult->setLog(errorMessage.str());
      *pBinaryResult = pResult.release();
    }
    return CL_INVALID_PROGRAM;
  }

  if (!isSPIRVSupported(errorMsg)) {
    if (pBinaryResult) {
      errorMessage << "Unsupported SPIR-V module\n" << errorMsg << '\n';
      pResult->setLog(errorMessage.str());
      *pBinaryResult = pResult.release();
    }
    return CL_INVALID_PROGRAM;
  }

  // parse SPIR-V
  std::unique_ptr<llvm::LLVMContext> context(new llvm::LLVMContext());
  llvm::Module *pModule = nullptr;
  std::stringstream inputStream(
      std::string(static_cast<const char *>(m_pProgDesc->pSPIRVContainer),
                  m_pProgDesc->uiSPIRVContainerSize),
      std::ios_base::in);


  SPIRV::TranslatorOpts opts;
  opts.enableAllExtensions();
  for (size_t i = 0; i < m_pProgDesc->uiSpecConstCount; ++i) {
    opts.setSpecConst(m_pProgDesc->puiSpecConstIds[i],
                      m_pProgDesc->puiSpecConstValues[i]);
  }

  {
    // Try to understand which version of OpenCL need to be specified for
    // built-ins lowering

    // From OpenCL API spec, ver. 2.1 rev. 23, Section 5.8.4.5 Options
    // Controlling the OpenCL C version:
    //
    // If the –cl-std build option is not specified, the highest OpenCL C 1.x
    // language version supported by each device is used when compiling the
    // program for each device. Applications are required to specify the
    // -cl-std=CL2.0 option if they want to compile or build their programs with
    // OpenCL C 2.0.
    SPIRV::BIsRepresentation TargetRepr = SPIRV::BIsRepresentation::OpenCL12;

    std::string Options(m_pProgDesc->pszOptions);
    // The lowering of built-in(s) in OpenCL C 3.0 is compatible with which in
    // OpenCL C 2.0 for our runtime.
    if (Options.find("-cl-std=CL2.0") != std::string::npos ||
        Options.find("-cl-std=CL3.0") != std::string::npos) {
      TargetRepr = SPIRV::BIsRepresentation::OpenCL20;
    }

    // Hack for DPC++/SYCL: SYCL 1.2.1 is based on OpenCL 1.2 and our SYCL RT
    // doesn't pass any "-cl-std" build option to fallback to default OpenCL
    // 1.2.
    // However, SYCL program which uses Intel extensions might use features from
    // 2.0 standard, like sub-groups, non-uniform work-groups, work-group
    // functions, etc. - that means that we need to detect that SPIR-V is coming
    // from DPC++/SYCL and intentionally lower it to OpenCL 2.x
    //
    // This is a tricky part:
    // The only noticeable difference between SYCL flow and OpenCL flow is the
    // spirv.Source metadata: in SYCL the value for spirv.Source is OpenCL C++
    // (because SYCL does not have a dedicated enum value yet), while in OpenCL
    // OpSource is OpenCL C.
    //
    // OpSource is an *optional* instruction and can be omitted during SPIR-V
    // translation. It also is not emitted if we do not use SPIR-V as an
    // intermediate. These two cases are not supported now.
    if (m_sourceLanguage == spv::SourceLanguageOpenCL_CPP) {
      TargetRepr = SPIRV::BIsRepresentation::SPIRVFriendlyIR;
    }

    opts.setDesiredBIsRepresentation(TargetRepr);
  }

  bool success =
      llvm::readSpirv(*context, opts, inputStream, pModule, errorMsg);

  if (success) {
    assert(!verifyModule(*pModule, &llvm::errs()) &&
           "SPIR-V consumer returned a broken module!");
    // Adapts the output of SPIR-V consumer to backend-friendly format.
    success = ClangFECompilerMaterializeSPIRVTask(opts)
                   .MaterializeSPIRV(pModule);
    assert(!verifyModule(*pModule, &llvm::errs()) &&
           "SPIRVMaterializer broke the module!");
    // serialize to LLVM bitcode
    llvm::raw_svector_ostream ir_ostream(pResult->getIRBufferRef());
    llvm::WriteBitcodeToFile(*pModule, ir_ostream);

    pResult->setIRName(std::string(pModule->getName()));
  }

  // setting the result in both successful and unsucceessful cases to pass the
  // error log.
  pResult->setLog(errorMsg);
  pResult->setIRType(IR_TYPE_COMPILED_OBJECT);

  if (pBinaryResult) {
    *pBinaryResult = pResult.release();
  }

  return success ? CL_SUCCESS : CL_INVALID_PROGRAM;
}

// We would like to avoid copying the SPIR-V module from
// FESPIRVProgramDescriptor to the std::istream (or its underlying buffer)
// object. Normally we would do it in the following way:
// std::stringbuf buf;
// buf.pubsetbuf(pSPIRVData, uiSPIRVSize);
// std::istream SPIRVStream(&buf);
// While it works fine on Linux, on Windows std::stringbuf::pubsetbuf() does
// nothing! Admittedly, according to the C++ specification, the effect of this
// method is "Implementation defined".
// To workaround this flaw, we initialize std::istream with a struct derived
// from std::stringbuf. In constructior of this struct we set the buffer
// without copying by calling setg() - a protected method of std::streambuf.
struct strbuf : public std::stringbuf {
  strbuf(char *buffer, std::streamsize size) {
    this->setg(buffer, buffer, buffer + size);
  }
};

void ClangFECompilerParseSPIRVTask::getSpecConstInfo(
     IOCLFESpecConstInfo** pSpecConstInfo) {
  if (!pSpecConstInfo) {
    return;
  }
  auto pResult = std::make_unique<OCLFESpecConstInfo>();

  char *pSPIRVData =
    static_cast<char *>(const_cast<void *>(m_pProgDesc->pSPIRVContainer));
  size_t uiSPIRVSize = m_pProgDesc->uiSPIRVContainerSize;
  strbuf SPIRVbuffer(pSPIRVData, uiSPIRVSize);
  std::istream SPIRVStream(&SPIRVbuffer);

  llvm::getSpecConstInfo(SPIRVStream, pResult->getRef());
  *pSpecConstInfo = pResult.release();
}

