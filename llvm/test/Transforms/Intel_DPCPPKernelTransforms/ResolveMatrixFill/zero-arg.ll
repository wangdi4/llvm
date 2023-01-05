; RUN: opt -passes=dpcpp-kernel-resolve-matrix-fill -S %s | FileCheck %s
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-resolve-matrix-fill -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-resolve-matrix-fill -enable-debugify -S %s 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-resolve-matrix-fill -enable-debugify -S %s 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

define void @test() {
entry:
; CHECK: %call.i = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 0, i32 12, i32 12, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  %call.i = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 0, i32 12, i32 12, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

declare <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32, i32, i32, metadata, metadata, metadata)

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
