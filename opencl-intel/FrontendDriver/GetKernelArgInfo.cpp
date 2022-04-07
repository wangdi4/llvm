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

#include "GetKernelArgInfo.h"
#include "CompilationUtils.h"
#include "cache_binary_handler.h"
#include "elf_binary.h"
#include "frontend_api.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"

#include <memory>

using namespace Intel::OpenCL::ELFUtils;

int ClangFECompilerGetKernelArgInfoTask::GetKernelArgInfo(
    const void *pBin, size_t uiBinarySize, const char *szKernelName,
    IOCLFEKernelArgInfo **ppResult) {
  const char *psBin = static_cast<const char*>(pBin);
  const void *pIRBuffer = nullptr;
  size_t uiIRBufferSize = 0;

  if (CacheBinaryReader::IsValidCacheObject(pBin, uiBinarySize)) {
    CacheBinaryReader reader(pBin, uiBinarySize);
    pIRBuffer = reader.GetSectionData(g_irSectionName);
    uiIRBufferSize = reader.GetSectionSize(g_irSectionName);
  } else if (OCLElfBinaryReader::IsValidOpenCLBinary(psBin, uiBinarySize)) {
    OCLElfBinaryReader reader(psBin, uiBinarySize);
    const char *tmpBuf = nullptr;
    reader.GetIR(tmpBuf, uiIRBufferSize);
    pIRBuffer = tmpBuf;
  } else {
    pIRBuffer = pBin;
    uiIRBufferSize = uiBinarySize;
  }
  try {
    llvm::StringRef sBin(static_cast<const char *>(pIRBuffer), uiIRBufferSize);
    auto pBinBuff(llvm::MemoryBuffer::getMemBuffer(sBin, "", false));

    std::unique_ptr<llvm::LLVMContext> context(new llvm::LLVMContext());
    auto ModuleOr =
        parseBitcodeFile(pBinBuff.get()->getMemBufferRef(), *context);
    if (!ModuleOr)
      throw ModuleOr.takeError();

    std::unique_ptr<llvm::Module> pModule = std::move(ModuleOr.get());
    if (!pModule)
      throw std::bad_alloc();

    auto Func = pModule->getFunction(szKernelName);
    if (!Func)
      throw std::string("Can't find ") + szKernelName + " in the module.";
    if (Func->getCallingConv() != llvm::CallingConv::SPIR_KERNEL)
      throw std::string("Function \"") + szKernelName +
          "\" is not an OpenCL kernel.";

    std::unique_ptr<OCLFEKernelArgInfo> pResult(new OCLFEKernelArgInfo);
    using namespace Intel::OpenCL::DeviceBackend;
    std::vector<cl_kernel_argument_info> argInfos =
        CompilationUtils::parseKernelArgumentInfos(Func);
    for (auto &argInfo : argInfos)
      pResult->addInfo(argInfo);

    if (ppResult)
      *ppResult = pResult.release();

    return CL_SUCCESS;
  } catch (std::bad_alloc &) {
    return CL_OUT_OF_HOST_MEMORY;
  } catch (std::string &) {
    return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
  } catch (llvm::Error &err) {
    std::string Message;
    handleAllErrors(std::move(err),
                    [&](llvm::ErrorInfoBase &EIB) { Message = EIB.message(); });
    return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
  }
}
