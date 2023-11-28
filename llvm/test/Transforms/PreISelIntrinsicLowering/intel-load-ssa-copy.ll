; RUN: opt -mtriple=x86_64-pc-linux-gnu -pre-isel-intrinsic-lowering -S -o - %s | FileCheck %s

; CHECK-LABEL: @foo
define void @foo(ptr %p) {
  %q = call ptr @llvm.ssa.copy.i32(ptr %p)
; CHECK-NOT: llvm.ssa.copy
  store i32 1, ptr %q
; CHECK: store i32 1, ptr %p
  ret void
}

declare ptr @llvm.ssa.copy.i32(ptr returned %p) readnone
