; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that HIRRuntimeDD was applied since the memory is being
; accessed contiguously in the loop. This case checks when the contiguous
; access happens in the store and load instructions.

; HIR before transformation

;  BEGIN REGION { }
;        + DO i1 = 0, 99, 1   <DO_LOOP>
;        |   %1 = (%B)[i1];
;        |   (%A)[4 * i1 + sext.i32.i64(%t)] = %1;
;        |   %temp1 = (%A)[4 * i1 + sext.i32.i64(%t)];
;        |   %4 = (%B)[i1 + 1];
;        |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %4;
;        |   %temp2 = (%A)[4 * i1 + sext.i32.i64(%t) + 1];
;        |   %7 = (%B)[i1 + 2];
;        |   (%A)[4 * i1 + sext.i32.i64(%t) + 2] = %7;
;        |   %temp3 = (%A)[4 * i1 + sext.i32.i64(%t) + 2];
;        |   %10 = (%B)[i1 + 3];
;        |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %10;
;        |   %temp4 = (%A)[4 * i1 + sext.i32.i64(%t) + 3];
;        + END LOOP
;  END REGION


; HIR after transformation

; CHECK:  BEGIN REGION { }
; CHECK:        %mv.test = &((%B)[102]) >=u &((%A)[sext.i32.i64(%t)]);
; CHECK:        %mv.test1 = &((%A)[sext.i32.i64(%t) + 399]) >=u &((%B)[0]);
; CHECK:        %mv.and = %mv.test  &  %mv.test1;
; CHECK:        if (%mv.and == 0)
; CHECK:        {
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>  <MVTag: {{[0-9]+}}>
; CHECK:           |   %1 = (%B)[i1];
; CHECK:           |   (%A)[4 * i1 + sext.i32.i64(%t)] = %1;
; CHECK:           |   %temp1 = (%A)[4 * i1 + sext.i32.i64(%t)];
; CHECK:           |   %4 = (%B)[i1 + 1];
; CHECK:           |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %4;
; CHECK:           |   %temp2 = (%A)[4 * i1 + sext.i32.i64(%t) + 1];
; CHECK:           |   %7 = (%B)[i1 + 2];
; CHECK:           |   (%A)[4 * i1 + sext.i32.i64(%t) + 2] = %7;
; CHECK:           |   %temp3 = (%A)[4 * i1 + sext.i32.i64(%t) + 2];
; CHECK:           |   %10 = (%B)[i1 + 3];
; CHECK:           |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %10;
; CHECK:           |   %temp4 = (%A)[4 * i1 + sext.i32.i64(%t) + 3];
; CHECK:           + END LOOP
; CHECK:        }
; CHECK:        else
; CHECK:        {
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>  <MVTag: {{[0-9]+}}> <nounroll> <novectorize>
; CHECK:           |   %1 = (%B)[i1];
; CHECK:           |   (%A)[4 * i1 + sext.i32.i64(%t)] = %1;
; CHECK:           |   %temp1 = (%A)[4 * i1 + sext.i32.i64(%t)];
; CHECK:           |   %4 = (%B)[i1 + 1];
; CHECK:           |   (%A)[4 * i1 + sext.i32.i64(%t) + 1] = %4;
; CHECK:           |   %temp2 = (%A)[4 * i1 + sext.i32.i64(%t) + 1];
; CHECK:           |   %7 = (%B)[i1 + 2];
; CHECK:           |   (%A)[4 * i1 + sext.i32.i64(%t) + 2] = %7;
; CHECK:           |   %temp3 = (%A)[4 * i1 + sext.i32.i64(%t) + 2];
; CHECK:           |   %10 = (%B)[i1 + 3];
; CHECK:           |   (%A)[4 * i1 + sext.i32.i64(%t) + 3] = %10;
; CHECK:           |   %temp4 = (%A)[4 * i1 + sext.i32.i64(%t) + 3];
; CHECK:           + END LOOP
; CHECK:        }
; CHECK:  END REGION


;Module Before HIR
; ModuleID = 'strided.c'
source_filename = "strided.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind uwtable
define dso_local void @foo(i32* nocapture noundef writeonly %A, i32* nocapture noundef readonly %B, i32 noundef %t) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %t to i64
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %2 = shl nuw nsw i64 %indvars.iv, 2
  %3 = add nsw i64 %2, %0
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %3
  store i32 %1, i32* %arrayidx2, align 4, !tbaa !3
  %tempArr1 = getelementptr inbounds i32, i32* %A, i64 %3
  %temp1 = load i32, i32* %tempArr1, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv.next
  %4 = load i32, i32* %arrayidx5, align 4, !tbaa !3
  %5 = add nsw i64 %3, 1
  %arrayidx10 = getelementptr inbounds i32, i32* %A, i64 %5
  store i32 %4, i32* %arrayidx10, align 4, !tbaa !3
  %tempArr2 = getelementptr inbounds i32, i32* %A, i64 %5
  %temp2 = load i32, i32* %tempArr2, align 4, !tbaa !3
  %6 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx13 = getelementptr inbounds i32, i32* %B, i64 %6
  %7 = load i32, i32* %arrayidx13, align 4, !tbaa !3
  %8 = add nsw i64 %3, 2
  %arrayidx18 = getelementptr inbounds i32, i32* %A, i64 %8
  store i32 %7, i32* %arrayidx18, align 4, !tbaa !3
  %tempArr3 = getelementptr inbounds i32, i32* %A, i64 %8
  %temp3 = load i32, i32* %tempArr3, align 4, !tbaa !3
  %9 = add nuw nsw i64 %indvars.iv, 3
  %arrayidx21 = getelementptr inbounds i32, i32* %B, i64 %9
  %10 = load i32, i32* %arrayidx21, align 4, !tbaa !3
  %11 = add nsw i64 %3, 3
  %arrayidx26 = getelementptr inbounds i32, i32* %A, i64 %11
  store i32 %10, i32* %arrayidx26, align 4, !tbaa !3
  %tempArr4 = getelementptr inbounds i32, i32* %A, i64 %11
  %temp4 = load i32, i32* %tempArr4, align 4, !tbaa !3
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
