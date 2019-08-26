;  2 separate reductions in a loop
;   for (long int i=0; i< n; i++) {
;			s2  += b[i];
;			s1  += a[i];
;			s1  += a[i+1] +1;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-safe-reduction-analysis   | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s
; CHECK:  %s2.019 = %s2.019  +  %0; <Safe Reduction>
; CHECK:  %add2 = %s1.018  +  %1; <Safe Reduction>
; CHECK:  %s1.018 = %add2  +  %add5; <Safe Reduction>

; ModuleID = 'sum8.c'
source_filename = "sum8.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common global [10000 x float] zeroinitializer, align 16
@a = common global [10000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub(i64 %n, i64 %n2, i64 %n3, float* nocapture %sum1, float* nocapture %sum2) #0 {
entry:
  %cmp17 = icmp sgt i64 %n, 0
  br i1 %cmp17, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %s2.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %add, %for.body ]
  %s1.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %add6, %for.body ]
  store float %s1.0.lcssa, float* %sum1, align 4, !tbaa !1
  store float %s2.0.lcssa, float* %sum2, align 4, !tbaa !1
  ret i32 0

for.body:                                         ; preds = %entry, %for.body
  %i.020 = phi i64 [ %add3, %for.body ], [ 0, %entry ]
  %s2.019 = phi float [ %add, %for.body ], [ 0.000000e+00, %entry ]
  %s1.018 = phi float [ %add6, %for.body ], [ 0.000000e+00, %entry ]
  %arrayidx = getelementptr inbounds [10000 x float], [10000 x float]* @b, i64 0, i64 %i.020
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %add = fadd float %s2.019, %0
  %arrayidx1 = getelementptr inbounds [10000 x float], [10000 x float]* @a, i64 0, i64 %i.020
  %1 = load float, float* %arrayidx1, align 4, !tbaa !1
  %add2 = fadd float %s1.018, %1
  %add3 = add nuw nsw i64 %i.020, 1
  %arrayidx4 = getelementptr inbounds [10000 x float], [10000 x float]* @a, i64 0, i64 %add3
  %2 = load float, float* %arrayidx4, align 4, !tbaa !1
  %add5 = fadd float %2, 1.000000e+00
  %add6 = fadd float %add2, %add5
  %exitcond = icmp eq i64 %add3, %n
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12282)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
