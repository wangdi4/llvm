//==---------------- Intel_ResolveMatrixLayout.cpp -  C++ -*---------------==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/Intel_ResolveMatrixLayout.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-resolve-matrix-layout"

static CallInst *generateCall(Module *M, StringRef FnName, Type *ReturnType,
                              ArrayRef<Value *> Args, IRBuilder<> &Builder,
                              const Twine &Name = "",
                              AttributeList AttrList = AttributeList()) {
  SmallVector<Type *> ArgTypes;
  for (auto *Arg : Args)
    ArgTypes.push_back(Arg->getType());
  auto *FuncType = FunctionType::get(ReturnType, ArgTypes, false);
  auto Func = M->getOrInsertFunction(FnName, FuncType, AttrList);
  return Builder.CreateCall(Func, Args, Name);
}

static AllocaInst *createMatrixAllocaInst(Function &F, FixedVectorType *FVT) {
  Module *M = F.getParent();
  const DataLayout &DL = M->getDataLayout();

  auto AllocaAlignment = DL.getPrefTypeAlign(FVT);
  unsigned AllocaAS = DL.getAllocaAddrSpace();
  AllocaInst *AllocaRes =
      new AllocaInst(FVT, AllocaAS, "", &F.getEntryBlock().front());
  AllocaRes->setAlignment(AllocaAlignment);
  return AllocaRes;
}

static std::pair<bool, Value *> resolveMatrixLayoutLoadHelper(
    IRBuilder<> &Builder, CallInst *CI, StringRef BuiltinName,
    PointerType *PtrType, Value *NewMemL, Value *OldStride, Value *NewStride,
    bool ShouldAddWorkList, SmallVector<User *> &WorkList) {
  FixedVectorType *MatrixType = cast<FixedVectorType>(CI->getType());
  Value *MatAlloca = createMatrixAllocaInst(*CI->getFunction(), MatrixType);
  Value *Src =
      Builder.CreatePointerBitCastOrAddrSpaceCast(CI->getOperand(0), PtrType);
  Value *Dst = Builder.CreateBitCast(MatAlloca, PtrType);
  SmallVector<Value *, 5> Args = {Src, Dst, CI->getOperand(3),
                                  CI->getOperand(4), OldStride};
  generateCall(CI->getModule(), BuiltinName, Builder.getVoidTy(), Args,
               Builder);
  // stride should be calculated according to the size of the temporary matrix
  SmallVector<Value *, 8> ArgsForNewLoad = {Dst,
                                            NewStride,
                                            CI->getOperand(2),
                                            CI->getOperand(3),
                                            CI->getOperand(4),
                                            CI->getOperand(5),
                                            NewMemL,
                                            CI->getOperand(7),
                                            CI->getOperand(8)};
  SmallVector<Type *, 2> TypesForNewLoad = {MatrixType, Dst->getType()};
  CallInst *NewMatrixLoad = Builder.CreateIntrinsic(
      Intrinsic::experimental_matrix_load, TypesForNewLoad, ArgsForNewLoad);
  if (ShouldAddWorkList)
    WorkList.push_back(NewMatrixLoad);
  return std::make_pair(true, NewMatrixLoad);
}

// transform
// call void @llvm.experimental.matrix.store.v8f32.p4.v2(<8 x float>
// %0, ptr %dst,  i64 %stride, i1 false, i32 4, i32 2, metadata
// !"matrix.rowmajor", metadata !"matrix.columnmajor", metadata
// !"scope.subgroup", metadata !"matrix.use.unnecessary")
//
// into:
// %alloc = alloca [float x 8]
// %ptr = bitcast %alloc to i8*
// %stride = 2 * sizeof(float)
// %newstride =
// call void @llvm.experimental.matrix.store.v8f32.p4.v2(<8 x float>
// %0, ptr %ptr,  i64 %stride, i1 false, i32 4, i32 2, metadata
// !"matrix.rowmajor", metadata !"matrix.rowmajor", metadata !"scope.subgroup",
// metadata !"matrix.use.unnecessary")
//
// call @_Z44matrix_layout_transform_rowmajor_to_colmajorPU3AS4sS0_iii(float*
//   %ptr, float* %dst, i32 8, i32 16, i64 32)
//
// Thus NewMemL OldStride NewStride should be re-calculated
static std::pair<bool, Value *> resolveMatrixLayoutStoreHelper(
    IRBuilder<> &Builder, CallInst *CI, StringRef BuiltinName,
    PointerType *PtrType, Value *NewMemL, Value *OldStride, Value *NewStride,
    bool ShouldAddWorkList, SmallVector<User *> &WorkList) {
  FixedVectorType *MatrixType =
      cast<FixedVectorType>(CI->getOperand(0)->getType());
  Value *MatAlloca = createMatrixAllocaInst(*CI->getFunction(), MatrixType);
  Value *Ptr =
      Builder.CreatePointerBitCastOrAddrSpaceCast(CI->getOperand(1), PtrType);
  Value *Dst = Builder.CreateBitCast(MatAlloca, PtrType);

  // stride should be calculated according to the size of the temporary matrix
  // we do tilestore for rowmajor matrix and then tranpose the matrix.
  SmallVector<Value *, 8> ArgsForNewStore = {CI->getOperand(0),  // Matrix
                                             Dst,                // Dst Ptr
                                             NewStride,          // Stride
                                             CI->getOperand(3),  // IsVolatile
                                             CI->getOperand(4),  // Row
                                             CI->getOperand(5),  // Col
                                             CI->getOperand(6),  // Mat Layout
                                             NewMemL,            // Mem Layout
                                             CI->getOperand(8),  // Group
                                             CI->getOperand(9)}; // MatrixUse
  SmallVector<Type *, 2> TypesForNewStore = {MatrixType,         // Matrix
                                             Dst->getType()};    // Ptr
  CallInst *NewMatrixStore = Builder.CreateIntrinsic(
      Intrinsic::experimental_matrix_store, TypesForNewStore, ArgsForNewStore);
  SmallVector<Value *, 5> Args = {Dst,               // Src Mem
                                  Ptr,               // Dst Mem
                                  CI->getOperand(4), // Row
                                  CI->getOperand(5), // Col
                                  OldStride};        // Stride
  CallInst *BuiltinCall = generateCall(CI->getModule(), BuiltinName,
                                       Builder.getVoidTy(), Args, Builder);
  if (ShouldAddWorkList)
    WorkList.push_back(NewMatrixStore);
  return std::make_pair(true, BuiltinCall);
}

static std::pair<bool, Value *>
resolveMatrixLayoutStore(CallInst *CI, SmallVector<User *> &WorkList) {
  int64_t MCols = cast<ConstantInt>(CI->getOperand(5))->getSExtValue();
  FixedVectorType *MatrixType =
      cast<FixedVectorType>(CI->getOperand(0)->getType());
  Metadata *MatL = cast<MetadataAsValue>(CI->getOperand(6))->getMetadata();
  Metadata *MemL = cast<MetadataAsValue>(CI->getOperand(7))->getMetadata();
  Metadata *MatUse = cast<MetadataAsValue>(CI->getOperand(9))->getMetadata();
  bool IsMatUseUnnecessary =
      cast<MDString>(MatUse)->getString().equals("matrix.use.unnecessary");
  bool IsMatUseA = cast<MDString>(MatUse)->getString().equals("matrix.use.a");
  bool IsMatUseB = cast<MDString>(MatUse)->getString().equals("matrix.use.b");
  bool IsMatUseC =
      cast<MDString>(MatUse)->getString().equals("matrix.use.accumulator");
  bool IsMemLPacked = cast<MDString>(MemL)->getString().equals("matrix.packed");
  bool IsMemLRowMajor =
      cast<MDString>(MemL)->getString().equals("matrix.rowmajor");
  bool IsMemLColMajor =
      cast<MDString>(MemL)->getString().equals("matrix.columnmajor");
  // bool IsMatLPackedB =
  //     cast<MDString>(MatL)->getString().equals("matrix.packed.b");
  bool IsMatLRowMajor =
      cast<MDString>(MatL)->getString().equals("matrix.rowmajor");
  if ((cast<MDString>(MatL)->getString() == cast<MDString>(MemL)->getString() &&
       IsMatUseUnnecessary) ||
      (IsMatUseB && IsMemLPacked) || (IsMatUseA && IsMemLRowMajor) ||
      (IsMatUseC && IsMemLRowMajor) ||
      (IsMatUseB && IsMemLRowMajor &&
       MatrixType->getElementType()->isFloatTy()))
    return std::make_pair(false, CI);
  IRBuilder<> Builder(CI);
  LLVMContext &Ctx = Builder.getContext();
  if (((IsMatLRowMajor && IsMemLColMajor && IsMatUseUnnecessary) ||
       ((IsMatUseA || IsMatUseC) && IsMemLColMajor)) &&
      MatrixType->getElementType()->isFloatTy()) {
    return resolveMatrixLayoutStoreHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_rowmajor_to_colmajorPU3AS4fS0_iii",
        PointerType::get(Ctx, 0),
        MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.rowmajor")),
        Builder.CreateMul(CI->getOperand(2), Builder.getInt64(4)), // OldStride
        Builder.getInt64(MCols),                                   // NewStride
        false, WorkList);
  } else {
    errs() << "Unsuppoted combination of matrix.use&matL&memL:\n"
           << "Unsuppoted matrix.use:" << cast<MDString>(MatUse)->getString()
           << "!\n"
           << "Unsuppoted matL:" << cast<MDString>(MatL)->getString() << "!\n"
           << "Unsuppoted memL:" << cast<MDString>(MemL)->getString() << "!\n";
    llvm_unreachable(nullptr);
  }
}
static std::pair<bool, Value *>
resolveMatrixLayoutLoad(CallInst *CI, SmallVector<User *> &WorkList) {
  int64_t MCols = cast<ConstantInt>(CI->getOperand(4))->getSExtValue();
  FixedVectorType *MatrixType = cast<FixedVectorType>(CI->getType());
  Metadata *MatL = cast<MetadataAsValue>(CI->getOperand(5))->getMetadata();
  Metadata *MemL = cast<MetadataAsValue>(CI->getOperand(6))->getMetadata();
  Metadata *MatUse = cast<MetadataAsValue>(CI->getOperand(8))->getMetadata();
  bool IsMatUseUnnecessary =
      cast<MDString>(MatUse)->getString().equals("matrix.use.unnecessary");
  bool IsMatUseA = cast<MDString>(MatUse)->getString().equals("matrix.use.a");
  bool IsMatUseB = cast<MDString>(MatUse)->getString().equals("matrix.use.b");
  bool IsMatUseC =
      cast<MDString>(MatUse)->getString().equals("matrix.use.accumulator");
  bool IsMemLPacked = cast<MDString>(MemL)->getString().equals("matrix.packed");
  bool IsMemLRowMajor =
      cast<MDString>(MemL)->getString().equals("matrix.rowmajor");
  bool IsMemLColMajor =
      cast<MDString>(MemL)->getString().equals("matrix.columnmajor");
  bool IsMatLPackedB =
      cast<MDString>(MatL)->getString().equals("matrix.packed.b");
  bool IsMatLRowMajor =
      cast<MDString>(MatL)->getString().equals("matrix.rowmajor");
  if ((cast<MDString>(MatL)->getString() == cast<MDString>(MemL)->getString() &&
       IsMatUseUnnecessary) ||
      (IsMatUseB && IsMemLPacked) || (IsMatUseA && IsMemLRowMajor) ||
      (IsMatUseC && IsMemLRowMajor) ||
      (IsMatUseB && IsMemLRowMajor &&
       MatrixType->getElementType()->isFloatTy()))
    return std::make_pair(false, CI);
  IRBuilder<> Builder(CI);
  LLVMContext &Ctx = Builder.getContext();

  if (((IsMatLPackedB && IsMemLRowMajor && IsMatUseUnnecessary) ||
       (IsMatUseB && IsMemLRowMajor)) &&
      MatrixType->getElementType()->isIntegerTy(8)) {
    // Transform
    // %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
    //   i32* addressspace(4) %ptr, i64 stride, i1 false, i32 4, i32 2,
    //   metadata !"matrix.packed_b", metadata !"matrix.rowmajor", metadata
    //   !"scope.subgroup", metadata !"matrix.use.unnecessary")
    // =>
    // %alloc = alloca [i8 x 8]
    // %ptr2 = bitcast %alloc to i8*
    // matrix_layout_transform_rowmajor_to_vnni(ptr, %ptr2, rows, cols, stride)
    // %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
    //   i8% ptr2, i64 stride, i1 false, i32 4, i32 2,
    //   metadata !"matrix.packed_b", metadata !"matrix.packed_b", metadata
    //   !"scope.subgroup")
    // PLS: matrix_layout_transform_rowmajor_to_vnni's cols and rows are
    // the real cols and rows of a matrix
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z40matrix_layout_transform_rowmajor_to_vnniPU3AS4cS0_iii",
        Builder.getInt8PtrTy(),
        IsMatUseUnnecessary
            ? MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.packed.b"))
            : MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.packed")),
        CI->getOperand(1), Builder.getInt64(MCols * 4), false, WorkList);
  } else if (((IsMatLPackedB && IsMemLRowMajor && IsMatUseUnnecessary) ||
              (IsMatUseB && IsMemLRowMajor)) &&
             MatrixType->getElementType()->isIntegerTy(16)) {
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z40matrix_layout_transform_rowmajor_to_vnniPU3AS4sS0_iii",
        PointerType::get(Ctx, 0),
        IsMatUseUnnecessary
            ? MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.packed.b"))
            : MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.packed")),
        Builder.CreateMul(CI->getOperand(1), Builder.getInt64(2)),
        Builder.getInt64(MCols * 2), false, WorkList);
  } else if (((IsMatLRowMajor && IsMemLColMajor && IsMatUseUnnecessary) ||
              ((IsMatUseA || IsMatUseC) && IsMemLColMajor)) &&
             MatrixType->getElementType()->isIntegerTy(8)) {
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4cS0_iii",
        Builder.getInt8PtrTy(),
        MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.rowmajor")),
        CI->getOperand(1), Builder.getInt64(MCols), false, WorkList);
  } else if (((IsMatLRowMajor && IsMemLColMajor && IsMatUseUnnecessary) ||
              ((IsMatUseA || IsMatUseC) && IsMemLColMajor)) &&
             MatrixType->getElementType()->isFloatTy()) {
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4fS0_iii",
        PointerType::get(Ctx, 0),
        MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.rowmajor")),
        Builder.CreateMul(CI->getOperand(1), Builder.getInt64(4)),
        Builder.getInt64(MCols), false, WorkList);
  } else if (((IsMatLRowMajor && IsMemLColMajor && IsMatUseUnnecessary) ||
              ((IsMatUseA || IsMatUseC) && IsMemLColMajor)) &&
             MatrixType->getElementType()->isIntegerTy(16)) {
    // Transform
    //   %res = tail call <128 x i16>
    //   @"llvm.experimental.matrix.load.v128i16.p4bfloat16"(%"bfloat16"
    //   addrspace(4)* %call.ascast.i72.i, i64 16, i1 false, i32 8, i32 16,
    //   metadata !"matrix.rowmajor", metadata !"matrix.columnmajor", metadata
    //   !"scope.subgroup", metadata !"matrix.use.unnecessary")
    // into:
    //   @_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4sS0_iii(i16*
    //   %10, i16* %11, i32 8, i32 16, i64 32)
    // %res = call <128 x i16> @llvm.experimental.matrix.load.v128i16.p0i16(i16*
    // %11, i64 16, i1 false, i32 8, i32 16, metadata !"matrix.rowmajor",
    // metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata
    // !"matrix.use.unnecessary")
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4sS0_iii",
        PointerType::get(Ctx, 0),
        MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.rowmajor")),
        Builder.CreateMul(CI->getOperand(1), Builder.getInt64(2)),
        Builder.getInt64(MCols), false, WorkList);
  } else if (((IsMatLPackedB && IsMemLColMajor && IsMatUseUnnecessary) ||
              (IsMatUseB && IsMemLColMajor)) &&
             MatrixType->getElementType()->isIntegerTy(8)) {
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4cS0_iii",
        Builder.getInt8PtrTy(),
        MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.rowmajor")),
        CI->getOperand(1), Builder.getInt64(MCols), true, WorkList);
  } else if (((IsMatLPackedB && IsMemLColMajor && IsMatUseUnnecessary) ||
              (IsMatUseB && IsMemLColMajor)) &&
             MatrixType->getElementType()->isIntegerTy(16)) {
    // Transform
    //     %res = tail call <128 x i16>
    //     @"llvm.experimental.matrix.load.v128i16.p4bfloat16(%"bfloat16"
    //     addrspace(4)* %call.ascast.i77.i, i64 32, i1 false, i32 16, i32 8,
    //     metadata !"matrix.packed.b", metadata !"matrix.columnmajor", metadata
    //     !"scope.subgroup", metadata !"matrix.use.unnecessary")
    // into:
    //  call void
    //  @_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4sS0_iii(i16* %13,
    //  i16* %14, i32 16, i32 8, i64 64)
    // call void @_Z40matrix_layout_transform_rowmajor_to_vnniPU3AS4sS0_iii(i16*
    // %14, i16* %15, i32 16, i32 8, i64 16)
    // %res = call <128 x i16>
    //@llvm.experimental.matrix.load.v128i16.p0i16(i16* %15, i64 16, i1 false,
    // i32 16, i32 8, metadata !"matrix.packed.b", metadata !"matrix.packed.b",
    // metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
    //
    // resolveMatrixLayoutLoadHelper's OldStride should be specified in int8
    // since it is feed to
    // _Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4sS0_iii;
    // NewStride should be specified in int16 since it is feed to matrix.load;
    // _Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4sS0_iii's cols&rows
    // are the real matrix's cols&rows instead of cols&rows of colmajor's
    // matrix.
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4sS0_iii",
        PointerType::get(Ctx, 0),
        MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.rowmajor")),
        Builder.CreateMul(CI->getOperand(1), Builder.getInt64(2)),
        Builder.getInt64(MCols), true, WorkList);
  } else {
    errs() << "Unsuppoted combination of matrix.use&matL&memL:\n"
           << "Unsuppoted matrix.use:" << cast<MDString>(MatUse)->getString()
           << "!\n"
           << "Unsuppoted matL:" << cast<MDString>(MatL)->getString() << "!\n"
           << "Unsuppoted memL:" << cast<MDString>(MemL)->getString() << "!\n";
    llvm_unreachable(nullptr);
  }

  return std::make_pair(false, CI);
}

bool ResolveMatrixLayoutPass::runImpl(Module &M) {
  bool Changed = false;
  for (auto &F : M) {
    if (F.getIntrinsicID() != Intrinsic::experimental_matrix_load &&
        F.getIntrinsicID() != Intrinsic::experimental_matrix_store)
      continue;
    SmallVector<User *> WorkList(F.users());
    for (auto *U : WorkList) {
      CallInst *CI = cast<CallInst>(U);
      bool Resolved;
      Value *Replacement;
      if (F.getIntrinsicID() == Intrinsic::experimental_matrix_load)
        std::tie(Resolved, Replacement) = resolveMatrixLayoutLoad(CI, WorkList);
      else if (F.getIntrinsicID() == Intrinsic::experimental_matrix_store)
        std::tie(Resolved, Replacement) =
            resolveMatrixLayoutStore(CI, WorkList);
      if (Resolved) {
        CI->replaceAllUsesWith(Replacement);
        CI->eraseFromParent();
        Changed = true;
      }
    }
  }
  return Changed;
}

PreservedAnalyses ResolveMatrixLayoutPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
