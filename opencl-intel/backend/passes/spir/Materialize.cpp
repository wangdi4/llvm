/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Materialize.h"
#include "CompilationUtils.h"
#include "MetaDataApi.h"
#include "OCLPassSupport.h"
#include <NameMangleAPI.h>

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
  Res.insert(Res.end() - 1, AccessQual.begin(), AccessQual.end());
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
#define CASE(X, Y) Case("opencl.image" #X, reflection::PRIMITIVE_IMAGE_##Y)
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
class MaterializeBlockFunctor : public BlockFunctor {
public:
  void operator()(llvm::BasicBlock &BB) {
    auto M = BB.getModule();
    for (llvm::BasicBlock::iterator b = BB.begin(), e = BB.end(); e != b; ++b) {
      if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&*b)) {
        if ((llvm::CallingConv::SPIR_FUNC == CI->getCallingConv()) ||
            (llvm::CallingConv::SPIR_KERNEL == CI->getCallingConv())) {
          CI->setCallingConv(llvm::CallingConv::C);
          m_isChanged = true;
        }
        auto *F = CI->getCalledFunction();
        StringRef FName = F->getName();
        if (!isMangledName(FName.data()) || FName.find("image") == std::string::npos)
          continue;
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
          continue;
        std::vector<Value *> Args;
        std::vector<Type *> ArgTys;
        ArgTys.push_back(
            getOrCreateOpaquePtrType(M, updateImageTypeName(STName, AccQ)));
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

        // Create function with updated name
        auto NewF = Function::Create(FT, F->getLinkage(), NewName);
        NewF->copyAttributesFrom(F);

        F->getParent()->getFunctionList().insert(F->getIterator(), NewF);

        CallInst *New = CallInst::Create(NewF, Args, "", CI);
        //assert(New->getType() == Call->getType());
        New->setCallingConv(CI->getCallingConv());
        New->setAttributes(NewF->getAttributes());
        if (CI->isTailCall())
          New->setTailCall();
        New->setDebugLoc(CI->getDebugLoc());
        CI->replaceAllUsesWith(New);

        m_isChanged = true;
      }
    }
  }
};

// Function functor, to be applied for every function in the module.
// 1. Delegates call to basic-block functors.
// 2. Replaces calling conventions of function declarations.
class MaterializeFunctionFunctor : public FunctionFunctor {
public:
  void operator()(llvm::Function &F) {
    MaterializeBlockFunctor bbMaterializer;
    llvm::CallingConv::ID CConv = F.getCallingConv();
    if (llvm::CallingConv::SPIR_FUNC == CConv ||
        llvm::CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(llvm::CallingConv::C);
      m_isChanged = true;
    }
    std::for_each(F.begin(), F.end(), bbMaterializer);
    m_isChanged |= bbMaterializer.isChanged();
  }
};

//
// SpirMaterializer
//

char SpirMaterializer::ID = 0;

SpirMaterializer::SpirMaterializer() : ModulePass(ID) {}

const char *SpirMaterializer::getPassName() const {
  return "spir materializer";
}

bool SpirMaterializer::runOnModule(llvm::Module &Module) {
  bool Ret = false;

  MaterializeFunctionFunctor fMaterializer;
  // Take care of calling conventions
  std::for_each(Module.begin(), Module.end(), fMaterializer);

  return Ret || fMaterializer.isChanged();
}

OCL_INITIALIZE_PASS_BEGIN(SpirMaterializer, "", "", false, false)
OCL_INITIALIZE_PASS_END(SpirMaterializer, "spir-materializer",
  "Prepares SPIR modules for BE consumption.",
  false, // Not CGF only pass.
  false  // Not an analysis pass.
)

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
  TargetMachine *TM = builder.selectTarget();

  // That's how MCJIT does when being created.
  M.setDataLayout(TM->createDataLayout());
}
}
