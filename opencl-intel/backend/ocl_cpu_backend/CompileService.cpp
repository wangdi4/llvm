// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#include "CompileService.h"
#include "BitCodeContainer.h"
#include "CPUProgram.h"
#include "LibraryProgramManager.h"
#include "ObjectCodeContainer.h"
#include "Program.h"
#include "cache_binary_handler.h"
#include "elf_binary.h"
#include "exceptions.h"

#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
using namespace Intel::OpenCL::ELFUtils;

CompileService::CompileService() {}

cl_dev_err_code
CompileService::CreateProgram(const void *pBinary, size_t uiBinarySize,
                              ICLDevBackendProgram_ **ppProgram) {
  assert(m_backendFactory);

  try {
    const char *pBinaryData = (const char *)pBinary;
    size_t uiBinaryDataSize = uiBinarySize;

    if (nullptr == pBinary || uiBinarySize == 0 || nullptr == ppProgram) {
      return CL_DEV_INVALID_VALUE;
    }

    std::unique_ptr<Program> spProgram(m_backendFactory->CreateProgram());

    // check if it is Binary object
    if (OCLElfBinaryReader::IsValidOpenCLBinary((const char *)pBinary,
                                                uiBinarySize)) {
      OCLElfBinaryReader reader((const char *)pBinary, uiBinarySize);
      reader.GetIR(pBinaryData, uiBinaryDataSize);
      spProgram->SetBitCodeContainer(
          new BitCodeContainer(pBinaryData, uiBinaryDataSize, "main"));
      GetProgramBuilder()->ParseProgram(spProgram.get());
    } else if (CacheBinaryReader::IsValidCacheObject((const char *)pBinary,
                                                     uiBinarySize)) {
      spProgram->SetObjectCodeContainer(
          new ObjectCodeContainer(pBinaryData, uiBinaryDataSize));
    }
    // check if it is LLVM IR object
    else if (!memcmp(_CL_LLVM_BITCODE_MASK_, pBinary,
                     sizeof(_CL_LLVM_BITCODE_MASK_) - 1)) {
      spProgram->SetBitCodeContainer(
          new BitCodeContainer(pBinaryData, uiBinaryDataSize, "main"));
      GetProgramBuilder()->ParseProgram(spProgram.get());
    } else {
      throw Exceptions::DeviceBackendExceptionBase("Unknown binary type",
                                                   CL_DEV_INVALID_BINARY);
    }
#ifdef OCL_DEV_BACKEND_PLUGINS
    // Notify the plugin manager
    m_pluginManager.OnCreateProgram(pBinaryData, uiBinaryDataSize,
                                    spProgram.get());
#endif
    *ppProgram = spProgram.release();
    return CL_DEV_SUCCESS;
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    return e.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  }
}

void CompileService::ReleaseProgram(ICLDevBackendProgram_ *pProgram) const {
  std::lock_guard<llvm::sys::Mutex> lock(m_buildLock);
#ifdef OCL_DEV_BACKEND_PLUGINS
  m_pluginManager.OnReleaseProgram(pProgram);
#endif
  if (pProgram != LibraryProgramManager::getInstance()->getProgram())
    delete pProgram;
}

cl_dev_err_code
CompileService::BuildProgram(ICLDevBackendProgram_ *pProgram,
                             const ICLDevBackendOptions *pOptions,
                             const char *pBuildOpts) {
  if (nullptr == pProgram) {
    return CL_DEV_INVALID_VALUE;
  }

  // if an exception is caught, this mutex should be unlocked safely
  // on windows, this mutex will remain unlocked if it is locked in the try{}
  // code block
  std::lock_guard<llvm::sys::Mutex> lock(m_buildLock);

  try {
    return GetProgramBuilder()->BuildProgram(static_cast<Program *>(pProgram),
                                             pOptions, pBuildOpts);
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    return e.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  }
}

cl_dev_err_code CompileService::FinalizeProgram(ICLDevBackendProgram_ *Prog) {
  if (!Prog)
    return CL_DEV_INVALID_VALUE;

  try {
    return GetProgramBuilder()->FinalizeProgram(static_cast<Program *>(Prog));
  } catch (Exceptions::DeviceBackendExceptionBase &E) {
    // FIXME: The commit 2c5a21ce041a1259a9e041fa2a521b269a92b6af delays program
    // global ctors/dll load from clBuildProgram. So for LLDJIT some build
    // errors such as undefined symbol may occur after call clCreateKernel
    // instead of clBuildProgram. Here we save LLDJIT log to program build log.
    Program *ProgInstance = static_cast<Program *>(Prog);
    if (static_cast<CPUProgram *>(ProgInstance)->GetExecutionEngine()) {
      std::string BuildLog = ProgInstance->GetBuildLog();
      ProgInstance->SetBuildLog(BuildLog + '\n' + E.what());
    }
    return E.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  }
}

cl_dev_err_code CompileService::GetLibraryProgram(ICLDevBackendProgram_ **Prog,
                                                  const char **KernelNames) {
  assert(m_backendFactory && "m_backendFactory not initialized");
  if (!Prog || !KernelNames)
    return CL_DEV_INVALID_VALUE;

  auto *LPM = LibraryProgramManager::getInstance();
  *Prog = LPM->getProgram();
  if (!*Prog)
    return CL_DEV_OUT_OF_MEMORY;
  *KernelNames = LPM->getKernelNames();
  return CL_DEV_SUCCESS;
}

cl_dev_err_code CompileService::DumpCodeContainer(
    const ICLDevBackendCodeContainer *pCodeContainer,
    const ICLDevBackendOptions *pOptions) const {
  assert(pCodeContainer);
  assert(pOptions);

  try {
    const BitCodeContainer *pContainer =
        static_cast<const BitCodeContainer *>(pCodeContainer);
    llvm::Module *pModule = pContainer->GetModule();
    assert(pModule);

    std::string fname =
        pOptions->GetStringValue(CL_DEV_BACKEND_OPTION_DUMPFILE, "");

    if (fname.empty()) {
      llvm::outs() << *pModule;
    } else {
      std::error_code ec;
      llvm::raw_fd_ostream ostr(fname.c_str(), ec, llvm::sys::fs::FA_Write);
      if (!ec) {
        pContainer->GetModule()->print(ostr, 0);
      } else {
        throw Exceptions::DeviceBackendExceptionBase(
            std::string("Can't open the dump file ") + fname + ":" +
            ec.message());
      }
    }
    return CL_DEV_SUCCESS;
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    return e.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  }
}

void CompileService::Release() {
  // LLJIT instance in library program must be released before
  // IntelJITEventListener dtor.
  LibraryProgramManager::getInstance()->release();
  delete this;
}

// dump JIT'ed code as object binary or x86 assembly
cl_dev_err_code
CompileService::DumpJITCodeContainer(ICLDevBackendProgram_ * /*program*/,
                                     const ICLDevBackendOptions * /*options*/,
                                     bool /*dumpBinary*/) const {
  assert(false);
  return CL_DEV_NOT_SUPPORTED;
}
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
