; XFAIL: *
; REQUIRES: asserts
; RUN: opt -O2 -debug-only=hir-loop-interchange  -hir-loop-interchange -print-after=hir-loop-interchange < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-loop-interchange,print<hir>" -aa-pipeline="basic-aa" -O2 -debug-only=hir-loop-interchange  < %s 2>&1 | FileCheck %s
; TODO: ( 3 2 1 ) is the most preferrable permutation.
;       Currently, interchanged into (2 3 1).
;       Locality computation should be fixed.
; CHECK: Loopnest Interchanged: ( 1 2 3 ) --> ( 3 2 1 )
;
;IR Dump Before HIR Loop Interchange ***
;
;       BEGIN REGION { }
;             + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
;             |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
;             |   |   %0 = (@c)[0][i2][i1];
;             |   |
;             |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
;             |   |   |   %mul = (@a)[0][i2][i3]  *  (@b)[0][i3][i1];
;             |   |   |   %0 = %0  +  %mul;
;             |   |   + END LOOP
;             |   |
;             |   |   (@c)[0][i2][i1] = %0;
;             |   + END LOOP
;             + END LOOP
;       END REGION
;

; ModuleID = 'matmul6.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common global [1024 x [1024 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(i64 %N) #0 {
entry:
  %cmp.42 = icmp sgt i64 %N, 0
  br i1 %cmp.42, label %for.cond.4.preheader.preheader, label %for.end.19

for.cond.4.preheader.preheader:                   ; preds = %entry, %for.inc.17
  %j.043 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %entry ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.cond.4.for.inc.14_crit_edge, %for.cond.4.preheader.preheader
  %i.040 = phi i64 [ %inc15, %for.cond.4.for.inc.14_crit_edge ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %i.040, i64 %j.043
  %arrayidx7.promoted = load double, double* %arrayidx7, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %0 = phi double [ %arrayidx7.promoted, %for.body.6.lr.ph ], [ %add, %for.body.6 ]
  %k.038 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.040, i64 %k.038
  %1 = load double, double* %arrayidx9, align 8, !tbaa !1
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %k.038, i64 %j.043
  %2 = load double, double* %arrayidx11, align 8, !tbaa !1
  %mul = fmul double %1, %2
  %add = fadd double %0, %mul
  %inc = add nuw nsw i64 %k.038, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.cond.4.for.inc.14_crit_edge, label %for.body.6

for.cond.4.for.inc.14_crit_edge:                  ; preds = %for.body.6
  store double %add, double* %arrayidx7, align 8, !tbaa !1
  %inc15 = add nuw nsw i64 %i.040, 1
  %exitcond45 = icmp eq i64 %inc15, %N
  br i1 %exitcond45, label %for.inc.17, label %for.body.6.lr.ph

for.inc.17:                                       ; preds = %for.cond.4.for.inc.14_crit_edge
  %inc18 = add nuw nsw i64 %j.043, 1
  %exitcond46 = icmp eq i64 %inc18, %N
  br i1 %exitcond46, label %for.end.19, label %for.cond.4.preheader.preheader

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
