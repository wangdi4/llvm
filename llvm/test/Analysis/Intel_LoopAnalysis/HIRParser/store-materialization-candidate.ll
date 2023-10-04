; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-allow-loop-materialization-regions=true -disable-output  2>&1 | FileCheck %s

; Verify that we create a region for a bblock with stores which looks like a materialization candidate.

; CHECK: BEGIN REGION

; CHECK: EntryBB: %entry.split

; CHECK: (%A)[1] = %t;
; CHECK: (%A)[2] = %t;
; CHECK: ret ;

; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(ptr nocapture %A, i32 %t) {
entry:
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 1
  store i32 %t, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr %A, i64 2
  store i32 %t, ptr %arrayidx1, align 4
  ret void
}

