; RUN: opt -pre-isel-intrinsic-lowering -S -o - %s | FileCheck %s

; CHECK-LABEL: @foo
define void @foo(i32* %p) {
  %q = call i32* @llvm.ssa.copy.i32(i32* %p)
; CHECK-NOT: llvm.ssa.copy
  store i32 1, i32* %q
; CHECK: store i32 1, i32* %p
  ret void
}

declare i32* @llvm.ssa.copy.i32(i32* returned %p) readnone

