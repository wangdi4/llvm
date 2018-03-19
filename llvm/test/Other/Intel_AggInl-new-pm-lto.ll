; Testing AggInlAAPass and dependant analyses with new PM.

; The test below emulates situation when results of wholeprogram analysis are
; invalidated before they are used by inlineaggressive.
; RUN: opt -disable-verify -debug-pass-manager -passes='optimize-dyn-casts,invalidate<wholeprogram>,require<inlineaggressive>' \
; RUN:     -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-All,CHECK-InvWP

; CHECK-All: Starting llvm::Module pass manager run.
; CHECK-InvWP-NEXT: Running pass: OptimizeDynamicCastsPass
; CHECK-InvWP-NEXT: Running analysis: WholeProgramAnalysis
; CHECK-InvWP: Running pass: InvalidateAnalysisPass<{{.*}}WholeProgramAnalysis>
; CHECK-InvWP: Invalidating analysis: WholeProgramAnalysis
; CHECK-InvWP: Running pass: RequireAnalysisPass<{{.*}}InlineAggAnalysis
; CHECK-InvWP: Running analysis: InlineAggAnalysis on <stdin>
; CHECK-InvWP: Running analysis: WholeProgramAnalysis

; CHECK-All: Finished llvm::Module pass manager run.

define void @main()  {
    ret void
}
