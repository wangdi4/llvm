; RUN: opt -S -passes=handle-pragma-vector-aligned < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %P1, ptr %P2, i64 %N) {
entry:
  %cmp7 = icmp sgt i64 %N, 0
  br i1 %cmp7, label %for.body.preheader, label %for.cond.cleanup

; CHECK-LABEL: for.body.preheader:
;
; CHECK-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr %P1, i64 16) ]
; CHECK-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr %P2, i64 16) ]
;
; CHECK-NEXT:    br label %for.body
for.body.preheader:
  br label %for.body

for.cond.cleanup.loopexit:
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void

for.body:
  %I.08 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %ptridx = getelementptr inbounds float, ptr %P1, i64 %I.08
  %0 = load float, ptr %ptridx, align 4
  %conv = sitofp i64 %I.08 to float
  %add = fadd fast float %0, %conv
  %ptridx1 = getelementptr inbounds float, ptr %P2, i64 %I.08
  store float %add, ptr %ptridx1, align 4
  %inc = add nuw nsw i64 %I.08, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !0
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.vector.aligned"}
