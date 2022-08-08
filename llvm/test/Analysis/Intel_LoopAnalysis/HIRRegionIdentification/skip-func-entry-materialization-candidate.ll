; RUN: opt < %s -passes="print<hir-region-identification>" -debug-only=hir-region-identification 2>&1 | FileCheck %s

; Verify that we skip materialization candidate in the function entry block if
; it contains an alloca inst.

; CHECK: Skipping function entry block containing alloca as loop materialization candidate.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32* nocapture %A, i32 %t) {
entry:
  %alloc = alloca i32, align 4
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 1
  store i32 %t, i32* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %A, i64 2
  store i32 %t, i32* %arrayidx1, align 4
  ret void
}

