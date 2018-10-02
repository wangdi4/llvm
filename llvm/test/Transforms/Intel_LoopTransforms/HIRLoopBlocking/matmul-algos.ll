; Check if 
; 1. Inner two levels of matmul are blocked using a K&R algorithm (prefix KANDR)
; 2. Outer two levels of matmul are blocked using Outer algorithm (prefix OUTER)
; 3. All three levels are blocked using default algorithm (prefix DEFAULT)
;
; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-loop-blocking -print-after=hir-loop-blocking -debug-only=hir-loop-blocking -disable-hir-loop-blocking-trip-count-check -hir-loop-blocking-algo=kandr -print-before=hir-loop-blocking -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s --check-prefix=KANDR
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-loop-blocking -print-after=hir-loop-blocking -debug-only=hir-loop-blocking -disable-hir-loop-blocking-trip-count-check -print-before=hir-loop-blocking  -hir-verify-cf-def-level  < %s 2>&1 | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-loop-blocking -print-after=hir-loop-blocking -debug-only=hir-loop-blocking -disable-hir-loop-blocking-trip-count-check -hir-loop-blocking-algo=outer -print-before=hir-loop-blocking -hir-verify-cf-def-level -analyze -hir-dd-analysis -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s --check-prefix=OUTER

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-loop-blocking -disable-hir-loop-blocking-trip-count-check -hir-loop-blocking-algo=kandr -hir-verify-cf-def-level 2>&1 < %s | FileCheck %s --check-prefix=KANDR
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-loop-blocking -disable-hir-loop-blocking-trip-count-check -hir-verify-cf-def-level  2>&1 < %s | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,print<hir>,hir-loop-blocking,print<hir>,print<hir-dd-analysis>" -aa-pipeline="basic-aa" -debug-only=hir-loop-blocking -disable-hir-loop-blocking-trip-count-check -hir-loop-blocking-algo=outer -hir-verify-cf-def-level -hir-dd-analysis-verify=Region 2>&1 < %s | FileCheck %s --check-prefix=OUTER

;
; for(i=0; i<N; i++)
;   for(j=0; j<N; j++)
;     for(k=0; k<N; k++)
;       c[i][j] = c[i][j] + a[i][k] * b[k][j];


; KANDR:     BEGIN REGION { modified }
; KANDR:           + DO i1 = 0, %N + -1, 1   
; KANDR:           |   + DO i2 = 0, %N + -1, 1
; KANDR:           |   |   + DO i3 = 0, %N + -1, 1
; KANDR:           |   |   |   %0 = (@c)[0][i1][i3];
; KANDR:           |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; KANDR:           |   |   |   %0 = %0  +  %mul;
; KANDR:           |   |   |   (@c)[0][i1][i3] = %0;
; KANDR:           |   |   + END LOOP
; KANDR:           |   + END LOOP
; KANDR:           + END LOOP
; KANDR:     END REGION

; KANDR: Blocked at Level 2
; KANDR: Blocked at Level 3


; KANDR:     BEGIN REGION { modified }
; KANDR:    + DO i1 = 0, (%N + -1)/u64, 1
; KANDR:    |   %min = (-64 * i1 + %N + -1 <= 63) ? -64 * i1 + %N + -1 : 63;
; KANDR:    |   
; KANDR:    |   + DO i2 = 0, (%N + -1)/u64, 1  
; KANDR:    |   |   %min3 = (-64 * i2 + %N + -1 <= 63) ? -64 * i2 + %N + -1 : 63;
; KANDR:    |   |   
; KANDR:    |   |   + DO i3 = 0, %N + -1, 1 
; KANDR:    |   |   |   + DO i4 = 0, %min, 1 
; KANDR:    |   |   |   |   + DO i5 = 0, %min3, 1 
; KANDR:    |   |   |   |   |   %0 = (@c)[0][i3][64 * i2 + i5];
; KANDR:    |   |   |   |   |   %mul = (@a)[0][i3][64 * i1 + i4]  *  (@b)[0][64 * i1 + i4][64 * i2 + i5];
; KANDR:    |   |   |   |   |   %0 = %0  +  %mul;
; KANDR:    |   |   |   |   |   (@c)[0][i3][64 * i2 + i5] = %0;
; KANDR:    |   |   |   |   + END LOOP
; KANDR:    |   |   |   + END LOOP
; KANDR:    |   |   + END LOOP
; KANDR:    |   + END LOOP
; KANDR:    + END LOOP
; KANDR:   END REGION


; OUTER:       BEGIN REGION { modified }
; OUTER:             + DO i1 = 0, %N + -1, 1
; OUTER:             |   + DO i2 = 0, %N + -1, 1
; OUTER:             |   |   + DO i3 = 0, %N + -1, 1
; OUTER:             |   |   |   %0 = (@c)[0][i1][i3];
; OUTER:             |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; OUTER:             |   |   |   %0 = %0  +  %mul;
; OUTER:             |   |   |   (@c)[0][i1][i3] = %0;
; OUTER:             |   |   + END LOOP
; OUTER:             |   + END LOOP
; OUTER:             + END LOOP
; OUTER:       END REGION

; OUTER: Blocked at Level 1
; OUTER: Blocked at Level 2


; OUTER: BEGIN REGION { modified }
; OUTER:   + DO i1 = 0, (%N + -1)/u64, 1  
; OUTER:   |   %min = (-64 * i1 + %N + -1 <= 63) ? -64 * i1 + %N + -1 : 63;
; OUTER:   |   
; OUTER:   |   + DO i2 = 0, (%N + -1)/u64, 1 
; OUTER:   |   |   %min3 = (-64 * i2 + %N + -1 <= 63) ? -64 * i2 + %N + -1 : 63;
; OUTER:   |   |   
; OUTER:   |   |   + DO i3 = 0, %min, 1  
; OUTER:   |   |   |   + DO i4 = 0, %min3, 1
; OUTER:   |   |   |   |   + DO i5 = 0, %N + -1, 1 
; OUTER:   |   |   |   |   |   %0 = (@c)[0][64 * i1 + i3][i5];
; OUTER:   |   |   |   |   |   %mul = (@a)[0][64 * i1 + i3][64 * i2 + i4]  *  (@b)[0][64 * i2 + i4][i5];
; OUTER:   |   |   |   |   |   %0 = %0  +  %mul;
; OUTER:   |   |   |   |   |   (@c)[0][64 * i1 + i3][i5] = %0;
; OUTER:   |   |   |   |   + END LOOP
; OUTER:   |   |   |   + END LOOP
; OUTER:   |   |   + END LOOP
; OUTER:   |   + END LOOP
; OUTER:   + END LOOP
; OUTER: END REGION


; Verify that DD is able to create '=' DV for IVs coupled due to stripmining like [64 * i1 + i3].

; OUTER-DAG: (@c)[0][64 * i1 + i3][i5] --> (@c)[0][64 * i1 + i3][i5] FLOW (= * = * =)
; OUTER-DAG: (@c)[0][64 * i1 + i3][i5] --> (@c)[0][64 * i1 + i3][i5] OUTPUT (= * = * =)
; OUTER-DAG: (@c)[0][64 * i1 + i3][i5] --> (@c)[0][64 * i1 + i3][i5] ANTI (= * = * =)


; DEFAULT:       BEGIN REGION { modified }
; DEFAULT:             + DO i1 = 0, %N + -1, 1
; DEFAULT:             |   + DO i2 = 0, %N + -1, 1
; DEFAULT:             |   |   + DO i3 = 0, %N + -1, 1
; DEFAULT:             |   |   |   %0 = (@c)[0][i1][i3];
; DEFAULT:             |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; DEFAULT:             |   |   |   %0 = %0  +  %mul;
; DEFAULT:             |   |   |   (@c)[0][i1][i3] = %0;
; DEFAULT:             |   |   + END LOOP
; DEFAULT:             |   + END LOOP
; DEFAULT:             + END LOOP
; DEFAULT:       END REGION

; DEFAULT: Blocked at Level 1
; DEFAULT: Blocked at Level 2
; DEFAULT: Blocked at Level 3

; DEFAULT: BEGIN REGION { modified }
; DEFAULT:      + DO i1 = 0, (%N + -1)/u64, 1   
; DEFAULT:      |   %min = (-64 * i1 + %N + -1 <= 63) ? -64 * i1 + %N + -1 : 63;
; DEFAULT:      |   
; DEFAULT:      |   + DO i2 = 0, (%N + -1)/u64, 1   
; DEFAULT:      |   |   %min3 = (-64 * i2 + %N + -1 <= 63) ? -64 * i2 + %N + -1 : 63;
; DEFAULT:      |   |   
; DEFAULT:      |   |   + DO i3 = 0, (%N + -1)/u64, 1   
; DEFAULT:      |   |   |   %min4 = (-64 * i3 + %N + -1 <= 63) ? -64 * i3 + %N + -1 : 63;
; DEFAULT:      |   |   |   
; DEFAULT:      |   |   |   + DO i4 = 0, %min, 1   
; DEFAULT:      |   |   |   |   + DO i5 = 0, %min3, 1   
; DEFAULT:      |   |   |   |   |   + DO i6 = 0, %min4, 1   
; DEFAULT:      |   |   |   |   |   |   %0 = (@c)[0][64 * i1 + i4][64 * i3 + i6];
; DEFAULT:      |   |   |   |   |   |   %mul = (@a)[0][64 * i1 + i4][64 * i2 + i5]  *  (@b)[0][64 * i2 + i5][64 * i3 + i6];
; DEFAULT:      |   |   |   |   |   |   %0 = %0  +  %mul;
; DEFAULT:      |   |   |   |   |   |   (@c)[0][64 * i1 + i4][64 * i3 + i6] = %0;
; DEFAULT:      |   |   |   |   |   + END LOOP
; DEFAULT:      |   |   |   |   + END LOOP
; DEFAULT:      |   |   |   + END LOOP
; DEFAULT:      |   |   + END LOOP
; DEFAULT:      |   + END LOOP
; DEFAULT:      + END LOOP
; DEFAULT: END REGION


; ModuleID = 'matmul-algos.ll'
source_filename = "matmul-algos.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common global [1024 x [1024 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(i64 %N) #0 {
entry:
  %cmp.41 = icmp sgt i64 %N, 0
  br i1 %cmp.41, label %for.cond.4.preheader.preheader.preheader, label %for.end.19

for.cond.4.preheader.preheader.preheader:         ; preds = %entry
  br label %for.cond.4.preheader.preheader

for.cond.4.preheader.preheader:                   ; preds = %for.cond.4.preheader.preheader.preheader, %for.inc.17
  %i.042 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %for.cond.4.preheader.preheader.preheader ]
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
