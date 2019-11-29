//===-------- Intel_RegionSplitter.cpp - Class definition -*- C++ -*-------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// \file
// This class is used for function splitting. The RegionSplitter will take
// a set of basic blocks and wraps then in a new function. The original
// function will replace these basic blocks with a call to the new function.
// This class enables inlining for the input function, while the new function
// can be placed in a cold region rather than being inlined. The advantage is
// to improve code locality. For example:
//
// define i32 @foo(i32 %a) #0 {
// entry:
//  %cmp8 = icmp eq %a, 10
//  br i1 %cmp8, label %for.end, label %for.body
//
// for.body:
//  %Num.010 = phi i32 [ %add, %for.body ], [ %a, %entry ]
//  %add = add nsw i32 %Num.010, 1
//  %cmp = icmp eq %add, 10
//  br i1 %cmp, label %for.end, label %for.body
//
// for.end:
//  %Num.0.lcssa = phi i32 [ %a, %entry ], [ %add, %for.body ]
//  ret i32 %Num.0.lcssa
// }
//
// Consider that the splinter region in the previous example is the basic block
// %for.body, then the RegionSplitter will produce the following:
//
// define internal void @foo.for.body(i32 %a, i32* %add.out) {
// newFuncRoot:
//  br label %for.body
//
// for.body.for.end_crit_edge.exitStub:
//  ret void
//
// for.body:
//  %Num.010 = phi i32 [ %add, %for.body ], [ %a, %newFuncRoot ]
//  %add = add nsw i32 %Num.010, 1
//  store i32 %add, i32* %add.out
//  %cmp = icmp eq %add, 10
//  br i1 %cmp, label %for.body.for.end_crit_edge.exitStub, label %for.body
// }
//
// The basic block %for.body will be wrapped in a new function(@foo.for.body)
// and the original function will be transformed as follow:
//
// define i32 @foo(i32 %a) {
// entry:
//  %add.loc = alloca i32
//  %cmp8 = icmp eq %a, 10
//  br i1 %cmp8, label %for.end, label %codeRepl
//
// codeRepl:
//  call void @foo.for.body(i32 %a, i32* %add.loc)
//  %add.reload = load i32, i32* %add.loc
//  br label %for.body.for.end_crit_edge
//
// for.body.for.end_crit_edge:
//  br label %for.end
//
// for.end:
//  %Num.0.lcssa = phi 32 [ %a, %entry ], [ %add.reload,
//                                          %for.body.for.end_crit_edge ]
//
//  ret i32 %Num.0.lcssa
// }
//
// The basic block %for.body is replaced with a call to @foo.for.body.

#include "llvm/Transforms/Utils/Intel_RegionSplitter.h"
#include "llvm/ProfileData/ProfileCommon.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

using namespace llvm;

// Public interface routine that performs all the steps required
// to split the 'Region' into a new function. Returns a pointer
// to the newly created function.
//
Function *RegionSplitter::splitRegion(const SplinterRegionT &Region) {
  prepareRegionForSplit(Region);
  Function *NewF = doSplit(Region);

  return NewF;
}

// Perform the splitting using the input loop body. Returns the
// new function created.
Function *RegionSplitter::splitRegion(Loop &LoopRegion) {
  Function *NewF = doSplit(LoopRegion);

  return NewF;
}

// Modifies the IR to overcome some limitations in the type of IR that
// is handled by the CodeExtractor class. Eventually, the CodeExtractor
// class may be updated to handle these cases, but for now handle them
// here.
//
bool RegionSplitter::prepareRegionForSplit(const SplinterRegionT &Region) {
  // Some edges that exit the region need to be split so that each path that
  // exits the splinter region and returns to the original function will be
  // uniquely identifiable. This is necessary to handle the case where 2 or
  // more Value objects get defined within the region being split out, and
  // get referenced by the same PHI node.
  //
  // For example, if the original function contains the following IR:
  //
  // if.end11:                      ; preds = %while.cond, %if.else9, %if.then
  // %rs.0 = phi i32[365, %if.then], [%0, %if.else9], [%x.addr.0, %while.cond]
  //
  // If the 'if.else9' and 'while.cond' nodes are both within the splinter
  // region, following the extraction, they would both be defined by the block
  // containing call to the new function following the code extraction, such
  // as:
  //   %rs.0 = phi i32[365, %if.then], [%0, %splitR], [%x.addr.0, %splitR]
  // This would be invalid, because it would not clear which value should be
  // used.
  //
  // By splitting the necessary edges, the source values for the PHI nodes
  // stay will stay in blocks that are kept within the original function.
  // A return value of the call to the extracted function will be used to
  // determine edge should be executed following the return of the function
  // call.

  // Collect the set of edges which exit the splinter region, and execute
  // a PHINode instruction.
  SetVector<std::pair<BasicBlock *, BasicBlock *>> SplitEdges;

  for (BasicBlock *BB : Region.getArrayRef()) {
    for (auto SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI) {
      if (!Region.count(*SI)) {
        // Only need to check the first instruction, since any PHI nodes must
        // be at the start of the basic block.
        Instruction *I = &(*SI)->getInstList().front();
        if (isa<PHINode>(*I))
          SplitEdges.insert(std::make_pair(BB, *SI));
      }
    }
  }

  for (auto E : SplitEdges) {
    SplitEdge(E.first, E.second);
  }

  return true;
}

// Do the steps to extract the 'Region' to a new function.
// Returns the new function, if successful, otherwise nullptr.
Function *RegionSplitter::doSplit(const SplinterRegionT &Region) {
  CodeExtractor Extractor(Region.getArrayRef(), &DT, false, &BFI, &BPI);
  CodeExtractorAnalysisCache CEAC(*Region.front()->getParent());
  Function *NewF = Extractor.extractCodeRegion(CEAC);
  if (NewF == nullptr)
    return nullptr;

  setProperties(*NewF);

  return NewF;
}

// Create a new function with the input loop body if it possible. Returns
// the new function, else return nullptr.
Function *RegionSplitter::doSplit(Loop &L) {
  CodeExtractor Extractor(DT, L, false, &BFI, &BPI);

  if (!Extractor.isEligible())
    return nullptr;

  CodeExtractorAnalysisCache CEAC(*L.getBlocks().front()->getParent());
  Function *NewF = Extractor.extractCodeRegion(CEAC);
  if (NewF == nullptr)
    return nullptr;

  setProperties(*NewF);

  return NewF;
}

// Set the properties and attributes of the function created
void RegionSplitter::setProperties(Function &NewF) {

  // Mark the function to be kept in a cold segment.
  NewF.setSectionPrefix(getUnlikelySectionPrefix());

  // Override any inlining directives, if present, and prevent the split out
  // routine from being inlined back to the original function.
  NewF.removeFnAttr(Attribute::AlwaysInline);
  NewF.removeFnAttr("always-inline-recursive");
  NewF.removeFnAttr(Attribute::InlineHint);
  NewF.removeFnAttr("inline-hint-recursive");
  NewF.addFnAttr(Attribute::NoInline);
}

// Return 'true' if there is only a single entry basic block that enters the
// region, and all exits from the region go to the same basic block outside of
// the region (or all paths out of the region return from the function).
bool RegionSplitter::isSingleEntrySingleExit(SplinterRegionT &Region) {
  assert(!Region.empty());

  const BasicBlock *EntryBlock = Region.front();
  BasicBlock *RegionSuccessorBlock = nullptr;
  bool RegionExitsFunction = false;

  for (auto &BB : Region.getArrayRef()) {
    // Check for entry points into the region, other than the initial
    // block.
    if (BB != EntryBlock) {
      for (auto Pred : predecessors(BB)) {
        if (!Region.count(Pred)) {
          return false;
        }
      }
    }

    if (BB->getTerminator() && BB->getTerminator()->getNumSuccessors() == 0) {
      if (RegionSuccessorBlock != nullptr) {
        return false;
      }

      RegionExitsFunction = true;
    }

    // Check if after executing the region, control flow could go to more
    // than 1 block of the original function.
    for (auto Succ : successors(BB)) {
      if (!Region.count(Succ)) {
        if ((RegionSuccessorBlock && RegionSuccessorBlock != Succ) ||
            RegionExitsFunction) {
          return false;
        }

        RegionSuccessorBlock = Succ;
      }
    }
  }

  return true;
}
