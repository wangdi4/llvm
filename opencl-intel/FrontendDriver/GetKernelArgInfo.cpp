//==---- GetKernelArgInfo.cpp --- OpenCL front-end compiler -------*- C++ -*---=
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-----------------------------------------------------------------------===

#include "cache_binary_handler.h"
#include "common_clang.h"
#include "elf_binary.h"
#include "frontend_api.h"
#include "GetKernelArgInfo.h"

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
    char *tmpBuf = nullptr;
    reader.GetIR(tmpBuf, uiIRBufferSize);
    pIRBuffer = tmpBuf;
  } else {
    pIRBuffer = pBin;
    uiIRBufferSize = uiBinarySize;
  }
  return ::GetKernelArgInfo((const void *)pIRBuffer, uiIRBufferSize,
                            szKernelName, ppResult);

}
