; RUN: opt < %s -loop-simplify -hir-de-ssa | opt -analyze -hir-parser | FileCheck %s

; This command checks that -hir-de-ssa invalidates SCEV so that the parser doesn't pick up the cached version. HIR output should be the same as for the above command.
; RUN: opt < %s -loop-simplify -hir-de-ssa -HIRCompleteUnroll -print-before=HIRCompleteUnroll 2>&1 | FileCheck %s

; Check parsing output for the loop with division in upper
; CHECK: DO i1 = 0, 6, 1
; CHECK-NEXT: %add.out = %add
; CHECK-NEXT: %add = %add  +  %ii.0
; CHECK-NEXT: %ii.0 = %ii.0  /  2
; CHECK-NEXT: if (%add.out + 1 < %ii.0 + %add)
; CHECK-NEXT: {
; CHECK-NEXT: DO i2 = 0, (%add.out + %ii.0 + -1 * %add + -2)/u2, 1
; CHECK-NEXT: %4 = (%A)[2 * i2 + %add.out + 1]
; CHECK-NEXT: %5 = (%A)[2 * i2 + %add.out];
; CHECK-NEXT: %6 = (%A)[2 * i2 + %add.out + 2]
; CHECK-NEXT: (%A)[i2 + %ii.0 + %add + 1] = ((1 + (-1 * %5)) * %4) + -1 * (%6 * %6)
; CHECK-NEXT: END LOOP
; CHECK-NEXT: }
; CHECK-NEXT: END LOOP



; ModuleID = 'div1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A) {
entry:
  br label %do.body

do.body:                                          ; preds = %do.cond, %entry
  %ii.0 = phi i64 [ 64, %entry ], [ %div, %do.cond ]
  %ipntp.0 = phi i64 [ 0, %entry ], [ %add, %do.cond ]
  %add = add nsw i64 %ipntp.0, %ii.0
  %div = sdiv i64 %ii.0, 2
  %add1 = add i64 %ipntp.0, 1
  %cmp.36 = icmp slt i64 %add1, %add
  br i1 %cmp.36, label %for.body.lr.ph, label %do.cond

for.body.lr.ph:                                   ; preds = %do.body
  %0 = add i64 %add1, %ii.0
  %1 = add nsw i64 %ii.0, -2
  %2 = lshr i64 %1, 1
  %3 = add i64 %0, %2
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %k.038 = phi i64 [ %add1, %for.body.lr.ph ], [ %add12, %for.body ]
  %i.037 = phi i64 [ %add, %for.body.lr.ph ], [ %inc, %for.body ]
  %inc = add nsw i64 %i.037, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %k.038
  %4 = load i32, i32* %arrayidx, align 4
  %sub = add nsw i64 %k.038, -1
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 %sub
  %5 = load i32, i32* %arrayidx3, align 4
  %mul = mul nsw i32 %5, %4
  %sub4 = sub i32 %4, %mul
  %add5 = add nsw i64 %k.038, 1
  %arrayidx6 = getelementptr inbounds i32, i32* %A, i64 %add5
  %6 = load i32, i32* %arrayidx6, align 4
  %mul9 = mul nsw i32 %6, %6
  %sub10 = sub i32 %sub4, %mul9
  %arrayidx11 = getelementptr inbounds i32, i32* %A, i64 %inc
  store i32 %sub10, i32* %arrayidx11, align 4
  %add12 = add nsw i64 %k.038, 2
  %exitcond = icmp eq i64 %inc, %3
  br i1 %exitcond, label %do.cond, label %for.body

do.cond:                                          ; preds = %for.body, %do.body
  %cmp13 = icmp sgt i64 %ii.0, 1
  br i1 %cmp13, label %do.body, label %do.end

do.end:                                           ; preds = %do.cond
  ret void
}

