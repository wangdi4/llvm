; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR \
; RUN:     -debug-only=VPlanHCFGBuilder < %s 2>&1 \
; RUN:     | FileCheck %s

; REQUIRES: asserts

; CHECK: Max trip count is {{[0-9]+}} set by pragma loop count
; CHECK: Min trip count is {{[0-9]+}} set by pragma loop count
; CHECK: Average trip count is {{[0-9]+}} set by pragma loop count

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
define dso_local void @_Z3foov() local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* @N, align 4, !tbaa !2
  %cmp10 = icmp sgt i32 %0, 0
  br i1 %cmp10, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %1 = sext i32 %0 to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %3 = load i32, i32* %arrayidx2, align 4, !tbaa !6
  %add = add nsw i32 %3, %2
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %1
  br i1 %cmp, label %for.body, label %for.end.loopexit, !llvm.loop !8

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 0c33cc6c66834fefc27761ae0dd746a9d425fce7) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4c7f58006ee1efa9b91412043eb2a8b5ce8e4873)"}
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
