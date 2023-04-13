// INTEL CONFIDENTIAL
//
// Copyright 2011-2022 Intel Corporation.
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

#include "OpenCLProgram.h"
#include "Exception.h"
#include "OCLBuilder.h"
#include "SATestException.h"
#include "cl_types.h"
#include "exceptions.h"
#include "opencl_clang.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Bitstream/BitstreamWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <fstream>
#include <memory.h>
#include <stdio.h>

#define DEBUG_TYPE "OpenCLProgram"
// debug macros
#include "llvm/Support/Debug.h"

using namespace Validation;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::OpenCL::ClangFE;
using std::string;

static std::string buildLibName(const char *s) {
  std::stringstream ret;
#ifdef _WIN32
  ret << s << ".dll";
#else
  ret << "lib" << s << ".so";
#endif
  return ret.str();
}

OpenCLProgram::OpenCLProgram(OpenCLProgramConfiguration *oclProgramConfig,
                             const std::string cpuArch)
    : C(new llvm::LLVMContext) {
  C->setOpaquePointers(false);
  std::string programFile = oclProgramConfig->GetProgramFilePath();
  switch (oclProgramConfig->GetProgramFileType()) {
  case CL: {
    // building the command line options
    //
    // include directives
    OpenCLIncludeDirs *includeDirs(oclProgramConfig->GetIncludeDirs());
    std::stringstream buildOptions;
    if (includeDirs) {
      for (OpenCLIncludeDirs::IncludeDirsList::const_iterator it =
               includeDirs->beginIncldueDirs();
           it != includeDirs->endIncludeDirs(); ++it) {
        buildOptions << " -I \"" << *it << "\"";
      }
    }
    // recorded flags
    buildOptions << " " << oclProgramConfig->GetCompilationFlags();
    // reading the source file to be compiled
    std::fstream indata(programFile.c_str());
    if (!indata.is_open()) // file couldn't be opened
      throw Exception::IOError(programFile.c_str());
    indata.seekg(0, std::ios::end);
    size_t length = indata.tellg();
    indata.seekg(0, std::ios::beg);
    if (length <= 0)
      throw Exception::IOError("empty source file");
    char *source = new char[length + 1];
    indata.read(source, length);
    size_t read = indata.gcount();
    // in Win-based OS, each line results one-character less read to
    // the buffer, so the actual num of bytes read is lower than the
    // actual file length.
    assert(read <= length);
    source[read] = '\0';
    indata.close();
    // building the CL code:
    bool isFPGA = oclProgramConfig->GetDeviceMode() == FPGA_EMU_DEVICE;
    OCLBuilder &builder = OCLBuilder::Instance()
                              .withSource(source)
                              .withBuildOptions(buildOptions.str().c_str())
                              .withFpgaEmulator(isFPGA)
                              .withExtensions(isFPGA)
                              .withOpenCLCFeatures()
                              .createCompiler();
    IOCLFEBinaryResult *result = builder.build();
    delete[] source;
    //
    // Allocating the container
    //
    m_buffer.assign((const char *)result->GetIR(),
                    (const char *)result->GetIR() + result->GetIRSize());
    result->Release();
    // we have an issue here. When uncommenting the line, the unloading
    // of clang causes the teardown of static llvm variables, which
    // in turn causes code that uses llvm to crush. how can we overcome
    // this?
    // builder.close();
    break;
  }
  case LL: {
    llvm::SMDiagnostic err;
    llvm::LLVMContext context;
    context.setOpaquePointers(false);
    std::unique_ptr<llvm::Module> M(
        llvm::parseAssemblyFile(programFile, err, context));
    llvm::SmallVector<char, 8> buffer;
    llvm::raw_svector_ostream outStream(buffer);
    WriteBitcodeToFile(*M.get(), outStream);
    m_buffer.assign(buffer.begin(), buffer.end());
  } break;
  case BC:
    BCOpenCLProgram(programFile);
    break;
  }
}

OpenCLProgram::~OpenCLProgram(void) {}

const char *OpenCLProgram::GetProgramContainer() const {
  return m_buffer.data();
}

unsigned int OpenCLProgram::GetProgramContainerSize() const {
  return m_buffer.size();
}

void OpenCLProgram::BCOpenCLProgram(const string &programFile) {
  std::ifstream testFile(programFile.c_str(), std::ios::binary);
  m_buffer.assign(std::istreambuf_iterator<char>(testFile),
                  std::istreambuf_iterator<char>());
}

llvm::Module *OpenCLProgram::ParseToModule(void) const {
  const char *pIR = this->GetProgramContainer();
  size_t stIRsize = this->GetProgramContainerSize();

  // Input parameters validation
  if (NULL == pIR || 0 == stIRsize) {
    throw Exception::TestReferenceRunnerException(
        "Program container is invalid.");
  }

  //////////////////////////////////////////////////////////////////////
  // Create llvm module from program.

  // Create Memory buffer to store IR data
  llvm::StringRef bitCodeStr(pIR, stIRsize);
  std::unique_ptr<llvm::MemoryBuffer> pMemBuffer =
      llvm::MemoryBuffer::getMemBuffer(bitCodeStr, "", false);
  if (nullptr == pMemBuffer) {
    throw Exception::TestReferenceRunnerException(
        "Can't create LLVM memory buffer from program bytecode.");
  }

  auto pModuleOrErr = expectedToErrorOrAndEmitErrors(
      *C, parseBitcodeFile(pMemBuffer->getMemBufferRef(), *C));
  if (!pModuleOrErr) {
    throw Exception::TestReferenceRunnerException(
        "Unable to parse bytecode into LLVM module");
  }
  LLVM_DEBUG(llvm::dbgs() << "Module LLVM error: "
                          << pModuleOrErr.getError().value() << "\n"
                          << "            message: "
                          << pModuleOrErr.getError().message() << "\n");
  return pModuleOrErr.get().release();
}
