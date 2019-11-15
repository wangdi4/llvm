//=== --------- Intel_InlineReiortSetup.cpp - Inlining Report Setup ------=== //
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file describes inlining report setup process.
//
// I. If there is no inlining report for function yet, the setupInlineReport()
// creates inlining report for each function and call instruction of the module
// linking them into the metadata inline report structure. (Compile step)
// II. If inlining report exists, the pass verifies that all call sites that
// appear in the code have corresponding call site inlining report nodes in the
// function inlining report. If metadata node is detached from the instruction,
// it will be linked back. If inlining report is missing for the call site, then
// the least common ancestor node for the surrounding call sites will be found
// and the newly created inlining report will be inserted into the ancestor
// node. (Link step)
//
//===----------------------------------------------------------------------===//
//
#include "llvm/Transforms/IPO/Intel_InlineReportSetup.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;
using namespace MDInliningReport;

extern cl::opt<unsigned> IntelInlineReportLevel;

#define DEBUG_TYPE "inlinereportsetup"


// Walking metadata nodes is not convenient. To make the procedure of inserting
// new nodes easier I use auxiliary structure: InlineReportTree. It replicates
// the structure of the metadata tree with additional information extremely
// useful to walk the tree, find least common ancestor node and updating current
// inline report metadata and tree.
class InlineReportTreeNode {
public:
  // Name, line and column number to match call instruction to metadata.
  std::string Name;
  unsigned Line;
  unsigned Col;
  // Index in the parent metadata dependents' list of current node
  unsigned Idx;
  // Depth in the inline report tree
  unsigned Depth;
  // Pointer to the metadata node corresponding to the auxiliary tree node.
  MDNode *InlineReport;
  // Pointer to the metadata corresponding to the dependents of the current
  // node..
  MDNode *CSs;
  // Parent auxiliary tree node.
  InlineReportTreeNode *Parent;
  // List of auxiliary tree dependent nodes.
  std::vector<InlineReportTreeNode *> Children;

  // Function to insert new inlining report into the current inline report tree.
  InlineReportTreeNode *insertNewChild(Instruction *CallI, unsigned InsertAt,
                                       InlineReportBuilder &MDIR);

#ifndef NDEBUG
  void dump() {
    errs() << "\n\tNode(" << this << ", name:" << Name << ", line:" << Line;
    errs() << ", col:" << Col << ", idx:" << Idx << ", depth:" << Depth;
    errs() << ", ir:" << InlineReport << ", css:" << CSs << ", parent:";
    errs() << Parent << ", numChildren:" << Children.size() << ")";
  }

  void dumpRecursive() {
    dump();
    for (auto *Node : Children)
      Node->dumpRecursive();
  }
#endif // NDEBUG
};

// Function to insert new inlining report into the current inline report tree.
InlineReportTreeNode *
InlineReportTreeNode::insertNewChild(Instruction *CallI, unsigned InsertAt,
                                     InlineReportBuilder &MDIR) {
  auto CB = dyn_cast<CallBase>(CallI);
  assert(CB && "Expected call instruction");
  LLVMContext &C = CallI->getParent()->getParent()->getContext();
  // Create and fill up new InlineReportTreeNode.
  InlineReportTreeNode *NewChild = new InlineReportTreeNode();
  Function *Callee = CB->getCalledFunction();
  NewChild->Name = Callee ? (Callee->hasName() ? Callee->getName() : "") : "";

  // Create inlining report metadata and attach it to the call instruction if it
  // has no attached metadata yet.
  CallSiteInliningReport *CSIR = nullptr;
  if (MDNode *CSMD = CallI->getMetadata(CallSiteTag)) {
    CSIR = new CallSiteInliningReport(cast<MDTuple>(CSMD));
  } else {
    InlineReason Reason = NinlrNoReason;
    if (isa<IntrinsicInst>(CallI))
      Reason = NinlrIntrinsic;
    else if (Callee && Callee->isDeclaration())
      Reason = NinlrExtern;
    CSIR = new CallSiteInliningReport(CB, nullptr, Reason);
    CallI->setMetadata(CallSiteTag, CSIR->get());
    MDIR.addCallback(CallI, CSIR->get());
  }

  // Update list of callsites for parent inlining report metadata
  SmallVector<Metadata *, 100> Ops;
  Ops.push_back(llvm::MDString::get(C, CallSitesTag));
  if (CSs) {
    unsigned CSsNumOps = CSs->getNumOperands();
    for (unsigned I = 1; I < CSsNumOps; ++I) {
      if (I == InsertAt + 1)
        Ops.push_back(CSIR->get());
      Ops.push_back(CSs->getOperand(I));
    }
    if (InsertAt + 1 == CSsNumOps)
      Ops.push_back(CSIR->get());
  } else {
    assert(Children.empty() && "Inline report setup: empty children list");
    assert((InsertAt == 0) && "Inline report setup: wrong index");
    Ops.push_back(CSIR->get());
  }
  MDNode *NewCSs = MDTuple::getDistinct(C, Ops);
  InlineReport->replaceOperandWith(CSMDIR_CSs, NewCSs);
  CSs = NewCSs;

  // Fillup info
  unsigned LineNum = 0, ColNum = 0;
  CSIR->getLineAndCol(&LineNum, &ColNum);
  NewChild->Line = LineNum;
  NewChild->Col = ColNum;
  NewChild->Idx = InsertAt;
  NewChild->Depth = Depth + 1;
  NewChild->InlineReport = CSIR->get();
  NewChild->CSs = dyn_cast_or_null<MDNode>(CSIR->get()->getOperand(CSMDIR_CSs));
  NewChild->Parent = this;

  // Update children list of current metadata node
  auto It = Children.begin() + InsertAt;
  Children.insert(It, NewChild);

  // Update index values for all subsequent children in the parent
  // InlineReportTreeNode.
  for (unsigned I = InsertAt + 1; I < Children.size(); ++I)
    Children[I]->Idx += 1;
  return NewChild;
}

// Create an auxiliary tree node for the current metadata node.
InlineReportTreeNode *buildNode(MDTuple *MDT, unsigned Idx, unsigned Depth,
                                InlineReportTreeNode *Parent) {
  if (!MDT)
    return nullptr;

  unsigned NumOperands = MDT->getNumOperands();

  // Create inline report tree node.
  InlineReportTreeNode *TreeNode = new InlineReportTreeNode();
  InliningReport IR(MDT);
  TreeNode->Name = IR.getName();
  unsigned LineNum = 0, ColNum = 0;
  if (NumOperands == CallSiteMDSize) {
    CallSiteInliningReport CSIR(MDT);
    CSIR.getLineAndCol(&LineNum, &ColNum);
  }
  // Initialize inline report tree node.
  TreeNode->Idx = Idx;
  TreeNode->Depth = Depth;
  TreeNode->Line = LineNum;
  TreeNode->Col = ColNum;
  TreeNode->InlineReport = MDT;
  TreeNode->Parent = Parent;

  // Walk over tree node dependents and create nodes for them.
  if (MDT->getOperand(FMDIR_CSs)) {
    if (MDTuple *MDCSs = cast<MDTuple>(MDT->getOperand(FMDIR_CSs))) {
      TreeNode->CSs = MDCSs;
      for (unsigned I = 1; I < MDCSs->getNumOperands(); ++I) {
        MDTuple *MDChild = dyn_cast_or_null<MDTuple>(MDCSs->getOperand(I));
        InlineReportTreeNode *Child =
            buildNode(MDChild, I - 1, Depth + 1, TreeNode);
        if (Child)
          TreeNode->Children.push_back(Child);
      }
    } else {
      TreeNode->CSs = nullptr;
    }
  } else {
    TreeNode->CSs = nullptr;
  }
  return TreeNode;
}

// Build auxiliary inline report tree for the current metadata inline report
// of the function.
static InlineReportTreeNode *buildInlineReportTreeForFunction(Function *F) {
  if (!F || !F->getMetadata(FunctionTag))
    return nullptr;
  MDTuple *MDRoot = cast<MDTuple>(F->getMetadata(FunctionTag));
  InlineReportTreeNode *Res = buildNode(MDRoot, 0, 0, nullptr);
  return Res;
}

// Collect all leaf nodes of the auxiliary tree in the DFO into CSs vector.
static void collectIRCallSites(InlineReportTreeNode *Root,
                               std::vector<InlineReportTreeNode *> &CSs) {
  if (!Root)
    return;

  // If function have no callsites - return.
  if (Root->Children.size() == 0) {
    if (Root->Depth == 0)
      return;
    MDTuple *MDT = cast<MDTuple>(Root->InlineReport);
    if (MDT->getNumOperands() == CallSiteMDSize) {
      // If call site was inlined and has no children - skip it.
      int64_t IsInlined = 0;
      getOpVal(MDT->getOperand(CSMDIR_IsInlined), "isInlined: ", &IsInlined);
      if (IsInlined)
        return;
      // If call site was deleted, skip creating an inline report tree node
      // for it.
      int64_t Reason = InlineReportTypes::NinlrNoReason;
      getOpVal(MDT->getOperand(CSMDIR_InlineReason), "reason: ", &Reason);
      if (Reason == InlineReportTypes::NinlrDeleted)
        return;
    }
    CSs.push_back(Root);
  } else {
    for (auto *Node : Root->Children)
      collectIRCallSites(Node, CSs);
  }
}

// Free tne memory used for auxiliary inline report tree.
static void releaseInlineReportTreeForFunction(InlineReportTreeNode *Tree) {
  for (auto *Child : Tree->Children)
    releaseInlineReportTreeForFunction(Child);
  delete Tree;
}

// Find least common ancestor in the auxiliary tree to insert new inlining
// report node.
//
// Ex.:
// I consider the case when a, b and c were originally call sites of main, but
// after the first inlining pass they were inlined and replaced with k, o, m and
// n. Now, before the second inliner pass, I need to link the corresponding call site
// metadata nodes (which hang from function inlining report) to the real
// instruction in the function. I also take into account that there could appear
// new call sites due to other compiler optimizations.
//
//
//             main
//           /  |  \
//          a   b   c
//        / |   |   |
//       k  l   m   n
//          |
//          o
//
// According to this example we should see the following call sites in the
// function main():
//    main() {
//      k();
//      o();
//      m();
//      n()
//    }
//
//  If there were new call sites that appeared as a result of compiler
//  transformations we need to find an appropriate place to attach their
//  inlining reports.
//
//  Ex.:
//    main() {
//      k();
//      libcall1();
//      o();
//      m();
//      libcall2();
//      n();
//
// The resulting inline report tree will look as follows:
//              main
//           /    | |  \
//          a     b lc2 c
//        / | \   |     |
//       k lc1 l  m     n
//             |
//             o
//
static InlineReportTreeNode *
findLeastCommonAncestor(InlineReportTreeNode *Node1,
                        InlineReportTreeNode *Node2, unsigned *InsertAt) {
  assert(Node1 && Node2 && InsertAt && "Empty node in findLCA");
  *InsertAt = 0;
  if (Node1->Depth == Node2->Depth) {
    if (Node1->Parent == Node2->Parent) {
      *InsertAt = Node1->Idx + 1;
      return Node1->Parent;
    }
    return findLeastCommonAncestor(Node1->Parent, Node2->Parent, InsertAt);
  }
  if (Node1->Depth > Node2->Depth)
    return findLeastCommonAncestor(Node1->Parent, Node2, InsertAt);
  return findLeastCommonAncestor(Node1, Node2->Parent, InsertAt);
}

// Check if call instruction is matching to metadata node.
static bool matchCallSiteToMetadata(CallBase *CB, MDNode *MD) {
  assert(CB && MD && "Bad instruction or metadata");
  LLVM_DEBUG({
    dbgs() << "\nMDIR setup: match call site to metadata ";
    CB->dump();
    MD->dump();
  });
  // Consider the case when CallInst has inlining report attached.
  MDNode *CallIMetadata = CB->getMetadata(CallSiteTag);
  if (CallIMetadata) {
    if (CallIMetadata == MD)
      return true;
    return false;
  }

  // Consider the case when CallInst has no inlining report attached.
  InliningReport IR(cast<MDTuple>(MD));
  std::string MDNameStr = IR.getName();
  Function *CallICallee = CB->getCalledFunction();
  if (!CallICallee || !CallICallee->hasName()) {
    if (MDNameStr.empty())
      return true;
    return false;
  }

  // Compare inline report node to the call site.
  StringRef CIName = CallICallee->getName();
  if (MDNameStr == CIName)
    return true;
  return false;
}

// Main verification function. It collects a list of leaf nodes in the inlining
// report and tries to match it to the list of the actual callsites. Adding new
// nodes to the inlining report if needed.
static bool verifyFunctionInliningReport(Function *F,
                                         InlineReportBuilder &MDIR) {
  if (!F || !F->hasName())
    return false;

  LLVM_DEBUG(dbgs() << "\nMDIR setup: verify function inlining report for "
                    << F->getName() << "\n");
  InlineReportTreeNode *Root = buildInlineReportTreeForFunction(F);
  // Not-deleted and not-inlined inline report node for which we expect to find
  // corresponding instruction in the function.
  std::vector<InlineReportTreeNode *> IRCallSites;
  collectIRCallSites(Root, IRCallSites);
  // Call sites in function code. Skip some intrinsics for now.
  std::vector<CallBase *> FunctionCallSites;
  uint64_t NumCallSitesWithMDIR = 0;
  for (BasicBlock &BB : *F)
    for (Instruction &I : BB) {
      if (auto *II = dyn_cast<IntrinsicInst>(&I))
        if (!(MDIR.getLevel() & DontSkipIntrin) &&
            shouldSkipIntrinsic(II))
          continue;
      auto CB = dyn_cast<CallBase>(&I);
      if (!CB)
        continue;

      if (CB->getMetadata(CallSiteTag))
        NumCallSitesWithMDIR++;
      FunctionCallSites.push_back(CB);
    }

  // If number of call sites match and they all are linked to corresponding
  // metadata then we are all set. No need for further verification.
  if (IRCallSites.size() == FunctionCallSites.size() &&
      IRCallSites.size() == NumCallSitesWithMDIR) {
    releaseInlineReportTreeForFunction(Root);
    return true;
  }

  LLVM_DEBUG(dbgs() << "\nMDIR setup: verification started for " << F->getName()
                    << "\n");
  unsigned IRCSSize = IRCallSites.size(), FCSSize = FunctionCallSites.size();
  unsigned I = 0;
  if (IRCSSize != 0 && FCSSize != 0) {
    // Try to match inline report call sites to the function call sites
    if (!matchCallSiteToMetadata(FunctionCallSites[0],
                                 IRCallSites[0]->InlineReport)) {
      InlineReportTreeNode *NewIRCSNode =
          Root->insertNewChild(FunctionCallSites[0], 0, MDIR);
      auto It = IRCallSites.begin();
      IRCallSites.insert(It, NewIRCSNode);
    } else {
      FunctionCallSites[0]->setMetadata(CallSiteTag,
                                        IRCallSites[0]->InlineReport);
      MDIR.addCallback(FunctionCallSites[0], IRCallSites[0]->InlineReport);
    }

    for (I = 1; (I < IRCallSites.size()) && (I < FCSSize); ++I) {
      if (matchCallSiteToMetadata(FunctionCallSites[I],
                                  IRCallSites[I]->InlineReport)) {
        FunctionCallSites[I]->setMetadata(CallSiteTag,
                                          IRCallSites[I]->InlineReport);
        MDIR.addCallback(FunctionCallSites[I], IRCallSites[I]->InlineReport);
        continue;
      }
      unsigned InsertAt = 0;
      InlineReportTreeNode *LCA = findLeastCommonAncestor(
          IRCallSites[I - 1], IRCallSites[I], &InsertAt);
      // Didn't match => create IR tree node and add it to the list.
      InlineReportTreeNode *NewIRCSNode =
          LCA->insertNewChild(FunctionCallSites[I], InsertAt, MDIR);
      auto It = IRCallSites.begin() + I;
      IRCallSites.insert(It, NewIRCSNode);
    }
    if (I < IRCallSites.size()) {
      releaseInlineReportTreeForFunction(Root);
      return false;
    }
  }

  // If there are some trailing call sites without inlining reports left -
  // insert them into the function in the very end.
  unsigned InsertAt = Root->Children.size();
  for (; I < FCSSize; ++I) {
    InlineReportTreeNode *NewIRCSNode =
        Root->insertNewChild(FunctionCallSites[I], InsertAt++, MDIR);
    IRCallSites.push_back(NewIRCSNode);
  }
  releaseInlineReportTreeForFunction(Root);
  return true;
}

// Function to create initial function inlining report.
MDNode *createFunctionInliningReport(Function *F, InlineReportBuilder &MDIR) {
  std::vector<MDTuple *> CSs;
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      auto CB = dyn_cast<CallBase>(&I);
      // If this isn't a call, or it is a call to an intrinsic, it can
      // never be inlined.
      if (!CB)
        continue;
      InlineReason Reason = NinlrNoReason;
      if (auto *II = dyn_cast<IntrinsicInst>(&I)) {
        if (!(MDIR.getLevel() & DontSkipIntrin) &&
            shouldSkipIntrinsic(II))
          continue;
        Reason = NinlrIntrinsic;
      } else if (Function *Callee = CB->getCalledFunction()) {
        if (Callee->isDeclaration())
          Reason = NinlrExtern;
      }

      CallSiteInliningReport *CSIR =
          new CallSiteInliningReport(CB, nullptr, Reason);
      CB->setMetadata(CallSiteTag, CSIR->get());
      MDIR.addCallback(&I, CSIR->get());
      CSs.push_back(CSIR->get());
    }
  }
  FunctionInliningReport *NewFIR =
      new FunctionInliningReport(F, &CSs, false /*isDead*/);
  MDIR.addCallback(F, NewFIR->get());
  return NewFIR->get();
}

// This function implements two tasks:
// 1. if inlining report for function already exists - verify it.
// 2. if function doesn't have inlining report yet - create it.
static MDNode *
findOrCreateFunctionInliningReport(Function *F, NamedMDNode *ModuleInlineReport,
                                   InlineReportBuilder &MDIR) {
  if (!F->hasName())
    return nullptr;
  unsigned FuncCnt = ModuleInlineReport->getNumOperands();
  unsigned FuncIndex = 0;
  bool IsInModule = false;

  // 1. If we already have inlining report for function, then:
  //    a) check that it is attached to module inlining report
  //    b) verify that all call sites have inlining report metadata and those
  //    metadata nodes are attached to the appropriate instructions.
  MDNode *FIR = F->getMetadata(FunctionTag);
  if (FIR) {
    // TODO: change the algorithm to be O(n).
    for (; FuncIndex < FuncCnt; ++FuncIndex) {
      MDNode *IR = ModuleInlineReport->getOperand(FuncIndex);
      if (IR == FIR) {
        IsInModule = true;
        break;
      }
    }
    if (MDIR.getLevel() & CompositeReport) {
      if (!IsInModule) {
        // We do not expect to actually get in here because on the link step we
        // expect every function we encounter to already have an inlining report
        // in the metadata.
        ModuleInlineReport->addOperand(FIR);
        FuncIndex = ModuleInlineReport->getNumOperands() - 1;
      }

      // Now verify and re-connect callsites metadata if needed.
      if (verifyFunctionInliningReport(F, MDIR)) {
        MDIR.addCallback(F, FIR);
        LLVM_DEBUG(dbgs() << "MDIR setup: verification successful for "
                          << F->getName() << '\n');
        return FIR;
      }
      LLVM_DEBUG(dbgs() << "MDIR setup: verification failed for "
                        << F->getName() << '\n');
      MDNode *NewFIR = createFunctionInliningReport(F, MDIR);
      ModuleInlineReport->setOperand(FuncIndex, NewFIR);
      F->setMetadata(FunctionTag, NewFIR);
      return NewFIR;
    }
  }

  // 2. There is no inlining report for function yet - create it and attach to
  // module inlining report. We also get here if we need separate inline
  // report for each inlining step.
  LLVM_DEBUG(dbgs() << "MDIR setup: create function inlining report for "
                    << F->getName() << '\n');
  FIR = createFunctionInliningReport(F, MDIR);
  F->setMetadata(FunctionTag, FIR);
  // If we don't need composite inline report for the function, we should
  // replace existing compile-step inline report with newly created.
  if (!(MDIR.getLevel() & CompositeReport) && IsInModule)
    ModuleInlineReport->setOperand(FuncIndex, FIR);
  else
    ModuleInlineReport->addOperand(FIR);
  return FIR;
}

// Since we create inlining report for both declarations and definitions, on the
// link step after automatic merge of the module metadata lists we could see
// separate inlining reports for declaration and definition of the same
// function. So if the inlining report for the function definition is in the
// list, we need to exclude inlining report for its declaration. Note that
// inlining report for two declarations would be identical and would be merged
// in one metadata node automatically.
static void removeDuplicatedFunctionMDNodes(NamedMDNode *ModuleInlineReport,
                                            Module &M) {
  SmallVector<MDNode *, 100> Ops;
  for (unsigned I = 0; I < ModuleInlineReport->getNumOperands(); ++I) {
    MDNode *Node = ModuleInlineReport->getOperand(I);
    InliningReport IR(cast<MDTuple>(Node));
    Function *F = M.getFunction(IR.getName());
    // If function was declared in multiple places, equivalent function
    // inline report metadata will be merged automatically.
    if (F) {
      MDTuple *FIR = cast<MDTuple>(Node);
      assert(FIR && "Bad function inline report");
      int64_t IsDecl = 0;
      getOpVal(FIR->getOperand(FMDIR_IsDeclaration),
               "isDeclaration: ", &IsDecl);
      if (IsDecl) {
        if (F->isDeclaration() && F->getMetadata(FunctionTag) == FIR)
          Ops.push_back(Node);
        else
          continue;
      } else
        Ops.push_back(Node);
    } else {
      // It is a dead function. Keep it as it is.
      Ops.push_back(Node);
    }
  }
  ModuleInlineReport->clearOperands();
  for (auto *Op : Ops)
    ModuleInlineReport->addOperand(Op);
}

// Entry point of the setup process.
bool setupInlineReport(Module &M, InlineReportBuilder &MDIR) {
  if (!MDIR.isMDIREnabled())
    return false;
  LLVM_DEBUG(dbgs() << "\nMDIR setup: start\n");
  NamedMDNode *ModuleInlineReport = M.getOrInsertNamedMetadata(ModuleTag);
  removeDuplicatedFunctionMDNodes(ModuleInlineReport, M);
  for (Function &F : M)
    findOrCreateFunctionInliningReport(&F, ModuleInlineReport, MDIR);

  LLVM_DEBUG(dbgs() << "\nMDIR setup: finish\n");
  return false;
}

namespace {
struct InlineReportSetup : public ModulePass {
  static char ID;
  InlineReportSetup(InlineReportBuilder *IRB = nullptr)
      : ModulePass(ID), MDIR(IRB) {
    initializeInlineReportSetupPass(*PassRegistry::getPassRegistry());
    if (!IRB)
      MDIR = new InlineReportBuilder(IntelInlineReportLevel);
    MDIR->setLevel(IntelInlineReportLevel);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    return setupInlineReport(M, *MDIR);
  }

  InlineReportBuilder *getMDReport() { return MDIR; }

private:
  InlineReportBuilder *MDIR;
};
} // namespace

char InlineReportSetup::ID = 0;
INITIALIZE_PASS(InlineReportSetup, "inlinereportsetup", "Setup inlining report",
                false, false)

ModulePass *llvm::createInlineReportSetupPass(InlineReportBuilder *MDIR) {
  return new InlineReportSetup(MDIR);
}

InlineReportSetupPass::InlineReportSetupPass(InlineReportBuilder *IRB)
    : MDIR(IRB) {
  if (!IRB)
    MDIR = new InlineReportBuilder(IntelInlineReportLevel);
  MDIR->setLevel(IntelInlineReportLevel);
}

PreservedAnalyses InlineReportSetupPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  setupInlineReport(M, *MDIR);
  return PreservedAnalyses::all();
}

