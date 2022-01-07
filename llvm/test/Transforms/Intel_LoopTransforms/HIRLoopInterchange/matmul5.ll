; for(k=0; k<N; k++)
 ;    for(j=0; j<N; j++)
  ;       for(i=0; i<N; i++)
   ;          c[i][j] = c[i][j] + a[i][k] * b[k][j];
; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -O2 -loopopt -debug-only=hir-loop-interchange -hir-loop-interchange -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="default<O2>,hir-loop-interchange" -aa-pipeline="basic-aa" -loopopt -debug-only=hir-loop-interchange -disable-output < %s 2>&1 | FileCheck %s
; CHECK: Interchanged:
; CHECK-SAME:  ( 3 1 2 )

; ModuleID = 'matmul5.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common global [1024 x [1024 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(i64 %N) #0 {
entry:
  %cmp.41 = icmp sgt i64 %N, 0
  br i1 %cmp.41, label %for.cond.4.preheader.preheader, label %for.end.19

for.cond.4.preheader.preheader:                   ; preds = %entry, %for.inc.17
  %k.042 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %entry ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.inc.14, %for.cond.4.preheader.preheader
  %j.039 = phi i64 [ %inc15, %for.inc.14 ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %k.042, i64 %j.039
  %0 = load double, double* %arrayidx11, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %i.037 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %i.037, i64 %j.039
  %1 = load double, double* %arrayidx7, align 8, !tbaa !1
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.037, i64 %k.042
  %2 = load double, double* %arrayidx9, align 8, !tbaa !1
  %mul = fmul double %2, %0
  %add = fadd double %1, %mul
  store double %add, double* %arrayidx7, align 8, !tbaa !1
  %inc = add nuw nsw i64 %i.037, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.inc.14, label %for.body.6

for.inc.14:                                       ; preds = %for.body.6
  %inc15 = add nuw nsw i64 %j.039, 1
  %exitcond44 = icmp eq i64 %inc15, %N
  br i1 %exitcond44, label %for.inc.17, label %for.body.6.lr.ph

for.inc.17:                                       ; preds = %for.inc.14
  %inc18 = add nuw nsw i64 %k.042, 1
  %exitcond45 = icmp eq i64 %inc18, %N
  br i1 %exitcond45, label %for.end.19, label %for.cond.4.preheader.preheader

for.end.19:                                       ; preds = %for.inc.17, %entry
  ret i32 0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1456) (llvm/branches/loopopt 1546)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
