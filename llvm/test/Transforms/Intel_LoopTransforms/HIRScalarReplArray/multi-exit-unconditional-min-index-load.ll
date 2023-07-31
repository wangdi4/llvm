; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-scalarrepl-array" -aa-pipeline="basic-aa" -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s

; Verify that we can scalar replace the load group {(%A)[i1], (%A)[i1 + 1]}
; because the min index load (%A)[i1] is unconditionally executed.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %indvars.iv.out = i1;
; CHECK: |   if ((%A)[i1 + 1] == (%A)[i1])
; CHECK: |   {
; CHECK: |      goto for.end.split.loop.exit;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: %scalarepl = (%A)[0];
; CHECK: + DO i1 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %indvars.iv.out = i1;
; CHECK: |   %scalarepl1 = (%A)[i1 + 1];
; CHECK: |   if (%scalarepl1 == %scalarepl)
; CHECK: |   {
; CHECK: |      goto for.end.split.loop.exit;
; CHECK: |   }
; CHECK: |   %scalarepl = %scalarepl1;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind readonly uwtable
define dso_local i32 @foo(ptr nocapture noundef readonly %A) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond:                                         ; preds = %for.body
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !3

for.body:                                         ; preds = %entry, %for.cond
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.cond ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !5
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx2, align 4, !tbaa !5
  %cmp3 = icmp eq i32 %0, %1
  br i1 %cmp3, label %for.end.split.loop.exit, label %for.cond, !llvm.loop !3

for.end.split.loop.exit:                          ; preds = %for.body
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %for.body ]
  %2 = trunc i64 %indvars.iv.lcssa to i32
  br label %for.end

for.end.loopexit:                                 ; preds = %for.cond
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.end.split.loop.exit
  %i.0.lcssa = phi i32 [ %2, %for.end.split.loop.exit ], [ 100, %for.end.loopexit ]
  ret i32 %i.0.lcssa
}

attributes #0 = { argmemonly nofree norecurse nosync nounwind readonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
