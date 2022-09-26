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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ChannelPipeTransformation.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPChannelPipeUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <stack>

using namespace llvm;
using namespace DPCPPChannelPipeUtils;
using namespace DPCPPKernelMetadataAPI;
using namespace CompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-channel-pipe-transformation"

using ValueToValueMap = DenseMap<Value *, Value *>;
using ValueToValueStableMap = MapVector<Value *, Value *>;
using AllocaMapType = DenseMap<std::pair<Function *, Type *>, Value *>;
using ValueValuePair = std::pair<Value *, Value *>;
// Stack is needed here to implement iterative DFS through global channel uses
// The second argument defines underlying container for stack: by default it is
// a std::deque, let's change it to std::vector since the last one has less
// overheads on accessing elements
using WorkListType = std::stack<ValueValuePair, std::vector<ValueValuePair>>;

extern unsigned DPCPPChannelDepthEmulationMode;

class ChannelPipeTransformationLegacy : public ModulePass {
public:
  static char ID;
  ChannelPipeTransformationLegacy() : ModulePass(ID) {
    llvm::initializeChannelPipeTransformationLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  virtual StringRef getPassName() const override {
    return "ChannelPipeTransformationLegacy";
  }

  bool runOnModule(llvm::Module &M) override;

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

private:
  ChannelPipeTransformationPass Impl;
};

char ChannelPipeTransformationLegacy::ID = 0;
INITIALIZE_PASS_BEGIN(ChannelPipeTransformationLegacy, DEBUG_TYPE,
                      "Transform Intel FPGA channels into OpenCL 2.0 pipes",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_END(ChannelPipeTransformationLegacy, DEBUG_TYPE,
                    "Transform Intel FPGA channels into OpenCL 2.0 pipes",
                    false, false)

static bool isGlobalChannel(const GlobalValue *GV, const Type *ChannelTy) {
  auto *GVValueTy = GV->getValueType();

  if (ChannelTy == GVValueTy)
    return true;

  if (auto *GVArrTy = dyn_cast<ArrayType>(GVValueTy))
    if (ChannelTy == getArrayElementType(GVArrTy))
      return true;

  return false;
}

static GlobalVariable *createGlobalPipeScalar(Module &M, Type *PipeTy,
                                              const Twine &Name) {
  auto *PipeGV = new GlobalVariable(
      M, PipeTy, /*isConstant=*/false, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, Name, /*InsertBefore=*/nullptr,
      GlobalValue::ThreadLocalMode::NotThreadLocal, ADDRESS_SPACE_GLOBAL);

  PipeGV->setInitializer(ConstantPointerNull::get(cast<PointerType>(PipeTy)));
  PipeGV->setAlignment(M.getDataLayout().getPreferredAlign(PipeGV));
  return PipeGV;
}

static GlobalVariable *createGlobalPipeArray(Module &M, Type *PipeTy,
                                             ArrayRef<size_t> Dimensions,
                                             const Twine &Name) {
  auto *PipeArrayTy = createMultiDimArray(PipeTy, Dimensions);
  assert(PipeArrayTy && "Is Dimensions invalid? Size of Dimensions must greater than 0.");

  auto *PipeGV = new GlobalVariable(
      M, PipeArrayTy, /*isConstant=*/false, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, Name, /*InsertBefore=*/nullptr,
      GlobalValue::ThreadLocalMode::NotThreadLocal, ADDRESS_SPACE_GLOBAL);

  PipeGV->setInitializer(ConstantAggregateZero::get(PipeArrayTy));
  PipeGV->setAlignment(M.getDataLayout().getPreferredAlign(PipeGV));
  return PipeGV;
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
                                            Value *BS, Value *PipeArrayGlobal,
                                            const ChannelPipeMD &PipeMD) {
  auto *PipePtrArrayTy =
      cast<ArrayType>(cast<GlobalVariable>(PipeArrayGlobal)->getValueType());
  auto *PipePtrTy = getArrayElementType(PipePtrArrayTy);
  auto *PipePtrPtrTy = PointerType::get(PipePtrTy, ADDRESS_SPACE_GLOBAL);

  SmallVector<size_t, 8> Dimensions;
  getArrayTypeDimensions(PipePtrArrayTy, Dimensions);

  Type *BSIndexTy = Type::getIntNTy(
      M.getContext(),
      M.getDataLayout().getPointerSizeInBits(ADDRESS_SPACE_GLOBAL));
  Value *ZeroConstantInt = ConstantInt::get(BSIndexTy, 0);

  size_t DimensionsNum = Dimensions.size();
  SmallVector<size_t, 8> IndicesListForPipeElem(DimensionsNum, 0);
  SmallVector<Value *, 8> GEPIndicesListForPipeElem(DimensionsNum + 1, 0);

  size_t BSItemSize = OpenCLInterface::__pipe_get_total_size_fpga(
      PipeMD.PacketSize, PipeMD.Depth, DPCPPChannelDepthEmulationMode);
  size_t BSItemsCount = getNumElementsOfNestedArray(PipePtrArrayTy);

  // iterate over all elements from backing store
  for (size_t I = 0; I < BSItemsCount; ++I) {
    // create GEP from backing store
    Value *IndexListForBSElem[] = {ZeroConstantInt,
                                   ConstantInt::get(BSIndexTy, I * BSItemSize)};
    Value *BSElemPtr = Builder.CreateGEP(
        BS->getType()->getScalarType()->getPointerElementType(), BS,
        ArrayRef<Value *>(IndexListForBSElem, 2));

    // convert current indices list to GEP indices
    convertToGEPIndicesList(IndicesListForPipeElem,
                            GEPIndicesListForPipeElem, M);
    // increment current indices
    incrementIndicesList(IndicesListForPipeElem, Dimensions);

    // create GEP from pipe array
    Value *PipeElemPtr =
        Builder.CreateGEP(PipePtrArrayTy, PipeArrayGlobal,
                          ArrayRef<Value *>(GEPIndicesListForPipeElem));
    Builder.CreateStore(Builder.CreateBitCast(BSElemPtr, PipePtrTy),
                        Builder.CreateBitCast(PipeElemPtr, PipePtrPtrTy));
  }
}

static void initializeGlobalPipeArray(GlobalVariable *PipeGV,
                                      const ChannelPipeMD &MD,
                                      Function *GlobalCtor,
                                      Function *PipeInitArray) {
  auto *BS = DPCPPChannelPipeUtils::createPipeBackingStore(PipeGV, MD);

  IRBuilder<> Builder(GlobalCtor->getEntryBlock().getTerminator());

  Value *PacketSize = Builder.getInt32(MD.PacketSize);
  Value *Depth = Builder.getInt32(MD.Depth);

  // TODO: seems to be a good place to rewrite. Generate loops in IR instead of
  // generating a huge bunch of GEP instructions
  generateBSItemsToPipeArrayStores(
      *(PipeGV->getParent()), Builder, BS, PipeGV, MD);

  ArrayType *PipePtrArrayTy = cast<ArrayType>(PipeGV->getValueType());

  size_t BSNumItems = getNumElementsOfNestedArray(PipePtrArrayTy);
  Value *Mode = Builder.getInt32(DPCPPChannelDepthEmulationMode);

  Value *CallArgs[] = {
      Builder.CreateBitCast(PipeGV,
                            PipeInitArray->getFunctionType()->getParamType(0)),
      Builder.getInt32(BSNumItems), PacketSize, Depth, Mode
  };
  Builder.CreateCall(PipeInitArray, CallArgs);
}

static bool replaceGlobalChannels(Module &M, Type *ChannelTy, Type *PipeTy,
                                  ValueToValueStableMap &VMap,
                                  RuntimeService &RTS) {
  bool Changed = false;
  Function *GlobalCtor = nullptr;
  for (auto &ChannelGV : M.globals()) {
    if (!isGlobalChannel(&ChannelGV, ChannelTy))
      continue;

    if (!GlobalCtor)
      GlobalCtor = createPipeGlobalCtor(M);

    ChannelPipeMD MD =
        getChannelPipeMetadata(&ChannelGV, DPCPPChannelDepthEmulationMode);
    GlobalVariable *PipeGV = nullptr;
    if (auto *GVArrTy = dyn_cast<ArrayType>(ChannelGV.getValueType())) {
      SmallVector<size_t, 8> Dimensions;
      getArrayTypeDimensions(GVArrTy, Dimensions);

      PipeGV = createGlobalPipeArray(M, PipeTy, Dimensions,
                                     ChannelGV.getName() + ".pipe");
      Function *PipeInitFunc = importFunctionDecl(
          &M, RTS.findFunctionInBuiltinModules("__pipe_init_array_fpga"));
      initializeGlobalPipeArray(PipeGV, MD, GlobalCtor, PipeInitFunc);
    } else {
      PipeGV = createGlobalPipeScalar(M, PipeTy, ChannelGV.getName() + ".pipe");
      Function *PipeInitFunc = importFunctionDecl(
          &M, RTS.findFunctionInBuiltinModules("__pipe_init_fpga"));
      initializeGlobalPipeScalar(PipeGV, MD, GlobalCtor, PipeInitFunc);
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
    if (ChannelMD.DepthIsIgnored.hasValue()) {
      PipeGV->setMetadata(ChannelMD.DepthIsIgnored.getID(),
          ChannelGV.getMetadata(ChannelMD.DepthIsIgnored.getID()));
    }

    VMap[&ChannelGV] = PipeGV;

    // ChannelGV is replaced with PipeGV. We set ChannelGV linkage to internal
    // so that it is eliminated later by globaldce pass.
    ChannelGV.setLinkage(GlobalValue::InternalLinkage);

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
    return Folder.FoldGEP(PipeTy->getPointerElementType(), cast<Constant>(Pipe),
                          IdxList);
  }

  // Temporary constant PtrToInt operator that uses channels may be created in
  // InstCombine pass. It has no users now, but processing of this case is added
  // for the future.
  if (isa<Constant>(ChannelUser)) {
    auto *CE = cast<ConstantExpr>(ChannelUser);
    assert(CE->getOpcode() == Instruction::PtrToInt &&
           "Unexpected constant value with channel variable");

    ConstantFolder Folder;
    return Folder.CreatePtrToInt(cast<Constant>(Pipe), CE->getType());
  }

  auto *ChannelInst = cast<Instruction>(ChannelUser);
  IRBuilder<> Builder(ChannelInst);

  if (auto *Alloca = dyn_cast<AllocaInst>(ChannelInst))
    return Builder.CreateAlloca(PipeTy, Alloca->getType()->getAddressSpace(),
                                Alloca->getArraySize(),
                                "pipe." + Alloca->getName());

  if (auto *Store = dyn_cast<StoreInst>(ChannelInst)) {
    Value *DestPtr = UndefValue::get(PointerType::getUnqual(PipeTy));
    auto *NewStore = Builder.CreateAlignedStore(
        UndefPipe, DestPtr, Store->getAlign(), Store->isVolatile());
    NewStore->setOrdering(Store->getOrdering());
    NewStore->setSyncScopeID(Store->getSyncScopeID());
    return NewStore;
  }

  if (auto *GEP = dyn_cast<GetElementPtrInst>(ChannelInst)) {
    SmallVector<Value *, 8> IdxList(GEP->idx_begin(), GEP->idx_end());
    return Builder.CreateGEP(PipeTy->getPointerElementType(), UndefPipe, IdxList,
                      ChannelInst->getName());
  }

  if (auto *Load = dyn_cast<LoadInst>(ChannelInst))
    return Builder.CreateLoad(PipeTy->getPointerElementType(), UndefPipe,
                              Load->isVolatile(), Load->getName());

  if (auto *Select = dyn_cast<SelectInst>(ChannelInst))
    return Builder.CreateSelect(Select->getCondition(), UndefPipe, UndefPipe,
                                ChannelInst->getName());

  if (auto *Phi = dyn_cast<PHINode>(ChannelInst)) {
    auto *NewPhi = Builder.CreatePHI(PipeTy, Phi->getNumIncomingValues(),
                                     ChannelInst->getName());
    for (auto *BB : Phi->blocks())
      NewPhi->addIncoming(UndefPipe, BB);

    return NewPhi;
  }

  llvm_unreachable("Unsupported instruction.");
}

static CallInst *createCallInstStub(CallInst *Call, Function *Func,
                                    Type *ChannelTy, Type *PipeTy) {
  auto *UndefPipe = UndefValue::get(PipeTy);
  SmallVector<Value *, 8> Args;
  for (auto &Arg : Call->args()) {
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

static Value *getPacketPtr(Module &M, CallInst *ChannelCall,
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
    assert(cast<PointerType>(Packet->getType())
               ->getPointerElementType()->isStructTy() &&
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

  if (!CK.Blocking) {
    // Channel and pipe built-ins have different (inverted) meaning for success
    // and failure.
    auto *BoolRet = Builder.CreateICmpEQ(PipeBoolRet, Builder.getInt32(0));
    if (CK.Access == ChannelKind::READ) {
      // <value ty> read_channel_nb(channel, bool*)
      // Struct is passed by pointer as a first arugment.
      // void read_channel_nb(<struct ty> *, channel, bool*)
      if (!UsesStruct)
        ChannelCall->replaceAllUsesWith(Builder.CreateLoad(
            PipePacketPtr->getType()->getPointerElementType(), PipePacketPtr));

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
      ChannelCall->replaceAllUsesWith(Builder.CreateLoad(
          PipePacketPtr->getType()->getPointerElementType(), PipePacketPtr));
    }
    // Struct is passed by pointer as a first arugment. No return value,
    // nothing to do
    // void read_channel(<struct ty> *ret, channel)
  }

  // Write built-ins don't have return value, nothing to do
  // void write_channel(channel, <value ty>)
}

static void replaceChannelBuiltinCall(Module &M, CallInst *ChannelCall,
                                      Value *GlobalPipe, Value *Pipe,
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

  auto PipeMD = GlobalVariableMetadataAPI(cast<GlobalVariable>(GlobalPipe));

  IRBuilder<> Builder(ChannelCall);
  Value *Args[] = {
      // %opencl.pipe_rw_t to %opencl.pipe_ro_t/%opencl.pipe_wo_t
      Builder.CreatePointerCast(Pipe, FTy->getParamType(0), ""),
      Builder.CreatePointerCast(PacketPtr, FTy->getParamType(1), ""),
      ConstantInt::get(FTy->getParamType(2), PipeMD.PipePacketSize.get()),
      ConstantInt::get(FTy->getParamType(3), PipeMD.PipePacketAlign.get())
  };

  CallInst *BoolRet = Builder.CreateCall(Builtin, Args, ChannelCall->getName());

  replaceChannelCallResult(ChannelCall, CK, PacketPtr, BoolRet);
}

static bool isChannelBuiltinCall(CallInst *Call) {
  Function *CalledFunction = Call->getCalledFunction();
  assert(CalledFunction && "Indirect function call?");

  if (ChannelKind Kind = getChannelKind(CalledFunction->getName()))
    return true;

  return false;
}

static Function *createUserFunctionStub(CallInst *Call, Type *ChannelTy,
                                             Type *PipeTy) {
  Function *ExistingF = Call->getCalledFunction();
  assert(ExistingF && "Indirect function call?");

  FunctionType *ExistingFTy = ExistingF->getFunctionType();
  SmallVector<Type *, 4> ArgTys(ExistingFTy->param_begin(),
                                ExistingFTy->param_end());
  for (auto &ArgTy : ArgTys)
    if (ArgTy == ChannelTy)
      ArgTy = PipeTy;

  FunctionType *NewFTy = FunctionType::get(ExistingFTy->getReturnType(), ArgTys,
                                           ExistingFTy->isVarArg());
  Function *Replacement =
      Function::Create(NewFTy, ExistingF->getLinkage(),
                       "pipe." + ExistingF->getName(), ExistingF->getParent());

  ValueToValueMapTy VMap;
  auto *ExistFArgIt = ExistingF->arg_begin();
  auto *RArgIt = Replacement->arg_begin();
  auto *RArgItE = Replacement->arg_end();
  for (; RArgIt != RArgItE; ++RArgIt, ++ExistFArgIt)
    VMap[&*ExistFArgIt] = &*RArgIt;

  SmallVector<ReturnInst *, 4> Returns;
  CloneFunctionInto(Replacement, ExistingF, VMap,
                    CloneFunctionChangeType::LocalChangesOnly, Returns, "");
  assert(Replacement && "CloneFunctionInfo failed");

  ExistingF->deleteBody();

  return Replacement;
}

static Argument *getFunctionArg(Function *F, const unsigned ArgNo) {
  assert(ArgNo < F->arg_size() && "Invalid ArgNo");
  return F->arg_begin() + ArgNo;
}

static void replaceLocalChannelUses(Function *UserFunc, Type * /*ChannelTy*/,
                                    Type *PipeTy, const unsigned ArgNo,
                                    ::ValueToValueMap &VMap,
                                    SmallPtrSetImpl<Instruction *> &ToDelete,
                                    WorkListType &WorkList) {
  Argument *Arg = getFunctionArg(UserFunc, ArgNo);
  assert(Arg->getType() == PipeTy && "No appropriate function argument found");

  // It is possible that replaceLocalChannelUses will be called several times
  // for the same UserFunc and ArgNo, e.g. user-defined function were called
  // several times
  //
  // It is enough to replace argument only once
  if (VMap.count(Arg))
    return;

  for (auto *ArgUser : Arg->users()) {
    if (auto *Store = dyn_cast<StoreInst>(ArgUser)) {
      Value *POperand = Store->getPointerOperand();
      assert(isa<AllocaInst>(POperand) && "Expected alloca for argument");
      // Let's do an initial replacement of alloca
      auto *&PipeUser = VMap[POperand];
      if (!PipeUser) {
        PipeUser = createPipeUserStub(POperand, Arg);
        WorkList.push(std::make_pair(POperand, PipeUser));
      }
      ToDelete.insert(Store);
    }
  }

  WorkList.push(std::make_pair(Arg, Arg));
  VMap[Arg] = Arg;
}

void findUsesToDelete(Function *F, SetVector<Function *> &UsesToDelete) {
  for (User *U : F->users()) {
    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
      if (!Inst->getFunction()->use_empty())
        findUsesToDelete(Inst->getFunction(), UsesToDelete);
      UsesToDelete.insert(Inst->getFunction());
    }
  }
}

static void cleanup(Module &M, SmallPtrSetImpl<Instruction *> &ToDelete,
                    ::ValueToValueMap &VMap) {
  for (auto *I : ToDelete) {
    if (!I->use_empty()) {
      assert(isa<CallInst>(I) && "Expected that only calls to user functions "
                                 "still has users");
      I->replaceAllUsesWith(VMap[I]);
    }

    I->eraseFromParent();

    // Need to erase the instruction from VMap to avoid flaky failures since it
    // might be polluted after it's deleted.
    VMap.erase(I);
  }

  SetVector <Function *> UsesToDelete;
  for (auto It : VMap) {
    if (Function *F = dyn_cast<Function>(It.first)) {
      Function *R = cast<Function>(It.second);
      findUsesToDelete(F, UsesToDelete);
      R->takeName(F);
      UsesToDelete.insert(F);
    }
  }

  for (Function *F : UsesToDelete)
    F->eraseFromParent();

  // remove channel built-ins declarations
  SmallVector<Function *, 8> FToDelete;
  for (auto &F : M)
    if (F.isDeclaration() && getChannelKind(F.getName()))
      FToDelete.push_back(&F);

  for (auto &F : FToDelete) {
    assert(F->use_empty() && "Users of channel built-in are still exists");
    F->eraseFromParent();
  }
}

static void replaceGlobalChannelUses(Module &M, Type *ChannelTy,
                                     ValueToValueStableMap &GlobalVMap,
                                     RuntimeService &RTS) {
  ::ValueToValueMap VMap;
  SmallPtrSet<Instruction *, 32> ToDelete;
  // See comments about WorkListType typedef for explanations
  WorkListType WorkList;
  // This map is used to cache temporary alloca instuctions used by pipe
  // built-ins
  AllocaMapType AllocaMap;

  for (const auto &KV : GlobalVMap)
    WorkList.push(std::make_pair(KV.first, KV.second));

  Value *GlobalPipe = nullptr;

  while (!WorkList.empty()) {
    Value *Channel = WorkList.top().first;
    Value *Pipe = WorkList.top().second;
    WorkList.pop();

    if (GlobalVMap.count(Channel))
      GlobalPipe = Pipe;

    for (const auto &U : Channel->uses()) {
      auto OpNo = U.getOperandNo();
      Value *ChannelUser = U.getUser();

      if (CallInst *Call = dyn_cast<CallInst>(ChannelUser)) {
        ToDelete.insert(Call);
        if (isChannelBuiltinCall(Call)) {
          replaceChannelBuiltinCall(M, Call, GlobalPipe, Pipe, AllocaMap, RTS);
        } else {
          // handle calls to user-functions here
          assert(Call->getCalledFunction() && "Indirect function call?");
          Value *&FuncReplacementIt = VMap[Call->getCalledFunction()];
          // FuncReplacementIt must be used only to save result of
          // createUserFunctionStub. Further usages may result in segfaults due
          // to invalid value of FuncReplacement. This may occur if VMap is
          // re-allocated: all references to it's keys is going to be
          // invalidated.
          Value *FuncReplacement = FuncReplacementIt;
          if (!FuncReplacement)
            FuncReplacement = FuncReplacementIt =
                createUserFunctionStub(Call, ChannelTy, Pipe->getType());

          replaceLocalChannelUses(cast<Function>(FuncReplacement), ChannelTy,
                                  Pipe->getType(), OpNo, VMap, ToDelete,
                                  WorkList);

          Value *&CallInstReplacement = VMap[Call];
          if (!CallInstReplacement)
            CallInstReplacement =
                createCallInstStub(Call, cast<Function>(FuncReplacement),
                                   ChannelTy, Pipe->getType());

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
      if ((!isa<GEPOperator>(PipeUser) || isa<GetElementPtrInst>(PipeUser)) &&
          !isa<Constant>(PipeUser))
        cast<User>(PipeUser)->setOperand(OpNo, Pipe);
    }
  }

  cleanup(M, ToDelete, VMap);
}

bool ChannelPipeTransformationLegacy::runOnModule(Module &M) {
  BuiltinLibInfo *BLI =
      &getAnalysis<BuiltinLibInfoAnalysisLegacy>().getResult();
  return Impl.runImpl(M, BLI);
}

PreservedAnalyses
ChannelPipeTransformationPass::run(Module &M, ModuleAnalysisManager &MAM) {
  BuiltinLibInfo *BLI = &MAM.getResult<BuiltinLibInfoAnalysis>(M);
  return runImpl(M, BLI) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool ChannelPipeTransformationPass::runImpl(Module &M, BuiltinLibInfo *BLI) {
  RuntimeService &RTS = BLI->getRuntimeService();
  auto *ChannelValueTy = getStructByName("opencl.channel_t", &M);
  if (!ChannelValueTy)
    return false;

  auto *ChannelTy = PointerType::get(ChannelValueTy, ADDRESS_SPACE_GLOBAL);
  auto *PipeTyName = "opencl.pipe_rw_t";
  auto *PipeValueTy = StructType::getTypeByName(M.getContext(), PipeTyName);
  if (!PipeValueTy)
    PipeValueTy = StructType::create(M.getContext(), PipeTyName);
  auto *PipeTy = PointerType::get(PipeValueTy, ADDRESS_SPACE_GLOBAL);

  ValueToValueStableMap GlobalVMap;
  if (!replaceGlobalChannels(M, ChannelTy, PipeTy, GlobalVMap, RTS))
    return false;
  replaceGlobalChannelUses(M, ChannelTy, GlobalVMap, RTS);

  return true;
}

void ChannelPipeTransformationLegacy::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
}

ModulePass *llvm::createChannelPipeTransformationLegacyPass() {
  return new ChannelPipeTransformationLegacy();
}
