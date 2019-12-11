// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#include "LLDJITBuilder.h"

#include "CPUCompiler.h"
#include "CompilationUtils.h"

#include "lld/Common/Driver.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

llvm::ExecutionEngine *
LLDJITBuilder::CreateExecutionEngine(llvm::Module *M, llvm::TargetMachine *TM) {
  std::string Err;
  std::unique_ptr<llvm::Module> ModuleOwner(M);

  ExecutionEngine *EE = LLDJIT::createJIT(
      std::move(ModuleOwner), &Err, std::unique_ptr<llvm::TargetMachine>(TM));

  if (EE == nullptr) {
    throw Exceptions::CompilerException(
        "Failed to create LLDJIT execution engine");
  }

  EE->generateCodeForModule(M);

  return EE;
}

void LLDJITBuilder::prepareModuleForLLD(llvm::Module *M) {
  convertToMSVCModule(M);
  exportKernelSymbols(M);
  addDllMainFunction(M);
  adjustFunctionAttributes(M);

#if 0
  // Enable this to dump the prepared module to disk for debugging purposes.
  std::error_code EC;
  llvm::raw_fd_ostream OS("module.bc", EC, llvm::sys::fs::F_None);
  WriteBitcodeToFile(*M, OS);
  OS.close();
#endif
}

void LLDJITBuilder::addDllMainFunction(llvm::Module *M) {
  /* Goal is to construct something like this:
   * define i32 @_DllMainCRTStartup(i64 %hModule, i32 %dwReason, i64
   * %lpvReserved) {
   *   entry: ret i32 1
   * }
   */
#ifdef _WIN64
#define POINTER_TYPE Type::getInt64PtrTy(Ctx)
  assert(sizeof(BOOL) == 4 && sizeof(HANDLE) == 8 && sizeof(DWORD) == 4 &&
         sizeof(LPVOID) == 8);
#else
#define POINTER_TYPE Type::getInt32PtrTy(Ctx)
  assert(sizeof(BOOL) == 4 && sizeof(HANDLE) == 4 && sizeof(DWORD) == 4 &&
         sizeof(LPVOID) == 4);
#endif

  LLVMContext &Ctx = M->getContext();
  Type *RetType(Type::getInt32Ty(Ctx));
  Type *HModType(POINTER_TYPE);
  Type *ReasonType(Type::getInt32Ty(Ctx));
  Type *ReservedType(POINTER_TYPE);
  llvm::FunctionCallee FC = M->getOrInsertFunction("_DllMainCRTStartup", RetType, HModType,
                                    ReasonType, ReservedType);
  Function *DllMain = cast<Function>(FC.getCallee());
  DllMain->setCallingConv(CallingConv::X86_StdCall);
  BasicBlock *Block = BasicBlock::Create(M->getContext(), "entry", DllMain);
  llvm::IRBuilder<> Builder(Block);
  Builder.CreateRet(ConstantInt::get(IntegerType::get(Ctx, 32), 1, false));
}
void LLDJITBuilder::convertToMSVCModule(llvm::Module *M) {
#if _WIN64
  M->setDataLayout("e-m:w-p270:32:32-p271:32:32-p272:64:64-"
                   "i64:64-f80:128-n8:16:32:64-S128");
  M->setTargetTriple("x86_64-pc-windows-msvc");
#else
  M->setDataLayout("e-m:x-p:32:32-p270:32:32-p271:32:32-p272:64:64-"
                   "i64:64-f80:32-n8:16:32-a:0:32-S32");
  M->setTargetTriple("i686-pc-windows-msvc");
#endif
  M->addModuleFlag(llvm::Module::Warning, "CodeView", 1);
}
void LLDJITBuilder::adjustFunctionAttributes(llvm::Module *M) {
  for (llvm::Function &F : M->functions()) {
    F.setAttributes(
        F.getAttributes()
            .addAttribute(F.getContext(), AttributeList::FunctionIndex,
                          Attribute::UWTable)
            .addAttribute(F.getContext(), AttributeList::FunctionIndex,
                          Attribute::OptimizeNone)
            .addAttribute(F.getContext(), AttributeList::FunctionIndex,
                          Attribute::NoInline));
  }
}
void LLDJITBuilder::exportKernelSymbols(llvm::Module *M) {
  for (llvm::Function &F : M->functions()) {
    if (F.hasMetadata("kernel_arg_addr_space")) {
      F.setDLLStorageClass(
          llvm::GlobalValue::DLLStorageClassTypes::DLLExportStorageClass);
    }
  }
}
