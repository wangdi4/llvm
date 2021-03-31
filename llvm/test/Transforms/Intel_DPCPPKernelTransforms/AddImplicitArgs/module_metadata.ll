; RUN: opt -dpcpp-kernel-add-implicit-args %s -S | FileCheck %s
; check the metadata is preserved correctly during transformations

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() {
entry:
  ret void
}

; CHECK: !opencl.spir.version = !{![[VER:[0-9]+]]}
; CHECK: ![[VER]] = !{i32 2, i32 0}

!opencl.spir.version = !{!0}
!0 = !{i32 2, i32 0}
