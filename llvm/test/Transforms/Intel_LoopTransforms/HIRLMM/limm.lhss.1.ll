; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
; (This test is based on Interchange/matmul2.ll)
;
; C Source Code:
;int foo(int **restrict A, int **restrict B, int **restrict C, const int N) {
;  int i, j, k;
;
;  //3 levels of loop nests, do matmul-like operations
;  //This is what hir-loop-interchange produced!!
;  for (i = 0; i < N; i++) { // i1
;    for (j = 0; j < N; j++) { //i2
;      for (k = 0; k < N; k++) { //i3
;        C[i][k] = C[i][k] +    B[j][k] * A[i][j];
;      }
;    }
;  }
;
;  return 0;
;}
;
; [LIMM Analysis]
;MemRefCollection, entries: 1
;  (@c)[0][i1][%j.043] {  R  W  } 1W : 1R  legal
;
;
; LIMM Opportunity
; - LILH:  (0)
; - LISS:  (0)
; - LILHSS:(1)
;
; Input HIR:
;
;          BEGIN REGION { modified }
;<27>            + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
;<28>            |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
;<3>             |   |   %0 = (@b)[0][i2][%j.043];
;<11>            |   |   %mul = (@a)[0][i1][i2]  *  %0;
;<12>            |   |   %add = (@c)[0][i1][%j.043]  +  %mul;
;<13>            |   |   (@c)[0][i1][%j.043] = %add;
;<28>            |   + END LOOP
;<27>            + END LOOP
;          END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK:        |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK:        |   |   %0 = (@b)[0][i2][%j.043];
; CHECK:        |   |   %mul = (@a)[0][i1][i2]  *  %0;
; CHECK:        |   |   %add = (@c)[0][i1][%j.043]  +  %mul;
; CHECK:        |   |   (@c)[0][i1][%j.043] = %add;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; ***
;
; Check that after hir-lmm, the loop-inv load(s)/store(s) are promoted/sinked properly
;
; CHECK: Function
;
; CHECK: BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK:        |      %limm = (@c)[0][i1][%j.043];
; CHECK:        |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK:        |   |   %0 = (@b)[0][i2][%j.043];
; CHECK:        |   |   %mul = (@a)[0][i1][i2]  *  %0;
; CHECK:        |   |   %add = %limm  +  %mul;
; CHECK:        |   |   %limm = %add;
; CHECK:        |   + END LOOP
; CHECK:        |      (@c)[0][i1][%j.043] = %limm;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; ModuleID = 'matmul2.c'
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

for.body.6.lr.ph:                                 ; preds = %for.inc.14, %for.cond.4.preheader.preheader
  %k.040 = phi i64 [ %inc15, %for.inc.14 ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %k.040, i64 %j.043
  %0 = load double, double* %arrayidx11, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %i.038 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %i.038, i64 %j.043
  %1 = load double, double* %arrayidx7, align 8, !tbaa !1
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.038, i64 %k.040
  %2 = load double, double* %arrayidx9, align 8, !tbaa !1
  %mul = fmul double %2, %0
  %add = fadd double %1, %mul
  store double %add, double* %arrayidx7, align 8, !tbaa !1
  %inc = add nuw nsw i64 %i.038, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.inc.14, label %for.body.6

for.inc.14:                                       ; preds = %for.body.6
  %inc15 = add nuw nsw i64 %k.040, 1
  %exitcond45 = icmp eq i64 %inc15, %N
  br i1 %exitcond45, label %for.inc.17, label %for.body.6.lr.ph

for.inc.17:                                       ; preds = %for.inc.14
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
