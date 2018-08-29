; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we unroll i1 loop by 4 and i2 loop by 4 by 'equalizing' the unroll factors.

; + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; |   |   %0 = (@b)[0][i2][i1];
; |   |
; |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; |   |   |   %mul = (@a)[0][i3][i2]  *  %0;
; |   |   |   %add = (@c)[0][i3][i1]  +  %mul;
; |   |   |   (@c)[0][i3][i1] = %add;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; CHECK: %tgu = (%N)/u4;

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 256>
; CHECK: |   %tgu5 = (%N)/u4;
; CHECK: |
; CHECK: |   + DO i2 = 0, %tgu5 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 256>
; CHECK: |   |   %temp6 = (@b)[0][4 * i2][4 * i1];
; CHECK: |   |   %temp7 = (@b)[0][4 * i2][4 * i1 + 1];
; CHECK: |   |   %temp8 = (@b)[0][4 * i2][4 * i1 + 2];
; CHECK: |   |   %temp9 = (@b)[0][4 * i2][4 * i1 + 3];

; CHECK: |   |   %0 = (@b)[0][4 * i2 + 3][4 * i1 + 3];
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK: |   |   |   %mul = (@a)[0][i3][4 * i2]  *  %temp6;
; CHECK: |   |   |   %add = (@c)[0][i3][4 * i1]  +  %mul;
; CHECK: |   |   |   (@c)[0][i3][4 * i1] = %add;
; CHECK: |   |   |   %mul = (@a)[0][i3][4 * i2]  *  %temp7;
; CHECK: |   |   |   %add = (@c)[0][i3][4 * i1 + 1]  +  %mul;
; CHECK: |   |   |   (@c)[0][i3][4 * i1 + 1] = %add;


; Skipping rest of the loop body as it is too big.

; CHECK: |   + DO i2 = 4 * %tgu5, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK: |   |   %temp = (@b)[0][i2][4 * i1];
; CHECK: |   |   %temp3 = (@b)[0][i2][4 * i1 + 1];
; CHECK: |   |   %temp4 = (@b)[0][i2][4 * i1 + 2];
; CHECK: |   |   %0 = (@b)[0][i2][4 * i1 + 3];
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK: |   |   |   %mul = (@a)[0][i3][i2]  *  %temp;
; CHECK: |   |   |   %add = (@c)[0][i3][4 * i1]  +  %mul;
; CHECK: |   |   |   (@c)[0][i3][4 * i1] = %add;

; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: + DO i1 = 4 * %tgu, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK: |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK: |   |   %0 = (@b)[0][i2][i1];
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK: |   |   |   %mul = (@a)[0][i3][i2]  *  %0;
; CHECK: |   |   |   %add = (@c)[0][i3][i1]  +  %mul;
; CHECK: |   |   |   (@c)[0][i3][i1] = %add;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common global [1024 x [1024 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(i64 %N) #0 {
entry:
  %cmp.42 = icmp sgt i64 %N, 0
  br i1 %cmp.42, label %for.cond.4.preheader.preheader.preheader, label %for.end.19

for.cond.4.preheader.preheader.preheader:         ; preds = %entry
  br label %for.cond.4.preheader.preheader

for.cond.4.preheader.preheader:                   ; preds = %for.cond.4.preheader.preheader.preheader, %for.inc.17
  %j.043 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %for.cond.4.preheader.preheader.preheader ]
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
  br i1 %exitcond46, label %for.end.19.loopexit, label %for.cond.4.preheader.preheader

for.end.19.loopexit:                              ; preds = %for.inc.17
  br label %for.end.19

for.end.19:                                       ; preds = %for.end.19.loopexit, %entry
  ret i32 0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1456) (llvm/branches/loopopt 1546)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
