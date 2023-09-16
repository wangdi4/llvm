; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser -disable-output < %s 2>&1 | FileCheck %s

; Verify that the smax blob smax(0, (-2 + (2 * %0))) is not broken with a
; multiplier of 2 like this-
; 2 * smax(0, (-1 + %0))

; This transformation is incorrect if the operations inside min/max expr
; overflow.

; CHECK: + DO i1 = 0, 5, 1   <DO_LOOP>
; CHECK: |   %0 = (@arr_99)[0][3 * i1 + 1];
; CHECK: |   %spec.select = @llvm.smax.i64(2 * %0 + -2,  0);
; CHECK: |   %conv13 = smax(0, (-2 + (2 * %0)))  &  254;
; CHECK: |   %xor2728 = 2 * zext.i7.i64(trunc.i64.i7((smax(0, (-2 + (2 * %0))) /u 2)))  ^  %xor2728;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr_99 = dso_local local_unnamed_addr global [17 x i64] zeroinitializer, align 16

define dso_local void @main(i64 %seed.promoted) {
entry:
  br label %for.body

for.body:                                        ; preds = %entry, %for.body
  %indvars.iv30 = phi i64 [ 1, %entry ], [ %indvars.iv.next31, %for.body ]
  %xor2728 = phi i64 [ %seed.promoted, %entry ], [ %xor, %for.body ]
  %arrayidx7 = getelementptr inbounds [17 x i64], ptr @arr_99, i64 0, i64 %indvars.iv30
  %0 = load i64, ptr %arrayidx7, align 8
  %add = shl i64 %0, 1
  %shl = add i64 %add, -2
  %spec.select = tail call i64 @llvm.smax.i64(i64 %shl, i64 0)
  %conv13 = and i64 %spec.select, 254
  %xor = xor i64 %conv13, %xor2728
  %indvars.iv.next31 = add nuw nsw i64 %indvars.iv30, 3
  %cmp3 = icmp ult i64 %indvars.iv30, 14
  br i1 %cmp3, label %for.body, label %for.cond.cleanup4

for.cond.cleanup4:                                ; preds = %for.body
  %spec.select.lcssa = phi i64 [ %spec.select, %for.body ]
  %conv13.lcssa = phi i64 [ %conv13, %for.body ]
  %xor.lcssa = phi i64 [ %xor, %for.body ]
  ret void
}

declare i64 @llvm.smax.i64(i64, i64)

