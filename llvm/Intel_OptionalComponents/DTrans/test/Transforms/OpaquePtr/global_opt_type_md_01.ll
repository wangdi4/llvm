; This test verifies that intel_dtrans_type metadata is attached
; to @llvm.compiler.used global variables even after applying
; GlobalOpt pass.
;
; RUN: opt < %s -opaque-pointers -S -passes=globalopt 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @llvm.compiler.used = appending global [1 x ptr] [ptr @foo], section "llvm.metadata", !intel_dtrans_type ![[DT0:[0-9]+]]

; CHECK: ![[DT0]] = !{!"A", i32 1, ![[DT1:[0-9]+]]}
; CHECK: ![[DT1]] = !{i8 0, i32 1}

@llvm.compiler.used = appending global [1 x ptr] [ptr @foo], section "llvm.metadata", !intel_dtrans_type !1

define void @foo() {
  ret void
}

!0 = !{i8 0, i32 1}
!1 = !{!"A", i32 1, !0}
