; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter" -disable-output -intel-opt-report=medium 2>&1 < %s | FileCheck %s

; Verify that we can preserve negative opt report remarks for the loopnest even
; when we do not generate code in HIRCodeGen.

; Incoming HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   + DO i2 = 0, 99, 1   <DO_LOOP> <novectorize>
; |   |   (%A)[i1 + i2] = i2;
; |   + END LOOP
; + END LOOP

; CHECK: LOOP BEGIN
; CHECK:     remark #15553: loop was not vectorized: outer loop is not an auto-vectorization candidate.

; CHECK:    LOOP BEGIN
; CHECK:        remark #15319: Loop was not vectorized: novector directive used
; CHECK:    LOOP END
; CHECK: LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture noundef writeonly %A) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc4
  %indvars.iv17 = phi i64 [ 0, %entry ], [ %indvars.iv.next18, %for.inc4 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv17
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %0
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.inc4, label %for.body3, !llvm.loop !0

for.inc4:                                         ; preds = %for.body3
  %indvars.iv.next18 = add nuw nsw i64 %indvars.iv17, 1
  %exitcond19.not = icmp eq i64 %indvars.iv.next18, 100
  br i1 %exitcond19.not, label %for.end6, label %for.cond1.preheader

for.end6:                                         ; preds = %for.inc4
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.vectorize.width", i32 1}
