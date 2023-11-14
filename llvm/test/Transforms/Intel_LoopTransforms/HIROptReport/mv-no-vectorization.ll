; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-vec-dir-insert,print<hir>,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="scoped-noalias-aa,basic-aa" -disable-output -intel-opt-report=medium 2>&1 < %s | FileCheck %s 

; Verify that we are able to preserve negative vectorization opt report remark
; even when we do not generate code for the loop.

; CHECK: if (%mv.and == 0)
; CHECK: {
; CHECK:    %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC(),  QUAL.OMP.SAFELEN(20) ]
; CHECK:    + DO i1 = 0, 99, 1   <DO_LOOP>  <MVTag: 24>
; CHECK:    |   (%A)[2 * i1] = (%B)[2 * i1 + 5];
; CHECK:    |   (%A)[2 * i1 + 1] = (%B)[2 * i1];
; CHECK:    |   (%A)[2 * i1 + 40] = (%B)[2 * i1 + 1];
; CHECK:    + END LOOP
; CHECK:    @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:    + DO i1 = 0, 99, 1   <DO_LOOP>  <MVTag: 24> <nounroll> <novectorize>
; CHECK:    |   (%A)[2 * i1] = (%B)[2 * i1 + 5];
; CHECK:    |   (%A)[2 * i1 + 1] = (%B)[2 * i1];
; CHECK:    |   (%A)[2 * i1 + 40] = (%B)[2 * i1 + 1];
; CHECK:    + END LOOP
; CHECK: }

; Check that the pre loopopt remark is preserved on the underling loop and
; in addition we add the remark for performing multiversioning and not
; performing vectorization. Although, we are not generating multiverioned code,
; I am not sure we can do any better.
; Note that vectorizer is not emitting reason for non-profitability.

; CHECK:      LOOP BEGIN
; CHECK-NEXT: <Multiversioned v1>
; CHECK-NEXT:    remark #99999: Dummy remark for testing
; CHECK-NEXT:    remark #25228: Loop multiversioned for Data Dependence
; CHECK-NEXT:    remark #15335: loop was not vectorized: vectorization possible but seems inefficient. Use vector always directive or -vec-threshold0 to override
; CHECK-NEXT: LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr writeonly %A, ptr readonly %B) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = shl nuw nsw i64 %indvars.iv, 1
  %1 = add nuw nsw i64 %0, 5
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %1
  %2 = load i32, ptr %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds i32, ptr %A, i64 %0
  store i32 %2, ptr %arrayidx3, align 4
  %arrayidx6 = getelementptr inbounds i32, ptr %B, i64 %0
  %3 = load i32, ptr %arrayidx6, align 4
  %4 = or i64 %0, 1
  %arrayidx10 = getelementptr inbounds i32, ptr %A, i64 %4
  store i32 %3, ptr %arrayidx10, align 4
  %arrayidx14 = getelementptr inbounds i32, ptr %B, i64 %4
  %5 = load i32, ptr %arrayidx14, align 4
  %6 = add nuw nsw i64 %0, 40
  %arrayidx18 = getelementptr inbounds i32, ptr %A, i64 %6
  store i32 %5, ptr %arrayidx18, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !0

for.end:                                          ; preds = %for.body
  ret void
}

!0 = distinct !{!0, !1}
!1 = distinct !{!"intel.optreport", !3}
!3 = !{!"intel.optreport.remarks", !4}
!4 = !{!"intel.optreport.remark", i32 99999}
