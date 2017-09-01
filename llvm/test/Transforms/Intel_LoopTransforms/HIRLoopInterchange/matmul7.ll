; for(i=0; i<N; i++) 
 ;  for(j=0; j<N; j++) 
  ;   for(k=0; k<N; k++) 
   ;      c[i][j] = c[i][j] + a[i][k] * b[k][j] + d[j] + d[j+1];
; REQUIRES: asserts 
; RUN: opt -O2 -loopopt -debug -hir-loop-interchange < %s 2>&1 | FileCheck %s
; CHECK: Interchanged:
; CHECK-SAME:  ( 1 3 2 )  
; XFAIL: * 
; Framework error:    copy stmt found between  i & j loop
;

; ModuleID = 'matmul7.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common global [1024 x [1024 x double]] zeroinitializer, align 16
@d = common global [1024 x double] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(i64 %N) #0 {
entry:
  %cmp.48 = icmp sgt i64 %N, 0
  br i1 %cmp.48, label %for.cond.4.preheader.preheader, label %for.end.24

for.cond.4.preheader.preheader:                   ; preds = %entry, %for.inc.22
  %i.049 = phi i64 [ %inc23, %for.inc.22 ], [ 0, %entry ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.cond.4.for.inc.19_crit_edge, %for.cond.4.preheader.preheader
  %j.046 = phi i64 [ %inc20, %for.cond.4.for.inc.19_crit_edge ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %i.049, i64 %j.046
  %arrayidx12 = getelementptr inbounds [1024 x double], [1024 x double]* @d, i64 0, i64 %j.046
  %0 = load double, double* %arrayidx12, align 8, !tbaa !1
  %add14 = add nuw nsw i64 %j.046, 1
  %arrayidx15 = getelementptr inbounds [1024 x double], [1024 x double]* @d, i64 0, i64 %add14
  %1 = load double, double* %arrayidx15, align 8, !tbaa !1
  %arrayidx7.promoted = load double, double* %arrayidx7, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %2 = phi double [ %arrayidx7.promoted, %for.body.6.lr.ph ], [ %add16, %for.body.6 ]
  %k.044 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.049, i64 %k.044
  %3 = load double, double* %arrayidx9, align 8, !tbaa !1
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %k.044, i64 %j.046
  %4 = load double, double* %arrayidx11, align 8, !tbaa !1
  %mul = fmul double %3, %4
  %add = fadd double %2, %mul
  %add13 = fadd double %0, %add
  %add16 = fadd double %1, %add13
  %inc = add nuw nsw i64 %k.044, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.cond.4.for.inc.19_crit_edge, label %for.body.6

for.cond.4.for.inc.19_crit_edge:                  ; preds = %for.body.6
  store double %add16, double* %arrayidx7, align 8, !tbaa !1
  %inc20 = add nuw nsw i64 %j.046, 1
  %exitcond51 = icmp eq i64 %inc20, %N
  br i1 %exitcond51, label %for.inc.22, label %for.body.6.lr.ph

for.inc.22:                                       ; preds = %for.cond.4.for.inc.19_crit_edge
  %inc23 = add nuw nsw i64 %i.049, 1
  %exitcond52 = icmp eq i64 %inc23, %N
  br i1 %exitcond52, label %for.end.24, label %for.cond.4.preheader.preheader

for.end.24:                                       ; preds = %for.inc.22, %entry
  ret i32 0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 1711)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
