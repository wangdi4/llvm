/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "ChannelPipeTransformation.h"

#include <llvm/ADT/SmallString.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <BuiltinLibInfo.h>
#include <CompilationUtils.h>
#include <InitializePasses.h>
#include <OCLAddressSpace.h>
#include <OCLPassSupport.h>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

typedef DenseMap<const Value*, Value*> ValueToValueMap;

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

static bool createPipeGlobals(Module &M, ValueToValueMap &ChannelToPipeMap) {
  auto *ChannelTy = M.getTypeByName("opencl.channel_t");
  if (!ChannelTy) {
    return false;
  }
  auto *ChannelPtrPtrTy = PointerType::get(
      PointerType::get(ChannelTy, Utils::OCLAddressSpace::Global),
      Utils::OCLAddressSpace::Global);

  auto *PipeTy = StructType::create(M.getContext(), "opencl.pipe_t");
  auto *PipePtrTy = PointerType::get(PipeTy, Utils::OCLAddressSpace::Global);

  SmallVector<const GlobalVariable *, 32> ChannelGlobals;
  for (const auto &GV : M.globals()) {
    if (GV.getType() == ChannelPtrPtrTy) {
      ChannelGlobals.push_back(&GV);
    }
  }

  for (auto *GV : ChannelGlobals) {
    StringRef Name = GV->hasName() ? GV->getName() : "";
    SmallString<16> NameStr;

    auto PipeGVName = ("pipe." + Name).toStringRef(NameStr);
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

  return ChannelGlobals.size() > 0;
}

static bool replaceReadChannel(Function &F, Function &Replacement,
                               Module &M, ValueToValueMap ChannelToPipeMap) {
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
    Value *PacketSize = (ArgIt++)->get();
    Value *PacketAlign = (ArgIt++)->get();

    if (!ResultPtr) {
      // primitive type result is returned by value from read_channel
      // make an alloca to pass it by pointer to read_pipe
      ResultPtr = new AllocaInst(ResultTy, "", /*InsertBefore*/ ChannelCall);
    }

    assert(ArgIt == ChannelCall->arg_end() &&
           "Unexpected number of arguments in read_channel_altera.");


    auto *ReadPipe = &Replacement;
    if (Replacement.getParent() != &M) {
      ReadPipe = dyn_cast<Function>(
          CompilationUtils::importFunctionDecl(&M, &Replacement));
    }
    auto *ReadPipeFTy = ReadPipe->getFunctionType();

    // discover the global value, from where our channel argument came from
    Value *ChanGlobal = dyn_cast<LoadInst>(ChanArg)->getPointerOperand();
    Value *PipeGlobal = ChannelToPipeMap[ChanGlobal];

    Value *PipeCallArgs[] = {
      new LoadInst(PipeGlobal, "", /*InsertBefore*/ChannelCall),
      CastInst::CreatePointerBitCastOrAddrSpaceCast(
          ResultPtr, ReadPipeFTy->getParamType(1),
          "", /*InsertBefore*/ChannelCall),
      PacketSize,
      PacketAlign
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
                                Module &M, ValueToValueMap ChannelToPipeMap) {
  bool Changed = false;
  for (auto *U : F.users()) {
    auto *ChannelCall = dyn_cast<CallInst>(U);
    if (!ChannelCall) {
      continue;
    }

    auto *Chan = ChannelCall->getArgOperand(0);
    auto *Val = ChannelCall->getArgOperand(1);
    auto *PacketSize = ChannelCall->getArgOperand(2);
    auto *PacketAlign = ChannelCall->getArgOperand(3);

    auto *WritePipe = &Replacement;
    if (Replacement.getParent() != &M) {
      WritePipe = dyn_cast<Function>(
          CompilationUtils::importFunctionDecl(&M, &Replacement));
    }
    auto *WritePipeFTy = WritePipe->getFunctionType();

    // discover the global value, from where our channel argument came from
    Value *ChanGlobal = dyn_cast<LoadInst>(Chan)->getPointerOperand();
    Value *PipeGlobal = ChannelToPipeMap[ChanGlobal];

    Value *ValPtr = new AllocaInst(Val->getType(), "",
                                   /*InsertBefore*/ChannelCall);
    new StoreInst(Val, ValPtr, /*InsertBefore*/ChannelCall);

    Value *PipeCallArgs[] = {
      new LoadInst(PipeGlobal, "", /*InsertBefore*/ChannelCall),
      CastInst::CreatePointerBitCastOrAddrSpaceCast(
          ValPtr, WritePipeFTy->getParamType(1),
          "", /*InsertBefore*/ChannelCall),
      PacketSize,
      PacketAlign
    };

    CallInst::Create(WritePipe, PipeCallArgs, "", ChannelCall);
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
  for (auto *M : BuiltinModules) {
    if (!ReadPipe) {
      ReadPipe = M->getFunction("__read_pipe_2");
    }

    if (!WritePipe) {
      WritePipe = M->getFunction("__write_pipe_2");
    }
  }

  assert(ReadPipe && "no '__read_pipe_2' built-in declared in RTL");
  assert(WritePipe && "no '__write_pipe_2' built-in declared in RTL");

  bool Changed = false;
  for (auto &F : M) {
    auto Name = F.getName();
    if (Name.npos != Name.find("read_channel_altera11ocl_channel")) {
      Changed |= replaceReadChannel(F, *ReadPipe, M, ChannelToPipeMap);
    } else if (Name.npos !=
               F.getName().find("write_channel_altera11ocl_channel")) {
      Changed |= replaceWriteChannel(F, *WritePipe, M, ChannelToPipeMap);
    }
  }
  return Changed;
}

bool ChannelPipeTransformation::runOnModule(Module &M) {
  bool Changed = false;
  ValueToValueMap ChannelToPipeMap;
  Changed |= createPipeGlobals(M, ChannelToPipeMap);

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  SmallVector<Module*, 2> BuiltinModules = BLI.getBuiltinModules();

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


