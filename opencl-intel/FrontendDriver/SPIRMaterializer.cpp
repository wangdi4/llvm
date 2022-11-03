// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
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

#include "SPIRMaterializer.h"
#include "FrontendResultImpl.h"
#include "frontend_api.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/NameMangleAPI.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"

#include <algorithm>

using namespace llvm;
using namespace llvm::NameMangleAPI;

namespace Intel {
namespace OpenCL {
namespace ClangFE {

static bool checkSPIR12Version(llvm::Module &M) {
  auto NamedMD = M.getNamedMetadata("opencl.spir.version");
  if (!NamedMD || NamedMD->getNumOperands() == 0)
    return false;
  auto VerMD = dyn_cast<MDTuple>(NamedMD->getOperand(0));
  assert(VerMD->getNumOperands() == 2 &&
         "Invalid operand number for opencl.spir.version");
  auto CMajor = mdconst::extract<ConstantInt>(VerMD->getOperand(0));
  auto VerMajor = CMajor->getZExtValue();
  auto CMinor = mdconst::extract<ConstantInt>(VerMD->getOperand(1));
  auto VerMinor = CMinor->getZExtValue();
  if (VerMajor == 1 && VerMinor == 2)
    return true;
  return false;
}

static void updateMetadata(llvm::Module &M) {
  llvm::NamedMDNode *Kernels = M.getNamedMetadata("opencl.kernels");
  if (!Kernels)
    return;
  for (auto *Kernel : Kernels->operands()) {
    auto I = Kernel->op_begin();
    auto Func = llvm::mdconst::dyn_extract<llvm::Function>(*I);
    for (++I; I != Kernel->op_end(); ++I) {
      auto *KernelAttr = cast<MDNode>(*I);
      auto AttrIt = KernelAttr->op_begin();
      MDString *AttrName = cast<MDString>(*AttrIt);
      std::vector<Metadata *> Operands;
      for (++AttrIt; AttrIt != KernelAttr->op_end(); ++AttrIt)
        Operands.push_back(*AttrIt);
      if (AttrName->getString() == "vec_type_hint" ||
          AttrName->getString() == "work_group_size_hint" ||
          AttrName->getString() == "reqd_work_group_size" ||
          AttrName->getString() == "max_work_group_size" ||
          AttrName->getString() == "intel_reqd_sub_group_size" ||
          AttrName->getString() == "required_num_sub_groups" ||
          AttrName->getString() == "kernel_arg_addr_space" ||
          AttrName->getString() == "kernel_arg_access_qual" ||
          AttrName->getString() == "kernel_arg_type" ||
          AttrName->getString() == "kernel_arg_base_type" ||
          AttrName->getString() == "kernel_arg_name" ||
          AttrName->getString() == "kernel_arg_type_qual")
        Func->setMetadata(AttrName->getString(),
                          llvm::MDNode::get(M.getContext(), Operands));
    }
  }

  M.eraseNamedMetadata(Kernels);

  // OpenCL CTS SPIR 1.2 bitcode is compiled with LLVM 3.2 and TBAA metadata
  //   !18 = !{!19, !19, i64 0}
  //   !19 = !{!"int", !20}
  //   !20 = !{!"omnipotent char", !21}
  //   !21 = !{!"Simple C/C++ TBAA"}
  // would be considered as no-alias with current LLVM TBAA metadata
  //   !21 = !{!"Simple C/C++ TBAA"}
  //   !22 = !{!23, !23, i64 0}
  //   !23 = !{!"int", !24, i64 0}
  //   !24 = !{!"omnipotent char", !21, i64 0}
  // although the two metadata have the same semantic.
  // The two different metadata could be mixed after BuiltinImport pass and
  // cause correctness issue after alias-based optimization.
  // According to LLVM "IR Backwards Compatibility", non-debug metadata is
  // defined to be safe to drop, so we drop all TBAA metadata.
  for (auto &F : M)
    for (auto &I : instructions(&F))
      I.setMetadata(LLVMContext::MD_tbaa, nullptr);
}

static std::string updateImageTypeName(StringRef Name, StringRef Acc) {
  std::string AccessQual = Acc.str();
  std::string Res = Name.str();

  assert(Res.find("_t") && "Invalid image type name");
  Res.insert(Res.find("_t") + 1, AccessQual);

  return Res;
}

enum SPIRAddressSpace {
  SPIRAS_Private,
  SPIRAS_Global,
  SPIRAS_Constant,
  SPIRAS_Local,
  SPIRAS_Generic,
  SPIRAS_Count,
};

static PointerType *getOrCreateOpaquePtrType(Module *M, const std::string &Name,
                                             const SPIRAddressSpace AS) {
  auto OpaqueType = StructType::getTypeByName(M->getContext(), Name);
  if (!OpaqueType)
    OpaqueType = StructType::create(M->getContext(), Name);
  return PointerType::get(OpaqueType, AS);
}

#ifndef NDEBUG
static bool isPointerToOpaqueStructType(llvm::Type *Ty) {
  if (auto PT = dyn_cast<PointerType>(Ty))
    if (auto ST = dyn_cast<StructType>(PT->getElementType()))
      if (ST->isOpaque())
        return true;
  return false;
}
#endif // NDEBUG

static reflection::TypePrimitiveEnum getPrimitiveType(Type *T) {
  assert(isPointerToOpaqueStructType(T) && "Invalid type");
  auto Name = T->getPointerElementType()->getStructName();
#define CASE(X, Y)                                                             \
  StartsWith("opencl.image" #X, reflection::PRIMITIVE_IMAGE_##Y)
  return StringSwitch<reflection::TypePrimitiveEnum>(Name)
      .CASE(1d_ro_t, 1D_RO_T)
      .CASE(1d_wo_t, 1D_WO_T)
      .CASE(1d_rw_t, 1D_RW_T)
      .CASE(2d_ro_t, 2D_RO_T)
      .CASE(2d_wo_t, 2D_WO_T)
      .CASE(2d_rw_t, 2D_RW_T)
      .CASE(3d_ro_t, 3D_RO_T)
      .CASE(3d_wo_t, 3D_WO_T)
      .CASE(3d_rw_t, 3D_RW_T)
      .CASE(1d_array_ro_t, 1D_ARRAY_RO_T)
      .CASE(1d_array_wo_t, 1D_ARRAY_WO_T)
      .CASE(1d_array_rw_t, 1D_ARRAY_RW_T)
      .CASE(1d_buffer_ro_t, 1D_BUFFER_RO_T)
      .CASE(1d_buffer_wo_t, 1D_BUFFER_WO_T)
      .CASE(1d_buffer_rw_t, 1D_BUFFER_RW_T)
      .CASE(2d_array_depth_ro_t, 2D_ARRAY_DEPTH_RO_T)
      .CASE(2d_array_depth_wo_t, 2D_ARRAY_DEPTH_WO_T)
      .CASE(2d_array_depth_rw_t, 2D_ARRAY_DEPTH_RW_T)
      .CASE(2d_array_depth_ro_t, 2D_ARRAY_DEPTH_RO_T)
      .CASE(2d_array_depth_wo_t, 2D_ARRAY_DEPTH_WO_T)
      .CASE(2d_array_depth_rw_t, 2D_ARRAY_DEPTH_RW_T)
      .CASE(2d_array_ro_t, 2D_ARRAY_RO_T)
      .CASE(2d_array_wo_t, 2D_ARRAY_WO_T)
      .CASE(2d_array_rw_t, 2D_ARRAY_RW_T)
      .CASE(2d_depth_ro_t, 2D_DEPTH_RO_T)
      .CASE(2d_depth_wo_t, 2D_DEPTH_WO_T)
      .CASE(2d_depth_rw_t, 2D_DEPTH_RW_T)
      .Default(reflection::PRIMITIVE_NONE);
}

static void
changeImageCall(llvm::CallInst *CI,
                llvm::SmallVectorImpl<Instruction *> &InstToRemove) {
  auto *F = CI->getCalledFunction();
  if (!F)
    return;

  StringRef FName = F->getName();
  if (!isMangledName(FName.data()))
    return;

  // Update image type names with image access qualifiers
  if (FName.find("image") == std::string::npos)
    return;

  auto FD = demangle(FName.data());
  auto AccQ = StringSwitch<std::string>(FD.Name)
                  .Case("write_imagef", "wo_")
                  .Case("write_imagei", "wo_")
                  .Case("write_imageui", "wo_")
                  .Default("ro_");
  auto ImgArg = CI->getArgOperand(0);
  auto ImgArgTy = ImgArg->getType();
  assert(isPointerToOpaqueStructType(ImgArgTy) && "Expect image type argument");
  auto STName = ImgArgTy->getPointerElementType()->getStructName();
  assert(STName.startswith("opencl.image") && "Expect image type argument");
  if (STName.find("_ro_t") != std::string::npos ||
      STName.find("_wo_t") != std::string::npos ||
      STName.find("_rw_t") != std::string::npos)
    return;
  std::vector<Value *> Args;
  std::vector<Type *> ArgTys;
  ArgTys.push_back(getOrCreateOpaquePtrType(CI->getParent()->getModule(),
                                            updateImageTypeName(STName, AccQ),
                                            SPIRAS_Global));
  Args.push_back(
      BitCastInst::CreatePointerCast(CI->getArgOperand(0), ArgTys[0], "", CI));
  for (unsigned i = 1; i < CI->arg_size(); ++i) {
    // Cast old sampler type(i32) with new(opaque*) before passing to builtin
    if (auto primitiveType = dyn_cast<reflection::PrimitiveType>(
            (reflection::ParamType *)FD.Parameters[i].get())) {
      if (primitiveType->getPrimitive() == reflection::PRIMITIVE_SAMPLER_T &&
          CI->getArgOperand(i)->getType()->isIntegerTy()) {
        auto SamplerTy = getOrCreateOpaquePtrType(
            CI->getParent()->getModule(), "opencl.sampler_t", SPIRAS_Constant);
        auto IntToPtr =
            new IntToPtrInst(CI->getArgOperand(i), SamplerTy, "", CI);
        Args.push_back(IntToPtr);
        ArgTys.push_back(IntToPtr->getType());
        continue;
      }
    }
    Args.push_back(CI->getArgOperand(i));
    ArgTys.push_back(CI->getArgOperand(i)->getType());
  }
  auto *FT = FunctionType::get(F->getReturnType(), ArgTys, F->isVarArg());
  auto OldImgTy = dyn_cast<reflection::PrimitiveType>(
      (reflection::ParamType *)FD.Parameters[0].get());
  assert(OldImgTy && "Illformed function descriptor");
  OldImgTy->setPrimitive(getPrimitiveType(ArgTys[0]));
  auto NewName = mangle(FD);

  // Check if a new function is already added to the module.
  auto NewF = F->getParent()->getFunction(NewName);
  if (!NewF) {
    // Create function with updated name
    NewF = Function::Create(FT, F->getLinkage(), NewName);
    NewF->copyAttributesFrom(F);
    NewF->copyMetadata(F, 0);

    F->getParent()->getFunctionList().insert(F->getIterator(), NewF);
  }

  CallInst *New = CallInst::Create(NewF, Args, "", CI);
  New->setCallingConv(CI->getCallingConv());
  New->setAttributes(NewF->getAttributes());
  if (CI->isTailCall())
    New->setTailCall();
  New->setDebugLoc(CI->getDebugLoc());

  // Replace old call instruction with updated one
  CI->replaceAllUsesWith(New);
  InstToRemove.push_back(CI);
}

static void changeAddrSpaceCastCall(llvm::CallInst *CI) {
  auto *F = CI->getCalledFunction();
  if (!F)
    return;

  StringRef FName = F->getName();
  if (!isMangledName(FName.data()))
    return;

  // Updates address space qualifier function names with unmangled ones
  if (FName.find("to_global") != std::string::npos ||
      FName.find("to_local") != std::string::npos ||
      FName.find("to_private") != std::string::npos) {
    reflection::FunctionDescriptor FD = demangle(FName.data());
    F->setName("__" + FD.Name);
  }
}

// Basic block functors, to be applied on each block in the module.
// 1. Demangles address space qualifier function names
// 2. Updates image type names with image access qualifiers
//    + adds old sampler type(int32) to new sampler type(opaque*) casts
static void MaterializeBBlock(llvm::BasicBlock &BB) {
  llvm::SmallVector<Instruction *, 4> InstToRemove;

  for (auto &I : BB) {
    if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
      changeImageCall(CI, InstToRemove);
      changeAddrSpaceCastCall(CI);
    }
  }

  // Remove unused instructions
  for (auto &I : InstToRemove) {
    assert(I->use_empty() && "Cannot erase used instructions");
    I->eraseFromParent();
  }
}

// Function functor, to be applied for every function in the module.
// Delegates call to basic-block functors
static void MaterializeFunction(llvm::Function &F) {
  std::for_each(F.begin(), F.end(), MaterializeBBlock);
}

// Function functor, to be applied for every function in the module.
// Translates SPIR 1.2 built-in names to OpenCL CPU RT built-in names if needed
static void RemangleBuiltins(llvm::Function &F, bool isSpir12) {
  if (!F.isDeclaration())
    return;

  StringRef FName = F.getName();
  if (!isMangledName(FName.data()))
    return;

  // Function might have substituted args only if there is 'S' char in the name
  bool IsRemanglingNeeded = FName.find('S') != StringRef::npos;

  if (!IsRemanglingNeeded) {
    // Fixing mangling of CV qualifiers
    for (const auto &Arg : F.args()) {
      if (Arg.getType()->isPointerTy()) {
        IsRemanglingNeeded = true;
        break;
      }
    }
  }

  // Didn't find any reason to remangle
  if (!IsRemanglingNeeded)
    return;

  // Mangler is able to demangle from SPIR1.2 mangling but always
  // mangles to OpenCL CPU RT style
  auto FD = demangle(FName.data(), isSpir12);
  assert(!FD.isNull() && "Cannot demangle function name using SPIR12 rules.");
  auto NewName = mangle(FD);
  assert(NewName != reflection::FunctionDescriptor::nullString() &&
         "Failed to remangle SPIR12 function name.");
  F.setName(NewName);
}

int ClangFECompilerMaterializeSPIRTask::MaterializeSPIR(llvm::Module &M,
                                                        bool isSpir12) {
  updateMetadata(M);

  for (llvm::Module::iterator I = M.begin(), E = M.end(); I != E;) {
    llvm::Function *F = &*I++;
    RemangleBuiltins(*F, isSpir12);
  }
  std::for_each(M.begin(), M.end(), MaterializeFunction);

  return 0;
}

int ClangFECompilerMaterializeSPIRTask::MaterializeSPIR(
    IOCLFEBinaryResult **pBinaryResult) {
  std::unique_ptr<OCLFEBinaryResult> pResult(new OCLFEBinaryResult());

  std::unique_ptr<llvm::LLVMContext> C(new llvm::LLVMContext());
  auto MemBuff = llvm::MemoryBuffer::getMemBuffer(
      llvm::StringRef((const char *)m_pProgDesc->pSPIRContainer,
                      (size_t)m_pProgDesc->uiSPIRContainerSize),
      "", false);
  auto ModuleOrErr = parseBitcodeFile(*MemBuff.get(), *C.get());
  if (!ModuleOrErr) {
    if (pBinaryResult) {
      pResult->setLog("Can't parse SPIR 1.2 module\n");
      *pBinaryResult = pResult.release();
    }
    return CL_INVALID_PROGRAM;
  }
  llvm::Module *pModule = ModuleOrErr.get().get();
  bool isSpir12 = checkSPIR12Version(*pModule);
  int res = MaterializeSPIR(*pModule, isSpir12);

  llvm::raw_svector_ostream ir_ostream(pResult->getIRBufferRef());
  llvm::WriteBitcodeToFile(*pModule, ir_ostream);

  pResult->setIRType(IR_TYPE_COMPILED_OBJECT);
  pResult->setIRName(std::string(pModule->getName()));

  if (pBinaryResult) {
    *pBinaryResult = pResult.release();
  }
  return res;
}

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
