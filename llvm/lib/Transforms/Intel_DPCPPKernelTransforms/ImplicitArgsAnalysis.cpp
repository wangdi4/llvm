//===- ImplicitArgsAnalysis.cpp - DPC++ kernel implicit argument analysis -===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ImplicitArgsAnalysis.h"
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
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/ImplicitArgsUtils.h"

using namespace llvm;
using namespace llvm::CompilationUtils;

ImplicitArgsInfo::ImplicitArgsInfo(Module &M)
    : ArgTypes(ImplicitArgsUtils::IA_NUMBER, 0),
      WGInfoMembersTypes(NDInfo::LAST, 0), M(&M), InitializedTo(0) {
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
  WGInfoMembersTypes[NDInfo::INTERNAL_GLOBAL_SIZE] = Sizet3Ty;
  WGInfoMembersTypes[NDInfo::INTERNAL_LOCAL_SIZE] = Sizet2x3Ty;
  WGInfoMembersTypes[NDInfo::INTERNAL_WG_NUMBER] = Sizet3Ty;
  assert(NDInfo::INTERNAL_WG_NUMBER + 1 == NDInfo::LAST);
  // Initialize the implicit argument types
  ArgTypes[ImplicitArgsUtils::IA_SLM_BUFFER] =
      PointerType::get(CompilationUtils::getSLMBufferElementType(*C), 3);
  ArgTypes[ImplicitArgsUtils::IA_WORK_GROUP_INFO] = PointerType::get(
      CompilationUtils::getWorkGroupInfoElementType(*C, WGInfoMembersTypes), 0);
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
  auto *Addr = cast<GEPOperator>(Builder.CreateGEP(
      CompilationUtils::getWorkGroupInfoElementType(*C, WGInfoMembersTypes),
      WorkInfo, ArrayRef<Value *>(Params)));
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
  auto *Addr = cast<GEPOperator>(Builder.CreateGEP(
      CompilationUtils::getWorkGroupInfoElementType(*C, WGInfoMembersTypes),
      WorkInfo, ArrayRef<Value *>(Params)));
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
                                              bool IsUserRequired,
                                              Value *Dimension,
                                              IRBuilder<> &Builder) {

  // The OpenCL 2.0 program can be compiled with the option
  // -cl-uniform-work-group-size.
  if (uniformWGSize) {
    return GenerateGetEnqueuedLocalSize(WorkInfo, IsUserRequired, Dimension,
                                        Builder);
  }

  // The non-uniform local size is stored as the second array so
  // get_local_size must return value from it in case it is the last WG
  // in the specified dimension.
  // %WGIdPlusOne = add nsw i64 %WGId, 1
  // %LastWG = icmp eq i64 %WGIdPlusOne, %WGNum
  // %LocalSizeIdx = zext i1 %2 to i32
  Value *WGNum = GenerateGetFromWorkInfo(
      IsUserRequired ? NDInfo::WG_NUMBER : NDInfo::INTERNAL_WG_NUMBER, WorkInfo,
      Dimension, Builder);
  Value *WGId = GenerateGetGroupID(pWGId, Dimension, Builder);
  Value *WGIdPlusOne =
      Builder.CreateNSWAdd(WGId, ConstantInt::get(WGId->getType(), 1));
  Value *LastWG = Builder.CreateICmpEQ(WGNum, WGIdPlusOne);
  Value *LocalSizeIdx = Builder.CreateZExt(LastWG, IntegerType::get(*C, 32));

  return GenerateGetLocalSizeGeneric(WorkInfo, IsUserRequired, LocalSizeIdx,
                                     Dimension, Builder);
}

Value *ImplicitArgsInfo::GenerateGetEnqueuedLocalSize(Value *WorkInfo,
                                                      bool IsUserRequired,
                                                      unsigned Dimension,
                                                      IRBuilder<> &Builder) {
  // the uniform local size is stored as the first array
  return GenerateGetEnqueuedLocalSize(
      WorkInfo, IsUserRequired,
      ConstantInt::get(IntegerType::get(*C, 32), Dimension), Builder);
}

Value *ImplicitArgsInfo::GenerateGetEnqueuedLocalSize(Value *WorkInfo,
                                                      bool IsUserRequired,
                                                      Value *Dimension,
                                                      IRBuilder<> &Builder) {
  // the uniform local size is stored as the first array
  return GenerateGetLocalSizeGeneric(
      WorkInfo, IsUserRequired, ConstantInt::get(IntegerType::get(*C, 32), 0),
      Dimension, Builder);
}

// the following implementation is generic for get_local_size and
// get_enqueued_local_size
Value *ImplicitArgsInfo::GenerateGetLocalSizeGeneric(Value *WorkInfo,
                                                     bool IsUserRequired,
                                                     Value *LocalSizeIdx,
                                                     Value *Dimension,
                                                     IRBuilder<> &Builder) {
  auto RecordID =
      IsUserRequired ? NDInfo::LOCAL_SIZE : NDInfo::INTERNAL_LOCAL_SIZE;
  SmallVector<Value *, 4> Params;
  Params.push_back(ConstantInt::get(Type::getInt32Ty(*C), 0));
  Params.push_back(ConstantInt::get(Type::getInt32Ty(*C), RecordID));
  Params.push_back(LocalSizeIdx);
  Params.push_back(Dimension);
  auto *Addr = cast<GEPOperator>(Builder.CreateGEP(
      CompilationUtils::getWorkGroupInfoElementType(*C, WGInfoMembersTypes),
      WorkInfo, ArrayRef<Value *>(Params)));
  StringRef Name(NDInfo::getRecordName(RecordID));
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
  // TODO: Remove the non-opaque code when OpaquePtr is fully enabled on OpenCL.
  // Function GetElementPtrInst *Create(Type *PointeeType, Value *Ptr, ...)
  // calls isOpaqueOrPointeeTypeMatches which requires that parameter
  // PointeeType is equal to Ptr->PointeeTy when Ptr is non-opaque. It is not
  // vey reasonable to do such check, only make sure there are right type ID for
  // PointeeType will work well. So there are build errors when use
  // CompilationUtils:: getWorkGroupIDElementType(..) as PointeeType. And
  // function isOpaqueOrPointeeTypeMatches will be useless after non-opaque
  // pointers are removed.
  Type *Ty =
      GroupID->getType()->getScalarType()->isOpaquePointerTy()
          ? CompilationUtils::getWorkGroupIDElementType(M)
          : GroupID->getType()
                ->getScalarType()
                ->getNonOpaquePointerElementType();
  auto *IdAddr = cast<GEPOperator>(Builder.CreateGEP(Ty, GroupID, Dimension));
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
    auto *GEP =
        cast<GEPOperator>(Builder.CreateGEP(A->getAllocatedType(), A, Indices));
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

ImplicitArgsAnalysisLegacy::ImplicitArgsAnalysisLegacy() : ModulePass(ID) {
  initializeImplicitArgsAnalysisLegacyPass(*PassRegistry::getPassRegistry());
}

bool ImplicitArgsAnalysisLegacy::runOnModule(Module &M) {
  Result.reset(new ImplicitArgsInfo(M));
  return false;
}

ModulePass *llvm::createImplicitArgsAnalysisLegacyPass() {
  return new ImplicitArgsAnalysisLegacy();
}
