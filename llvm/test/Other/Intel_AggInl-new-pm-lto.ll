; Testing AggInlAAPass and analyses that it needs with new PM.

; RUN: opt -disable-verify -debug-pass-manager -passes='lto<O2>' -S < %s 2>&1 \
; RUN:     | FileCheck %s --check-prefixes=CHECK-All,CHECK-Agg
; RUN: opt -disable-verify -debug-pass-manager -passes='lto<O2>' \
; RUN:     -enable-npm-inline-aggressive-analysis=false -S < %s 2>&1 \
; RUN:     | FileCheck %s --check-prefixes=CHECK-All,CHECK-NoAgg

; The test below emulates situation when results of analysis are
; invalidated before they are used by agginlaa.
; RUN: opt -disable-verify -debug-pass-manager -passes='require<inlineaggressive>,function(jump-threading),invalidate<inlineaggressive>,function(agginlaa)' \
; RUN:     -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-All,CHECK-InvAgg

; The test below emulates situation when results of wholeprogram analysis are
; invalidated before they are used by inlineaggressive.
; RUN: opt -disable-verify -debug-pass-manager -passes='optimize-dyn-casts,invalidate<wholeprogram>,require<inlineaggressive>' \
; RUN:     -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-All,CHECK-InvWP

; CHECK-All: Starting llvm::Module pass manager run.
; CHECK-Agg: Running pass: InlineListsPass
; CHECK-Agg-NEXT: Running pass: RequireAnalysisPass<{{.*}}AndersensAA
; CHECK-Agg-NEXT: Running analysis: AndersensAA
; CHECK-Agg-NEXT: Running analysis: CallGraphAnalysis
; CHECK-Agg-NEXT: Running pass: ModuleToFunctionPassAdaptor<{{.*}}IndirectCallConvPass
; CHECK-Agg-NEXT: Running pass: RequireAnalysisPass<{{.*}}InlineAggAnalysis
; CHECK-Agg-NEXT: Running analysis: InlineAggAnalysis
; CHECK-Agg: Running pass: AggInlAAPass

; CHECK-NoAgg-NOT: Running pass: RequireAnalysisPass<{{.*}}InlineAggAnalysis
; CHECK-NoAgg-NOT: Running analysis: InlineAggAnalysis
; CHECK-NoAgg-NOT: Running pass: AggInlAAPass

; CHECK-InvAgg-NEXT: Running pass: RequireAnalysisPass<{{.*}}InlineAggAnalysis
; CHECK-InvAgg-NEXT: Running analysis: InlineAggAnalysis
; CHECK-InvAgg: Running pass: JumpThreadingPass
; CHECK-InvAgg: Running pass: InvalidateAnalysisPass<{{.*}}InlineAggAnalysis>
; CHECK-InvAgg: Invalidating analysis: InlineAggAnalysis
; CHECK-InvAgg: Running pass: AggInlAAPass
; CHECK-InvAgg-NEXT: Running analysis: OuterAnalysisManagerProxy

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
