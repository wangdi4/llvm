//===- Target/X86/Intel_X86LowerMatrixIntrinsics.cpp - ----------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// This file defines the pass which transforms the following matrix
/// intrinsics into amx's intrinsics:
/// llvm.experimental.matrix.store => llvm.x86.tilestored64.internal
/// llvm.experimental.matrix.load => llvm.x86.tileloadd64.internal
/// llvm.experimental.matrix.mad => llvm.x86.tdpbssd(or tdpbf16ps).internal
///
/// Example:
/// Calculate the formatted size according to the real size, layout&type(i8 or
/// i16).
/// %val = call void @llvm.experimental.matrix.store.v4i8.p4i8(<8 x i8> %src,
/// i32* ptr, i64 %stride, i1 false, i32 4, i32 2, metadata !"matrix.rowmajor",
/// metadata !"matrix.rowmajor", metadata !"scope.subgroup").
/// =>
/// %amxsrc = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i8> %src)
/// %val = call x86_amx @llvm.x86.tilestored64.internal(i32 1, i32 8, i8* ptr,
/// i64 stride, x86_amx %amxsrc)
//
//===----------------------------------------------------------------------===//
//

#include "X86.h"
#include "X86TargetMachine.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

#define DEBUG_TYPE "x86-lower-matrix-intrinsics"

namespace {

class X86LowerMatrixIntrinsicsPass : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid..

  X86LowerMatrixIntrinsicsPass() : FunctionPass(ID) {
    initializeX86LowerMatrixIntrinsicsPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    bool MadeChange = false;
    // Put matrix intrinsics into worklist
    SmallVector<Instruction *, 8> Worklist;
    for (BasicBlock *BB : depth_first(&F)) {
      for (BasicBlock::iterator BBI = BB->begin(), BBIE = BB->end();
           BBI != BBIE; ++BBI) {
        IntrinsicInst *II = dyn_cast<IntrinsicInst>(&*BBI);
        if (II) {
          switch (II->getIntrinsicID()) {
          default:
            break;
          case Intrinsic::experimental_matrix_load:
          case Intrinsic::experimental_matrix_store:
          case Intrinsic::experimental_matrix_mad:
          case Intrinsic::experimental_matrix_sumad:
          case Intrinsic::experimental_matrix_usmad:
          case Intrinsic::experimental_matrix_uumad:
          case Intrinsic::experimental_matrix_extract_row_slice:
          case Intrinsic::experimental_matrix_insert_row_slice:
          case Intrinsic::experimental_matrix_fill:
            Worklist.push_back(&*BBI);
            break;
          }
        }
      }
    }
    // Process matrix intrinsics int the worklist
    for (auto It : Worklist) {
      IntrinsicInst *II = dyn_cast<IntrinsicInst>(&*It);
      assert(II && "Not a valiad intrinsic");
      MadeChange |= ProcessMatrixIntrinsics(II);
    }
    return MadeChange;
  }

private:
  bool ProcessMatrixIntrinsics(IntrinsicInst *II);
  bool ProcessMatrixLoad(IntrinsicInst *II);
  bool ProcessMatrixStore(IntrinsicInst *II);
  bool ProcessMatrixMad(IntrinsicInst *II);
  bool ProcessMatrixExtractRowSlice(IntrinsicInst *II);
  bool ProcessMatrixInsertRowSlice(IntrinsicInst *II);
  bool ProcessMatrixFill(IntrinsicInst *II);
};

} // end anonymous namespace

char X86LowerMatrixIntrinsicsPass::ID = 0;

INITIALIZE_PASS_BEGIN(X86LowerMatrixIntrinsicsPass, DEBUG_TYPE,
                      "X86 transform matrix intrinsics to amx intrinsics",
                      false, false)
INITIALIZE_PASS_END(X86LowerMatrixIntrinsicsPass, DEBUG_TYPE,
                    "X86 transform matrix intrinsics to amx intrinsics", false,
                    false)

FunctionPass *llvm::createX86LowerMatrixIntrinsicsPass() {
  return new X86LowerMatrixIntrinsicsPass();
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixIntrinsics(IntrinsicInst *II) {
  bool MadeChange = false;
  switch (II->getIntrinsicID()) {
  default:
    break;
  case Intrinsic::experimental_matrix_load:
    MadeChange |= ProcessMatrixLoad(II);
    break;
  case Intrinsic::experimental_matrix_store:
    MadeChange |= ProcessMatrixStore(II);
    break;
  case Intrinsic::experimental_matrix_mad:
  case Intrinsic::experimental_matrix_sumad:
  case Intrinsic::experimental_matrix_usmad:
  case Intrinsic::experimental_matrix_uumad:
    MadeChange |= ProcessMatrixMad(II);
    break;
  case Intrinsic::experimental_matrix_extract_row_slice:
    MadeChange |= ProcessMatrixExtractRowSlice(II);
    break;
  case Intrinsic::experimental_matrix_insert_row_slice:
    MadeChange |= ProcessMatrixInsertRowSlice(II);
    break;
  case Intrinsic::experimental_matrix_fill:
    MadeChange |= ProcessMatrixFill(II);
    break;
  }
  return MadeChange;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixLoad(IntrinsicInst *II) {
  // Calculate the formatted size according to the real size, layout&type(i8 or
  // i16).
  // %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
  //   i32* addressspace(4) ptr, i64 stride, i1 false, i32 4, i32 2,
  //   metadata !"matrix.packed_b", metadata !"matrix.packed_b", metadata
  //   !"scope.subgroup")
  // =>
  // %val = call x86_amx @llvm.x86.tileloadd64.internal(i32 1, i32 8, i8* ptr,
  // i64 stride).
  // %res = call x86_amx @llvm.x86.cast.tile.to.vector.v4i8(<4 x i8> %val)
  IRBuilder<> Builder(II);
  int64_t MRows = cast<ConstantInt>(II->getOperand(3))->getSExtValue();
  int64_t MCols = cast<ConstantInt>(II->getOperand(4))->getSExtValue();
  FixedVectorType *MatrixType = cast<FixedVectorType>(II->getType());
  Type *MatrixElemType = MatrixType->getElementType();
  int64_t Factor = 1;
  int64_t SizeFactor = 1;
  if (MatrixElemType->isIntegerTy(16))
    SizeFactor = 2;
  else if (MatrixElemType->isFloatTy() || MatrixElemType->isIntegerTy(32))
    SizeFactor = 4;
  else if (MatrixElemType->isIntegerTy(8))
    SizeFactor = 1;
  else {
    errs() << "Unsuppoted MatrixElemType:" << MatrixElemType << "!\n"
           << "AMX provides support for int8_t, uint8_t, int32_t, bf16 and "
              "float!\n";
    llvm_unreachable(nullptr);
  }
  Metadata *MDLayout = cast<MetadataAsValue>(II->getOperand(5))->getMetadata();
  // If it is packed_b, the type can only be int8/bf16.
  // If it is row_major, the type can be int8/bf16/float/int32, Factor can only
  // be 1.
  if (cast<MDString>(MDLayout)->getString().equals("matrix.packed.b") &&
      MatrixElemType->isIntegerTy(8))
    Factor = 4;
  else if (cast<MDString>(MDLayout)->getString().equals("matrix.packed.b") &&
           MatrixElemType->isIntegerTy(16))
    Factor = 2;
  else if (cast<MDString>(MDLayout)->getString().equals("matrix.rowmajor"))
    Factor = 1;
  else {
    errs() << "Unsuppoted Layout:" << cast<MDString>(MDLayout)->getString()
           << "!\n"
           << "We support layout: matrix.rowmajor and matrix.packed.b!\n";
    llvm_unreachable(nullptr);
  }
  // Handle cases where it is vxi8 and packedb.
  assert(MRows >= Factor && MRows % Factor == 0 &&
         "Invalid Matrix Rows Value!");
  int64_t ResRows = MRows / Factor;
  int64_t ResCols = MCols * Factor * SizeFactor;
  if (ResRows > 16 || ResCols > 64) {
    errs() << "Unsupported Size for tileload! Rows = " << ResRows
           << "Cols = " << ResCols << "!\n"
           << "We support Size: Rows <= 16 and Cols <= 64!\n";
    llvm_unreachable(nullptr);
  }
  Value *Rows = Builder.getInt16(ResRows);
  Value *Cols = Builder.getInt16(ResCols);
  Value *Ptr = II->getOperand(0)->getType()->getPointerAddressSpace() == 0
                   ? Builder.CreateBitCast(
                         II->getOperand(0),
                         llvm::Type::getInt8PtrTy(Builder.getContext()))
                   : Builder.CreateAddrSpaceCast(
                         II->getOperand(0),
                         llvm::Type::getInt8PtrTy(Builder.getContext()));
  // Create the argument list
  std::array<Value *, 4> Args{
      Rows, Cols, Ptr,
      Builder.CreateMul(II->getOperand(1), Builder.getInt64(SizeFactor))};
  Value *NewInst =
      Builder.CreateIntrinsic(Intrinsic::x86_tileloadd64_internal, None, Args);
  II->replaceAllUsesWith(Builder.CreateIntrinsic(
      Intrinsic::x86_cast_tile_to_vector, {MatrixType}, {NewInst}));
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixStore(IntrinsicInst *II) {
  // Calculate the formatted size according to the real size, layout&type(i8 or
  // i16).
  // %val = call void @llvm.experimental.matrix.store.v4i8.p4i8(<8 x i8> %src,
  // i32* ptr, i64 %stride, i1 false, i32 4, i32 2, metadata !"matrix.rowmajor",
  // metadata !"matrix.rowmajor", metadata !"scope.subgroup").
  // =>
  // %amxsrc = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i8> %src)
  // %val = call x86_amx @llvm.x86.tilestored64.internal(i32 1, i32 8, i8* ptr,
  // i64 stride, x86_amx %amxsrc).
  IRBuilder<> Builder(II);
  int64_t MRows = cast<ConstantInt>(II->getOperand(4))->getSExtValue();
  int64_t MCols = cast<ConstantInt>(II->getOperand(5))->getSExtValue();
  FixedVectorType *MatrixType =
      cast<FixedVectorType>(II->getOperand(0)->getType());
  Type *MatrixElemType = MatrixType->getElementType();
  int64_t Factor = 1;
  int64_t SizeFactor = 1;
  // FIXME: SizeFactor = MatrixElemType->getScalarSizeInBits()/8?
  if (MatrixElemType->isIntegerTy(16))
    SizeFactor = 2;
  else if (MatrixElemType->isFloatTy() || MatrixElemType->isIntegerTy(32))
    SizeFactor = 4;
  else if (MatrixElemType->isIntegerTy(8))
    SizeFactor = 1;
  else {
    errs() << "Unsuppoted MatrixElemType:" << MatrixElemType << "!\n"
           << "AMX provides support for int8_t, uint8_t, int32_t, bf16 and "
              "float!\n";
    llvm_unreachable(nullptr);
  }
  Metadata *MDLayout = cast<MetadataAsValue>(II->getOperand(6))->getMetadata();
  // If it is wordpackedb, the type can only be int8/bf16.
  // If it is row_major, the type can be int8/bf16/float/int32.
  if (cast<MDString>(MDLayout)->getString().equals("matrix.packed.b") &&
      MatrixElemType->isIntegerTy(8))
    Factor = 4;
  else if (cast<MDString>(MDLayout)->getString().equals("matrix.packed.b") &&
           MatrixElemType->isIntegerTy(16))
    Factor = 2;
  else if (cast<MDString>(MDLayout)->getString().equals("matrix.rowmajor"))
    Factor = 1;
  else {
    errs() << "Unsuppoted Layout:" << cast<MDString>(MDLayout)->getString()
           << "!\n"
           << "We support layout: matrix.rowmajor and matrix.packed.b!\n";
    llvm_unreachable(nullptr);
  }
  assert(MRows >= Factor && MRows % Factor == 0 &&
         "Invalid Matrix Rows Value!");
  int64_t ResRows = MRows / Factor;
  int64_t ResCols = MCols * Factor * SizeFactor;
  if (ResRows > 16 || ResCols > 64) {
    errs() << "Unsupported Size for tilestore! Rows = " << ResRows
           << "Cols = " << ResCols << "!\n"
           << "We support Size: Rows <= 16 and Cols <= 64!\n";
    llvm_unreachable(nullptr);
  }
  Value *Rows = Builder.getInt16(ResRows);
  Value *Cols = Builder.getInt16(ResCols);
  Value *Ptr = II->getOperand(1)->getType()->getPointerAddressSpace() == 0
                   ? Builder.CreateBitCast(
                         II->getOperand(1),
                         llvm::Type::getInt8PtrTy(Builder.getContext()))
                   : Builder.CreateAddrSpaceCast(
                         II->getOperand(1),
                         llvm::Type::getInt8PtrTy(Builder.getContext()));
  // Create the argument list
  std::array<Value *, 5> Args{
      Rows, Cols, Ptr,
      Builder.CreateMul(II->getOperand(2), Builder.getInt64(SizeFactor)),
      Builder.CreateIntrinsic(Intrinsic::x86_cast_vector_to_tile,
                              {II->getOperand(0)->getType()},
                              {II->getOperand(0)})};
  Value *NewInst =
      Builder.CreateIntrinsic(Intrinsic::x86_tilestored64_internal, None, Args);
  II->replaceAllUsesWith(NewInst);
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixMad(IntrinsicInst *II) {
  // Transform %mad = call <4 x i8>
  // @llvm.experimental.matrix.mad.v4i8.v8i8.v8i8( <8 x i8> %A, metadata
  // !"matrix.rowmajor", <8 x i8> %B, metadata !"matrix.packed.b", <4 x i32>
  // %C, metadata !"matrix.rowmajor", i32 2(A.rows), i32 4(B.rows), i32
  // 2(C.cols), metadata !"scope.subgroup").
  // into:
  // %a = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i8> %A).
  // %b = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i8> %B).
  // %c = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i32> %C).
  // %d = call x86_amx @llvm.x86.tdpbssd.internal(i16 2(C.rows), i16 2*4(C.cols
  // in int8), i16 4(A.cols), x86_amx %c, x86_amx %a, x86_amx %b).
  // A.cols = B.rows, C.rows = A.rows, C.cols in int8 = B.rows * 4.
  IRBuilder<> Builder(II);
  FixedVectorType *MatrixType = cast<FixedVectorType>(II->getType());
  Type *MatrixElemType = MatrixType->getElementType();
  Intrinsic::ID IID;
  switch (II->getIntrinsicID()) {
  default:
    assert(false && "Invalid Intrinsic ID!");
    break;
  case Intrinsic::experimental_matrix_mad:
    IID = MatrixElemType->isFloatTy() ? Intrinsic::x86_tdpbf16ps_internal
                                      : Intrinsic::x86_tdpbssd_internal;
    break;
  case Intrinsic::experimental_matrix_sumad:
    IID = Intrinsic::x86_tdpbsud_internal;
    break;
  case Intrinsic::experimental_matrix_usmad:
    IID = Intrinsic::x86_tdpbusd_internal;
    break;
  case Intrinsic::experimental_matrix_uumad:
    IID = Intrinsic::x86_tdpbuud_internal;
    break;
  }

  Value *M =
      Builder.getInt16(cast<ConstantInt>(II->getOperand(6))->getSExtValue());
  Value *K =
      Builder.getInt16(cast<ConstantInt>(II->getOperand(7))->getSExtValue() *
                       (MatrixElemType->isFloatTy() ? 2 : 1));
  Value *N = Builder.getInt16(
      cast<ConstantInt>(II->getOperand(8))->getSExtValue() * 4);
  // M=A.rows, N=C.cols*4, K=B.rows,C,A,B
  std::array<Value *, 6> Args{
      M,
      N,
      K,
      Builder.CreateIntrinsic(Intrinsic::x86_cast_vector_to_tile,
                              {II->getOperand(4)->getType()},
                              {II->getOperand(4)}),
      Builder.CreateIntrinsic(Intrinsic::x86_cast_vector_to_tile,
                              {II->getOperand(0)->getType()},
                              {II->getOperand(0)}),
      Builder.CreateIntrinsic(Intrinsic::x86_cast_vector_to_tile,
                              {II->getOperand(2)->getType()},
                              {II->getOperand(2)})};
  Value *NewInst = Builder.CreateIntrinsic(IID, None, Args);
  II->replaceAllUsesWith(Builder.CreateIntrinsic(
      Intrinsic::x86_cast_tile_to_vector, {MatrixType}, {NewInst}));
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixExtractRowSlice(
    IntrinsicInst *II) {
  // Transform
  // %slice = call <4 x i32>
  // @llvm.experimental.matrix.extract.row.slice.v8i32(<16 x i32> %mat,  i32 %x,
  // i32 %y, i32 4, i32 4, i32 4, metadata !"matrix.rowmajor") into several
  // extractelement from %mat and insertelement to %slice.
  IRBuilder<> Builder(II);
  int64_t ElemNum = cast<ConstantInt>(II->getOperand(3))->getSExtValue();
  FixedVectorType *SliceType = cast<FixedVectorType>(II->getType());

  Metadata *MDLayout = cast<MetadataAsValue>(II->getOperand(6))->getMetadata();
  if (!cast<MDString>(MDLayout)->getString().equals("matrix.rowmajor")) {
    errs() << "Unsuppoted Layout:" << cast<MDString>(MDLayout)->getString()
           << "!\n"
           << "We support layout for slicing: matrix.rowmajor!\n";
    llvm_unreachable(nullptr);
  }
  // offset = which_row*num_of_cols+which_col
  Value *Offset =
      Builder.CreateAdd(Builder.CreateMul(II->getOperand(1), II->getOperand(5)),
                        II->getOperand(2));
  Value *DstSlice = PoisonValue::get(SliceType);
  for (int64_t ElemI = 0; ElemI < ElemNum; ++ElemI) {
    Value *Elem = Builder.CreateExtractElement(II->getOperand(0), Offset);
    DstSlice =
        Builder.CreateInsertElement(DstSlice, Elem, Builder.getInt32(ElemI));
    if (ElemI < ElemNum - 1)
      Offset = Builder.CreateAdd(Offset, Builder.getInt32(1));
  }
  II->replaceAllUsesWith(DstSlice);
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixInsertRowSlice(
    IntrinsicInst *II) {
  // Transform %0 = call <16 x i32>
  // @llvm.experimental.matrix.insert.row.slice.v8i32(<16 x i32> %mat, <4 x i32>
  // %slice, i32 %x, i32 %y, i32 4, i32 4, i32 4, metadata !"matrix.rowmajor")
  // into several extractelement from %slice and insertelement to %mat.
  IRBuilder<> Builder(II);
  int64_t ElemNum = cast<ConstantInt>(II->getOperand(4))->getSExtValue();

  Metadata *MDLayout = cast<MetadataAsValue>(II->getOperand(7))->getMetadata();
  if (!cast<MDString>(MDLayout)->getString().equals("matrix.rowmajor")) {
    errs() << "Unsuppoted Layout:" << cast<MDString>(MDLayout)->getString()
           << "!\n"
           << "We support layout for slicing: matrix.rowmajor!\n";
    llvm_unreachable(nullptr);
  }
  // offset = which_row*num_of_cols+which_col
  Value *Offset =
      Builder.CreateAdd(Builder.CreateMul(II->getOperand(2), II->getOperand(6)),
                        II->getOperand(3));
  Value *DstMatrix = II->getOperand(0);
  for (int64_t ElemI = 0; ElemI < ElemNum; ++ElemI) {
    Value *Elem = Builder.CreateExtractElement(II->getOperand(1),
                                               Builder.getInt32(ElemI));
    DstMatrix = Builder.CreateInsertElement(DstMatrix, Elem, Offset);
    if (ElemI < ElemNum - 1)
      Offset = Builder.CreateAdd(Offset, Builder.getInt32(1));
  }
  II->replaceAllUsesWith(DstMatrix);
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixFill(IntrinsicInst *II) {
  // Transform
  // %fill = call <16 x i8> @llvm.experimental.matrix.fill.v16i8(i8 0, i32 4,
  // i32 4, metadata !"matrix.packed.b", metadata !"scope.subgroup")
  // into:
  // %tmp0 = call x86_amx @llvm.x86.tilezero.internal(i16 1, i16 16)
  // %tmp1 = call <16 x i8> @llvm.x86.cast.tile.to.vector.v16i8(x86_amx %tmp0)
  IRBuilder<> Builder(II);

  int64_t MRows = cast<ConstantInt>(II->getOperand(1))->getSExtValue();
  int64_t MCols = cast<ConstantInt>(II->getOperand(2))->getSExtValue();
  FixedVectorType *MatrixType = cast<FixedVectorType>(II->getType());
  Type *MatrixElemType = MatrixType->getElementType();
  int64_t Factor = 1;
  int64_t SizeFactor = 1;
  assert(MatrixElemType == II->getOperand(0)->getType() &&
         "invalid MatrixElemType, MatrixElemType should equal to MatrixFill's "
         "Source ElemType");
  // Check whether we create a zero matrix by int_experimental_matrix_fill.
  // We could have support filling a non-zero matrix but OCL-Optimizer help us
  // do this thing and transform it into plain llvm IR.
  if (isa<ConstantInt>(II->getOperand(0)))
    assert(cast<ConstantInt>(II->getOperand(0))->isZero() &&
           "MatrixFill's val is not zero!");
  else if (isa<ConstantFP>(II->getOperand(0)))
    assert(cast<ConstantFP>(II->getOperand(0))->isZero() &&
           "MatrixFill's val is not zero!");
  else {
    errs() << "Unsuppoted MatrixElemType:" << MatrixElemType << "!\n"
           << "AMX provides support for int8_t, uint8_t, int32_t, bf16 and "
              "float!\n";
    llvm_unreachable(nullptr);
  }

  if (MatrixElemType->isIntegerTy(16)) {
    SizeFactor = 2;
  } else if (MatrixElemType->isFloatTy() || MatrixElemType->isIntegerTy(32))
    SizeFactor = 4;
  else if (MatrixElemType->isIntegerTy(8))
    SizeFactor = 1;
  else {
    errs() << "Unsuppoted MatrixElemType:" << MatrixElemType << "!\n"
           << "AMX provides support for int8_t, uint8_t, int32_t, bf16 and "
              "float!\n";
    llvm_unreachable(nullptr);
  }
  Metadata *MDLayout = cast<MetadataAsValue>(II->getOperand(3))->getMetadata();
  // If it is packed_b, the type can only be int8/bf16.
  // If it is row_major, the type can be int8/bf16/float/int32, Factor can only
  // be 1.
  if (cast<MDString>(MDLayout)->getString().equals("matrix.packed.b") &&
      MatrixElemType->isIntegerTy(8))
    Factor = 4;
  else if (cast<MDString>(MDLayout)->getString().equals("matrix.packed.b") &&
           MatrixElemType->isIntegerTy(16))
    Factor = 2;
  else if (cast<MDString>(MDLayout)->getString().equals("matrix.rowmajor"))
    Factor = 1;
  else {
    errs() << "Unsuppoted Layout:" << cast<MDString>(MDLayout)->getString()
           << "!\n"
           << "We support layout: matrix.rowmajor and matrix.packed.b!\n";
    llvm_unreachable(nullptr);
  }
  // Handle cases where it is vxi8 and packedb.
  assert(MRows >= Factor && MRows % Factor == 0 &&
         "Invalid Matrix Rows Value!");
  int64_t ResRows = MRows / Factor;
  int64_t ResCols = MCols * Factor * SizeFactor;
  if (ResRows > 16 || ResCols > 64) {
    errs() << "Unsupported Size for tilezero! Rows = " << ResRows
           << "Cols = " << ResCols << "!\n"
           << "We support Size: Rows <= 16 and Cols <= 64!\n";
    llvm_unreachable(nullptr);
  }
  Value *Rows = Builder.getInt16(ResRows);
  Value *Cols = Builder.getInt16(ResCols);
  // Create the argument list
  std::array<Value *, 2> Args{Rows, Cols};
  Value *NewInst =
      Builder.CreateIntrinsic(Intrinsic::x86_tilezero_internal, None, Args);
  II->replaceAllUsesWith(Builder.CreateIntrinsic(
      Intrinsic::x86_cast_tile_to_vector, {MatrixType}, {NewInst}));
  II->eraseFromParent();
  return true;
}
