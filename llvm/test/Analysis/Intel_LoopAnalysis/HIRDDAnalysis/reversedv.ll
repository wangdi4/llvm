;        for (i1=1; i1 <= 40; i1++) 
;            for (i2=1; i2 <= 41; i2++) 
;                for (i3=1 ; i3 <= 42; i3++) 
;                    for (i4=1; i4 <= 43; i4++) 
;                        A[i3][i4][i2][i1] = A[i3+1][i4-1][i2-1][i1+n] + 1 ;
;

; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' for function 'sub1'
; CHECK-DAG: ANTI (* < > <)
; CHECK-DAG: FLOW (* > < >)
; ModuleID = 'reverse.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [50 x [50 x [50 x [50 x double]]]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub1(i64 %n, i64 %m) #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.26, %entry
  %i1.049 = phi i64 [ 1, %entry ], [ %inc27, %for.inc.26 ]
  %add = add nsw i64 %i1.049, %n
  br label %for.cond.4.preheader

for.cond.4.preheader:                             ; preds = %for.inc.23, %for.cond.1.preheader
  %i2.048 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc24, %for.inc.23 ]
  %sub = add nsw i64 %i2.048, -1
  br label %for.cond.7.preheader

for.cond.7.preheader:                             ; preds = %for.inc.20, %for.cond.4.preheader
  %i3.047 = phi i64 [ 1, %for.cond.4.preheader ], [ %inc21, %for.inc.20 ]
  %add11 = add nuw nsw i64 %i3.047, 1
  br label %for.body.9

for.body.9:                                       ; preds = %for.body.9, %for.cond.7.preheader
  %i4.046 = phi i64 [ 1, %for.cond.7.preheader ], [ %inc, %for.body.9 ]
  %sub10 = add nsw i64 %i4.046, -1
  %arrayidx14 = getelementptr inbounds [50 x [50 x [50 x [50 x double]]]], [50 x [50 x [50 x [50 x double]]]]* @A, i64 0, i64 %add11, i64 %sub10, i64 %sub, i64 %add
  %0 = load double, double* %arrayidx14, align 8, !tbaa !1
  %add15 = fadd double %0, 1.000000e+00
  %arrayidx19 = getelementptr inbounds [50 x [50 x [50 x [50 x double]]]], [50 x [50 x [50 x [50 x double]]]]* @A, i64 0, i64 %i3.047, i64 %i4.046, i64 %i2.048, i64 %i1.049
  store double %add15, double* %arrayidx19, align 8, !tbaa !1
  %inc = add nuw nsw i64 %i4.046, 1
  %exitcond = icmp eq i64 %inc, 44
  br i1 %exitcond, label %for.inc.20, label %for.body.9

for.inc.20:                                       ; preds = %for.body.9
  %inc21 = add nuw nsw i64 %i3.047, 1
  %exitcond50 = icmp eq i64 %inc21, 43
  br i1 %exitcond50, label %for.inc.23, label %for.cond.7.preheader

for.inc.23:                                       ; preds = %for.inc.20
  %inc24 = add nuw nsw i64 %i2.048, 1
  %exitcond51 = icmp eq i64 %inc24, 42
  br i1 %exitcond51, label %for.inc.26, label %for.cond.4.preheader

for.inc.26:                                       ; preds = %for.inc.23
  %inc27 = add nuw nsw i64 %i1.049, 1
  %exitcond52 = icmp eq i64 %inc27, 41
  br i1 %exitcond52, label %for.end.28, label %for.cond.1.preheader

for.end.28:                                       ; preds = %for.inc.26
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1456) (llvm/branches/loopopt 1607)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}



