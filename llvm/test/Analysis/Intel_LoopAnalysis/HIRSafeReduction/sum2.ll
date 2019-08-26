;    for (long int i=0; i< n; i++)
;        for (long int j=0; j< n; j++)
;           s1  +=  a[j][i];
;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s
; CHECK:   DO i2 = 0, %n + -1, 1
; CHECK:   %s1.022 = %s1.022  +  %0; <Safe Reduction>
;
; ModuleID = 'sum2.c'
source_filename = "sum2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub(i64 %n, i64 %n2, i64 %n3, float* nocapture %s) #0 {
entry:
  %cmp21 = icmp sgt i64 %n, 0
  br i1 %cmp21, label %for.cond1.preheader, label %for.cond.cleanup

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %i.023 = phi i64 [ %inc7, %for.cond.cleanup3 ], [ 0, %entry ]
  %s1.022 = phi float [ %add, %for.cond.cleanup3 ], [ 0.000000e+00, %entry ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  %s1.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %add, %for.cond.cleanup3 ]
  store float %s1.0.lcssa, float* %s, align 4, !tbaa !1
  ret i32 0

for.cond.cleanup3:                                ; preds = %for.body4
  %inc7 = add nuw nsw i64 %i.023, 1
  %exitcond26 = icmp eq i64 %inc7, %n
  br i1 %exitcond26, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %j.020 = phi i64 [ %inc, %for.body4 ], [ 0, %for.cond1.preheader ]
  %s1.119 = phi float [ %add, %for.body4 ], [ %s1.022, %for.cond1.preheader ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @a, i64 0, i64 %j.020, i64 %i.023
  %0 = load float, float* %arrayidx5, align 4, !tbaa !1
  %add = fadd float %s1.119, %0
  %inc = add nuw nsw i64 %j.020, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12282)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
