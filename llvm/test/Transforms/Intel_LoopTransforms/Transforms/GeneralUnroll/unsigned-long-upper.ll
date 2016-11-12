; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-before=hir-general-unroll -print-after=hir-general-unroll < %s 2>&1 | FileCheck %s

; Verify that general unroll is able to handle loops with very big constant trip counts which are outside the positive int64_t range.

; CHECK: Before HIR General Unroll
; CHECK: + DO i1 = 0, -1152921504606846979, 1   <DO_LOOP>
; CHECK: |   %rem = i1  %u  50;
; CHECK: |   %0 = (@A)[0][%rem];
; CHECK: |   %rem1 = i1 + 1  %u  50;
; CHECK: |   (@A)[0][%rem1] = i1 + %0 + 1;
; CHECK: + END LOOP

; CHECK: After HIR General Unroll
; CHECK: + DO i1 = 0, 2161727821137838078, 1   <DO_LOOP>
; CHECK: DO i1 = -1152921504606846984, -1152921504606846979, 1


source_filename = "unsigned-long-iv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [50 x i64] zeroinitializer, align 16
@.str = private unnamed_addr constant [4 x i8] c"%lu\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.08 = phi i64 [ 1, %entry ], [ %inc, %for.body ]
  %sub = add i64 %i.08, -1
  %rem = urem i64 %sub, 50
  %arrayidx = getelementptr inbounds [50 x i64], [50 x i64]* @A, i64 0, i64 %rem
  %0 = load i64, i64* %arrayidx, align 8
  %add = add i64 %0, %i.08
  %rem1 = urem i64 %i.08, 50
  %arrayidx2 = getelementptr inbounds [50 x i64], [50 x i64]* @A, i64 0, i64 %rem1
  store i64 %add, i64* %arrayidx2, align 8
  %inc = add nuw i64 %i.08, 1
  %exitcond = icmp eq i64 %inc, -1152921504606846977
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

