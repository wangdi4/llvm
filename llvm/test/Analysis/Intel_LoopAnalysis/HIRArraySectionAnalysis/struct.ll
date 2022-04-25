; RUN: opt < %s -tbaa -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-array-section-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline="basic-aa,tbaa" -passes="hir-ssa-deconstruction,print<hir-array-section-analysis>" -disable-output 2>&1 | FileCheck %s

; Check the result of analysis.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   (%a)[0].1[i1][i1] = 5;
;       |   (%a)[0].0[i1] = 2;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:   + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:   %a: *
; CHECK:   + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

%struct.A = type { [100 x i32], [10 x [10 x i32]] }

define dso_local void @foo(%struct.A* nocapture %a) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.011 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %idxprom = zext i32 %i.011 to i64
  %arrayidx2 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 1, i64 %idxprom, i64 %idxprom, !intel-tbaa !2
  store i32 5, i32* %arrayidx2, align 4, !tbaa !2
  %arrayidx4 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0, i64 %idxprom, !intel-tbaa !10
  store i32 2, i32* %arrayidx4, align 4, !tbaa !10
  %inc = add nuw nsw i32 %i.011, 1
  %exitcond = icmp eq i32 %inc, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 400}
!3 = !{!"struct@A", !4, i64 0, !8, i64 400}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"array@_ZTSA10_A10_i", !9, i64 0}
!9 = !{!"array@_ZTSA10_i", !5, i64 0}
!10 = !{!3, !5, i64 0}
