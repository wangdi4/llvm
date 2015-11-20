; Check HIR parsing of IF instruction with TRUE predicate
; |   if (0 true 0)
; |   <RVAL-REG> LINEAR i1 0 {undefined} {sb:1}
; |   <RVAL-REG> LINEAR i1 0 {undefined} {sb:1}

; RUN: opt < %s -loop-rotate | opt -analyze -hir-parser -hir-details | FileCheck %s

; CHECK:      if ({{.*}}true{{.*}})
; CHECK:      <RVAL-REG>{{.*}}{undefined}
; CHECK-NEXT: <RVAL-REG>{{.*}}{undefined}

; ModuleID = '2.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %A = alloca [5 x i32], align 16
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br i1 true, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 %idxprom
  store i32 %i.0, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arrayidx2 = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 0
  %0 = load i32, i32* %arrayidx2, align 4
  ret i32 %0
}

