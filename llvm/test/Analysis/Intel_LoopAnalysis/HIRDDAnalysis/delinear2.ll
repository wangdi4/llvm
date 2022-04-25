;         for (long j = 1; j < n1; j++)
;            for (long k = 1; k < n1; k++)
;                A[k + n1 *j ]  += 1;
; RUN: opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0  | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
; CHECK-DAG:  (%0)[%n1 * i1 + i2 + %n1 + 1] --> (%0)[%n1 * i1 + i2 + %n1 + 1] ANTI (= =)
;
;Module Before HIR; ModuleID = 'delinear2.c'
source_filename = "delinear2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global double* null, align 8

; Function Attrs: norecurse nounwind uwtable
define void @foo(i64 %n1, i64 %n2, i64 %n3) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i64 %n1, 1
  br i1 %cmp19, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %0 = load double*, double** @A, align 8
  br label %for.body4.lr.ph

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4.lr.ph:                                  ; preds = %for.body.lr.ph, %for.cond.cleanup3
  %j.020 = phi i64 [ 1, %for.body.lr.ph ], [ %inc7, %for.cond.cleanup3 ]
  %mul = mul nsw i64 %j.020, %n1
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %inc7 = add nuw nsw i64 %j.020, 1
  %exitcond21 = icmp eq i64 %inc7, %n1
  br i1 %exitcond21, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %k.018 = phi i64 [ 1, %for.body4.lr.ph ], [ %inc, %for.body4 ]
  %add = add nsw i64 %k.018, %mul
  %arrayidx = getelementptr inbounds double, double* %0, i64 %add
  %1 = load double, double* %arrayidx, align 8, !tbaa !2
  %add5 = fadd double %1, 1.000000e+00
  store double %add5, double* %arrayidx, align 8, !tbaa !2
  %inc = add nuw nsw i64 %k.018, 1
  %exitcond = icmp eq i64 %inc, %n1
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
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

