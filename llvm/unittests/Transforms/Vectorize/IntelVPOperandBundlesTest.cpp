//===- llvm/unittest/Transforms/Vectorize/IntelVPOperandBundlesTest.cpp ---===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h"
#include "IntelVPlanTestBase.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

class VPOperandBundlesTest : public vpo::VPlanTestBase {
protected:
  Function *Foo;
  std::unique_ptr<VPlanVector> Plan;

  void buildVPlanFromString(const char *ModuleString) {
    Module &M = parseModule(ModuleString);
    Foo = M.getFunction("foo");
    BasicBlock *LoopHeader = Foo->getEntryBlock().getSingleSuccessor();
    Plan = buildHCFG(LoopHeader);
  }

  void setup() {
    buildVPlanFromString(
        "declare void @llvm.assume(i1)\n"
        "define void @foo(ptr %p) {\n"
        "entry:\n"
        "  br label %for.body\n"
        "for.body:\n"
        "  %ind = phi i32 [ 0, %entry ], [ %ind.next, %for.body ]\n"
        "  call void @llvm.assume(i1 true) [ \"align\"(ptr %p, i32 16), "
        "\"nonnull\"(ptr %p) ]\n"
        "  store i32 1, ptr %p\n"
        "  %ind.next = add nuw nsw i32 %ind, 1\n"
        "  %cond = icmp eq i32 %ind.next, 256\n"
        "  br i1 %cond, label %for.body, label %exit\n"
        "exit:\n"
        "  ret void\n"
        "}\n");
  }

  SmallVector<const VPCallInstruction *, 1> getCallInsts() const {
    return SmallVector<const VPCallInstruction *, 1>(map_range(
        make_filter_range(
            vpinstructions(Plan.get()),
            [](const VPInstruction &I) { return isa<VPCallInstruction>(&I); }),
        [](const VPInstruction &I) { return cast<VPCallInstruction>(&I); }));
  }
};

#define ASSERT_BUNDLE_EQ(LHS, RHS)                                             \
  do {                                                                         \
    EXPECT_EQ((LHS).Tag, (RHS).Tag);                                           \
    ASSERT_EQ((LHS).Inputs.size(), (RHS).Inputs.size());                       \
    for (unsigned I = 0; I < (LHS).Inputs.size(); ++I)                         \
      EXPECT_EQ((LHS).Inputs[I], (RHS).Inputs[I]);                             \
  } while (0)

#define SETUP_AND_ASSERT_SINGLE_CALL(CALL)                                     \
  do {                                                                         \
    setup();                                                                   \
                                                                               \
    const auto Calls = getCallInsts();                                         \
    ASSERT_EQ(Calls.size(), (size_t)1);                                        \
                                                                               \
    CALL = Calls[0];                                                           \
    ASSERT_TRUE(Call->hasOperandBundles());                                    \
  } while (0)

TEST_F(VPOperandBundlesTest, BundleOpStatistics) {
  const VPCallInstruction *Call;
  SETUP_AND_ASSERT_SINGLE_CALL(Call);

  EXPECT_TRUE(Call->hasOperandBundles());
  EXPECT_EQ(Call->getNumOperandBundles(), (unsigned int)2);
  EXPECT_EQ(Call->getNumTotalBundleOperands(), (unsigned int)3);
  EXPECT_FALSE(Call->bundle_operands().empty());
}

TEST_F(VPOperandBundlesTest, getOperandBundleAt) {
  const VPCallInstruction *Call;
  SETUP_AND_ASSERT_SINGLE_CALL(Call);

  const VPOperandBundle AlignBundle = Call->getOperandBundleAt(0);
  EXPECT_EQ(AlignBundle.Tag, "align");
  EXPECT_EQ(AlignBundle.Inputs.size(), (size_t)2);

  const VPValue *AlignedVal = AlignBundle.Inputs[0];
  ASSERT_EQ(AlignedVal->getVPValueID(), VPConstant::VPExternalDefSC);
  EXPECT_EQ(cast<VPExternalDef>(AlignedVal)->getOrigName(), "p");

  const VPValue *Alignment = AlignBundle.Inputs[1];
  ASSERT_EQ(Alignment->getVPValueID(), VPConstant::VPConstantSC);
  EXPECT_EQ(cast<VPConstant>(Alignment)->getZExtValue(), (uint64_t)16);

  const VPOperandBundle NonNullBundle = Call->getOperandBundleAt(1);
  EXPECT_EQ(NonNullBundle.Tag, "nonnull");
  EXPECT_EQ(NonNullBundle.Inputs.size(), (size_t)1);

  const VPValue *NonNullVal = NonNullBundle.Inputs[0];
  ASSERT_EQ(NonNullVal->getVPValueID(), VPConstant::VPExternalDefSC);
  EXPECT_EQ(cast<VPExternalDef>(NonNullVal)->getOrigName(), "p");
  EXPECT_EQ(NonNullVal, AlignedVal);
}

TEST_F(VPOperandBundlesTest, getOperandBundles) {
  const VPCallInstruction *Call;
  SETUP_AND_ASSERT_SINGLE_CALL(Call);

  SmallVector<VPOperandBundle, 1> Bundles;
  Call->getOperandBundles(Bundles);
  ASSERT_EQ(Bundles.size(), (size_t)2);
  ASSERT_BUNDLE_EQ(Bundles[0], Call->getOperandBundleAt(0));
  ASSERT_BUNDLE_EQ(Bundles[1], Call->getOperandBundleAt(1));
}

TEST_F(VPOperandBundlesTest, getOperandBundleFromTag) {
  const VPCallInstruction *Call;
  SETUP_AND_ASSERT_SINGLE_CALL(Call);

  const std::optional<VPOperandBundle> AlignBundle =
      Call->getOperandBundle("align");
  ASSERT_TRUE(AlignBundle.has_value());
  ASSERT_BUNDLE_EQ(*AlignBundle, Call->getOperandBundleAt(0));

  const std::optional<VPOperandBundle> NonNullBundle =
      Call->getOperandBundle("nonnull");
  ASSERT_TRUE(NonNullBundle.has_value());
  ASSERT_BUNDLE_EQ(*NonNullBundle, Call->getOperandBundleAt(1));

  ASSERT_EQ(Call->getOperandBundle("nonexistent"), std::nullopt);
}
