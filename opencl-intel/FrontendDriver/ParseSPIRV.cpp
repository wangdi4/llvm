//==---- ParseSPIRV.cpp --- OpenCL front-end compiler -------------*- C++ -*---=
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-----------------------------------------------------------------------===

#include "clang_device_info.h"
#include "common_clang.h" //IOCLFEBinaryResult
#include "FrontendResultImpl.h"
#include "ParseSPIRV.h"
#include "SPIRMaterializer.h"

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SPIRV.h> // llvm::ReadSPIRV
#include <llvm/Support/SwapByteOrder.h>
#include <spirv/1.0/spirv.hpp> // spv::MagicNumber

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

bool ClangFECompilerParseSPIRVTask::isSPIRVSupported() const {
  std::uint32_t const *spirvBC =
      reinterpret_cast<std::uint32_t const *>(m_pProgDesc->pSPIRVContainer);
  // Pointer behind the last work in SPIR-V BC
  std::uint32_t const *const spirvEnd =
      spirvBC + m_pProgDesc->uiSPIRVContainerSize / 4;

  if (spirvEnd <= spirvBC + FirstOpIdx)
    return false;
  // Check SPIR-V magic number is sane
  std::uint32_t const magicNumber = spirvBC[MagicNumberIdx];
  if (!m_littleEndian &&
      magicNumber != llvm::sys::SwapByteOrder_32(magicNumber))
    return false;

  // SPIR-V version is 1.0
  std::uint32_t const version = getSPIRVWord(spirvBC + SPIRVVersionIdx);
  if (version != spv::Version)
    return false;

  // Look for OpCapability instructions and check the declared capabilites
  // are supported by CPU device.
  std::uint32_t const *currentWord = spirvBC + FirstOpIdx;
  while (currentWord < spirvEnd) {
    std::uint32_t const word = getSPIRVWord(currentWord);
    std::uint16_t wordCount = word >> 16;
    std::uint16_t opCode = word;
    // According to logical layout defined by SPIR-V spec. OpMemoryModel is
    // requeied
    if (opCode == spv::OpCapability) {
      std::uint32_t const capability = getSPIRVWord(currentWord + 1);
      switch (capability) {
      default:
        return false;
      case spv::CapabilityImageBasic:
      case spv::CapabilitySampledBuffer:
      case spv::CapabilitySampled1D:
      case spv::CapabilityImageReadWrite:
        if (!m_sDeviceInfo.bImageSupport)
          return false;
        break;

      case spv::CapabilityFloat64:
        if (!m_sDeviceInfo.bDoubleSupport)
          return false;
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
      case spv::CapabilityGroups:
      case spv::CapabilityDeviceEnqueue:
      case spv::CapabilityLiteralSampler:
      case spv::CapabilityInt16:
      case spv::CapabilityGenericPointer:
      case spv::CapabilityInt8:
        break;
      }
      // OpMemoryModel is a mandatory entry in a valid SPIR-V module
    } else if (opCode == spv::OpMemoryModel) {
      if (getSPIRVWord(currentWord + 2) != spv::MemoryModelOpenCL)
        return false;
      return true;
    }
    // Go for the next SPIR-V op.
    currentWord = currentWord + wordCount;
  }

  return false;
}

//
// ClangFECompilerParseSPIRVTask call implementation.
// Description:
// Implements conversion from a SPIR-V 1.0 program (incapsulated in
// ClangFECompilerParseSPIRVTask)
// to a llvm::Module, converts build options to LLVM metadata according to SPIR
// specification.
int ClangFECompilerParseSPIRVTask::ParseSPIRV(
    IOCLFEBinaryResult **pBinaryResult) {
  std::unique_ptr<OCLFEBinaryResult> pResult(new OCLFEBinaryResult());

  // verify build options
  unsigned int uiUnrecognizedOptionsSize = strlen(m_pProgDesc->pszOptions) + 1;
  std::unique_ptr<char> szUnrecognizedOptions(
      new char[uiUnrecognizedOptionsSize]);
  szUnrecognizedOptions.get()[uiUnrecognizedOptionsSize - 1] = '\0';

  if (!::CheckCompileOptions(m_pProgDesc->pszOptions,
                             szUnrecognizedOptions.get(),
                             uiUnrecognizedOptionsSize)) {
    std::stringstream errorMessage;
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
  if (!isSPIRVSupported()) {
    if (pBinaryResult) {
      pResult->setLog("Usupported SPIR-V module\n");
      *pBinaryResult = pResult.release();
    }
    return CL_INVALID_PROGRAM;
  }

  // parse SPIR-V
  std::unique_ptr<llvm::LLVMContext> context(new llvm::LLVMContext());
  llvm::Module *pModule;
  std::string errorMsg;
  std::stringstream inputStream(
      std::string((const char *)m_pProgDesc->pSPIRVContainer,
                  m_pProgDesc->uiSPIRVContainerSize),
      std::ios_base::in);

  bool success = llvm::ReadSPIRV(*context, inputStream, pModule, errorMsg);

  // Respect build options.
  // Compiler options layout in llvm metadata is defined by SPIR spec.
  // For example:
  // !opencl.compiler.options = !{!11}
  // !11 = !{!"-cl-fast-relaxed-math", !""-cl-mad-enable"}
  if (success) {
    llvm::NamedMDNode *OCLCompOptsMD =
        pModule->getOrInsertNamedMetadata("opencl.compiler.options");
    // we do not expect spir-v parser to handle build options
    assert(OCLCompOptsMD->getNumOperands() == 0 &&
           "SPIR-V parser is not expected to handle compile options");

    if (OCLCompOptsMD->getNumOperands() == 0) {
      llvm::SmallVector<llvm::Metadata *, 5> OCLBuildOptions;

      std::vector<std::string> buildOptionsSeparated;
      std::stringstream optionsStrstream(m_pProgDesc->pszOptions);
      std::copy(std::istream_iterator<std::string>(optionsStrstream),
                std::istream_iterator<std::string>(),
                std::back_inserter(buildOptionsSeparated));

      for (auto option : buildOptionsSeparated) {
        OCLBuildOptions.push_back(llvm::MDString::get(*context, option));
      }

      OCLCompOptsMD->addOperand(llvm::MDNode::get(*context, OCLBuildOptions));
    }
  }

  assert(!verifyModule(*pModule) &&
         "SPIR-V consumer returned a broken module!");

  if (success) {
    // Currently SPIR-V consumer returns SPIR-like LLVM IR, so we need to
    // convert it to the current LLVM IR version style.
    // MaterializeSPIR returns 0 on success.
    success = !ClangFECompilerMaterializeSPIRTask::MaterializeSPIR(*pModule);

    assert(!verifyModule(*pModule) && "SPIR Materializer broke the module!");
  }

  // setting the result in both sucessful an uncussessful cases
  // to pass the error log.

  // serialize to LLVM bitcode
  llvm::raw_svector_ostream ir_ostream(pResult->getIRBufferRef());
  llvm::WriteBitcodeToFile(pModule, ir_ostream);

  pResult->setLog(errorMsg);
  pResult->setIRType(IR_TYPE_COMPILED_OBJECT);
  pResult->setIRName(pModule->getName());

  if (pBinaryResult) {
    *pBinaryResult = pResult.release();
  }

  return success ? CL_SUCCESS : CL_INVALID_PROGRAM;
}
