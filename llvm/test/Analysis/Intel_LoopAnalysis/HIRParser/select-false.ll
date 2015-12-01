; Check HIR parsing of select instruction with FALSE predicate 
; |   %sel = (0 false 0) ? i1 : 1;
; |   <REG> NON-LINEAR i32 %sel {sb:6}
; |   <REG> LINEAR i1 0 {undefined} {sb:1}
; |   <REG> LINEAR i1 0 {undefined} {sb:1}
; |   <REG> LINEAR i32 i1 {sb:3}

; RUN: opt < %s -loop-rotate | opt -analyze -hir-parser -hir-details | FileCheck %s
; This has been marked as XFAIL due to CQ 378686
; XFAIL: *

; CHECK:      = ({{.*}}false{{.*}})
; CHECK:      <REG>{{.*}}{undefined}
; CHECK-NEXT: <REG>{{.*}}{undefined}

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
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 %idxprom
  %sel = select i1 0, i32 %i.0, i32 1
  store i32 %sel, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arrayidx1 = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 0
  %0 = load i32, i32* %arrayidx1, align 4
  ret i32 %0
}

