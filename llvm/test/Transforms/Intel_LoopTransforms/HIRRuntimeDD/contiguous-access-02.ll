; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that HIRRuntimeDD was not applied since the memory is not
; being accessed contiguously in the loop. One of the entries won't produce a
; distance of 1.

; HIR before transformation

;  BEGIN REGION { }
;        + DO i1 = 0, 99, 1   <DO_LOOP>
;        |   %1 = (%B)[i1];
;        |   (%A)[4 * i1 + sext.i32.i64(%t)] = %1;
;        |   %4 = (%B)[i1 + 1];
;        |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %4;
;        |   %7 = (%B)[i1 + 2];
;        |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %7;
;        |   %10 = (%B)[i1 + 3];
;        |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %10;
;        + END LOOP
;  END REGION

; HIR after transformation

; CHECK:  BEGIN REGION { }
; CHECK-NOT:    | if (
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   %1 = (%B)[i1];
; CHECK:        |   (%A)[4 * i1 + sext.i32.i64(%t)] = %1;
; CHECK:        |   %4 = (%B)[i1 + 1];
; CHECK:        |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %4;
; CHECK:        |   %7 = (%B)[i1 + 2];
; CHECK:        |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %7;
; CHECK:        |   %10 = (%B)[i1 + 3];
; CHECK:        |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %10;
; CHECK:        + END LOOP
; CHECK-NOT:    | else
; CHECK:  END REGION

;Module Before HIR
; ModuleID = 'strided.c'
source_filename = "strided.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind uwtable
define dso_local void @foo(ptr nocapture noundef writeonly %A, ptr nocapture noundef readonly %B, i32 noundef %t) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %t to i64
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %2 = shl nuw nsw i64 %indvars.iv, 2
  %3 = add nsw i64 %2, %0
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %3
  store i32 %1, ptr %arrayidx2, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv.next
  %4 = load i32, ptr %arrayidx5, align 4, !tbaa !3
  %5 = add nsw i64 %3, 1
  %arrayidx10 = getelementptr inbounds i32, ptr %A, i64 %5
  store i32 %4, ptr %arrayidx10, align 4, !tbaa !3
  %6 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx13 = getelementptr inbounds i32, ptr %B, i64 %6
  %7 = load i32, ptr %arrayidx13, align 4, !tbaa !3
  %8 = add nsw i64 %3, 3
  %arrayidx18 = getelementptr inbounds i32, ptr %A, i64 %8
  store i32 %7, ptr %arrayidx18, align 4, !tbaa !3
  %9 = add nuw nsw i64 %indvars.iv, 3
  %arrayidx21 = getelementptr inbounds i32, ptr %B, i64 %9
  %10 = load i32, ptr %arrayidx21, align 4, !tbaa !3
  %11 = add nsw i64 %3, 3
  %arrayidx26 = getelementptr inbounds i32, ptr %A, i64 %11
  store i32 %10, ptr %arrayidx26, align 4, !tbaa !3
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !7

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { argmemonly nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
