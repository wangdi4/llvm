; RUN: opt -passes=sycl-kernel-resolve-matrix-fill -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-resolve-matrix-fill -enable-debugify -S %s 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

define void @test() {
entry:
; CHECK: %call.i = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 0, i32 12, i32 12, metadata !"scope.subgroup", metadata !"matrix.use.a")
  %call.i = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 0, i32 12, i32 12, metadata !"scope.subgroup", metadata !"matrix.use.a")
  ret void
}

declare <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32, i32, i32, metadata, metadata)

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
