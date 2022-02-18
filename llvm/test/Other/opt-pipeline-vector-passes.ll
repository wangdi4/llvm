; RUN: opt -disable-verify -debug-pass-manager -passes='default<O1>' -force-vector-width=4 -S %s 2>&1 | FileCheck %s --check-prefixes=O1
; RUN: opt -disable-verify -debug-pass-manager -passes='default<O1>' -enable-lv -force-vector-width=4 -S %s 2>&1 | FileCheck %s --check-prefixes=O1-LV
; RUN: opt -disable-verify -debug-pass-manager -passes='default<O2>' -force-vector-width=4 -S %s 2>&1 | FileCheck %s --check-prefixes=O2
; RUN: opt -disable-verify -debug-pass-manager -passes='default<O2>' -enable-lv -force-vector-width=4 -S %s 2>&1 | FileCheck %s --check-prefixes=O2-LV ;INTEL
; RUN: opt -disable-verify -debug-pass-manager -passes='default<O2>' -force-vector-width=4 -extra-vectorizer-passes -S %s 2>&1 | FileCheck %s --check-prefixes=O2_EXTRA

; When the loop doesn't get vectorized, no extra vector passes should run.
; RUN: opt -disable-verify -debug-pass-manager -passes='default<O2>' -force-vector-width=0 -extra-vectorizer-passes -S %s 2>&1 | FileCheck %s --check-prefixes=O2

; REQUIRES: asserts

; INTEL_CUSTOMIZATION
; The loop vectorizer is not run by default at both -O1/-O2.

; Removed LoopVectorizePass pass as it is disabled by default

; SLP does not run at -O1. Loop vectorization runs, but it only
; works on loops explicitly annotated with pragmas.
; O1-NOT:    Running pass: SLPVectorizerPass
; O1:        Running pass: VectorCombinePass

; With the enable-lv option:
; The loop vectorizer still runs at both -O1/-O2 even with the
; debug flag, but it only works on loops explicitly annotated
; with pragmas.
; SLP does not run at -O1. Loop vectorization runs, but it only
; works on loops explicitly annotated with pragmas.
; O1-LV-LABEL:  Running pass: LoopVectorizePass
; O1-LV-NOT:    Running pass: SLPVectorizerPass
; O1-LV:        Running pass: VectorCombinePass

; Everything runs at -O2, except LV, which is disabled by default
; O2:        Running pass: SLPVectorizerPass
; O2:        Running pass: VectorCombinePass

; Everything runs at -O2 with -enable-lv, including LV.
; O2-LV-LABEL:  Running pass: LoopVectorizePass
; O2-LV-NOT:    Running pass: EarlyCSEPass
; O2-LV-NOT:    Running pass: LICMPass
; O2-LV:        Running pass: SLPVectorizerPass
; O2-LV:        Running pass: VectorCombinePass
; END INTEL_CUSTOMIZATION

; Optionally run cleanup passes.
; O2_EXTRA-LABEL: Running pass: EarlyCSEPass ;INTEL
; O2_EXTRA: Running pass: CorrelatedValuePropagationPass
; O2_EXTRA: Running pass: InstCombinePass
; O2_EXTRA: Running pass: LICMPass
; O2_EXTRA: Running pass: SimpleLoopUnswitchPass
; O2_EXTRA: Running pass: SimplifyCFGPass
; O2_EXTRA: Running pass: InstCombinePass
; O2_EXTRA: Running pass: SLPVectorizerPass
; O2_EXTRA: Running pass: EarlyCSEPass
; O2_EXTRA: Running pass: VectorCombinePass

define i64 @f(i1 %cond, i32* %src, i32* %dst) {
entry:
  br label %loop

loop:
  %i = phi i64 [ 0, %entry ], [ %inc, %loop ]
  %src.i = getelementptr i32, i32* %src, i64 %i
  %src.v = load i32, i32* %src.i
  %add = add i32 %src.v, 10
  %dst.i = getelementptr i32, i32* %dst, i64 %i
  store i32 %add, i32* %dst.i
  %inc = add nuw nsw i64 %i, 1
  %ec = icmp ne i64 %inc, 1000
  br i1 %ec, label %loop, label %exit

exit:
  ret i64 %i
}
