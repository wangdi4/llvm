//===- Target/X86/Intel_X86LowerMatrixIntrinsics.cpp - ----------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
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

static bool isMatBPacked(Metadata *MatUse, Metadata *MemLayout,
                         Metadata *MatLayout) {
  return (cast<MDString>(MatUse)->getString().equals(
              "matrix.use.unnecessary") &&
          cast<MDString>(MemLayout)->getString().equals("matrix.packed.b") &&
          cast<MDString>(MatLayout)->getString().equals("matrix.packed.b")) ||
         (cast<MDString>(MemLayout)->getString().equals("matrix.packed") &&
          cast<MDString>(MatUse)->getString().equals("matrix.use.b"));
}

static bool isMatCRowmajor(Metadata *MatUse, Metadata *MemLayout,
                           Metadata *MatLayout) {
  return (cast<MDString>(MatUse)->getString().equals(
              "matrix.use.unnecessary") &&
          cast<MDString>(MemLayout)->getString().equals("matrix.rowmajor") &&
          cast<MDString>(MatLayout)->getString().equals("matrix.rowmajor")) ||
         (cast<MDString>(MemLayout)->getString().equals("matrix.rowmajor") &&
          cast<MDString>(MatUse)->getString().equals("matrix.use.accumulator"));
}

static bool isMatARowmajor(Metadata *MatUse, Metadata *MemLayout,
                           Metadata *MatLayout) {
  return (cast<MDString>(MatUse)->getString().equals(
              "matrix.use.unnecessary") &&
          cast<MDString>(MemLayout)->getString().equals("matrix.rowmajor") &&
          cast<MDString>(MatLayout)->getString().equals("matrix.rowmajor")) ||
         (cast<MDString>(MemLayout)->getString().equals("matrix.rowmajor") &&
          cast<MDString>(MatUse)->getString().equals("matrix.use.a"));
}

static bool isMatBRowmajor(Metadata *MatUse, Metadata *MemLayout,
                           Metadata *MatLayout) {
  return (cast<MDString>(MatUse)->getString().equals(
              "matrix.use.unnecessary") &&
          cast<MDString>(MemLayout)->getString().equals("matrix.rowmajor") &&
          cast<MDString>(MatLayout)->getString().equals("matrix.rowmajor")) ||
         (cast<MDString>(MemLayout)->getString().equals("matrix.rowmajor") &&
          cast<MDString>(MatUse)->getString().equals("matrix.use.b"));
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
  //   metadata !"matrix.packed.b", metadata !"matrix.packed.b", metadata
  //   !"scope.subgroup")
  // =>
  // %val = call x86_amx @llvm.x86.tileloadd64.internal(i32 1, i32 8, i8* ptr,
  // i64 stride).
  // %res = call x86_amx @llvm.x86.cast.tile.to.vector.v4i8(<4 x i8> %val)
  //
  // In the unified matrix, we switch to check the matrixuse instead of
  // matrixlayout, we don't care the value of matrixlayout metadata.
  //
  // %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
  //   i32* addressspace(4) ptr, i64 stride, i1 false, i32 4, i32 2,
  //   metadata !"matrix.rowmajor", metadata !"matrix.packed", metadata
  //   !"scope.subgroup", metadata !"matrix.use.b")
  // =>
  // %val = call x86_amx @llvm.x86.tileloadd64.internal(i32 1, i32 8, i8* ptr,
  // i64 stride).
  // %res = call x86_amx @llvm.x86.cast.tile.to.vector.v4i8(<4 x i8> %val)
  //
  IRBuilder<> Builder(II);
  int64_t MRows = cast<ConstantInt>(II->getOperand(3))->getSExtValue();
  int64_t MCols = cast<ConstantInt>(II->getOperand(4))->getSExtValue();
  FixedVectorType *MatrixType = cast<FixedVectorType>(II->getType());
  Type *MatrixElemType = MatrixType->getElementType();
  int64_t Factor = 1;
  int64_t SizeFactor = 1;
  if ((MatrixElemType->isIntegerTy(16) || MatrixElemType->isHalfTy()))
    SizeFactor = 2;
  else if (MatrixElemType->isFloatTy() || MatrixElemType->isIntegerTy(32))
    SizeFactor = 4;
  else if (MatrixElemType->isIntegerTy(8))
    SizeFactor = 1;
  else {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted MatrixElemType:" << MatrixElemType
       << "AMX provides support for int8_t, uint8_t, int32_t, bf16, half, and "
          "float!\n";
    report_fatal_error(Twine(OS.str()));
  }
  Metadata *MatUse = cast<MetadataAsValue>(II->getOperand(8))->getMetadata();
  Metadata *MatLayout = cast<MetadataAsValue>(II->getOperand(5))->getMetadata();
  Metadata *MemLayout = cast<MetadataAsValue>(II->getOperand(6))->getMetadata();
  // If it is packed_b, the type can only be int8/bf16.
  // If it is row_major, the type can be int8/bf16/float/int32, Factor can only
  // be 1.
  if (isMatBPacked(MatUse, MemLayout, MatLayout) &&
      MatrixElemType->isIntegerTy(8))
    Factor = 4;
  else if (isMatBPacked(MatUse, MemLayout, MatLayout) &&
           (MatrixElemType->isIntegerTy(16) || MatrixElemType->isHalfTy()))
    Factor = 2;
  else if (isMatARowmajor(MatUse, MemLayout, MatLayout) ||
           isMatCRowmajor(MatUse, MemLayout, MatLayout) ||
           (isMatBRowmajor(MatUse, MemLayout, MatLayout) &&
            MatrixElemType->isFloatTy()))
    Factor = 1;
  else {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted Layout:" << cast<MDString>(MemLayout)->getString()
       << "!\n"
       << "Unsuppoted matrix.use:" << cast<MDString>(MatUse)->getString()
       << "!\n"
       << "We support layout&use: matrix.rowmajor(A,C)(int8, int16) and "
          "matrix.packed(B)(float) and matrix.rowmajor(B)(float)!\n";
    report_fatal_error(Twine(OS.str()));
  }
  // Handle cases where it is vxi8 and packedb.
  assert(MRows >= Factor && MRows % Factor == 0 &&
         "Invalid Matrix Rows Value!");
  int64_t ResRows = MRows / Factor;
  int64_t ResCols = MCols * Factor * SizeFactor;
  if (ResRows > 16 || ResCols > 64) {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsupported Size for tileload! Rows = " << ResRows
       << "Cols = " << ResCols << "!\n"
       << "We support Size: Rows <= 16 and Cols <= 64!\n";
    report_fatal_error(Twine(OS.str()));
  }
  Value *Rows = Builder.getInt16(ResRows);
  Value *Cols = Builder.getInt16(ResCols);
  Value *Ptr = II->getOperand(0)->getType()->getPointerAddressSpace() == 0
                   ? Builder.CreateBitCast(
                         II->getOperand(0),
                         llvm::PointerType::getUnqual(Builder.getContext()))
                   : Builder.CreateAddrSpaceCast(
                         II->getOperand(0),
                         llvm::PointerType::getUnqual(Builder.getContext()));
  // Create the argument list
  std::array<Value *, 4> Args{
      Rows, Cols, Ptr,
      Builder.CreateMul(II->getOperand(1), Builder.getInt64(SizeFactor))};
  Value *NewInst = Builder.CreateIntrinsic(Intrinsic::x86_tileloadd64_internal,
                                           std::nullopt, Args);
  II->replaceAllUsesWith(Builder.CreateIntrinsic(
      Intrinsic::x86_cast_tile_to_vector, {MatrixType}, {NewInst}));
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixStore(IntrinsicInst *II) {
  // Calculate the formatted size according to the real size, layout&type(i8 or
  // i16).
  // %val = call void @llvm.experimental.matrix.store.v4i8.p4i8(<8 x i8> %src,
  // i32* ptr, i64 %stride, i1 false, i32 4, i32 2, metadata !"matrix.packed.b",
  // metadata !"matrix.packed.b", metadata !"scope.subgroup").
  // =>
  // %amxsrc = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i8> %src)
  // %val = call x86_amx @llvm.x86.tilestored64.internal(i32 1, i32 8, i8* ptr,
  // i64 stride, x86_amx %amxsrc).
  //
  // In the unified matrix, we switch to check the matrixuse instead of
  // matrixlayout, we don't care the value of matrixlayout metadata.
  //
  // %val = call void @llvm.experimental.matrix.store.v4i8.p4i8(<8 x i8> %src,
  // i32* ptr, i64 %stride, i1 false, i32 4, i32 2, metadata
  // !"matrix.columnmajor", metadata
  // !"matrix.packed", metadata !"scope.subgroup", metadata !"matrix.use.b").
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
  if ((MatrixElemType->isIntegerTy(16) || MatrixElemType->isHalfTy()))
    SizeFactor = 2;
  else if (MatrixElemType->isFloatTy() || MatrixElemType->isIntegerTy(32))
    SizeFactor = 4;
  else if (MatrixElemType->isIntegerTy(8))
    SizeFactor = 1;
  else {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted MatrixElemType:" << MatrixElemType << "!\n"
       << "AMX provides support for int8_t, uint8_t, int32_t, bf16 and "
          "float!\n";
    report_fatal_error(Twine(OS.str()));
  }
  Metadata *MatUse = cast<MetadataAsValue>(II->getOperand(9))->getMetadata();
  Metadata *MatLayout = cast<MetadataAsValue>(II->getOperand(6))->getMetadata();
  Metadata *MemLayout = cast<MetadataAsValue>(II->getOperand(7))->getMetadata();
  // If it is wordpackedb, the type can only be int8/bf16.
  // If it is row_major, the type can be int8/bf16/float/int32.
  if (isMatBPacked(MatUse, MemLayout, MatLayout) &&
      MatrixElemType->isIntegerTy(8))
    Factor = 4;
  else if (isMatBPacked(MatUse, MemLayout, MatLayout) &&
           (MatrixElemType->isIntegerTy(16) || MatrixElemType->isHalfTy()))
    Factor = 2;
  else if (isMatARowmajor(MatUse, MemLayout, MatLayout) ||
           isMatCRowmajor(MatUse, MemLayout, MatLayout))
    Factor = 1;
  else {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted Layout:" << cast<MDString>(MemLayout)->getString()
       << "!\n"
       << "Unsuppoted matrix.use:" << cast<MDString>(MatUse)->getString()
       << "!\n"
       << "We support layout&use: matrix.rowmajor(A,C) and "
          "matrix.packed(B)!\n";
    report_fatal_error(Twine(OS.str()));
  }
  assert(MRows >= Factor && MRows % Factor == 0 &&
         "Invalid Matrix Rows Value!");
  int64_t ResRows = MRows / Factor;
  int64_t ResCols = MCols * Factor * SizeFactor;
  if (ResRows > 16 || ResCols > 64) {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsupported Size for tilestore! Rows = " << ResRows
       << "Cols = " << ResCols << "!\n"
       << "We support Size: Rows <= 16 and Cols <= 64!\n";
    report_fatal_error(Twine(OS.str()));
  }
  Value *Rows = Builder.getInt16(ResRows);
  Value *Cols = Builder.getInt16(ResCols);
  Value *Ptr = II->getOperand(1)->getType()->getPointerAddressSpace() == 0
                   ? Builder.CreateBitCast(
                         II->getOperand(1),
                         llvm::PointerType::getUnqual(Builder.getContext()))
                   : Builder.CreateAddrSpaceCast(
                         II->getOperand(1),
                         llvm::PointerType::getUnqual(Builder.getContext()));
  // Create the argument list
  std::array<Value *, 5> Args{
      Rows, Cols, Ptr,
      Builder.CreateMul(II->getOperand(2), Builder.getInt64(SizeFactor)),
      Builder.CreateIntrinsic(Intrinsic::x86_cast_vector_to_tile,
                              {II->getOperand(0)->getType()},
                              {II->getOperand(0)})};
  Value *NewInst = Builder.CreateIntrinsic(Intrinsic::x86_tilestored64_internal,
                                           std::nullopt, Args);
  II->replaceAllUsesWith(NewInst);
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixMad(IntrinsicInst *II) {
  // Transform %mad = call <4 x i8>
  // @llvm.experimental.matrix.mad.v4i8.v8i8.v8i8( <8 x i8> %A,
  // <8 x i8> %B, <4 x i32>, %C, i32 2(A.rows), i32 4(B.rows), i32 2(C.cols),
  // metadata !"scope.subgroup").
  // into:
  // %a = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i8> %A).
  // %b = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i8> %B).
  // %c = call x86_amx @llvm.x86.cast.vector.to.tile.v4i8(<4 x i32> %C).
  // %d = call x86_amx @llvm.x86.tdpbssd.internal(i16 2(C.rows), i16 2*4(C.cols
  // in int8), i16 4(A.cols), x86_amx %c, x86_amx %a, x86_amx %b).
  // A.cols = B.rows, C.rows = A.rows, C.cols in int8 = B.rows * 4.
  IRBuilder<> Builder(II);
  FixedVectorType *MatrixType = cast<FixedVectorType>(II->getType());
  Type *DstMatrixElemType = MatrixType->getElementType();
  Type *SrcMatrixElemType =
      cast<FixedVectorType>(II->getOperand(0)->getType())->getElementType();
  Intrinsic::ID IID;
  switch (II->getIntrinsicID()) {
  default:
    assert(false && "Invalid Intrinsic ID!");
    break;
  case Intrinsic::experimental_matrix_mad:
    if (SrcMatrixElemType->isFloatTy() && DstMatrixElemType->isFloatTy()) {
#if INTEL_FEATURE_ISA_AMX_TF32
      IID = Intrinsic::x86_tmmultf32ps_internal;
#else  // INTEL_FEATURE_ISA_AMX_TF32
      report_fatal_error(
          "unsupported Matrix type: A&B is tf32 and C is float!");
#endif // INTEL_FEATURE_ISA_AMX_TF32
    } else if (SrcMatrixElemType->isIntegerTy(16) &&
               DstMatrixElemType->isFloatTy()) {
      IID = Intrinsic::x86_tdpbf16ps_internal;
    } else if (SrcMatrixElemType->isIntegerTy(8) &&
               DstMatrixElemType->isIntegerTy(32)) {
      IID = Intrinsic::x86_tdpbssd_internal;
    } else if (SrcMatrixElemType->isHalfTy() &&
               DstMatrixElemType->isFloatTy()) {
      IID = Intrinsic::x86_tdpfp16ps_internal;
      break;
    } else {
      report_fatal_error("unsupported Matrix type of matrix.mad!");
    }
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
      Builder.getInt16(cast<ConstantInt>(II->getOperand(3))->getSExtValue());
  // K is measured by bytes
  Value *K =
      Builder.getInt16(cast<ConstantInt>(II->getOperand(4))->getSExtValue() *
                       (SrcMatrixElemType->getPrimitiveSizeInBits() / 8));
  // N is measured by bytes
  Value *N = Builder.getInt16(
      cast<ConstantInt>(II->getOperand(5))->getSExtValue() * 4);
  // M=A.rows, N=C.cols*4, K=B.rows,C,A,B
  std::array<Value *, 6> Args{
      M,
      N,
      K,
      Builder.CreateIntrinsic(Intrinsic::x86_cast_vector_to_tile,
                              {II->getOperand(2)->getType()},
                              {II->getOperand(2)}),
      Builder.CreateIntrinsic(Intrinsic::x86_cast_vector_to_tile,
                              {II->getOperand(0)->getType()},
                              {II->getOperand(0)}),
      Builder.CreateIntrinsic(Intrinsic::x86_cast_vector_to_tile,
                              {II->getOperand(1)->getType()},
                              {II->getOperand(1)})};
  Value *NewInst = Builder.CreateIntrinsic(IID, std::nullopt, Args);
  II->replaceAllUsesWith(Builder.CreateIntrinsic(
      Intrinsic::x86_cast_tile_to_vector, {MatrixType}, {NewInst}));
  II->eraseFromParent();
  return true;
}

static AllocaInst *createAllocaInstAtEntry(IRBuilder<> &Builder, BasicBlock *BB,
                                           Type *Ty) {
  Function &F = *BB->getParent();
  Module *M = BB->getModule();
  const DataLayout &DL = M->getDataLayout();

  LLVMContext &Ctx = Builder.getContext();
  auto AllocaAlignment = DL.getPrefTypeAlign(Type::getX86_AMXTy(Ctx));
  unsigned AllocaAS = DL.getAllocaAddrSpace();
  AllocaInst *AllocaRes =
      new AllocaInst(Ty, AllocaAS, "", &F.getEntryBlock().front());
  AllocaRes->setAlignment(AllocaAlignment);
  return AllocaRes;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixExtractRowSlice(
    IntrinsicInst *II) {
  // Transform
  // %slice = call <4 x i32>
  // @llvm.experimental.matrix.extract.row.slice.v8i32(<16 x i32> %mat, i32 %x,
  // i32 %y, i32 4, i32 4, i32 4, metadata !"matrix.rowmajor") into
  // =>
  // alloc <16 x i32>* %ptr
  // store <16 x i32> %mat, <16 x i32>* %ptr
  // %i32_base = bitcast <16 x i32>* %ptr to i32*
  // %i32_ptr = GEP i32 i32* %i32_base offset
  // %slice_ptr = bitcast i8* %i32_ptr to <4 x i32>*
  // %slice = load <4 x i32>, <4 x i32>* slice_ptr;
  IRBuilder<> Builder(II);
  FixedVectorType *SliceType = cast<FixedVectorType>(II->getType());

  Metadata *MDLayout = cast<MetadataAsValue>(II->getOperand(6))->getMetadata();
  if (!cast<MDString>(MDLayout)->getString().equals("matrix.rowmajor")) {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted Layout:" << cast<MDString>(MDLayout)->getString() << "!\n"
       << "We support layout for slicing: matrix.rowmajor!\n";
    report_fatal_error(Twine(OS.str()));
  }
  Value *AllocaAddr = createAllocaInstAtEntry(Builder, II->getParent(),
                                              II->getOperand(0)->getType());
  Builder.CreateStore(II->getOperand(0), AllocaAddr);
  Value *Base = Builder.CreateBitCast(
      AllocaAddr, PointerType::getUnqual(SliceType->getElementType()));
  // offset = which_row*num_of_cols+which_col
  Value *Offset =
      Builder.CreateAdd(Builder.CreateMul(II->getOperand(1), II->getOperand(5)),
                        II->getOperand(2));
  Value *ElemPtr = Builder.CreateGEP(SliceType->getElementType(), Base, Offset);
  Value *SlicePtr =
      Builder.CreateBitCast(ElemPtr, PointerType::getUnqual(SliceType));
  Value *DstSlice = Builder.CreateLoad(SliceType, SlicePtr);
  II->replaceAllUsesWith(DstSlice);
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixInsertRowSlice(
    IntrinsicInst *II) {
  // Transform %resmat = call <16 x i32>
  // @llvm.experimental.matrix.insert.row.slice.v8i32(<16 x i32> %src, <4 x i32>
  // %slice, i32 %x, i32 %y, i32 4, i32 4, i32 4, metadata !"matrix.rowmajor")
  // =>
  // alloc <16 x i32>* %ptr
  // store <16 x i32> %src, <16 x i32>* %ptr
  // %i32_base = bitcast <16 x i32>* %ptr to i32*
  // %i32_ptr = GEP i32 i32* %i32_base offset
  // %slice_ptr = bitcast i8* %i32_ptr to <4 x i32>*
  // store <4 x i32> slice, <4 x i32>* slice_ptr;
  IRBuilder<> Builder(II);
  FixedVectorType *MatrixType = cast<FixedVectorType>(II->getType());
  FixedVectorType *SliceType =
      cast<FixedVectorType>(II->getOperand(1)->getType());

  Metadata *MDLayout = cast<MetadataAsValue>(II->getOperand(7))->getMetadata();
  if (!cast<MDString>(MDLayout)->getString().equals("matrix.rowmajor")) {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted Layout:" << cast<MDString>(MDLayout)->getString() << "!\n"
       << "We support layout for slicing: matrix.rowmajor!\n";
    report_fatal_error(Twine(OS.str()));
  }

  Value *AllocaAddr = createAllocaInstAtEntry(Builder, II->getParent(),
                                              II->getOperand(0)->getType());
  Builder.CreateStore(II->getOperand(0), AllocaAddr);
  Value *Base = Builder.CreateBitCast(
      AllocaAddr, PointerType::getUnqual(SliceType->getElementType()));
  // offset = which_row*num_of_cols+which_col
  Value *Offset =
      Builder.CreateAdd(Builder.CreateMul(II->getOperand(2), II->getOperand(6)),
                        II->getOperand(3));
  Value *ElemPtr = Builder.CreateGEP(SliceType->getElementType(), Base, Offset);
  Value *SlicePtr =
      Builder.CreateBitCast(ElemPtr, PointerType::getUnqual(SliceType));
  Builder.CreateStore(II->getOperand(1), SlicePtr);
  Value *ResMat = Builder.CreateLoad(MatrixType, AllocaAddr);
  II->replaceAllUsesWith(ResMat);
  II->eraseFromParent();
  return true;
}

bool X86LowerMatrixIntrinsicsPass::ProcessMatrixFill(IntrinsicInst *II) {
  // Transform
  // %fill = call <16 x i8> @llvm.experimental.matrix.fill.v16i8(i8 0, i32 4,
  // i32 4, metadata !"matrix.packed.b", metadata !"scope.subgroup")
  // =>
  // %tmp0 = call x86_amx @llvm.x86.tilezero.internal(i16 1, i16 16)
  // %tmp1 = call <16 x i8> @llvm.x86.cast.tile.to.vector.v16i8(x86_amx %tmp0)
  //
  // In the unified matrix, we switch to check the matrixuse instead of
  // matrixlayout, we don't care the value of matrixlayout metadata.
  //
  // %fill = call <16 x i8> @llvm.experimental.matrix.fill.v16i8(i8 0, i32 4,
  // i32 4, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata
  // !"matrix.use.b")
  // =>
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
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted MatrixElemType:" << MatrixElemType << "!\n"
       << "AMX provides support for int8_t, uint8_t, int32_t, bf16 and "
          "float!\n";
    report_fatal_error(Twine(OS.str()));
  }

  if ((MatrixElemType->isIntegerTy(16) || MatrixElemType->isHalfTy())) {
    SizeFactor = 2;
  } else if (MatrixElemType->isFloatTy() || MatrixElemType->isIntegerTy(32))
    SizeFactor = 4;
  else if (MatrixElemType->isIntegerTy(8))
    SizeFactor = 1;
  else {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted MatrixElemType:" << MatrixElemType << "!\n"
       << "AMX provides support for int8_t, uint8_t, int32_t, bf16 and "
          "float!\n";
    report_fatal_error(Twine(OS.str()));
  }
  Metadata *MatUse = cast<MetadataAsValue>(II->getOperand(4))->getMetadata();
  // If it is packed_b, the type can only be int8/bf16.
  // If it is row_major, the type can be int8/bf16/float/int32, Factor can only
  // be 1.
  if (cast<MDString>(MatUse)->getString().equals("matrix.use.b") &&
      MatrixElemType->isIntegerTy(8))
    Factor = 4;
  else if (cast<MDString>(MatUse)->getString().equals("matrix.use.b") &&
           (MatrixElemType->isIntegerTy(16) || MatrixElemType->isHalfTy()))
    Factor = 2;
  else if (cast<MDString>(MatUse)->getString().equals("matrix.use.a") ||
           cast<MDString>(MatUse)->getString().equals("matrix.use.accumulator"))
    Factor = 1;
  else {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsuppoted matrix use:" << cast<MDString>(MatUse)->getString()
       << "!\n"
       << "We support matrix use: matrix.use.a, matrix.use.b and "
          "matrix.use.accumulator!\n";
    report_fatal_error(Twine(OS.str()));
  }
  // Handle cases where it is vxi8 and packedb.
  assert(MRows >= Factor && MRows % Factor == 0 &&
         "Invalid Matrix Rows Value!");
  int64_t ResRows = MRows / Factor;
  int64_t ResCols = MCols * Factor * SizeFactor;
  if (ResRows > 16 || ResCols > 64) {
    std::string Str;
    raw_string_ostream OS(Str);
    OS << "Unsupported Size for tilezero! Rows = " << ResRows
       << "Cols = " << ResCols << "!\n"
       << "We support Size: Rows <= 16 and Cols <= 64!\n";
    report_fatal_error(Twine(OS.str()));
  }
  Value *Rows = Builder.getInt16(ResRows);
  Value *Cols = Builder.getInt16(ResCols);
  // Create the argument list
  std::array<Value *, 2> Args{Rows, Cols};
  Value *NewInst = Builder.CreateIntrinsic(Intrinsic::x86_tilezero_internal,
                                           std::nullopt, Args);
  II->replaceAllUsesWith(Builder.CreateIntrinsic(
      Intrinsic::x86_cast_tile_to_vector, {MatrixType}, {NewInst}));
  II->eraseFromParent();
  return true;
}
