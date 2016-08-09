; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that loop's upper was converted to positive value by parser.

; CHECK: + DO i1 = 0, 253, 1   <DO_LOOP>
; CHECK: |   {al:1}(%A)[i1] = i1;
; CHECK: + END LOOP


; ModuleID = 'char.c'
source_filename = "char.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i8* nocapture %A) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i8 [ 0, %entry ], [ %inc, %for.body ]
  %idxprom = zext i8 %i.05 to i64
  %arrayidx = getelementptr inbounds i8, i8* %A, i64 %idxprom
  store i8 %i.05, i8* %arrayidx, align 1
  %inc = add i8 %i.05, 1
  %cmp = icmp ult i8 %inc, -2
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

