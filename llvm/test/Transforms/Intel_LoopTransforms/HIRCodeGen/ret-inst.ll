; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-cg" < %s -force-hir-cg -S | FileCheck %s

; Verify that return instruction is correctly handled by CG.

; CHECK: region.0:
; CHECK: ret void

; ModuleID = 't2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(ptr nocapture %arr, ptr nocapture %barr) {
entry:
  br label %exit

exit:
  ret void
}

