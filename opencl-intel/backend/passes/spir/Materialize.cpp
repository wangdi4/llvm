/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Materialize.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "OCLAddressSpace.h"
#include <NameMangleAPI.h>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"

#include <algorithm>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

// Supported target triples.
const char *PC_LIN64 = "x86_64-pc-linux";          // Used for RH64/SLES64.
const char *PC_LIN32 = "i686-pc-linux";            // Used for Android.
const char *PC_WIN32 = "i686-pc-win32-msvc-elf";   // Win 32 bit.
const char *PC_WIN64 = "x86_64-pc-win32-msvc-elf"; // Win 64 bit.

static bool isPointerToOpaqueStructType(llvm::Type *Ty) {
  if (auto PT = dyn_cast<PointerType>(Ty))
    if (auto ST = dyn_cast<StructType>(PT->getElementType()))
      if (ST->isOpaque())
        return true;
  return false;
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

static PointerType *getOrCreateOpaquePtrType(Module *M,
  const std::string &Name) {
  auto OpaqueType = M->getTypeByName(Name);
  if (!OpaqueType)
    OpaqueType = StructType::create(M->getContext(), Name);
  return PointerType::get(OpaqueType, SPIRAS_Global);
}

static reflection::TypePrimitiveEnum getPrimitiveType(Type *T) {
  assert(isPointerToOpaqueStructType(T) && "Invalid type");
  auto Name = T->getPointerElementType()->getStructName();
#define CASE(X, Y) StartsWith("opencl.image" #X, reflection::PRIMITIVE_IMAGE_##Y)
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

// Basic block functors, to be applied on each block in the module.
// 1. Replaces calling conventions in calling sites.
// 2. Translates SPIR 1.2 built-in names to OpenCL CPU RT built-in names.
class MaterializeBlockFunctor : public BlockFunctor {
public:
  MaterializeBlockFunctor(BuiltinLibInfo &BLI) : BLI(BLI) {}
  void operator()(llvm::BasicBlock &BB) {
    llvm::SmallVector<Instruction *, 4> InstToRemove;

    for (llvm::BasicBlock::iterator b = BB.begin(), e = BB.end(); e != b; ++b) {
      if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&*b)) {
        m_isChanged |= changeCallingConv(CI);
        m_isChanged |= changeImageCall(CI, InstToRemove);
        m_isChanged |= changeAddrSpaceCastCall(CI);
#ifdef BUILD_FPGA_EMULATOR
        m_isChanged |= changePipeCall(CI, InstToRemove);
#endif
      }
    }

    // Remove unused instructions
    for (auto inst : InstToRemove) {
      assert(inst->use_empty() && "Cannot erase used instructions");
      inst->eraseFromParent();
    }
  }

  bool changeCallingConv(llvm::CallInst *CI) {
    if ((llvm::CallingConv::SPIR_FUNC == CI->getCallingConv()) ||
        (llvm::CallingConv::SPIR_KERNEL == CI->getCallingConv())) {
      CI->setCallingConv(llvm::CallingConv::C);
      return true;
    }

    return false;
  }

  bool changeImageCall(llvm::CallInst *CI,
                       llvm::SmallVectorImpl<Instruction *> &InstToRemove) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();
    if (!isMangledName(FName.data()))
      return false;

    // Update image type names with image access qualifiers
    if (FName.find("image") == std::string::npos)
      return false;

    auto FD = demangle(FName.data());
    auto AccQ = StringSwitch<std::string>(FD.name)
      .Case("write_imagef", "wo_")
      .Case("write_imagei", "wo_")
      .Case("write_imageui", "wo_")
      .Default("ro_");
    auto ImgArg = CI->getArgOperand(0);
    auto ImgArgTy = ImgArg->getType();
    assert(isPointerToOpaqueStructType(ImgArgTy) &&
           "Expect image type argument");
    auto STName = ImgArgTy->getPointerElementType()->getStructName();
    assert(STName.startswith("opencl.image") &&
           "Expect image type argument");
    if (STName.find("_ro_t") != std::string::npos ||
        STName.find("_wo_t") != std::string::npos ||
        STName.find("_rw_t") != std::string::npos)
      return false;
    std::vector<Value *> Args;
    std::vector<Type *> ArgTys;
    ArgTys.push_back(
        getOrCreateOpaquePtrType(CI->getParent()->getModule(),
                                 updateImageTypeName(STName, AccQ)));
    Args.push_back(BitCastInst::CreatePointerCast(CI->getArgOperand(0),
                                                  ArgTys[0], "", CI));
    for (unsigned i = 1; i < CI->getNumArgOperands(); ++i) {
      Args.push_back(CI->getArgOperand(i));
      ArgTys.push_back(CI->getArgOperand(i)->getType());
    }
    auto *FT = FunctionType::get(F->getReturnType(), ArgTys, F->isVarArg());
    dyn_cast<reflection::PrimitiveType>(
        (reflection::ParamType *)FD.parameters[0])
      ->setPrimitive(getPrimitiveType(ArgTys[0]));
    auto NewName = mangle(FD);

    // Check if a new function is already added to the module.
    auto NewF = F->getParent()->getFunction(NewName);
    if (!NewF) {
      // Create function with updated name
      NewF = Function::Create(FT, F->getLinkage(), NewName);
      NewF->copyAttributesFrom(F);

      F->getParent()->getFunctionList().insert(F->getIterator(), NewF);
    }

    CallInst *New = CallInst::Create(NewF, Args, "", CI);
    //assert(New->getType() == Call->getType());
    New->setCallingConv(CI->getCallingConv());
    New->setAttributes(NewF->getAttributes());
    if (CI->isTailCall())
      New->setTailCall();
    New->setDebugLoc(CI->getDebugLoc());

    // Replace old call instruction with updated one
    CI->replaceAllUsesWith(New);
    InstToRemove.push_back(CI);

    return true;
  }

  bool changeAddrSpaceCastCall(llvm::CallInst *CI) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();
    if (!isMangledName(FName.data()))
      return false;

    // Updates address space qualifier function names with unmangled ones
    if (FName.find("to_global") != std::string::npos ||
        FName.find("to_local") != std::string::npos ||
        FName.find("to_private") != std::string::npos) {
      reflection::FunctionDescriptor FD = demangle(FName.data());
      F->setName("__" + FD.name);

      return true;
    }

    return false;
  }

  bool changePipeCall(llvm::CallInst *CI,
                      llvm::SmallVectorImpl<Instruction *> &InstToRemove) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();

    bool PipeBI = StringSwitch<bool>(FName)
      .Case("__read_pipe_2", true)
      .Case("__read_pipe_4", true)
      .Case("__write_pipe_2", true)
      .Case("__write_pipe_4", true)
      .Case("__reserve_read_pipe", true)
      .Case("__reserve_write_pipe", true)
      .Default(false);

    if (!PipeBI)
      return false;

    auto m_runtimeModuleList = BLI.getBuiltinModules();

    llvm::StructType *InternalPipeTy = nullptr;
    llvm::Module *PipesModule = nullptr;
    for (auto *M : m_runtimeModuleList) {
      if ((InternalPipeTy = M->getTypeByName("struct.__pipe_t"))) {
        PipesModule = M;
        break;
      }
    }

    if (!InternalPipeTy) {
      llvm_unreachable("Cannot find __pipe_t struct in RTL.");
      return false;
    }

    auto *GlobalPipeTy = PointerType::get(InternalPipeTy,
          Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Global);

    auto PipeArg = BitCastInst::CreatePointerCast(CI->getArgOperand(0),
                                                  GlobalPipeTy, "", CI);
    SmallVector<Value *, 4> NewArgs;
    NewArgs.push_back(PipeArg);

    // skip the first argument (the pipe), and the 2 last arguments (packet size
    // and max packets, they are not used by the BIs)
    assert(CI->getNumArgOperands() > 2 && "Unexpected number of arguments");
    for (size_t i = 1; i < CI->getNumArgOperands() - 2; ++i) {
      NewArgs.push_back(CI->getArgOperand(i));
    }

    llvm::SmallString<256> NewFName(FName);
    NewFName.append("_intel");

    llvm::Function *NewF = cast<Function>(
        CompilationUtils::importFunctionDecl(CI->getParent()->getModule(),
                                             PipesModule->getFunction(
                                                 NewFName)));

    llvm::CallInst *NewCI = llvm::CallInst::Create(NewF, NewArgs, "", CI);

    NewCI->setCallingConv(CI->getCallingConv());
    NewCI->setAttributes(CI->getAttributes());
    if (CI->isTailCall())
      NewCI->setTailCall();
    NewCI->setDebugLoc(CI->getDebugLoc());

    // Replace old call instruction with updated one
    CI->replaceAllUsesWith(NewCI);
    InstToRemove.push_back(CI);

    return true;
  }

private:
  BuiltinLibInfo &BLI;
};

// Function functor, to be applied for every function in the module.
// 1. Delegates call to basic-block functors.
// 2. Replaces calling conventions of function declarations.
class MaterializeFunctionFunctor : public FunctionFunctor {
public:
  MaterializeFunctionFunctor(BuiltinLibInfo &BLI): BLI(BLI) {}

  void operator()(llvm::Function &F) {
    MaterializeBlockFunctor bbMaterializer(BLI);
    llvm::CallingConv::ID CConv = F.getCallingConv();
    if (llvm::CallingConv::SPIR_FUNC == CConv ||
        llvm::CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(llvm::CallingConv::C);
      m_isChanged = true;
    }
    std::for_each(F.begin(), F.end(), bbMaterializer);
    m_isChanged |= bbMaterializer.isChanged();
  }

private:
  BuiltinLibInfo &BLI;
};

static void updateMetadata(llvm::Module &M) {
  llvm::NamedMDNode *Kernels = M.getNamedMetadata("opencl.kernels");
  if (!Kernels) return;
  for (auto *Kernel : Kernels->operands()) {
    auto I = Kernel->op_begin();
    auto Func = llvm::mdconst::dyn_extract<llvm::Function>(*I);
    for (++I; I != Kernel->op_end(); ++I) {
      auto *KernelAttr = cast<MDNode>(*I);
      auto AttrIt = KernelAttr->op_begin();
      MDString *AttrName = cast<MDString>(*AttrIt);
      std::vector<Metadata*> Operands;
      for (++AttrIt; AttrIt != KernelAttr->op_end(); ++AttrIt)
        Operands.push_back(*AttrIt);
      if (AttrName->getString() == "vec_type_hint" ||
          AttrName->getString() == "work_group_size_hint" ||
          AttrName->getString() == "reqd_work_group_size" ||
          AttrName->getString() == "max_work_group_size" ||
          AttrName->getString() == "intel_reqd_sub_group_size" ||
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
}
//
// SpirMaterializer
//

char SpirMaterializer::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(SpirMaterializer, "", "", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(SpirMaterializer, "spir-materializer",
  "Prepares SPIR modules for BE consumption.",
  false, // Not CGF only pass.
  true
)

SpirMaterializer::SpirMaterializer() : ModulePass(ID) {}

bool SpirMaterializer::runOnModule(llvm::Module &Module) {
  bool Ret = false;

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();

  updateMetadata(Module);
  MaterializeFunctionFunctor fMaterializer(BLI);
  // Take care of calling conventions
  std::for_each(Module.begin(), Module.end(), fMaterializer);

  return Ret || fMaterializer.isChanged();
}

}

extern "C" {
llvm::ModulePass *createSpirMaterializer() {
  return new intel::SpirMaterializer();
}

void UpdateTargetTriple(llvm::Module *pModule) {
  std::string triple = pModule->getTargetTriple();

  // Force ELF codegen on Windows (MCJIT does not support COFF format)
  if (((triple.find("win32") != std::string::npos) ||
       (triple.find("windows") != std::string::npos)) &&
      triple.find("-elf") == std::string::npos) {
    pModule->setTargetTriple(triple + "-elf"); // transforms:
                                               // x86_64-pc-win32
                                               // i686-pc-win32
                                               // to:
                                               // x86_64-pc-win32-elf
                                               // i686-pc-win32-elf
  }
}

void materializeSpirDataLayout(llvm::Module &M) {
  llvm::StringRef Triple(M.getTargetTriple());
  if (!Triple.startswith("spir"))
    return;

  // Replace the triple with that of the actual host, in case the triple is
  // SPIR.
  // Statically assigning the triple by the host's identity.
  Triple =
#if defined(_M_X64)
      intel::PC_WIN64;
#elif defined(__LP64__)
      intel::PC_LIN64;
#elif defined(_WIN32)
      intel::PC_WIN32;
#elif defined(__ANDROID__)
      intel::PC_LIN32;
#else
#error "Unsupported host platform"
#endif
  M.setTargetTriple(Triple);

  // Since we codegen ELF only, we can't let MCJIT to guess native platform as
  // it will be COFF on Win. We could've easily hardcoded data layouts here and
  // get away with it ('til next upgrade). To solve this problem once and for
  // all (hopefully) and avoid MCJIT assertion regarding DataLayout mismatch
  // between module's and the one produced via EngineBuilder in CPUCompiler.cpp
  // we'll perform the following trick:
  //
  // Create a dummy module with a correct target triple (-elf) and create
  // EngineBuilder from it. Then we set datalayout of our module to the correct
  // one produced by TargetMachine.

  std::unique_ptr<Module> dummyModule(new Module("empty", M.getContext()));
  dummyModule.get()->setTargetTriple(M.getTargetTriple());
  UpdateTargetTriple(dummyModule.get());
  EngineBuilder builder(std::move(dummyModule));
  auto TM =std::unique_ptr<TargetMachine>(builder.selectTarget());

  // That's how MCJIT does when being created.
  M.setDataLayout(TM->createDataLayout());
}
}
