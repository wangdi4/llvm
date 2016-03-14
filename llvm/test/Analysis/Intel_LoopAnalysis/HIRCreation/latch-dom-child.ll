; RUN: opt < %s -analyze -hir-creation | FileCheck %s

; Check that the lexical links for the loop are created correctly when the loop header directly dominates loop latch block. Loop latch should be the lexically last bblock.
; CHECK: for.body.12:
; CHECK: if.else:
; CHECK: land.lhs.true.25:
; CHECK: if.then.28:
; CHECK: if.then.40:
; CHECK: if.then.46:
; CHECK: if.end:
; CHECK: for.inc.51:

; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noreturn nounwind
define void @main() #0 {
entry:
  br label %for.body.12

for.body.12:                                      ; preds = %for.inc.51, %entry
  %storemerge.28157 = phi i32 [ 1, %entry ], [ %inc52, %for.inc.51 ]
  br i1 undef, label %for.inc.51, label %if.else

if.else:                                          ; preds = %for.body.12
  br i1 undef, label %land.lhs.true.25, label %if.else.36

land.lhs.true.25:                                 ; preds = %if.else
  br i1 undef, label %if.then.28, label %if.else.36

if.then.28:                                       ; preds = %land.lhs.true.25
  br label %for.inc.51

if.else.36:                                       ; preds = %land.lhs.true.25, %if.else
  br i1 undef, label %for.inc.51, label %if.then.40

if.then.40:                                       ; preds = %if.else.36
  br i1 undef, label %if.then.46, label %if.end

if.then.46:                                       ; preds = %if.then.40
  br label %if.end

if.end:                                           ; preds = %if.then.46, %if.then.40
  br label %for.inc.51

for.inc.51:                                       ; preds = %if.end, %if.else.36, %if.then.28, %for.body.12
  %i.0 = phi i32 [ undef, %if.then.28 ], [ %storemerge.28157, %if.else.36 ], [ %storemerge.28157, %if.end ], [ %storemerge.28157, %for.body.12 ]
  %inc52 = add nsw i32 %i.0, 1
  %cmp1 = icmp slt i32 %inc52, 15
  br i1 %cmp1, label %for.body.12, label %if.end.54

if.end.54:                                        ; preds = %for.inc.51
  ret void
}
