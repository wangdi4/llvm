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

#include "cache_binary_handler.h"
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

    llvm::MDNode *pAddressQualifiers =
        Func->getMetadata("kernel_arg_addr_space");
    llvm::MDNode *pAccessQualifiers =
        Func->getMetadata("kernel_arg_access_qual");
    llvm::MDNode *pTypeNames = Func->getMetadata("kernel_arg_type");
    llvm::MDNode *pTypeQualifiers = Func->getMetadata("kernel_arg_type_qual");
    llvm::MDNode *pArgNames = Func->getMetadata("kernel_arg_name");
    llvm::MDNode *pHostAccessible =
        Func->getMetadata("kernel_arg_host_accessible");
    llvm::MDNode *pLocalMemSize = Func->getMetadata("local_mem_size");
    assert(pAddressQualifiers && pAccessQualifiers && pTypeNames &&
           pTypeQualifiers && "invalid kernel metadata");

    std::unique_ptr<OCLFEKernelArgInfo> pResult(new OCLFEKernelArgInfo);
    for (unsigned int i = 0; i < pAddressQualifiers->getNumOperands(); ++i) {
      CachedArgInfo argInfo;

      // Address qualifier
      llvm::ConstantInt *pAddressQualifier =
          llvm::mdconst::dyn_extract<llvm::ConstantInt>(
              pAddressQualifiers->getOperand(i));
      assert(pAddressQualifier &&
             "pAddressQualifier is not a valid ConstantInt*");

      uint64_t uiAddressQualifier = pAddressQualifier->getZExtValue();
      switch (uiAddressQualifier) {
      case 0:
        argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
        break;
      case 1:
        argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_GLOBAL;
        break;
      case 2:
        argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_CONSTANT;
        break;
      case 3:
        argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_LOCAL;
        break;
      }

      // Access qualifier
      llvm::MDString *pAccessQualifier =
          llvm::cast<llvm::MDString>(pAccessQualifiers->getOperand(i));

      argInfo.accessQualifier =
          llvm::StringSwitch<cl_kernel_arg_access_qualifier>(
              pAccessQualifier->getString())
              .Case("read_only", CL_KERNEL_ARG_ACCESS_READ_ONLY)
              .Case("write_only", CL_KERNEL_ARG_ACCESS_WRITE_ONLY)
              .Case("read_write", CL_KERNEL_ARG_ACCESS_READ_ONLY)
              .Default(CL_KERNEL_ARG_ACCESS_NONE);

      // Type qualifier
      llvm::MDString *pTypeQualifier =
          llvm::cast<llvm::MDString>(pTypeQualifiers->getOperand(i));
      argInfo.typeQualifier = 0;
      llvm::StringRef typeQualStr = pTypeQualifier->getString();
      if (typeQualStr.find("const") != llvm::StringRef::npos)
        argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_CONST;
      if (typeQualStr.find("restrict") != llvm::StringRef::npos)
        argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_RESTRICT;
      if (typeQualStr.find("volatile") != llvm::StringRef::npos)
        argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_VOLATILE;
      if (typeQualStr.find("pipe") != llvm::StringRef::npos)
        argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_PIPE;

      // Type name
      llvm::MDString *pTypeName =
          llvm::cast<llvm::MDString>(pTypeNames->getOperand(i));
      argInfo.typeName = pTypeName->getString().str();

      if (pArgNames) {
        // Parameter name
        llvm::MDString *pArgName =
            llvm::cast<llvm::MDString>(pArgNames->getOperand(i));

        argInfo.name = pArgName->getString().str();
      }
      if (pHostAccessible) {
        auto *pHostAccessibleFlag = llvm::cast<llvm::ConstantAsMetadata>(
            pHostAccessible->getOperand(i));

        argInfo.hostAccessible =
            pHostAccessibleFlag &&
            llvm::cast<llvm::ConstantInt>(pHostAccessibleFlag->getValue())
                ->isOne();
      } else {
        argInfo.hostAccessible = false;
      }
      if (pLocalMemSize) {
        auto *pLocalMemSizeFlag = llvm::cast<llvm::ConstantAsMetadata>(
            pLocalMemSize->getOperand(i));

        argInfo.localMemSize =
            llvm::cast<llvm::ConstantInt>(pLocalMemSizeFlag->getValue())
                ->getZExtValue();
      } else {
        argInfo.localMemSize = 0;
      }

      pResult->addInfo(argInfo);
    }

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
