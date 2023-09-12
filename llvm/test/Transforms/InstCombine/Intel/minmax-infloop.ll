; RUN: opt -enable-intel-advanced-opts=true -passes=instcombine -S < %s | FileCheck %s

; If we allow FoldOpIntoSelect to convert the select operands to i8 and
; hoist the trunc above the select, we will sink the trunc back down in
; visitSelectInst, creating an infinite loop.
; min/max recognition will avoid this case, but we suppress min/max with the
; combination of options and flags in this test.
; min/max was partially disabled with commit 4e9df2a0d0.

; CHECK: select i1{{.*}}i32 0, i32

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(double %0, ptr %dest) #0 {
entry:
  %conv13 = fptosi double %0 to i32
  %cmp37 = icmp slt i32 %conv13, 0
  %spec.store.select = select i1 %cmp37, i32 0, i32 %conv13
  %conv62 = trunc i32 %spec.store.select to i8
  store i8 %conv62, ptr %dest, align 1
  ret void
}


attributes #0 = { "pre_loopopt" "target-cpu"="skylake-avx512" }
