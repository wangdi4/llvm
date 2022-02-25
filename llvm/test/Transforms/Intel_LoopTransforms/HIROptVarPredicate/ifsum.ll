;  Checking for a clean compile.  Need to invoke VPlan which will call
;  Reduction analysis.
;
;   for (int i=0; i < N; i++) {
;    tsum = 0.0;
;    for (int j=0; j < N; j++) {
;      if ((j == winner)&&(Y[i][j] > 0.0)) {
; 	tsum += B[j];
;      }
;    }
;    B[i] = tsum;
;  }
;
; REQUIRES: asserts
; RUN: opt < %s  -enable-new-pm=0 -hir-ssa-deconstruction   -hir-temp-cleanup  -analyze -hir-safe-reduction-analysis -hir-opt-var-predicate  -print-after=hir-opt-var-predicate | FileCheck %s
; RUN: opt < %s  -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>,hir-opt-var-predicate,print<hir>"  2>&1 -disable-output | FileCheck %s
;
; CHECK: <Safe Reduction>
;
; ModuleID = 'art.c'
source_filename = "art.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@winner = common dso_local local_unnamed_addr global i32 0, align 4
@Y = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local float @art_match(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp34 = icmp sgt i32 %N, 0
  br i1 %cmp34, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %0 = load i32, i32* @winner, align 4
  %1 = zext i32 %0 to i64
  %arrayidx11 = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %1
  %wide.trip.count = sext i32 %N to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv38 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next39, %for.cond.cleanup3 ]
  %arrayidx7 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @Y, i64 0, i64 %indvars.iv38, i64 %1
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  %tsum.2.lcssa.lcssa = phi float [ %tsum.2.lcssa, %for.cond.cleanup3 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %tsum.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %tsum.2.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret float %tsum.0.lcssa

for.cond.cleanup3:                                ; preds = %for.inc
  %tsum.2.lcssa = phi float [ %tsum.2, %for.inc ]
  %arrayidx13 = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %indvars.iv38, !intel-tbaa !2
  store float %tsum.2.lcssa, float* %arrayidx13, align 4, !tbaa !2
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next39, %wide.trip.count
  br i1 %exitcond41, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.body4:                                        ; preds = %for.inc, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %tsum.131 = phi float [ 0.000000e+00, %for.body4.lr.ph ], [ %tsum.2, %for.inc ]
  %cmp5 = icmp eq i64 %indvars.iv, %1
  br i1 %cmp5, label %land.lhs.true, label %for.inc

land.lhs.true:                                    ; preds = %for.body4
  %2 = load float, float* %arrayidx7, align 4, !tbaa !7
  %cmp8 = fcmp ogt float %2, 0.000000e+00
  br i1 %cmp8, label %if.then, label %for.inc

if.then:                                          ; preds = %land.lhs.true
  %3 = load float, float* %arrayidx11, align 4, !tbaa !2
  %add = fadd float %tsum.131, %3
  br label %for.inc

for.inc:                                          ; preds = %for.body4, %land.lhs.true, %if.then
  %tsum.2 = phi float [ %add, %if.then ], [ %tsum.131, %land.lhs.true ], [ %tsum.131, %for.body4 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA100_A100_f", !9, i64 0}
!9 = !{!"array@_ZTSA100_f", !4, i64 0}
