; RUN: opt -S < %s | FileCheck %s
; RUN: llvm-dis < %s.bc | FileCheck %s

define void @test_fill() {
entry:
; CHECK: %call.i = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 0, i32 12, i32 12, metadata !"scope.subgroup", metadata !"matrix.use.a")
  %call.i = call <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32 0, i32 12, i32 12, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

define void @test_length(<144 x i32> %mat) {
entry:
; CHECK: %slice.length = call i64 @llvm.experimental.matrix.wi.slice.length.v144i32(<144 x i32> %mat, i32 12, i32 12, metadata !"scope.subgroup", metadata !"matrix.use.a")
  %slice.length = call i64 @llvm.experimental.matrix.wi.slice.length.v144i32(<144 x i32> %mat, i32 12, i32 12, metadata !"matrix.packed.a", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

define void @test_insert(<144 x i32> %mat, i32 %val, i64 %element.index) {
entry:
; CHECK: %the.element = call <144 x i32> @llvm.experimental.matrix.wi.slice.insertelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i32 %val, i64 %element.index, metadata !"scope.subgroup", metadata !"matrix.use.b")
  %the.element = call <144 x i32> @llvm.experimental.matrix.wi.slice.insertelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i32 %val, i64 %element.index, metadata !"matrix.columnmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

define void @test_extract(<144 x i32> %mat, i64 %element.index) {
entry:
; CHECK: %the.element = call i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index, metadata !"scope.subgroup", metadata !"matrix.use.b")
  %the.element = call i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index, metadata !"matrix.packed.b", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ret void
}

declare <144 x i32> @llvm.experimental.matrix.fill.v144i32.i32(i32, i32, i32, metadata, metadata, metadata)
declare i64 @llvm.experimental.matrix.wi.slice.length.v144i32(<144 x i32>, i32, i32, metadata, metadata, metadata)
declare <144 x i32> @llvm.experimental.matrix.wi.slice.insertelement.v144i32.i64(<144 x i32>, i32, i32, i32, i64, metadata, metadata, metadata)
declare i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144 x i32>, i32, i32, i64, metadata, metadata, metadata)
