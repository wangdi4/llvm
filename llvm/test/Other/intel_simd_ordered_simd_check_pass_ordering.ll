; RUN: opt -enable-new-pm=0 -O2 -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s --match-full-lines --strict-whitespace

; #pragma omp ordered simd is processed by running the following passes for
; LLVM-IR and HIR path:
; - VPO CFG Restructuring : It should be called before Function Outlining of
;                           Ordered regions because the directives should be in
;                           their own single-predecessor single-successor basic
;                           blocks.
; - Function Outlining of Ordered Regions: Calls Code Extractor that removes the
;                                          simd ordered region from the code and
;                                          replaces it with a function call.
; - VPO CFG Restructuring : Call this pass again because Code Extractor might
;                           add new instructions in the same blocks as the
;                           directives.
; - Inliner for always_inline functions : Inlines the simd ordered code back to
;                                         the code.
; - A No-Op Barrier Pass : It should be called after the inliner in order to
;                          synchronize the passes.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @var_tripcount() local_unnamed_addr {
;      CHECK:    VecClone
; CHECK-NEXT:    FunctionPass Manager
; CHECK-NEXT:      Dominator Tree Construction
; CHECK-NEXT:      Early CSE
; CHECK-NEXT:      Natural Loop Information [LoopOpt]
; CHECK-NEXT:      Canonicalize natural loops [LoopOpt]
; CHECK-NEXT:      LCSSA Verifier [LoopOpt]
; CHECK-NEXT:      Loop-Closed SSA Form Pass [LoopOpt]
; CHECK-NEXT:      VPO CFGRestructuring [LoopOpt]
; CHECK-NEXT:    Function Outlining of Ordered Regions [LoopOpt]
; CHECK-NEXT:      FunctionPass Manager
; CHECK-NEXT:        Dominator Tree Construction
; CHECK-NEXT:        Natural Loop Information
; CHECK-NEXT:        Scalar Evolution Analysis
; CHECK-NEXT:        Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:        Function Alias Analysis Results
; CHECK-NEXT:        VPO Work-Region Collection
; CHECK-NEXT:        Lazy Branch Probability Analysis
; CHECK-NEXT:        Lazy Block Frequency Analysis
; CHECK-NEXT:        Optimization Remark Emitter
; CHECK-NEXT:        VPO Work-Region Information
; CHECK-NEXT:    FunctionPass Manager
; CHECK-NEXT:      Dominator Tree Construction [LoopOpt]
; CHECK-NEXT:      Natural Loop Information [LoopOpt]
; CHECK-NEXT:      Pragma omp simd if clause reduction to simdlen(1) [LoopOpt]
; CHECK-NEXT:      Scalar Evolution Analysis [LoopOpt]
; CHECK-NEXT:      Post-Dominator Tree Construction [LoopOpt]
; CHECK-NEXT:      HIR Region Identification [LoopOpt]
;      CHECK:      VPlan HIR Vectorizer [Full LoopOpt]
; CHECK-NEXT:      VPlan HIR Vectorizer [Light LoopOpt]
;      CHECK:      HIR Code Generation [LoopOpt]
; CHECK-NEXT:      Simplify the CFG [LoopOpt]
; CHECK-NEXT:      Subscript Intrinsic Lowering [LoopOpt]
; CHECK-NEXT:      Dominator Tree Construction [LoopOpt]
; CHECK-NEXT:      SROA [LoopOpt]
; CHECK-NEXT:      Natural Loop Information [LoopOpt]
; CHECK-NEXT:      Phi Values Analysis [LoopOpt]
; CHECK-NEXT:      Basic Alias Analysis (stateless AA impl) [LoopOpt]
; CHECK-NEXT:      Function Alias Analysis Results [LoopOpt]
; CHECK-NEXT:      Memory Dependence Analysis [LoopOpt]
; CHECK-NEXT:      Lazy Branch Probability Analysis [LoopOpt]
; CHECK-NEXT:      Lazy Block Frequency Analysis [LoopOpt]
; CHECK-NEXT:      Optimization Remark Emitter [LoopOpt]
; CHECK-NEXT:      Global Value Numbering [LoopOpt]
; CHECK-NEXT:      SROA [LoopOpt]
; CHECK-NEXT:      Basic Alias Analysis (stateless AA impl) [LoopOpt]
; CHECK-NEXT:      Function Alias Analysis Results [LoopOpt]
; CHECK-NEXT:      Lazy Branch Probability Analysis [LoopOpt]
; CHECK-NEXT:      Lazy Block Frequency Analysis [LoopOpt]
; CHECK-NEXT:      Optimization Remark Emitter [LoopOpt]
; CHECK-NEXT:      Combine redundant instructions [LoopOpt]
; CHECK-NEXT:      Loop Carried CSE [LoopOpt]
; CHECK-NEXT:      Function Alias Analysis Results [LoopOpt]
; CHECK-NEXT:      Post-Dominator Tree Construction [LoopOpt]
; CHECK-NEXT:      Memory SSA [LoopOpt]
; CHECK-NEXT:      Dead Store Elimination [LoopOpt]
; CHECK-NEXT:    CallGraph Construction
; CHECK-NEXT:    Call Graph SCC Pass Manager
; CHECK-NEXT:      Inliner for always_inline functions [LoopOpt]
;      CHECK:    Function Outlining of Ordered Regions
; CHECK-NEXT:      FunctionPass Manager
; CHECK-NEXT:        Dominator Tree Construction
; CHECK-NEXT:        Natural Loop Information
; CHECK-NEXT:        Scalar Evolution Analysis
; CHECK-NEXT:        Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:        Function Alias Analysis Results
; CHECK-NEXT:        VPO Work-Region Collection
; CHECK-NEXT:        Lazy Branch Probability Analysis
; CHECK-NEXT:        Lazy Block Frequency Analysis
; CHECK-NEXT:        Optimization Remark Emitter
; CHECK-NEXT:        VPO Work-Region Information
; CHECK-NEXT:    FunctionPass Manager
; CHECK-NEXT:      Dominator Tree Construction
; CHECK-NEXT:      Natural Loop Information
; CHECK-NEXT:      Pragma omp simd if clause reduction to simdlen(1)
; CHECK-NEXT:      VPO CFGRestructuring
; CHECK-NEXT:      Replace known math operations with optimized library functions
; CHECK-NEXT:      LCSSA Verifier
; CHECK-NEXT:      Loop-Closed SSA Form Pass
; CHECK-NEXT:      Scalar Evolution Analysis
; CHECK-NEXT:      Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:      Function Alias Analysis Results
; CHECK-NEXT:      VPO Work-Region Collection
; CHECK-NEXT:      Lazy Branch Probability Analysis
; CHECK-NEXT:      Lazy Block Frequency Analysis
; CHECK-NEXT:      Optimization Remark Emitter
; CHECK-NEXT:      VPO Work-Region Information
; CHECK-NEXT:      Demanded bits analysis
; CHECK-NEXT:      Loop Access Analysis
; CHECK-NEXT:      VPlan Vectorizer
; CHECK-NEXT:      Dominator Tree Construction
; CHECK-NEXT:      Replace known math operations with optimized library functions
; CHECK-NEXT:    CallGraph Construction
; CHECK-NEXT:    Call Graph SCC Pass Manager
; CHECK-NEXT:      Inliner for always_inline functions
entry:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
