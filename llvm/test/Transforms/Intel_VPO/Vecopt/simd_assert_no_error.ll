; Test checks that -vplan-simd-assert-no-error option emits a warning (not an error) for a non-vectorized loop
;
; Original source:
;   void foo() {
;     #pragma vector always assert
;     for(;;) {}
;   }

; RUN: opt -S -disable-output -passes=vplan-vec,transform-warning -vplan-simd-assert-no-error < %s 2>&1 | FileCheck %s
; CHECK: warning: {{.*}}: loop not vectorized:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  br label %for.cond, !llvm.loop !9
}

!9 = distinct !{!9, !12, !13}
!12 = !{!"llvm.loop.vectorize.enable", i1 true}
!13 = !{!"llvm.loop.intel.vector.assert"}
