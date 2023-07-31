; Source code
;
;int A[10000];
;int B[10000];
;void foo(){
;  int i;
;  for(i = 0; i < 1000; i++){
;    A[i+1] = A[i] + A[i+5000];
;  }
;}
;
; Check that when prefetchW is enabled, we set 'IsWrite' flag for the previous candidate if there
; is any lval ref in the same memory stream, though the previous candidate may not be a lval ref.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" -hir-prefetching-skip-non-modified-regions="false" -hir-prefetching-skip-num-memory-streams-check="true" -hir-prefetching-trip-count-threshold=500 -hir-prefetching-num-cachelines-threshold=100 -hir-prefetching-skip-AVX2-check="true" -hir-prefetching-prefetchw="true" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<16>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<3>                |   %0 = (@A)[0][i1];
;<6>                |   %2 = (@A)[0][i1 + 5000];
;<10>               |   (@A)[0][i1 + 1] = %0 + %2;
;<16>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: foo
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:           |   %0 = (@A)[0][i1];
; CHECK:           |   %2 = (@A)[0][i1 + 5000];
; CHECK:           |   (@A)[0][i1 + 1] = %0 + %2;
; CHECK:           |   @llvm.prefetch.p0(&((i8*)(@A)[0][i1 + 40]),  1,  3,  1);
; CHECK:           |   @llvm.prefetch.p0(&((i8*)(@A)[0][i1 + 5040]),  0,  3,  1);
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [10000 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [10000 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10000 x i32], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %1 = add nuw nsw i64 %indvars.iv, 5000
  %arrayidx2 = getelementptr inbounds [10000 x i32], ptr @A, i64 0, i64 %1, !intel-tbaa !2
  %2 = load i32, ptr %arrayidx2, align 4, !tbaa !2
  %add3 = add nsw i32 %2, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx6 = getelementptr inbounds [10000 x i32], ptr @A, i64 0, i64 %indvars.iv.next, !intel-tbaa !2
  store i32 %add3, ptr %arrayidx6, align 4, !tbaa !2
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !7

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
