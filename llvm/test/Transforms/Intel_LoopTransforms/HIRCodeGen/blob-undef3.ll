
; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-cg" < %s -force-hir-cg -S | FileCheck %s
; Check HIR CG of cases with undefined values in CanonExpr (Blob Coef in Mul)
; |   (%A)[0][i1] = 5 * i1 + undef * %0 + 1;
; |   <LVAL-REG> (LINEAR [5 x i32]* %A)[0][LINEAR i64 i1] {sb:0}
; |      <BLOB> LINEAR [5 x i32]* %A {sb:12}
; |   <RVAL-REG> NON-LINEAR i32 5 * i1 + undef * %0 + 1 {undefined} {sb:9}
; |      <BLOB> LINEAR i32 undef {undefined} {sb:13}
; |      <BLOB> NON-LINEAR i32 %0 {sb:5}

; verify 5 * i1 + undef * %0 + 1;

; check blob calculation
;CHECK: [[BLOB_LOAD:%t[0-9]+.*]] = load i32, i32*
;CHECK: [[BLOB_MUL:%.*]] = mul nsw i32 [[BLOB_LOAD]], undef

;and that its added to 5*i
;CHECK: [[IV_LOAD:%.*]] = load i32, i32* %i1.i32
;CHECK: [[IV_MUL:%.*]] = mul i32 5, [[IV_LOAD]]
;CHECK: [[IV_BLOB_SUM:%.*]] = add i32 [[BLOB_MUL]], [[IV_MUL]]
;CHECK: add i32 [[IV_BLOB_SUM]], 1

; Check wrap flags on IV
;CHECK: [[IV_LOAD1:%.*]] = load i32, i32* %i1.i32
;CHECK: [[IV_UPDATE:%.*]] = add nuw nsw i32 [[IV_LOAD1]], 1
;CHECK: icmp ne i32 [[IV_LOAD1]]

; ModuleID = 'blob-undef3.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common global i32 0, align 4

define i32 @main() {
entry:
  %A = alloca [5 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %mul = mul nsw i32 5, %i.01
  %0 = load i32, i32* @x, align 4
  %y = load i32, i32* @x, align 8
  %mul1 = mul nsw i32 undef, %0
  %add = add nsw i32 %mul, %mul1
  %add2 = add nsw i32 %add, 1
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 %idxprom
  store i32 %add2, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 5
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %arrayidx3 = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 0
  %1 = load i32, i32* %arrayidx3, align 4
  ret i32 %1
}
