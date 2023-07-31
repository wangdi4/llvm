; RUN: opt -disable-output -mattr=+avx512vl -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-enable-peel-rem-strip=0 < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -mattr=+avx512vl -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-enable-peel-rem-strip=0 < %s 2>&1 | FileCheck %s --check-prefix=DOLOOPCHECK
;
; LIT test to check that we do not vectorize remainder loops with call to sqrt
; intrinsic when the only use(s) of the call result occur in a FDIV instruction
; as the second operand and where the number of accuracy bits are set to 14.
; This is a workaround to avoid precision issues in FP computation in benchmarks.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; DOLOOPCHECK:         Function: novecrem
; DOLOOPCHECK:              BEGIN REGION
; DOLOOPCHECK-COUNT-2:            DO i1 =
; DOLOOPCHECK-NOT:                DO i1 =
; DOLOOPCHECK:              END REGION
;
; CHECK:               Function: novecrem
; CHECK:                    BEGIN REGION
; CHECK:                          DO i1 = 0, {{.*}}, 8
; CHECK:                          DO i1 = {{.*}}, {{.*}}, 1
; CHECK:                    END REGION
;
define void @novecrem(ptr %darr, i64 %n1) {
entry:
  %cmp7 = icmp sgt i64 %n1, 0
  br i1 %cmp7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.08 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, ptr %darr, i64 %l1.08
  %0 = load double, ptr %arrayidx, align 8
  %1 = tail call fast double @llvm.sqrt.f64(double %0) #0
  %div = fdiv fast double 1.000000e+00, %1
  store double %div, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.08, 1
  %exitcond.not = icmp eq i64 %inc, %n1
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !9

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.intel.vector.vecremainder", !"true"}

; DOLOOPCHECK:         Function: vecrem1
; DOLOOPCHECK:              BEGIN REGION
; DOLOOPCHECK-COUNT-2:            DO i1 =
; DOLOOPCHECK-NOT:                DO i1 =
; DOLOOPCHECK:              END REGION
;
; CHECK:               Function: vecrem1
; CHECK:                    BEGIN REGION
; CHECK:                          DO i1 = 0, {{.*}}, 8
; CHECK:                          DO i1 = {{.*}}, {{.*}}, 8
; CHECK:                    END REGION
;
define void @vecrem1(ptr %darr, i64 %n1) {
entry:
  %cmp7 = icmp sgt i64 %n1, 0
  br i1 %cmp7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.08 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, ptr %darr, i64 %l1.08
  %0 = load double, ptr %arrayidx, align 8
  %1 = tail call fast double @llvm.sqrt.f64(double %0) #0
  store double %1, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.08, 1
  %exitcond.not = icmp eq i64 %inc, %n1
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !11

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.intel.vector.vecremainder", !"true"}
; DOLOOPCHECK:         Function: vecrem2
; DOLOOPCHECK:              BEGIN REGION
; DOLOOPCHECK-COUNT-2:            DO i1 =
; DOLOOPCHECK-NOT:                DO i1 =
; DOLOOPCHECK:              END REGION
;
; CHECK:               Function: vecrem2
; CHECK:                    BEGIN REGION
; CHECK:                          DO i1 = 0, {{.*}}, 8
; CHECK:                          DO i1 = {{.*}}, {{.*}}, 8
; CHECK:                    END REGION
;
define void @vecrem2(ptr %darr, i64 %n1) {
entry:
  %cmp7 = icmp sgt i64 %n1, 0
  br i1 %cmp7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.08 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, ptr %darr, i64 %l1.08
  %0 = load double, ptr %arrayidx, align 8
  %1 = tail call fast double @llvm.sqrt.f64(double %0)
  %div = fdiv fast double 1.000000e+00, %1
  store double %div, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.08, 1
  %exitcond.not = icmp eq i64 %inc, %n1
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !13

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.intel.vector.vecremainder", !"true"}

declare double @llvm.sqrt.f64(double)
attributes #0 = { "imf-accuracy-bits"="14" "imf-accuracy-bits-sqrt"="14" }
