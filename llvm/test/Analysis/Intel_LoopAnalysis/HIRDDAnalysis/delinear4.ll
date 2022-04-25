;	for (long i = 1; i < 10000; i++)
;		for (long j = 1; j < n1; j++)
;			for (long k = 1; k < n2; k++)
;				A[k + n1 *j ] =	A[k + n1 *j ] + 1;
; This test  cannot be delinearized because  UB of k is not related to n1
; if delinearized, (* = =) is formed
; RUN: opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0  | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
; CHECK-NOT:  (%0)[%n1 * i2 + i3 + %n1 + 1] --> (%0)[%n1 * i2 + i3 + %n1 + 1] ANTI (* = =)
;Module Before HIR; ModuleID = 'delinear4.c'
source_filename = "delinear4.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global double* null, align 8

; Function Attrs: norecurse nounwind uwtable
define void @foo(i64 %n1, i64 %n2, i64 %n3) local_unnamed_addr #0 {
entry:
  %cmp233 = icmp sgt i64 %n1, 1
  %cmp631 = icmp sgt i64 %n2, 1
  %0 = load double*, double** @A, align 8
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.body:                                         ; preds = %for.cond.cleanup3, %entry
  %i.035 = phi i64 [ 1, %entry ], [ %inc17, %for.cond.cleanup3 ]
  br i1 %cmp233, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.body
  br label %for.body4

for.cond.cleanup3.loopexit:                       ; preds = %for.cond.cleanup7
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.body
  %inc17 = add nuw nsw i64 %i.035, 1
  %exitcond37 = icmp eq i64 %inc17, 10000
  br i1 %exitcond37, label %for.cond.cleanup, label %for.body

for.body4:                                        ; preds = %for.cond.cleanup7, %for.body4.lr.ph
  %j.034 = phi i64 [ 1, %for.body4.lr.ph ], [ %inc14, %for.cond.cleanup7 ]
  br i1 %cmp631, label %for.body8.lr.ph, label %for.cond.cleanup7

for.body8.lr.ph:                                  ; preds = %for.body4
  %mul = mul nsw i64 %j.034, %n1
  br label %for.body8

for.cond.cleanup7.loopexit:                       ; preds = %for.body8
  br label %for.cond.cleanup7

for.cond.cleanup7:                                ; preds = %for.cond.cleanup7.loopexit, %for.body4
  %inc14 = add nuw nsw i64 %j.034, 1
  %exitcond36 = icmp eq i64 %inc14, %n1
  br i1 %exitcond36, label %for.cond.cleanup3.loopexit, label %for.body4

for.body8:                                        ; preds = %for.body8, %for.body8.lr.ph
  %k.032 = phi i64 [ 1, %for.body8.lr.ph ], [ %inc, %for.body8 ]
  %add = add nsw i64 %k.032, %mul
  %arrayidx = getelementptr inbounds double, double* %0, i64 %add
  %1 = load double, double* %arrayidx, align 8, !tbaa !2
  %add9 = fadd double %1, 1.000000e+00
  store double %add9, double* %arrayidx, align 8, !tbaa !2
  %inc = add nuw nsw i64 %k.032, 1
  %exitcond = icmp eq i64 %inc, %n2
  br i1 %exitcond, label %for.cond.cleanup7.loopexit, label %for.body8
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

