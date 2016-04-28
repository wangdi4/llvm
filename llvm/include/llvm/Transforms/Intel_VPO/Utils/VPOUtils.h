//=======-- VPOUtils.h - Class definitions for VPO utilites -*- C++ -*-=======//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the VPOUtils class and provides a set of common utilities
/// that are shared across the Vectorizer and Parallelizer.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H
#define LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Analysis/Intel_Directives.h"
#include <unordered_map>

// Used for Parallel Section Transformations
#include <stack>
#include "llvm/IR/IRBuilder.h"

namespace llvm {

class Value;
class Module;
class Function;
class Type;
class BasicBlock;
class Loop;
class LoopInfo;
class DominatorTree;
class StringRef;
class CallInst;
class IntrinsicInst;
class Constant;
class LLVMContext;

namespace vpo {

/// Data structure Used for Parallel Section Transformations
typedef struct ParSectNode {
    BasicBlock *EntryBB;
    BasicBlock *ExitBB;
    int DirBeginID;
    SmallVector<ParSectNode *, 8> Children;
} ParSectNode;
//////////////////////////////////////

typedef SmallVector<BasicBlock *, 32> VPOSmallVectorBB;
typedef SmallVector<Instruction *, 32> VPOSmallVectorInst;

/// \brief This class contains a set of utility functions used by VPO passes.
class VPOUtils {

private:
    /// \brief The BBSet and the ClonedBBSet (held in \p VMap) merge at the
    /// successor of ExitBB in BBSet, which we call the "merged bblock". This
    /// function adds PHI instructions to the merged bblock for any value that
    /// is live outside BBSet.
    static void addPHINodes(
            ValueToValueMapTy &VMap,
            SmallVectorImpl<BasicBlock *> &BBSet,
            SmallVectorImpl<Instruction *> &LiveOut);

public:
    /// Constructor and destructor
    VPOUtils() {}
    ~VPOUtils() {}

    // Map StringRefs to OMP_DIRECTIVES
    static StringMap<int> DirectiveIDs;

    // Map StringRefs to OMP_CLAUSES
    static StringMap<int> ClauseIDs;

    /// \brief Initialize maps of directive & clause string to ID.
    /// This routine must be invoked (once) before calling query
    /// routines such as getDirectiveID() and getClauseID().
    static void initDirectiveAndClauseStringMap();

    /// \brief This function restructures the CFG on demand, where each
    /// directive for Cilk, OpenMP, Offload, Vectorization is put into a
    /// standalone basic block. This is a pre-required process for WRegion
    /// construction for each function. 
    ///
    /// Note that, since WRegion construction requires
    /// DominatorTreeWrapperPass and LoopInfoWrapperPass to be executed prior
    /// to it, when calling CFGRestructuring, we need to update DominatorTree
    /// and LoopInfo whenever a basic block splitting happens.
    static void CFGRestructuring(Function &F, DominatorTree *DT = nullptr,
                     LoopInfo *LI = nullptr);
  
    /// \brief Return true if the intrinsic Id is intel_directive.
    static bool isIntelDirective(Intrinsic::ID Id);

    /// \brief Return true if the intrinsic Id corresponds to a clause:
    ///    intel_directive_qual,
    ///    intel_directive_qual_opnd, or 
    ///    intel_directive_qual_opndlist.
    static bool isIntelClause(Intrinsic::ID Id);

    /// \brief Return true if the intrinsic Id corresponds to an
    /// Intel directive or clause.
    static bool isIntelDirectiveOrClause(Intrinsic::ID Id);

    /// \brief Return the string representation of the metadata argument used
    /// within a call to one of these intrinsics: 
    ///    llvm.intel.directive, 
    ///    llvm.intel.directive.qual, 
    ///    llvm.intel.directive.qual.opnd, and 
    ///    llvm.intel.directive.qual.opndlist.
    static StringRef getDirectiveMetadataString(IntrinsicInst *Call);

    /// \brief Similar to getDirectiveString(), 
    /// but strips out the leading "DIR_OMP_" prefix substring
    static StringRef getDirectiveName(int Id);

    /// \brief Similar to getClauseString(), 
    /// but strips out the leading "QUAL_OMP_" prefix substring
    static StringRef getClauseName(int Id);

    /// \brief Returns true if the string corresponds to an OpenMP directive.
    static bool isOpenMPDirective(StringRef DirFullName);

    /// \brief Returns true if the string corresponds to an OpenMP clause.
    static bool isOpenMPClause(StringRef ClauseFullName);

    /// \brief Returns the ID (enum) corresponding to OpenMP directives.
    static int getDirectiveID(StringRef DirFullName);

    /// \brief Returns the ID (enum) corresponding to OpenMP clauses.
    static int getClauseID(StringRef ClauseFullName);

    /// \brief Removes calls to directive intrinsics.
    static bool stripDirectives(Function &F);

    /// Utilities to handle directives & clauses 

    /// \brief Return true iff DirString corresponds to a directive that
    /// begins a region (eg, DIR_OMP_PARALLEL, DIR_OMP_SIMD, etc.
    static bool isBeginDirective(StringRef DirString);
    static bool isBeginDirective(int DirID);

    /// \brief Return true iff DirString corresponds to a directive that
    /// ends a region (eg, DIR_OMP_END_PARALLEL, DIR_OMP_END_SIMD, etc.
    static bool isEndDirective(StringRef DirString);
    static bool isEndDirective(int DirID);

    /// \brief Return true iff DirString corresponds to a directive that
    /// begins or ends a region
    static bool isBeginOrEndDirective(StringRef DirString);
    static bool isBeginOrEndDirective(int DirID);

    /// \brief Return true iff DirString corresponds to a stand-alone 
    /// directive (doesn't begin or end a region). Eg: DIR_OMP_FLUSH
    static bool isSoloDirective(StringRef DirString);
    static bool isSoloDirective(int DirID);

    /// \brief Return true iff DirString corresponds to DIR_QUAL_LIST_END,
    /// the mandatory marker to end a directive
    static bool isListEndDirective(StringRef DirString);
    static bool isListEndDirective(int DirID);

    /// \brief Return true iff the ClauseID represents a REDUCTION clause,
    /// such as QUAL_OMP_REDUCTION_ADD
    static bool isReductionClause(int ClauseID);

    ////////////////// MultiVersioning Transformation ////////////////////////
    //
    /// \brief Given a single-entry and single-exit region represented by
    /// BBSet, generates the following code, where ClonedBBSet is the cloned
    /// region of BBSet:
    /// 
    /// if (Cond)
    ///   BBSet;
    /// else
    ///   ClonedBBSet;
    ///
    /// Client needs to recompute Loop information if needed and may need to
    /// recompute WRegion information as well depending on what the cloned
    /// region will look like, e.g., becoming serial code or remaining as a
    /// parallel region.
    static void singleRegionMultiVersioning(
            BasicBlock *EntryBB,
            BasicBlock *ExitBB,
            SmallVectorImpl<BasicBlock *> &BBSet,
            Value *Cond,
            DominatorTree *DT = nullptr);

    /// \brief Clones all the basic blocks in BBSet to ClonedBBSet and remaps
    /// all the values in the ClonedBBSet. The function does not maintain DT
    /// and LI information. So the client needs to do that. The cloned
    /// basic blocks are attached to the end of function \p F's basic block
    /// list. Function \p F can be the same as where BBSet resides, or
    /// different. \p CodeInfo (optional) helps to collect additional
    /// information about the cloned region, such as if it contains call,
    /// dynamic or static alloca instructions. \p VMap bookkeeps both basic
    /// block mapping and instruction mapping. 
    static void cloneBBSet(
            SmallVectorImpl<BasicBlock *> &BBSet,
            SmallVectorImpl<BasicBlock *> &ClonedBBSet,
            ValueToValueMapTy &VMap,
            const Twine &NameSuffix,
            Function *F,
            ClonedCodeInfo *CodeInfo = nullptr);

    /// \brief Collects all the instructions in BBSet who have uses (live-out)
    /// outside the BBSet and stores them in \p LiveOut.
    static void findDefsUsedOutsideOfRegion(
            SmallVectorImpl<BasicBlock *> &BBSet,
            SmallVectorImpl<Instruction *> &LiveOut);

    /////////////// End MultiVersioning Transformation ////////////////////////
    
    ////////// Functions for Parallel Section Transformation //////////////////
    //
    // Transforms OpenMP parallel sections to parallel do loop and work-sharing
    // sections to work-sharing do loop in function \p F. Use a post-order
    // travesal on the Section Tree (built based on the Dominator Tree) to do
    // the transformation so that children can be deleted after
    // transformation at each node. The core transforamtion function is
    // doParSectTrans.
    static bool parSectTransformer(
          Function *F, DominatorTree *DT = nullptr);
    static void parSectTransRecursive(Function *F, ParSectNode *Node,
                        int &Counter, DominatorTree *DT = nullptr);
    static void doParSectTrans(Function *F, ParSectNode *Node,
                        int Counter, DominatorTree *DT = nullptr);

    // Generate an empty loop with lower bound \p LB, upper bound \p UB, stride
    // \p stride, right after the InsertBlock pointed by \p Builder.
    static Value *genNewLoop(Value *LB, Value *UB, Value *Stride,
                        IRBuilder<> &Builder, int Counter,
                        DominatorTree *DT = nullptr);

    // Generate a switch statement for the parallel section or work-sharing
    // section represented by \p Node. Each section in the parallel section or
    // work-sharing section corresponds to a case statement in the switch.
    static SwitchInst *genParSectSwitch(Value *SwitchCond, ParSectNode *Node,
                        IRBuilder<> &Builder, int Counter,
                        DominatorTree *DT = nullptr);

    // Generate a tree data structure called Section Tree to represent the
    // nesting relationship of OMP_PARALLEL_SECTIONS (or OMP_SECTIONS) and
    // OMP_SECTION in a function. It is used for the transformations of OpenMP
    // parallel sections and work-sharing sections.
    static ParSectNode *buildParSectTree(Function *F,
                        DominatorTree *DT = nullptr);
    static void buildParSectTreeRecursive(BasicBlock* BB, 
                        std::stack<ParSectNode *> &SectionStack, 
                        DominatorTree *DT = nullptr);
    static void printParSectTree(ParSectNode *Node);

    ///////////////// End Parallel Section Transformation /////////////////
  
    //////////////// Functions for vector code generation /////////////////
    /// \brief Return a call to the llvm.masked.gather intrinsic. A null Mask
    /// defaults to an unmasked gather. A null PassThru value uses undef value
    /// for pass through value.
    static CallInst* createMaskedGatherCall(Module *M,
                                            Value *VecPtr,
                                            IRBuilder<> *Builder,
                                            unsigned Alignment = 0,
                                            Value *Mask = nullptr,
                                            Value *PassThru = nullptr);

    /// \brief Return a call to the llvm.masked.scatter intrinsic. A null Mask
    /// defaults to an unmasked scatter. 
    static CallInst* createMaskedScatterCall(Module *M,
                                             Value *VecPtr,
                                             Value *VecData,
                                             IRBuilder<> *Builder,
                                             unsigned Alignment = 0,
                                             Value *Mask = nullptr);

    ///////////////////// End vector code generation  /////////////////////
};

} // End vpo namespace

} // End llvm namespace
#endif


