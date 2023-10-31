; RUN: llvm-dis < %s.bc | FileCheck %s

; intel-upgrade-matrix-mad-intrinsic.ll.bc was generated using this file
; verifying auto-upgrade for the @llvm.experimental.matrix.(s/u)mad.*
; intrinsic functions

define <64 x i32> @foo(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C) {
  ; CHECK: call <64 x i32> @llvm.experimental.matrix.mad.v64i32.v128i32.v128i32(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  %result = call <64 x i32> @llvm.experimental.matrix.mad.v64i32.v128i32.v128i32(<128 x i32> %A, metadata !"matrix.columnmajor", <128 x i32> %B, metadata !"matrix.columnmajor", <64 x i32> %C, metadata !"matrix.columnmajor", i32 8, i32 16, i32 8, metadata !"scope.subgroup")
  ret <64 x i32> %result
}

define <64 x i32> @boo(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C) {
  ; CHECK: call <64 x i32> @llvm.experimental.matrix.sumad.v64i32.v128i32.v128i32(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  %result = call <64 x i32> @llvm.experimental.matrix.sumad.v64i32.v128i32.v128i32(<128 x i32> %A, metadata !"matrix.columnmajor", <128 x i32> %B, metadata !"matrix.columnmajor", <64 x i32> %C, metadata !"matrix.columnmajor", i32 8, i32 16, i32 8, metadata !"scope.subgroup")
  ret <64 x i32> %result
}

define <64 x i32> @goo(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C) {
  ; CHECK: call <64 x i32> @llvm.experimental.matrix.usmad.v64i32.v128i32.v128i32(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  %result = call <64 x i32> @llvm.experimental.matrix.usmad.v64i32.v128i32.v128i32(<128 x i32> %A, metadata !"matrix.columnmajor", <128 x i32> %B, metadata !"matrix.columnmajor", <64 x i32> %C, metadata !"matrix.columnmajor", i32 8, i32 16, i32 8, metadata !"scope.subgroup")
  ret <64 x i32> %result
}

define <64 x i32> @zoo(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C) {
  ; CHECK: call <64 x i32> @llvm.experimental.matrix.uumad.v64i32.v128i32.v128i32(<128 x i32> %A, <128 x i32> %B, <64 x i32> %C, i32 8, i32 16, i32 8, metadata !"scope.subgroup", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none", metadata !"matrix.reinterpret.type.none")
  %result = call <64 x i32> @llvm.experimental.matrix.uumad.v64i32.v128i32.v128i32(<128 x i32> %A, metadata !"matrix.columnmajor", <128 x i32> %B, metadata !"matrix.columnmajor", <64 x i32> %C, metadata !"matrix.columnmajor", i32 8, i32 16, i32 8, metadata !"scope.subgroup")
  ret <64 x i32> %result
}

; CHECK-DAG: declare <64 x i32> @llvm.experimental.matrix.mad.v64i32.v128i32.v128i32(<128 x i32>, <128 x i32>, <64 x i32>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)
declare <64 x i32> @llvm.experimental.matrix.mad.v64i32.v128i32.v128i32(<128 x i32>, metadata, <128 x i32>, metadata, <64 x i32>, metadata, i32, i32, i32, metadata)

; CHECK-DAG: declare <64 x i32> @llvm.experimental.matrix.sumad.v64i32.v128i32.v128i32(<128 x i32>, <128 x i32>, <64 x i32>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)
declare <64 x i32> @llvm.experimental.matrix.sumad.v64i32.v128i32.v128i32(<128 x i32>, metadata, <128 x i32>, metadata, <64 x i32>, metadata, i32, i32, i32, metadata)

; CHECK-DAG: declare <64 x i32> @llvm.experimental.matrix.usmad.v64i32.v128i32.v128i32(<128 x i32>, <128 x i32>, <64 x i32>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)
declare <64 x i32> @llvm.experimental.matrix.usmad.v64i32.v128i32.v128i32(<128 x i32>, metadata, <128 x i32>, metadata, <64 x i32>, metadata, i32, i32, i32, metadata)

; CHECK-DAG: declare <64 x i32> @llvm.experimental.matrix.uumad.v64i32.v128i32.v128i32(<128 x i32>, <128 x i32>, <64 x i32>, i32, i32, i32, metadata, metadata, metadata, metadata, metadata)
declare <64 x i32> @llvm.experimental.matrix.uumad.v64i32.v128i32.v128i32(<128 x i32>, metadata, <128 x i32>, metadata, <64 x i32>, metadata, i32, i32, i32, metadata)
