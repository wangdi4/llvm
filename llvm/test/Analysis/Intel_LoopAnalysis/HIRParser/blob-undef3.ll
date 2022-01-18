; Check HIR parsing of cases with undefined values in CanonExpr (Blob Coef in Mul)
; |   (%A)[0][i1] = 5 * i1 + undef * %0 + 1;
; |   <LVAL-REG> (LINEAR [5 x i32]* %A)[0][LINEAR i64 i1] {sb:0}
; |      <BLOB> LINEAR [5 x i32]* %A {sb:12}
; |   <RVAL-REG> NON-LINEAR i32 5 * i1 + undef * %0 + 1 {sb:2}
; |      <BLOB> NON-LINEAR i32 %0 {sb:5}

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; CHECK: HasSignedIV: Yes

; CHECK: ={{.*}}undef * %0{{.*}};
; CHECK: <RVAL-REG>{{.*}}undef * %0{{.*}}
; undef is assumed as a constant so we do not create blob ddrefs for it.
; CHECK-NOT: <BLOB> {{.*}} undef


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
