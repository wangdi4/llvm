; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-distribute-loopnest,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Check that we distribute the following loop with independent refs that are vectorizable
; in separate loopnests.

; Before Distribution

;          BEGIN REGION { }
;                + DO i1 = 0, 99, 1   <DO_LOOP>
;                |   (@A)[0][i1] = 1.000000e+00;
;                |
;                |   + DO i2 = 0, 99, 1   <DO_LOOP>
;                |   |   (@B)[0][i1][i2] = 5.000000e+00;
;                |   + END LOOP
;                + END LOOP
;          END REGION

; After Distribution

; CHECK:   BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   (@A)[0][i1] = 1.000000e+00;
; CHECK:         + END LOOP

; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:         |   |   (@B)[0][i1][i2] = 5.000000e+00;
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
; CHECK:   END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(write, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc8
  %indvars.iv18 = phi i64 [ 0, %entry ], [ %indvars.iv.next19, %for.inc8 ]
  %arrayidx = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv18
  store float 1.000000e+00, ptr %arrayidx, align 4
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx7 = getelementptr inbounds [100 x [100 x float]], ptr @B, i64 0, i64 %indvars.iv18, i64 %indvars.iv
  store float 5.000000e+00, ptr %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.inc8, label %for.body3

for.inc8:                                         ; preds = %for.body3
  %indvars.iv.next19 = add nuw nsw i64 %indvars.iv18, 1
  %exitcond20.not = icmp eq i64 %indvars.iv.next19, 100
  br i1 %exitcond20.not, label %for.end10, label %for.body

for.end10:                                        ; preds = %for.inc8
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind memory(write, argmem: none, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }


