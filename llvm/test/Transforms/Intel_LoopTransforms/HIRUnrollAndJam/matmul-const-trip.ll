; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we unroll i1 loop by 4 and i2 loop by 4 by 'equalizing' the unroll factors.

; CHECK: Function

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   %0 = (@A)[0][i1][i3];
; CHECK: |   |   |   %1 = (@B)[0][i3][i2];
; CHECK: |   |   |   %2 = (@C)[0][i1][i2];
; CHECK: |   |   |   (@C)[0][i1][i2] = %2 + (%0 * %1);
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: Function

; unroll & jam disabling pragma should be added to each loop which has been transformed.

; CHECK: + DO i1 = 0, 24, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   + DO i2 = 0, 24, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   |   + DO i3 = 0, 99, 1   <DO_LOOP>

; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK-NOT: DO i


; ModuleID = 'matmul_preproc.ll'
source_filename = "matmul_preproc.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x [100 x i32]] zeroinitializer, align 16
@B = common global [100 x [100 x i32]] zeroinitializer, align 16
@C = common global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc.20, %entry
  %i.03 = phi i32 [ 0, %entry ], [ %inc21, %for.inc.20 ]
  %j.02.in = bitcast i32 0 to i32
  br label %for.body.3

for.body.3:                                       ; preds = %for.inc.17, %for.body
  %j.02 = phi i32 [ 0, %for.body ], [ %inc18, %for.inc.17 ]
  %k.01.in = bitcast i32 0 to i32
  br label %for.body.6

for.body.6:                                       ; preds = %for.inc, %for.body.3
  %k.01 = phi i32 [ 0, %for.body.3 ], [ %inc, %for.inc ]
  %idxprom = sext i32 %k.01 to i64
  %idxprom7 = sext i32 %i.03 to i64
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %idxprom7, i64 %idxprom
  %0 = load i32, ptr %arrayidx8, align 4
  %idxprom9 = sext i32 %j.02 to i64
  %idxprom10 = sext i32 %k.01 to i64
  %arrayidx12 = getelementptr inbounds [100 x [100 x i32]], ptr @B, i64 0, i64 %idxprom10, i64 %idxprom9
  %1 = load i32, ptr %arrayidx12, align 4
  %mul = mul nsw i32 %0, %1
  %idxprom13 = sext i32 %j.02 to i64
  %idxprom14 = sext i32 %i.03 to i64
  %arrayidx16 = getelementptr inbounds [100 x [100 x i32]], ptr @C, i64 0, i64 %idxprom14, i64 %idxprom13
  %2 = load i32, ptr %arrayidx16, align 4
  %add = add nsw i32 %2, %mul
  store i32 %add, ptr %arrayidx16, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body.6
  %inc = add nsw i32 %k.01, 1
  %cmp5 = icmp slt i32 %inc, 100
  br i1 %cmp5, label %for.body.6, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc.17

for.inc.17:                                       ; preds = %for.end
  %inc18 = add nsw i32 %j.02, 1
  %cmp2 = icmp slt i32 %inc18, 100
  br i1 %cmp2, label %for.body.3, label %for.end.19

for.end.19:                                       ; preds = %for.inc.17
  br label %for.inc.20

for.inc.20:                                       ; preds = %for.end.19
  %inc21 = add nsw i32 %i.03, 1
  %cmp = icmp slt i32 %inc21, 100
  br i1 %cmp, label %for.body, label %for.end.22

for.end.22:                                       ; preds = %for.inc.20
  ret void
}

