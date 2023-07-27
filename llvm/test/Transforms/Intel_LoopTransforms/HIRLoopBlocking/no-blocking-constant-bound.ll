; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa"  -hir-loop-blocking-algo=kandr 2>&1 < %s | FileCheck %s --check-prefix=KANDR
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa"  -hir-loop-blocking-algo=outer 2>&1 < %s | FileCheck %s --check-prefix=OUTER
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa"  2>&1 < %s | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa"  -hir-loop-blocking-blocksize=24 2>&1 < %s | FileCheck %s --check-prefix=USERBS

; Make sure blocking is not enabled with a constant trip count smaller than a default threshold.
; TODO: clean-up -hir-loop-interchange if possible so that  "modified" is not displayed.
;

; for(i=0; i<200; i++)
;   for(j=0; j<200; j++)
;     for(k=0; k<200; k++)
;       c[i][j] = c[i][j] + a[i][k] * b[k][j];

; KANDR: Function: sub

; KANDR:     BEGIN REGION { modified }
; KANDR:           + DO i1 = 0, 199, 1
; KANDR:           |   + DO i2 = 0, 199, 1
; KANDR:           |   |   + DO i3 = 0, 199, 1
; KANDR:           |   |   |   %0 = (@c)[0][i1][i3];
; KANDR:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; KANDR:           |   |   |   %0 = %0  +  %mul;
; KANDR:           |   |   |   (@c)[0][i1][i3] = %0;
; KANDR:           |   |   + END LOOP
; KANDR:           |   + END LOOP
; KANDR:           + END LOOP
; KANDR:     END REGION

; KANDR: Function: sub

; KANDR:     BEGIN REGION { modified }
; KANDR:           + DO i1 = 0, 199, 1
; KANDR:           |   + DO i2 = 0, 199, 1
; KANDR:           |   |   + DO i3 = 0, 199, 1
; KANDR:           |   |   |   %0 = (@c)[0][i1][i3];
; KANDR:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; KANDR:           |   |   |   %0 = %0  +  %mul;
; KANDR:           |   |   |   (@c)[0][i1][i3] = %0;
; KANDR:           |   |   + END LOOP
; KANDR:           |   + END LOOP
; KANDR:           + END LOOP
; KANDR:     END REGION

;

; OUTER: Function: sub

; OUTER:     BEGIN REGION { modified }
; OUTER:           + DO i1 = 0, 199, 1
; OUTER:           |   + DO i2 = 0, 199, 1
; OUTER:           |   |   + DO i3 = 0, 199, 1
; OUTER:           |   |   |   %0 = (@c)[0][i1][i3];
; OUTER:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; OUTER:           |   |   |   %0 = %0  +  %mul;
; OUTER:           |   |   |   (@c)[0][i1][i3] = %0;
; OUTER:           |   |   + END LOOP
; OUTER:           |   + END LOOP
; OUTER:           + END LOOP
; OUTER:     END REGION

; OUTER: Function: sub

; OUTER:     BEGIN REGION { modified }
; OUTER:           + DO i1 = 0, 199, 1
; OUTER:           |   + DO i2 = 0, 199, 1
; OUTER:           |   |   + DO i3 = 0, 199, 1
; OUTER:           |   |   |   %0 = (@c)[0][i1][i3];
; OUTER:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; OUTER:           |   |   |   %0 = %0  +  %mul;
; OUTER:           |   |   |   (@c)[0][i1][i3] = %0;
; OUTER:           |   |   + END LOOP
; OUTER:           |   + END LOOP
; OUTER:           + END LOOP
; OUTER:     END REGION

;
; DEFAULT: Function: sub

; DEFAULT:     BEGIN REGION { modified }
; DEFAULT:           + DO i1 = 0, 199, 1
; DEFAULT:           |   + DO i2 = 0, 199, 1
; DEFAULT:           |   |   + DO i3 = 0, 199, 1
; DEFAULT:           |   |   |   %0 = (@c)[0][i1][i3];
; DEFAULT:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; DEFAULT:           |   |   |   %0 = %0  +  %mul;
; DEFAULT:           |   |   |   (@c)[0][i1][i3] = %0;
; DEFAULT:           |   |   + END LOOP
; DEFAULT:           |   + END LOOP
; DEFAULT:           + END LOOP
; DEFAULT:     END REGION

; DEFAULT: Function: sub

; DEFAULT:     BEGIN REGION { modified }
; DEFAULT:           + DO i1 = 0, 199, 1
; DEFAULT:           |   + DO i2 = 0, 199, 1
; DEFAULT:           |   |   + DO i3 = 0, 199, 1
; DEFAULT:           |   |   |   %0 = (@c)[0][i1][i3];
; DEFAULT:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; DEFAULT:           |   |   |   %0 = %0  +  %mul;
; DEFAULT:           |   |   |   (@c)[0][i1][i3] = %0;
; DEFAULT:           |   |   + END LOOP
; DEFAULT:           |   + END LOOP
; DEFAULT:           + END LOOP
; DEFAULT:     END REGION
;

; USERBS: Function: sub

; USERBS:     BEGIN REGION { modified }
; USERBS:           + DO i1 = 0, 199, 1
; USERBS:           |   + DO i2 = 0, 199, 1
; USERBS:           |   |   + DO i3 = 0, 199, 1
; USERBS:           |   |   |   %0 = (@c)[0][i1][i3];
; USERBS:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; USERBS:           |   |   |   %0 = %0  +  %mul;
; USERBS:           |   |   |   (@c)[0][i1][i3] = %0;
; USERBS:           |   |   + END LOOP
; USERBS:           |   + END LOOP
; USERBS:           + END LOOP
; USERBS:     END REGION

; USERBS: Function: sub

; USERBS:     BEGIN REGION { modified }
; USERBS:           + DO i1 = 0, 199, 1
; USERBS:           |   + DO i2 = 0, 199, 1
; USERBS:           |   |   + DO i3 = 0, 199, 1
; USERBS:           |   |   |   %0 = (@c)[0][i1][i3];
; USERBS:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; USERBS:           |   |   |   %0 = %0  +  %mul;
; USERBS:           |   |   |   (@c)[0][i1][i3] = %0;
; USERBS:           |   |   + END LOOP
; USERBS:           |   + END LOOP
; USERBS:           + END LOOP
; USERBS:     END REGION
;

; ModuleID = 'matmul-constant-bound-algos.ll'
source_filename = "matmul-constant-bound-algos.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [200 x [200 x double]] zeroinitializer, align 16
@a = common global [200 x [200 x double]] zeroinitializer, align 16
@b = common global [200 x [200 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub() #0 {
entry:
  %cmp.41 = icmp sgt i64 200, 0
  br i1 %cmp.41, label %for.cond.4.preheader.preheader.preheader, label %for.end.19

for.cond.4.preheader.preheader.preheader:         ; preds = %entry
  br label %for.cond.4.preheader.preheader

for.cond.4.preheader.preheader:                   ; preds = %for.cond.4.preheader.preheader.preheader, %for.inc.17
  %i.042 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %for.cond.4.preheader.preheader.preheader ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.cond.4.for.inc.14_crit_edge, %for.cond.4.preheader.preheader
  %j.039 = phi i64 [ %inc15, %for.cond.4.for.inc.14_crit_edge ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx7 = getelementptr inbounds [200 x [200 x double]], ptr @c, i64 0, i64 %i.042, i64 %j.039
  %arrayidx7.promoted = load double, ptr %arrayidx7, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %0 = phi double [ %arrayidx7.promoted, %for.body.6.lr.ph ], [ %add, %for.body.6 ]
  %k.037 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx9 = getelementptr inbounds [200 x [200 x double]], ptr @a, i64 0, i64 %i.042, i64 %k.037
  %1 = load double, ptr %arrayidx9, align 8, !tbaa !1
  %arrayidx11 = getelementptr inbounds [200 x [200 x double]], ptr @b, i64 0, i64 %k.037, i64 %j.039
  %2 = load double, ptr %arrayidx11, align 8, !tbaa !1
  %mul = fmul double %1, %2
  %add = fadd double %0, %mul
  %inc = add nuw nsw i64 %k.037, 1
  %exitcond = icmp eq i64 %inc, 200
  br i1 %exitcond, label %for.cond.4.for.inc.14_crit_edge, label %for.body.6

for.cond.4.for.inc.14_crit_edge:                  ; preds = %for.body.6
  store double %add, ptr %arrayidx7, align 8, !tbaa !1
  %inc15 = add nuw nsw i64 %j.039, 1
  %exitcond44 = icmp eq i64 %inc15, 200
  br i1 %exitcond44, label %for.inc.17, label %for.body.6.lr.ph

for.inc.17:                                       ; preds = %for.cond.4.for.inc.14_crit_edge
  %inc18 = add nuw nsw i64 %i.042, 1
  %exitcond45 = icmp eq i64 %inc18, 200
  br i1 %exitcond45, label %for.end.19.loopexit, label %for.cond.4.preheader.preheader

for.end.19.loopexit:                              ; preds = %for.inc.17
  br label %for.end.19

for.end.19:                                       ; preds = %for.end.19.loopexit, %entry
  ret i32 0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1312) (llvm/branches/loopopt 1440)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
