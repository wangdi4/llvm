; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that HIRRuntimeDD wasn't applied because the access
; is not contiguous in the innermost loop. It was created from the following
; test case:

; void foo(int *A, int *B, int m, int t) {
;   for (int i = 0; i < 100; i++) {
;     if (m != i) {
;       B[i] = m;
;     }
;     for (int j = 0; j < 100; j++) {
;       int t0 = B[2 * j];
;       A[4 * i + t] = t0;
;       int t1 = B[2 * j + 10];
;       A[4 * i + t + 1] = t1;
;       int t2 = B[2 * j + 20];
;       A[4 * i + t + 2] = t2;
;       int t3 = B[2 * j + 30];
;       A[4 * i + t + 3] = t3;
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
;        |      (%B)[i1] = %m;
;        |   }
;        |
;        |   + DO i2 = 0, 99, 1   <DO_LOOP>
;        |   |   %8 = (%B)[2 * i2];
;        |   |   (%A)[4 * i1 + sext.i32.i64(%t)] = %8;
;        |   |   %10 = (%B)[2 * i2 + 10];
;        |   |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %10;
;        |   |   %12 = (%B)[2 * i2 + 20];
;        |   |   (%A)[4 * i1 + sext.i32.i64(%t) + 2] = %12;
;        |   |   %14 = (%B)[2 * i2 + 30];
;        |   |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %14;
;        |   + END LOOP
;        + END LOOP
;  END REGION

; HIR after transformation

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   if (i1 != %m)
; CHECK:        |   {
; CHECK:        |      (%B)[i1] = %m;
; CHECK:        |   }
; CHECK:        |
; CHECK-NOT:    |   %mv.test = &((%B)[228]) >=u &((%A)[4 * i1 + sext.i32.i64(%t)]);
; CHECK-NOT:    |   %mv.test2 = &((%A)[4 * i1 + sext.i32.i64(%t) + 3]) >=u &((%B)[0]);
; CHECK-NOT:    |   %mv.and = %mv.test  &  %mv.test2;
; CHECK-NOT:    |   if (%mv.and == 0)
; CHECK:        |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   |   %8 = (%B)[2 * i2];
; CHECK:        |   |   (%A)[4 * i1 + sext.i32.i64(%t)] = %8;
; CHECK:        |   |   %10 = (%B)[2 * i2 + 10];
; CHECK:        |   |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %10;
; CHECK:        |   |   %12 = (%B)[2 * i2 + 20];
; CHECK:        |   |   (%A)[4 * i1 + sext.i32.i64(%t) + 2] = %12;
; CHECK:        |   |   %14 = (%B)[2 * i2 + 30];
; CHECK:        |   |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %14;
; CHECK:        |   + END LOOP
; CHECK-NOT:    |   else
; CHECK:        + END LOOP
; CHECK:   END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPiS_ii(ptr nocapture noundef writeonly %A, ptr nocapture noundef %B, i32 noundef %m, i32 noundef %t) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %t to i64
  %1 = zext i32 %m to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup4
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup4
  %indvars.iv76 = phi i64 [ 0, %entry ], [ %indvars.iv.next77, %for.cond.cleanup4 ]
  %cmp1.not = icmp eq i64 %indvars.iv76, %1
  br i1 %cmp1.not, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv76
  store i32 %m, ptr %arrayidx, align 4, !tbaa !3
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %2 = shl nsw i64 %indvars.iv76, 2
  %3 = add nsw i64 %2, %0
  %arrayidx10 = getelementptr inbounds i32, ptr %A, i64 %3
  %4 = add nsw i64 %3, 1
  %arrayidx19 = getelementptr inbounds i32, ptr %A, i64 %4
  %5 = add nsw i64 %3, 2
  %arrayidx28 = getelementptr inbounds i32, ptr %A, i64 %5
  %6 = add nsw i64 %3, 3
  %arrayidx37 = getelementptr inbounds i32, ptr %A, i64 %6
  br label %for.body5

for.cond.cleanup4:                                ; preds = %for.body5
  %indvars.iv.next77 = add nuw nsw i64 %indvars.iv76, 1
  %exitcond83.not = icmp eq i64 %indvars.iv.next77, 100
  br i1 %exitcond83.not, label %for.cond.cleanup, label %for.body, !llvm.loop !7

for.body5:                                        ; preds = %if.end, %for.body5
  %indvars.iv = phi i64 [ 0, %if.end ], [ %indvars.iv.next, %for.body5 ]
  %7 = shl nuw nsw i64 %indvars.iv, 1
  %arrayidx7 = getelementptr inbounds i32, ptr %B, i64 %7
  %8 = load i32, ptr %arrayidx7, align 4, !tbaa !3
  store i32 %8, ptr %arrayidx10, align 4, !tbaa !3
  %9 = add nuw nsw i64 %7, 10
  %arrayidx14 = getelementptr inbounds i32, ptr %B, i64 %9
  %10 = load i32, ptr %arrayidx14, align 4, !tbaa !3
  store i32 %10, ptr %arrayidx19, align 4, !tbaa !3
  %11 = add nuw nsw i64 %7, 20
  %arrayidx23 = getelementptr inbounds i32, ptr %B, i64 %11
  %12 = load i32, ptr %arrayidx23, align 4, !tbaa !3
  store i32 %12, ptr %arrayidx28, align 4, !tbaa !3
  %13 = add nuw nsw i64 %7, 30
  %arrayidx32 = getelementptr inbounds i32, ptr %B, i64 %13
  %14 = load i32, ptr %arrayidx32, align 4, !tbaa !3
  store i32 %14, ptr %arrayidx37, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup4, label %for.body5, !llvm.loop !9
}

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
