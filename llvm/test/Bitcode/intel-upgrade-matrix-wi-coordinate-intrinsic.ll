; RUN: opt -S < %s | FileCheck %s
; RUN: llvm-dis < %s.bc | FileCheck %s

define <2 x i64> @foo() {
; CHECK: %call = call <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v2i64.v256f32(<256 x float> zeroinitializer, i32 16, i32 16, i64 0, metadata !"scope.subgroup")
  %call = call <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v256f32(<256 x float> zeroinitializer, i32 16, i32 16, i64 0, metadata !"scope.subgroup")
  ret <2 x i64> %call
}

; CHECK: declare <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v2i64.v256f32(<256 x float>, i32, i32, i64, metadata)
declare <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v256f32(<256 x float>, i32, i32, i64, metadata)
