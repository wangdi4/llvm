//===--- ParSecTrans.cpp - Pre-pass Transformations of Parallel Sections --===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// \file
//
// This file implements VPO pre-pass transformations for parallel sections, 
// which transforms OpenMP parallel sections to parallel do loop and OpenMP
// work-sharing sections to work-sharing do loop:
//
// #pragma omp parallel sections // or #pragma omp sections
// {
//   #pragma omp section
//     Xdirection();
//   #pragma omp section
//     Ydirection();
//   #pragma omp section
//     Zdirection();
// }
//
// is transformed to 
//
// #pragma omp parallel sections   // or #pragma omp sections
//   for (int i = 0; i <= 2 ; i++) {
//     switch(i) {
//       case 0:
//         Xdirection();
//         break;
//       case 1:
//         Ydirection();
//         break;
//       case 2:
//         Zdirection();
//         break;
//       default:
//     }
//   }
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"


using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-parsectrans"

/////////////// Transformation Description /////////////////////
//
// For the following user code,
//
// #pragma omp parallel sections   (or #pragma omp sections)
// {
//   #pragma omp section  (this directive can be omitted in the user code)
//     Xdirection();
//   #pragma omp section
//     Ydirection();
//   #pragma omp section
//     Zdirection();
// }
//
// the compiler will generate the following CFG:
//
//      DIR_OMP_PARALLEL_SECTIONS    (or DIR_OMP_SECTIONS)
//                |
//          DIR_OMP_SECTION          (this directive must present in CFG)
//                |
//            Xdirection()
//                |
//       DIR_OMP_END_SECTION         (this directive must present in CFG)
//                |
//          DIR_OMP_SECTION
//                |
//            Ydirection()
//                |
//       DIR_OMP_END_SECTION
//                |
//          DIR_OMP_SECTION
//                |
//            Zdirection()
//                |
//       DIR_OMP_END_SECTION
//                |
//    DIR_OMP_END_PARALLEL_SECTIONS (or DIR_END_OMP_SECTIONS)
//
// which is the input to this transformation. Note that:
//
// 1) Each directive must have an END directive to pair with;
//
// 2) Each directive is represented by a group of Intel directive intrinsics
// that must reside in a standalone basic block, e.g.,
//
// par.sections.begin:
//   call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.SECTIONS")
//   .... // directive qualifiers
//   ...
//   call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
//   br label %par.sections.body
//
// ...
//
// par.sections.end:
//   call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.SECTIONS")
//   .... // directive qualifiers
//   ...
//   call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
//   br label %after.par
//
// 3) The directive DIR_OMP_SECTION/DIR_OMP_END_SECTION for the first section
// must be presented in the CFG, although it can be omitted in the user code;
//
// 4) There can be data flow across sections inside a parallel
// section or work-sharing section; however, if that happens, the variable and
// its related operations will be guarded in a critical section which inforces
// the load and store of the variable from/to memory first (this is OpenMP
// shared-memory model). In other words, such variable will not be
// registerized, and we do not have to worry about the SSA form or update for
// it.
//
// 5) Each OMP_PARALLEL_SECTIONS, OMP_SECTIONS and OMP_SECTION must form a
// single-entry and single-exit region;
//
// 6) OMP_PARALLEL_SECTIONS or OMP_SECTIONS can be an empty region, e.g.,(in
// the CFG form)
//
//      DIR_OMP_PARALLEL_SECTIONS    (or DIR_OMP_SECTIONS)
//                |
//                |                  (no code between them)
//                |
//    DIR_OMP_END_PARALLEL_SECTIONS  (or DIR_OMP_END_SECTIONS)
//
// 7) OMP_PARALLEL_SECTIONS can be nested, for example (presented in user
// code): 
//
// #pragma omp parallel sections                  (par1)
// {
//   #pragma omp section                          (sec1)
//     #pragma omp parallel sections              (par2)
//       #pragma omp section                      (sec2)
//       #pragma omp section                      (sec3)
//
//   #pragma omp section                          (sec4)
//     #pragma omp parallel sections              (par3)
//       #pragma omp section                      (sec5)
//       #pragma omp section                      (sec6)
//
//   #pragma omp section                          (sec7)
// }
// #pragma omp sections                           (secs1)
// {
//   //empty
// }
//
// To do the transformations, we build a Section Tree first (based on
// Dominator Tree) to represent this nested relationship. For example, for the
// above code, we will have:
//
//                 Root
//                 /  \
//             secs1   par1
//                   /   |  \
//                sec1  sec4 sec7
//                 |      |
//               par2    par3
//               / \     / \
//            sec2sec3 sec5sec6  
//
// (Note that, the order of children of each node does not matter.)
//
// And then, we traverse the Section Tree in a post order, and do the
// transformation for each OMP_PARALLEL_SECTIONS or OMP_SECTIONS node in the
// tree if it has children (which must be OMP_SECTION nodes). We use the
// post-order traversal since we can delete/free all the children after we visit
// each node. By the time we finish all the transformations, only Root node is
// left in the tree, which we will delete at last.
//
// The transformation function returns TRUE if any transformation happens,
// otherwise returns FALSE.
//
bool VPOUtils::parSectTransformer(
  Function *F,
  DominatorTree *DT
)
{
  ParSectNode *Root = buildParSectTree(F, DT);

#ifdef DEBUG
  printParSectTree(Root);
#endif

  // Keep track of how many transformations take place; used for naming
  // purpose.
  int Counter = 0;

  parSectTransRecursive(F, Root, Counter, DT);

  delete Root;

  if (Counter)
    return true;
  else 
    return false;
}

ParSectNode *VPOUtils::buildParSectTree(Function *F, DominatorTree *DT)
{
  // Used to find a pair of directives' basic blocks.
  std::stack<ParSectNode *> SectStack;

  // The Root node does not correspond to any region represented by
  // OMP_PARALLEL_SECTIONS, OMP_SECTIONS or OMP_SECTION, but its children do.
  ParSectNode *Root = new ParSectNode();
  Root->EntryBB = nullptr;
  Root->ExitBB = nullptr;

  SectStack.push(Root);

  buildParSectTreeRecursive(&F->getEntryBlock(), SectStack, DT); 

  return Root;
}

// Pre-order traversal on Dominator Tree with the use of a stack 
// to build Section Tree.
void VPOUtils::buildParSectTreeRecursive(
  BasicBlock* BB, 
  std::stack<ParSectNode *> &SectStack,
  DominatorTree *DT
)
{
  auto DomNode = DT->getNode(BB);
  ParSectNode *Node = nullptr;

  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {

    if (IntrinsicInst *Call = dyn_cast<IntrinsicInst>(&*I)) { 
      Intrinsic::ID IntrinId = Call->getIntrinsicID();
      if (IntrinId == Intrinsic::intel_directive) {

        StringRef DirStr = VPOUtils::getDirectiveMetadataString(Call);
        int DirID = VPOUtils::getDirectiveID(DirStr);

        if (DirID == DIR_OMP_SECTION ||
            DirID == DIR_OMP_SECTIONS ||
            DirID == DIR_OMP_PARALLEL_SECTIONS) {

          Node = new ParSectNode();
          Node->EntryBB = BB;
          Node->DirBeginID = DirID;

          ParSectNode *Parent = SectStack.top();
          Parent->Children.push_back(Node);
          SectStack.push(Node);
        }
        if (DirID == DIR_OMP_END_SECTION ||
            DirID == DIR_OMP_END_SECTIONS ||
            DirID == DIR_OMP_END_PARALLEL_SECTIONS) {

          Node = SectStack.top();
          Node->ExitBB = BB;

          SectStack.pop();
        }
      }
    }
  }

  /// Walk over dominator children.
  for (auto D = DomNode->begin(), E = DomNode->end(); D != E; ++D) {
    auto DomChildBB = (*D)->getBlock();
    buildParSectTreeRecursive(DomChildBB, SectStack, DT);
  }
}

// Pre-order traversal on Section Tree to print debug messages.
void VPOUtils::printParSectTree(ParSectNode *Node)
{

  if (!Node->EntryBB && !Node->ExitBB) {
    DEBUG(dbgs() << "\nSectionTree: Root:\n");
  }
  else if (Node->EntryBB && Node->ExitBB) {
    DEBUG(dbgs() << "\n\n\nSectionTreeNode: EntryBB:\n"
                 << *(Node->EntryBB)
                 << "\nExitBB:\n"
                 << *(Node->ExitBB));
  }

  if (Node->Children.size() == 0) {
    DEBUG(dbgs() << "\nNo Children:\n"); 
    return;
  }

  DEBUG(dbgs() << "\nStarting Chidren Printing:\n"); 

  for (auto *Child: Node->Children)
    printParSectTree(Child);

  DEBUG(dbgs() << "\nEnding Chidren Printing:\n"); 
}

// Post-order traversal
void VPOUtils::parSectTransRecursive(
  Function *F,
  ParSectNode *Node,
  int &Counter,
  DominatorTree *DT
)
{
  // This is a leaf node. Nothing to do with it.
  if (Node->Children.size() == 0)
    return;

  for (auto *Child: Node->Children)
    parSectTransRecursive(F, Child, Counter, DT);

  // We only need to do transformations at OMP_PARALLEL_SECTIONS and
  // OMP_SECTIONS nodes, not OMP_SECTION nodes or the tree Root.
  //
  if (Node->EntryBB && Node->ExitBB) 
    if (Node->DirBeginID == DIR_OMP_SECTIONS ||
        Node->DirBeginID == DIR_OMP_PARALLEL_SECTIONS) {
            
      // Just a check
      for (auto *Child: Node->Children) {
        assert(Child->EntryBB->getSinglePredecessor()
               && "Not a single-entry OMP Section");
        assert(Child->ExitBB->getSingleSuccessor()
               && "Not a single-exit OMP Section");
      }

      Counter++;
      doParSectTrans(F, Node, Counter, DT);
    }

  // Free children
  for (auto *Child: Node->Children)
    delete Child;
}

// This function does the real transformation work. For the following CFG:
//
//      OMP_PARALLEL_SECTIONS  (or OMP_SECTIONS)
//                |
//          OMP_SECTION    
//                |
//               X()
//                |
//          OMP_END_SECTION 
//                |
//          OMP_SECTION
//                |
//               Y()
//                |
//         OMP_END_SECTION
//                |
//              .....
//                |
//    OMP_END_PARALLEL_SECTIONS  
//
// the function transforms it to:
//
//      OMP_PARALLEL_SECTIONS  (or OMP_SECTIONS)
//                |
//          Loop PreheaderBB
//                |
//            Loop HeaderBB: <-------------|
//             i = phi(0, i')              |
//             switch (i)                  |
//             /  |  ...  \                |
//            /   |   ...  \               |
//  OMP_SECTION OMP_SECTION..DefaultBB     |
//       |        |             /          |
//      X()      Y()           /           |
//       |        |           /            | 
//  END_SECTION END_SECTION  /             |
//       \         |        /              |
//        \        |       /               |
//         \       | ...  /                |
//          SwitchEpilogBB                 |
//                 |                       |
//             SwitchSuccBB:               |
//               i' = i + 1                | 
//          if (i' <= (NumSections-1))     |
//                | |                      |
//                | |----------------------|
//                |
//            Loop ExitBB
//                |
//    OMP_END_PARALLEL_SECTIONS  
//
// Note that, the directives OMP_SECTION and OMP_END_SECTION will be deleted
// although we show them here for illustration purpose.
//
void VPOUtils::doParSectTrans(
  Function *F,
  ParSectNode *Node,
  int Counter,
  DominatorTree *DT
)
{
  assert(Node->Children.size() != 0 && "No section nodes to be transformed");

  BasicBlock *SectionsEntryBB = Node->EntryBB;
  BasicBlock *SectionsExitBB = Node->ExitBB;

  // 1) Take all sections out first, so OMP_PARALLEL_SECTIONS(or OMP_SECTIONS)
  // is directly connected to OMP_END_PARALLEL_SECTIONS(or OMP_END_SECTIONS)
  //
  //      OMP_PARALLEL_SECTIONS  (or OMP_SECTIONS)
  //                |
  //    OMP_END_PARALLEL_SECTIONS 
  //
  IRBuilder<> Builder(SectionsEntryBB);
  SectionsEntryBB->getTerminator()->eraseFromParent();
  Builder.CreateBr(SectionsExitBB);

  // 2) Insert an empty loop between the pair of directives:
  //
  //      OMP_PARALLEL_SECTIONS  (or OMP_SECTIONS)
  //                |
  //          Loop PreheaderBB
  //                |
  //            Loop HeaderBB: <-------------|
  //             i = phi(0, i')              |
  //             i' = i + 1                  | 
  //          if (i' <= (NumSections-1))     |
  //                | |----------------------|
  //                |
  //            Loop ExitBB
  //                |
  //    OMP_END_PARALLEL_SECTIONS
  //
  // Generating "if (i' < = NumSections -1)" is more efficient than
  // "if (i' < NumSections)" for the loop.
  //
  // generateLoop() will return the first non-PHI instruction as the loop
  // insertion point, for the following code to insert the loop body.
  //
  unsigned NumSections = Node->Children.size();
  IntegerType *type = Type::getInt32Ty(F->getContext());
  Constant *LB = ConstantInt::get(type, 0);
  Constant *UB = ConstantInt::get(type, (NumSections - 1));
  Constant *Stride = ConstantInt::get(type, 1);

  Value *IV = genNewLoop(LB, UB, Stride, Builder, Counter, DT);

  // 3) Insert a switch statement right before the first non-PHI instruction
  // in the loop. The code (e.g., sec1, sec2) for each OMP_SECTION will be
  // inserted at the point for each case of the switch.
  //
  //      OMP_PARALLEL_SECTIONS  (or OMP_SECTIONS)
  //                |
  //          Loop PreheaderBB
  //                |
  //            Loop HeaderBB: <-------------|
  //             i = phi(0, i')              |
  //             switch (i)                  |
  //             /  |  ...  \                |
  //        sec1  sec2 ... DefualtCase       |
  //          \      | ...  /                |
  //          SwitchEpilogBB                 |
  //                 |                       |
  //             SwitchSuccBB:               |
  //               i' = i + 1                | 
  //          if (i' <= (NumSections-1))     |
  //                | |                      |
  //                | |----------------------|
  //                |
  //            Loop ExitBB
  //                |
  //      OMP_END_PARALLEL_SECTIONS  
  //
  // We always generate an empty basic block for the default case, which does
  // not correspond to any OMP_SECTION since CreateSwitch needs it.
  //
  // generateSwitch() will return the switch instruction created.
  //
  SwitchInst *SwitchInstruction = genParSectSwitch(IV, Node, Builder, Counter, DT);
}

// Insert an empty loop right after BeforeBB
//
//             BeforeBB
//                |
//           PreheaderBB
//                |
//            HeaderBB:      <-------------|
//             i = phi(LB, i')             |
//             i' = i + Stride             | latch
//             if (i' <= UB)               |
//                |   |____________________|
//                |
//              ExitBB
//
Value *VPOUtils::genNewLoop(
  Value *LB,
  Value *UB,
  Value *Stride,
  IRBuilder<> &Builder,
  int Counter,
  DominatorTree *DT
)
{
  assert(LB->getType() == UB->getType() && "Loop bound types do not match");

  IntegerType *LoopIVType = dyn_cast<IntegerType>(UB->getType());
  assert(LoopIVType && "UB is not integer?");

  Function *F = Builder.GetInsertBlock()->getParent();
  LLVMContext &Context = F->getContext();
  StringRef FName = F->getName();

  BasicBlock *BeforeBB = Builder.GetInsertBlock();

  BasicBlock *PreHeaderBB = 
      BasicBlock::Create(Context, FName + ".sec.loop.preheader." + Twine(Counter), F);

  BasicBlock *HeaderBB = 
      BasicBlock::Create(Context, FName + ".sec.loop.header." + Twine(Counter), F);

  // The default insertion point is InsertBlock->end(), we need to move it one
  // step back to point to the terminator instruction in InsertBlock, for
  // SplitBlock to use to create ExitBB
  //
  Builder.SetInsertPoint(&*--Builder.GetInsertPoint());
  BasicBlock *ExitBB = 
      SplitBlock(BeforeBB, &*Builder.GetInsertPoint(), DT, nullptr);
  ExitBB->setName(FName + ".sec.loop.exit." + Twine(Counter));

  // BeforeBB
  BeforeBB->getTerminator()->setSuccessor(0, PreHeaderBB);

  // PreHeaderBB
  Builder.SetInsertPoint(PreHeaderBB);
  Builder.CreateBr(HeaderBB);

  // HeaderBB
  Builder.SetInsertPoint(HeaderBB);
  PHINode *IV = Builder.CreatePHI(
                LoopIVType, 2, FName + ".sec.loop.indvar." + Twine(Counter));
  IV->addIncoming(LB, PreHeaderBB);
  Value *IncrementedIV = Builder.CreateAdd(
                    IV, Stride, FName + ".sec.loop.incr." + Twine(Counter),
                    true/*HasNUW*/, true/*HasNSW*/);
  Value *LoopCondition = Builder.CreateICmp(ICmpInst::ICMP_SLE, IV, UB);
  LoopCondition->setName(FName + ".sec.loop.cond." + Twine(Counter));
    
  // Loop latch
  Builder.CreateCondBr(LoopCondition, HeaderBB, ExitBB);
  IV->addIncoming(IncrementedIV, HeaderBB);

  // Now move the newly created loop blocks from the end of basic block list
  // to the proper place, which is right before loop ExitBB. This will not
  // affect CFG, but CFG printing and readability.
  F->getBasicBlockList().splice(ExitBB->getIterator(),
                                F->getBasicBlockList(),
                                PreHeaderBB->getIterator(),
                                F->end());

  if (DT) {
    DT->addNewBlock(PreHeaderBB, BeforeBB);
    DT->addNewBlock(HeaderBB, PreHeaderBB);
    DT->changeImmediateDominator(ExitBB, HeaderBB);
  }
    
  // The loop body should be added here.
  Builder.SetInsertPoint(HeaderBB->getFirstNonPHI());

  // Return the loop PHI instruction
  return IV;
}

// Insert a switch statement at the SwitchInsertPoint in SwitchBB:
//
// Given the following basic block:
//
//             --------------------
//             |SwitchBB:         |
//             |......            |
//             |SwitchInsertPoint |
//             |......            |
//             --------------------
//
// the function transforms it to:
//
//             --------------------
//             |SwitchBB:         |
//             |......            |
//             |switch(i)         |
//             --------------------
//             /    |    ...  \                
//        case1  case2    ... DefualtCase       
//           \      |     ...  /                
//             SwitchEpilogBB                 
//                  |                       
//             --------------------
//             |SwitchsuccBB:     |
//             |SwitchInsertPoint |
//             |......            |
//             --------------------
//
SwitchInst *VPOUtils::genParSectSwitch(
  Value *SwitchCond,
  ParSectNode *Node,
  IRBuilder<> &Builder,
  int Counter,
  DominatorTree *DT
)
{

  BasicBlock *SwitchBB = Builder.GetInsertBlock();
  BasicBlock::iterator InsertPoint = Builder.GetInsertPoint();

  Function *F = SwitchBB->getParent();
  LLVMContext &Context = F->getContext();
  StringRef FName = F->getName();

  unsigned NumCases = Node->Children.size();

  // Split SwitchBB at the SwitchInsertPoint
  BasicBlock *SwitchSuccBB = 
      SplitBlock(SwitchBB, InsertPoint, DT, nullptr);
  SwitchSuccBB->setName(FName + ".sec.sw.succBB." + Twine(Counter));

  // Insert the Switch right before the original terminator
  Builder.SetInsertPoint(SwitchBB->getTerminator());

  BasicBlock *Default = BasicBlock::Create(
          Context, FName + ".sec.sw.default." + Twine(Counter), F);
  SwitchInst *SwitchInstruction = 
      Builder.CreateSwitch(SwitchCond, Default, NumCases);

  BasicBlock *Epilog = BasicBlock::Create(
          Context, FName + ".sec.sw.epilog." + Twine(Counter), F);
  Builder.SetInsertPoint(Epilog);
  Builder.CreateBr(SwitchSuccBB);

  for (unsigned i = 0, e = NumCases; i != e; ++i) {

    ConstantInt *CaseValue = 
        ConstantInt::get(Type::getInt32Ty(Context), i);

    BasicBlock *SectionEntryBB = Node->Children[i]->EntryBB;
    BasicBlock *SectionExitBB = Node->Children[i]->ExitBB;

    SectionEntryBB->setName(
            FName + ".sec.sw.case" + Twine(i) + "." + Twine(Counter));
    SwitchInstruction->addCase(CaseValue, SectionEntryBB); 

    SectionExitBB->getTerminator()->eraseFromParent();
    Builder.SetInsertPoint(SectionExitBB);
    Builder.CreateBr(Epilog);

    if (DT) {
      DT->changeImmediateDominator(SectionEntryBB, SwitchBB);
    }

    // Delete DIR_OMP_SECTION directive, which has the following form:
    //
    // sec.begin:
    // call void @llvm.intel.directive(metadata !"DIR.OMP.SECTION");
    // call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END");
    // br label %sec.body
    //
    SectionEntryBB->getInstList().pop_front();
    SectionEntryBB->getInstList().pop_front();

    // Delete DIR_OMP_END_SECTION directive, which has the following form:
    //
    // sec.end:
    // call void @llvm.intel.directive(metadata !"DIR.OMP.END.SECTION");
    // call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END");
    // br label %after.sec
    //
    SectionExitBB->getInstList().pop_front();
    SectionExitBB->getInstList().pop_front();
  }

  Builder.SetInsertPoint(Default);
  Builder.CreateBr(Epilog);

  SwitchBB->getTerminator()->eraseFromParent();

  // Do the block moving
  F->getBasicBlockList().splice(SwitchSuccBB->getIterator(),
                                F->getBasicBlockList(),
                                Default->getIterator(),
                                F->end());
  if (DT) {
    DT->addNewBlock(Default, SwitchBB);
    DT->addNewBlock(Epilog, SwitchBB);
    DT->changeImmediateDominator(SwitchSuccBB, Epilog);
  }

  return SwitchInstruction;
}
