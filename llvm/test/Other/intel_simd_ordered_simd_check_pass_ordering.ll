; RUN: opt -passes="default<O2>" -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --match-full-lines --check-prefixes=CHECK,NOHIR
; RUN: opt -passes="default<O2>" -debug-pass-manager -loopopt < %s -o /dev/null 2>&1 | FileCheck %s --match-full-lines --check-prefixes=CHECK,HIR

; #pragma omp ordered simd is processed by running the following passes for
; LLVM-IR and HIR path:
; - VPOCFGRestructuringPass : It should be called before Function Outlining of
;                             Ordered regions because the directives should be in
;                             their own single-predecessor single-successor basic
;                             blocks.
; - VPlanPragmaOmpOrderedSimdExtractPass : Calls Code Extractor that removes the
;                                          simd ordered region from the code and
;                                          replaces it with a function call.
; - VPOCFGRestructuringPass : Call this pass again because Code Extractor might
;                             add new instructions in the same blocks as the
;                             directives.
; - AlwaysInlinerPass : Inlines the simd ordered code back to the code.
; - VPODirectiveCleanupPass : It should be called after the inliner in order to
;                             synchronize the passes.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @var_tripcount() local_unnamed_addr {
; CHECK:  Running pass: VecClonePass on [module]
; CHECK:  Running analysis: LoopAnalysis on var_tripcount
; CHECK:  Running pass: VPlanPragmaOmpSimdIfPass on var_tripcount (1 instruction)
; HIR:  Running pass: VPlanPragmaOmpOrderedSimdExtractPass on [module]
; HIR:  Running pass: HIRSSADeconstructionPass on var_tripcount (1 instruction)
; HIR:  Skipping pass: HIRVecDirInsertPass on var_tripcount
; HIR:  Skipping pass: vpo::VPlanDriverHIRPass on var_tripcount
; HIR:  Running pass: HIRPostVecCompleteUnrollPass on var_tripcount (1 instruction)
; HIR:  Running pass: HIRGeneralUnrollPass on var_tripcount (1 instruction)
; HIR:  Running analysis: HIRLoopResourceAnalysis on var_tripcount
; HIR:  Running pass: HIRScalarReplArrayPass on var_tripcount (1 instruction)
; HIR:  Running pass: HIRCodeGenPass on var_tripcount (1 instruction)
; HIR:  Running analysis: WRegionInfoAnalysis on var_tripcount
; HIR:  Running analysis: WRegionCollectionAnalysis on var_tripcount
; CHECK:  Running pass: LoopSimplifyPass on var_tripcount (1 instruction)
; CHECK:  Running pass: LCSSAPass on var_tripcount (1 instruction)
; CHECK:  Running pass: VPOCFGRestructuringPass on var_tripcount (1 instruction)
; CHECK:  Running pass: VPlanPragmaOmpOrderedSimdExtractPass on [module]
; NOHIR:  Running analysis: WRegionInfoAnalysis on var_tripcount
; NOHIR:  Running analysis: WRegionCollectionAnalysis on var_tripcount
; CHECK:  Running pass: VPOCFGRestructuringPass on var_tripcount (1 instruction)
; CHECK:  Running analysis: DominatorTreeAnalysis on var_tripcount
; CHECK:  Running analysis: LoopAnalysis on var_tripcount
; CHECK:  Running pass: MathLibraryFunctionsReplacementPass on var_tripcount (1 instruction)
; CHECK:  Running pass: vpo::VPlanDriverLLVMPass on var_tripcount (1 instruction)
; CHECK:  Running analysis: OptimizationRemarkEmitterAnalysis on var_tripcount
; CHECK:  Running analysis: OptReportOptionsAnalysis on var_tripcount
; CHECK:  Running analysis: WRegionInfoAnalysis on var_tripcount
; CHECK:  Running analysis: WRegionCollectionAnalysis on var_tripcount
; CHECK:  Running pass: MathLibraryFunctionsReplacementPass on var_tripcount (1 instruction)
; CHECK:  Running pass: AlwaysInlinerPass on [module]
; CHECK:  Running pass: VPOCFGRestructuringPass on var_tripcount (1 instruction)
; CHECK:  Running pass: VPODirectiveCleanupPass on var_tripcount (1 instruction)
entry:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
