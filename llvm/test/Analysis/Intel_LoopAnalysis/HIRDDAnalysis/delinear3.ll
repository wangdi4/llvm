; 	for (long i = 1; i < 10000; i++)
; 		for (long j = 1; j < n1; j++)
; 			for (long k = 1; k < n1; k++)
;				A[k + n1 *j ] =	A[k + 1 + n1 *j ] + 1;
;
; RUN: opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0  | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
; CHECK-DAG:  (%0)[%n1 * i2 + i3 + %n1 + 2] --> (%0)[%n1 * i2 + i3 + %n1 + 1] ANTI (* = >)
; CHECK-DAG:  (%0)[%n1 * i2 + i3 + %n1 + 1] --> (%0)[%n1 * i2 + i3 + %n1 + 2] FLOW (* = <)
; CHECK-DAG:  (%0)[%n1 * i2 + i3 + %n1 + 1] --> (%0)[%n1 * i2 + i3 + %n1 + 1] OUTPUT (* = =) (? 0 0)
;
;Module Before HIR; ModuleID = 'delinear3.c'
source_filename = "delinear3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global double* null, align 8

; Function Attrs: norecurse nounwind uwtable
define void @foo(i64 %n1, i64 %n2, i64 %n3) local_unnamed_addr #0 {
entry:
  %cmp235 = icmp sgt i64 %n1, 1
  %0 = load double*, double** @A, align 8
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.body:                                         ; preds = %for.cond.cleanup3, %entry
  %i.037 = phi i64 [ 1, %entry ], [ %inc18, %for.cond.cleanup3 ]
  br i1 %cmp235, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.body
  br label %for.body8.lr.ph

for.cond.cleanup3.loopexit:                       ; preds = %for.cond.cleanup7
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.body
  %inc18 = add nuw nsw i64 %i.037, 1
  %exitcond39 = icmp eq i64 %inc18, 10000
  br i1 %exitcond39, label %for.cond.cleanup, label %for.body

for.body8.lr.ph:                                  ; preds = %for.body4.lr.ph, %for.cond.cleanup7
  %j.036 = phi i64 [ 1, %for.body4.lr.ph ], [ %inc15, %for.cond.cleanup7 ]
  %mul = mul nsw i64 %j.036, %n1
  br label %for.body8

for.cond.cleanup7:                                ; preds = %for.body8
  %inc15 = add nuw nsw i64 %j.036, 1
  %exitcond38 = icmp eq i64 %inc15, %n1
  br i1 %exitcond38, label %for.cond.cleanup3.loopexit, label %for.body8.lr.ph

for.body8:                                        ; preds = %for.body8, %for.body8.lr.ph
  %k.034 = phi i64 [ 1, %for.body8.lr.ph ], [ %add, %for.body8 ]
  %add = add nuw nsw i64 %k.034, 1
  %add9 = add nsw i64 %add, %mul
  %arrayidx = getelementptr inbounds double, double* %0, i64 %add9
  %1 = load double, double* %arrayidx, align 8, !tbaa !2
  %add10 = fadd double %1, 1.000000e+00
  %add12 = add nsw i64 %k.034, %mul
  %arrayidx13 = getelementptr inbounds double, double* %0, i64 %add12
  store double %add10, double* %arrayidx13, align 8, !tbaa !2
  %exitcond = icmp eq i64 %add, %n1
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8
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

