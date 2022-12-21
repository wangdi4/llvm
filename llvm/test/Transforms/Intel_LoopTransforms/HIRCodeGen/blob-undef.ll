
; RUN: opt -passes="hir-ssa-deconstruction,hir-cg" < %s -force-hir-cg -S | FileCheck %s
; Check HIR CG of cases with undefined values in CanonExpr (Add)
; |   (%A)[0][i1] = 5 * i1 + 2 * %0 + undef;
; |   <LVAL-REG> (LINEAR [5 x i32]* %A)[0][LINEAR i64 i1] {sb:0}
; |      <BLOB> LINEAR [5 x i32]* %A {sb:12}
; |   <RVAL-REG> NON-LINEAR i32 5 * i1 + 2 * %0 + undef {undefined} {sb:9}
; |      <BLOB> LINEAR i32 undef {undefined} {sb:13}
; |      <BLOB> NON-LINEAR i32 %0 {sb:5}

; Really only care about the folowing canon expr here
; 5 * i1 + 2 * %0 + undef;

; check blob calculation
;CHECK: [[BLOB_LOAD:%t[0-9]+.*]] = load i32, i32*
;CHECK: [[BLOB_MUL:%.*]] = shl i32 [[BLOB_LOAD]], 1
;CHECK: [[UNDEF_ADD:%.*]] = add i32 [[BLOB_MUL]], undef

;and that its added to 5*i
;CHECK: [[IV_LOAD:%.*]] = load i32, i32* %i1.i32
;CHECK: [[IV_MUL:%.*]] = mul i32 5, [[IV_LOAD]]
;CHECK: add i32 [[UNDEF_ADD]], [[IV_MUL]]


; ModuleID = 'blob-undef.ll'
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
  %mul1 = mul nsw i32 2, %0
  %add = add nsw i32 %mul, %mul1
  %add2 = add nsw i32 %add, undef
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

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1250) (llvm/branches/loopopt 1310)"}
