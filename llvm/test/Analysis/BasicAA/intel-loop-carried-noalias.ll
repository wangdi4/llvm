; RUN: opt < %s -basic-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt -convert-to-subscript -S < %s | opt -basic-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
source_filename = "m1.cc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @_Z3fooPA2_ii([2 x i32]* %A, i32 %n) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %storemerge = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %storemerge, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %storemerge to i64
; No dependence inside the loop,
; loop carried is not reported
; A[i][0] += 2;
; CHECK: NoAlias: i32* %arrayidx1, i32* %arrayidx4
  %arrayidx1 = getelementptr inbounds [2 x i32], [2 x i32]* %A, i64 %idxprom, i64 2
  %0 = load i32, i32* %arrayidx1, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, i32* %arrayidx1, align 4
  %idxprom2 = sext i32 %storemerge to i64
; A[i][0] += 2;
  %arrayidx4 = getelementptr inbounds [2 x i32], [2 x i32]* %A, i64 %idxprom2, i64 0
  %1 = load i32, i32* %arrayidx4, align 4
  %add5 = add nsw i32 %1, 2
  store i32 %add5, i32* %arrayidx4, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %storemerge, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { noinline nounwind uwtable }
