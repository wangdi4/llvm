; Check HIR parsing of IF instruction with undefined predicate

; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; CHECK: HasSignedIV: Yes

; CHECK:      if (undef != 0)

; ModuleID = 'if-undef.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main() {
entry:
  %A = alloca [5 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  br i1 undef, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [5 x i32], ptr %A, i32 0, i64 %idxprom
  store i32 %i.01, ptr %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 5
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %arrayidx2 = getelementptr inbounds [5 x i32], ptr %A, i32 0, i64 0
  %0 = load i32, ptr %arrayidx2, align 4
  ret i32 %0
}
