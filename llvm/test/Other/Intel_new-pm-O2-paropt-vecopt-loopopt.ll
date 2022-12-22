; INTEL_CUSTOMIZATION
; RUN: opt -disable-verify -disable-output -verify-cfg-preserved=0 \
; RUN:     -debug-pass-manager  -passes='default<O2>' \
; RUN:     -paropt=31 -vecopt=true -loopopt=1 -S %s 2>&1 | FileCheck %s

;; This test is to be populated with pipeline for parvec with loopopt.
;; Now it checks that VPOCFGRestructuringPass is run before VPORename/Restore
;; in post-HIR cleanup phase.

; CHECK:        Running pass: HIRCodeGenPass
; CHECK-NEXT:   Invalidating analysis: HIRFrameworkAnalysis
; CHECK-NEXT:   Running pass: VPOCFGRestructuringPass
; CHECK-NEXT:   Running pass: VPORenameOperandsPass
; CHECK-NEXT:   Running analysis: WRegionInfoAnalysis
; CHECK-NEXT:   Running analysis: WRegionCollectionAnalysis
; CHECK-NEXT:   Running analysis: OptimizationRemarkEmitterAnalysis
; CHECK-NEXT:   Running pass: SimplifyCFGPass
; CHECK-NEXT:   Running pass: LowerSubscriptIntrinsicPass
; CHECK-NEXT:   Running pass: SROAPass
; CHECK-NEXT:   Running pass: GVNPass
; CHECK-NEXT:   Running analysis: MemoryDependenceAnalysis
; CHECK-NEXT:   Running analysis: PhiValuesAnalysis
; CHECK-NEXT:   Running pass: SROAPass
; CHECK-NEXT:   Running pass: VPOCFGRestructuringPass
; CHECK-NEXT:   Running pass: InstCombinePass
; CHECK-NEXT:   Running pass: LoopCarriedCSEPass
; CHECK-NEXT:   Running pass: DSEPass
; CHECK-NEXT:   Running analysis: MemorySSAAnalysis
; CHECK-NEXT:   Running pass: VPORestoreOperandsPass

; The IR below was taken from new-pm-O0-defaults.ll

declare void @bar() local_unnamed_addr

define void @foo(i32 %n) local_unnamed_addr {
entry:
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  tail call void @bar()
  %cmp = icmp eq i32 %iv, %n
  br i1 %cmp, label %exit, label %loop
exit:
  ret void
}
; end INTEL_CUSTOMIZATION
