; for(i=0; i<N; i++) 
 ;  for(j=0; j<N; j++)  
  ;   for(k=0; k<N; k++)     
   ;     c[i][j] = c[i][j] + a[i][k] * b[k][j] +  c2[i][k] * a[i][j];   
; REQUIRES: asserts
; RUN: opt -O2 -loopopt -debug  -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK: Interchanged:
; CHECK-SAME:  ( 1 3 2 )  
; XFAIL: *
;  Disable now because safe reduction is not ready

; ModuleID = 'matmul8.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common global [1024 x [1024 x double]] zeroinitializer, align 16
@c2 = common global [1024 x [1024 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(i64 %N) #0 {
entry:
  %cmp.51 = icmp sgt i64 %N, 0
  br i1 %cmp.51, label %for.cond.4.preheader.preheader, label %for.end.25

for.cond.4.preheader.preheader:                   ; preds = %entry, %for.inc.23
  %i.052 = phi i64 [ %inc24, %for.inc.23 ], [ 0, %entry ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.cond.4.for.inc.20_crit_edge, %for.cond.4.preheader.preheader
  %j.049 = phi i64 [ %inc21, %for.cond.4.for.inc.20_crit_edge ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %i.052, i64 %j.049
  %arrayidx15 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.052, i64 %j.049
  %0 = load double, double* %arrayidx15, align 8, !tbaa !1
  %arrayidx7.promoted = load double, double* %arrayidx7, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %1 = phi double [ %arrayidx7.promoted, %for.body.6.lr.ph ], [ %add17, %for.body.6 ]
  %k.047 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.052, i64 %k.047
  %2 = load double, double* %arrayidx9, align 8, !tbaa !1
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %k.047, i64 %j.049
  %3 = load double, double* %arrayidx11, align 8, !tbaa !1
  %mul = fmul double %2, %3
  %add = fadd double %1, %mul
  %arrayidx13 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c2, i64 0, i64 %i.052, i64 %k.047
  %4 = load double, double* %arrayidx13, align 8, !tbaa !1
  %mul16 = fmul double %4, %0
  %add17 = fadd double %add, %mul16
  %inc = add nuw nsw i64 %k.047, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.cond.4.for.inc.20_crit_edge, label %for.body.6

for.cond.4.for.inc.20_crit_edge:                  ; preds = %for.body.6
  store double %add17, double* %arrayidx7, align 8, !tbaa !1
  %inc21 = add nuw nsw i64 %j.049, 1
  %exitcond54 = icmp eq i64 %inc21, %N
  br i1 %exitcond54, label %for.inc.23, label %for.body.6.lr.ph

for.inc.23:                                       ; preds = %for.cond.4.for.inc.20_crit_edge
  %inc24 = add nuw nsw i64 %i.052, 1
  %exitcond55 = icmp eq i64 %inc24, %N
  br i1 %exitcond55, label %for.end.25, label %for.cond.4.preheader.preheader

for.end.25:                                       ; preds = %for.inc.23, %entry
  ret i32 0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 1711)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
