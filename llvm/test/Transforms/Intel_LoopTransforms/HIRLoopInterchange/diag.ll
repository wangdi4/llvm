; for (k = 0; k < N; k++)
; for (i = 0; i < N; i++)
; for (j = 0; j < N; j++)
; c[i][j] = c[i][j] + a[i][k] * b[k][j];

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange" -aa-pipeline="basic-aa" -hir-loop-interchange-print-diag=1 < %s 2>&1 | FileCheck %s --check-prefix=DIAGALL

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange" -aa-pipeline="basic-aa" -hir-loop-interchange-print-diag=1 -hir-loop-interchange-print-diag-func=sub < %s 2>&1 | FileCheck %s --check-prefix=DIAGFUNC

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange" -aa-pipeline="basic-aa" -hir-loop-interchange-print-diag=1 -hir-loop-interchange-print-diag-func=foo < %s 2>&1 | FileCheck %s --check-prefix=DIAGNOFUNC
;
;
; Test is the same as matmul3-negative.ll. Diagnosis message is printed depending on commandline options.

; No refs either in pre/post loop of the innermost or in the innermost loop itself
; suggests interchange of loops may help. Subscripts in all dimesions are
; well aligned with loopnest. Compare against matmul3.ll.
;
; <27>         + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; <28>         |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; <3>          |   |   %0 = (@a)[0][%k.043][i1];
; <11>         |   |   %mul = %0  *  (@b)[0][%k.043][i2];
; <12>         |   |   %add = (@c)[0][i1][i2]  +  %mul;
; <13>         |   |   (@c)[0][i1][i2] = %add;
; <28>         |   + END LOOP
; <27>         + END LOOP

; DIAGALL: Func: sub
; DIAGALL-SAME: No Interchange:  Current Loop nest is already most favorable to locality

; DIAGFUNC: Func: sub
; DIAGFUNC-SAME: No Interchange:  Current Loop nest is already most favorable to locality

; DIAGNOFUNC-NOT: Func: sub
; DIAGNOFUNC-NOT: No Interchange:  Current Loop nest is already most favorable to locality
;
; ModuleID = 'matmul3.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global[1024 x[1024 x double]] zeroinitializer, align 16
@a = common global[1024 x[1024 x double]] zeroinitializer, align 16
@b = common global[1024 x[1024 x double]] zeroinitializer, align 16

; Function Attrs : nounwind uwtable
define i32 @sub(i64 %N) #0 {
entry:
  %cmp.42 = icmp sgt i64 %N, 0
  br i1 %cmp.42, label %for.cond.4.preheader.preheader, label %for.end.19

for.cond.4.preheader.preheader:                   ; preds = %entry, %for.inc.17
  %k.043 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %entry ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.inc.14, %for.cond.4.preheader.preheader
  %i.040 = phi i64 [ %inc15, %for.inc.14 ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], ptr @a, i64 0, i64 %k.043, i64 %i.040
  %0 = load double, ptr %arrayidx9, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %j.038 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], ptr @c, i64 0, i64 %i.040, i64 %j.038
  %1 = load double, ptr %arrayidx7, align 8, !tbaa !1
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], ptr @b, i64 0, i64 %k.043, i64 %j.038
  %2 = load double, ptr %arrayidx11, align 8, !tbaa !1
  %mul = fmul double %0, %2
  %add = fadd double %1, %mul
  store double %add, ptr %arrayidx7, align 8, !tbaa !1
  %inc = add nuw nsw i64 %j.038, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.inc.14, label %for.body.6

for.inc.14:                                       ; preds = %for.body.6
  %inc15 = add nuw nsw i64 %i.040, 1
  %exitcond45 = icmp eq i64 %inc15, %N
  br i1 %exitcond45, label %for.inc.17, label %for.body.6.lr.ph

for.inc.17:                                       ; preds = %for.inc.14
  %inc18 = add nuw nsw i64 %k.043, 1
  %exitcond46 = icmp eq i64 %inc18, %N
  br i1 %exitcond46, label %for.end.19, label %for.cond.4.preheader.preheader

for.end.19:                                       ; preds = %for.inc.17, %entry
  ret i32 0
}

attributes #0 = {nounwind uwtable "disable-tail-calls" = "false" "less-precise-fpmad" = "false" "no-frame-pointer-elim" = "false" "no-infs-fp-math" = "false" "no-nans-fp-math" = "false" "stack-protector-buffer-size" = "8" "target-cpu" = "x86-64" "target-features" = "+sse,+sse2" "unsafe-fp-math" = "false" "use-soft-float" = "false"}

!llvm.ident = !{ !0 }

!0 = !{ !"clang version 3.8.0 (trunk 1456) (llvm/branches/loopopt 1546)" }
!1 = !{ !2, !2, i64 0 }
!2 = !{ !"double", !3, i64 0 }
!3 = !{ !"omnipotent char", !4, i64 0 }
!4 = !{ !"Simple C/C++ TBAA" }
