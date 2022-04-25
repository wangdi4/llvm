;   for (long i = 0; i <= n-2; i++) {
;        for (long j = 0; j <= m ; j++) {
;            A[ 2*i + 4*n*j + 2*n +2] += 1;
; RUN: opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0  | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; CHECK-DAG:  (%0)[2 * i1 + 4 * %n * i2 + 2 * %n + 2] --> (%0)[2 * i1 + 4 * %n * i2 + 2 * %n + 2] ANTI (= =)
;
;Module Before HIR; ModuleID = 'delinear1.c'
source_filename = "delinear1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global double* null, align 8

; Function Attrs: norecurse nounwind uwtable
define void @foo(i64 %n, i64 %m) local_unnamed_addr #0 {
entry:
  %cmp25 = icmp slt i64 %n, 2
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %cmp223 = icmp slt i64 %m, 0
  %0 = load double*, double** @A, align 8
  %mul5 = shl i64 %n, 2
  %1 = add i64 %n, -1
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup3, %for.body.lr.ph
  %i.026 = phi i64 [ 0, %for.body.lr.ph ], [ %inc12, %for.cond.cleanup3 ]
  br i1 %cmp223, label %for.cond.cleanup3, label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.body
  %tmp = add i64 %i.026, %n
  %tmp22 = shl i64 %tmp, 1
  %add8 = add i64 %tmp22, 2
  br label %for.body4

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.body
  %inc12 = add nuw nsw i64 %i.026, 1
  %exitcond27 = icmp eq i64 %inc12, %1
  br i1 %exitcond27, label %for.cond.cleanup.loopexit, label %for.body

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %j.024 = phi i64 [ 0, %for.body4.lr.ph ], [ %inc, %for.body4 ]
  %mul6 = mul nsw i64 %mul5, %j.024
  %add9 = add i64 %add8, %mul6
  %arrayidx = getelementptr inbounds double, double* %0, i64 %add9
  %2 = load double, double* %arrayidx, align 8, !tbaa !2
  %add10 = fadd double %2, 1.000000e+00
  store double %add10, double* %arrayidx, align 8, !tbaa !2
  %inc = add nuw nsw i64 %j.024, 1
  %exitcond = icmp eq i64 %j.024, %m
  br i1 %exitcond, label %for.cond.cleanup3.loopexit, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

