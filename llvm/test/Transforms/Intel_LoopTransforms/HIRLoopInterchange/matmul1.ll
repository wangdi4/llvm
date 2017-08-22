; for(i=0; i<N; i++) 
;   for(j=0; j<N; j++) 
;     for(k=0; k<N; k++) 
;       c[i][j] = c[i][j] + a[i][k] * b[k][j];
; REQUIRES: asserts 
; RUN: opt -O2  -loopopt -debug -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK: Interchanged:
; CHECK-SAME:  ( 1 3 2 )  
; Disable test now because safe-reduction is not ready
; XFAIL: *
; ModuleID = 'matmul.ll'
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
  %i.042 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %entry ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.cond.4.for.inc.14_crit_edge, %for.cond.4.preheader.preheader
  %j.039 = phi i64 [ %inc15, %for.cond.4.for.inc.14_crit_edge ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %i.042, i64 %j.039
  %arrayidx7.promoted = load double, double* %arrayidx7, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %0 = phi double [ %arrayidx7.promoted, %for.body.6.lr.ph ], [ %add, %for.body.6 ]
  %k.037 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.042, i64 %k.037
  %1 = load double, double* %arrayidx9, align 8, !tbaa !1
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %k.037, i64 %j.039
  %2 = load double, double* %arrayidx11, align 8, !tbaa !1
  %mul = fmul double %1, %2
  %add = fadd double %0, %mul
  %inc = add nuw nsw i64 %k.037, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.cond.4.for.inc.14_crit_edge, label %for.body.6

for.cond.4.for.inc.14_crit_edge:                  ; preds = %for.body.6
  store double %add, double* %arrayidx7, align 8, !tbaa !1
  %inc15 = add nuw nsw i64 %j.039, 1
  %exitcond44 = icmp eq i64 %inc15, %N
  br i1 %exitcond44, label %for.inc.17, label %for.body.6.lr.ph

for.inc.17:                                       ; preds = %for.cond.4.for.inc.14_crit_edge
  %inc18 = add nuw nsw i64 %i.042, 1
  %exitcond45 = icmp eq i64 %inc18, %N
  br i1 %exitcond45, label %for.end.19, label %for.cond.4.preheader.preheader

for.end.19:                                       ; preds = %for.inc.17, %entry
  ret i32 0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1312) (llvm/branches/loopopt 1440)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
