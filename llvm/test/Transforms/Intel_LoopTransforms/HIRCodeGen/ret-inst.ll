; RUN: opt < %s -hir-create-function-level-region -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s
; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-cg" < %s -force-hir-cg -S | FileCheck %s

; Verify that return instruction is correctly handled by CG.

; CHECK: region.0:
; CHECK: ret void

; ModuleID = 't2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %arr, i32* nocapture %barr) {
entry:
  br label %exit

exit:
  ret void
}

