; RUN: opt -S -vplan-vec -vplan-build-vect-candidates=10000\
; RUN:     -debug-only=VPlanHCFGBuilder < %s 2>&1 \
; RUN:     | FileCheck %s

; REQUIRES: asserts

; CHECK: Min trip count is 6 set by pragma loop count
; CHECK-NEXT: Max trip count is 10 set by pragma loop count
; CHECK-NEXT: Average trip count is 8 set by pragma loop count

;Set the pragma loop_count min/max/avg in the Vectorizer LoopInfoAnalysis for HIR path
;int a[1024], b[1024];
;int N=1024;
;void foo()
; {
;   int i;
;   #pragma loop_count min(6)
;   #pragma loop_count max(10)
;   #pragma loop_count avg(8)
;   for (i = 0; i < N; i++)
;     b[i] = a[i]+b[i];
;}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@N = dso_local local_unnamed_addr global i32 1024, align 4

; Function Attrs: norecurse nounwind uwtable
define dso_local void @_Z3foov(i64 %TC) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !6
  %add = add nsw i32 %1, %0
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %TC
  br i1 %cmp, label %for.body, label %for.end.loopexit, !llvm.loop !8

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1024_i", !3, i64 0}
!8 = distinct !{!8, !9, !10, !11}
!9 = !{!"llvm.loop.intel.loopcount_minimum", i32 6}
!10 = !{!"llvm.loop.intel.loopcount_maximum", i32 10}
!11 = !{!"llvm.loop.intel.loopcount_average", i32 8}
