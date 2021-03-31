//===- ImplicitArgsAnalysis.cpp - DPC++ kernel implicit argument analysis -===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ImplicitArgsAnalysis.h"
#include "ImplicitArgsUtils.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DevLimits.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"

using namespace llvm;

static std::string AppendWithDimension(const Twine &S, const Value *Dimension) {
  if (const ConstantInt *C = dyn_cast<ConstantInt>(Dimension))
    return (S + Twine(C->getZExtValue())).str();
  else
    return (S + Twine("var")).str();
}

ImplicitArgsInfo::ImplicitArgsInfo(Module &M)
    : ArgTypes(ImplicitArgsUtils::IA_NUMBER, 0),
      WGInfoMembersTypes(NDInfo::LAST, 0), InitializedTo(0) {
  init(&M.getContext(), M.getDataLayout().getPointerSizeInBits());
}

ImplicitArgsInfo::~ImplicitArgsInfo() {}

void ImplicitArgsInfo::init(LLVMContext *LC, unsigned PointerSizeInBits) {
  C = LC;
  assert(PointerSizeInBits == 32 || PointerSizeInBits == 64);
  if (InitializedTo == PointerSizeInBits)
    return;
  InitializedTo = PointerSizeInBits;
  IntegerType *SizetTy = IntegerType::get(*C, PointerSizeInBits);
  PointerType *SizetPtrTy = PointerType::get(SizetTy, 0);
  // Create Work Group/Work Item info structures
  assert(MAX_WI_DIM_POW_OF_2 == 4 && "MAX_WI_DIM_POW_OF_2 is not equal to 4!");
  // use 4 instead of MAX_WORK_DIM for alignment & for better calculation of
  // offset in Local ID buffer
  Type *PaddedDimIdTy = ArrayType::get(SizetTy, MAX_WI_DIM_POW_OF_2);

  Type *AbstractPtr = PointerType::get(StructType::get(*C), 0);
  Type *Sizet3Ty = ArrayType::get(SizetTy, MAX_WORK_DIM);
  Type *Sizet2x3Ty =
      ArrayType::get(ArrayType::get(SizetTy, MAX_WORK_DIM), WG_SIZE_NUM);
  WGInfoMembersTypes[NDInfo::WORK_DIM] = SizetTy;
  WGInfoMembersTypes[NDInfo::GLOBAL_OFFSET] = Sizet3Ty;
  WGInfoMembersTypes[NDInfo::GLOBAL_SIZE] = Sizet3Ty;
  WGInfoMembersTypes[NDInfo::LOCAL_SIZE] = Sizet2x3Ty;
  WGInfoMembersTypes[NDInfo::WG_NUMBER] = Sizet3Ty;
  WGInfoMembersTypes[NDInfo::RUNTIME_INTERFACE] = AbstractPtr;
  WGInfoMembersTypes[NDInfo::BLOCK2KERNEL_MAPPER] = AbstractPtr;
  assert(NDInfo::BLOCK2KERNEL_MAPPER + 1 == NDInfo::LAST);
  // Initialize the implicit argument types
  ArgTypes[ImplicitArgsUtils::IA_SLM_BUFFER] =
      PointerType::get(IntegerType::get(*C, 8), 3);
  ArgTypes[ImplicitArgsUtils::IA_WORK_GROUP_INFO] =
      PointerType::get(StructType::get(*C, WGInfoMembersTypes, false), 0);
  ArgTypes[ImplicitArgsUtils::IA_WORK_GROUP_ID] = SizetPtrTy;
  ArgTypes[ImplicitArgsUtils::IA_GLOBAL_BASE_ID] = PaddedDimIdTy;
  ArgTypes[ImplicitArgsUtils::IA_BARRIER_BUFFER] =
      PointerType::get(IntegerType::get(*C, 8), 0);
  ArgTypes[ImplicitArgsUtils::IA_RUNTIME_HANDLE] =
      PointerType::get(StructType::get(*C), 0);
  assert(ImplicitArgsUtils::IA_RUNTIME_HANDLE + 1 ==
         ImplicitArgsUtils::IA_NUMBER);
}

Value *ImplicitArgsInfo::GenerateGetFromWorkInfo(unsigned RecordID,
                                                 Value *WorkInfo,
                                                 unsigned Dimension,
                                                 IRBuilder<> &Builder) {
  return GenerateGetFromWorkInfo(
      RecordID, WorkInfo, ConstantInt::get(IntegerType::get(*C, 32), Dimension),
      Builder);
}
Value *ImplicitArgsInfo::GenerateGetFromWorkInfo(unsigned RecordID,
                                                 Value *WorkInfo,
                                                 Value *Dimension,
                                                 IRBuilder<> &Builder) {
  assert(RecordID < NDInfo::LAST && "Invalid value for RecordID");
  SmallVector<Value *, 4> Params;
  Params.push_back(ConstantInt::get(Type::getInt32Ty(*C), 0));
  Params.push_back(ConstantInt::get(Type::getInt32Ty(*C), RecordID));
  Params.push_back(Dimension);
  auto *Addr =
      cast<GEPOperator>(Builder.CreateGEP(WorkInfo, ArrayRef<Value *>(Params)));
  StringRef Name(NDInfo::getRecordName(RecordID));
  std::string NameWithDim = AppendWithDimension(Name, Dimension);
  return Builder.Insert(new LoadInst(Addr->getResultElementType(), Addr, "",
                                     /*volatile*/ false, Align()),
                        NameWithDim);
}
Value *ImplicitArgsInfo::GenerateGetFromWorkInfo(unsigned RecordID,
                                                 Value *WorkInfo,
                                                 IRBuilder<> &Builder) {
  assert(RecordID < NDInfo::LAST && "Invalid value for RecordID");
  SmallVector<Value *, 4> Params;
  Type *Int32Ty = Type::getInt32Ty(*C);
  Params.push_back(ConstantInt::get(Int32Ty, 0));
  Params.push_back(ConstantInt::get(Int32Ty, RecordID));
  auto *Addr =
      cast<GEPOperator>(Builder.CreateGEP(WorkInfo, ArrayRef<Value *>(Params)));
  StringRef Name(NDInfo::getRecordName(RecordID));
  Value *V = Builder.Insert(new LoadInst(Addr->getResultElementType(), Addr, "",
                                         /*volatile*/ false, Align()));
  if (V->getType() != Int32Ty && RecordID == NDInfo::WORK_DIM)
    V = Builder.CreateTrunc(V, Int32Ty);
  V->setName(Name);
  return V;
}
Value *ImplicitArgsInfo::GenerateGetGlobalOffset(Value *WorkInfo,
                                                 unsigned Dimension,
                                                 IRBuilder<> &Builder) {
  return GenerateGetGlobalOffset(
      WorkInfo, ConstantInt::get(IntegerType::get(*C, 32), Dimension), Builder);
}
Value *ImplicitArgsInfo::GenerateGetGlobalOffset(Value *WorkInfo,
                                                 Value *Dimension,
                                                 IRBuilder<> &Builder) {
  return GenerateGetFromWorkInfo(NDInfo::GLOBAL_OFFSET, WorkInfo, Dimension,
                                 Builder);
}

Value *ImplicitArgsInfo::GenerateGetLocalSize(bool uniformWGSize,
                                              Value *WorkInfo, Value *pWGId,
                                              Value *Dimension,
                                              IRBuilder<> &Builder) {

  // The OpenCL 2.0 program can be compiled with the option
  // -cl-uniform-work-group-size.
  if (uniformWGSize) {
    return GenerateGetEnqueuedLocalSize(WorkInfo, Dimension, Builder);
  }

  // The non-uniform local size is stored as the second array so
  // get_local_size must return value from it in case it is the last WG
  // in the specified dimension.
  // %WGIdPlusOne = add nsw i64 %WGId, 1
  // %LastWG = icmp eq i64 %WGIdPlusOne, %WGNum
  // %LocalSizeIdx = zext i1 %2 to i32
  Value *WGNum =
      GenerateGetFromWorkInfo(NDInfo::WG_NUMBER, WorkInfo, Dimension, Builder);
  Value *WGId = GenerateGetGroupID(pWGId, Dimension, Builder);
  Value *WGIdPlusOne =
      Builder.CreateNSWAdd(WGId, ConstantInt::get(WGId->getType(), 1));
  Value *LastWG = Builder.CreateICmpEQ(WGNum, WGIdPlusOne);
  Value *LocalSizeIdx = Builder.CreateZExt(LastWG, IntegerType::get(*C, 32));

  return GenerateGetLocalSizeGeneric(WorkInfo, LocalSizeIdx, Dimension,
                                     Builder);
}

Value *ImplicitArgsInfo::GenerateGetEnqueuedLocalSize(Value *WorkInfo,
                                                      unsigned Dimension,
                                                      IRBuilder<> &Builder) {
  // the uniform local size is stored as the first array
  return GenerateGetEnqueuedLocalSize(
      WorkInfo, ConstantInt::get(IntegerType::get(*C, 32), Dimension), Builder);
}

Value *ImplicitArgsInfo::GenerateGetEnqueuedLocalSize(Value *WorkInfo,
                                                      Value *Dimension,
                                                      IRBuilder<> &Builder) {
  // the uniform local size is stored as the first array
  return GenerateGetLocalSizeGeneric(
      WorkInfo, ConstantInt::get(IntegerType::get(*C, 32), 0), Dimension,
      Builder);
}

// the following implementation is generic for get_local_size and
// get_enqueued_local_size
Value *ImplicitArgsInfo::GenerateGetLocalSizeGeneric(Value *WorkInfo,
                                                     Value *LocalSizeIdx,
                                                     Value *Dimension,
                                                     IRBuilder<> &Builder) {
  SmallVector<Value *, 4> Params;
  Params.push_back(ConstantInt::get(Type::getInt32Ty(*C), 0));
  Params.push_back(ConstantInt::get(Type::getInt32Ty(*C), NDInfo::LOCAL_SIZE));
  Params.push_back(LocalSizeIdx);
  Params.push_back(Dimension);
  auto *Addr =
      cast<GEPOperator>(Builder.CreateGEP(WorkInfo, ArrayRef<Value *>(Params)));
  StringRef Name(NDInfo::getRecordName(NDInfo::LOCAL_SIZE));
  std::string NameWithDim = AppendWithDimension(Name, Dimension);
  return Builder.Insert(new LoadInst(Addr->getResultElementType(), Addr, "",
                                     /*volatile*/ false, Align()),
                        NameWithDim);
}

Value *ImplicitArgsInfo::GenerateGetGroupID(Value *GroupID, unsigned Dimension,
                                            IRBuilder<> &Builder) {
  return GenerateGetGroupID(
      GroupID, ConstantInt::get(IntegerType::get(*C, 32), Dimension), Builder);
}
Value *ImplicitArgsInfo::GenerateGetGroupID(Value *GroupID, Value *Dimension,
                                            IRBuilder<> &Builder) {
  auto *IdAddr = cast<GEPOperator>(Builder.CreateGEP(GroupID, Dimension));
  StringRef Name("GroupID_");
  std::string NameWithDim = AppendWithDimension(Name, Dimension);
  return Builder.Insert(new LoadInst(IdAddr->getResultElementType(), IdAddr, "",
                                     /*volatile*/ false, Align()),
                        NameWithDim);
}

Value *ImplicitArgsInfo::GenerateGetBaseGlobalID(Value *BaseGlobalID,
                                                 Value *Dimension,
                                                 IRBuilder<> &Builder) {
  StringRef Name("BaseGlobalID_");
  std::string NameWithDim = AppendWithDimension(Name, Dimension);
  if (ConstantInt *Dim = dyn_cast<ConstantInt>(Dimension)) {
    unsigned D = Dim->getZExtValue();
    return Builder.CreateExtractValue(BaseGlobalID, ArrayRef<unsigned>(D),
                                      NameWithDim);
  } else {
    // Cannot create an 'extractvalue' with non-const index, so need to pass
    // thru memory
    IRBuilder<> AllocaBuilder(
        &*Builder.GetInsertBlock()->getParent()->begin()->begin());
    AllocaInst *A = AllocaBuilder.CreateAlloca(BaseGlobalID->getType(), 0,
                                               "alloc_BaseGlobalID");
    Builder.CreateStore(BaseGlobalID, A);
    std::vector<Value *> Indices;
    Indices.push_back(ConstantInt::get(IntegerType::get(*C, 32), 0));
    Indices.push_back(Dimension);
    auto *GEP = cast<GEPOperator>(Builder.CreateGEP(A, Indices));
    return Builder.Insert(new LoadInst(GEP->getResultElementType(), GEP, "",
                                       /*volatile*/ false, Align()),
                          NameWithDim);
  }
}

// Provide a definition for the static class member used to identify passes.
AnalysisKey ImplicitArgsAnalysis::Key;

ImplicitArgsInfo ImplicitArgsAnalysis::run(Module &M,
                                           AnalysisManager<Module> &AM) {
  (void)AM;
  ImplicitArgsInfo IAResult(M);

  return IAResult;
}

INITIALIZE_PASS(ImplicitArgsAnalysisLegacy,
                "dpcpp-kernel-implicit-args-analysis",
                "Implicit arguments analysis", false, true)

char ImplicitArgsAnalysisLegacy::ID = 0;

ImplicitArgsAnalysisLegacy::ImplicitArgsAnalysisLegacy() : ModulePass(ID) {}

bool ImplicitArgsAnalysisLegacy::runOnModule(Module &M) {
  Result.reset(new ImplicitArgsInfo(M));
  return false;
}

ModulePass *llvm::createImplicitArgsAnalysisLegacyPass() {
  return new ImplicitArgsAnalysisLegacy();
}
