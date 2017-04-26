// Copyright (c) 2017 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

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
      }
    }

    assert(ChanMD && PacketSizeMD && PacketAlignMD &&
           "Invalid channel metadata");

    Value *Chan = ChanMD->getValue();
    ConstantInt *PacketSize = cast<ConstantInt>(PacketSizeMD->getValue());
    ConstantInt *PacketAlign = cast<ConstantInt>(PacketAlignMD->getValue());

    Value *Pipe = ChannelToPipeMap[Chan];
    assert(Pipe && "No channel to pipe mapping.");

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
    if (auto *PipePtrArrayTy = dyn_cast<ArrayType>(
            PipeOpaquePtr->getType()->getElementType())) {
      BSSize *= CompilationUtils::getArrayNumElements(PipePtrArrayTy);
    }
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

/**
 * \brief Calculates combination of indices to access the next element in a
 *        multidimensional array
 *
 * Examples:
 *   IndicesList(0, 0, 0, 0), Dimensions(5, 4, 3, 2) -> IndicesList(0, 0, 0, 1)
 *   IndicesList(0, 1, 2, 1), Dimensions(5, 4, 3, 2) -> IndicesList(0, 2, 0, 0)
 *
 * \param[in,out] IndicesList Current indicies
 * \param[in] Dimensions Number of elements in array in each dimension
 */
static void incrementIndicesList(SmallVectorImpl<size_t> &IndicesList,
                                 const SmallVectorImpl<size_t> &Dimensions) {
  size_t CurIndex = Dimensions.size() - 1;
  // Let's find rightmost index which we should increment
  // If index is going to overflow, reset it to zero and go to the left
  while (IndicesList[CurIndex] + 1 >= Dimensions[CurIndex] && CurIndex > 0) {
    IndicesList[CurIndex] = 0;
    --CurIndex;
  }
  ++IndicesList[CurIndex];
}

/**
 * \brief Converts vector of indices to vector of Value* to use it in GEP
 *        instruction
 *
 * \param[in] IndicesList List of indices needs to be converted
 * \param[out] GEPIndicesList Result vector of corresponding Value*. Note
 *                            that the size of GEPIndicesList will be equal
 *                            IndicesList.size() + 1: zero will be inserted
 *                            into first element of GEPIndicesList to use
 *                            this list in the GEP instruction
 * \param[in] M Used to create int32 type which used to create Value*
 */
static void convertToGEPIndicesList(const SmallVectorImpl<size_t> &IndicesList,
                                    SmallVectorImpl<Value *> &GEPIndicesList,
                                    Module &M) {
  Type *Int32Ty = Type::getInt32Ty(M.getContext());
  GEPIndicesList.resize(0);
  GEPIndicesList.push_back(ConstantInt::get(Int32Ty, 0));
  for (auto i: IndicesList) {
    GEPIndicesList.push_back(ConstantInt::get(Int32Ty, i));
  }
}

/**
 * \brief Generates set of store instuctions which maps items of global pipe
 *        array to elements of backing store
 *
 * \param[in] M Used to create int32 type in convertToGEPIndicesList function
 * \param[in] Builder IRBuilder used to create store instructions
 * \param[in] BS Backing store
 * \param[in] PipeArrayGlobal Array of pipes
 * \param[in] PipeMD Pipe metadata
 */
static void generateBSItemsToPipeArrayStores(Module &M, IRBuilder<> &Builder,
                                            Value *BS, Value *PipeArrayGlobal,
                                            const PipeMetadata &PipeMD) {
  auto *PipePtrArrayTy = cast<ArrayType>(
      cast<PointerType>(PipeArrayGlobal->getType())->getElementType());
  auto *PipePtrTy = CompilationUtils::getArrayElementType(PipePtrArrayTy);
  auto *PipePtrPtrTy = PointerType::get(PipePtrTy,
                                        Utils::OCLAddressSpace::Global);

  SmallVector<size_t, 8> Dimensions;
  CompilationUtils::getArrayTypeDimensions(PipePtrArrayTy, Dimensions);

  Type *BSIndexTy = Type::getIntNTy(
      M.getContext(),
      M.getDataLayout().getPointerSizeInBits(Utils::OCLAddressSpace::Global));
  Value *ZeroConstantInt = ConstantInt::get(BSIndexTy, 0);

  size_t DimensionsNum = Dimensions.size();
  SmallVector<size_t, 8> IndicesListForPipeElem(DimensionsNum, 0);
  SmallVector<Value *, 8> GEPIndicesListForPipeElem(DimensionsNum + 1, 0);

  size_t BSItemSize = pipe_get_total_size(PipeMD.PacketSize, PipeMD.Depth);
  size_t BSItemsCount = CompilationUtils::getArrayNumElements(PipePtrArrayTy);

  // iterate over all elements from backing store
  for (size_t i = 0; i < BSItemsCount; ++i) {
    // create GEP from backing store
    Value *IndexListForBSElem[] = {
      ZeroConstantInt,
      ConstantInt::get(BSIndexTy, i * BSItemSize)
    };
    Value *BSElemPtr = Builder.CreateGEP(
        BS, ArrayRef<Value *>(IndexListForBSElem, 2));

    // convert current indices list to GEP indices
    convertToGEPIndicesList(IndicesListForPipeElem,
                            GEPIndicesListForPipeElem, M);
    // increment current indices
    incrementIndicesList(IndicesListForPipeElem, Dimensions);

    // create GEP from pipe array
    Value *PipeElemPtr = Builder.CreateGEP(
        PipeArrayGlobal, ArrayRef<Value *>(GEPIndicesListForPipeElem));
    Builder.CreateStore(Builder.CreateBitCast(BSElemPtr, PipePtrTy),
                        Builder.CreateBitCast(PipeElemPtr, PipePtrPtrTy));

  }
}

static
Function *createPipesCtor(Module &M,
                          const PipeMetadataMap &PipesMD,
                          const ValueToValueMap &PipeToBSMap,
                          const SmallVectorImpl<Module *> &BuiltinModules) {
  Function *PipeInit = nullptr;
  Function *PipeInitArray = nullptr;
  for (const auto &BIModule : BuiltinModules) {
    if ((PipeInit = BIModule->getFunction("__pipe_init_intel"))) {
      PipeInit = cast<Function>(
          CompilationUtils::importFunctionDecl(&M, PipeInit));
    }
    if ((PipeInitArray = BIModule->getFunction("__pipe_init_array_intel"))) {
      PipeInitArray = cast<Function>(
          CompilationUtils::importFunctionDecl(&M, PipeInitArray));
    }

    if (PipeInit && PipeInitArray)
      break;
  }

  if (!PipeInit || !PipeInitArray) {
    assert(PipeInit && "__pipe_init_intel() not found in RTL.");
    assert(PipeInitArray && "__pipe_init_array_intel() not found in RTL.");
    return nullptr;
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
    PointerType *PipeGlobalTy = cast<PointerType>(PipeGlobal->getType());

    Type *Int32Ty = Type::getInt32Ty(M.getContext());
    Value *PipePacketSize = ConstantInt::get(Int32Ty, PipeMD.PacketSize);
    Value *PipeDepth = ConstantInt::get(Int32Ty, PipeMD.Depth);

    if (ArrayType *PipePtrArrayTy = dyn_cast<ArrayType>(
            PipeGlobalTy->getElementType())) {
      generateBSItemsToPipeArrayStores(M, Builder, BS, PipeGlobal, PipeMD);

      size_t BSNumItems = CompilationUtils::getArrayNumElements(PipePtrArrayTy);
      Value *CallArgs[] = {
        Builder.CreateBitCast(
            PipeGlobal, PipeInitArray->getFunctionType()->getParamType(0)),
        ConstantInt::get(Int32Ty, BSNumItems),
        PipePacketSize, PipeDepth
      };
      Builder.CreateCall(PipeInitArray, CallArgs);
    } else {
      Value *CallArgs[] = {
        Builder.CreateBitCast(
            BS, PipeInit->getFunctionType()->getParamType(0)),
            PipePacketSize, PipeDepth
      };
      Builder.CreateCall(PipeInit, CallArgs);
      Builder.CreateStore(
          Builder.CreateBitCast(BS, cast<PointerType>(
                                  PipeGlobal->getType())->getElementType()),
                                  PipeGlobal);
    }
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
  auto *ChannelPtrTy = PointerType::get(ChannelTy,
                                        Utils::OCLAddressSpace::Global);

  auto *PipeTy = StructType::create(M.getContext(), "opencl.pipe_t");
  auto *PipePtrTy = PointerType::get(PipeTy, Utils::OCLAddressSpace::Global);

  SmallVector<GlobalVariable *, 32> ChannelGlobals;
  for (auto &GV : M.globals()) {
    PointerType *GVPtrTy = GV.getType();
    Type *GVTy = GVPtrTy->getElementType();
    if (GVTy == ChannelPtrTy) {
      ChannelGlobals.push_back(&GV);
    } else if (auto *GVArrTy = dyn_cast<ArrayType>(GVTy)) {
      if (CompilationUtils::getArrayElementType(GVArrTy) == ChannelPtrTy) {
        ChannelGlobals.push_back(&GV);
      }
    }
  }

  for (auto *GV : ChannelGlobals) {
    SmallString<16> NameStr;
    auto PipeGVName = ("pipe." + GV->getName()).toStringRef(NameStr);
    PointerType *GVPtrTy = GV->getType();
    Type *GVTy = GVPtrTy->getElementType();

    auto *PipeGV = M.getGlobalVariable(PipeGVName);
    if (!PipeGV) {
      if (auto *GVArrTy = dyn_cast<ArrayType>(GVTy)) {
        SmallVector<size_t, 8> Dimensions;
        CompilationUtils::getArrayTypeDimensions(GVArrTy, Dimensions);
        auto *PipeArrayTy = CompilationUtils::createMultiDimArray(PipePtrTy,
                                                                  Dimensions);
        PipeGV = new GlobalVariable(M, PipeArrayTy, /*isConstant=*/false,
            GV->getLinkage(), /*Initializer=*/0, PipeGVName, /*InsertBefore=*/0,
            GlobalValue::ThreadLocalMode::NotThreadLocal,
            Utils::OCLAddressSpace::Global);
        PipeGV->setInitializer(ConstantAggregateZero::get(PipeArrayTy));
      } else {
        PipeGV = new GlobalVariable(M, PipePtrTy, /*isConstant=*/false,
            GV->getLinkage(), /*Initializer=*/0, PipeGVName, /*InsertBefore=*/0,
            GlobalValue::ThreadLocalMode::NotThreadLocal,
            Utils::OCLAddressSpace::Global);
        PipeGV->setInitializer(ConstantPointerNull::get(PipePtrTy));
      }
      PipeGV->setAlignment(4);
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

static Value *getPipeByChannel(Value *ChanGlobal,
                              ValueToValueMap ChannelToPipeMap,
                              IRBuilder<> &Builder) {
  Value *Pipe = nullptr;
  if (auto *ChanArrGEP = dyn_cast<GEPOperator>(ChanGlobal)) {
    // ChanGlobal is a GEP, so, it is read from array of channels
    // we need to replace GEP from array of channels
    // with GEP from array of pipes
    auto *PipeGlobal = ChannelToPipeMap[ChanArrGEP->getPointerOperand()];
    SmallVector<Value *, 8> Indices;
    auto IdxEnd = ChanArrGEP->idx_end();
    for (auto *Ind = ChanArrGEP->idx_begin(); Ind != IdxEnd; ++Ind) {
      Indices.push_back(*Ind);
    }

    Pipe = Builder.CreateGEP(PipeGlobal, ArrayRef<Value *>(Indices));
  } else {
    Pipe = ChannelToPipeMap[ChanGlobal];
  }

  return Pipe;
}

static void insertReadPipe(Function *ReadPipe,
                           Value *Pipe, Value *DstPtr,
                           IRBuilder<> &Builder) {
  auto *ReadPipeFTy = ReadPipe->getFunctionType();

  Value *PipeCallArgs[] = {
    Builder.CreateBitCast(
        Builder.CreateLoad(Pipe), ReadPipeFTy->getParamType(0)),
    Builder.CreatePointerBitCastOrAddrSpaceCast(
        DstPtr, ReadPipeFTy->getParamType(1))
  };

  Builder.CreateCall(ReadPipe, PipeCallArgs);
}

static void insertNBReadPipe(Function *NBReadPipe,
                             Value *Pipe, Value *DstPtr, Value *IsValidPtr,
                             IRBuilder<> &Builder, Module &M) {
  auto *ReadPipeFTy = NBReadPipe->getFunctionType();

  Value *PipeCallArgs[] = {
    Builder.CreateBitCast(
        Builder.CreateLoad(Pipe), ReadPipeFTy->getParamType(0)),
    Builder.CreatePointerBitCastOrAddrSpaceCast(
        DstPtr, ReadPipeFTy->getParamType(1))
  };

  Type *IsValidType =
      cast<PointerType>(IsValidPtr->getType())->getElementType();

  Builder.CreateStore(Builder.CreateZExt(Builder.CreateICmpEQ(
                          Builder.CreateCall(NBReadPipe, PipeCallArgs),
                          ConstantInt::get(NBReadPipe->getReturnType(), 0)),
                          IsValidType),
                      IsValidPtr);
}

static bool replaceReadChannel(Function &F, Function &ReadPipe,
                               Module &M,
                               ValueToValueMap ChannelToPipeMap,
                               bool NonBlocking = false) {
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
    Value *IsValidPtr = nullptr;
    Type *DstTy = ReadChannelFTy->getReturnType();
    if (DstTy->isVoidTy()) {
      // struct type result is passed by pointer as a first argument
      // the read_channel function returns void in this case
      DstPtr = (ArgIt++)->get();
      DstTy = DstPtr->getType();
    }
    Value *ChanArg = (ArgIt++)->get();
    if (NonBlocking) {
      IsValidPtr = (ArgIt++)->get();
    }

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
    IRBuilder<> Builder(ChannelCall);
    // TODO: remove original load instruction

    Value *Pipe = getPipeByChannel(ChanGlobal, ChannelToPipeMap, Builder);

    if (NonBlocking) {
      insertNBReadPipe(&ReadPipe, Pipe, DstPtr, IsValidPtr, Builder, M);
    } else {
      insertReadPipe(&ReadPipe, Pipe, DstPtr, Builder);
    }

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
                            IRBuilder<> &Builder) {
  auto *WritePipeFTy = WritePipe->getFunctionType();

  Value *PipeCallArgs[] = {
    Builder.CreateBitCast(
        Builder.CreateLoad(Pipe), WritePipeFTy->getParamType(0)),
    Builder.CreatePointerBitCastOrAddrSpaceCast(
        SrcPtr, WritePipeFTy->getParamType(1))
  };

  Builder.CreateCall(WritePipe, PipeCallArgs);
}

static Value *insertNBWritePipe(Function *NBWritePipe,
                                Value *Pipe, Value *SrcPtr,
                                IRBuilder<> &Builder, Module &M) {
  auto *WritePipeFTy = NBWritePipe->getFunctionType();

  Value *PipeCallArgs[] = {
    Builder.CreateBitCast(
        Builder.CreateLoad(Pipe), WritePipeFTy->getParamType(0)),
    Builder.CreatePointerBitCastOrAddrSpaceCast(
        SrcPtr, WritePipeFTy->getParamType(1))
  };

  return Builder.CreateICmpEQ(Builder.CreateCall(NBWritePipe, PipeCallArgs),
      ConstantInt::get(NBWritePipe->getReturnType(), 0));
}

static bool replaceWriteChannel(Function &F, Function &WritePipe,
                                Module &M,
                                ValueToValueMap ChannelToPipeMap,
                                bool NonBlocking = false) {
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
    IRBuilder<> Builder(ChannelCall);

    Function *TargetFn = ChannelCall->getParent()->getParent();
    Type *SrcType = Val->getType();
    Value *&SrcPtr = AllocaMap[std::make_pair(TargetFn, SrcType)];

    // Structs are passed by pointer, so if it is a pointer then use it as is
    if(SrcType->isPointerTy()) {
      assert(
          ((llvm::PointerType*)SrcType)->getPointerElementType()->isStructTy()
          && "Expected pointer to struct type.");
      SrcPtr = Val;
    }
    // If it is regular value make an alloca and store
    // the value to pass it by pointer to write_pipe
    else {
      if (!SrcPtr) {
        Instruction *InsertBefore =
          &*(TargetFn->getEntryBlock().getFirstInsertionPt());
        SrcPtr = new AllocaInst(SrcType, "write.src", InsertBefore);
      }
      new StoreInst(Val, SrcPtr, ChannelCall);
    }

    // TODO: remove original load instruction
    Value *Pipe = getPipeByChannel(ChanGlobal, ChannelToPipeMap, Builder);

    if (NonBlocking) {
      BasicBlock::iterator II(ChannelCall);
      ReplaceInstWithValue(ChannelCall->getParent()->getInstList(),
          II, insertNBWritePipe(&WritePipe, Pipe, SrcPtr, Builder, M));
      ;
    } else {
      insertWritePipe(&WritePipe, Pipe, SrcPtr, Builder);
      ChannelCall->eraseFromParent();
    }

    Changed = true;
  }
  return Changed;
}

static bool replaceChannelBuiltins(Module &M,
                                   ValueToValueMap ChannelToPipeMap,
                                   SmallVectorImpl<Module *> &BuiltinModules) {
  Function *ReadPipe = nullptr;
  Function *WritePipe = nullptr;
  Function *NBReadPipe = nullptr;
  Function *NBWritePipe = nullptr;
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

    if (!NBReadPipe) {
      NBReadPipe = cast<Function>(
        CompilationUtils::importFunctionDecl(
            &M, BIModule->getFunction("__read_pipe_2_intel")));
    }

    if (!NBWritePipe) {
      NBWritePipe = cast<Function>(
        CompilationUtils::importFunctionDecl(
            &M, BIModule->getFunction("__write_pipe_2_intel")));
    }

    if (ReadPipe && WritePipe && NBReadPipe && NBWritePipe)
        break;
  }

  assert(ReadPipe && "no '__read_pipe_2_bl_intel' built-in declared in RTL");
  assert(WritePipe && "no '__write_pipe_2_bl_intel' built-in declared in RTL");
  assert(NBReadPipe && "no '__read_pipe_2_intel' built-in declared in RTL");
  assert(NBWritePipe && "no '__write_pipe_2_intel' built-in declared in RTL");

  bool Changed = false;
  for (auto &F : M) {
    auto Name = F.getName();
    if (Name.npos != Name.find("read_channel_altera")) {
      Changed |= replaceReadChannel(F, *ReadPipe, M, ChannelToPipeMap);
    } else if (Name.npos != Name.find("read_channel_nb_altera")) {
      Changed |= replaceReadChannel(F, *NBReadPipe, M, ChannelToPipeMap, true);
    } else if (Name.npos != Name.find("write_channel_altera")) {
      Changed |= replaceWriteChannel(F, *WritePipe, M, ChannelToPipeMap);
    } else if (Name.npos != Name.find("write_channel_nb_altera")) {
      Changed |= replaceWriteChannel(
          F, *NBWritePipe, M, ChannelToPipeMap, true);
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
