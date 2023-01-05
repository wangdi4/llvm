; RUN: opt < %s -passes=hir-ssa-deconstruction -S | FileCheck %s

; Verify that we create a single operand phi copy for the liveout %add in the maerialization candidate region.

; CHECK: %liveoutcopy = phi i32 [ %add, %entry.split ]
; CHECK: ret i32 %liveoutcopy

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(i32* nocapture readonly %A) {
entry:
  %0 = load i32, i32* %A, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %A, i64 1
  %1 = load i32, i32* %arrayidx1, align 4
  %add = add nsw i32 %1, %0
  br label %exit

exit:
  ret i32 %add
}

