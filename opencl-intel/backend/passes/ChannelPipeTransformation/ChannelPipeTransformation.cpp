// Copyright (c) 2017-2018 Intel Corporation
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

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <BuiltinLibInfo.h>
#include <CompilationUtils.h>
#include <InitializePasses.h>
#include <MetadataAPI.h>
#include <OCLAddressSpace.h>
#include <OCLPassSupport.h>

#include <PipeCommon.h>

#include <algorithm>
#include <utility>
#include <vector>
#include <stack>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

static cl::opt<int> ChannelDepthEmulationMode("channel-depth-emulation-mode",
    cl::init(CHANNEL_DEPTH_MODE_IGNORE_DEPTH), cl::Hidden,
    cl::desc("Channel depth emulation mode"));

namespace intel {
char ChannelPipeTransformation::ID = 0;
OCL_INITIALIZE_PASS_BEGIN(ChannelPipeTransformation,
                          "channel-pipe-transformation",
                          "Transform Intel FPGA channels into OpenCL 2.0 pipes",
                          false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(ChannelPipeTransformation,
                        "channel-pipe-transformation",
                        "Transform Intel FPGA channels into OpenCL 2.0 pipes",
                        false, true)
}

#define DEBUG_TYPE "channel-pipe-transformation"

namespace {

struct ChannelMetadata {
  int PacketSize;
  int PacketAlign;
  int Depth;
  StringRef IO;
};

// Pipes and channels have the same metadata
typedef ChannelMetadata PipeMetadata;

typedef DenseMap<Value *, Value *> ValueToValueMap;
typedef DenseMap<std::pair<Function *, Type *>, Value *> AllocaMapType;

typedef std::pair<Value *, Value *> ValueValuePair;
// Stack is needed here to implement iterative DFS through global channel uses
// The second argument defines underlying container for stack: by default it is
// a std::deque, let's change it to std::vector since the last one has less
// overheads on accessing elements
typedef std::stack<ValueValuePair, std::vector<ValueValuePair>> WorkListType;

} // anonymous namespace

namespace intel {

static Function *getPipeBuiltin(OCLBuiltins &Builtins, PipeKind &Kind) {
  if (Kind.Blocking) {
    // There are no declarations and definitions of blocking pipe built-ins in
    // RTL's.
    // Calls to blocking pipe built-ins will be resolved later in PipeSupport,
    // so we just need to insert declarations here.
    PipeKind NonBlockingKind = Kind;
    NonBlockingKind.Blocking = false;

    Function *NonBlockingBuiltin =
        Builtins.get(CompilationUtils::getPipeName(NonBlockingKind));
    return cast<Function>(Builtins.getTargetModule().getOrInsertFunction(
        CompilationUtils::getPipeName(Kind),
        NonBlockingBuiltin->getFunctionType()));
  }

  return Builtins.get(CompilationUtils::getPipeName(Kind));
}

static bool isGlobalChannel(const GlobalValue *GV, const Type *ChannelTy) {
  auto *GVValueTy = GV->getType()->getElementType();

  if (ChannelTy == GVValueTy)
    return true;

  if (auto *GVArrTy = dyn_cast<ArrayType>(GVValueTy)) {
    if (ChannelTy == CompilationUtils::getArrayElementType(GVArrTy)) {
      return true;
    }
  }

  return false;
}

static GlobalVariable *createGlobalPipeScalar(Module &M, Type *PipeTy,
                                              const Twine &Name) {
  auto *PipeGV = new GlobalVariable(
      M, PipeTy, /*isConstant=*/false, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, Name, /*InsertBefore=*/nullptr,
      GlobalValue::ThreadLocalMode::NotThreadLocal,
      Utils::OCLAddressSpace::Global);

  PipeGV->setInitializer(ConstantPointerNull::get(cast<PointerType>(PipeTy)));
  PipeGV->setAlignment(M.getDataLayout().getPreferredAlignment(PipeGV));
  return PipeGV;
}

static GlobalVariable *createGlobalPipeArray(Module &M, Type *PipeTy,
                                             ArrayRef<size_t> Dimensions,
                                             const Twine &Name) {
  auto *PipeArrayTy = CompilationUtils::createMultiDimArray(PipeTy, Dimensions);

  auto *PipeGV = new GlobalVariable(
      M, PipeArrayTy, /*isConstant=*/false, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, Name, /*InsertBefore=*/nullptr,
      GlobalValue::ThreadLocalMode::NotThreadLocal,
      Utils::OCLAddressSpace::Global);

  PipeGV->setInitializer(ConstantAggregateZero::get(PipeArrayTy));
  PipeGV->setAlignment(M.getDataLayout().getPreferredAlignment(PipeGV));
  return PipeGV;
}

static GlobalVariable *createPipeBackingStore(GlobalVariable *GV,
                                              const PipeMetadata &MD) {
  Module *M = GV->getParent();
  Type *Int8Ty = IntegerType::getInt8Ty(M->getContext());

  size_t BSSize = __pipe_get_total_size(MD.PacketSize, MD.Depth,
      ChannelDepthEmulationMode);
  if (auto *PipePtrArrayTy =
          dyn_cast<ArrayType>(GV->getType()->getElementType())) {
    BSSize *= CompilationUtils::getArrayNumElements(PipePtrArrayTy);
  }

  auto *ArrayTy = ArrayType::get(Int8Ty, BSSize);

  SmallString<16> NameStr;
  auto Name = (GV->getName() + ".bs").toStringRef(NameStr);

  auto *BS =
      new GlobalVariable(*M, ArrayTy, /*isConstant=*/false, GV->getLinkage(),
                         /*initializer=*/nullptr, Name,
                         /*InsertBefore=*/nullptr, GlobalValue::NotThreadLocal,
                         Utils::OCLAddressSpace::Global);

  BS->setInitializer(ConstantAggregateZero::get(ArrayTy));
  BS->setAlignment(MD.PacketAlign);

  return BS;
}

static void initializeGlobalPipeScalar(GlobalVariable *PipeGV,
                                       const PipeMetadata &MD,
                                       Function *GlobalCtor,
                                       Function *PipeInit) {
  auto *BS = createPipeBackingStore(PipeGV, MD);

  IRBuilder<> Builder(GlobalCtor->getEntryBlock().getTerminator());

  Value *PacketSize = Builder.getInt32(MD.PacketSize);
  Value *Depth = Builder.getInt32(MD.Depth);
  Value *Mode = Builder.getInt32(ChannelDepthEmulationMode);

  Value *CallArgs[] = {
      Builder.CreateBitCast(BS, PipeInit->getFunctionType()->getParamType(0)),
      PacketSize, Depth, Mode
  };

  Builder.CreateCall(PipeInit, CallArgs);
  Builder.CreateStore(
      Builder.CreateBitCast(BS, PipeGV->getType()->getElementType()), PipeGV);
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

  size_t BSItemSize = __pipe_get_total_size(PipeMD.PacketSize, PipeMD.Depth,
                                            ChannelDepthEmulationMode);
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

static void initializeGlobalPipeArray(GlobalVariable *PipeGV,
                                      const PipeMetadata &MD,
                                      Function *GlobalCtor,
                                      Function *PipeInitArray) {
  auto *BS = createPipeBackingStore(PipeGV, MD);

  IRBuilder<> Builder(GlobalCtor->getEntryBlock().getTerminator());

  Value *PacketSize = Builder.getInt32(MD.PacketSize);
  Value *Depth = Builder.getInt32(MD.Depth);

  // TODO: seems to be a good place to rewrite. Generate loops in IR instead of
  // generating a huge bunch of GEP instructions
  generateBSItemsToPipeArrayStores(
      *(PipeGV->getParent()), Builder, BS, PipeGV, MD);

  ArrayType *PipePtrArrayTy =
      cast<ArrayType>(PipeGV->getType()->getElementType());

  size_t BSNumItems = CompilationUtils::getArrayNumElements(PipePtrArrayTy);
  Value *Mode = Builder.getInt32(ChannelDepthEmulationMode);

  Value *CallArgs[] = {
      Builder.CreateBitCast(PipeGV,
                            PipeInitArray->getFunctionType()->getParamType(0)),
      Builder.getInt32(BSNumItems), PacketSize, Depth, Mode
  };
  Builder.CreateCall(PipeInitArray, CallArgs);
}

static Function *createGlobalPipeCtor(Module &M) {
  auto *CtorTy = FunctionType::get(Type::getVoidTy(M.getContext()),
                                   ArrayRef<Type *>(), false);

  Function *Ctor =
      cast<Function>(M.getOrInsertFunction("__pipe_global_ctor", CtorTy));

  // The function will be called from the RT
  Ctor->setLinkage(GlobalValue::ExternalLinkage);

  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Ctor);
  ReturnInst::Create(M.getContext(), EntryBB);

  appendToGlobalCtors(M, Ctor, /*Priority=*/65535);

  return Ctor;
}

static ChannelMetadata getChannelMetadata(GlobalVariable *Channel) {
  auto GVMetadata = GlobalVariableMetadataAPI(Channel);

  assert(GVMetadata.PipePacketSize.hasValue() &&
         GVMetadata.PipePacketAlign.hasValue() &&
         "Channel metadata must contain packet_size and packet_align");

  ChannelMetadata CMD;
  CMD.PacketSize = GVMetadata.PipePacketSize.get();
  CMD.PacketAlign = GVMetadata.PipePacketAlign.get();
  CMD.Depth = GVMetadata.PipeDepth.hasValue() ? GVMetadata.PipeDepth.get() : 0;
  CMD.IO = GVMetadata.PipeIO.hasValue() ? GVMetadata.PipeIO.get() : "";

  return CMD;
}

static bool replaceGlobalChannels(Module &M, Type *ChannelTy, Type *PipeTy,
                                  ValueToValueMap &VMap,
                                  OCLBuiltins &Builtins) {
  bool Changed = false;
  Function *GlobalCtor = nullptr;

  for (auto &ChannelGV : M.globals()) {
    if (!isGlobalChannel(&ChannelGV, ChannelTy)) {
      continue;
    }

    if (!GlobalCtor) {
      GlobalCtor = createGlobalPipeCtor(M);
    }

    PipeMetadata MD = getChannelMetadata(&ChannelGV);
    GlobalVariable *PipeGV = nullptr;
    if (auto *GVArrTy =
            dyn_cast<ArrayType>(ChannelGV.getType()->getElementType())) {
      SmallVector<size_t, 8> Dimensions;
      CompilationUtils::getArrayTypeDimensions(GVArrTy, Dimensions);

      PipeGV = createGlobalPipeArray(M, PipeTy, Dimensions,
                                     ChannelGV.getName() + ".pipe");

      initializeGlobalPipeArray(PipeGV, MD, GlobalCtor,
                                Builtins.get("__pipe_init_array_intel"));
    } else {
      PipeGV = createGlobalPipeScalar(M, PipeTy, ChannelGV.getName() + ".pipe");

      initializeGlobalPipeScalar(PipeGV, MD, GlobalCtor,
                                 Builtins.get("__pipe_init_intel"));
    }

    auto ChannelMD = GlobalVariableMetadataAPI(&ChannelGV);
    PipeGV->setMetadata(ChannelMD.PipePacketSize.getID(),
        ChannelGV.getMetadata(ChannelMD.PipePacketSize.getID()));
    PipeGV->setMetadata(ChannelMD.PipePacketAlign.getID(),
        ChannelGV.getMetadata(ChannelMD.PipePacketAlign.getID()));
    PipeGV->setMetadata(ChannelMD.PipeDepth.getID(),
        ChannelGV.getMetadata(ChannelMD.PipeDepth.getID()));
    PipeGV->setMetadata(ChannelMD.PipeIO.getID(),
        ChannelGV.getMetadata(ChannelMD.PipeIO.getID()));

    VMap[&ChannelGV] = PipeGV;

    Changed = true;
  }

  return Changed;
}

static Value *createPipeUserStub(Value *ChannelUser, Value *Pipe) {
  Type *PipeTy = Pipe->getType();
  auto *UndefPipe = UndefValue::get(PipeTy);

  // isa<GEPOperator> returns true for instances of GetElementPtrInst class,
  // so there is an ugly if here
  if (isa<GEPOperator>(ChannelUser) && !isa<GetElementPtrInst>(ChannelUser)) {
    auto *GEPOp = cast<GEPOperator>(ChannelUser);
    SmallVector<Value *, 8> IdxList(GEPOp->idx_begin(), GEPOp->idx_end());

    ConstantFolder Folder;
    return Folder.CreateGetElementPtr(PipeTy->getPointerElementType(),
                                      cast<Constant>(Pipe), IdxList);
  }

  auto *ChannelInst = cast<Instruction>(ChannelUser);

  if (auto *Alloca = dyn_cast<AllocaInst>(ChannelInst)) {
    return new AllocaInst(PipeTy, Alloca->getType()->getAddressSpace(),
                          Alloca->getArraySize(), "pipe." + Alloca->getName(),
                          Alloca);
  }

  if (auto *Store = dyn_cast<StoreInst>(ChannelInst)) {
    Value *DestPtr = UndefValue::get(PointerType::getUnqual(PipeTy));
    return new StoreInst(UndefPipe, DestPtr, Store->isVolatile(),
                         Store->getAlignment(), Store->getOrdering(),
                         Store->getSyncScopeID(), Store);
  }

  if (auto *GEP = dyn_cast<GetElementPtrInst>(ChannelInst)) {
    SmallVector<Value *, 8> IdxList(GEP->idx_begin(), GEP->idx_end());

    return GetElementPtrInst::Create(PipeTy->getPointerElementType(), UndefPipe,
                                     IdxList, ChannelInst->getName(),
                                     ChannelInst);
  }

  if (auto *Load = dyn_cast<LoadInst>(ChannelInst)) {
    return new LoadInst(PipeTy->getPointerElementType(), UndefPipe,
                        Load->getName(), Load->isVolatile(), ChannelInst);
  }

  if (auto *Select = dyn_cast<SelectInst>(ChannelInst)) {
    return SelectInst::Create(Select->getCondition(), UndefPipe, UndefPipe,
                              ChannelInst->getName(), ChannelInst);
  }

  if (auto *Phi = dyn_cast<PHINode>(ChannelInst)) {
    auto *NewPhi = PHINode::Create(PipeTy, Phi->getNumIncomingValues(),
                                   ChannelInst->getName(), ChannelInst);
    for (auto *BB : Phi->blocks()) {
      NewPhi->addIncoming(UndefPipe, BB);
    }
    return NewPhi;
  }

  llvm_unreachable("Unsupported instruction.");
}

static CallInst *createCallInstStub(CallInst *Call, Function *Func,
                                    Type *ChannelTy, Type *PipeTy) {
  auto *UndefPipe = UndefValue::get(PipeTy);
  SmallVector<Value *, 8> Args;
  for (auto &Arg : Call->arg_operands()) {
    if (Arg->getType() == ChannelTy) {
      Args.push_back(UndefPipe);
    } else {
      Args.push_back(Arg);
    }
  }

  CallInst *Result = CallInst::Create(Func, Args, "", Call);
  Result->setDebugLoc(Call->getDebugLoc());
  return Result;
}

static Value *getPacketPtr(CallInst *ChannelCall,
                           ChannelKind CK, AllocaMapType &AllocaMap) {
  auto DL = ChannelCall->getModule()->getDataLayout();

  auto *TargetFn = ChannelCall->getFunction();
  auto AllocaInsertionPt =
      TargetFn->getEntryBlock().getFirstInsertionPt();

  auto *ChannelFun = ChannelCall->getCalledFunction();
  assert(ChannelFun && "Indirect call?");

  auto *FunTy = ChannelFun->getFunctionType();
  if (CK.Access == ChannelKind::READ) {
    // Read channel returns packet by value:
    //
    // <value ty> read_channel_intel(channel);
    // <value ty> read_channel_nb_intel(channel, bool*);
    //
    auto PacketTy = FunTy->getReturnType();
    if (PacketTy->isVoidTy()) {
      // Struct is passed by pointer as a first argument - we can reuse it.
      return ChannelCall->getArgOperand(0);
    }

    auto *&ReadPacketPtr = AllocaMap[std::make_pair(TargetFn, PacketTy)];
    if (!ReadPacketPtr)
      ReadPacketPtr = new AllocaInst(PacketTy, DL.getAllocaAddrSpace(),
                                     "read.dst", &*AllocaInsertionPt);
    return ReadPacketPtr;
  }

  // Write channel:
  // void write_channel_intel(channel, <value ty>)
  // bool write_channel_nb_intel(channel, <value ty>)

  auto *Packet = ChannelCall->getArgOperand(1);
  if (isa<PointerType>(Packet->getType())) {
    // Struct is passed by pointer anyway
    assert(cast<PointerType>(Packet->getType())
               ->getPointerElementType()->isStructTy() &&
               "Expected a pointer to a struct type.");
    return Packet;
  }

  auto *&WritePacketPtr = AllocaMap[std::make_pair(TargetFn, Packet->getType())];
  if (!WritePacketPtr)
    WritePacketPtr = new AllocaInst(Packet->getType(), DL.getAllocaAddrSpace(),
                                    "write.src", &*AllocaInsertionPt);

  new StoreInst(Packet, WritePacketPtr, ChannelCall);

  return WritePacketPtr;
}

static void replaceChannelCallResult(CallInst *ChannelCall, ChannelKind CK,
                                     Value *PipePacketPtr, Value *PipeBoolRet) {
  IRBuilder<> Builder(ChannelCall);
  bool UsesStruct = ChannelCall->getFunctionType()->getReturnType()->isVoidTy();

  if (!CK.Blocking) {
    // Channel and pipe built-ins have different (inverted) meaning for success
    // and failure.
    auto BoolRet = Builder.CreateICmpEQ(PipeBoolRet, Builder.getInt32(0));
    if (CK.Access == ChannelKind::READ) {
      // <value ty> read_channel_nb(channel, bool*)
      // Struct is passed by pointer as a first arugment.
      // void read_channel_nb(<struct ty> *, channel, bool*)
      if (!UsesStruct)
        ChannelCall->replaceAllUsesWith(Builder.CreateLoad(PipePacketPtr));

      unsigned IsValidIdx = UsesStruct ? 2 : 1;
      auto *IsValid = ChannelCall->getArgOperand(IsValidIdx);
      auto *IsValidTy = cast<PointerType>(IsValid->getType())->getElementType();
      Builder.CreateStore(Builder.CreateZExt(BoolRet, IsValidTy), IsValid);
    } else {
      // bool write_channel_nb(channel, <value ty>)
      ChannelCall->replaceAllUsesWith(BoolRet);
    }

    return;
  }

  // Now deal with blocking
  if (CK.Access == ChannelKind::READ) {
    if (!UsesStruct) {
      // <value ty> read_channel(channel);
      ChannelCall->replaceAllUsesWith(Builder.CreateLoad(PipePacketPtr));
    }
    // Struct is passed by pointer as a first arugment. No return value,
    // nothing to do
    // void read_channel(<struct ty> *ret, channel)
  }

  // Write built-ins don't have return value, nothing to do
  // void write_channel(channel, <value ty>)
}

static void replaceChannelBuiltinCall(CallInst *ChannelCall, Value *Pipe,
                                      AllocaMapType &AllocaMap,
                                      OCLBuiltins &Builtins) {
  ChannelKind CK = CompilationUtils::getChannelKind(
      ChannelCall->getCalledFunction()->getName());

  PipeKind PK;
  PK.Op = PipeKind::READWRITE;
  PK.Scope = PipeKind::WORK_ITEM;
  PK.Access = CK.Access == ChannelKind::READ ? PipeKind::READ : PipeKind::WRITE;
  PK.Blocking = CK.Blocking;

  Value *PacketPtr = getPacketPtr(ChannelCall, CK, AllocaMap);
  Function *Builtin = getPipeBuiltin(Builtins, PK);
  FunctionType *FTy = Builtin->getFunctionType();
  Value *Args[] = {
      CastInst::CreatePointerCast(Pipe, FTy->getParamType(0), "", ChannelCall),
      CastInst::CreatePointerCast(PacketPtr, FTy->getParamType(1), "",
                                  ChannelCall)
  };

  Value *BoolRet =
      CallInst::Create(Builtin, Args, ChannelCall->getName(), ChannelCall);

  replaceChannelCallResult(ChannelCall, CK, PacketPtr, BoolRet);
}

static bool isChannelBuiltinCall(CallInst *Call) {
  Function *CalledFunction = Call->getCalledFunction();
  assert(CalledFunction && "Indirect function call?");

  if (ChannelKind Kind =
          CompilationUtils::getChannelKind(CalledFunction->getName())) {
    return true;
  }

  return false;
}

static Function *createUserFunctionStub(CallInst *Call, Type *ChannelTy,
                                             Type *PipeTy) {
  Function *ExistingF = Call->getCalledFunction();
  assert(ExistingF && "Indirect function call?");

  FunctionType *ExistingFTy = ExistingF->getFunctionType();
  SmallVector<Type *, 4> ArgTys(ExistingFTy->param_begin(),
                                ExistingFTy->param_end());
  for (auto &ArgTy : ArgTys) {
    if (ArgTy == ChannelTy) {
      ArgTy = PipeTy;
    }
  }

  FunctionType *NewFTy = FunctionType::get(ExistingFTy->getReturnType(), ArgTys,
                                           ExistingFTy->isVarArg());
  Function *Replacement =
      Function::Create(NewFTy, ExistingF->getLinkage(),
                       "pipe." + ExistingF->getName(), ExistingF->getParent());

  ValueToValueMapTy VMap;
  auto ExistFArgIt = ExistingF->arg_begin();
  auto RArgIt = Replacement->arg_begin();
  auto RArgItE = Replacement->arg_end();
  for (; RArgIt != RArgItE; ++RArgIt, ++ExistFArgIt) {
    VMap[&*ExistFArgIt] = &*RArgIt;
  }

  SmallVector<ReturnInst *, 4> Returns;
  CloneFunctionInto(Replacement, ExistingF, VMap, true, Returns, "");
  assert(Replacement && "CloneFunctionInfo failed");

  ExistingF->deleteBody();

  return Replacement;
}

static void
replaceLocalChannelUses(Function *UserFunc, Type *ChannelTy, Type *PipeTy,
                        ValueToValueMap &VMap,
                        SmallPtrSetImpl<Instruction *> &ToDelete,
                        WorkListType &WorkList) {
  for (auto &Arg : UserFunc->args()) {
    if (Arg.getType() != PipeTy)
      continue;

    for (auto *ArgUser : Arg.users()) {
      if (auto *Store = dyn_cast<StoreInst>(ArgUser)) {
        Value *POperand = Store->getPointerOperand();
        assert(isa<AllocaInst>(POperand) && "Expected alloca for argument");
        // Let's do an initial replacement of alloca
        auto *&PipeUser = VMap[POperand];
        if (!PipeUser) {
          PipeUser = createPipeUserStub(POperand, &Arg);
          WorkList.push(std::make_pair(POperand, PipeUser));
        }
        ToDelete.insert(Store);
      }
    }

    WorkList.push(std::make_pair(&Arg, &Arg));
  }
}

static void cleanup(Module &M, SmallPtrSetImpl<Instruction *> &ToDelete,
                    ValueToValueMap &VMap) {
  for (auto I : ToDelete) {
    if (!I->use_empty()) {
      assert(isa<CallInst>(I) && "Expected that only calls to user functions "
                                 "still has users");
      I->replaceAllUsesWith(VMap[I]);
    }

    I->eraseFromParent();
  }

  for (auto It : VMap) {
    if (Function *F = dyn_cast<Function>(It.first)) {
      Function *R = cast<Function>(It.second);
      R->takeName(F);
      F->eraseFromParent();
    }
  }

  // remove channel built-ins declarations
  SmallVector<Function *, 8> FToDelete;
  for (auto &F : M) {
    if (F.isDeclaration() && CompilationUtils::getChannelKind(F.getName())) {
      FToDelete.push_back(&F);
    }
  }

  for (auto &F : FToDelete) {
    assert(F->use_empty() && "Users of channel built-in are still exists");
    F->eraseFromParent();
  }
}

static void replaceGlobalChannelUses(Module &M, Type *ChannelTy,
                                     ValueToValueMap &GlobalVMap,
                                     OCLBuiltins &Builtins) {
  ValueToValueMap VMap;
  SmallPtrSet<Instruction *, 32> ToDelete;
  // See comments about WorkListType typedef for explanations
  WorkListType WorkList;
  // This map is used to cache temporary alloca instuctions used by pipe
  // built-ins
  AllocaMapType AllocaMap;

  for (const auto &KV : GlobalVMap) {
    WorkList.push(std::make_pair(KV.first, KV.second));
  }

  while (!WorkList.empty()) {
    Value *Channel = WorkList.top().first;
    Value *Pipe = WorkList.top().second;
    WorkList.pop();

    for (const auto &U : Channel->uses()) {
      auto OpNo = U.getOperandNo();
      Value *ChannelUser = U.getUser();

      if (CallInst *Call = dyn_cast<CallInst>(ChannelUser)) {
        ToDelete.insert(Call);
        if (isChannelBuiltinCall(Call)) {
          replaceChannelBuiltinCall(Call, Pipe, AllocaMap, Builtins);
        } else {
          // handle calls to user-functions here
          assert(Call->getCalledFunction() && "Indirect function call?");
          Value *&FuncReplacement = VMap[Call->getCalledFunction()];
          if (!FuncReplacement) {
            FuncReplacement =
                createUserFunctionStub(Call, ChannelTy, Pipe->getType());
            replaceLocalChannelUses(cast<Function>(FuncReplacement), ChannelTy,
                                    Pipe->getType(), VMap, ToDelete, WorkList);
          }

          Value *&CallInstReplacement = VMap[Call];
          if (!CallInstReplacement) {
            CallInstReplacement =
                createCallInstStub(Call, cast<Function>(FuncReplacement),
                                   ChannelTy, Pipe->getType());
          }

          cast<User>(CallInstReplacement)->setOperand(OpNo, Pipe);
        }
        continue;
      }

      auto *&PipeUser = VMap[ChannelUser];
      if (!PipeUser) {
        PipeUser = createPipeUserStub(ChannelUser, Pipe);
        WorkList.push(std::make_pair(ChannelUser, PipeUser));
      }

      // isa<GEPOperator> returns true for instances of GetElementPtrInst class,
      // so there is an ugly if here
      if (!isa<GEPOperator>(PipeUser) || isa<GetElementPtrInst>(PipeUser))
        cast<User>(PipeUser)->setOperand(OpNo, Pipe);
    }
  }

  cleanup(M, ToDelete, VMap);
}

ChannelPipeTransformation::ChannelPipeTransformation() : ModulePass(ID) {
}

bool ChannelPipeTransformation::runOnModule(Module &M) {
  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  OCLBuiltins Builtins(M, BLI.getBuiltinModules());

  auto *ChannelValueTy =
      CompilationUtils::getStructByName("opencl.channel_t", &M);
  if (!ChannelValueTy)
    return false;

  auto *ChannelTy =
      PointerType::get(ChannelValueTy, Utils::OCLAddressSpace::Global);

  auto PipeTyName = "opencl.pipe_t";
  auto *PipeValueTy = M.getTypeByName(PipeTyName);
  if (!PipeValueTy) {
    PipeValueTy = StructType::create(M.getContext(), PipeTyName);
  }
  auto *PipeTy = PointerType::get(PipeValueTy, Utils::OCLAddressSpace::Global);

  ValueToValueMap GlobalVMap;
  bool Changed =
      replaceGlobalChannels(M, ChannelTy, PipeTy, GlobalVMap, Builtins);
  if (!Changed)
    return false;

  replaceGlobalChannelUses(M, ChannelTy, GlobalVMap, Builtins);

  return true;
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
