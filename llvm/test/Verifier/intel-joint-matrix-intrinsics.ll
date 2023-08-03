; RUN: opt -passes="require<verify>" -S < %s 2>&1 | FileCheck --check-prefix=CHECK1 %s
; RUN: sed -e s/.T2:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK2 %s
; RUN: sed -e s/.T3:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK3 %s

; Test that the verifier accepts legal code

; CHECK1: @foo
define <2 x i64> @foo(<256 x float> %matrix, i64 %idx) {
  %coord = call <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v2i64.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
  ret <2 x i64> %coord
}

; CHECK1: @boo
define <2 x i32> @boo(<256 x float> %matrix, i64 %idx) {
  %coord = call <2 x i32> @llvm.experimental.matrix.wi.element.coordinate.v2i32.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
  ret <2 x i32> %coord
}

declare <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v2i64.v256f32(<256 x float>, i32, i32, i64, metadata)

declare <2 x i32> @llvm.experimental.matrix.wi.element.coordinate.v2i32.v256f32(<256 x float>, i32, i32, i64, metadata)

; check for incorrect number of elements in the return vector

; CHECK2: experimental_matrix_wi_element_coordinate must return a two-element integer vector
;T2: define <3 x i64> @goo(<256 x float> %matrix, i64 %idx) {
;T2:   %coord = call <3 x i64> @llvm.experimental.matrix.wi.element.coordinate.v3i64.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
;T2:   ret <3 x i64> %coord
;T2: }
;T2:
;T2: declare <3 x i64> @llvm.experimental.matrix.wi.element.coordinate.v3i64.v256f32(<256 x float>, i32, i32, i64, metadata)

; check for incorrect element type in the return vector

; CHECK3: experimental_matrix_wi_element_coordinate must return a two-element integer vector
;T3: define <2 x float> @zoo(<256 x float> %matrix, i64 %idx) {
;T3:   %coord = call <2 x float> @llvm.experimental.matrix.wi.element.coordinate.v2f32.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
;T3:   ret <2 x float> %coord
;T3: }
;T3:
;T3: declare <2 x float> @llvm.experimental.matrix.wi.element.coordinate.v2f32.v256f32(<256 x float>, i32, i32, i64, metadata)
