; RUN: llvm-as < %s -o /dev/null 2>&1

define <2 x i64> @foo(<256 x float> %matrix, i64 %idx) {
  %coord = call <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
  ret <2 x i64> %coord
}

declare <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v256f32(<256 x float>, i32, i32, i64, metadata)
