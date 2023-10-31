; RUN: opt -passes="require<verify>" -S < %s 2>&1 | FileCheck --check-prefix=CHECK1 %s

; RUN: sed -e s/.T2:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK_INV_COORD_1 %s
; RUN: sed -e s/.T3:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK_INV_COORD_2 %s

; RUN: sed -e s/.T4:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK_INV_MAD1 %s
; RUN: sed -e s/.T5:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK_INV_MAD2 %s
; RUN: sed -e s/.T6:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK_INV_MAD3 %s
; RUN: sed -e s/.T7:// %s | not opt -passes="require<verify>" -disable-output 2>&1 | FileCheck --check-prefix=CHECK_INV_MAD4 %s

; Test that the verifier accepts legal code

; CHECK1: @coord1
define <2 x i64> @coord1(<256 x float> %matrix, i64 %idx) {
  %coord = call <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v2i64.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
  ret <2 x i64> %coord
}

; CHECK1: @coord2
define <2 x i32> @coord2(<256 x float> %matrix, i64 %idx) {
  %coord = call <2 x i32> @llvm.experimental.matrix.wi.element.coordinate.v2i32.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
  ret <2 x i32> %coord
}

; CHECK1: @mad1
define <64 x float> @mad1(<128 x float> %A, <128 x float> %B, <64 x float> %C) {
  %result = call <64 x float> @llvm.experimental.matrix.mad.v64f32.v128f32.v128f32(<128 x float> %A, <128 x float> %B, <64 x float> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  ret <64 x float> %result
}

; CHECK1: @mad2
define <64 x float> @mad2(<128 x i16> %A, <128 x i16> %B, <64 x float> %C) {
  %result = call <64 x float> @llvm.experimental.matrix.mad.v64f32.v128i16.v128i16(<128 x i16> %A, <128 x i16> %B, <64 x float> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  ret <64 x float> %result
}

; CHECK1: @sumad
define <64 x i32> @sumad(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C) {
  %result = call <64 x i32> @llvm.experimental.matrix.sumad.v64i32.v128i32.v128i32(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  ret <64 x i32> %result
}

; CHECK1: @usmad
define <16 x i32> @usmad(<32 x i32> %A, <16 x i32> %B, <16 x i32> %C) {
  %result = call <16 x i32> @llvm.experimental.matrix.usmad.v16i32.v32i32.v16i32(<32 x i32> %A, <16 x i32> %B, <16 x i32> %C, i32 8, i32 4, i32 4, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  ret <16 x i32> %result
}

; CHECK1: @uumad
define <64 x i64> @uumad(<128 x i32> %A, <128 x i32> %B, <64 x i64> %C) {
  %result = call <64 x i64> @llvm.experimental.matrix.uumad.v64i64.v128i32.v128i32(<128 x i32> %A, <128 x i32> %B, <64 x i64> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  ret <64 x i64> %result
}

declare <2 x i64> @llvm.experimental.matrix.wi.element.coordinate.v2i64.v256f32(<256 x float>, i32, i32, i64, metadata)

declare <2 x i32> @llvm.experimental.matrix.wi.element.coordinate.v2i32.v256f32(<256 x float>, i32, i32, i64, metadata)

declare <64 x float> @llvm.experimental.matrix.mad.v64f32.v128f32.v128f32(<128 x float>, <128 x float>, <64 x float>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)

declare <64 x float> @llvm.experimental.matrix.mad.v64f32.v128i16.v128i16(<128 x i16>, <128 x i16>, <64 x float>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)

declare <64 x i32> @llvm.experimental.matrix.sumad.v64i32.v128i32.v128i32(<128 x i32>, <128 x i32>, <64 x i32>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)

declare <16 x i32> @llvm.experimental.matrix.usmad.v16i32.v32i32.v16i32(<32 x i32>, <16 x i32>, <16 x i32>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)

declare <64 x i64> @llvm.experimental.matrix.uumad.v64i64.v128i32.v128i32(<128 x i32>, <128 x i32>, <64 x i64>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)

; check for incorrect number of elements in the return vector of llvm.experimental.matrix.wi.element.coordinate intrinsic

; CHECK_INV_COORD_1: experimental_matrix_wi_element_coordinate must return a two-element integer vector
;T2: define <3 x i64> @coord_inv(<256 x float> %matrix, i64 %idx) {
;T2:   %coord = call <3 x i64> @llvm.experimental.matrix.wi.element.coordinate.v3i64.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
;T2:   ret <3 x i64> %coord
;T2: }
;T2:
;T2: declare <3 x i64> @llvm.experimental.matrix.wi.element.coordinate.v3i64.v256f32(<256 x float>, i32, i32, i64, metadata)

; check for incorrect element type in the return vector of llvm.experimental.matrix.wi.element.coordinate intrinsic

; CHECK_INV_COORD_2: experimental_matrix_wi_element_coordinate must return a two-element integer vector
;T3: define <2 x float> @coord_inv(<256 x float> %matrix, i64 %idx) {
;T3:   %coord = call <2 x float> @llvm.experimental.matrix.wi.element.coordinate.v2f32.v256f32(<256 x float> %matrix, i32 16, i32 16, i64 %idx, metadata !"scope.subgroup")
;T3:   ret <2 x float> %coord
;T3: }
;T3:
;T3: declare <2 x float> @llvm.experimental.matrix.wi.element.coordinate.v2f32.v256f32(<256 x float>, i32, i32, i64, metadata)

; check for incorrect rowA parameter nature

; CHECK_INV_MAD1: 4th, 5th and 6th parameters of experimental_matrix_(s/u)mad family of intrinsic functions must be integer constants
;T4: define <64 x float> @mad_inv(<128 x float> %A, <128 x float> %B, <64 x float> %C, i32 %val) {
;T4:   %result = call <64 x float> @llvm.experimental.matrix.mad.v64f32.v128f32.v128f32(<128 x float> %A, <128 x float> %B, <64 x float> %C, i32 %val, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
;T4:   ret <64 x float> %result
;T4: }

; check for inconsistent row/col parameters values

; CHECK_INV_MAD2: Inconsistent number of rows and columns of matrix A, B and C used by an intrinsic from experimental_matrix_(s/u)mad family
;T5: define <64 x float> @mad_inv(<128 x float> %A, <128 x float> %B, <64 x float> %C) {
;T5:   %result = call <64 x float> @llvm.experimental.matrix.mad.v64f32.v128f32.v128f32(<128 x float> %A, <128 x float> %B, <64 x float> %C, i32 16, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
;T5:   ret <64 x float> %result
;T5: }

; CHECK_INV_MAD3: Inconsistent number of rows and columns of matrix A, B and C used by an intrinsic from experimental_matrix_(s/u)mad family
;T6: define <64 x float> @mad_inv(<128 x float> %A, <128 x float> %B, <64 x float> %C) {
;T6:   %result = call <64 x float> @llvm.experimental.matrix.mad.v64f32.v128f32.v128f32(<128 x float> %A, <128 x float> %B, <64 x float> %C, i32 8, i32 16, i32 16, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
;T6:   ret <64 x float> %result
;T6: }

; CHECK_INV_MAD4: experimental_matrix_(s/u)mad family of intrinsic functions with an exception of experimental_matrix_mad must operate over matrices of integer type
;T7: define <64 x float> @mad_inv(<128 x float> %A, <128 x float> %B, <64 x float> %C) {
;T7:   %result = call <64 x float> @llvm.experimental.matrix.uumad.v64f32.v128f32.v128f32(<128 x float> %A, <128 x float> %B, <64 x float> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
;T7:   ret <64 x float> %result
;T7: }
;T7:
;T7: declare <64 x float> @llvm.experimental.matrix.uumad.v64f32.v128f32.v128f32(<128 x float>, <128 x float>, <64 x float>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)
