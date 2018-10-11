//===--- TBAATest.cpp - Mixed TBAA unit tests -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/AliasAnalysisEvaluator.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"   // INTEL
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "gtest/gtest.h"

namespace llvm {
namespace {

class TBAATest : public testing::Test {
protected:
  TBAATest() : M("TBAATest", C), MD(C) {}

  LLVMContext C;
  Module M;
  MDBuilder MD;
  TypeBasedAAResult TBAA;   // INTEL
};

static StoreInst *getFunctionWithSingleStore(Module *M, StringRef Name) {
  auto &C = M->getContext();
  FunctionType *FTy = FunctionType::get(Type::getVoidTy(C), {});
  auto *F = cast<Function>(M->getOrInsertFunction(Name, FTy));
  auto *BB = BasicBlock::Create(C, "entry", F);
  auto *IntType = Type::getInt32Ty(C);
  auto *PtrType = Type::getInt32PtrTy(C);
  auto *SI = new StoreInst(ConstantInt::get(IntType, 42),
                           ConstantPointerNull::get(PtrType), BB);
  ReturnInst::Create(C, nullptr, BB);

  return SI;
}

#if INTEL_CUSTOMIZATION
static std::pair <StoreInst*, LoadInst*> getFunctionWithLoadStore(Module *M, StringRef Name) {
  auto &C = M->getContext();
  FunctionType *FTy = FunctionType::get(Type::getVoidTy(C), {});
  auto *F = cast<Function>(M->getOrInsertFunction(Name, FTy));
  auto *BB = BasicBlock::Create(C, "entry", F);

  auto *CharType = Type::getInt8Ty(C);
  auto *CharGlobal = new GlobalVariable(*M, CharType, false,
                                    GlobalValue::ExternalLinkage, nullptr);
  auto *SI = new StoreInst(ConstantInt::get(CharType, 10),
                           dyn_cast<Value>(CharGlobal), BB);

  auto *IntPtrType = Type::getInt32PtrTy(C);
  auto *LI = new LoadInst(ConstantPointerNull::get(IntPtrType), "load", BB);

  ReturnInst::Create(C, nullptr, BB);

  return std::make_pair(SI, LI);
}
#endif // INTEL_CUSTOMIZATION

TEST_F(TBAATest, checkVerifierBehaviorForOldTBAA) {
  auto *SI = getFunctionWithSingleStore(&M, "f1");
  auto *F = SI->getFunction();

  // C++ unit test case to avoid going through the auto upgrade logic.
  auto *RootMD = MD.createTBAARoot("Simple C/C++ TBAA");
  auto *MD1 = MD.createTBAANode("omnipotent char", RootMD);
  auto *MD2 = MD.createTBAANode("int", MD1);
  SI->setMetadata(LLVMContext::MD_tbaa, MD2);

  SmallVector<char, 0> ErrorMsg;
  raw_svector_ostream Outs(ErrorMsg);

  StringRef ExpectedFailureMsg(
      "Old-style TBAA is no longer allowed, use struct-path TBAA instead");

  EXPECT_TRUE(verifyFunction(*F, &Outs));
  EXPECT_TRUE(StringRef(ErrorMsg.begin(), ErrorMsg.size())
                  .startswith(ExpectedFailureMsg));
}

TEST_F(TBAATest, checkTBAAMerging) {
  auto *SI = getFunctionWithSingleStore(&M, "f2");
  auto *F = SI->getFunction();

  auto *RootMD = MD.createTBAARoot("tbaa-root");
  auto *MD1 = MD.createTBAANode("scalar-a", RootMD);
  auto *StructTag1 = MD.createTBAAStructTagNode(MD1, MD1, 0);
  auto *MD2 = MD.createTBAANode("scalar-b", RootMD);
  auto *StructTag2 = MD.createTBAAStructTagNode(MD2, MD2, 0);

  auto *GenericMD = MDNode::getMostGenericTBAA(StructTag1, StructTag2);

  EXPECT_EQ(GenericMD, nullptr);

  // Despite GenericMD being nullptr, we expect the setMetadata call to be well
  // defined and produce a well-formed function.
  SI->setMetadata(LLVMContext::MD_tbaa, GenericMD);

  EXPECT_TRUE(!verifyFunction(*F));
}

#if INTEL_CUSTOMIZATION
// This test checks if TBAA applies its rule commutatively i.e.,
// it returns the same alias results for the same pair of values
// even if their ordering is different.
// JIRA: LCPT-1366
TEST_F(TBAATest, checkTBAACommutativity) {
  std::pair <StoreInst*, LoadInst*> LoadStore = getFunctionWithLoadStore(&M, "f3");
  StoreInst *SI = LoadStore.first;
  LoadInst *LI = LoadStore.second;

  auto *RootMD = MD.createTBAARoot("tbaa-root");
  auto *MDChar = MD.createTBAANode("omnipotent char", RootMD);
  auto *CharAccessTag = MD.createTBAAStructTagNode(MDChar, MDChar, 0);
  auto *MDInt = MD.createTBAANode("int", MDChar);
  auto *IntAccessTag = MD.createTBAAStructTagNode(MDInt, MDInt, 0);

  SI->setMetadata(LLVMContext::MD_tbaa, CharAccessTag);
  LI->setMetadata(LLVMContext::MD_tbaa, IntAccessTag);

  Value *GlobalVal = SI->getPointerOperand();
  Type *CharType = GlobalVal->getType()->getPointerElementType();
  (void)CharType;
  assert((isa<GlobalValue>(GlobalVal) &&
          CharType->getPrimitiveSizeInBits() == 8) &&
          "Variable should be a Global char!");

  const MemoryLocation Loc1 = MemoryLocation::get(SI);
  const MemoryLocation Loc2 = MemoryLocation::get(LI);

  auto AliasResult1 = TBAA.alias(Loc1, Loc2);
  auto AliasResult2 = TBAA.alias(Loc2, Loc1);

  EXPECT_EQ(AliasResult1, MayAlias);
  EXPECT_EQ(AliasResult2, MayAlias);
}
#endif // INTEL_CUSTOMIZATION

} // end anonymous namspace
} // end llvm namespace
