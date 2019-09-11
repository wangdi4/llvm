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
#if OPENCL_INTREE_BUILD
#include <LLVMSPIRVLib.h> // llvm::ReadSPIRV
#else
#include <LLVMSPIRVLib/LLVMSPIRVLib.h> // llvm::ReadSPIRV
#endif
#include <llvm/Support/SwapByteOrder.h>
#include <spirv/1.1/spirv.hpp> // spv::MagicNumber, spv::Version

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
  return m_littleEndian ? *wordPtr : llvm::sys::SwapByteOrder_32(*wordPtr);
}

bool ClangFECompilerParseSPIRVTask::isSPIRV(const void *pBinary,
                                            const size_t BinarySize)
{
  if (!pBinary || BinarySize < sizeof (std::uint32_t))
    return false;
  auto Magic = *static_cast<const std::uint32_t*>(pBinary);
  // Also try with other endianness. See the tip in SPIR-V spec s3.1
  return spv::MagicNumber == Magic ||
         spv::MagicNumber == llvm::sys::SwapByteOrder_32(Magic);
}

bool ClangFECompilerParseSPIRVTask::isSPIRVSupported(std::string &error) const {
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

  // Require SPIR-V version 1.1.
  // We do not fully support 1.1, yet want to use some of the features.
  std::uint32_t const version = getSPIRVWord(spirvBC + SPIRVVersionIdx);
  if (version > spv::Version) {
    errStr << "Version required by the module (" << version
           << ") is higher than supported version (" << spv::Version << ')';
    error = errStr.str();
    return false;
  }

  // Look for OpCapability instructions and check the declared capabilites
  // are supported by CPU device.
  std::uint32_t const *currentWord = spirvBC + FirstOpIdx;
  while (currentWord < spirvEnd) {
    std::uint32_t const word = getSPIRVWord(currentWord);
    // Word Count is the high-order 16 bits of word 0 of the instruction.
    std::uint16_t wordCount = word >> 16;
    // Opcode is the low-order 16 bits of word 0 of the instruction.
    std::uint16_t opCode = word & 0x0000ffff;
    if (opCode == spv::OpCapability) {
      std::uint32_t const capability = getSPIRVWord(currentWord + 1);
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
      case spv::CapabilityInt64Atomics:
      // Function pointers support
      case spv::CapabilityFunctionPointersINTEL:
      case spv::CapabilityIndirectReferencesINTEL:
      // Intel FPGA capabilities
      case spv::CapabilityFPGAMemoryAttributesINTEL:
      case spv::CapabilityFPGALoopControlsINTEL:
      case spv::CapabilityFPGARegINTEL:
      case spv::CapabilityBlockingPipesINTEL:
      case spv::CapabilityUnstructuredLoopControlsINTEL:
        break;
      }
    // According to logical layout defined by the SPIR-V spec. single
    // OpMemoryModel instruction is requeied.
    } else if (opCode == spv::OpMemoryModel) {
      if (getSPIRVWord(currentWord + 2) != spv::MemoryModelOpenCL) {
        error = "Memory model declared in SPIRV module is not "
                "OpenCL(the only supported memory model)";
        return false;
      }
      return true;
    }
    // Go for the next SPIR-V op.
    currentWord = currentWord + wordCount;
  }

  error = "OpMemoryModel is missing, but it is required";
  return false;
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
  llvm::Module *pModule;
  std::stringstream inputStream(
      std::string(static_cast<const char *>(m_pProgDesc->pSPIRVContainer),
                  m_pProgDesc->uiSPIRVContainerSize),
      std::ios_base::in);

  bool success = llvm::readSpirv(*context, inputStream, pModule, errorMsg);

  assert(!verifyModule(*pModule) &&
         "SPIR-V consumer returned a broken module!");

  if (success) {
    // Adapts the output of SPIR-V consumer to backend-friendly format.
    // It returns 0 on success.
    success = !ClangFECompilerMaterializeSPIRVTask(m_pProgDesc)
                   .MaterializeSPIRV(*pModule);
    assert(!verifyModule(*pModule) && "SPIRVMaterializer broke the module!");
  }

  // setting the result in both sucessful an uncussessful cases
  // to pass the error log.

  // serialize to LLVM bitcode
  llvm::raw_svector_ostream ir_ostream(pResult->getIRBufferRef());
  llvm::WriteBitcodeToFile(*pModule, ir_ostream);

  pResult->setLog(errorMsg);
  pResult->setIRType(IR_TYPE_COMPILED_OBJECT);
  pResult->setIRName(pModule->getName());

  if (pBinaryResult) {
    *pBinaryResult = pResult.release();
  }

  return success ? CL_SUCCESS : CL_INVALID_PROGRAM;
}
