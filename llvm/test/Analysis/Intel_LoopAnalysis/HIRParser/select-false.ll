; Check that HIR parsing of select instruction with FALSE predicate is folded into the constant operand.

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details -hir-details-constants | FileCheck %s

; CHECK: NSW: Yes

; CHECK: DO i32 i1 = 0, 4
; CHECK: (%A)[0][i1] = 1;
; CHECK: <RVAL-REG> i32 1 {sb:1}
; CHECK: END LOOP

; ModuleID = '2.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() {
entry:
  %A = alloca [5 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 %idxprom
  %sel = select i1 false, i32 %i.01, i32 1
  store i32 %sel, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 5
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %arrayidx1 = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 0
  %0 = load i32, i32* %arrayidx1, align 4
  ret i32 %0
}

