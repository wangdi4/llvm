; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we create a region for a bblock with loads and stores which looks like a materialization candidate.

; CHECK: BEGIN REGION
; CHECK: EntryBB: %entry.split

; CHECK: %0 = (%B)[0];
; CHECK: (%A)[0] = %0;
; CHECK: %1 = (%B)[1];
; CHECK: (%A)[1] = %1;
; CHECK: %2 = (%B)[2];
; CHECK: (%A)[2] = %2;
; CHECK: ret ;

; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %A, i32* nocapture readonly %B) {
entry:
  %0 = load i32, i32* %B, align 4
  store i32 %0, i32* %A, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %B, i64 1
  %1 = load i32, i32* %arrayidx2, align 4
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 1
  store i32 %1, i32* %arrayidx3, align 4
  %arrayidx4 = getelementptr inbounds i32, i32* %B, i64 2
  %2 = load i32, i32* %arrayidx4, align 4
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 2
  store i32 %2, i32* %arrayidx5, align 4
  ret void
}

