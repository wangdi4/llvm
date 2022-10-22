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
#include "cache_binary_handler.h"
#include "cl_sys_defines.h"
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
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"

#include <memory>

using namespace Intel::OpenCL::ELFUtils;

static std::vector<cl_kernel_argument_info>
parseKernelArgumentInfos(Function *F) {
  assert(F && "Invalid function");

  std::vector<cl_kernel_argument_info> ArgInfos;

  MDNode *AddressQualifiers = F->getMetadata("kernel_arg_addr_space");
  MDNode *AccessQualifiers = F->getMetadata("kernel_arg_access_qual");
  MDNode *TypeNames = F->getMetadata("kernel_arg_type");
  MDNode *TypeQualifiers = F->getMetadata("kernel_arg_type_qual");
  MDNode *ArgNames = F->getMetadata("kernel_arg_name");
  MDNode *HostAccessible = F->getMetadata("kernel_arg_host_accessible");
  MDNode *LocalMemSize = F->getMetadata("local_mem_size");

  unsigned KernelArgCount = F->arg_size();
  for (unsigned int I = 0; I < KernelArgCount; ++I) {
    Argument *Arg = F->getArg(I);
    cl_kernel_argument_info ArgInfo;
    memset(&ArgInfo, 0, sizeof(ArgInfo));

    // Address qualifier
    unsigned AddrQ = 0;
    if (AddressQualifiers) {
      assert(AddressQualifiers->getNumOperands() == KernelArgCount &&
             "If kernel has 'kernel_arg_addr_space' metadata, its operand "
             "count must match with kernel arg count!");
      ConstantInt *AddressQualifier =
          mdconst::dyn_extract<ConstantInt>(AddressQualifiers->getOperand(I));
      assert(AddressQualifier &&
             "AddressQualifier is not a valid ConstantInt*");
      AddrQ = AddressQualifier->getZExtValue();
    } else {
      // kernel_arg_addr_space might not exist for a SYCL kernel.
      // Decode from the kernel argument itself.
      if (auto *PTy = dyn_cast<PointerType>(Arg->getType()))
        AddrQ = PTy->getAddressSpace();
    }
    switch (AddrQ) {
    case 0:
      ArgInfo.addressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
      break;
    case 1:
      ArgInfo.addressQualifier = CL_KERNEL_ARG_ADDRESS_GLOBAL;
      break;
    case 2:
      ArgInfo.addressQualifier = CL_KERNEL_ARG_ADDRESS_CONSTANT;
      break;
    case 3:
      ArgInfo.addressQualifier = CL_KERNEL_ARG_ADDRESS_LOCAL;
      break;
    default:
      throw std::string("Invalid address qualifier: ") + std::to_string(AddrQ);
      break;
    }

    // Access qualifier
    // kernel_arg_access_qual might not exist for a SYCL kernel, leave it as
    // "none" by default.
    StringRef AccessQ = "none";
    if (AccessQualifiers) {
      assert(AccessQualifiers->getNumOperands() == KernelArgCount &&
             "If kernel has 'kernel_arg_access_qual' metadata, its operand "
             "count must match with kernel arg count!");
      AccessQ = cast<MDString>(AccessQualifiers->getOperand(I))->getString();
    }
    ArgInfo.accessQualifier =
        StringSwitch<cl_kernel_arg_access_qualifier>(AccessQ)
            .Case("read_only", CL_KERNEL_ARG_ACCESS_READ_ONLY)
            .Case("write_only", CL_KERNEL_ARG_ACCESS_WRITE_ONLY)
            .Case("read_write", CL_KERNEL_ARG_ACCESS_READ_WRITE)
            .Default(CL_KERNEL_ARG_ACCESS_NONE);

    // Type qualifier
    // kernel_arg_type_qual might not exist for a SYCL kernel, leave it as ""
    // by default.
    StringRef TypeQ = "";
    if (TypeQualifiers) {
      assert(TypeQualifiers->getNumOperands() == KernelArgCount &&
             "If kernel has 'kernel_arg_type_qual' metadata, its operand "
             "count must match with kernel arg count!");
      TypeQ = cast<MDString>(TypeQualifiers->getOperand(I))->getString();
    }
    ArgInfo.typeQualifier = 0;
    if (TypeQ.contains("const"))
      ArgInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_CONST;
    if (TypeQ.contains("restrict"))
      ArgInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_RESTRICT;
    if (TypeQ.contains("volatile"))
      ArgInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_VOLATILE;
    if (TypeQ.contains("pipe"))
      ArgInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_PIPE;

    // Type name
    std::string TypeName = "";
    if (TypeNames) {
      assert(TypeNames->getNumOperands() == KernelArgCount &&
             "If kernel has 'kernel_arg_type' metadata, its operand count "
             "must match with kernel arg count!");
      TypeName = cast<MDString>(TypeNames->getOperand(I))->getString().str();
    } else {
      // kernel_arg_type might not exist for a SYCL kernel.
      // Decode from the kernel argument itself.
      raw_string_ostream OS(TypeName);
      // FIXME: Type::print function is empty in release build, so TypeName will
      // be empty string. We need to look for solution to get type name although
      // empty string may not lead to stability issue.
      Arg->getType()->print(OS, /*IsForDebug*/ false, /*NoDetails*/ true);
      OS.flush();
    }
    ArgInfo.typeName = STRDUP(TypeName.c_str());

    if (ArgNames) {
      // Parameter name
      MDString *ArgName = cast<MDString>(ArgNames->getOperand(I));
      ArgInfo.name = STRDUP(ArgName->getString().str().c_str());
    }

    if (HostAccessible) {
      auto *HostAccessibleFlag =
          cast<ConstantAsMetadata>(HostAccessible->getOperand(I));

      ArgInfo.hostAccessible =
          HostAccessibleFlag &&
          cast<ConstantInt>(HostAccessibleFlag->getValue())->isOne();
    }

    if (LocalMemSize) {
      auto *LocalMemSizeFlag =
          cast<ConstantAsMetadata>(LocalMemSize->getOperand(I));

      ArgInfo.localMemSize =
          cast<ConstantInt>(LocalMemSizeFlag->getValue())->getZExtValue();
    }

    ArgInfos.push_back(ArgInfo);
  }
  return ArgInfos;
}

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
    std::vector<cl_kernel_argument_info> argInfos =
        parseKernelArgumentInfos(Func);
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
