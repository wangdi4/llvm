;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute -print-after=hir-loop-distribute -hir-loop-distribute-heuristics=mem-rec < %s 2>&1 | FileCheck %s
;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute -print-after=hir-loop-distribute -hir-loop-distribute-heuristics=mem-rec -disable-hir-loop-distribute < %s 2>&1 | FileCheck --check-prefix=CHECK-DISABLE %s
;we want to dist on 10-13 to remove loop carried dep
;          BEGIN REGION { }
;<29>         + DO i1 = 0, 99998, 1   <DO_LOOP>
;<28>         |   + DO i2 = 0, 99998, 1   <DO_LOOP>
;<5>          |   |   %0 = (@B)[0][i1 + 1][i2 + 1];
;<7>          |   |   %1 = (@C)[0][i1 + 1][i2 + 1];
;<8>          |   |   %add = %0  +  %1;
;<10>         |   |   (@A)[0][i1 + 1][i2 + 1] = %add;
;<13>         |   |   %3 = (@A)[0][i1 + 1][i2];
;<14>         |   |   %conv18 = %3  *  2.000000e+00;
;<16>         |   |   (@D)[0][i1 + 1][i2 + 1] = %conv18;
;<28>         |   + END LOOP
;<29>         + END LOOP

; CHECK: BEGIN REGION
; CHECK-NEXT: DO i1 = 0, 99998, 1
; CHECK-NEXT: DO i2 = 0, 99998, 1
; CHECK: (@A)[0][i1 + 1][i2 + 1] = 
; CHECK-NEXT: END LOOP

; CHECK-NEXT: DO i2 = 0, 99998, 1
; CHECK: (@A)[0][i1 + 1][i2] 
; CHECK: END LOOP
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END REGION

;do nothing if disable option is on
; CHECK-DISABLE-NOT: BEGIN REGION{{.*}}modified
; CHECK-DISABLE: DO i1 = 0, 99998, 1
; CHECK-DISABLE-NEXT: DO i2 = 0, 99998, 1
; CHECK-DISABLE-NOT: DO i1 = 0, 99998, 1
; CHECK-DISABLE-NOT: DO i2 = 0, 99998, 1
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [100000 x [100000 x float]] zeroinitializer, align 16
@B = global [100000 x [100000 x float]] zeroinitializer, align 16
@C = global [100000 x [100000 x float]] zeroinitializer, align 16
@D = global [100000 x [100000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @_Z3foov() {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.23, %entry
  %indvars.iv43 = phi i64 [ 1, %entry ], [ %indvars.iv.next44, %for.inc.23 ]
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %indvars.iv = phi i64 [ 1, %for.cond.1.preheader ], [ %indvars.iv.next, %for.body.3 ]
  %arrayidx5 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @B, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  %0 = load float, float* %arrayidx5, align 4
  %arrayidx9 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @C, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  %1 = load float, float* %arrayidx9, align 4
  %add = fadd float %0, %1
  %arrayidx13 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @A, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  store float %add, float* %arrayidx13, align 4
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx17 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @A, i64 0, i64 %indvars.iv43, i64 %2
  %3 = load float, float* %arrayidx17, align 4
  %conv18 = fmul float %3, 2.000000e+00
  %arrayidx22 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @D, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  store float %conv18, float* %arrayidx22, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond, label %for.inc.23, label %for.body.3

for.inc.23:                                       ; preds = %for.body.3
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next44, 100000
  br i1 %exitcond45, label %for.end.25, label %for.cond.1.preheader

for.end.25:                                       ; preds = %for.inc.23
  ret void
}



; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) 

