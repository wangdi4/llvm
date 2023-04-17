; RUN: llvm-as < %s -o /dev/null 2>&1

define <2 x i32> @foo(<256 x float> %matrix, i64 %idx) {
  %coord = call <2 x i32> @llvm.experimental.matrix.wi.element.coordinate.v256f32(<256 x float> %matrix, i32 16, i32 16, metadata !"scope.subgroup")
  ret <2 x i32> %coord
}

declare <2 x i32> @llvm.experimental.matrix.wi.element.coordinate.v256f32(<256 x float>, i32, i32, metadata)
