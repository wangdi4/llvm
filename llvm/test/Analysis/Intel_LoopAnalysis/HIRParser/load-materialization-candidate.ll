; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-allow-loop-materialization-regions=true -disable-output  2>&1 | FileCheck %s

; Verify that we create a region for a bblock with stores which looks like a materialization candidate.

; CHECK: BEGIN REGION
; CHECK: EntryBB: %entry.split

; CHECK: %0 = (%A)[0];
; CHECK: %1 = (%A)[1];
; CHECK: ret %0 + %1;

; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(ptr nocapture readonly %A) {
entry:
  %0 = load i32, ptr %A, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr %A, i64 1
  %1 = load i32, ptr %arrayidx1, align 4
  %add = add nsw i32 %1, %0
  ret i32 %add
}

