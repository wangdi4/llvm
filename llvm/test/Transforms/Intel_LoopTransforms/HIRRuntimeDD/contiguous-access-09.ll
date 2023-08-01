; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that HIRRuntimeDD was applied because the access
; is contiguous in the innermost loop. It was created from the following
; test case:

; void foo(int *A, int *B, int m, int t) {
;   for (int i = 0; i < 100; i++) {
;     if (m != i) {
;       B[i] = m;
;     }
;     for (int j = 0; j < 100; j++) {
;       int t0 = B[2 * j];
;       A[4 * i + t] = t0;
;       int t1 = B[2 * j + 1];
;       A[4 * i + t + 1] = t1;
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
;        |   |   %6 = (%B)[2 * i2];
;        |   |   (%A)[4 * i1 + sext.i32.i64(%t)] = %6;
;        |   |   %8 = (%B)[2 * i2 + 1];
;        |   |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %8;
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
; CHECK:        |   %mv.test = &((%B)[199]) >=u &((%A)[4 * i1 + sext.i32.i64(%t)]);
; CHECK:        |   %mv.test2 = &((%A)[4 * i1 + sext.i32.i64(%t) + 1]) >=u &((%B)[0]);
; CHECK:        |   %mv.and = %mv.test  &  %mv.test2;
; CHECK:        |   if (%mv.and == 0)
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, 99, 1   <DO_LOOP>  <MVTag: {{[0-9]+}}>
; CHECK:        |      |   %6 = (%B)[2 * i2];
; CHECK:        |      |   (%A)[4 * i1 + sext.i32.i64(%t)] = %6;
; CHECK:        |      |   %8 = (%B)[2 * i2 + 1];
; CHECK:        |      |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %8;
; CHECK:        |      + END LOOP
; CHECK:        |   }
; CHECK:        |   else
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, 99, 1   <DO_LOOP>  <MVTag: {{[0-9]+}}> <nounroll> <novectorize>
; CHECK:        |      |   %6 = (%B)[2 * i2];
; CHECK:        |      |   (%A)[4 * i1 + sext.i32.i64(%t)] = %6;
; CHECK:        |      |   %8 = (%B)[2 * i2 + 1];
; CHECK:        |      |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %8;
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
define dso_local void @_Z3fooPiS_ii(ptr nocapture noundef writeonly %A, ptr nocapture noundef %B, i32 noundef %m, i32 noundef %t) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %t to i64
  %1 = zext i32 %m to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup4
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup4
  %indvars.iv44 = phi i64 [ 0, %entry ], [ %indvars.iv.next45, %for.cond.cleanup4 ]
  %cmp1.not = icmp eq i64 %indvars.iv44, %1
  br i1 %cmp1.not, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv44
  store i32 %m, ptr %arrayidx, align 4, !tbaa !3
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %2 = shl nsw i64 %indvars.iv44, 2
  %3 = add nsw i64 %2, %0
  %arrayidx10 = getelementptr inbounds i32, ptr %A, i64 %3
  %4 = add nsw i64 %3, 1
  %arrayidx19 = getelementptr inbounds i32, ptr %A, i64 %4
  br label %for.body5

for.cond.cleanup4:                                ; preds = %for.body5
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond49.not = icmp eq i64 %indvars.iv.next45, 100
  br i1 %exitcond49.not, label %for.cond.cleanup, label %for.body, !llvm.loop !7

for.body5:                                        ; preds = %if.end, %for.body5
  %indvars.iv = phi i64 [ 0, %if.end ], [ %indvars.iv.next, %for.body5 ]
  %5 = shl nuw nsw i64 %indvars.iv, 1
  %arrayidx7 = getelementptr inbounds i32, ptr %B, i64 %5
  %6 = load i32, ptr %arrayidx7, align 4, !tbaa !3
  store i32 %6, ptr %arrayidx10, align 4, !tbaa !3
  %7 = or i64 %5, 1
  %arrayidx14 = getelementptr inbounds i32, ptr %B, i64 %7
  %8 = load i32, ptr %arrayidx14, align 4, !tbaa !3
  store i32 %8, ptr %arrayidx19, align 4, !tbaa !3
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
