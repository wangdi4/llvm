; This test checks that LLVM emits an error for not vectorized loop when
; annotated with the 'llvm.loop.intel.vector.assert' metadata
;
; Original Source:
;   void foo() {
;   #pragma vector always assert
;     for(;;){}
;   }

; RUN: not opt -S -passes=vplan-vec,transform-warning < %s 2>&1 | FileCheck %s
; CHECK: error: {{.*}}: loop not vectorized:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo() {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  br label %for.cond, !llvm.loop !0
}

!0 = distinct !{!0, !1, !2, !3}
!1 = !{!"llvm.loop.vectorize.ignore_profitability"}
!2 = !{!"llvm.loop.vectorize.enable", i1 true}
!3 = !{!"llvm.loop.intel.vector.assert"}
