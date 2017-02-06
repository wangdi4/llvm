/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "ChannelPipeTransformation.h"

#include <llvm/ADT/SmallString.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <BuiltinLibInfo.h>
#include <CompilationUtils.h>
#include <InitializePasses.h>
#include <OCLAddressSpace.h>
#include <OCLPassSupport.h>

#include <PipeCommon.h>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace {

struct PipeMetadata {
  PipeMetadata() :
      PacketSize(0), PacketAlign(0) {
  }

  PipeMetadata(int PacketSize, int PacketAlign) :
      PacketSize(PacketSize), PacketAlign(PacketAlign) {
  }

  int PacketSize;
  int PacketAlign;
};

} // anonymous namespace

typedef DenseMap<Value*, Value*> ValueToValueMap;
typedef DenseMap<Value*, PipeMetadata> PipeMetadataMap;

namespace intel {

char ChannelPipeTransformation::ID = 0;
OCL_INITIALIZE_PASS_BEGIN(ChannelPipeTransformation, "channel-pipe-transformation",
                          "Transform Altera channels into OpenCL 2.0 pipes",
                          false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(ChannelPipeTransformation, "channel-pipe-transformation",
                        "Transform Altera channels into OpenCL 2.0 pipes",
                        false, true)


ChannelPipeTransformation::ChannelPipeTransformation() : ModulePass(ID) {
}

static void getPipesMetadata(const Module &M,
                             ValueToValueMap &ChannelToPipeMap,
                             PipeMetadataMap &PipesMD) {
  auto *MDs = M.getNamedMetadata("opencl.channels");
  if (!MDs) {
    llvm_unreachable("'opencl.channels' metadata not found.");
    return;
  }

  for (auto *MD : MDs->operands()) {
    assert(MD->getNumOperands() == 3 &&
           "Invalid number of channel metadata operands");
    auto *ChanMD = dyn_cast<ValueAsMetadata>(MD->getOperand(0).get());
    auto *PacketSizeMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1).get());
    auto *PacketAlignMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(2).get());

    if (!ChanMD || !PacketSizeMD || !PacketAlignMD) {
      llvm_unreachable("Invalid channel metadata.");
      continue;
    }

    Value *Chan = ChanMD->getValue();
    ConstantInt *PacketSize = cast<ConstantInt>(PacketSizeMD->getValue());
    ConstantInt *PacketAlign = cast<ConstantInt>(PacketAlignMD->getValue());

    Value *Pipe = ChannelToPipeMap[Chan];
    PipesMD[Pipe] = PipeMetadata(
        PacketSize->getLimitedValue(),
        PacketAlign->getLimitedValue());
  }
}

static void createPipeBackingStore(Module &M,
                                   const ValueToValueMap &ChannelToPipeMap,
                                   const PipeMetadataMap &PipesMD,
                                   ValueToValueMap &PipeToBSMap) {
  Type *Int8Ty = IntegerType::getInt8Ty(M.getContext());
  for (auto KV : ChannelToPipeMap) {
    auto *PipeOpaquePtr = cast<GlobalVariable>(KV.second);

    // TODO: asavonic: store channel depth in metadata
    auto PipeMD = PipesMD.lookup(PipeOpaquePtr);
    assert(PipeMD.PacketSize && PipeMD.PacketAlign &&
           "Pipe metadata not found.");

    size_t BSSize = pipe_get_total_size(PipeMD.PacketSize, 1);
    auto *ArrayTy = ArrayType::get(Int8Ty, BSSize);

    SmallString<16> NameStr;
    auto BSName =
      (PipeOpaquePtr->getName() + ".bs").toStringRef(NameStr);

    auto *PipeBS = new GlobalVariable(M, ArrayTy, /*isConstant=*/false,
                                      PipeOpaquePtr->getLinkage(),
                                      /*initializer=*/nullptr,
                                      BSName,
                                      /*InsertBefore=*/nullptr,
                                      GlobalValue::NotThreadLocal,
                                      Utils::OCLAddressSpace::Global);

    PipeBS->setInitializer(ConstantAggregateZero::get(ArrayTy));
    PipeBS->setAlignment(PipeMD.PacketAlign);
    PipeToBSMap[PipeOpaquePtr] = PipeBS;
  }
}

static
Function *createPipesCtor(Module &M,
                          const PipeMetadataMap &PipesMD,
                          const ValueToValueMap &PipeToBSMap,
                          const SmallVectorImpl<Module *> &BuiltinModules) {
  Function *PipeInit = nullptr;
  for (auto &BIModule : BuiltinModules) {
    if ((PipeInit = BIModule->getFunction("__pipe_init")))
      break;
  }
  if (!PipeInit) {
    assert(PipeInit && "__pipe_init() not found in RTL.");
    return nullptr;
  } else {
    PipeInit = cast<Function>(
        CompilationUtils::importFunctionDecl(&M, PipeInit));
  }

  auto *CtorTy = FunctionType::get(Type::getVoidTy(M.getContext()),
                                   ArrayRef<Type *>(), false);
  Function *Ctor = cast<Function>(
      M.getOrInsertFunction("__global_pipes_ctor", CtorTy));
  Ctor->setLinkage(GlobalValue::ExternalLinkage);

  BasicBlock* CtorEntry = BasicBlock::Create(M.getContext(), "entry", Ctor);
  IRBuilder<> Builder(CtorEntry);

  for (const auto &PipeBSPair : PipeToBSMap) {
    Value *PipeGlobal = PipeBSPair.first;
    Value *BS = PipeBSPair.second;

    auto PipeMD = PipesMD.lookup(PipeGlobal);
    assert(PipeMD.PacketSize && PipeMD.PacketAlign &&
           "Pipe metadata not found.");

    Value *CallArgs[] = {
      Builder.CreateBitCast(BS, PointerType::get(
                                IntegerType::getInt8Ty(M.getContext()),
                                Utils::OCLAddressSpace::Global)),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), PipeMD.PacketSize),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), PipeMD.PacketAlign)
    };
    Builder.CreateCall(PipeInit, CallArgs);
    Builder.CreateStore(
        Builder.CreateBitCast(BS, cast<PointerType>(
                                  PipeGlobal->getType())->getElementType()),
        PipeGlobal);
  }

  Builder.CreateRetVoid();

  return Ctor;
}

static bool createPipeGlobals(Module &M,
                              ValueToValueMap &ChannelToPipeMap,
                              PipeMetadataMap &PipesMD,
                              SmallVectorImpl<Module *> &BuiltinModules) {
  auto *ChannelTy = M.getTypeByName("opencl.channel_t");
  if (!ChannelTy) {
    return false;
  }
  auto *ChannelPtrPtrTy = PointerType::get(
      PointerType::get(ChannelTy, Utils::OCLAddressSpace::Global),
      Utils::OCLAddressSpace::Global);

  auto *PipeTy = StructType::create(M.getContext(), "opencl.pipe_t");
  auto *PipePtrTy = PointerType::get(PipeTy, Utils::OCLAddressSpace::Global);

  SmallVector<GlobalVariable *, 32> ChannelGlobals;
  for (auto &GV : M.globals()) {
    if (GV.getType() == ChannelPtrPtrTy) {
      ChannelGlobals.push_back(&GV);
    }
  }

  for (auto *GV : ChannelGlobals) {
    SmallString<16> NameStr;
    auto PipeGVName = ("pipe." + GV->getName()).toStringRef(NameStr);

    auto *PipeGV = M.getGlobalVariable(PipeGVName);
    if (!PipeGV) {
      PipeGV = new GlobalVariable(M, PipePtrTy, /*isConstant=*/false,
                                  GV->getLinkage(),
                                  /*Initializer=*/0,
                                  PipeGVName);
      PipeGV->setAlignment(4);
      PipeGV->setInitializer(ConstantPointerNull::get(PipePtrTy));
    }

    ChannelToPipeMap[GV] = PipeGV;
  }

  getPipesMetadata(M, ChannelToPipeMap, PipesMD);

  ValueToValueMap PipeToBSMap;
  createPipeBackingStore(M, ChannelToPipeMap, PipesMD, PipeToBSMap);
  Function *Ctor = createPipesCtor(M, PipesMD, PipeToBSMap, BuiltinModules);
  if (Ctor) {
    appendToGlobalCtors(M, Ctor, /*Priority=*/65535);
  }

  return ChannelGlobals.size() > 0;
}

static bool replaceReadChannel(Function &F, Function &Replacement,
                               Module &M,
                               ValueToValueMap ChannelToPipeMap,
                               PipeMetadataMap PipesMD) {
  bool Changed = false;
  for (auto *U : F.users()) {
    auto *ChannelCall = dyn_cast<CallInst>(U);
    if (!ChannelCall) {
      continue;
    }

    auto *ReadChannelFTy = ChannelCall->getCalledFunction();
    auto ArgIt = ChannelCall->arg_begin();

    Value *ResultPtr = nullptr;
    Type *ResultTy = ReadChannelFTy->getReturnType();
    if (ResultTy->isVoidTy()) {
      // struct type result is passed by pointer as a first argument
      ResultPtr = (ArgIt++)->get();
      ResultTy = ResultPtr->getType();
    }
    Value *ChanArg = (ArgIt++)->get();

    if (!ResultPtr) {
      // primitive type result is returned by value from read_channel
      // make an alloca to pass it by pointer to read_pipe
      ResultPtr = new AllocaInst(ResultTy, "", /*InsertBefore*/ ChannelCall);
    }

    assert(ArgIt == ChannelCall->arg_end() &&
           "Unexpected number of arguments in read_channel_altera.");


    auto *ReadPipe = &Replacement;
    if (Replacement.getParent() != &M) {
      ReadPipe = cast<Function>(
          CompilationUtils::importFunctionDecl(&M, &Replacement));
    }
    auto *ReadPipeFTy = ReadPipe->getFunctionType();

    // discover the global value, from where our channel argument came from
    Value *ChanGlobal = cast<LoadInst>(ChanArg)->getPointerOperand();
    Value *PipeGlobal = ChannelToPipeMap[ChanGlobal];

    auto PipeMD = PipesMD.lookup(PipeGlobal);
    assert(PipeMD.PacketSize && PipeMD.PacketAlign &&
           "Pipe metadata not found.");

    Value *PipeCallArgs[] = {
      new LoadInst(PipeGlobal, "", /*InsertBefore*/ChannelCall),
      CastInst::CreatePointerBitCastOrAddrSpaceCast(
          ResultPtr, ReadPipeFTy->getParamType(1),
          "", /*InsertBefore*/ChannelCall),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), PipeMD.PacketSize),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), PipeMD.PacketAlign)
    };

    CallInst::Create(ReadPipe, PipeCallArgs, "", ChannelCall);


    if (ReadChannelFTy->getReturnType()->isVoidTy()) {
      ChannelCall->eraseFromParent();
    } else {
      BasicBlock::iterator II(ChannelCall);
      ReplaceInstWithValue(ChannelCall->getParent()->getInstList(),
                           II,
                           new LoadInst(ResultPtr, "", ChannelCall));
    }

    Changed = true;
  }
  return Changed;
}

static bool replaceWriteChannel(Function &F, Function &Replacement,
                                Module &M,
                                ValueToValueMap ChannelToPipeMap,
                                PipeMetadataMap PipesMD) {
  bool Changed = false;
  for (auto *U : F.users()) {
    auto *ChannelCall = dyn_cast<CallInst>(U);
    if (!ChannelCall) {
      continue;
    }

    auto *Chan = ChannelCall->getArgOperand(0);
    auto *Val = ChannelCall->getArgOperand(1);

    auto *WritePipe = &Replacement;
    if (Replacement.getParent() != &M) {
      WritePipe = cast<Function>(
          CompilationUtils::importFunctionDecl(&M, &Replacement));
    }
    auto *WritePipeFTy = WritePipe->getFunctionType();

    // discover the global value, from where our channel argument came from
    Value *ChanGlobal = cast<LoadInst>(Chan)->getPointerOperand();
    Value *PipeGlobal = ChannelToPipeMap[ChanGlobal];

    auto PipeMD = PipesMD.lookup(PipeGlobal);
    assert(PipeMD.PacketSize && PipeMD.PacketAlign &&
           "Pipe metadata not found.");


    Value *ValPtr = new AllocaInst(Val->getType(), "",
                                   /*InsertBefore*/ChannelCall);
    new StoreInst(Val, ValPtr, /*InsertBefore*/ChannelCall);

    Value *PipeCallArgs[] = {
      new LoadInst(PipeGlobal, "", /*InsertBefore*/ChannelCall),
      CastInst::CreatePointerBitCastOrAddrSpaceCast(
          ValPtr, WritePipeFTy->getParamType(1),
          "", /*InsertBefore*/ChannelCall),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), PipeMD.PacketSize),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), PipeMD.PacketAlign)
    };

    CallInst::Create(WritePipe, PipeCallArgs, "", ChannelCall);
    ChannelCall->eraseFromParent();

    Changed = true;
  }
  return false;
}

static bool replaceChannelBuiltins(Module &M,
                                   ValueToValueMap ChannelToPipeMap,
                                   PipeMetadataMap PipesMD,
                                   SmallVectorImpl<Module *> &BuiltinModules) {
  Function *ReadPipe = nullptr;
  Function *WritePipe = nullptr;
  for (auto *BIModule : BuiltinModules) {
    if (!ReadPipe) {
      ReadPipe = BIModule->getFunction("__read_pipe_2_bl_intel");
    }

    if (!WritePipe) {
      WritePipe = BIModule->getFunction("__write_pipe_2_bl_intel");
    }
  }

  assert(ReadPipe && "no '__read_pipe_2_bl_intel' built-in declared in RTL");
  assert(WritePipe && "no '__write_pipe_2_bl_intel' built-in declared in RTL");

  bool Changed = false;
  for (auto &F : M) {
    auto Name = F.getName();
    if (Name.npos != Name.find("read_channel_altera")) {
      Changed |= replaceReadChannel(F, *ReadPipe, M,
                                    ChannelToPipeMap, PipesMD);
    } else if (Name.npos !=
               F.getName().find("write_channel_altera")) {
      Changed |= replaceWriteChannel(F, *WritePipe, M,
                                     ChannelToPipeMap, PipesMD);
    }
  }
  return Changed;
}

bool ChannelPipeTransformation::runOnModule(Module &M) {
  bool Changed = false;

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  SmallVector<Module*, 2> BuiltinModules = BLI.getBuiltinModules();

  ValueToValueMap ChannelToPipeMap;
  PipeMetadataMap PipesMD;
  Changed |= createPipeGlobals(M, ChannelToPipeMap, PipesMD, BuiltinModules);
  Changed |= replaceChannelBuiltins(M, ChannelToPipeMap, PipesMD,
                                    BuiltinModules);

  return Changed;
}

void ChannelPipeTransformation::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfo>();
}


} // namespace intel

extern "C"{
  ModulePass* createChannelPipeTransformationPass() {
    return new intel::ChannelPipeTransformation();
  }
}
