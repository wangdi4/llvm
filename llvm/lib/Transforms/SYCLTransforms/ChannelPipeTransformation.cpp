//===-- ChannelPipeTransformation.cpp -------------------------------------===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/ChannelPipeTransformation.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/DiagnosticInfo.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/SYCLChannelPipeUtils.h"
#include <numeric>
#include <stack>

using namespace llvm;
using namespace SYCLChannelPipeUtils;
using namespace SYCLKernelMetadataAPI;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-channel-pipe-transformation"

using AllocaMapType = DenseMap<std::pair<Function *, Type *>, Value *>;
// Stack is needed here to implement iterative DFS through global channel uses
// The second argument defines underlying container for stack: by default it is
// a std::deque, let's change it to std::vector since the last one has less
// overheads on accessing elements
using WorkListType = std::stack<Value *, std::vector<Value *>>;

extern unsigned SYCLChannelDepthEmulationMode;

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
  for (auto I : IndicesList)
    GEPIndicesList.push_back(ConstantInt::get(Int32Ty, I));
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
                                             GlobalVariable *BS,
                                             GlobalVariable *PipeArrayGlobal,
                                             const ChannelPipeMD &PipeMD) {
  auto *PipePtrArrayTy = cast<ArrayType>(PipeArrayGlobal->getValueType());
  auto *PipePtrTy = getArrayElementType(PipePtrArrayTy);
  auto *PipePtrPtrTy = PointerType::get(PipePtrTy, ADDRESS_SPACE_GLOBAL);

  SmallVector<size_t, 8> Dimensions;
  getArrayTypeDimensions(PipePtrArrayTy, Dimensions);
  unsigned NumDims = Dimensions.size();
  assert(NumDims >= 1 && "invalid array type dimensions");

  Type *BSIndexTy = Type::getIntNTy(
      M.getContext(),
      M.getDataLayout().getPointerSizeInBits(ADDRESS_SPACE_GLOBAL));
  Value *Zero = ConstantInt::get(BSIndexTy, 0);
  size_t BSItemSize = OpenCLInterface::__pipe_get_total_size_fpga(
      PipeMD.PacketSize, PipeMD.Depth, SYCLChannelDepthEmulationMode);

  // For small array, there is no need to create loop.
  constexpr size_t Threshold = 8;
  size_t BSItemsCount = std::accumulate(Dimensions.begin(), Dimensions.end(), 1,
                                        std::multiplies<size_t>());
  if (BSItemsCount <= Threshold) {
    SmallVector<size_t, 8> IndicesListForPipeElem(NumDims, 0);
    SmallVector<Value *, 8> GEPIndicesListForPipeElem(NumDims + 1);
    // Iterate over all elements from backing store.
    for (size_t I = 0; I < BSItemsCount; ++I) {
      // Create GEP from backing store.
      Value *IndexListForBSElem[] = {
          Zero, ConstantInt::get(BSIndexTy, I * BSItemSize)};
      Value *BSElemPtr = Builder.CreateGEP(
          BS->getValueType(), BS, ArrayRef<Value *>(IndexListForBSElem, 2));

      // Convert current indices list to GEP indices.
      convertToGEPIndicesList(IndicesListForPipeElem, GEPIndicesListForPipeElem,
                              M);
      // Increment current indices.
      incrementIndicesList(IndicesListForPipeElem, Dimensions);

      // Create GEP from pipe array.
      Value *PipeElemPtr =
          Builder.CreateGEP(PipePtrArrayTy, PipeArrayGlobal,
                            ArrayRef<Value *>(GEPIndicesListForPipeElem));
      Builder.CreateStore(Builder.CreateBitCast(BSElemPtr, PipePtrTy),
                          Builder.CreateBitCast(PipeElemPtr, PipePtrPtrTy));
    }
    return;
  }

  BasicBlock *Exit = Builder.GetInsertBlock();
  auto *Header = Exit->splitBasicBlockBefore(Exit->getTerminator(), "body");
  auto *Preheader =
      Header->splitBasicBlockBefore(Header->getTerminator(), "preheader");
  unsigned NumLatches = NumDims - 1;
  SmallVector<BasicBlock *, 8> Latches(NumLatches);
  for (size_t I = 0; I < NumLatches; ++I) {
    BasicBlock *BB = Exit;
    Latches[I] = BB->splitBasicBlockBefore(BB->getTerminator(),
                                           Twine("latch") + Twine(I + 1));
  }

  Builder.SetInsertPoint(Header->getTerminator());
  SmallVector<PHINode *, 8> PHIs(NumDims);
  for (int I = (int)NumDims - 1; I >= 0; --I) {
    auto *PHI =
        Builder.CreatePHI(BSIndexTy, NumDims + 1, Twine("i") + Twine(I));
    PHI->addIncoming(Zero, Preheader);
    PHI->addIncoming(PHI, Header);
    for (int I2 = 0; I2 < (int)NumLatches; ++I2) {
      PHI->addIncoming(I <= I2 ? cast<Value>(Zero) : cast<Value>(PHI),
                       Latches[I2]);
    }
    PHIs[I] = PHI;
  }

  Value *One = ConstantInt::get(BSIndexTy, 1);
  PHINode *LinearIdx = nullptr;
  if (NumDims > 1) {
    LinearIdx = Builder.CreatePHI(BSIndexTy, NumDims + 1, "j");
    auto *LinearIdxInc =
        Builder.CreateAdd(LinearIdx, One, Twine(LinearIdx->getName()) + ".inc");
    LinearIdx->addIncoming(Zero, Preheader);
    for (unsigned I = 0; I < NumDims; ++I)
      LinearIdx->addIncoming(LinearIdxInc, I == 0 ? Header : Latches[I - 1]);
  }

  SmallVector<Value *, 8> Increments(NumDims);
  for (unsigned I = 0; I < NumDims; ++I) {
    BasicBlock *BB = (I == 0) ? Header : Latches[I - 1];
    auto *OldTerminator = BB->getTerminator();
    Builder.SetInsertPoint(OldTerminator);
    Increments[I] =
        Builder.CreateAdd(PHIs[I], One, Twine(PHIs[I]->getName()) + ".inc");
    PHIs[I]->setIncomingValue(1 + I, Increments[I]);
    auto *Cond = Builder.CreateCmp(
        CmpInst::ICMP_SLT, Increments[I],
        ConstantInt::get(BSIndexTy, (uint64_t)Dimensions[NumDims - 1 - I]));
    Builder.CreateCondBr(Cond, Header, I == (NumDims - 1) ? Exit : Latches[I]);
    OldTerminator->eraseFromParent();
  }

  Builder.SetInsertPoint(cast<Instruction>(Increments[0]));
  // Create GEP from backing store.
  Value *Index = LinearIdx ? LinearIdx : PHIs[0];
  Value *BSIdx =
      Builder.CreateMul(Index, ConstantInt::get(BSIndexTy, BSItemSize),
                        "elem.ind", /*HasNUW*/ true, /*HasNSW*/ true);
  Value *IndexListForBSElem[] = {Zero, BSIdx};
  Value *BSElemPtr =
      Builder.CreateGEP(BS->getValueType(), BS, IndexListForBSElem);

  // Convert current indices list to GEP indices.
  SmallVector<Value *, 8> GEPIndicesListForPipeElem = {Zero};
  GEPIndicesListForPipeElem.append(PHIs.rbegin(), PHIs.rend());

  // Create GEP from pipe array.
  auto *PipeElemPtr = Builder.CreateGEP(PipePtrArrayTy, PipeArrayGlobal,
                                        GEPIndicesListForPipeElem);
  Builder.CreateStore(Builder.CreateBitCast(BSElemPtr, PipePtrTy),
                      Builder.CreateBitCast(PipeElemPtr, PipePtrPtrTy));
}

static void initializeGlobalPipeArray(GlobalVariable *PipeGV,
                                      const ChannelPipeMD &MD,
                                      Function *GlobalCtor,
                                      Function *PipeInitArray) {
  auto *BS = SYCLChannelPipeUtils::createPipeBackingStore(PipeGV, MD);
  auto *Terminator = GlobalCtor->getEntryBlock().getTerminator();

  IRBuilder<> Builder(Terminator);

  Value *PacketSize = Builder.getInt32(MD.PacketSize);
  Value *Depth = Builder.getInt32(MD.Depth);

  generateBSItemsToPipeArrayStores(
      *(PipeGV->getParent()), Builder, BS, PipeGV, MD);

  ArrayType *PipePtrArrayTy = cast<ArrayType>(PipeGV->getValueType());

  size_t BSNumItems = getNumElementsOfNestedArray(PipePtrArrayTy);
  Value *Mode = Builder.getInt32(SYCLChannelDepthEmulationMode);

  Terminator->getParent()->setName("exit");
  Builder.SetInsertPoint(Terminator);
  SmallVector<Value *, 6> CallArgs = {
      Builder.CreateBitCast(PipeGV,
                            PipeInitArray->getFunctionType()->getParamType(0)),
      Builder.getInt32(BSNumItems), PacketSize, Depth, Mode};

  if (MD.Protocol >= 0) {
    CallArgs.push_back(Builder.getInt32(MD.Protocol));
  }

  Builder.CreateCall(PipeInitArray, CallArgs);
}

static void initGlobalPipes(Module &M, RuntimeService &RTS,
                            SetVector<GlobalVariable *> &GlobalPipes) {
  Function *GlobalCtor = createPipeGlobalCtor(M);
  SmallVector<GlobalVariable *, 2> DepthIgnoredGVs;
  for (GlobalVariable *GV : GlobalPipes) {
    auto ChannelMD = GlobalVariableMetadataAPI(GV);
    if (!ChannelMD.PipeDepth.hasValue() &&
        SYCLChannelDepthEmulationMode == ChannelDepthMode::Default)
      DepthIgnoredGVs.push_back(GV);

    ChannelPipeMD MD = getChannelPipeMetadata(GV);
    if (isa<ArrayType>(GV->getValueType())) {
      Function *PipeInitFunc = importFunctionDecl(
          &M, RTS.findFunctionInBuiltinModules(
                  MD.Protocol < 0 ? "__pipe_init_array_fpga"
                                  : "__pipe_init_array_ext_fpga"));
      initializeGlobalPipeArray(GV, MD, GlobalCtor, PipeInitFunc);
    } else {
      Function *PipeInitFunc =
          importFunctionDecl(&M, RTS.findFunctionInBuiltinModules(
                                     MD.Protocol < 0 ? "__pipe_init_fpga"
                                                     : "__pipe_init_ext_fpga"));
      initializeGlobalPipeScalar(GV, MD, GlobalCtor, PipeInitFunc);
    }
  }

  if (!DepthIgnoredGVs.empty()) {
    std::string NameList;
    raw_string_ostream OS(NameList);
    for (auto *GV : DepthIgnoredGVs)
      OS << "\n - " << GV->getName().str();
    M.getContext().diagnose(OptimizationWarningDiagInfo(
        Twine("The default channel depths in the emulation flow will be "
              "different from the hardware flow depth (0) to speed up "
              "emulation. The following channels are affected:") +
        NameList));
  }
}

static Value *getPacketPtr(Module &M, CallInst *ChannelCall,
                           ChannelKind CK, AllocaMapType &AllocaMap) {
  auto DL = M.getDataLayout();

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
    auto *PacketTy = FunTy->getReturnType();
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
  // void write_channel_intel(channel, <value1 ty1>, <value2 ty2>)
  // bool write_channel_nb_intel(channel, <value1 ty1>, <value2 ty2>)

  assert(ChannelCall->arg_size() <= 3 &&
               "Too many arguments for channel write function!");
  IRBuilder<> Builder(ChannelCall);
  if(ChannelCall->arg_size() == 3) {
    // This is a channel write with coerced arguments
    // At this time, struct size is > 8 bytes and <= 16 bytes
    // Need to create a struct that contains the coerced arguments and
    // the size of the new struct is the same as the original struct type
    auto *PacketArg1 = ChannelCall->getArgOperand(1);
    auto *PacketArg2 = ChannelCall->getArgOperand(2);

    SmallVector<Type *, 2> CoercedTypeVec;
    CoercedTypeVec.push_back(PacketArg1->getType());
    CoercedTypeVec.push_back(PacketArg2->getType());
    StructType *StructMergeTy =
        StructType::create(M.getContext(), CoercedTypeVec,
                           "struct.channelpipetransformation.merge");

    auto *&WritePacketPtr = AllocaMap[std::make_pair(TargetFn, StructMergeTy)];
    if(!WritePacketPtr)
      WritePacketPtr = new AllocaInst(StructMergeTy, DL.getAllocaAddrSpace(),
                                      "write.src", &*AllocaInsertionPt);

    SmallVector<Value *, 2> Indices(
        2, ConstantInt::get(IntegerType::get(M.getContext(), 32), 0));
    auto *GepArg1 =
        Builder.CreateGEP(StructMergeTy, WritePacketPtr, Indices, "");
    Builder.CreateStore(PacketArg1, GepArg1);
    Indices[1] = ConstantInt::get(IntegerType::get(M.getContext(), 32), 1);
    auto *GepArg2 =
        Builder.CreateGEP(StructMergeTy, WritePacketPtr, Indices, "");
    Builder.CreateStore(PacketArg2, GepArg2);

    return WritePacketPtr;
  }

  // Normal channel write function, three cases:
  // 1. argument is not struct
  // 2. argument is struct with size > 16 bytes (not coerced)
  // 3. argument is struct with size <= 8 bytes (coerced, but argument is not split)
  auto *Packet = ChannelCall->getArgOperand(1);
  if (isa<PointerType>(Packet->getType())) {
    // Struct is passed by pointer anyway
    [[maybe_unused]] Type *EltTy = nullptr;
    if (auto *AI = dyn_cast<AllocaInst>(Packet))
      EltTy = AI->getAllocatedType();
    assert(EltTy && EltTy->isStructTy() &&
           "Expected a pointer to a struct type.");
    return Packet;
  }

  auto *&WritePacketPtr = AllocaMap[std::make_pair(TargetFn, Packet->getType())];
  if (!WritePacketPtr)
    WritePacketPtr = new AllocaInst(Packet->getType(), DL.getAllocaAddrSpace(),
                                    "write.src", &*AllocaInsertionPt);

  Builder.CreateStore(Packet, WritePacketPtr);

  return WritePacketPtr;
}

static void replaceChannelCallResult(CallInst *ChannelCall, ChannelKind CK,
                                     Value *PipePacketPtr, Value *PipeBoolRet) {
  IRBuilder<> Builder(ChannelCall);
  bool UsesStruct = ChannelCall->getFunctionType()->getReturnType()->isVoidTy();
  Type *PipePacketPtrValueTy;
  if (auto *AI = dyn_cast<AllocaInst>(PipePacketPtr))
    PipePacketPtrValueTy = AI->getAllocatedType();
  else
    assert(false && "unable to get pipe value type");

  if (!CK.Blocking) {
    // Channel and pipe built-ins have different (inverted) meaning for success
    // and failure.
    auto *BoolRet = Builder.CreateICmpEQ(PipeBoolRet, Builder.getInt32(0));
    if (CK.Access == ChannelKind::READ) {
      // <value ty> read_channel_nb(channel, bool*)
      // Struct is passed by pointer as a first arugment.
      // void read_channel_nb(<struct ty> *, channel, bool*)
      if (!UsesStruct)
        ChannelCall->replaceAllUsesWith(
            Builder.CreateLoad(PipePacketPtrValueTy, PipePacketPtr));

      unsigned IsValidIdx = UsesStruct ? 2 : 1;
      auto *IsValid = ChannelCall->getArgOperand(IsValidIdx);
      auto *Strip = IsValid->stripPointerCasts();
      Type *IsValidTy;
      if (auto *AI = dyn_cast<AllocaInst>(Strip))
        IsValidTy = AI->getAllocatedType();
      else
        IsValidTy = IntegerType::get(Strip->getContext(), 8);
      LLVM_DEBUG(
          dbgs() << "replaceChannelCallResult, non-blocking read, IsValid: "
                 << *IsValid << ", IsValidTy: " << *IsValidTy << "\n");
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
      ChannelCall->replaceAllUsesWith(
          Builder.CreateLoad(PipePacketPtrValueTy, PipePacketPtr));
    }
    // Struct is passed by pointer as a first arugment. No return value,
    // nothing to do
    // void read_channel(<struct ty> *ret, channel)
  }

  // Write built-ins don't have return value, nothing to do
  // void write_channel(channel, <value ty>)
}

static void replaceChannelBuiltinCall(Module &M, CallInst *ChannelCall,
                                      GlobalVariable *GlobalPipe, Value *Pipe,
                                      AllocaMapType &AllocaMap,
                                      RuntimeService &RTS) {
  assert(GlobalPipe && "Failed to find corresponding global pipe");
  assert(ChannelCall && "ChannelCall should be null");
  assert(ChannelCall->getCalledFunction() && "Indirect function is unexpected");
  ChannelKind CK =
      getChannelKind(ChannelCall->getCalledFunction()->getName());

  PipeKind PK;
  PK.Op = PipeKind::OpKind::ReadWrite;
  PK.Scope = PipeKind::ScopeKind::WorkItem;
  PK.Access = CK.Access == ChannelKind::READ ? PipeKind::AccessKind::Read
                                             : PipeKind::AccessKind::Write;
  PK.Blocking = CK.Blocking;
  PK.FPGA = true;

  Value *PacketPtr = getPacketPtr(M, ChannelCall, CK, AllocaMap);
  Function *Builtin = getPipeBuiltin(M, RTS, PK);
  FunctionType *FTy = Builtin->getFunctionType();

  auto PipeMD = GlobalVariableMetadataAPI(GlobalPipe);

  IRBuilder<> Builder(ChannelCall);
  Value *Args[] = {
      Pipe, Builder.CreatePointerCast(PacketPtr, FTy->getParamType(1)),
      ConstantInt::get(FTy->getParamType(2), PipeMD.PipePacketSize.get()),
      ConstantInt::get(FTy->getParamType(3), PipeMD.PipePacketAlign.get())};

  CallInst *BoolRet = Builder.CreateCall(Builtin, Args, ChannelCall->getName());

  replaceChannelCallResult(ChannelCall, CK, PacketPtr, BoolRet);
}

static bool isChannelBuiltinCall(CallInst *Call) {
  Function *CalledFunction = Call->getCalledFunction();
  assert(CalledFunction && "Indirect function call?");

  return getChannelKind(CalledFunction->getName());
}

static Argument *getFunctionArg(Function *F, const unsigned ArgNo) {
  assert(ArgNo < F->arg_size() && "Invalid ArgNo");
  return F->arg_begin() + ArgNo;
}

static void replaceLocalChannelUses(Function *UserFunc, const unsigned ArgNo,
                                    SmallPtrSetImpl<Value *> &Visited,
                                    WorkListType &WorkList) {
  Argument *Arg = getFunctionArg(UserFunc, ArgNo);

  // It is possible that replaceLocalChannelUses will be called several times
  // for the same UserFunc and ArgNo, e.g. user-defined function were called
  // several times
  //
  // It is enough to replace argument only once
  if (!Visited.insert(Arg).second)
    return;

  for (auto *ArgUser : Arg->users()) {
    if (auto *Store = dyn_cast<StoreInst>(ArgUser)) {
      Value *POperand = Store->getPointerOperand();
      assert(isa<AllocaInst>(POperand) && "Expected alloca for argument");
      WorkList.push(POperand);
    }
  }

  WorkList.push(Arg);
}

/// Clean-up channel built-ins declarations.
static void removeChannelBuiltinDecl(Module &M) {
  for (auto &F : make_early_inc_range(M)) {
    if (!F.isDeclaration())
      continue;
    ChannelKind Kind = getChannelKind(F.getName());
    if (!Kind)
      continue;
    if (!F.use_empty()) {
      // Remove users of channel built-in in non-kernel function that isn't used
      // by any kernel.
      FuncSet Root;
      Root.insert(&F);
      FuncSet UserFuncs;
      LoopUtils::fillFuncUsersSet(Root, UserFuncs);
      for ([[maybe_unused]] auto *K : getKernels(M))
        assert(!UserFuncs.contains(K) &&
               "Channel built-in is still used by kernel");
      for (User *U : make_early_inc_range(F.users())) {
        auto *UI = cast<Instruction>(U);
        if (Kind.Access == ChannelKind::READ)
          UI->replaceAllUsesWith(UndefValue::get(UI->getType()));
        UI->eraseFromParent();
      }
      assert(F.use_empty() && "Channel built-in still has use");
    }
    F.eraseFromParent();
  }
}

static void replaceGlobalPipeUses(Module &M, RuntimeService &RTS,
                                  SetVector<GlobalVariable *> &GlobalPipes) {
  // See comments about WorkListType typedef for explanations
  WorkListType WorkList;
  for (auto *GV : GlobalPipes)
    WorkList.push(GV);
  SmallPtrSet<Value *, 32> Visited;
  // This map is used to cache temporary alloca instuctions used by pipe
  // built-ins
  AllocaMapType AllocaMap;
  GlobalVariable *GlobalPipe = nullptr;

  while (!WorkList.empty()) {
    Value *Pipe = WorkList.top();
    WorkList.pop();

    if (auto *GV = dyn_cast<GlobalVariable>(Pipe);
        GV && GlobalPipes.contains(GV))
      GlobalPipe = GV;

    for (const auto &U : make_early_inc_range(Pipe->uses())) {
      auto OpNo = U.getOperandNo();
      User *Usr = U.getUser();

      if (CallInst *Call = dyn_cast<CallInst>(Usr)) {
        if (isChannelBuiltinCall(Call)) {
          replaceChannelBuiltinCall(M, Call, GlobalPipe, Pipe, AllocaMap, RTS);
          Call->eraseFromParent();
        } else {
          // handle calls to user-functions here
          Function *Callee = Call->getCalledFunction();
          assert(Callee && "Indirect function call?");
          replaceLocalChannelUses(Callee, OpNo, Visited, WorkList);
        }
        continue;
      }

      if (Visited.insert(Usr).second)
        WorkList.push(Usr);
    }
  }

  removeChannelBuiltinDecl(M);
}

PreservedAnalyses
ChannelPipeTransformationPass::run(Module &M, ModuleAnalysisManager &MAM) {
  RuntimeService &RTS =
      MAM.getResult<BuiltinLibInfoAnalysis>(M).getRuntimeService();
  SetVector<GlobalVariable *> GlobalPipes;
  llvm::for_each(M.globals(), [&](GlobalVariable &GV) {
    if (isGlobalPipe(&GV))
      GlobalPipes.insert(&GV);
  });
  if (GlobalPipes.empty())
    return PreservedAnalyses::all();

  replaceGlobalPipeUses(M, RTS, GlobalPipes);

  initGlobalPipes(M, RTS, GlobalPipes);

  return PreservedAnalyses::none();
}
