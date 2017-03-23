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

#include <utility>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace {

struct PipeMetadata {
  PipeMetadata() :
      PacketSize(0), PacketAlign(0), Depth(1) {
  }

  PipeMetadata(int PacketSize, int PacketAlign) :
      PacketSize(PacketSize), PacketAlign(PacketAlign), Depth(1) {
  }

  PipeMetadata(int PacketSize, int PacketAlign, int Depth) :
      PacketSize(PacketSize), PacketAlign(PacketAlign), Depth(Depth) {
  }

  int PacketSize;
  int PacketAlign;
  int Depth;
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
    assert(MD->getNumOperands() >= 3 &&
           "Channel metedata must contain at least 3 operands");
    auto *ChanMD = dyn_cast<ValueAsMetadata>(MD->getOperand(0).get());
    ConstantAsMetadata *PacketSizeMD = nullptr;
    ConstantAsMetadata *PacketAlignMD = nullptr;
    ConstantAsMetadata *DepthMD = nullptr;

    for (unsigned i = 1; i < MD->getNumOperands(); ++i) {
      MDNode *MDN = dyn_cast<MDNode>(MD->getOperand(i).get());

      auto *Key = dyn_cast<MDString>(MDN->getOperand(0).get());
      if (Key->getString() == "packet_size") {
        PacketSizeMD = dyn_cast<ConstantAsMetadata>(MDN->getOperand(1).get());
      } else if (Key->getString() == "packet_align") {
        PacketAlignMD = dyn_cast<ConstantAsMetadata>(MDN->getOperand(1).get());
      } else if (Key->getString() == "depth") {
        DepthMD = dyn_cast<ConstantAsMetadata>(MDN->getOperand(1).get());
      } else {
        llvm_unreachable("Unknown metadata operand key");
        continue;
      }
    }

    if (!ChanMD || !PacketSizeMD || !PacketAlignMD) {
      llvm_unreachable("Invalid channel metadata.");
      continue;
    }

    Value *Chan = ChanMD->getValue();
    ConstantInt *PacketSize = cast<ConstantInt>(PacketSizeMD->getValue());
    ConstantInt *PacketAlign = cast<ConstantInt>(PacketAlignMD->getValue());

    Value *Pipe = ChannelToPipeMap[Chan];
    if (!DepthMD) {
      PipesMD[Pipe] = PipeMetadata(
          PacketSize->getLimitedValue(),
          PacketAlign->getLimitedValue());
    } else {
      ConstantInt *Depth = cast<ConstantInt>(DepthMD->getValue());
      auto DepthValue = Depth->getLimitedValue();
      if (DepthValue == 0)
        DepthValue = 1;

      PipesMD[Pipe] = PipeMetadata(
          PacketSize->getLimitedValue(),
          PacketAlign->getLimitedValue(),
          DepthValue);
    }
  }
}

static void createPipeBackingStore(Module &M,
                                   const ValueToValueMap &ChannelToPipeMap,
                                   const PipeMetadataMap &PipesMD,
                                   ValueToValueMap &PipeToBSMap) {
  Type *Int8Ty = IntegerType::getInt8Ty(M.getContext());
  for (auto KV : ChannelToPipeMap) {
    auto *PipeOpaquePtr = cast<GlobalVariable>(KV.second);

    auto PipeMD = PipesMD.lookup(PipeOpaquePtr);
    assert(PipeMD.PacketSize && PipeMD.PacketAlign && PipeMD.Depth &&
           "Pipe metadata not found.");

    size_t BSSize = pipe_get_total_size(PipeMD.PacketSize, PipeMD.Depth);
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
    if ((PipeInit = BIModule->getFunction("__pipe_init_intel")))
      break;
  }
  if (!PipeInit) {
    assert(PipeInit && "__pipe_init_intel() not found in RTL.");
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
    assert(PipeMD.PacketSize && PipeMD.PacketAlign && PipeMD.Depth &&
           "Pipe metadata not found.");

    Value *CallArgs[] = {
      Builder.CreateBitCast(BS, PipeInit->getFunctionType()->getParamType(0)),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), PipeMD.PacketSize),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), PipeMD.Depth)
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

  PipeMetadataMap PipesMD;
  getPipesMetadata(M, ChannelToPipeMap, PipesMD);

  ValueToValueMap PipeToBSMap;
  createPipeBackingStore(M, ChannelToPipeMap, PipesMD, PipeToBSMap);
  Function *Ctor = createPipesCtor(M, PipesMD, PipeToBSMap, BuiltinModules);
  if (Ctor) {
    appendToGlobalCtors(M, Ctor, /*Priority=*/65535);
  }

  return ChannelGlobals.size() > 0;
}

static void insertReadPipe(Function *ReadPipe,
                            Value *Pipe, Value *DstPtr,
                            Instruction *BeforeInst) {
  IRBuilder<> Builder(BeforeInst);
  auto *ReadPipeFTy = ReadPipe->getFunctionType();

  Value *PipeCallArgs[] = {
    Builder.CreateBitCast(
        Builder.CreateLoad(Pipe), ReadPipeFTy->getParamType(0)),
    Builder.CreatePointerBitCastOrAddrSpaceCast(
        DstPtr, ReadPipeFTy->getParamType(1))
  };

  Builder.CreateCall(ReadPipe, PipeCallArgs);
}


static bool replaceReadChannel(Function &F, Function &ReadPipe,
                               Module &M,
                               ValueToValueMap ChannelToPipeMap) {
  bool Changed = false;

  SmallVector<User *, 32> ReadChannelUsers(F.user_begin(), F.user_end());
  DenseMap<std::pair<Function *, Type *>, Value *> AllocaMap;
  for (auto *U : ReadChannelUsers) {
    auto *ChannelCall = dyn_cast<CallInst>(U);
    if (!ChannelCall) {
      continue;
    }

    auto *ReadChannelFTy = ChannelCall->getCalledFunction();
    auto ArgIt = ChannelCall->arg_begin();

    Value *DstPtr = nullptr;
    Type *DstTy = ReadChannelFTy->getReturnType();
    if (DstTy->isVoidTy()) {
      // struct type result is passed by pointer as a first argument
      // the read_channel function returns void in this case
      DstPtr = (ArgIt++)->get();
      DstTy = DstPtr->getType();
    }
    Value *ChanArg = (ArgIt++)->get();

    Function *TargetFn = ChannelCall->getParent()->getParent();
    if (!DstPtr) {
      // primitive type result is returned by value from read_channel
      // make an alloca to pass it by pointer to read_pipe
      Value *&Alloca = AllocaMap[std::make_pair(TargetFn, DstTy)];
      if (!Alloca) {
        Instruction *InsertBefore =
          &*(TargetFn->getEntryBlock().getFirstInsertionPt());
        Alloca = new AllocaInst(DstTy, "read.dst", InsertBefore);
      }

      DstPtr = Alloca;
    }

    assert(ArgIt == ChannelCall->arg_end() &&
           "Unexpected number of arguments in read_channel_altera.");

    // discover the global value, from where our channel argument came from
    Value *ChanGlobal = cast<LoadInst>(ChanArg)->getPointerOperand();
    Value *PipeGlobal = ChannelToPipeMap[ChanGlobal];

    insertReadPipe(&ReadPipe, PipeGlobal, DstPtr, ChannelCall);

    if (ReadChannelFTy->getReturnType()->isVoidTy()) {
      ChannelCall->eraseFromParent();
    } else {
      BasicBlock::iterator II(ChannelCall);
      ReplaceInstWithValue(ChannelCall->getParent()->getInstList(),
                           II,
                           new LoadInst(DstTy, DstPtr, "",
                                        /*isVolatile=*/false, ChannelCall));
    }

    Changed = true;
  }
  return Changed;
}

static void insertWritePipe(Function *WritePipe,
                            Value *Pipe, Value *SrcPtr,
                            Instruction *BeforeInst) {
  IRBuilder<> Builder(BeforeInst);
  auto *WritePipeFTy = WritePipe->getFunctionType();

  Value *PipeCallArgs[] = {
    Builder.CreateBitCast(
        Builder.CreateLoad(Pipe), WritePipeFTy->getParamType(0)),
    Builder.CreatePointerBitCastOrAddrSpaceCast(
        SrcPtr, WritePipeFTy->getParamType(1))
  };

  Builder.CreateCall(WritePipe, PipeCallArgs);
}

static bool replaceWriteChannel(Function &F, Function &WritePipe,
                                Module &M,
                                ValueToValueMap ChannelToPipeMap) {
  bool Changed = false;

  SmallVector<User *, 32> WriteChannelUsers(F.user_begin(), F.user_end());
  DenseMap<std::pair<Function *, Type *>, Value *> AllocaMap;

  for (auto *U : WriteChannelUsers) {
    auto *ChannelCall = dyn_cast<CallInst>(U);
    if (!ChannelCall) {
      continue;
    }

    auto *Chan = ChannelCall->getArgOperand(0);
    auto *Val = ChannelCall->getArgOperand(1);

    // discover the global value, from where our channel argument came from
    Value *ChanGlobal = cast<LoadInst>(Chan)->getPointerOperand();
    Value *PipeGlobal = ChannelToPipeMap[ChanGlobal];

    Function *TargetFn = ChannelCall->getParent()->getParent();
    Type *SrcType = Val->getType();
    Value *&SrcPtr = AllocaMap[std::make_pair(TargetFn, SrcType)];
    if (!SrcPtr) {
      Instruction *InsertBefore =
        &*(TargetFn->getEntryBlock().getFirstInsertionPt());
      SrcPtr = new AllocaInst(SrcType, "write.src", InsertBefore);
    }
    new StoreInst(Val, SrcPtr, ChannelCall);

    insertWritePipe(&WritePipe, PipeGlobal, SrcPtr, ChannelCall);

    ChannelCall->eraseFromParent();

    Changed = true;
  }
  return false;
}

static bool replaceChannelBuiltins(Module &M,
                                   ValueToValueMap ChannelToPipeMap,
                                   SmallVectorImpl<Module *> &BuiltinModules) {
  Function *ReadPipe = nullptr;
  Function *WritePipe = nullptr;
  for (auto *BIModule : BuiltinModules) {
    if (!ReadPipe) {
      ReadPipe = cast<Function>(
        CompilationUtils::importFunctionDecl(
            &M, BIModule->getFunction("__read_pipe_2_bl_intel")));
    }

    if (!WritePipe) {
      WritePipe = cast<Function>(
        CompilationUtils::importFunctionDecl(
            &M, BIModule->getFunction("__write_pipe_2_bl_intel")));
    }
  }

  assert(ReadPipe && "no '__read_pipe_2_bl_intel' built-in declared in RTL");
  assert(WritePipe && "no '__write_pipe_2_bl_intel' built-in declared in RTL");

  bool Changed = false;
  for (auto &F : M) {
    auto Name = F.getName();
    if (Name.npos != Name.find("read_channel_altera")) {
      Changed |= replaceReadChannel(F, *ReadPipe, M, ChannelToPipeMap);
    } else if (Name.npos !=
               F.getName().find("write_channel_altera")) {
      Changed |= replaceWriteChannel(F, *WritePipe, M, ChannelToPipeMap);
    }
  }
  return Changed;
}

bool ChannelPipeTransformation::runOnModule(Module &M) {
  bool Changed = false;

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  SmallVector<Module*, 2> BuiltinModules = BLI.getBuiltinModules();

  ValueToValueMap ChannelToPipeMap;
  Changed |= createPipeGlobals(M, ChannelToPipeMap, BuiltinModules);
  Changed |= replaceChannelBuiltins(M, ChannelToPipeMap, BuiltinModules);

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
