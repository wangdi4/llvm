; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we skip unreachable materialization candidates.

; CHECK-NOT: REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(ptr %A, ptr %B, float %t, i32 %n) {
entry:
  %arrayidx3 = getelementptr inbounds float, ptr %A, i64 1
  %arrayidx4 = getelementptr inbounds float, ptr %A, i64 2
  ret void

unreachable:
  store float %t, ptr %arrayidx3, align 4
  store float %t, ptr %arrayidx4, align 4
  br label %unreachable
}

