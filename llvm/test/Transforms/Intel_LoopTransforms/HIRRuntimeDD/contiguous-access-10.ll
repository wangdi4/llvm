; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that HIRRuntimeDD was applied because the access
; is contiguous in the innermost loop. The goal of this test case is to
; make sure that RuntimeDD was applied when there are multiple dimensions.
; It was created from the following test case:

; void foo(int A[][100], int B[][100], int m, int t) {
;   for (int i = 0; i < 100; i++) {
;     if (m != i) {
;       A[i][0] = m;
;     }
;     for (int j = 0; j < 100; j++) {
;       int t0 = B[i][2 * j];
;       A[4 * i][t] = t0;
;       int t1 = B[i][2 * j + 1];
;       A[4 * i][t + 3] = t1;
;     }
;   }
; }

; The loopnest is imperfect on purpose. It was created this way in order to
; try triggering the RuntimeDD for the innermost loop.

; HIR before transformation

;  BEGIN REGION { }
;        + DO i1 = 0, 99, 1   <DO_LOOP>
;        |   if (i1 != %m)
;        |   {
;        |      (%A)[i1][0] = %m;
;        |   }
;        |
;        |   + DO i2 = 0, 99, 1   <DO_LOOP>
;        |   |   %3 = (%B)[i1][2 * i2];
;        |   |   (%A)[4 * i1][%t] = %3;
;        |   |   %5 = (%B)[i1][2 * i2 + 1];
;        |   |   (%A)[4 * i1][%t + 3] = %5;
;        |   + END LOOP
;        + END LOOP
;  END REGION


; HIR after transformation

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   if (i1 != %m)
; CHECK:        |   {
; CHECK:        |      (%A)[i1][0] = %m;
; CHECK:        |   }
; CHECK:        |   %mv.test = &((%B)[i1][199]) >=u &((%A)[4 * i1][%t]);
; CHECK:        |   %mv.test2 = &((%A)[4 * i1][%t + 3]) >=u &((%B)[i1][0]);
; CHECK:        |   %mv.and = %mv.test  &  %mv.test2;
; CHECK:        |   if (%mv.and == 0)
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, 99, 1   <DO_LOOP>  <MVTag: 39>
; CHECK:        |      |   %3 = (%B)[i1][2 * i2];
; CHECK:        |      |   (%A)[4 * i1][%t] = %3;
; CHECK:        |      |   %5 = (%B)[i1][2 * i2 + 1];
; CHECK:        |      |   (%A)[4 * i1][%t + 3] = %5;
; CHECK:        |      + END LOOP
; CHECK:        |   }
; CHECK:        |   else
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, 99, 1   <DO_LOOP>  <MVTag: 39> <nounroll> <novectorize>
; CHECK:        |      |   %3 = (%B)[i1][2 * i2];
; CHECK:        |      |   (%A)[4 * i1][%t] = %3;
; CHECK:        |      |   %5 = (%B)[i1][2 * i2 + 1];
; CHECK:        |      |   (%A)[4 * i1][%t + 3] = %5;
; CHECK:        |      + END LOOP
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPA100_iS0_ii(ptr nocapture noundef writeonly %A, ptr nocapture noundef readonly %B, i32 noundef %m, i64 noundef %t) local_unnamed_addr #0 {
entry:
  %add24 = add nsw i64 %t, 3
  %0 = zext i32 %m to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup5
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup5
  %indvars.iv53 = phi i64 [ 0, %entry ], [ %indvars.iv.next54, %for.cond.cleanup5 ]
  %cmp1.not = icmp eq i64 %indvars.iv53, %0
  br i1 %cmp1.not, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr %A, i64 %indvars.iv53, i64 0
  store i32 %m, ptr %arrayidx2, align 4, !tbaa !3
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %1 = shl nsw i64 %indvars.iv53, 2
  %arrayidx15 = getelementptr inbounds [100 x i32], ptr %A, i64 %1, i64 %t
  %arrayidx26 = getelementptr inbounds [100 x i32], ptr %A, i64 %1, i64 %add24
  br label %for.body6

for.cond.cleanup5:                                ; preds = %for.body6
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond56.not = icmp eq i64 %indvars.iv.next54, 100
  br i1 %exitcond56.not, label %for.cond.cleanup, label %for.body, !llvm.loop !8

for.body6:                                        ; preds = %if.end, %for.body6
  %indvars.iv = phi i64 [ 0, %if.end ], [ %indvars.iv.next, %for.body6 ]
  %2 = shl nuw nsw i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr %B, i64 %indvars.iv53, i64 %2
  %3 = load i32, ptr %arrayidx10, align 4, !tbaa !3
  store i32 %3, ptr %arrayidx15, align 4, !tbaa !3
  %4 = or i64 %2, 1
  %arrayidx20 = getelementptr inbounds [100 x i32], ptr %B, i64 %indvars.iv53, i64 %4
  %5 = load i32, ptr %arrayidx20, align 4, !tbaa !3
  store i32 %5, ptr %arrayidx26, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup5, label %for.body6, !llvm.loop !10
}

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
