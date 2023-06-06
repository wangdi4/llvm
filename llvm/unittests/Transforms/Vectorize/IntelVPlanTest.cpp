//===- llvm/unittest/Transforms/Vectorize/IntelVPlanTest.cpp --------------===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanTestBase.h"

#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

using VPInstructionPtr =
    std::unique_ptr<VPInstruction, DropAllReferencesDeleter>;

class VPUtilsTest : public testing::Test {
public:
  VPUtilsTest() {}

protected:
  LLVMContext C;
  std::unique_ptr<Module> M;
  std::unique_ptr<DataLayout> DL;
  std::unique_ptr<VPExternalValues> Externals;
  SmallVector<VPInstructionPtr, 16> VPInsts;

  template <unsigned N> VPConstantInt *makeGEPIndex(int64_t Idx) {
    auto *CI = ConstantInt::getIntegerValue(IntegerType::getIntNTy(C, N),
                                            APInt{N, (uint64_t)Idx, true});
    return cast<VPConstantInt>(Externals->getVPConstant(CI));
  }

  void SetUp() override {
    M.reset(new Module("GEPTest", C));
    DL.reset(new DataLayout(M.get()));
    Externals.reset(new VPExternalValues(M.get()));
  };

  void TearDown() override {
    VPInsts.clear();
    Externals.reset();
    DL.reset();
    M.reset();
  }

  template <typename Inst, typename... ArgTys>
  Inst *makeInst(ArgTys &&...Args) {
    auto *I = new Inst(std::forward<ArgTys>(Args)...);
    VPInsts.emplace_back(I);
    return I;
  }
};

TEST_F(VPUtilsTest, GEPIter) {
  // %Base.ty = type { [256 x i32] }
  auto *BaseTy = StructType::get(
      C, {ArrayType::get(IntegerType::getInt32Ty(C), 256)}, false);

  // @base = %Base.ty* null
  VPValue *BasePtr = Externals->getVPConstant(
      ConstantPointerNull::get(BaseTy->getPointerTo()));

  // getelementptr {[256 x i32]}, {[256 x i32]}* %base, i64 0, i32 0, i64 32
  auto *GEP =
      makeInst<VPGEPInstruction>(BaseTy, BaseTy, BasePtr->getType(), BasePtr,
                                 ArrayRef<VPValue *>{
                                     makeGEPIndex<64>(0),
                                     makeGEPIndex<32>(0),
                                     makeGEPIndex<64>(32),
                                 });

  vp_gep_type_iterator I = gep_type_begin(*GEP);
  vp_gep_type_iterator E = gep_type_end(*GEP);

  // CurTy = {[256 x i32]}*, IndexedType = {[256 x i32]}, Op = i64 0
  ASSERT_NE(I, E);
  EXPECT_FALSE(I.getStructTypeOrNull());
  EXPECT_TRUE(I.getIndexedType()->isStructTy());
  EXPECT_TRUE(isa<VPConstantInt>(I.getOperand()));
  ++I;

  // CurTy = {[256 x i32]}, IndexedType = [256 x i32], Op = 32 0
  ASSERT_NE(I, E);
  EXPECT_TRUE(I.getStructTypeOrNull());
  EXPECT_TRUE(I.getIndexedType()->isArrayTy());
  EXPECT_TRUE(isa<VPConstantInt>(I.getOperand()));
  ++I;

  // CurTy = [256 x i32], IndexedType = i32, Op = i64 32
  ASSERT_NE(I, E);
  EXPECT_FALSE(I.getStructTypeOrNull());
  EXPECT_TRUE(I.getIndexedType()->isIntegerTy(32));
  EXPECT_TRUE(isa<VPConstantInt>(I.getOperand()));
  ++I;

  ASSERT_EQ(I, E);
}
